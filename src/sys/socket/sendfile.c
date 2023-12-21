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

#if TEST
#define sendfile userspace_sendfile
#endif
int sendfile(int fd, int s, off_t offset, off_t *len, struct sf_hdtr *hdtr, int flags)
{
    int error = 0;
    off_t sbytes = 0;
    off_t nbytes = *len;
    off_t file_size;
    ssize_t writev_retval, sizeof_hdtr;
    off_t xfsize;
    struct stat fd_stat_buf, s_stat_buf;
    struct iovec *iov;
    struct user_sf_hdtr user_hdtr;
    struct user64_sf_hdtr user64_hdtr;
    int i;
    int ret = 0;

    // Apple reserved this flag for potential future use (must be 0)
    if (flags != 0) {
        errno = EINVAL;
        ret = -1;
        goto done;
    }

    if (offset < 0) {
        errno = EINVAL;
        ret = -1;
        goto done;
    }

    // nbytes may not be NULL
    if (nbytes == USER_ADDR_NULL) {
        errno = EINVAL;
        ret = -1;
        goto done;
    }

    // Get the stat buffer of fd
    if (fstat(fd, &fd_stat_buf) < 0) {
        ret = -1;
        goto done;
    }
    file_size = fd_stat_buf.st_size;

    if (fstat(s, &s_stat_buf) < 0) {
        ret = -1;
        goto done;
    }

    // s must be a socket fd
    if (S_ISSOCK(s_stat_buf.st_mode)) {
        int opt;
        socklen_t optlen = sizeof(opt);

        if (getsockopt(s, SOL_SOCKET, SO_TYPE, &opt, &optlen) == -1) {
            ret = -1;
            goto done;
        } else {
            // must be SOCK_STREAM
            if (opt != SOCK_STREAM) {
                errno = EINVAL;
                ret = -1;
                goto done;
            }
        }

        if (getsockopt(s, SOL_SOCKET, SO_ERROR, &opt, &optlen) == -1) {
            ret = -1;
            goto done;
        } else {
            // must be SS_ISCONNECTED, but we cannot check so_state in userspace
            if (opt != 0) {
                errno = ENOTCONN;
                ret = -1;
                goto done;
            }
        }
    } else {
        errno = EBADF;
        ret = -1;
        goto done;
    }

    // Check if offset is beyond the end of file
    if (offset > file_size) {
        *len = 0;
        goto done;
    }

    // If specified, get the pointer to the sf_hdtr struct for any headers/trailers.
    extern int copyin(const void *uaddr, void *kaddr, size_t len);
    if (hdtr != USER_ADDR_NULL) {
        char *hdtrp;

        bzero(&user_hdtr, sizeof(user_hdtr));
        hdtrp = (char *)&user64_hdtr;
        sizeof_hdtr = sizeof(user64_hdtr);

        memcpy(hdtr, hdtrp, sizeof_hdtr);

        user_hdtr.headers = user64_hdtr.headers;
        user_hdtr.hdr_cnt = user64_hdtr.hdr_cnt;
        user_hdtr.trailers = user64_hdtr.trailers;
        user_hdtr.trl_cnt = user64_hdtr.trl_cnt;

        // Send any headers. Wimp out and use writev(2).
        if (user_hdtr.headers != USER_ADDR_NULL) {
            writev_retval = writev$NOCANCEL(s, user_hdtr.headers, user_hdtr.hdr_cnt);
            if (writev_retval < 0) {
                ret = -1;
                goto done;
            }
            sbytes += writev_retval;
        }
    }

/*
    // Save orig offset
    off_t orig_fd_off = lseek(fd, 0, SEEK_CUR);
    if (orig_fd_off == -1 || lseek(fd, offset, SEEK_SET) == -1) {
        ret = -1;
        goto done;
    }
*/
    // Alloc buffer
    int buffer_len = MIN(len, 80 * 1024 * sizeof(char));
    char *buffer = (char *)malloc(buffer_len);
    if (!buffer) {
        ret = -1;
        goto done;
    }

#if 0 && SENDFILE_6
    // Use len
    while (len > 0) {
        ssize_t to_read = MIN(len, buffer_len);
        ssize_t read_bytes;
        while ((read_bytes = read(in_fd, buffer, to_read)) < 0 && errno == EINTR);
        if (read_bytes == -1) {
            free(buffer)
            ret = -1;
            goto done;
        } else if (read_bytes == 0) break;

        ssize_t write_off = 0;
        while (read_bytes > 0) {
            ssize_t sent;
            while ((sent = write(s, buffer + write_off, (size_t)read_bytes)) < 0 && errno == EINTR);
            if (sent == -1) {
                free(buffer);
                ret = -1;
                goto done;
            }
            read_bytes -= sent;
            len -= sent;
            write_off += sent;
            
    }
#else
    // Use sbytes/nbytes
    while (sbytes < nbytes) {
        ssize_t to_send = nbytes - sbytes;
        if (to_send > file_size - offset) {
            to_send = file_size - offset;
        }

        ssize_t read_bytes;
        do {
            read_bytes = read(fd, buffer, to_send);
        } while (read_bytes < 0 && errno == EINTR);

        if (read_bytes < 0) {
            ret = -1;
            goto done;
        } else if (read_bytes == 0) {
            break;
        }

        ssize_t sent = 0;
        while (sent < read_bytes) {
            ssize_t result = write(s, buffer + sent, read_bytes - sent);
            if (result < 0) {
                if (errno == EAGAIN) {
                    *len = sbytes;
                    continue;
                }
                ret = -1;
                goto done;
            }
            sent += result;
        }

        sbytes += sent;
        offset += sent;
    }
#endif

    // Send trailers. Wimp out and use writev(2).
    if (hdtr != USER_ADDR_NULL &&
        user_hdtr.trailers != USER_ADDR_NULL) {
        writev_retval = writev$NOCANCEL(s, user_hdtr.trailers, user_hdtr.trl_cnt);
        if (writev_retval < 0)
            goto done;
        sbytes += writev_retval;
    }

    // nbytes = sbytes
    *len = sbytes;
done:
    // Succ on 0, otherwise -1 with errno
    return ret;
}

#if TEST
#undef sendfile
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>

void test_sendfile() {
    int fds[2];
    char buffer[1024];
    off_t len;
    int fd, s;

    // Create a test file
    fd = open("testfile_sendfile", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    write(fd, "Hello, world!", 13);
    lseek(fd, 0, SEEK_SET);

    // Create a socket pair
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) != -1);

    // Use sendfile to send the file data over the socket
    len = 13;
    assert(sendfile(fd, fds[0], 0, &len, NULL, 0) != -1);
    assert(len == 13);

    // Read the data from the other end of the socket
    assert(read(fds[1], buffer, sizeof(buffer)) == 13);
    assert(strncmp(buffer, "Hello, world!", 13) == 0);

    close(fd);
    close(fds[0]);
    close(fds[1]);
}

void test_userspace_sendfile() {
    int fds[2];
    char buffer[1024];
    off_t len;
    int fd, s;

    // Create a test file
    fd = open("testfile_compat_sendfile", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    write(fd, "Hello, world!", 13);
    lseek(fd, 0, SEEK_SET);

    // Create a socket pair
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) != -1);

    // Use userspace_sendfile to send the file data over the socket
    len = 13;
    assert(userspace_sendfile(fd, fds[0], 0, &len, NULL, 0) != -1);
    assert(len == 13);

    // Read the data from the other end of the socket
    assert(read(fds[1], buffer, sizeof(buffer)) == 13);
    assert(strncmp(buffer, "Hello, world!", 13) == 0);

    close(fd);
    close(fds[0]);
    close(fds[1]);
}

int main() {
    test_sendfile();
    test_userspace_sendfile();

    return 0;
}
#endif
