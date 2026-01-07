#ifndef _LIBRECOMPAT_ERRNO_H_
#define _LIBRECOMPAT_ERRNO_H_

#include_next <errno.h>

#define __set_errno(val) (errno = (val))

// glibc-compatible program invocation name variables
extern char *program_invocation_name; /* __progname_full */
extern char *program_invocation_short_name; /* __progname */

// These are defined in stdlib/init_misc.c
extern char *__progname_full;
extern char *__progname;

#endif
