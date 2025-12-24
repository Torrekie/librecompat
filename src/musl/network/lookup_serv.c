#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <paths.h>
#include "lookup.h"

#ifdef LIBRECOMPAT_ROOTLESS
#ifdef _PATH_SERVICES
#undef _PATH_SERVICES
#endif
#define _PATH_SERVICES "/var/jb/etc/services"
#endif

int __lookup_serv(struct service buf[static MAXSERVS], const char *name, int proto, int flags)
{
	char line[128];
	int cnt = 0;
	char *p, *z = "";
	unsigned long port = 0;

	if (name) {
		if (!*name) return EAI_SERVICE;
		port = strtoul(name, &z, 10);
	}
	if (!*z) {
		if (port > 65535) return EAI_SERVICE;
		if (proto != IPPROTO_UDP) {
			buf[cnt].port = port;
			buf[cnt++].proto = IPPROTO_TCP;
		}
		if (proto != IPPROTO_TCP) {
			buf[cnt].port = port;
			buf[cnt++].proto = IPPROTO_UDP;
		}
		return cnt;
	}

	if (flags & AI_NUMERICSERV) return EAI_SERVICE;

	size_t l = strlen(name);

	unsigned char _buf[1032];
	FILE _f, *f = fopen(_PATH_SERVICES, "rb");
	if (!f) return EAI_SERVICE;

	while (fgets(line, sizeof line, f) && cnt < MAXSERVS) {
		if ((p=strchr(line, '#'))) *p++='\n', *p=0;

		/* Find service name */
		for(p=line; (p=strstr(p, name)); p++) {
			if (p>line && !isspace(p[-1])) continue;
			if (p[l] && !isspace(p[l])) continue;
			break;
		}
		if (!p) continue;

		/* Skip past canonical name at beginning of line */
		for (p=line; *p && !isspace(*p); p++);

		port = strtoul(p, &z, 10);
		if (port > 65535 || z==p) continue;
		if (!strncmp(z, "/udp", 4)) {
			if (proto == IPPROTO_TCP) continue;
			buf[cnt].port = port;
			buf[cnt++].proto = IPPROTO_UDP;
		}
		if (!strncmp(z, "/tcp", 4)) {
			if (proto == IPPROTO_UDP) continue;
			buf[cnt].port = port;
			buf[cnt++].proto = IPPROTO_TCP;
		}
	}
	f->_close(f);
	return cnt > 0 ? cnt : EAI_SERVICE;
}

