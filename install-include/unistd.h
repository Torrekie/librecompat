#ifndef _LIBRECOMPAT_UNISTD_H_
#define _LIBRECOMPAT_UNISTD_H_

#include <librecompat/librecompat_config.h>

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

#if LIBRECOMPAT_ROOTLESS
#ifndef confstr
#define confstr compat_confstr
#endif

#ifndef getusershell
#define getusershell compat_getusershell
#endif

#ifndef setusershell
#define setusershell compat_setusershell
#endif

#ifndef endusershell
#define endusershell compat_endusershell
#endif

#endif

#include_next <unistd.h>

#ifdef  __USE_POSIX2
/* Get definitions and prototypes for functions to process the
   arguments in ARGV (ARGC of them, minus the program name) for
   options given in OPTS.  */
# include <bits/getopt_posix.h>
#endif

__BEGIN_DECLS

int compat_getopt(int, char * const [], const char *);

char *get_current_dir_name(void);

int fdatasync(int fd);

#if LIBRECOMPAT_ROOTLESS
size_t compat_confstr(int, char *, size_t);
char *compat_getusershell(void);
void compat_setusershell(void);
void compat_endusershell(void);
#endif


__END_DECLS

#endif /* _LIBRECOMPAT_UNISTD_H_ */
