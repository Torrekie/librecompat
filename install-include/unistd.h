#ifndef _LIBRECOMPAT_UNISTD_H_
#define _LIBRECOMPAT_UNISTD_H_

#ifndef getopt
#define getopt compat_getopt
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

__END_DECLS

#endif /* _LIBRECOMPAT_UNISTD_H_ */
