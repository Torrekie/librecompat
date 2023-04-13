#include <string.h>

void
__argz_extract (const char *argz, size_t len, char **argv)
{
	while (len > 0) {
		size_t part_len = strlen (argz);
		*argv++ = (char *) argz;
		argz += part_len + 1;
		len -= part_len + 1;
	}
	*argv = 0;
}

void
argz_extract (const char *argz, size_t len, char **argv)
{
	return __argz_extract(argz, len, argv);
}
