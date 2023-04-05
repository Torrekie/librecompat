#ifndef _LIBRECOMPAT_FCNTL_H_
#define _LIBRECOMPAT_FCNTL_H_

#include_next <fcntl.h>

// bits/fcntl.h
#define POSIX_FADV_NORMAL       0 /* No further special treatment.  */
#define POSIX_FADV_RANDOM       1 /* Expect random page references.  */
#define POSIX_FADV_SEQUENTIAL   2 /* Expect sequential page references.  */
#define POSIX_FADV_WILLNEED     3 /* Will need these pages.  */
#define POSIX_FADV_DONTNEED     4 /* Don't need these pages.  */
#define POSIX_FADV_NOREUSE      5 /* Data will be accessed once.  */

int posix_fallocate(int fd, off_t offset, off_t len);
int posix_fadvise(int fd, off_t offset, off_t len, int advice);

#endif /* _LIBRECOMPAT_FCNTL_H_ */
