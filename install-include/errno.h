#ifndef _LIBRECOMPAT_ERRNO_H_
#define _LIBRECOMPAT_ERRNO_H_

#include_next <errno.h>

#define __set_errno(val) (errno = (val))

extern char *program_invocation_name; /* __progname_full */
extern char *program_invocation_short_name; /* __progname */

#endif
