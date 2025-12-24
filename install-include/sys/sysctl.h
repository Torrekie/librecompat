#ifndef _LIBRECOMPAT_SYSCTL_H_
#define _LIBRECOMPAT_SYSCTL_H_

#include <librecompat/librecompat_config.h>

#if LIBRECOMPAT_ROOTLESS
#ifndef sysctl
#define sysctl compat_sysctl
#endif
#endif

#include_next <sys/sysctl.h>

__BEGIN_DECLS

#if LIBRECOMPAT_ROOTLESS
int compat_sysctl(int *, u_int, void *, size_t *, void *, size_t);
#endif

__END_DECLS

#endif
