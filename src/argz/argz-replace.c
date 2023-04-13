#include <errno.h>
#include <stdlib.h>
#include <string.h>

extern errno_t __argz_append (char **argz, size_t *argz_len, const char *buf, size_t buf_len);
extern errno_t __argz_add (char **argz, size_t *argz_len, const char *str);
extern char *argz_next (const char *argz, size_t argz_len, const char *entry);
extern void *mempcpy(void *__restrict dst, const void *__restrict src, size_t len);

static void
str_append (char **to, size_t *to_len, const char *buf, const size_t buf_len)
{
	size_t new_len = *to_len + buf_len;
	char *new_to = realloc (*to, new_len + 1);

	if (new_to) {
		*((char *) mempcpy (new_to + *to_len, buf, buf_len)) = '\0';
		*to = new_to;
		*to_len = new_len;
	} else {
		free (*to);
		*to = 0;
	}
}
errno_t
__argz_replace (char **argz, size_t *argz_len, const char *str, const char *with,
		unsigned *replace_count)
{
	errno_t err = 0;

	if (str && *str) {
		char *arg = 0;
		char *src = *argz;
		size_t src_len = *argz_len;
		char *dst = 0;
		size_t dst_len = 0;
		int delayed_copy = 1;
		size_t str_len = strlen (str), with_len = strlen (with);

		while (!err && (arg = argz_next (src, src_len, arg))) {
			char *match = strstr (arg, str);
			if (match) {
				char *from = match + str_len;
				size_t to_len = match - arg;
				char *to = strndup (arg, to_len);

				while (to && from) {
					str_append (&to, &to_len, with, with_len);
					if (to) {
						match = strstr (from, str);
						if (match) {
							str_append (&to, &to_len, from, match - from);
							from = match + str_len;
						} else {
							str_append (&to, &to_len, from, strlen (from));
							from = 0;
						}
					}
				}

				if (to) {
					if (delayed_copy) {
						if (arg > src)
							err = __argz_append (&dst, &dst_len, src, (arg - src));
						delayed_copy = 0;
					}
					if (! err)
						err = __argz_add (&dst, &dst_len, to);
					free (to);
				} else
					err = ENOMEM;

				if (replace_count)
					(*replace_count)++;
			} else if (! delayed_copy)
				err = __argz_add (&dst, &dst_len, arg);
		}

		if (! err) {
			if (! delayed_copy) {
				free (src);
				*argz = dst;
				*argz_len = dst_len;
			}
		} else if (dst_len > 0)
			free (dst);
	}

	return err;
}

errno_t
argz_replace (char **argz, size_t *argz_len, const char *str, const char *with,
		unsigned *replace_count)
{
	return __argz_replace(argz, argz_len, str, with, replace_count);
}
