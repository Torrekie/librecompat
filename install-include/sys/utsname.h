#ifndef _LIBRECOMPAT_SYS_UTSNAME_H_
#define _LIBRECOMPAT_SYS_UTSNAME_H_

#ifndef uname
#define uname compat_uname
#endif

#include_next <sys/utsname.h>

__BEGIN_DECLS

int compat_uname(struct utsname *);

__END_DECLS

#endif
