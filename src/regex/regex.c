/* Extended regular expression matching and search library.
   Copyright (C) 2002-2025 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Isamu Hasegawa <isamu@yamato.ibm.com>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#define __STDC_WANT_IEC_60559_BFP_EXT__

#pragma clang diagnostic ignored "-Wvla"

/* Make sure no one compiles this code with a C++ compiler.  */
#if defined __cplusplus
# error "This is C code, use a C compiler"
#endif

/* On Darwin, we should explicitly define this */
#define _REGEX_LARGE_OFFSETS 1

/* Enable re_comp and re_exec */
#define _REGEX_RE_COMP 1

/* On some systems, limits.h sets RE_DUP_MAX to a lower value than
   GNU regex allows.  Include it before <regex.h>, which correctly
   #undefs RE_DUP_MAX and sets it to the right value.  */
#include <limits.h>

#ifndef likely
#define likely(expr) __builtin_expect(!!(expr), 1)
#endif

#ifndef unlikely
#define unlikely(expr) __builtin_expect(!!(expr), 0)
#endif

/* Define some glibc-specific macros that the regex code expects */
#ifndef __glibc_likely
#define __glibc_likely(expr) likely(expr)
#endif

#ifndef __glibc_unlikely
#define __glibc_unlikely(expr) unlikely(expr)
#endif

#if defined(__APPLE__) && defined(__MACH__)
#define STR1(x) #x
#define STR(x)  STR1(x)

/* Declare alias with same type as orig, then create the alias in asm. */
#define weak_alias(alias, orig)                                              \
    extern __typeof__(orig) alias;                                           \
    __asm__(".weak_definition " STR(__USER_LABEL_PREFIX__) #alias "\n"       \
            ".globl "          STR(__USER_LABEL_PREFIX__) #alias "\n"       \
            STR(__USER_LABEL_PREFIX__) #alias " = "                         \
            STR(__USER_LABEL_PREFIX__) #orig "\n");
#endif

#define TYPE_SIGNED(t) (! ((t) 0 < (t) -1))

#include "regex.h"
#include "regex_internal.h"

#include "regex_internal.c"
#include "regcomp.c"
#include "regexec.c"
