#include <string.h>

size_t
__argz_count (const char *argz, size_t len)
{
	size_t count = 0;
	while (len > 0) {
		size_t part_len = strlen(argz);
		argz += part_len + 1;
		len -= part_len + 1;
		count++;
	}
	return count;
}

size_t
argz_count (const char *argz, size_t len)
{
	return __argz_count(argz, len);
}
