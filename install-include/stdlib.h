#ifndef _LIBRECOMPAT_STDLIB_H_
#define _LIBRECOMPAT_STDLIB_H_

#include_next <stdlib.h>

#include <mach/boolean.h>
#include <sys/cdefs.h>
#include <Availability.h>
#include <os/availability.h>
#include <malloc/malloc.h>

API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0))
void * reallocarray(void * in_ptr, size_t nmemb, size_t size) __DARWIN_EXTSN(reallocarray) __result_use_check;

API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0))
void * reallocarrayf(void * in_ptr, size_t nmemb, size_t size) __DARWIN_EXTSN(reallocarrayf) __result_use_check;

#endif
