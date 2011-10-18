/* $XConsortium: extnsionst.h /main/15 1996/08/01 19:18:11 dpw $ */
/* $XFree86: xc/programs/Xserver/include/extnsionst.h,v 3.2 1996/12/23 07:09:27 dawes Exp $ */
/***********************************************************

Copyright (c) 1987  X Consortium

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


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

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
#ifndef EXTENSIONSTRUCT_H
#define EXTENSIONSTRUCT_H 

#include "misc.h"
#include "screenint.h"
#include "extension.h"
#include "gc.h"

typedef struct _ExtensionEntry {
    int index;
    void (* CloseDown)(	/* called at server shutdown */
#if NeedNestedPrototypes
	struct _ExtensionEntry * /* extension */
#endif
);
    char *name;               /* extension name */
    int base;                 /* base request number */
    int eventBase;            
    int eventLast;
    int errorBase;
    int errorLast;
    int num_aliases;
    char **aliases;
    pointer extPrivate;
    unsigned short (* MinorOpcode)(	/* called for errors */
#if NeedNestedPrototypes
	ClientPtr /* client */
#endif
);
#ifdef XCSECURITY
    Bool secure;		/* extension visible to untrusted clients? */
#endif
} ExtensionEntry;

/* any attempt to declare the types of the parameters to the functions
 * in EventSwapVector fails.  The functions take pointers to two events,
 * but the exact event types that are declared vary from one function 
 * to another.  You can't even put void *, void * (the ibm compiler
 * complains, anyway).
 */
typedef void (*EventSwapPtr) (
#if NeedFunctionPrototypes && defined(EVENT_SWAP_PTR)
	xEvent *,
	xEvent *
#endif
);

extern EventSwapPtr EventSwapVector[128];

extern void NotImplemented (	/* FIXME: this may move to another file... */
#if NeedFunctionPrototypes && defined(EVENT_SWAP_PTR)
	xEvent *,
	xEvent *
#endif
);

typedef void (* ExtensionLookupProc)(	/*args indeterminate*/
#ifdef	EXTENSION_PROC_ARGS
	EXTENSION_PROC_ARGS
#endif
);

typedef struct _ProcEntry {
    char *name;
    ExtensionLookupProc proc;
} ProcEntryRec, *ProcEntryPtr;

typedef struct _ScreenProcEntry {
    int num;
    ProcEntryPtr procList;
} ScreenProcEntry;

#define    SetGCVector(pGC, VectorElement, NewRoutineAddress, Atom)    \
    pGC->VectorElement = NewRoutineAddress;

#define    GetGCValue(pGC, GCElement)    (pGC->GCElement)


extern ExtensionEntry *AddExtension(
#if NeedFunctionPrototypes
    char* /*name*/,
    int /*NumEvents*/,
    int /*NumErrors*/,
    int (* /*MainProc*/)(
#if NeedNestedPrototypes
	ClientPtr /*client*/
#endif
),
    int (* /*SwappedMainProc*/)(
#if NeedNestedPrototypes
	ClientPtr /*client*/
#endif
),
    void (* /*CloseDownProc*/)(
#if NeedNestedPrototypes
	ExtensionEntry * /*extension*/
#endif
),
    unsigned short (* /*MinorOpcodeProc*/)(
#if NeedNestedPrototypes
	ClientPtr /*client*/
#endif
	)
#endif /* NeedFunctionPrototypes */
);

extern Bool AddExtensionAlias(
#if NeedFunctionPrototypes
    char* /*alias*/,
    ExtensionEntry * /*extension*/
#endif
);

extern ExtensionLookupProc LookupProc(
#if NeedFunctionPrototypes
    char* /*name*/,
    GCPtr /*pGC*/
#endif
);

extern Bool RegisterProc(
#if NeedFunctionPrototypes
    char* /*name*/,
    GCPtr /*pGC*/,
    ExtensionLookupProc /*proc*/
#endif
);

extern Bool RegisterScreenProc(
#if NeedFunctionPrototypes
    char* /*name*/,
    ScreenPtr /*pScreen*/,
    ExtensionLookupProc /*proc*/
#endif
);

extern void DeclareExtensionSecurity(
#if NeedFunctionPrototypes
    char * /*extname*/,
    Bool /*secure*/
#endif
);

#endif /* EXTENSIONSTRUCT_H */

