#ifndef _LIBRECOMPAT_CONFIG_H_
#define _LIBRECOMPAT_CONFIG_H_

#include <os/base.h>

#define LIBRECOMPAT_ROOTLESS 1

/* Determine if rootless by target version */
#ifndef LIBRECOMPAT_ROOTLESS
#define LIBRECOMPAT_ROOTLESS (__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__ && __ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__ >= 150000)
#endif

/* Disable glibc getopt() by default, it sometimes could be problematic */
#ifndef LIBRECOMPAT_GETOPT
#define LIBRECOMPAT_GETOPT 0
#endif

/* Disable glibc regex by default */
#ifndef LIBRECOMPAT_REGEX
#define LIBRECOMPAT_REGEX 0
#endif

#endif
