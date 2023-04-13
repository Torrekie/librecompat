#include <errno.h>
#include <stdlib.h>
#include <string.h>

errno_t
__argz_add_sep (char **argz, size_t *argz_len, const char *string, int delim)
{
	size_t nlen = strlen (string) + 1;

	if (nlen > 1) {
		const char *rp;
		char *wp;

		*argz = (char *) realloc (*argz, *argz_len + nlen);
		if (*argz == NULL)
			return ENOMEM;

		wp = *argz + *argz_len;
		rp = string;
		do
			if (*rp == delim) {
				if (wp > *argz && wp[-1] != '\0')
					*wp++ = '\0';
				else
					--nlen;
			} else
				*wp++ = *rp;
		while (*rp++ != '\0');

		*argz_len += nlen;
	}

	return 0;
}

errno_t
argz_add_sep (char **argz, size_t *argz_len, const char *string, int delim)
{
	return __argz_add_sep(argz, argz_len, string, delim);
}
