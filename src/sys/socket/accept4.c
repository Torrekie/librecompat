#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>

#define SOCK_CLOEXEC	0x10000000
#define SOCK_NONBLOCK	0x20000000

// Torrekie: Terrible impl, I wonder if we can do something that kernel does
int
accept4(int s, struct sockaddr *restrict addr, socklen_t *restrict addrlen, int flags)
{
	// Only need to check these two flags in userspace
	if (flags & ~(SOCK_CLOEXEC | SOCK_NONBLOCK)) {
		errno = EINVAL;
		return -1;
	}
	// Call accept first, then we operate on fds (uhh sucks)
	int fd = accept(s, addr, addrlen);
	int newflags = 0;

	if (fd < 0)
		return fd;

	if (flags & SOCK_NONBLOCK) {
		newflags |= O_NONBLOCK;
		flags &= ~SOCK_NONBLOCK;
	}
	if (flags & SOCK_CLOEXEC) {
		newflags |= O_CLOEXEC;
		flags &= ~SOCK_CLOEXEC;
	}
	/* Apple does not like operating close-on-exec stuff in
	 * their system functions that related with fd, but we
	 * still need to do this for matching behaviors */
	if (fcntl(fd, F_SETFL, newflags) < 0) {
		int saved_errno = errno;
		close(fd);
		errno = saved_errno;
		return -1;
	}

	return fd;
}
