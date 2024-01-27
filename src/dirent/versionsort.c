/* Copyright (C) 1992-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#include <dirent.h>

extern int __strverscmp(const char* s1, const char* s2);

#if !__DARWIN_64_BIT_INO_T
# include <string.h>

int
versionsort (const struct dirent **a, const struct dirent **b)
{
  return __strverscmp ((*a)->d_name, (*b)->d_name);
}

#endif
