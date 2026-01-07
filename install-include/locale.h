#ifndef _LIBRECOMPAT_LOCALE_H_
#define _LIBRECOMPAT_LOCALE_H_

#if defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__) && (__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__ >= 150000)

#include_next <locale.h>

__BEGIN_DECLS

char *setlocale(int, const char *) __asm("_compat_setlocale");
char *compat_setlocale(int, const char *);

__END_DECLS

#else

#include_next <locale.h>

#endif

#endif
