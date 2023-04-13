#ifndef _ARGZ_H

#include <string/argz.h>

# ifndef _ISOMAC

extern error_t __argz_create_sep (const char *__restrict __string,
				  int __sep, char **__restrict __argz,
				  size_t *__restrict __len)
     __THROW;
extern error_t __argz_append (char **__restrict __argz,
			      size_t *__restrict __argz_len,
			      const char *__restrict __buf, size_t __buf_len)
     __THROW;
extern error_t __argz_add (char **__restrict __argz,
			   size_t *__restrict __argz_len,
			   const char *__restrict __str)
     __THROW;
extern error_t __argz_add_sep (char **__restrict __argz,
			       size_t *__restrict __argz_len,
			       const char *__restrict __string, int __delim)
     __THROW;
extern void __argz_delete (char **__restrict __argz,
			   size_t *__restrict __argz_len,
			   char *__restrict __entry)
     __THROW;
extern error_t __argz_insert (char **__restrict __argz,
			      size_t *__restrict __argz_len,
			      char *__restrict __before,
			      const char *__restrict __entry)
     __THROW;
extern error_t __argz_replace (char **__restrict __argz,
			       size_t *__restrict __argz_len,
			       const char *__restrict __str,
			       const char *__restrict __with,
			       unsigned int *__restrict __replace_count);

# endif /* !_ISOMAC */
#endif
