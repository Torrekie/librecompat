#ifndef _LIBRECOMPAT_UNISTD_H_
#define _LIBRECOMPAT_UNISTD_H_

#if __has_include(<librecompat/librecompat_config.h>)
#include <librecompat/librecompat_config.h>
#elif __has_include("librecompat_config.h")
#include "librecompat_config.h"
#else
#include <librecompat_config.h>
#endif

#if defined(LIBRECOMPAT_GETOPT) && LIBRECOMPAT_GETOPT != 0
/* Wrapper defines, for use of librecompat */
#ifndef getopt
#define getopt compat_getopt
#endif

#ifndef getopt_long
#define getopt_long compat_getopt_long
#endif

#ifndef getopt_long_only
#define getopt_long_only compat_getopt_long_only
#endif

#ifdef  __USE_POSIX2
/* Get definitions and prototypes for functions to process the
   arguments in ARGV (ARGC of them, minus the program name) for
   options given in OPTS.  */
# include <bits/getopt_posix.h>
#endif
#endif

#if defined(LIBRECOMPAT_ROOTLESS) && LIBRECOMPAT_ROOTLESS != 0
#ifndef confstr
#define confstr compat_confstr
#endif

char *getusershell(void) __asm("_compat_getusershell");
void setusershell(void) __asm("_compat_setusershell");
void endusershell(void) __asm("_compat_endusershell");
#endif

#include_next <unistd.h>

__BEGIN_DECLS

#if defined(LIBRECOMPAT_GETOPT) && LIBRECOMPAT_GETOPT != 0
int compat_getopt(int, char * const [], const char *);
#endif

char *get_current_dir_name(void);

int fdatasync(int fd);

#if defined(LIBRECOMPAT_ROOTLESS) && LIBRECOMPAT_ROOTLESS != 0
size_t compat_confstr(int, char *, size_t);
char *compat_getusershell(void);
void compat_setusershell(void);
void compat_endusershell(void);
#endif


__END_DECLS

#endif /* _LIBRECOMPAT_UNISTD_H_ */
