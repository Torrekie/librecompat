#ifdef LIBRECOMPAT_ROOTLESS
#include <locale.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

/* Hacky: Libc _PathLocale variable */
extern char *_PathLocale;

#ifdef _PATH_LOCALE
#undef _PATH_LOCALE
#endif
#define _PATH_LOCALE "/var/jb/usr/share/locale"

char *
compat_setlocale(int category, const char *locale)
{
	char *result;

	/* Hacky: try with jailbreak locale path */
	if (_PathLocale == NULL)
		_PathLocale = _PATH_LOCALE;

	result = setlocale(category, locale);
	if (result != NULL)
		return result;

	/* failed, try with default */
	if (_PathLocale == _PATH_LOCALE || strcmp(_PathLocale, _PATH_LOCALE) == 0) {
		/* Reset _PathLocale to allow fallback to default */
		_PathLocale = NULL;

		result = setlocale(category, locale);
	}

	return result;
}

#endif


