/* $XConsortium: resource.h /main/23 1996/10/30 11:18:23 rws $ */
/***********************************************************

Copyright (c) 1987, 1989  X Consortium

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


Copyright 1987, 1989 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
#ifndef RESOURCE_H
#define RESOURCE_H 1
#include "misc.h"

/*****************************************************************
 * STUFF FOR RESOURCES 
 *****************************************************************/

/* classes for Resource routines */

typedef unsigned long RESTYPE;

#define RC_VANILLA	((RESTYPE)0)
#define RC_CACHED	((RESTYPE)1<<31)
#define RC_DRAWABLE	((RESTYPE)1<<30)
/*  Use class RC_NEVERRETAIN for resources that should not be retained
 *  regardless of the close down mode when the client dies.  (A client's
 *  event selections on objects that it doesn't own are good candidates.)
 *  Extensions can use this too!
 */
#define RC_NEVERRETAIN	((RESTYPE)1<<29)
#define RC_LASTPREDEF	RC_NEVERRETAIN
#define RC_ANY		(~(RESTYPE)0)

/* types for Resource routines */

#define RT_WINDOW	((RESTYPE)1|RC_CACHED|RC_DRAWABLE)
#define RT_PIXMAP	((RESTYPE)2|RC_CACHED|RC_DRAWABLE)
#define RT_GC		((RESTYPE)3|RC_CACHED)
#define RT_FONT		((RESTYPE)4)
#define RT_CURSOR	((RESTYPE)5)
#define RT_COLORMAP	((RESTYPE)6)
#define RT_CMAPENTRY	((RESTYPE)7)
#define RT_OTHERCLIENT	((RESTYPE)8|RC_NEVERRETAIN)
#define RT_PASSIVEGRAB	((RESTYPE)9|RC_NEVERRETAIN)
#define RT_LASTPREDEF	((RESTYPE)9)
#define RT_NONE		((RESTYPE)0)

/* bits and fields within a resource id */
#define CLIENTOFFSET 22					/* client field */
#define RESOURCE_ID_MASK	0x3FFFFF		/* low 22 bits */
#define CLIENT_BITS(id) ((id) & 0x1fc00000)		/* hi 7 bits */
#define CLIENT_ID(id) ((int)(CLIENT_BITS(id) >> CLIENTOFFSET))
#define SERVER_BIT		0x20000000		/* use illegal bit */

#ifdef INVALID
#undef INVALID	/* needed on HP/UX */
#endif

/* Invalid resource id */
#define INVALID	(0)

#define BAD_RESOURCE 0xe0000000

typedef int (*DeleteType)(
#if NeedNestedPrototypes
    pointer /*value*/,
    XID /*id*/
#endif
);

typedef void (*FindResType)(
#if NeedNestedPrototypes
    pointer /*value*/,
    XID /*id*/,
    pointer /*cdata*/
#endif
);

extern RESTYPE CreateNewResourceType(
#if NeedFunctionPrototypes
    DeleteType /*deleteFunc*/
#endif
);

extern RESTYPE CreateNewResourceClass(
#if NeedFunctionPrototypes
void
#endif
);

extern Bool InitClientResources(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern XID FakeClientID(
#if NeedFunctionPrototypes
    int /*client*/
#endif
);

extern Bool AddResource(
#if NeedFunctionPrototypes
    XID /*id*/,
    RESTYPE /*type*/,
    pointer /*value*/
#endif
);

extern void FreeResource(
#if NeedFunctionPrototypes
    XID /*id*/,
    RESTYPE /*skipDeleteFuncType*/
#endif
);

extern void FreeResourceByType(
#if NeedFunctionPrototypes
    XID /*id*/,
    RESTYPE /*type*/,
    Bool /*skipFree*/
#endif
);

extern Bool ChangeResourceValue(
#if NeedFunctionPrototypes
    XID /*id*/,
    RESTYPE /*rtype*/,
    pointer /*value*/
#endif
);

extern void FindClientResourcesByType(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    RESTYPE /*type*/,
    FindResType /*func*/,
    pointer /*cdata*/
#endif
);

extern void FreeClientNeverRetainResources(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern void FreeClientResources(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern void FreeAllResources(
#if NeedFunctionPrototypes
void
#endif
);

extern Bool LegalNewID(
#if NeedFunctionPrototypes
    XID /*id*/,
    ClientPtr /*client*/
#endif
);

extern pointer LookupIDByType(
#if NeedFunctionPrototypes
    XID /*id*/,
    RESTYPE /*rtype*/
#endif
);

extern pointer LookupIDByClass(
#if NeedFunctionPrototypes
    XID /*id*/,
    RESTYPE /*classes*/
#endif
);

/* These are the access modes that can be passed in the last parameter
 * to SecurityLookupIDByType/Class.  The Security extension doesn't
 * currently make much use of these; they're mainly provided as an
 * example of what you might need for discretionary access control.
 * You can or these values together to indicate multiple modes
 * simultaneously.
 */

#define SecurityUnknownAccess	0	/* don't know intentions */
#define SecurityReadAccess	(1<<0)	/* inspecting the object */
#define SecurityWriteAccess	(1<<1)	/* changing the object */
#define SecurityDestroyAccess	(1<<2)	/* destroying the object */

#ifdef XCSECURITY

extern pointer SecurityLookupIDByType(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    XID /*id*/,
    RESTYPE /*rtype*/,
    Mask /*access_mode*/
#endif
);

extern pointer SecurityLookupIDByClass(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    XID /*id*/,
    RESTYPE /*classes*/,
    Mask /*access_mode*/
#endif
);

#else /* not XCSECURITY */

#define SecurityLookupIDByType(client, id, rtype, access_mode) \
        LookupIDByType(id, rtype)

#define SecurityLookupIDByClass(client, id, classes, access_mode) \
        LookupIDByClass(id, classes)

#endif /* XCSECURITY */

extern void GetXIDRange(
#if NeedFunctionPrototypes
    int /*client*/,
    Bool /*server*/,
    XID * /*minp*/,
    XID * /*maxp*/
#endif
);

extern unsigned int GetXIDList(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    unsigned int /*count*/,
    XID * /*pids*/
#endif
);

#endif /* RESOURCE_H */

