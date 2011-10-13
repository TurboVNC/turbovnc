/* $XFree86: xc/programs/Xserver/mfb/mfbclip.c,v 1.5 2001/12/14 20:00:05 dawes Exp $ */
/***********************************************************

Copyright 1987, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.


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
/* $Xorg: mfbclip.c,v 1.4 2001/02/09 02:05:18 xorgcvs Exp $ */
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include "regionstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "regionstr.h"
#include "gc.h"
#include "maskbits.h"
#include "mi.h"
#include "mfb.h"

#define ADDRECT(reg,r,fr,rx1,ry1,rx2,ry2)			\
if (((rx1) < (rx2)) && ((ry1) < (ry2)) &&			\
    (!((reg)->data->numRects &&					\
       ((r-1)->y1 == (ry1)) &&					\
       ((r-1)->y2 == (ry2)) &&					\
       ((r-1)->x1 <= (rx1)) &&					\
       ((r-1)->x2 >= (rx2)))))					\
{								\
    if ((reg)->data->numRects == (reg)->data->size)		\
    {								\
	miRectAlloc(reg, 1);					\
	fr = REGION_BOXPTR(reg);				\
	r = fr + (reg)->data->numRects;				\
    }								\
    r->x1 = (rx1);						\
    r->y1 = (ry1);						\
    r->x2 = (rx2);						\
    r->y2 = (ry2);						\
    (reg)->data->numRects++;					\
    if(r->x1 < (reg)->extents.x1)				\
	(reg)->extents.x1 = r->x1;				\
    if(r->x2 > (reg)->extents.x2)				\
	(reg)->extents.x2 = r->x2;				\
    r++;							\
}

/* Convert bitmap clip mask into clipping region. 
 * First, goes through each line and makes boxes by noting the transitions
 * from 0 to 1 and 1 to 0.
 * Then it coalesces the current line with the previous if they have boxes
 * at the same X coordinates.
 */
RegionPtr
mfbPixmapToRegion(pPix)
    PixmapPtr	pPix;
{
    register RegionPtr	pReg;
    register PixelType	*pw, w;
    register int	ib;
    int			width, h, base, rx1 = 0, crects;
    PixelType		*pwLineEnd;
    int			irectPrevStart, irectLineStart;
    register BoxPtr	prectO, prectN;
    BoxPtr		FirstRect, rects, prectLineStart;
    Bool		fInBox, fSame;
    register PixelType	mask0 = mask[0];
    PixelType		*pwLine;
    int			nWidth;

    pReg = REGION_CREATE(pPix->drawable.pScreen, NULL, 1);
    if(!pReg)
	return NullRegion;
    FirstRect = REGION_BOXPTR(pReg);
    rects = FirstRect;

    pwLine = (PixelType *) pPix->devPrivate.ptr;
    nWidth = pPix->devKind / PGSZB;

    width = pPix->drawable.width;
    pReg->extents.x1 = width - 1;
    pReg->extents.x2 = 0;
    irectPrevStart = -1;
    for(h = 0; h < pPix->drawable.height; h++)
    {
	pw = pwLine;
	pwLine += nWidth;
	irectLineStart = rects - FirstRect;
	/* If the Screen left most bit of the word is set, we're starting in
	 * a box */
	if(*pw & mask0)
	{
	    fInBox = TRUE;
	    rx1 = 0;
	}
	else
	    fInBox = FALSE;
	/* Process all words which are fully in the pixmap */
	pwLineEnd = pw + (width >> PWSH);
	for (base = 0; pw < pwLineEnd; base += PPW)
	{
	    w = *pw++;
	    if (fInBox)
	    {
		if (!~w)
		    continue;
	    }
	    else
	    {
		if (!w)
		    continue;
	    }
	    for(ib = 0; ib < PPW; ib++)
	    {
	        /* If the Screen left most bit of the word is set, we're
		 * starting a box */
		if(w & mask0)
		{
		    if(!fInBox)
		    {
			rx1 = base + ib;
			/* start new box */
			fInBox = TRUE;
		    }
		}
		else
		{
		    if(fInBox)
		    {
			/* end box */
			ADDRECT(pReg, rects, FirstRect,
				rx1, h, base + ib, h + 1);
			fInBox = FALSE;
		    }
		}
		/* Shift the word VISUALLY left one. */
		w = SCRLEFT(w, 1);
	    }
	}
	if(width & PIM)
	{
	    /* Process final partial word on line */
	    w = *pw++;
	    for(ib = 0; ib < (width & PIM); ib++)
	    {
	        /* If the Screen left most bit of the word is set, we're
		 * starting a box */
		if(w & mask0)
		{
		    if(!fInBox)
		    {
			rx1 = base + ib;
			/* start new box */
			fInBox = TRUE;
		    }
		}
		else
		{
		    if(fInBox)
		    {
			/* end box */
			ADDRECT(pReg, rects, FirstRect,
				rx1, h, base + ib, h + 1);
			fInBox = FALSE;
		    }
		}
		/* Shift the word VISUALLY left one. */
		w = SCRLEFT(w, 1);
	    }
	}
	/* If scanline ended with last bit set, end the box */
	if(fInBox)
	{
	    ADDRECT(pReg, rects, FirstRect,
		    rx1, h, base + (width & PIM), h + 1);
	}
	/* if all rectangles on this line have the same x-coords as
	 * those on the previous line, then add 1 to all the previous  y2s and 
	 * throw away all the rectangles from this line 
	 */
	fSame = FALSE;
	if(irectPrevStart != -1)
	{
	    crects = irectLineStart - irectPrevStart;
	    if(crects == ((rects - FirstRect) - irectLineStart))
	    {
	        prectO = FirstRect + irectPrevStart;
	        prectN = prectLineStart = FirstRect + irectLineStart;
		fSame = TRUE;
	        while(prectO < prectLineStart)
		{
		    if((prectO->x1 != prectN->x1) || (prectO->x2 != prectN->x2))
		    {
			  fSame = FALSE;
			  break;
		    }
		    prectO++;
		    prectN++;
		}
		if (fSame)
		{
		    prectO = FirstRect + irectPrevStart;
		    while(prectO < prectLineStart)
		    {
			prectO->y2 += 1;
			prectO++;
		    }
		    rects -= crects;
		    pReg->data->numRects -= crects;
		}
	    }
	}
	if(!fSame)
	    irectPrevStart = irectLineStart;
    }
    if (!pReg->data->numRects)
	pReg->extents.x1 = pReg->extents.x2 = 0;
    else
    {
	pReg->extents.y1 = REGION_BOXPTR(pReg)->y1;
	pReg->extents.y2 = REGION_END(pReg)->y2;
	if (pReg->data->numRects == 1)
	{
	    xfree(pReg->data);
	    pReg->data = (RegDataPtr)NULL;
	}
    }
#ifdef DEBUG
    if (!miValidRegion(pReg))
	FatalError("Assertion failed file %s, line %d: expr\n", __FILE__, __LINE__);
#endif
    return(pReg);
}

mfbPixmapToRegionProc *
mfbPixmapToRegionWeak(void)
{
    return mfbPixmapToRegion;
}
