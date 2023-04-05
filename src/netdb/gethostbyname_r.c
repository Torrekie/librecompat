#include <netdb.h>
#include <string.h>

int gethostbyname_r(const char *name, struct hostent *ret, char *buf, size_t buflen, struct hostent **result, int *h_errnop) {
	int err;
	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	err = getaddrinfo(name, NULL, &hints, &res);
	if (err != 0) {
		switch (err) {
			case EAI_AGAIN:
				*h_errnop = TRY_AGAIN;
				break;
			case EAI_NODATA:
				*h_errnop = NO_DATA;
				break;
			case EAI_FAIL:
				*h_errnop = NO_RECOVERY;
				break;
			case EAI_SYSTEM:
				*h_errnop = NETDB_INTERNAL;
				break;
			default:
				*h_errnop = HOST_NOT_FOUND;
				break;
		}
		*result = NULL;
		return -1;
	}
	struct hostent *host = gethostbyname(name);
	if (host == NULL) {
		*h_errnop = h_errno;
		*result = NULL;
		return -1;
	}
	memcpy(ret, host, sizeof(struct hostent));
	*result = ret;
	*h_errnop = NETDB_SUCCESS;
	return 0;
}
