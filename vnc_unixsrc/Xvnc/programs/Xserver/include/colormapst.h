/*

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

*/
/* $XConsortium: colormapst.h /main/4 1996/06/17 10:54:31 mor $ */
#ifndef CMAPSTRUCT_H
#define CMAPSTRUCT_H 1

#include "colormap.h"
#include "screenint.h"

/* Shared color -- the color is used by AllocColorPlanes */
typedef struct
{
    unsigned short color;
    short  refcnt;
} SHAREDCOLOR;

/* LOCO -- a local color for a PseudoColor cell. DirectColor maps always
 * use the first value (called red) in the structure.  What channel they
 * are really talking about depends on which map they are in. */
typedef struct
{
    unsigned short	red, green, blue;
} LOCO;

/* SHCO -- a shared color for a PseudoColor cell. Used with AllocColorPlanes.
 * DirectColor maps always use the first value (called red) in the structure.
 * What channel they are really talking about depends on which map they
 * are in. */
typedef struct 
{
    SHAREDCOLOR *red, *green, *blue;
} SHCO;


/* color map entry */
typedef struct _CMEntry
{
    union
    {
	LOCO	local;
	SHCO	shco;
    } co;
    short	refcnt;
    Bool	fShared;
} Entry;

/* COLORMAPs can be used for either Direct or Pseudo color.  PseudoColor
 * only needs one cell table, we arbitrarily pick red.  We keep track
 * of that table with freeRed, numPixelsRed, and clientPixelsRed */

typedef struct _ColormapRec
{
    VisualPtr	pVisual;
    short	class;		/* PseudoColor or DirectColor */
    long	mid;		/* client's name for colormap */
    ScreenPtr	pScreen;	/* screen map is associated with */
    short	flags;		/* 1 = IsDefault
				 * 2 = AllAllocated */
    int		freeRed;
    int		freeGreen;
    int		freeBlue;
    int		*numPixelsRed;	
    int		*numPixelsGreen;	
    int		*numPixelsBlue;	
    Pixel	**clientPixelsRed;
    Pixel	**clientPixelsGreen;
    Pixel	**clientPixelsBlue;
    Entry	*red;
    Entry 	*green;
    Entry	*blue;
    pointer	devPriv;
    DevUnion	*devPrivates;	/* dynamic devPrivates added after devPriv
				   already existed - must keep devPriv */
} ColormapRec;
	      
#endif /* COLORMAP_H */
