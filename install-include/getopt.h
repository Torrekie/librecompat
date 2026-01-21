#ifndef _LIBRECOMPAT_GETOPT_H_
#define _LIBRECOMPAT_GETOPT_H_

#if __has_include(<librecompat/librecompat_config.h>)
#include <librecompat/librecompat_config.h>
#elif __has_include("librecompat_config.h")
#include "librecompat_config.h"
#else
#include <librecompat_config.h>
#endif

#if defined(LIBRECOMPAT_GETOPT) && LIBRECOMPAT_GETOPT != 0
#include <posix/getopt.h>
#else
/* Fallback to system getopt impl */
#include_next <getopt.h>
#endif

#endif
