#include <sys/param.h>
#include <sys/sysctl.h>

#include <errno.h>
#include <paths.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>	/* for P_tmpdir */

#if __DARWIN_UNIX03
#define CONFSTR_ERR_RET	0
#else /* !__DARWIN_UNIX03 */
#define CONFSTR_ERR_RET	-1
#endif /* __DARWIN_UNIX03 */

extern int compat_sysctl(int *name, u_int namelen, void *oldp, size_t *oldlenp, void *newp,
         size_t newlen);

size_t
compat_confstr(int name, char *buf, size_t len)
{
	size_t tlen;
	int mib[2], sverrno;
	char *p;

	switch (name) {
	case _CS_PATH:
		mib[0] = CTL_USER;
		mib[1] = USER_CS_PATH;
		if (compat_sysctl(mib, 2, NULL, &tlen, NULL, 0) == -1)
			return (CONFSTR_ERR_RET);
		if (len != 0 && buf != NULL) {
			if ((p = malloc(tlen)) == NULL)
				return (CONFSTR_ERR_RET);
			if (compat_sysctl(mib, 2, p, &tlen, NULL, 0) == -1) {
				sverrno = errno;
				free(p);
				errno = sverrno;
				return (CONFSTR_ERR_RET);
			}
			/*
			 * POSIX 1003.2 requires partial return of
			 * the string -- that should be *real* useful.
			 */
			(void)strncpy(buf, p, len - 1);
			buf[len - 1] = '\0';
			free(p);
		}
		return (tlen);
	}
	return confstr(name, buf, len);
}
