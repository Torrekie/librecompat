#include <errno.h>
#include <string.h>
#include <stdlib.h>

extern errno_t __argz_add (char **argz, size_t *argz_len, const char *str);

errno_t
__argz_insert (char **argz, size_t *argz_len, char *before, const char *entry)
{
	if (! before)
		return __argz_add (argz, argz_len, entry);

	if (before < *argz || before >= *argz + *argz_len)
		return EINVAL;

	if (before > *argz)
		while (before[-1])
			before--;

	{
		size_t after_before = *argz_len - (before - *argz);
		size_t entry_len = strlen	(entry) + 1;
		size_t new_argz_len = *argz_len + entry_len;
		char *new_argz = realloc (*argz, new_argz_len);

		if (new_argz) {
			before = new_argz + (before - *argz);
			memmove (before + entry_len, before, after_before);
			memmove (before, entry, entry_len);
			*argz = new_argz;
			*argz_len = new_argz_len;
			return 0;
		} else
			return ENOMEM;
	}
}

errno_t
argz_insert (char **argz, size_t *argz_len, char *before, const char *entry)
{
	return __argz_insert(argz, argz_len, before, entry);
}
