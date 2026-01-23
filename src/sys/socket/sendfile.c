#include <sys/socket.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/param.h>
#include <strings.h>
#include <limits.h>

// Avoid Kernel includes
/* In-kernel representation */
struct user_sf_hdtr {
	u_int64_t headers;    /* pointer to an array of header struct iovec's */
	int hdr_cnt;            /* number of header iovec's */
	u_int64_t trailers;   /* pointer to an array of trailer struct iovec's */
	int trl_cnt;            /* number of trailer iovec's */
};

/* LP64 user version of struct sf_hdtr */
struct user64_sf_hdtr {
	__uint64_t headers;  /* pointer to an array of header struct iovec's */
	int hdr_cnt;            /* number of header iovec's */
	__uint64_t trailers; /* pointer to an array of trailer struct iovec's */
	int trl_cnt;            /* number of trailer iovec's */
};

// This normally processed by __DARWIN_NOCANCEL macro, but we explicitly want it
extern ssize_t writev$NOCANCEL(int, const struct iovec *, int);

// Buffer size for sendfile operations (256KB should be reasonable)
#define SENDFILE_BUFFER_SIZE (256 * 1024)

int compat_sendfile(int fd, int s, off_t offset, off_t *len, struct sf_hdtr *hdtr, int flags)
{
    off_t sbytes = 0;        // Total bytes sent (headers + file data + trailers)
    off_t nbytes = *len;     // Max bytes to send for headers + file data (0 = unlimited)
    off_t file_size;
    ssize_t writev_retval;
    struct stat fd_stat_buf, s_stat_buf;
    int ret = 0;
    char *buffer = NULL;
    off_t orig_offset = -1;
    int socket_type;
    socklen_t optlen = sizeof(socket_type);
    off_t sent_headers_and_file = 0; // Bytes sent for headers + file data

    // Initialize len to 0 for error cases (matches system behavior)
    *len = 0;

    // Apple reserved this flag for potential future use (must be 0)
    if (flags != 0) {
        errno = EINVAL;
        *len = sbytes;  // Set to 0 on error
        return -1;
    }

    if (offset < 0) {
        errno = EINVAL;
        *len = sbytes;  // Set to 0 on error
        return -1;
    }

    // len pointer may not be NULL
    if (len == NULL) {
        errno = EINVAL;
        return -1;  // Can't set *len if len is NULL
    }

    // Get the stat buffer of fd
    if (fstat(fd, &fd_stat_buf) < 0) {
        *len = sbytes;
        return -1;
    }
    file_size = fd_stat_buf.st_size;

    // Must be a regular file
    if (!S_ISREG(fd_stat_buf.st_mode)) {
        errno = ENOTSUP;
        *len = sbytes;
        return -1;
    }

    if (fstat(s, &s_stat_buf) < 0) {
        *len = sbytes;
        return -1;
    }

    // s must be a socket fd
    if (!S_ISSOCK(s_stat_buf.st_mode)) {
        errno = ENOTSOCK;
        *len = sbytes;
        return -1;
    }

    // Check socket type - must be SOCK_STREAM
    if (getsockopt(s, SOL_SOCKET, SO_TYPE, &socket_type, &optlen) == -1) {
        *len = sbytes;
        return -1;
    }
    if (socket_type != SOCK_STREAM) {
        errno = EINVAL;
        *len = sbytes;
        return -1;
    }

    // Check if socket has connection errors
    if (getsockopt(s, SOL_SOCKET, SO_ERROR, &socket_type, &optlen) == -1) {
        *len = sbytes;
        return -1;
    }
    if (socket_type != 0) {
        errno = ENOTCONN;
        *len = sbytes;
        return -1;
    }

    // Check if offset is beyond the end of file
    if (offset >= file_size) {
        *len = 0;
        return 0;
    }

    // Save original file position and seek to offset
    orig_offset = lseek(fd, 0, SEEK_CUR);
    if (orig_offset == -1) {
        *len = sbytes;
        return -1;
    }
    if (lseek(fd, offset, SEEK_SET) == -1) {
        *len = sbytes;
        return -1;
    }

    // Allocate buffer for data transfer
    buffer = (char *)malloc(SENDFILE_BUFFER_SIZE);
    if (!buffer) {
        errno = ENOMEM;
        ret = -1;
        *len = sbytes;
        goto cleanup;
    }

    // Phase 1: Send headers if specified
    if (hdtr != NULL && hdtr->headers != NULL && hdtr->hdr_cnt > 0) {
        writev_retval = writev(s, hdtr->headers, hdtr->hdr_cnt);
        if (writev_retval < 0) {
            ret = -1;
            *len = sbytes;
            goto cleanup;
        }
        sbytes += writev_retval;
        sent_headers_and_file += writev_retval;

        // If we've already sent enough (headers >= nbytes), skip file data
        if (nbytes > 0 && sent_headers_and_file >= nbytes) {
            goto send_trailers;
        }
    }

    // Phase 2: Send file data
    off_t file_pos = offset;
    while ((nbytes == 0 || sent_headers_and_file < nbytes) && file_pos < file_size) {
        size_t to_read;
        ssize_t read_bytes;

        // Calculate how much to read
        if (nbytes == 0) {
            // Send until EOF
            to_read = SENDFILE_BUFFER_SIZE;
        } else {
            // Limited by remaining bytes for headers + file data
            off_t remaining = nbytes - sent_headers_and_file;
            to_read = (size_t)remaining;
            if (to_read > SENDFILE_BUFFER_SIZE) {
                to_read = SENDFILE_BUFFER_SIZE;
            }
        }

        // Don't read beyond EOF
        off_t remaining_in_file = file_size - file_pos;
        if ((off_t)to_read > remaining_in_file) {
            to_read = remaining_in_file;
        }

        if (to_read == 0) {
            break;  // Nothing more to send
        }

        // Read data from file
        read_bytes = read(fd, buffer, to_read);
        if (read_bytes < 0) {
            if (errno == EINTR) {
                continue;  // Retry on interrupt
            }
            ret = -1;
            goto cleanup;
        } else if (read_bytes == 0) {
            break;  // EOF
        }

        // Write data to socket
        size_t total_sent = 0;
        while (total_sent < (size_t)read_bytes) {
            ssize_t sent = write(s, buffer + total_sent, read_bytes - total_sent);
            if (sent < 0) {
                if (errno == EINTR) {
                    continue;  // Retry on interrupt
                } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // For non-blocking sockets, return partial success
                    sbytes += total_sent;
                    sent_headers_and_file += total_sent;
                    *len = sbytes;
                    goto cleanup;
                } else {
                    ret = -1;
                    goto cleanup;
                }
            }
            total_sent += sent;
        }

        sbytes += total_sent;
        sent_headers_and_file += total_sent;
        file_pos += total_sent;
    }

send_trailers:
    // Phase 3: Send trailers if specified (always completely)
    if (hdtr != NULL && hdtr->trailers != NULL && hdtr->trl_cnt > 0) {
        writev_retval = writev(s, hdtr->trailers, hdtr->trl_cnt);
        if (writev_retval < 0) {
            ret = -1;
            *len = sbytes;
            goto cleanup;
        }
        sbytes += writev_retval;
    }

    // Return total bytes sent
    *len = sbytes;
    return ret;

cleanup:
    *len = sbytes;  // Always set len to bytes sent (0 on error)

    if (buffer) {
        free(buffer);
    }

    // Restore original file position if we changed it
    if (orig_offset != -1) {
        lseek(fd, orig_offset, SEEK_SET);
    }

    return ret;
}

/* Still try to override this symbol, since iOS just aborting when using undef syscalls
   instead of setting ENOSYS */
int sendfile(int fd, int s, off_t offset, off_t *len, struct sf_hdtr *hdtr, int flags)
{
    return compat_sendfile(fd, s, offset, len, hdtr, flags);
}
