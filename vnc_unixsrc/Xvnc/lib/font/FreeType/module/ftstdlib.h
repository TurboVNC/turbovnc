/* ftstdlib.h -- modified for XFree86. */
/* $XFree86: xc/lib/font/FreeType/ftstdlib.h,v 1.5 2003/02/22 06:00:36 dawes Exp $ */

#ifndef __FTSTDLIB_H__
#define __FTSTDLIB_H__

#ifndef FONTMODULE

#include <limits.h>

#define FT_UINT_MAX   UINT_MAX
#define FT_ULONG_MAX  ULONG_MAX

#include <ctype.h>

#define ft_isalnum   isalnum
#define ft_isupper   isupper
#define ft_islower   islower
#define ft_isdigit   isdigit
#define ft_isxdigit  isxdigit


#include <string.h>

#define ft_strlen   strlen
#define ft_strcat   strcat
#define ft_strcmp   strcmp
#define ft_strncmp  strncmp
#define ft_memcpy   memcpy
#define ft_strcpy   strcpy
#define ft_strncpy  strncpy
#define ft_memset   memset
#define ft_memmove  memmove
#define ft_memcmp   memcmp

#include <stdio.h>

#define ft_sprintf  sprintf

#include <stdlib.h>

#define ft_qsort  qsort
#define ft_exit   exit

#define ft_atoi   atoi

#include <setjmp.h>

#define ft_jmp_buf  jmp_buf   /* note: this cannot be a typedef since */
                              /*       jmp_buf is defined as a macro  */
                              /*       on certain platforms           */

#define ft_setjmp   setjmp    /* same thing here */
#define ft_longjmp  longjmp   /* "               */


#else

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
#define ft_xdigit   xf86isxdigit

#define ft_strlen   xf86strlen
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

#endif /* FONTMODULE */


#include <stdarg.h>


#endif /* __FTSTDLIB_H__ */


/* END */
