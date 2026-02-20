/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Portions of this file are derived from FreeBSD libc RPC netconfig code:
 *   - lib/libc/rpc/getnetpath.c
 *
 * Copyright (c) 2009, Sun Microsystems, Inc.
 * Copyright (c) 1989, Sun Microsystems, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "netconfig_compat_internal.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

struct netpath_alloc {
	struct netconfig *entry;
	struct netpath_alloc *next;
};

struct np_handle {
	unsigned int magic;
	int use_visible;
	void *nc_handle;
	char *netpath_buf;
	char *cursor;
	struct netpath_alloc *alloc_head;
	struct netpath_alloc *alloc_tail;
};

static void
netpath_unescape(char *token)
{
	char *readp;
	char *writep;

	writep = token;
	for (readp = token; *readp != '\0'; readp++) {
		if (*readp == '\\' && readp[1] != '\0') {
			readp++;
		}
		*writep++ = *readp;
	}
	*writep = '\0';
}

static char *
netpath_next_token(char *s, char delim)
{
	char *p;
	int escaped = 0;

	if (s == NULL || *s == '\0') {
		return NULL;
	}

	for (p = s; *p != '\0'; p++) {
		if (!escaped && *p == '\\') {
			escaped = 1;
			continue;
		}
		if (!escaped && *p == delim) {
			*p = '\0';
			netpath_unescape(s);
			return p + 1;
		}
		escaped = 0;
	}

	netpath_unescape(s);
	return NULL;
}

static int
netpath_track_entry(struct np_handle *np, struct netconfig *entry)
{
	struct netpath_alloc *node;

	node = malloc(sizeof(*node));
	if (node == NULL) {
		return -1;
	}
	node->entry = entry;
	node->next = NULL;
	if (np->alloc_tail != NULL) {
		np->alloc_tail->next = node;
	} else {
		np->alloc_head = node;
	}
	np->alloc_tail = node;
	return 0;
}

void *
setnetpath(void)
{
	struct np_handle *np;
	const char *env_netpath;

	np = calloc(1, sizeof(*np));
	if (np == NULL) {
		_nc_set_error(NC_NOMEM);
		return NULL;
	}
	np->magic = NP_HANDLE_MAGIC;

	env_netpath = getenv(NETPATH);
	if (env_netpath == NULL || *env_netpath == '\0') {
		np->use_visible = 1;
		np->nc_handle = setnetconfig();
		if (np->nc_handle == NULL) {
			free(np);
			return NULL;
		}
		return np;
	}

	np->netpath_buf = _nc_dup_cstr(env_netpath);
	if (np->netpath_buf == NULL) {
		_nc_set_error(NC_NOMEM);
		free(np);
		return NULL;
	}
	np->cursor = np->netpath_buf;
	return np;
}

struct netconfig *
getnetpath(void *handlep)
{
	struct np_handle *np = handlep;

	if (np == NULL || np->magic != NP_HANDLE_MAGIC) {
		errno = EINVAL;
		_nc_set_error(NC_NOTINIT);
		return NULL;
	}

	if (np->use_visible) {
		struct netconfig *entry;

		do {
			entry = getnetconfig(np->nc_handle);
			if (entry == NULL) {
				return NULL;
			}
		} while ((entry->nc_flag & NC_VISIBLE) == 0);
		return entry;
	}

	while (np->cursor != NULL) {
		char *token;
		struct netconfig *entry;

		token = np->cursor;
		np->cursor = netpath_next_token(token, ':');
		if (*token == '\0') {
			continue;
		}
		entry = getnetconfigent(token);
		if (entry == NULL) {
			if (_nc_get_error() == NC_NOTFOUND) {
				_nc_set_error(0);
				continue;
			}
			return NULL;
		}
		if (netpath_track_entry(np, entry) != 0) {
			freenetconfigent(entry);
			_nc_set_error(NC_NOMEM);
			return NULL;
		}
		return entry;
	}
	return NULL;
}

int
endnetpath(void *handlep)
{
	struct np_handle *np = handlep;
	struct netpath_alloc *cur;

	if (np == NULL || np->magic != NP_HANDLE_MAGIC) {
		errno = EINVAL;
		return -1;
	}

	np->magic = 0;
	if (np->nc_handle != NULL) {
		endnetconfig(np->nc_handle);
	}
	free(np->netpath_buf);

	cur = np->alloc_head;
	while (cur != NULL) {
		struct netpath_alloc *next = cur->next;

		freenetconfigent(cur->entry);
		free(cur);
		cur = next;
	}

	free(np);
	return 0;
}
