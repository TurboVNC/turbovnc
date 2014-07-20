#include <stdio.h>
#include <X11/fonts/fntfilst.h>
#include <X11/fonts/font.h>

#ifndef True
#define True (-1)
#endif
#ifndef False
#define False (0)
#endif

/* this probably works for Mach-O too, but probably not for PE */
#if (defined(__APPLE__) || defined(__ELF__)) && defined(__GNUC__) && (__GNUC__ >= 3)
#define weak __attribute__((weak))
#else
#define weak
#ifndef __SUNPRO_C /* Sun compilers use #pragma weak in .c files instead */
#define NO_WEAK_SYMBOLS
#endif
#endif

/* This is really just a hack for now... __APPLE__ really should be using
 * the weak symbols route above, but it's causing an as-yet unresolved issue,
 * so we're instead building with flat_namespace.
 */
#ifdef __APPLE__
#undef weak
#define weak
#endif

extern FontPtr find_old_font ( FSID id );
extern int set_font_authorizations ( char **authorizations,
				     int *authlen,
				     ClientPtr client );

extern unsigned long GetTimeInMillis (void);

extern void ErrorF(const char *format, ...);
extern void FatalError(const char *format, ...);

/* end of file */
