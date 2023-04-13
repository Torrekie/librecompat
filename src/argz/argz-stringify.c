#include <string.h>

void
__argz_stringify (char *argz, size_t len, int sep)
{
	if (len > 0)
		while (1) {
			size_t part_len = strnlen (argz, len);
			argz += part_len;
			len -= part_len;
			if (len-- <= 1)
				break;
			*argz++ = sep;
		}
	return;
}

void
argz_stringify (char *argz, size_t len, int sep)
{
	return __argz_stringify(argz, len, sep);
}
