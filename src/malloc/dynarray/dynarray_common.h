#pragma once

#include <stdbool.h>
#include <sys/types.h>

struct dynarray_header
{
  size_t used;
  size_t allocated;
  void *array;
};

/* Marker used in the allocated member to indicate that an error was
   encountered.  */
static inline size_t
__dynarray_error_marker (void)
{
  return -1;
}

/* Internal function.  See the has_failed function in
   dynarray-skeleton.c.  */
static inline bool
__dynarray_error (struct dynarray_header *list)
{
  return list->allocated == __dynarray_error_marker ();
}
/* Internal type.  */
struct dynarray_finalize_result
{
  void *array;
  size_t length;
};
