#include <string.h>

char *
__argz_next (const char *argz, size_t argz_len, const char *entry)
{
	if (entry) {
		if (entry < argz + argz_len)
			entry = strchr (entry, '\0') + 1;

		return entry >= argz + argz_len ? NULL : (char *) entry;
	}
	else if (argz_len > 0)
		return (char *) argz;

	return NULL;
}

char *
argz_next (const char *argz, size_t argz_len, const char *entry)
{
	return __argz_next(argz, argz_len, entry);
}
