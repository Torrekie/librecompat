#include <string.h>
#include <stdlib.h>

void
argz_delete (char **argz, size_t *argz_len, char *entry)
{
	if (entry) {
		size_t entry_len = strlen (entry) + 1;
		*argz_len -= entry_len;
		memmove (entry, entry + entry_len, *argz_len - (entry - *argz));
		if (*argz_len == 0) {
			free (*argz);
			*argz = 0;
		}
	}
}
