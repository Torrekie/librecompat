/*
 * Copyright (c) 2005, 2008 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#include "xlocale_private.h"
#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <dlfcn.h>
#include "collate.h"
#include "ldpart.h"
#include "lmonetary.h"
#include "lnumeric.h"
#include "mblocal.h"

extern struct xlocale_collate compat___xlocale_C_collate;

#ifndef nitems
#define	nitems(x)	(sizeof((x)) / sizeof((x)[0]))
#endif

#define NMBSTATET	10
#define C_LOCALE_INITIALIZER	{	\
	{ 1, NULL },			\
	{}, {}, {}, {}, {},		\
	{}, {}, {}, {}, {},		\
	OS_UNFAIR_LOCK_INIT,		\
	XMAGIC,				\
	0, 0, 0, 0, 1, 1, 0,	\
	{				\
		[XLC_COLLATE] = (void *)&compat___xlocale_C_collate,	\
		[XLC_CTYPE] = (void *)&_DefaultRuneXLocale,	\
	},				\
}

#define	LDPART_BUFFER(x)	\
    (((struct xlocale_ldpart *)(x))->buffer)

static char C[] = "C";
static struct _xlocale __c_locale = C_LOCALE_INITIALIZER;
extern const locale_t _c_locale;
//const locale_t _c_locale = (const locale_t)&__c_locale;
struct _xlocale __global_locale = C_LOCALE_INITIALIZER;
pthread_key_t __locale_key = (pthread_key_t)-1;

extern int __collate_load_tables(const char *, locale_t);
extern int __detect_path_locale(void);
extern const char *__get_locale_env(int);
extern int __messages_load_locale(const char *, locale_t);
extern int __monetary_load_locale(const char *, locale_t);
extern int __numeric_load_locale(const char *, locale_t);
extern int __setrunelocale(const char *, locale_t);
extern int __time_load_locale(const char *, locale_t);

static void destruct_locale(void *v);

/*
 * check that the encoding is the right size, isn't . or .. and doesn't
 * contain any slashes
 */
static inline __attribute__((always_inline)) int
_checkencoding(const char *encoding)
{
	return (encoding && (strlen(encoding) > ENCODING_LEN
	  || (encoding[0] == '.' && (encoding[1] == 0
	  || (encoding[1] == '.' && encoding[2] == 0)))
	  || strchr(encoding, '/') != NULL)) ? -1 : 0;
}

/*
 * copy a locale_t except anything before the magic value
 */
static inline __attribute__((always_inline)) void
_copylocale(locale_t dst, const locale_t src)
{
	memcpy(&dst->__magic, &src->__magic, sizeof(*dst) - offsetof(struct _xlocale, __magic));
}

/*
 * Modify a locale_t, setting the parts specified in the mask
 * to the locale specified by the string.  If the string is NULL, the C
 * locale is used.  If the string is empty, the value is determined from
 * the environment.  -1 is returned on error, and loc is in a partial state.
 */
static int
_modifylocale(locale_t loc, int mask, __const char *locale)
{
	int m, ret;
	const char *enc = NULL;
	char *oenc;

	if (!locale)
		locale = C;

	ret = __detect_path_locale();
	if (ret) {
		errno = ret;
		return -1;
	}

	if (*locale)
		enc = locale;
	for(m = 1; m <= _LC_LAST_MASK; m <<= 1) {
		if (m & mask) {
			switch(m) {
			case LC_COLLATE_MASK:
				if (!*locale) {
					enc = __get_locale_env(LC_COLLATE);
					if (_checkencoding(enc) < 0) {
						errno = EINVAL;
						return -1;
					}
				}
				oenc = (XLOCALE_COLLATE(loc)->__collate_load_error ? C :
				    loc->components[XLC_COLLATE]->locale);
				if (strcmp(enc, oenc) != 0 && __collate_load_tables(enc, loc) == _LDP_ERROR)
					return -1;
				xlocale_fill_name(loc->components[XLC_COLLATE],
				    enc);
				break;
			case LC_CTYPE_MASK:
				if (!*locale) {
					enc = __get_locale_env(LC_CTYPE);
					if (_checkencoding(enc) < 0) {
						errno = EINVAL;
						return -1;
					}
				}
				if (strcmp(enc, loc->components[XLC_CTYPE]->locale) != 0) {
					if ((ret = __setrunelocale(enc, loc)) != 0) {
						errno = ret;
						return -1;
					}
					xlocale_fill_name(loc->components[XLC_CTYPE],
					    enc);
					if (loc->__numeric_fp_cvt == LC_NUMERIC_FP_SAME_LOCALE)
						loc->__numeric_fp_cvt = LC_NUMERIC_FP_UNINITIALIZED;
				}
				break;
			case LC_MESSAGES_MASK:
				if (!*locale) {
					enc = __get_locale_env(LC_MESSAGES);
					if (_checkencoding(enc) < 0) {
						errno = EINVAL;
						return -1;
					}
				}
				oenc = (loc->_messages_using_locale ?
				    LDPART_BUFFER(XLOCALE_MESSAGES(loc)) : C);
				if (strcmp(enc, oenc) != 0 && __messages_load_locale(enc, loc) == _LDP_ERROR)
					return -1;
				xlocale_fill_name(loc->components[XLC_MESSAGES],
				    enc);
				break;
			case LC_MONETARY_MASK:
				if (!*locale) {
					enc = __get_locale_env(LC_MONETARY);
					if (_checkencoding(enc) < 0) {
						errno = EINVAL;
						return -1;
					}
				}
				oenc = (loc->_monetary_using_locale ?
				    LDPART_BUFFER(XLOCALE_MONETARY(loc)) : C);
				if (strcmp(enc, oenc) != 0 && __monetary_load_locale(enc, loc) == _LDP_ERROR)
					return -1;
				xlocale_fill_name(loc->components[XLC_MONETARY],
				    enc);
				break;
			case LC_NUMERIC_MASK:
				if (!*locale) {
					enc = __get_locale_env(LC_NUMERIC);
					if (_checkencoding(enc) < 0) {
						errno = EINVAL;
						return -1;
					}
				}
				oenc = (loc->_numeric_using_locale ?
				    LDPART_BUFFER(XLOCALE_NUMERIC(loc)) : C);
				if (strcmp(enc, oenc) != 0) {
					if (__numeric_load_locale(enc, loc) == _LDP_ERROR)
						return -1;
					xlocale_fill_name(loc->components[XLC_NUMERIC],
					    enc);
					loc->__numeric_fp_cvt = LC_NUMERIC_FP_UNINITIALIZED;
					xlocale_release(loc->__lc_numeric_loc);
					loc->__lc_numeric_loc = NULL;
				}
				break;
			case LC_TIME_MASK:
				if (!*locale) {
					enc = __get_locale_env(LC_TIME);
					if (_checkencoding(enc) < 0) {
						errno = EINVAL;
						return -1;
					}
				}
				oenc = (loc->_time_using_locale ?
				    LDPART_BUFFER(XLOCALE_TIME(loc)) : C);
				if (strcmp(enc, oenc) != 0 && __time_load_locale(enc, loc) == _LDP_ERROR)
					return -1;
				xlocale_fill_name(loc->components[XLC_TIME],
				    enc);
				break;
			}
		}
	}
	return 0;
}

/*
 * release all the memory objects (the memory will be freed when the refcount
 * becomes zero)
 */
static void
destruct_locale(void *v)
{
	locale_t loc = v;

	for (int i = XLC_COLLATE; i < XLC_LAST; i++)
		xlocale_release(loc->components[i]);
	xlocale_release(loc->__lc_numeric_loc);
	free(loc);
}

/*
 * PRIVATE EXTERNAL: Returns the locale that can be used by wcstod and
 * family, to convert the wide character string to a multi-byte string
 * (the LC_NUMERIC and LC_CTYPE locales may be different).
 */
__private_extern__ locale_t
__numeric_ctype(locale_t loc)
{
	switch(loc->__numeric_fp_cvt) {
	case LC_NUMERIC_FP_UNINITIALIZED: {
		const char *ctype = loc->components[XLC_CTYPE]->locale;
		const char *numeric = (loc->_numeric_using_locale ?
		    LDPART_BUFFER(XLOCALE_NUMERIC(loc)) : C);
		if (strcmp(ctype, numeric) == 0) {
			loc->__numeric_fp_cvt = LC_NUMERIC_FP_SAME_LOCALE;
			return loc;
		} else {
			loc->__lc_numeric_loc = newlocale(LC_CTYPE_MASK, numeric, (locale_t)&__c_locale);
			if (loc->__lc_numeric_loc) {
				loc->__numeric_fp_cvt = LC_NUMERIC_FP_USE_LOCALE;
				return loc->__lc_numeric_loc;
			} else { /* shouldn't happen, but just use the same locale */
				loc->__numeric_fp_cvt = LC_NUMERIC_FP_SAME_LOCALE;
				return loc;
			}
		}
	}
	case LC_NUMERIC_FP_SAME_LOCALE:
		return loc;
	case LC_NUMERIC_FP_USE_LOCALE:
		return loc->__lc_numeric_loc;
	}
	return loc;	/* shouldn't happen */
}

static void
__xlocale_release(void *loc)
{
	locale_t l = loc;
	xlocale_release(l);
}

/*
 * Called from the Libc initializer to setup the thread-specific key.
 */
/* Torrekie: This is basically how it was defined */
#ifndef __LIBC_PTHREAD_KEY_XLOCALE
#define __LIBC_PTHREAD_KEY_XLOCALE 10
#endif
__private_extern__ void
__xlocale_init(void)
{
	if (__locale_key == (pthread_key_t)-1) {
		__locale_key = __LIBC_PTHREAD_KEY_XLOCALE;
		pthread_key_init_np(__locale_key, __xlocale_release);
		void *real_collate = dlsym(RTLD_DEFAULT, "__xlocale_C_collate");
		if (real_collate) {
			__c_locale.components[XLC_COLLATE] = real_collate;
			__global_locale.components[XLC_COLLATE] = real_collate;
		}
	}
}

