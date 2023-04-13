#include <errno.h>
#include <string.h>
#include <stdlib.h>

errno_t
__argz_append (char **argz, size_t *argz_len, const char *buf, size_t buf_len)
{
	size_t new_argz_len = *argz_len + buf_len;
	char *new_argz = realloc (*argz, new_argz_len);
	if (new_argz) {
		memcpy (new_argz + *argz_len, buf, buf_len);
		*argz = new_argz;
		*argz_len = new_argz_len;
		return 0;
	} else
		return ENOMEM;
}

errno_t
__argz_add (char **argz, size_t *argz_len, const char *str)
{
	return __argz_append (argz, argz_len, str, strlen (str) + 1);
}

errno_t
argz_append (char **argz, size_t *argz_len, const char *buf, size_t buf_len)
{
	return __argz_append(argz, argz_len, buf, buf_len);
}

errno_t
argz_add (char **argz, size_t *argz_len, const char *str)
{
	return __argz_add(argz, argz_len, str);
}
