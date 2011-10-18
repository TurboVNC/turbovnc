/* $XConsortium: Xauth.h /main/18 1996/07/31 16:08:18 dpw $ */

/*

Copyright (c) 1988  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

*/

#ifndef _Xauth_h
#define _Xauth_h

typedef struct xauth {
    unsigned short   family;
    unsigned short   address_length;
    char    	    *address;
    unsigned short   number_length;
    char    	    *number;
    unsigned short   name_length;
    char    	    *name;
    unsigned short   data_length;
    char   	    *data;
} Xauth;

#ifndef _XAUTH_STRUCT_ONLY

# include   <X11/Xfuncproto.h>
# include   <X11/Xfuncs.h>

# include   <stdio.h>

# define FamilyLocal (256)	/* not part of X standard (i.e. X.h) */
# define FamilyWild  (65535)
# define FamilyNetname    (254)   /* not part of X standard */
# define FamilyKrb5Principal (253) /* Kerberos 5 principal name */
# define FamilyLocalHost (252)	/* for local non-net authentication */


_XFUNCPROTOBEGIN

char *XauFileName();

Xauth *XauReadAuth(
#if NeedFunctionPrototypes
FILE*	/* auth_file */
#endif
);

int XauLockAuth(
#if NeedFunctionPrototypes
_Xconst char*	/* file_name */,
int		/* retries */,
int		/* timeout */,
long		/* dead */
#endif
);

int XauUnlockAuth(
#if NeedFunctionPrototypes
_Xconst char*	/* file_name */
#endif
);

int XauWriteAuth(
#if NeedFunctionPrototypes
FILE*		/* auth_file */,
Xauth*		/* auth */
#endif
);

Xauth *XauGetAuthByName(
#if NeedFunctionPrototypes
_Xconst char*	/* display_name */
#endif
);

Xauth *XauGetAuthByAddr(
#if NeedFunctionPrototypes
#if NeedWidePrototypes
unsigned int	/* family */,
unsigned int	/* address_length */,
#else
unsigned short	/* family */,
unsigned short	/* address_length */,
#endif
_Xconst char*	/* address */,
#if NeedWidePrototypes
unsigned int	/* number_length */,
#else
unsigned short	/* number_length */,
#endif
_Xconst char*	/* number */,
#if NeedWidePrototypes
unsigned int	/* name_length */,
#else
unsigned short	/* name_length */,
#endif
_Xconst char*	/* name */
#endif
);

Xauth *XauGetBestAuthByAddr(
#if NeedFunctionPrototypes
#if NeedWidePrototypes
unsigned int	/* family */,
unsigned int	/* address_length */,
#else
unsigned short	/* family */,
unsigned short	/* address_length */,
#endif
_Xconst char*	/* address */,
#if NeedWidePrototypes
unsigned int	/* number_length */,
#else
unsigned short	/* number_length */,
#endif
_Xconst char*	/* number */,
int		/* types_length */,
char**		/* type_names */,
_Xconst int*	/* type_lengths */
#endif
);

void XauDisposeAuth(
#if NeedFunctionPrototypes
Xauth*		/* auth */
#endif
);

#ifdef K5AUTH
#include <krb5/krb5.h>
/* 9/93: krb5.h leaks some symbols */
#undef BITS32
#undef xfree

int XauKrb5Encode(
#if NeedFunctionPrototypes
     krb5_principal	/* princ */,
     krb5_data *	/* outbuf */
#endif
);

int XauKrb5Decode(
#if NeedFunctionPrototypes
     krb5_data		/* inbuf */,
     krb5_principal *	/* princ */
#endif
);
#endif /* K5AUTH */

_XFUNCPROTOEND

/* Return values from XauLockAuth */

# define LOCK_SUCCESS	0	/* lock succeeded */
# define LOCK_ERROR	1	/* lock unexpectely failed, check errno */
# define LOCK_TIMEOUT	2	/* lock failed, timeouts expired */

#endif /* _XAUTH_STRUCT_ONLY */

#endif /* _Xauth_h */
