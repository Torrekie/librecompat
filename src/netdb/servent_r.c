#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/sysctl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <os/availability.h>

#ifdef LIBRECOMPAT_ROOTLESS
#ifdef _PATH_SERVICES
#undef _PATH_SERVICES
#endif
#define _PATH_SERVICES "/var/jb/etc/services"
#endif

extern void *reallocarray(void * in_ptr, size_t nmemb, size_t size) __DARWIN_EXTSN(reallocarray);

struct servent_data {
	void *fp;
	char **aliases;
	int maxaliases;
	int stayopen;
	char *line;
};

void endservent_r(struct servent_data *sd)
{
	if (sd->fp) {
		fclose(sd->fp);
		sd->fp = NULL;
	}
	free(sd->aliases);
	sd->aliases = NULL;
	sd->maxaliases = 0;
	free(sd->line);
	sd->line = NULL;
	sd->stayopen = 0;
}

int getservent_r(struct servent *se, struct servent_data *sd)
{
	char *p, *cp, **q, *endp;
	size_t len;
	long l;
	int serrno;

	if (sd->fp == NULL && (sd->fp = fopen(_PATH_SERVICES, "re" )) == NULL)
		return (-1);
again:
	if ((p = fgetln(sd->fp, &len)) == NULL)
		return (-1);
	if (len == 0 || *p == '#' || *p == '\n')
		goto again;
	if (p[len-1] == '\n')
		len--;
	if ((cp = memchr(p, '#', len)) != NULL)
		len = cp - p;
	cp = realloc(sd->line, len + 1);
	if (cp == NULL)
		return (-1);
	sd->line = se->s_name = memcpy(cp, p, len);
	cp[len] = '\0';
	p = strpbrk(cp, " \t");
	if (p == NULL)
		goto again;
	*p++ = '\0';
	while (*p == ' ' || *p == '\t')
		p++;
	cp = strpbrk(p, ",/");
	if (cp == NULL)
		goto again;
	*cp++ = '\0';
	l = strtol(p, &endp, 10);
	if (endp == p || *endp != '\0' || l < 0 || l > USHRT_MAX)
		goto again;
	se->s_port = htons((in_port_t)l);
	se->s_proto = cp;
	if (sd->aliases == NULL) {
		sd->maxaliases = 10;
		sd->aliases = calloc(sd->maxaliases, sizeof(char *));
		if (sd->aliases == NULL) {
			serrno = errno;
			endservent_r(sd);
			errno = serrno;
			return (-1);
		}
	}
	q = se->s_aliases = sd->aliases;
	cp = strpbrk(cp, " \t");
	if (cp != NULL)
		*cp++ = '\0';
	while (cp && *cp) {
		if (*cp == ' ' || *cp == '\t') {
			cp++;
			continue;
		}
		if (q == &se->s_aliases[sd->maxaliases - 1]) {
			p = reallocarray(se->s_aliases, sd->maxaliases,
			    2 * sizeof(char *));
			if (p == NULL) {
				serrno = errno;
				endservent_r(sd);
				errno = serrno;
				return (-1);
			}
			sd->maxaliases *= 2;
			q = (char **)p + (q - se->s_aliases);
			se->s_aliases = sd->aliases = (char **)p;
		}
		*q++ = cp;
		cp = strpbrk(cp, " \t");
		if (cp != NULL)
			*cp++ = '\0';
	}
	*q = NULL;
	return (0);
}

void setservent_r(int f, struct servent_data *sd)
{
	if (sd->fp == NULL)
		sd->fp = fopen(_PATH_SERVICES, "re" );
	else
		rewind(sd->fp);
	sd->stayopen |= f;
}
