#ifndef _LIBRECOMPAT_SYS_CDEFS_H_

#include_next <sys/cdefs.h>

#ifndef __cplusplus
# define _Static_assert(expr, diagnostic) _Static_assert (expr, diagnostic)
#endif

#include <misc/sys/cdefs.h>

#endif
