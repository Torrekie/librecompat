#ifndef _LIBRECOMPAT_NETDB_H_
#define _LIBRECOMPAT_NETDB_H_

#include_next <netdb.h>

struct servent_data {
	void *fp;
	char **aliases;
	int maxaliases;
	int stayopen;
	char *line;
};

__BEGIN_DECLS

int gethostbyname_r __P((const char *name, struct hostent *result_buf,
                         char *buf, size_t buflen, struct hostent **result,
                         int *h_errnop));

int getservbyname_r __P((const char *name, const char *,
			 struct servent *, char *, size_t, struct servent **));
int getservbyport_r __P((int port, const char *,
			 struct servent *, char *, size_t, struct servent **));
int getservent_r __P((struct servent *, char *, size_t, struct servent **));

void		setservent_r __P((int));
void		endservent_r __P((void));

__END_DECLS

#endif
