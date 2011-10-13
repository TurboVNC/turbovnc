/* ftstdlib.h -- modified for XFree86. */
/* $XFree86: xc/lib/font/FreeType/ftstdlib.h,v 1.5 2003/02/22 06:00:36 dawes Exp $ */
#ifndef __MYFTSTDLIB_H__
#define __MYFTSTDLIB_H__


#ifndef FONTMODULE

# include <ftstdlib.h>
# ifndef ft_isdigit
#  define ft_isdigit isdigit
# endif

#else

#ifndef __FTSTDLIB_H__
#define __FTSTDLIB_H__
/* we don't include limits.h */
#define CHAR_BIT 8

#include "Xmd.h"
#define _XTYPEDEF_BOOL
#include "Xdefs.h"
#define DONT_DEFINE_WRAPPERS
#define DEFINE_SETJMP_WRAPPERS
#include "xf86_ansic.h"
#undef DONT_DEFINE_WRAPPERS

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((xf86size_t)&((TYPE*)0)->MEMBER)
#endif

#define FT_UINT_MAX   4294967295U
#ifdef LONG64
#define FT_ULONG_MAX 18446744073709551615UL
#else
#define FT_ULONG_MAX  4294967295UL
#endif

#define ft_isalnum  xf86isalnum
#define ft_isupper  xf86isupper
#define ft_islower  xf86islower
#define ft_isxdigit xf86isxdigit
/* works around a bug in freetype 2.1.8 */
#ifndef isdigit
#define isdigit xf86isdigit
#endif
#define ft_isdigit  xf86isdigit

#define ft_strlen   xf86strlen
#define ft_strcat   xf86strcat
#define ft_strrchr  xf86strrchr
#define ft_strcmp   xf86strcmp
#define ft_strncmp  xf86strncmp
#define ft_memcpy   xf86memcpy
#define ft_strcpy   xf86strcpy
#define ft_strncpy  xf86strncpy
#define ft_memset   xf86memset
#define ft_memmove  xf86memmove
#define ft_memcmp   xf86memcmp

#define ft_sprintf  xf86sprintf

#define ft_qsort  xf86qsort
#define ft_exit   xf86exit

#define ft_atoi   xf86atoi

#define ft_jmp_buf  jmp_buf
#define ft_setjmp   setjmp
#define ft_longjmp  longjmp

#endif /* __FTSTDLIB_H__ */
#endif /* FONTMODULE */


#include <stdarg.h>

#endif /* __MYFTSTDLIB_H__ */


/* END */
