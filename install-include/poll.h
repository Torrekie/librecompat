#ifndef _LIBRECOMPAT_POLL_H_
#define _LIBRECOMPAT_POLL_H_

#include <sys/_types/_sigset_t.h>
#include_next <poll.h>

__BEGIN_DECLS

int	ppoll(struct pollfd _pfd[], nfds_t _nfds,
	    const struct timespec *__restrict _timeout,
	    const sigset_t *__restrict _newsigmask);

__END_DECLS

#endif
