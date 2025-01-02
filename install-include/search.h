/* Declarations for System V style searching functions.
   Copyright (C) 1995-2023 Free Software Foundation, Inc.
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

#ifndef _LIBRECOMPAT_SEARCH_H_
#define	_LIBRECOMPAT_SEARCH_H_

#define __need_size_t
#include_next <search.h>

__BEGIN_DECLS

/* For use with hsearch(3).  */
#ifndef __COMPAR_FN_T
# define __COMPAR_FN_T
typedef int (*__compar_fn_t) (const void *, const void *);

typedef __compar_fn_t comparison_fn_t;
#endif

/* Opaque type for internal use.  */
struct _ENTRY;

/* Data type for reentrant functions.  */
struct hsearch_data
  {
    struct _ENTRY *table;
    unsigned int size;
    unsigned int filled;
  };

/* Reentrant versions which can handle multiple hashing tables at the
   same time.  */
extern int hsearch_r (ENTRY __item, ACTION __action, ENTRY **__retval,
		      struct hsearch_data *__htab);
extern int hcreate_r (size_t __nel, struct hsearch_data *__htab);
extern void hdestroy_r (struct hsearch_data *__htab);

#ifndef __ACTION_FN_T
# define __ACTION_FN_T
typedef void (*__action_fn_t) (const void *__nodep, VISIT __value,
			       int __level);
#endif

/* Like twalk, but pass down a closure parameter instead of the
   level.  */
extern void twalk_r (const void *__root,
		     void (*) (const void *__nodep, VISIT __value,
			       void *__closure),
		     void *__closure);

/* Callback type for function to free a tree node.  If the keys are atomic
   data this function should do nothing.  */
typedef void (*__free_fn_t) (void *__nodep);

/* Destroy the whole tree, call FREEFCT for each node or leaf.  */
extern void tdestroy (void *__root, __free_fn_t __freefct);

__END_DECLS

#endif /* search.h */
