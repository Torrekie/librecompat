#ifndef _LIBRECOMPAT_STRING_H_
#define _LIBRECOMPAT_STRING_H_

#include_next <string.h>

#ifdef __cplusplus
extern "C" {
#endif

char *strchrnul(const char *s, int c);

#ifdef __cplusplus
}
#endif

#endif
