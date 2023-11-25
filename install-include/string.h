#ifndef _LIBRECOMPAT_STRING_H_
#define _LIBRECOMPAT_STRING_H_

#include_next <string.h>

#ifdef __cplusplus
extern "C" {
#endif

char *strchrnul(const char *s, int c);

/* Torrekie: it seems compiler automatically optimizes mempcpy if provided declaration,
   but we are still providing implementation for mempcpy anyway. */
void *mempcpy(void *__restrict dst, const void *__restrict src, size_t len);
void *memrchr(const void *s, int c, size_t n);

#ifdef __cplusplus
}
#endif

#endif
