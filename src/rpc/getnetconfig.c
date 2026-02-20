/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Portions of this file are derived from FreeBSD libc RPC netconfig code:
 *   - lib/libc/rpc/getnetconfig.c
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

#include <ctype.h>
#include <dlfcn.h>
#include <limits.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXNETCONFIGLINE 1000

#define NC_TPI_CLTS_S "tpi_clts"
#define NC_TPI_COTS_S "tpi_cots"
#define NC_TPI_COTS_ORD_S "tpi_cots_ord"
#define NC_TPI_RAW_S "tpi_raw"

#define NC_NOFLAG_C '-'
#define NC_VISIBLE_C 'v'
#define NC_BROADCAST_C 'b'
#define NC_NOLOOKUP "-"

enum load_state {
	LOAD_STATE_OK = 0,
	LOAD_STATE_NOFILE = 1,
	LOAD_STATE_ERROR = -1
};

struct nc_handle {
	unsigned int magic;
	size_t index;
};

static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static struct netconfig *g_entries;
static size_t g_entry_count;
static size_t g_entry_cap;
static unsigned int g_refcount;
static int g_loaded;
static _Thread_local int g_nc_error;
static _Thread_local char g_nc_errbuf[128];

#if defined(__APPLE__)
#if defined(HARD_COREFOUNDATION)
#include <CoreFoundation/CoreFoundation.h>
#else
typedef const void *CFTypeRef;
typedef const void *CFStringRef;
typedef const void *CFDictionaryRef;
typedef unsigned long CFIndex;
typedef unsigned long CFTypeID;
typedef uint32_t CFStringEncoding;

#define NC_KCFSTRINGENCODINGUTF8 ((CFStringEncoding)0x08000100U)
#endif

struct sc_runtime {
	int available;
	void *sc_handle;
	CFTypeRef (*SCDynamicStoreCopyValue)(void *store, CFStringRef key);
#if !defined(HARD_COREFOUNDATION)
	CFStringRef (*CFStringCreateWithCString)(void *alloc, const char *c_str,
						 CFStringEncoding encoding);
	void (*CFRelease)(CFTypeRef cf);
	CFTypeID (*CFGetTypeID)(CFTypeRef cf);
	CFTypeID (*CFDictionaryGetTypeID)(void);
	CFIndex (*CFDictionaryGetCount)(CFDictionaryRef dict);
#endif
};

static struct sc_runtime g_sc_runtime;
static pthread_once_t g_sc_runtime_once = PTHREAD_ONCE_INIT;
#endif

void
_nc_set_error(int err)
{
	g_nc_error = err;
}

int
_nc_get_error(void)
{
	return g_nc_error;
}

char *
_nc_dup_cstr(const char *s)
{
	size_t len;
	char *copy;

	if (s == NULL) {
		return NULL;
	}
	len = strlen(s) + 1;
	copy = malloc(len);
	if (copy == NULL) {
		return NULL;
	}
	memcpy(copy, s, len);
	return copy;
}

static int
size_mul_ok(size_t a, size_t b, size_t *result)
{
	if (a != 0 && b > SIZE_MAX / a) {
		return 0;
	}
	*result = a * b;
	return 1;
}

static void
free_netconfig_fields(struct netconfig *ncp)
{
	size_t i;

	if (ncp == NULL) {
		return;
	}
	free(ncp->nc_netid);
	free(ncp->nc_protofmly);
	free(ncp->nc_proto);
	free(ncp->nc_device);
	if (ncp->nc_lookups != NULL) {
		for (i = 0; i < ncp->nc_nlookups; i++) {
			free(ncp->nc_lookups[i]);
		}
		free(ncp->nc_lookups);
	}
	memset(ncp, 0, sizeof(*ncp));
}

static int
parse_semantics(const char *token, unsigned long *semantics)
{
	if (strcmp(token, NC_TPI_COTS_ORD_S) == 0) {
		*semantics = NC_TPI_COTS_ORD;
		return 0;
	}
	if (strcmp(token, NC_TPI_COTS_S) == 0) {
		*semantics = NC_TPI_COTS;
		return 0;
	}
	if (strcmp(token, NC_TPI_CLTS_S) == 0) {
		*semantics = NC_TPI_CLTS;
		return 0;
	}
	if (strcmp(token, NC_TPI_RAW_S) == 0) {
		*semantics = NC_TPI_RAW;
		return 0;
	}
	return -1;
}

static int
parse_flags(const char *token, unsigned long *flags)
{
	*flags = NC_NOFLAG;
	for (; *token != '\0'; token++) {
		switch (*token) {
		case NC_NOFLAG_C:
			break;
		case NC_VISIBLE_C:
			*flags |= NC_VISIBLE;
			break;
		case NC_BROADCAST_C:
			*flags |= NC_BROADCAST;
			break;
		default:
			return -1;
		}
	}
	return 0;
}

static int
parse_lookups(const char *token, unsigned long *count_out, char ***lookups_out)
{
	char *work;
	char *saveptr;
	char *segment;
	char **lookups;
	size_t count;
	size_t cap;
	size_t bytes;

	*count_out = 0;
	*lookups_out = NULL;
	if (strcmp(token, NC_NOLOOKUP) == 0) {
		return 0;
	}

	work = _nc_dup_cstr(token);
	if (work == NULL) {
		return -1;
	}

	lookups = NULL;
	count = 0;
	cap = 0;
	saveptr = NULL;
	segment = strtok_r(work, ",", &saveptr);
	while (segment != NULL) {
		if (*segment != '\0') {
			char *value;

			if (count == cap) {
				char **new_lookups;
				size_t new_cap = (cap == 0) ? 4 : cap * 2;

				if (!size_mul_ok(new_cap, sizeof(*lookups), &bytes)) {
					goto fail;
				}
				new_lookups = realloc(lookups, bytes);
				if (new_lookups == NULL) {
					goto fail;
				}
				lookups = new_lookups;
				cap = new_cap;
			}

			value = _nc_dup_cstr(segment);
			if (value == NULL) {
				goto fail;
			}
			lookups[count++] = value;
		}
		segment = strtok_r(NULL, ",", &saveptr);
	}

	free(work);
	*count_out = (unsigned long)count;
	*lookups_out = lookups;
	return 0;

fail:
	if (lookups != NULL) {
		size_t i;

		for (i = 0; i < count; i++) {
			free(lookups[i]);
		}
		free(lookups);
	}
	free(work);
	return -1;
}

static int
parse_netconfig_line(char *line, struct netconfig *out, int *is_entry)
{
	char *saveptr;
	char *fields[7];
	size_t i;

	*is_entry = 0;
	memset(out, 0, sizeof(*out));

	if (line == NULL) {
		return -1;
	}

	line[strcspn(line, "\r\n")] = '\0';
	saveptr = NULL;
	for (i = 0; i < 7; i++) {
		fields[i] = strtok_r(i == 0 ? line : NULL, " \t", &saveptr);
		if (fields[i] == NULL) {
			return -1;
		}
	}

	out->nc_netid = _nc_dup_cstr(fields[0]);
	out->nc_protofmly = _nc_dup_cstr(fields[3]);
	out->nc_proto = _nc_dup_cstr(fields[4]);
	out->nc_device = _nc_dup_cstr(fields[5]);
	if (out->nc_netid == NULL || out->nc_protofmly == NULL ||
	    out->nc_proto == NULL || out->nc_device == NULL) {
		free_netconfig_fields(out);
		return -1;
	}
	if (parse_semantics(fields[1], &out->nc_semantics) != 0) {
		free_netconfig_fields(out);
		return -1;
	}
	if (parse_flags(fields[2], &out->nc_flag) != 0) {
		free_netconfig_fields(out);
		return -1;
	}
	if (parse_lookups(fields[6], &out->nc_nlookups, &out->nc_lookups) != 0) {
		free_netconfig_fields(out);
		return -1;
	}

	*is_entry = 1;
	return 0;
}

static int
append_global_entry_locked(struct netconfig *entry)
{
	size_t bytes;

	if (g_entry_count == g_entry_cap) {
		struct netconfig *new_entries;
		size_t new_cap = (g_entry_cap == 0) ? 8 : g_entry_cap * 2;

		if (!size_mul_ok(new_cap, sizeof(*g_entries), &bytes)) {
			return -1;
		}
		new_entries = realloc(g_entries, bytes);
		if (new_entries == NULL) {
			return -1;
		}
		g_entries = new_entries;
		g_entry_cap = new_cap;
	}

	g_entries[g_entry_count++] = *entry;
	memset(entry, 0, sizeof(*entry));
	return 0;
}

static void
clear_global_entries_locked(void)
{
	size_t i;

	for (i = 0; i < g_entry_count; i++) {
		free_netconfig_fields(&g_entries[i]);
	}
	free(g_entries);
	g_entries = NULL;
	g_entry_count = 0;
	g_entry_cap = 0;
	g_loaded = 0;
}

static int
load_file_entries_locked(void)
{
	FILE *fp;
	char line[MAXNETCONFIGLINE];
	int saw_data = 0;

	fp = fopen(NETCONFIG, "r");
	if (fp == NULL) {
		return LOAD_STATE_NOFILE;
	}

	while (fgets(line, sizeof(line), fp) != NULL) {
		struct netconfig entry;
		int is_entry;

		if (line[0] == '#') {
			continue;
		}

		if (strchr(line, '\n') == NULL && !feof(fp)) {
			fclose(fp);
			_nc_set_error(NC_BADFILE);
			return LOAD_STATE_ERROR;
		}

		if (parse_netconfig_line(line, &entry, &is_entry) != 0) {
			fclose(fp);
			_nc_set_error(NC_BADFILE);
			return LOAD_STATE_ERROR;
		}
		saw_data = 1;
		if (append_global_entry_locked(&entry) != 0) {
			free_netconfig_fields(&entry);
			fclose(fp);
			_nc_set_error(NC_NOMEM);
			return LOAD_STATE_ERROR;
		}
	}

	fclose(fp);
	if (!saw_data) {
		_nc_set_error(NC_BADFILE);
		return LOAD_STATE_ERROR;
	}
	return LOAD_STATE_OK;
}

static int
load_default_entries_with_visibility_locked(int v4_visible, int v6_visible)
{
	const char v4_flag = v4_visible ? 'v' : '-';
	const char v6_flag = v6_visible ? 'v' : '-';
	char defaults[6][MAXNETCONFIGLINE];
	const char *entries[] = {
		defaults[0],
		defaults[1],
		defaults[2],
		defaults[3],
		defaults[4],
		defaults[5],
	};
	size_t i;

	(void)snprintf(defaults[0], sizeof(defaults[0]),
	    "udp6 tpi_clts %c inet6 udp - -", v6_flag);
	(void)snprintf(defaults[1], sizeof(defaults[1]),
	    "tcp6 tpi_cots_ord %c inet6 tcp - -", v6_flag);
	(void)snprintf(defaults[2], sizeof(defaults[2]),
	    "udp tpi_clts %c inet udp - -", v4_flag);
	(void)snprintf(defaults[3], sizeof(defaults[3]),
	    "tcp tpi_cots_ord %c inet tcp - -", v4_flag);
	(void)snprintf(defaults[4], sizeof(defaults[4]),
	    "rawip tpi_raw - inet - - -");
	(void)snprintf(defaults[5], sizeof(defaults[5]),
	    "local tpi_cots_ord - loopback - - -");

	for (i = 0; i < sizeof(entries) / sizeof(entries[0]); i++) {
		struct netconfig entry;
		int is_entry;
		char line[MAXNETCONFIGLINE];
		size_t len = strlen(entries[i]);

		if (len + 1 > sizeof(line)) {
			_nc_set_error(NC_BADFILE);
			return -1;
		}
		memcpy(line, entries[i], len + 1);
		if (parse_netconfig_line(line, &entry, &is_entry) != 0 || !is_entry) {
			free_netconfig_fields(&entry);
			_nc_set_error(NC_BADFILE);
			return -1;
		}
		if (append_global_entry_locked(&entry) != 0) {
			free_netconfig_fields(&entry);
			_nc_set_error(NC_NOMEM);
			return -1;
		}
	}
	return 0;
}

static int
load_default_entries_locked(void)
{
	return load_default_entries_with_visibility_locked(1, 1);
}

#if defined(__APPLE__)
static int
sc_runtime_ready(void);

#if !defined(HARD_COREFOUNDATION)
static void *
sc_lookup_symbol(void *sc_handle, const char *name)
{
	void *sym;

	sym = dlsym(RTLD_DEFAULT, name);
	if (sym != NULL) {
		return sym;
	}
	return dlsym(sc_handle, name);
}
#endif

static void
sc_runtime_init_once(void)
{
	const int rtld_mode = RTLD_LAZY | RTLD_LOCAL;
	void *sym;
	void *sc_handle;

	sc_handle = dlopen(
	    "/System/Library/Frameworks/SystemConfiguration.framework/SystemConfiguration",
	    rtld_mode);
	if (sc_handle == NULL) {
		return;
	}

	sym = dlsym(sc_handle, "SCDynamicStoreCopyValue");
	if (sym == NULL) {
		goto fail;
	}
	memcpy(&g_sc_runtime.SCDynamicStoreCopyValue, &sym, sizeof(sym));

#if !defined(HARD_COREFOUNDATION)
	sym = sc_lookup_symbol(sc_handle, "CFStringCreateWithCString");
	if (sym == NULL) {
		goto fail;
	}
	memcpy(&g_sc_runtime.CFStringCreateWithCString, &sym, sizeof(sym));

	sym = sc_lookup_symbol(sc_handle, "CFRelease");
	if (sym == NULL) {
		goto fail;
	}
	memcpy(&g_sc_runtime.CFRelease, &sym, sizeof(sym));

	sym = sc_lookup_symbol(sc_handle, "CFGetTypeID");
	if (sym == NULL) {
		goto fail;
	}
	memcpy(&g_sc_runtime.CFGetTypeID, &sym, sizeof(sym));

	sym = sc_lookup_symbol(sc_handle, "CFDictionaryGetTypeID");
	if (sym == NULL) {
		goto fail;
	}
	memcpy(&g_sc_runtime.CFDictionaryGetTypeID, &sym, sizeof(sym));

	sym = sc_lookup_symbol(sc_handle, "CFDictionaryGetCount");
	if (sym == NULL) {
		goto fail;
	}
	memcpy(&g_sc_runtime.CFDictionaryGetCount, &sym, sizeof(sym));
#endif

	g_sc_runtime.sc_handle = sc_handle;
	g_sc_runtime.available = 1;
	return;

fail:
	dlclose(sc_handle);
}

static int
sc_runtime_ready(void)
{
	pthread_once(&g_sc_runtime_once, sc_runtime_init_once);
	return g_sc_runtime.available;
}

static int
sc_has_global_entity_key(const char *key_path)
{
	CFStringRef key;
	CFTypeRef value;
	int present;

	if (!sc_runtime_ready()) {
		return 0;
	}

#if defined(HARD_COREFOUNDATION)
	key = CFStringCreateWithCString(NULL, key_path, kCFStringEncodingUTF8);
#else
	key = g_sc_runtime.CFStringCreateWithCString(NULL,
						     key_path,
						     NC_KCFSTRINGENCODINGUTF8);
#endif
	if (key == NULL) {
		return 0;
	}

	value = g_sc_runtime.SCDynamicStoreCopyValue(NULL, key);
#if defined(HARD_COREFOUNDATION)
	CFRelease(key);
#else
	g_sc_runtime.CFRelease(key);
#endif
	if (value == NULL) {
		return 0;
	}

	present = 0;
#if defined(HARD_COREFOUNDATION)
	if (CFGetTypeID(value) == CFDictionaryGetTypeID() &&
	    CFDictionaryGetCount((CFDictionaryRef)value) > 0) {
#else
	if (g_sc_runtime.CFGetTypeID(value) == g_sc_runtime.CFDictionaryGetTypeID() &&
	    g_sc_runtime.CFDictionaryGetCount((CFDictionaryRef)value) > 0) {
#endif
		present = 1;
	}
#if defined(HARD_COREFOUNDATION)
	CFRelease(value);
#else
	g_sc_runtime.CFRelease(value);
#endif
	return present;
}

static int
load_systemconfiguration_entries_locked(void)
{
	int have_v4 = 0;
	int have_v6 = 0;
	int have_signal = 0;

	have_v4 = sc_has_global_entity_key("State:/Network/Global/IPv4");
	have_v6 = sc_has_global_entity_key("State:/Network/Global/IPv6");
	have_signal = have_v4 || have_v6;

	if (!have_signal) {
		int setup_v4;
		int setup_v6;

		setup_v4 = sc_has_global_entity_key("Setup:/Network/Global/IPv4");
		setup_v6 = sc_has_global_entity_key("Setup:/Network/Global/IPv6");
		if (setup_v4) {
			have_v4 = 1;
		}
		if (setup_v6) {
			have_v6 = 1;
		}
		have_signal = setup_v4 || setup_v6;
	}

	if (!have_signal) {
		return LOAD_STATE_NOFILE;
	}

	if (load_default_entries_with_visibility_locked(have_v4, have_v6) != 0) {
		return LOAD_STATE_ERROR;
	}
	return LOAD_STATE_OK;
}
#else
static int
load_systemconfiguration_entries_locked(void)
{
	return LOAD_STATE_NOFILE;
}
#endif

static int
ensure_entries_loaded_locked(void)
{
	int status;

	if (g_loaded) {
		return 0;
	}

	status = load_file_entries_locked();
	if (status == LOAD_STATE_OK) {
		g_loaded = 1;
		return 0;
	}
	if (status == LOAD_STATE_ERROR) {
		clear_global_entries_locked();
		return -1;
	}

	status = load_systemconfiguration_entries_locked();
	if (status == LOAD_STATE_OK) {
		g_loaded = 1;
		return 0;
	}
	if (status == LOAD_STATE_ERROR) {
		clear_global_entries_locked();
		return -1;
	}

	if (load_default_entries_locked() != 0) {
		clear_global_entries_locked();
		if (g_nc_error == 0) {
			_nc_set_error(NC_NONETCONFIG);
		}
		return -1;
	}
	g_loaded = 1;
	return 0;
}

static struct netconfig *
dup_entry(const struct netconfig *source)
{
	struct netconfig *copy;
	size_t i;
	size_t bytes;

	copy = calloc(1, sizeof(*copy));
	if (copy == NULL) {
		return NULL;
	}
	copy->nc_semantics = source->nc_semantics;
	copy->nc_flag = source->nc_flag;
	copy->nc_nlookups = source->nc_nlookups;
	memcpy(copy->nc_unused, source->nc_unused, sizeof(copy->nc_unused));

	copy->nc_netid = _nc_dup_cstr(source->nc_netid);
	copy->nc_protofmly = _nc_dup_cstr(source->nc_protofmly);
	copy->nc_proto = _nc_dup_cstr(source->nc_proto);
	copy->nc_device = _nc_dup_cstr(source->nc_device);
	if (copy->nc_netid == NULL || copy->nc_protofmly == NULL ||
	    copy->nc_proto == NULL || copy->nc_device == NULL) {
		free_netconfig_fields(copy);
		free(copy);
		return NULL;
	}

	if (source->nc_nlookups == 0) {
		return copy;
	}

	if (!size_mul_ok(source->nc_nlookups, sizeof(*copy->nc_lookups), &bytes)) {
		free_netconfig_fields(copy);
		free(copy);
		return NULL;
	}
	copy->nc_lookups = calloc(1, bytes);
	if (copy->nc_lookups == NULL) {
		free_netconfig_fields(copy);
		free(copy);
		return NULL;
	}
	for (i = 0; i < source->nc_nlookups; i++) {
		copy->nc_lookups[i] = _nc_dup_cstr(source->nc_lookups[i]);
		if (copy->nc_lookups[i] == NULL) {
			free_netconfig_fields(copy);
			free(copy);
			return NULL;
		}
	}
	return copy;
}

void *
setnetconfig(void)
{
	struct nc_handle *handle;

	handle = calloc(1, sizeof(*handle));
	if (handle == NULL) {
		_nc_set_error(NC_NOMEM);
		return NULL;
	}

	pthread_mutex_lock(&g_lock);
	if (ensure_entries_loaded_locked() != 0) {
		pthread_mutex_unlock(&g_lock);
		free(handle);
		return NULL;
	}
	g_refcount++;
	pthread_mutex_unlock(&g_lock);

	handle->magic = NC_HANDLE_MAGIC;
	handle->index = 0;
	return handle;
}

struct netconfig *
getnetconfig(void *handlep)
{
	struct nc_handle *handle = handlep;
	struct netconfig *entry = NULL;

	if (handle == NULL || handle->magic != NC_HANDLE_MAGIC) {
		_nc_set_error(NC_NOTINIT);
		return NULL;
	}

	pthread_mutex_lock(&g_lock);
	if (!g_loaded) {
		pthread_mutex_unlock(&g_lock);
		_nc_set_error(NC_NOTINIT);
		return NULL;
	}
	if (handle->index < g_entry_count) {
		entry = &g_entries[handle->index];
		handle->index++;
	}
	pthread_mutex_unlock(&g_lock);
	return entry;
}

int
endnetconfig(void *handlep)
{
	struct nc_handle *handle = handlep;

	if (handle == NULL || handle->magic != NC_HANDLE_MAGIC) {
		_nc_set_error(NC_NOTINIT);
		return -1;
	}

	handle->magic = 0;
	pthread_mutex_lock(&g_lock);
	if (g_refcount > 0) {
		g_refcount--;
	}
	if (g_refcount == 0) {
		clear_global_entries_locked();
	}
	pthread_mutex_unlock(&g_lock);

	free(handle);
	return 0;
}

struct netconfig *
getnetconfigent(const char *netid)
{
	size_t i;
	int found = 0;
	struct netconfig *copy = NULL;

	_nc_set_error(NC_NOTFOUND);
	if (netid == NULL || *netid == '\0') {
		return NULL;
	}

	pthread_mutex_lock(&g_lock);
	if (ensure_entries_loaded_locked() != 0) {
		pthread_mutex_unlock(&g_lock);
		return NULL;
	}
	for (i = 0; i < g_entry_count; i++) {
		if (strcmp(g_entries[i].nc_netid, netid) == 0) {
			found = 1;
			copy = dup_entry(&g_entries[i]);
			break;
		}
	}
	pthread_mutex_unlock(&g_lock);

	if (copy == NULL) {
		if (found) {
			_nc_set_error(NC_NOMEM);
		} else if (g_nc_error == 0) {
			_nc_set_error(NC_NOTFOUND);
		}
		return NULL;
	}
	return copy;
}

void
freenetconfigent(struct netconfig *netconfigp)
{
	if (netconfigp == NULL) {
		return;
	}
	free_netconfig_fields(netconfigp);
	free(netconfigp);
}

char *
nc_sperror(void)
{
	const char *message;

	switch (g_nc_error) {
	case NC_NONETCONFIG:
		message = "Netconfig database not found";
		break;
	case NC_NOMEM:
		message = "Not enough memory";
		break;
	case NC_NOTINIT:
		message = "Not initialized";
		break;
	case NC_BADFILE:
		message = "Netconfig database has invalid format";
		break;
	case NC_NOTFOUND:
		message = "Netid not found in netconfig database";
		break;
	default:
		message = "Unknown network selection error";
		break;
	}
	(void)snprintf(g_nc_errbuf, sizeof(g_nc_errbuf), "%s", message);
	return g_nc_errbuf;
}

void
nc_perror(const char *s)
{
	fprintf(stderr, "%s: %s\n", s, nc_sperror());
}
