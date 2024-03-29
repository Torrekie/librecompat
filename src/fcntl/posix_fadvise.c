#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#define	POSIX_FADV_NORMAL	0	/* no special treatment */
#define	POSIX_FADV_RANDOM	1	/* expect random page references */
#define	POSIX_FADV_SEQUENTIAL	2	/* expect sequential page references */
#define	POSIX_FADV_WILLNEED	3	/* will need these pages */
#define	POSIX_FADV_DONTNEED	4	/* dont need these pages */
#define	POSIX_FADV_NOREUSE	5	/* access data only once */


int posix_fadvise(int fd, off_t offset, off_t len, int advice) {
	if (offset < 0 || len <= 0) {
		return EINVAL;
	}

	int cmd;
	switch (advice) {
		case POSIX_FADV_NORMAL:
			cmd = F_RDADVISE;
			break;
		case POSIX_FADV_SEQUENTIAL:
		case POSIX_FADV_RANDOM:
			cmd = F_RDADVISE | F_RDAHEAD;
			break;
		case POSIX_FADV_NOREUSE:
			cmd = F_NOCACHE;
			break;
		case POSIX_FADV_WILLNEED: {
			int ret = posix_fadvise(fd, offset, len, POSIX_FADV_SEQUENTIAL);
			if (ret != 0) {
				return ret;
			}
			void *buf = malloc(len);
			if (buf == NULL) {
				return ENOMEM;
			}
			int nread = pread(fd, buf, len, offset);
			if (nread < 0) {
				free(buf);
				return errno;
			}
			free(buf);
			return 0;
		}
		case POSIX_FADV_DONTNEED: {
			int ret = posix_fadvise(fd, offset, len, POSIX_FADV_NOREUSE);
			if (ret != 0) {
				return ret;
			}
			void *buf = malloc(len);
			if (buf == NULL) {
				return ENOMEM;
			}
			memset(buf, 0, len);
			int nwrite = pwrite(fd, buf, len, offset);
			if (nwrite < 0) {
				free(buf);
				return errno;
			}
			free(buf);
			return 0;
		}
		default:
			return EINVAL;
	}

	struct radvisory ra = {
		.ra_offset = offset,
		.ra_count = len,
	};

	int ret = fcntl(fd, cmd, &ra);
	if (ret == -1) {
		return errno;
	}

	return 0;
}
