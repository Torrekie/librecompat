#include <errno.h>
#include <stdlib.h>
#include <string.h>


errno_t
__argz_create_sep (const char *string, int delim, char **argz, size_t *len)
{
	size_t nlen = strlen (string) + 1;

	if (nlen > 1) {
		const char *rp;
		char *wp;

		*argz = (char *) malloc (nlen);
		if (*argz == NULL)
			return ENOMEM;

		rp = string;
		wp = *argz;
		do
			if (*rp == delim) {
				if (wp > *argz && wp[-1] != '\0')
					*wp++ = '\0';
				else
					--nlen;
			}
			else
				*wp++ = *rp;
		while (*rp++ != '\0');

		if (nlen == 0) {
			free (*argz);
			*argz = NULL;
			*len = 0;
		}

		*len = nlen;
	} else {
		*argz = NULL;
		*len = 0;
	}

	return 0;
}

errno_t
argz_create_sep (const char *string, int delim, char **argz, size_t *len)
{
	return __argz_create_sep(string, delim, argz, len);
}
