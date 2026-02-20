#ifndef _NETCONFIG_H_
#define _NETCONFIG_H_

#if __has_include(<librecompat/librecompat_config.h>)
#include <librecompat/librecompat_config.h>
#elif __has_include("librecompat_config.h")
#include "librecompat_config.h"
#else
#include <librecompat_config.h>
#endif

#if LIBRECOMPAT_ROOTLESS
#define NETCONFIG "/var/jb/etc/netconfig"
#else
#define NETCONFIG "/etc/netconfig"
#endif
#define NETPATH "NETPATH"

struct netconfig {
	char *nc_netid;               /* Network ID */
	unsigned long nc_semantics;   /* Semantics (see below) */
	unsigned long nc_flag;        /* Flags (see below) */
	char *nc_protofmly;           /* Protocol family */
	char *nc_proto;               /* Protocol name */
	char *nc_device;              /* Network device pathname */
	unsigned long nc_nlookups;    /* Number of directory lookup libs */
	char **nc_lookups;            /* Names of the libraries */
	unsigned long nc_unused[9];   /* reserved */
};

typedef struct {
	struct netconfig **nc_head;
	struct netconfig **nc_curr;
} NCONF_HANDLE;

/* nc_semantics values */
#define NC_TPI_CLTS 1      /* Connectionless transport */
#define NC_TPI_COTS 2      /* Connection oriented transport */
#define NC_TPI_COTS_ORD 3  /* Connection oriented, ordered transport */
#define NC_TPI_RAW 4       /* Raw connection */

/* nc_flag values */
#define NC_NOFLAG 0x00
#define NC_VISIBLE 0x01
#define NC_BROADCAST 0x02

/* nc_protofmly values */
#define NC_NOPROTOFMLY "-"
#define NC_LOOPBACK "loopback"
#define NC_INET "inet"
#define NC_INET6 "inet6"
#define NC_IMPLINK "implink"
#define NC_PUP "pup"
#define NC_CHAOS "chaos"
#define NC_NS "ns"
#define NC_NBS "nbs"
#define NC_ECMA "ecma"
#define NC_DATAKIT "datakit"
#define NC_CCITT "ccitt"
#define NC_SNA "sna"
#define NC_DECNET "decnet"
#define NC_DLI "dli"
#define NC_LAT "lat"
#define NC_HYLINK "hylink"
#define NC_APPLETALK "appletalk"
#define NC_NIT "nit"
#define NC_IEEE802 "ieee802"
#define NC_OSI "osi"
#define NC_X25 "x25"
#define NC_OSINET "osinet"
#define NC_GOSIP "gosip"

/* nc_proto values */
#define NC_NOPROTO "-"
#define NC_TCP "tcp"
#define NC_UDP "udp"
#define NC_ICMP "icmp"

#ifdef __cplusplus
extern "C" {
#endif

void *setnetconfig(void);
struct netconfig *getnetconfig(void *);
struct netconfig *getnetconfigent(const char *);
void freenetconfigent(struct netconfig *);
int endnetconfig(void *);

void *setnetpath(void);
struct netconfig *getnetpath(void *);
int endnetpath(void *);

void nc_perror(const char *);
char *nc_sperror(void);

#ifdef __cplusplus
}
#endif

#endif /* _NETCONFIG_H_ */
