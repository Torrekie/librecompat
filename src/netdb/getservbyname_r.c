#define _GNU_SOURCE
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>

#define ALIGN (sizeof(struct { char a; char *b; }) - sizeof(char *))
#define MAXSERVS 2

struct service {
	uint16_t port;
	char proto;
};

extern int __lookup_serv(struct service buf[static MAXSERVS], const char *name, int proto, int flags);

int getservbyname_r(const char *name, const char *prots,
	struct servent *se, char *buf, size_t buflen, struct servent **res)
{
	struct service servs[MAXSERVS];
	int cnt, proto, align;

	/* Align buffer */
	align = -(uintptr_t)buf & ALIGN-1;
	if (buflen < 2*sizeof(char *)+align)
		return ERANGE;
	buf += align;

	if (!prots) proto = 0;
	else if (!strcmp(prots, "tcp")) proto = IPPROTO_TCP;
	else if (!strcmp(prots, "udp")) proto = IPPROTO_UDP;
	else return EINVAL;

	cnt = __lookup_serv(servs, name, proto, 0);
	if (cnt<0) switch (cnt) {
	case EAI_MEMORY:
	case EAI_SYSTEM:
		return ENOMEM;
	default:
		return ENOENT;
	}

	se->s_name = (char *)name;
	se->s_aliases = (void *)buf;
	se->s_aliases[0] = se->s_name;
	se->s_aliases[1] = 0;
	se->s_port = htons(servs[0].port);
	se->s_proto = servs[0].proto == IPPROTO_TCP ? "tcp" : "udp";

	*res = se;
	return 0;
}
