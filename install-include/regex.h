/* Definitions for data structures and routines for the regular
   expression library.
   Copyright (C) 1985, 1989-2023 Free Software Foundation, Inc.
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

#ifndef _LIBRECOMPAT_REGEX_H_
#define _LIBRECOMPAT_REGEX_H_ 1

#include_next <regex.h>
#include <sys/types.h>

/* Allow the use in C++ code.  */
#ifdef __cplusplus
extern "C" {
#endif

#ifdef _REGEX_LARGE_OFFSETS

/* Use types and values that are wide enough to represent signed and
   unsigned byte offsets in memory.  This currently works only when
   the regex code is used outside of the GNU C library; it is not yet
   supported within glibc itself, and glibc users should not define
   _REGEX_LARGE_OFFSETS.  */

/* The type of object sizes.  */
typedef size_t __re_size_t;

/* The type of object sizes, in places where the traditional code
   uses unsigned long int.  */
typedef size_t __re_long_size_t;

#else

/* The traditional GNU regex implementation mishandles strings longer
   than INT_MAX.  */
typedef unsigned int __re_size_t;
typedef unsigned long int __re_long_size_t;

#endif

/* The following two types have to be signed and unsigned integer type
   wide enough to hold a value of a pointer.  For most ANSI compilers
   ptrdiff_t and size_t should be likely OK.  Still size of these two
   types is 2 for Microsoft C.  Ugh... */
typedef long int s_reg_t;
typedef unsigned long int active_reg_t;

/* The following bits are used to determine the regexp syntax we
   recognize.  The set/not-set meanings are chosen so that Emacs syntax
   remains the value 0.  The bits are given in alphabetical order, and
   the definitions shifted by one from the previous bit; thus, when we
   add or remove a bit, only one other definition need change.  */
typedef unsigned long int reg_syntax_t;

/* If this bit is not set, then \ inside a bracket expression is literal.
   If set, then such a \ quotes the following character.  */
# define RE_BACKSLASH_ESCAPE_IN_LISTS ((unsigned long int) 1)

/* If this bit is not set, then + and ? are operators, and \+ and \? are
     literals.
   If set, then \+ and \? are operators and + and ? are literals.  */
# define RE_BK_PLUS_QM (RE_BACKSLASH_ESCAPE_IN_LISTS << 1)

/* If this bit is set, then character classes are supported.  They are:
     [:alpha:], [:upper:], [:lower:],  [:digit:], [:alnum:], [:xdigit:],
     [:space:], [:print:], [:punct:], [:graph:], and [:cntrl:].
   If not set, then character classes are not supported.  */
# define RE_CHAR_CLASSES (RE_BK_PLUS_QM << 1)

/* If this bit is set, then ^ and $ are always anchors (outside bracket
     expressions, of course).
   If this bit is not set, then it depends:
	^  is an anchor if it is at the beginning of a regular
	   expression or after an open-group or an alternation operator;
	$  is an anchor if it is at the end of a regular expression, or
	   before a close-group or an alternation operator.

   This bit could be (re)combined with RE_CONTEXT_INDEP_OPS, because
   POSIX draft 11.2 says that * etc. in leading positions is undefined.
   We already implemented a previous draft which made those constructs
   invalid, though, so we haven't changed the code back.  */
# define RE_CONTEXT_INDEP_ANCHORS (RE_CHAR_CLASSES << 1)

/* If this bit is set, then special characters are always special
     regardless of where they are in the pattern.
   If this bit is not set, then special characters are special only in
     some contexts; otherwise they are ordinary.  Specifically,
     * + ? and intervals are only special when not after the beginning,
     open-group, or alternation operator.  */
# define RE_CONTEXT_INDEP_OPS (RE_CONTEXT_INDEP_ANCHORS << 1)

/* If this bit is set, then *, +, ?, and { cannot be first in an re or
     immediately after an alternation or begin-group operator.  */
# define RE_CONTEXT_INVALID_OPS (RE_CONTEXT_INDEP_OPS << 1)

/* If this bit is set, then . matches newline.
   If not set, then it doesn't.  */
# define RE_DOT_NEWLINE (RE_CONTEXT_INVALID_OPS << 1)

/* If this bit is set, then . doesn't match NUL.
   If not set, then it does.  */
# define RE_DOT_NOT_NULL (RE_DOT_NEWLINE << 1)

/* If this bit is set, nonmatching lists [^...] do not match newline.
   If not set, they do.  */
# define RE_HAT_LISTS_NOT_NEWLINE (RE_DOT_NOT_NULL << 1)

/* If this bit is set, either \{...\} or {...} defines an
     interval, depending on RE_NO_BK_BRACES.
   If not set, \{, \}, {, and } are literals.  */
# define RE_INTERVALS (RE_HAT_LISTS_NOT_NEWLINE << 1)

/* If this bit is set, +, ? and | aren't recognized as operators.
   If not set, they are.  */
# define RE_LIMITED_OPS (RE_INTERVALS << 1)

/* If this bit is set, newline is an alternation operator.
   If not set, newline is literal.  */
# define RE_NEWLINE_ALT (RE_LIMITED_OPS << 1)

/* If this bit is set, then '{...}' defines an interval, and \{ and \}
     are literals.
  If not set, then '\{...\}' defines an interval.  */
# define RE_NO_BK_BRACES (RE_NEWLINE_ALT << 1)

/* If this bit is set, (...) defines a group, and \( and \) are literals.
   If not set, \(...\) defines a group, and ( and ) are literals.  */
# define RE_NO_BK_PARENS (RE_NO_BK_BRACES << 1)

/* If this bit is set, then \<digit> matches <digit>.
   If not set, then \<digit> is a back-reference.  */
# define RE_NO_BK_REFS (RE_NO_BK_PARENS << 1)

/* If this bit is set, then | is an alternation operator, and \| is literal.
   If not set, then \| is an alternation operator, and | is literal.  */
# define RE_NO_BK_VBAR (RE_NO_BK_REFS << 1)

/* If this bit is set, then an ending range point collating higher
     than the starting range point, as in [z-a], is invalid.
   If not set, then when ending range point collates higher than the
     starting range point, the range is ignored.  */
# define RE_NO_EMPTY_RANGES (RE_NO_BK_VBAR << 1)

/* If this bit is set, then an unmatched ) is ordinary.
   If not set, then an unmatched ) is invalid.  */
# define RE_UNMATCHED_RIGHT_PAREN_ORD (RE_NO_EMPTY_RANGES << 1)

/* If this bit is set, succeed as soon as we match the whole pattern,
   without further backtracking.  */
# define RE_NO_POSIX_BACKTRACKING (RE_UNMATCHED_RIGHT_PAREN_ORD << 1)

/* If this bit is set, do not process the GNU regex operators.
   If not set, then the GNU regex operators are recognized. */
# define RE_NO_GNU_OPS (RE_NO_POSIX_BACKTRACKING << 1)

/* If this bit is set, turn on internal regex debugging.
   If not set, and debugging was on, turn it off.
   This only works if regex.c is compiled -DDEBUG.
   We define this bit always, so that all that's needed to turn on
   debugging is to recompile regex.c; the calling code can always have
   this bit set, and it won't affect anything in the normal case. */
# define RE_DEBUG (RE_NO_GNU_OPS << 1)

/* If this bit is set, a syntactically invalid interval is treated as
   a string of ordinary characters.  For example, the ERE 'a{1' is
   treated as 'a\{1'.  */
# define RE_INVALID_INTERVAL_ORD (RE_DEBUG << 1)

/* If this bit is set, then ignore case when matching.
   If not set, then case is significant.  */
# define RE_ICASE (RE_INVALID_INTERVAL_ORD << 1)

/* This bit is used internally like RE_CONTEXT_INDEP_ANCHORS but only
   for ^, because it is difficult to scan the regex backwards to find
   whether ^ should be special.  */
# define RE_CARET_ANCHORS_HERE (RE_ICASE << 1)

/* If this bit is set, then \{ cannot be first in a regex or
   immediately after an alternation, open-group or \} operator.  */
# define RE_CONTEXT_INVALID_DUP (RE_CARET_ANCHORS_HERE << 1)

/* If this bit is set, then no_sub will be set to 1 during
   re_compile_pattern.  */
# define RE_NO_SUB (RE_CONTEXT_INVALID_DUP << 1)

/* This global variable defines the particular regexp syntax to use (for
   some interfaces).  When a regexp is compiled, the syntax used is
   stored in the pattern buffer, so changing this does not affect
   already-compiled regexps.  */
extern reg_syntax_t re_syntax_options;

/* Define combinations of the above bits for the standard possibilities.
   (The [[[ comments delimit what gets put into the Texinfo file, so
   don't delete them!)  */
/* [[[begin syntaxes]]] */
# define RE_SYNTAX_EMACS 0

# define RE_SYNTAX_AWK							\
  (RE_BACKSLASH_ESCAPE_IN_LISTS   | RE_DOT_NOT_NULL			\
   | RE_NO_BK_PARENS              | RE_NO_BK_REFS			\
   | RE_NO_BK_VBAR                | RE_NO_EMPTY_RANGES			\
   | RE_DOT_NEWLINE		  | RE_CONTEXT_INDEP_ANCHORS		\
   | RE_CHAR_CLASSES							\
   | RE_UNMATCHED_RIGHT_PAREN_ORD | RE_NO_GNU_OPS)

# define RE_SYNTAX_GNU_AWK						\
  ((RE_SYNTAX_POSIX_EXTENDED | RE_BACKSLASH_ESCAPE_IN_LISTS		\
    | RE_INVALID_INTERVAL_ORD)						\
   & ~(RE_DOT_NOT_NULL | RE_CONTEXT_INDEP_OPS				\
      | RE_CONTEXT_INVALID_OPS ))

# define RE_SYNTAX_POSIX_AWK						\
  (RE_SYNTAX_POSIX_EXTENDED | RE_BACKSLASH_ESCAPE_IN_LISTS		\
   | RE_INTERVALS	    | RE_NO_GNU_OPS				\
   | RE_INVALID_INTERVAL_ORD)

# define RE_SYNTAX_GREP							\
  ((RE_SYNTAX_POSIX_BASIC | RE_NEWLINE_ALT)				\
   & ~(RE_CONTEXT_INVALID_DUP | RE_DOT_NOT_NULL))

# define RE_SYNTAX_EGREP						\
  ((RE_SYNTAX_POSIX_EXTENDED | RE_INVALID_INTERVAL_ORD | RE_NEWLINE_ALT) \
   & ~(RE_CONTEXT_INVALID_OPS | RE_DOT_NOT_NULL))

/* POSIX grep -E behavior is no longer incompatible with GNU.  */
# define RE_SYNTAX_POSIX_EGREP						\
  RE_SYNTAX_EGREP

/* P1003.2/D11.2, section 4.20.7.1, lines 5078ff.  */
# define RE_SYNTAX_ED RE_SYNTAX_POSIX_BASIC

# define RE_SYNTAX_SED RE_SYNTAX_POSIX_BASIC

/* Syntax bits common to both basic and extended POSIX regex syntax.  */
# define _RE_SYNTAX_POSIX_COMMON					\
  (RE_CHAR_CLASSES | RE_DOT_NEWLINE      | RE_DOT_NOT_NULL		\
   | RE_INTERVALS  | RE_NO_EMPTY_RANGES)

# define RE_SYNTAX_POSIX_BASIC						\
  (_RE_SYNTAX_POSIX_COMMON | RE_BK_PLUS_QM | RE_CONTEXT_INVALID_DUP)

/* Differs from ..._POSIX_BASIC only in that RE_BK_PLUS_QM becomes
   RE_LIMITED_OPS, i.e., \? \+ \| are not recognized.  Actually, this
   isn't minimal, since other operators, such as \`, aren't disabled.  */
# define RE_SYNTAX_POSIX_MINIMAL_BASIC					\
  (_RE_SYNTAX_POSIX_COMMON | RE_LIMITED_OPS)

# define RE_SYNTAX_POSIX_EXTENDED					\
  (_RE_SYNTAX_POSIX_COMMON  | RE_CONTEXT_INDEP_ANCHORS			\
   | RE_CONTEXT_INDEP_OPS   | RE_NO_BK_BRACES				\
   | RE_NO_BK_PARENS        | RE_NO_BK_VBAR				\
   | RE_CONTEXT_INVALID_OPS | RE_UNMATCHED_RIGHT_PAREN_ORD)

/* Differs from ..._POSIX_EXTENDED in that RE_CONTEXT_INDEP_OPS is
   removed and RE_NO_BK_REFS is added.  */
# define RE_SYNTAX_POSIX_MINIMAL_EXTENDED				\
  (_RE_SYNTAX_POSIX_COMMON  | RE_CONTEXT_INDEP_ANCHORS			\
   | RE_CONTEXT_INVALID_OPS | RE_NO_BK_BRACES				\
   | RE_NO_BK_PARENS        | RE_NO_BK_REFS				\
   | RE_NO_BK_VBAR	    | RE_UNMATCHED_RIGHT_PAREN_ORD)
/* [[[end syntaxes]]] */

/* Maximum number of duplicates an interval can allow.  POSIX-conforming
   systems might define this in <limits.h>, but we want our
   value, so remove any previous define.  */
# ifdef _REGEX_INCLUDE_LIMITS_H
#  include <limits.h>
# endif
# ifdef RE_DUP_MAX
#  undef RE_DUP_MAX
# endif

/* RE_DUP_MAX is 2**15 - 1 because an earlier implementation stored
   the counter as a 2-byte signed integer.  This is no longer true, so
   RE_DUP_MAX could be increased to (INT_MAX / 10 - 1), or to
   ((SIZE_MAX - 9) / 10) if _REGEX_LARGE_OFFSETS is defined.
   However, there would be a huge performance problem if someone
   actually used a pattern like a\{214748363\}, so RE_DUP_MAX retains
   its historical value.  */
# define RE_DUP_MAX (0x7fff)

/* If any error codes are removed, changed, or added, update the
   '__re_error_msgid' table in regcomp.c.  */

typedef enum
{
  _REG_ENOSYS	= REG_ENOSYS,	/* This will never happen for this implementation.  */
  _REG_NOERROR	= 0,		/* Success.  */
  _REG_NOMATCH	= REG_NOMATCH,	/* Didn't find a match (for regexec).  */

  /* POSIX regcomp return error codes.  (In the order listed in the
     standard.)  */
  _REG_BADPAT	= REG_BADPAT,	/* Invalid pattern.  */
  _REG_ECOLLATE	= REG_ECOLLATE,	/* Invalid collating element.  */
  _REG_ECTYPE	= REG_ECTYPE,	/* Invalid character class name.  */
  _REG_EESCAPE	= REG_EESCAPE,	/* Trailing backslash.  */
  _REG_ESUBREG	= REG_ESUBREG,	/* Invalid back reference.  */
  _REG_EBRACK	= REG_EBRACK,	/* Unmatched left bracket.  */
  _REG_EPAREN	= REG_EPAREN,	/* Parenthesis imbalance.  */
  _REG_EBRACE	= REG_EBRACE,	/* Unmatched \{.  */
  _REG_BADBR	= REG_BADBR,	/* Invalid contents of \{\}.  */
  _REG_ERANGE	= REG_ERANGE,	/* Invalid range end.  */
  _REG_ESPACE	= REG_ESPACE,	/* Ran out of memory.  */
  _REG_BADRPT	= REG_BADRPT,	/* No preceding re for repetition op.  */

  /* Error codes we've added.  */
  // Torrekie: Make sure this not conflicting with existing errcode
  _REG_EEND	= 18,		/* Premature end.  */
  _REG_ESIZE	= 19,		/* Too large (e.g., repeat count too large).  */
  _REG_ERPAREN	= 20		/* Unmatched ) or \); not returned from regcomp.  */
} reg_errcode_t;

#define REG_NOERROR	_REG_NOERROR
#define REG_EEND	_REG_EEND
#define REG_ESIZE	_REG_ESIZE
#define REG_ERPAREN	_REG_ERPAREN

/* This data structure represents a compiled pattern.  Before calling
   the pattern compiler, the fields 'buffer', 'allocated', 'fastmap',
   and 'translate' can be set.  After the pattern has been compiled,
   the fields 're_nsub', 'not_bol' and 'not_eol' are available.  All
   other fields are private to the regex routines.  */

#ifndef RE_TRANSLATE_TYPE
# define __RE_TRANSLATE_TYPE unsigned char *
# define RE_TRANSLATE_TYPE __RE_TRANSLATE_TYPE
#endif

struct re_pattern_buffer
{
  /* Space that holds the compiled pattern.  The type
     'struct re_dfa_t' is private and is not declared here.  */
  struct re_dfa_t *buffer;

  /* Number of bytes to which 'buffer' points.  */
  __re_long_size_t allocated;

  /* Number of bytes actually used in 'buffer'.  */
  __re_long_size_t used;

  /* Syntax setting with which the pattern was compiled.  */
  reg_syntax_t syntax;

  /* Pointer to a fastmap, if any, otherwise zero.  re_search uses the
     fastmap, if there is one, to skip over impossible starting points
     for matches.  */
  char *fastmap;

  /* Either a translate table to apply to all characters before
     comparing them, or zero for no translation.  The translation is
     applied to a pattern when it is compiled and to a string when it
     is matched.  */
  __RE_TRANSLATE_TYPE translate;

  /* Number of subexpressions found by the compiler.  */
  size_t re_nsub;

  /* Zero if this pattern cannot match the empty string, one else.
     Well, in truth it's used only in 're_search_2', to see whether or
     not we should use the fastmap, so we don't set this absolutely
     perfectly; see 're_compile_fastmap' (the "duplicate" case).  */
  unsigned can_be_null : 1;

  /* If REGS_UNALLOCATED, allocate space in the 'regs' structure
     for 'max (RE_NREGS, re_nsub + 1)' groups.
     If REGS_REALLOCATE, reallocate space if necessary.
     If REGS_FIXED, use what's there.  */
#define REGS_UNALLOCATED 0
#define REGS_REALLOCATE 1
#define REGS_FIXED 2
  unsigned regs_allocated : 2;

  /* Set to zero when 're_compile_pattern' compiles a pattern; set to
     one by 're_compile_fastmap' if it updates the fastmap.  */
  unsigned fastmap_accurate : 1;

  /* If set, 're_match_2' does not return information about
     subexpressions.  */
  unsigned no_sub : 1;

  /* If set, a beginning-of-line anchor doesn't match at the beginning
     of the string.  */
  unsigned not_bol : 1;

  /* Similarly for an end-of-line anchor.  */
  unsigned not_eol : 1;

  /* If true, an anchor at a newline matches.  */
  unsigned newline_anchor : 1;
};



/* This is the structure we store register match data in.  See
   regex.texinfo for a full description of what registers match.  */
struct re_registers
{
  __re_size_t num_regs;
  regoff_t *start;
  regoff_t *end;
};


/* If 'regs_allocated' is REGS_UNALLOCATED in the pattern buffer,
   're_match_2' returns information about at least this many registers
   the first time a 'regs' structure is passed.  */
#ifndef RE_NREGS
# define RE_NREGS 30
#endif


/* Declarations for routines.  */

#ifndef _REGEX_NELTS
# if (defined __STDC_VERSION__ && 199901L <= __STDC_VERSION__ \
	&& !defined __STDC_NO_VLA__)
#  define _REGEX_NELTS(n) n
# else
#  define _REGEX_NELTS(n)
# endif
#endif

#ifndef _Attr_access_
# ifdef __attr_access
#  define _Attr_access_(arg) __attr_access (arg)
# elif defined __GNUC__ && 10 <= __GNUC__
#  define _Attr_access_(x) __attribute__ ((__access__ x))
# else
#  define _Attr_access_(x)
# endif
#endif

/* Sets the current default syntax to SYNTAX, and return the old syntax.
   You can also simply assign to the 're_syntax_options' variable.  */
extern reg_syntax_t re_set_syntax (reg_syntax_t __syntax);

/* Compile the regular expression PATTERN, with length LENGTH
   and syntax given by the global 're_syntax_options', into the buffer
   BUFFER.  Return NULL if successful, and an error string if not.

   To free the allocated storage, you must call 'regfree' on BUFFER.
   Note that the translate table must either have been initialized by
   'regcomp', with a malloc'ed value, or set to NULL before calling
   'regfree'.  */
extern const char *re_compile_pattern (const char *__pattern, size_t __length,
				       struct re_pattern_buffer *__buffer)
    _Attr_access_ ((__read_only__, 1, 2));


/* Compile a fastmap for the compiled pattern in BUFFER; used to
   accelerate searches.  Return 0 if successful and -2 if was an
   internal error.  */
extern int re_compile_fastmap (struct re_pattern_buffer *__buffer);


/* Search in the string STRING (with length LENGTH) for the pattern
   compiled into BUFFER.  Start searching at position START, for RANGE
   characters.  Return the starting position of the match, -1 for no
   match, or -2 for an internal error.  Also return register
   information in REGS (if REGS and BUFFER->no_sub are nonzero).  */
extern regoff_t re_search (struct re_pattern_buffer *__buffer,
			   const char *__String, regoff_t __length,
			   regoff_t __start, regoff_t __range,
			   struct re_registers *__regs)
    _Attr_access_ ((__read_only__, 2, 3));


/* Like 're_search', but search in the concatenation of STRING1 and
   STRING2.  Also, stop searching at index START + STOP.  */
extern regoff_t re_search_2 (struct re_pattern_buffer *__buffer,
			     const char *__string1, regoff_t __length1,
			     const char *__string2, regoff_t __length2,
			     regoff_t __start, regoff_t __range,
			     struct re_registers *__regs,
			     regoff_t __stop)
    _Attr_access_ ((__read_only__, 2, 3))
    _Attr_access_ ((__read_only__, 4, 5));


/* Like 're_search', but return how many characters in STRING the regexp
   in BUFFER matched, starting at position START.  */
extern regoff_t re_match (struct re_pattern_buffer *__buffer,
			  const char *__String, regoff_t __length,
			  regoff_t __start, struct re_registers *__regs)
    _Attr_access_ ((__read_only__, 2, 3));


/* Relates to 're_match' as 're_search_2' relates to 're_search'.  */
extern regoff_t re_match_2 (struct re_pattern_buffer *__buffer,
			    const char *__string1, regoff_t __length1,
			    const char *__string2, regoff_t __length2,
			    regoff_t __start, struct re_registers *__regs,
			    regoff_t __stop)
    _Attr_access_ ((__read_only__, 2, 3))
    _Attr_access_ ((__read_only__, 4, 5));


/* Set REGS to hold NUM_REGS registers, storing them in STARTS and
   ENDS.  Subsequent matches using BUFFER and REGS will use this memory
   for recording register information.  STARTS and ENDS must be
   allocated with malloc, and must each be at least 'NUM_REGS * sizeof
   (regoff_t)' bytes long.

   If NUM_REGS == 0, then subsequent matches should allocate their own
   register data.

   Unless this function is called, the first search or match using
   BUFFER will allocate its own register data, without
   freeing the old data.  */
extern void re_set_registers (struct re_pattern_buffer *__buffer,
			      struct re_registers *__regs,
			      __re_size_t __num_regs,
			      regoff_t *__starts, regoff_t *__ends);

#if defined _REGEX_RE_COMP
/* 4.2 bsd compatibility.  */
extern char *re_comp (const char *);
extern int re_exec (const char *);
#endif

/* For plain 'restrict', use glibc's __restrict if defined.
   Otherwise, GCC 2.95 and later have "__restrict"; C99 compilers have
   "restrict", and "configure" may have defined "restrict".
   Other compilers use __restrict, __restrict__, and _Restrict, and
   'configure' might #define 'restrict' to those words, so pick a
   different name.  */
#ifndef _Restrict_
# if defined __restrict \
     || 2 < __GNUC__ + (95 <= __GNUC_MINOR__) \
     || __clang_major__ >= 3
#  define _Restrict_ __restrict
# elif 199901L <= __STDC_VERSION__ || defined restrict
#  define _Restrict_ restrict
# else
#  define _Restrict_
# endif
#endif
/* For the ISO C99 syntax
     array_name[restrict]
   use glibc's __restrict_arr if available.
   Otherwise, GCC 3.1 and clang support this syntax (but not in C++ mode).
   Other ISO C99 compilers support it as well.  */
#ifndef _Restrict_arr_
# ifdef __restrict_arr
#  define _Restrict_arr_ __restrict_arr
# elif ((199901L <= __STDC_VERSION__ \
         || 3 < __GNUC__ + (1 <= __GNUC_MINOR__) \
         || __clang_major__ >= 3) \
        && !defined __cplusplus)
#  define _Restrict_arr_ _Restrict_
# else
#  define _Restrict_arr_
# endif
#endif

#ifdef __cplusplus
}
#endif	/* C++ */

#endif /* _LIBRECOMPAT_REGEX_H_ */