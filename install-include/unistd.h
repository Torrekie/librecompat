#ifndef _LIBRECOMPAT_UNISTD_H_
#define _LIBRECOMPAT_UNISTD_H_

#include_next <unistd.h>

char *get_current_dir_name(void);

int fdatasync(int fd);

#endif /* _LIBRECOMPAT_UNISTD_H_ */
