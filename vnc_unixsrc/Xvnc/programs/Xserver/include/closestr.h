/* $XConsortium: closestr.h,v 1.10 95/05/19 19:18:55 dpw Exp $ */
/* $XFree86: xc/programs/Xserver/include/closestr.h,v 3.0 1996/04/15 11:34:23 dawes Exp $ */
/*

Copyright (c) 1991  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/


#ifndef CLOSESTR_H
#define CLOSESTR_H

#define	NEED_REPLIES
#include "Xproto.h"
#include "closure.h"
#include "dix.h"
#include "misc.h"
#include "gcstruct.h"

/* closure structures */

/* OpenFont */

typedef struct _OFclosure {
    ClientPtr   client;
    short       current_fpe;
    short       num_fpes;
    FontPathElementPtr *fpe_list;
    Mask        flags;
    Bool        slept;

/* XXX -- get these from request buffer instead? */
    char       *origFontName;
    int		origFontNameLen;
    XID         fontid;
    char       *fontname;
    int         fnamelen;
    FontPtr	non_cachable_font;
}           OFclosureRec;

/* ListFontsWithInfo */

typedef struct _LFWIstate {
    char	pattern[256];  /* max len of font name */
    int		patlen;
    int		current_fpe;
    int		max_names;
    Bool	list_started;
    pointer	private;
} LFWIstateRec, *LFWIstatePtr;

typedef struct _LFWIclosure {
    ClientPtr		client;
    int			num_fpes;
    FontPathElementPtr	*fpe_list;
    xListFontsWithInfoReply *reply;
    int			length;
    LFWIstateRec	current;
    LFWIstateRec	saved;
    int			savedNumFonts;
    Bool		haveSaved;
    Bool		slept;
    char		*savedName;
} LFWIclosureRec;

/* ListFonts */

typedef struct _LFclosure {
    ClientPtr   client;
    int         num_fpes;
    FontPathElementPtr *fpe_list;
    FontNamesPtr names;
    LFWIstateRec current;
    LFWIstateRec saved;
    Bool        haveSaved;
    Bool        slept;
    char	*savedName;
    int		savedNameLen;
}	LFclosureRec;

/* PolyText */

typedef
    int			(* PolyTextPtr)(
#if NeedNestedPrototypes
			DrawablePtr /* pDraw */,
			GCPtr /* pGC */,
			int /* x */,
			int /* y */,
			int /* count */,
			void * /* chars or shorts */
#endif
			);

typedef struct _PTclosure {
    ClientPtr		client;
    DrawablePtr		pDraw;
    GC			*pGC;
    unsigned char	*pElt;
    unsigned char	*endReq;
    unsigned char	*data;
    int			xorg;
    int			yorg;
    CARD8		reqType;
    PolyTextPtr		polyText;
    int			itemSize;
    XID			did;
    int			err;
    Bool		slept;
} PTclosureRec;

/* ImageText */

typedef
    void		(* ImageTextPtr)(
#if NeedNestedPrototypes
			DrawablePtr /* pDraw */,
			GCPtr /* pGC */,
			int /* x */,
			int /* y */,
			int /* count */,
			void * /* chars or shorts */
#endif
			);

typedef struct _ITclosure {
    ClientPtr		client;
    DrawablePtr		pDraw;
    GC			*pGC;
    BYTE		nChars;
    unsigned char	*data;
    int			xorg;
    int			yorg;
    CARD8		reqType;
    ImageTextPtr	imageText;
    int			itemSize;
    XID			did;
    Bool		slept;
} ITclosureRec;
#endif				/* CLOSESTR_H */
