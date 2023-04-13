#include <errno.h>
#include <stdlib.h>
#include <string.h>

errno_t
__argz_create (char *const argv[], char **argz, size_t *len)
{
	int argc;
	size_t tlen = 0;
	char *const *ap;
	char *p;

	for (argc = 0; argv[argc] != NULL; ++argc)
		tlen += strlen (argv[argc]) + 1;

	if (tlen == 0)
		*argz = NULL;
	else {
		*argz = malloc (tlen);
		if (*argz == NULL)
			return ENOMEM;

		for (p = *argz, ap = argv; *ap; ++ap, ++p)
			p = stpcpy (p, *ap);
	}
	*len = tlen;

	return 0;
}

errno_t
argz_create (char *const argv[], char **argz, size_t *len)
{
	return __argz_create(argv, argz, len);
}
