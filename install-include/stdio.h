#ifndef _LIBRECOMPAT_STDIO_H_
#define _LIBRECOMPAT_STDIO_H_

#if __has_include(<librecompat/librecompat_config.h>)
#include <librecompat/librecompat_config.h>
#elif __has_include("librecompat_config.h")
#include "librecompat_config.h"
#else
#include <librecompat_config.h>
#endif

#ifdef LIBRECOMPAT_ROOTLESS
#ifndef popen
#define popen compat_popen
#endif
#endif

#include_next <stdio.h>

__BEGIN_DECLS

typedef ssize_t cookie_read_function_t(void *, char *, size_t);
typedef ssize_t cookie_write_function_t(void *, const char *, size_t);
typedef int cookie_seek_function_t(void *, int64_t *, int);
typedef int cookie_close_function_t(void *);
typedef struct {
	cookie_read_function_t	*read;
	cookie_write_function_t	*write;
	cookie_seek_function_t	*seek;
	cookie_close_function_t	*close;
} cookie_io_functions_t;
int	fdclose(FILE *fp, int *fdp);
FILE	*fopencookie(void *, const char *, cookie_io_functions_t);

#ifdef LIBRECOMPAT_ROOTLESS
FILE *compat_popen(const char *, const char *);
#endif

__END_DECLS

#endif
