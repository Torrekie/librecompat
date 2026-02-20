/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Shared internals for netconfig/getnetpath compatibility layer.
 */

#ifndef NETCONFIG_COMPAT_INTERNAL_H
#define NETCONFIG_COMPAT_INTERNAL_H

#include <errno.h>
#include <stddef.h>

#if defined(__has_include)
#if __has_include(<netconfig.h>)
#include <netconfig.h>
#define NC_HAVE_NETCONFIG_H 1
#endif
#endif

#if !defined(NC_HAVE_NETCONFIG_H)
#ifdef LIBRECOMPAT_ROOTLESS
#define NETCONFIG "/var/jb/etc/netconfig"
#else
#define NETCONFIG "/etc/netconfig"
#endif
#define NETPATH "NETPATH"

struct netconfig {
	char *nc_netid;
	unsigned long nc_semantics;
	unsigned long nc_flag;
	char *nc_protofmly;
	char *nc_proto;
	char *nc_device;
	unsigned long nc_nlookups;
	char **nc_lookups;
	unsigned long nc_unused[9];
};

typedef struct {
	struct netconfig **nc_head;
	struct netconfig **nc_curr;
} NCONF_HANDLE;

#define NC_TPI_CLTS 1
#define NC_TPI_COTS 2
#define NC_TPI_COTS_ORD 3
#define NC_TPI_RAW 4

#define NC_NOFLAG 0x00
#define NC_VISIBLE 0x01
#define NC_BROADCAST 0x02

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
#endif

#define NC_NONETCONFIG ENOENT
#define NC_NOMEM ENOMEM
#define NC_NOTINIT EINVAL
#define NC_BADFILE EBADF
#define NC_NOTFOUND ENOPROTOOPT

#define NC_HANDLE_MAGIC 0x4e434647u
#define NP_HANDLE_MAGIC 0x4e505448u

char *_nc_dup_cstr(const char *s);
void _nc_set_error(int err);
int _nc_get_error(void);

#endif /* NETCONFIG_COMPAT_INTERNAL_H */
