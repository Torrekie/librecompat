#ifndef _LIBRECOMPAT_SYS_SOCKET_H_
#define _LIBRECOMPAT_SYS_SOCKET_H_

#include_next <sys/socket.h>

#define SOCK_CLOEXEC	0x10000000
#define SOCK_NONBLOCK	0x20000000

__BEGIN_DECLS

#if __GNUC__ > 4
int accept4(int s, struct sockaddr *__restrict addr, socklen_t *__restrict addrlen, int flags);
#else
int accept4(int s, struct sockaddr *restrict addr, socklen_t *restrict addrlen, int flags);
#endif

__END_DECLS

#endif
