#ifndef _LIBRECOMPAT_SYSCTL_H_
#define _LIBRECOMPAT_SYSCTL_H_

#if __has_include(<librecompat/librecompat_config.h>)
#include <librecompat/librecompat_config.h>
#elif __has_include("librecompat_config.h")
#include "librecompat_config.h"
#else
#include <librecompat_config.h>
#endif

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
