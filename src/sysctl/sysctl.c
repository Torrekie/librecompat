/*-
 * Copyright (c) 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)sysctl.c	8.2 (Berkeley) 1/4/94";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID(
	"$FreeBSD: src/lib/libc/gen/sysctl.c,v 1.6 2007/01/09 00:27:55 imp Exp $");

#include <sys/param.h>
#include <sys/sysctl.h>

#include <errno.h>
#include <limits.h>
#include <paths.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifdef LIBRECOMPAT_ROOTLESS
#ifdef _PATH_STDPATH
#undef _PATH_STDPATH
#endif
#define _PATH_STDPATH "/usr/bin:/bin:/usr/sbin:/sbin:/var/jb/usr/bin:/var/jb/bin:/var/jb/usr/sbin:/var/jb/sbin"
#endif

int compat_sysctl(int *name, u_int namelen, void *oldp, size_t *oldlenp, void *newp,
		   size_t newlen) __attribute__((disable_tail_calls)) {
	switch (name[1]) {
	case USER_CS_PATH:
		if (oldp && *oldlenp < sizeof(_PATH_STDPATH)) {
			errno = ENOMEM;
			return -1;
		}
		*oldlenp = sizeof(_PATH_STDPATH);
		if (oldp != NULL)
			memmove(oldp, _PATH_STDPATH, sizeof(_PATH_STDPATH));
		return (0);
	}
	return sysctl(name, namelen, oldp, oldlenp, newp, newlen);
}
