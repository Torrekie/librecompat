#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

char* get_current_dir_name(void) {
    size_t size = 256; // start with a reasonable buffer size
    char *buf = NULL;
    char *ret = NULL;

    while (1) {
        buf = realloc(buf, size);
        if (buf == NULL) {
            goto error;
        }

        if (getcwd(buf, size) != NULL) {
            ret = buf;
            break;
        }

        if (errno != ERANGE) {
            goto error;
        }

        size *= 2;
    }

    return ret;

error:
    free(buf);
    return NULL;
}
