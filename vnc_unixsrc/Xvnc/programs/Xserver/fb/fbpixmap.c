/*
 * Id: fbpixmap.c,v 1.1 1999/11/02 03:54:45 keithp Exp $
 *
 * Copyright © 1998 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
/* $XFree86: xc/programs/Xserver/fb/fbpixmap.c,v 1.11 2002/09/16 18:05:34 eich Exp $ */

#include "fb.h"
#ifdef IN_MODULE
#include "xf86_ansic.h"
#endif

PixmapPtr
fbCreatePixmapBpp (ScreenPtr pScreen, int width, int height, int depth, int bpp)
{
    PixmapPtr	pPixmap;
    int		datasize;
    int		paddedWidth;
    int		adjust;
    int		base;

    paddedWidth = ((width * bpp + FB_MASK) >> FB_SHIFT) * sizeof (FbBits);
    datasize = height * paddedWidth;
#ifdef PIXPRIV
    base = pScreen->totalPixmapSize;
#else
    base = sizeof (PixmapRec);
#endif
    adjust = 0;
    if (base & 7)
	adjust = 8 - (base & 7);
    datasize += adjust;
#ifdef FB_DEBUG
    datasize += 2 * paddedWidth;
#endif
    pPixmap = AllocatePixmap(pScreen, datasize);
    if (!pPixmap)
	return NullPixmap;
    pPixmap->drawable.type = DRAWABLE_PIXMAP;
    pPixmap->drawable.class = 0;
    pPixmap->drawable.pScreen = pScreen;
    pPixmap->drawable.depth = depth;
    pPixmap->drawable.bitsPerPixel = bpp;
    pPixmap->drawable.id = 0;
    pPixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
    pPixmap->drawable.x = 0;
    pPixmap->drawable.y = 0;
    pPixmap->drawable.width = width;
    pPixmap->drawable.height = height;
    pPixmap->devKind = paddedWidth;
    pPixmap->refcnt = 1;
    pPixmap->devPrivate.ptr = (pointer) ((char *)pPixmap + base + adjust);
#ifdef FB_DEBUG
    pPixmap->devPrivate.ptr = (void *) ((char *) pPixmap->devPrivate.ptr + paddedWidth);
    fbInitializeDrawable (&pPixmap->drawable);
#endif

    return pPixmap;
}

PixmapPtr
fbCreatePixmap (ScreenPtr pScreen, int width, int height, int depth)
{
    int	bpp;
    bpp = BitsPerPixel (depth);
#ifdef FB_SCREEN_PRIVATE
    if (bpp == 32 && depth <= 24)
	bpp = fbGetScreenPrivate(pScreen)->pix32bpp;
#endif
    return fbCreatePixmapBpp (pScreen, width, height, depth, bpp);
}

Bool
fbDestroyPixmap (PixmapPtr pPixmap)
{
    if(--pPixmap->refcnt)
	return TRUE;
    xfree(pPixmap);
    return TRUE;
}

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
fbPixmapToRegion(PixmapPtr pPix)
{
    register RegionPtr	pReg;
    FbBits		*pw, w;
    register int	ib;
    int			width, h, base, rx1 = 0, crects;
    FbBits		*pwLineEnd;
    int			irectPrevStart, irectLineStart;
    register BoxPtr	prectO, prectN;
    BoxPtr		FirstRect, rects, prectLineStart;
    Bool		fInBox, fSame;
    register FbBits	mask0 = FB_ALLONES & ~FbScrRight(FB_ALLONES, 1);
    FbBits		*pwLine;
    int			nWidth;
    
    pReg = REGION_CREATE(pPix->drawable.pScreen, NULL, 1);
    if(!pReg)
	return NullRegion;
    FirstRect = REGION_BOXPTR(pReg);
    rects = FirstRect;

    pwLine = (FbBits *) pPix->devPrivate.ptr;
    nWidth = pPix->devKind >> (FB_SHIFT-3);

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
	pwLineEnd = pw + (width >> FB_SHIFT);
	for (base = 0; pw < pwLineEnd; base += FB_UNIT)
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
	    for(ib = 0; ib < FB_UNIT; ib++)
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
		w = FbScrLeft(w, 1);
	    }
	}
	if(width & FB_MASK)
	{
	    /* Process final partial word on line */
	    w = *pw++;
	    for(ib = 0; ib < (width & FB_MASK); ib++)
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
		w = FbScrLeft(w, 1);
	    }
	}
	/* If scanline ended with last bit set, end the box */
	if(fInBox)
	{
	    ADDRECT(pReg, rects, FirstRect,
		    rx1, h, base + (width & FB_MASK), h + 1);
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

#ifdef FB_DEBUG

#ifndef WIN32
#include <stdio.h>
#else
#include <dbg.h>
#endif

static Bool
fbValidateBits (FbStip *bits, int stride, FbStip data)
{
    while (stride--)
    {
	if (*bits != data)
	{
#ifdef WIN32
	    NCD_DEBUG ((DEBUG_FAILURE, "fdValidateBits failed at 0x%x (is 0x%x want 0x%x)",
			bits, *bits, data));
#else
	    fprintf (stderr, "fbValidateBits failed\n");
#endif
	    return FALSE;
	}
	bits++;
    }
}

void
fbValidateDrawable (DrawablePtr pDrawable)
{
    FbStip	*bits, *first, *last;
    int		stride, bpp;
    int		xoff, yoff;
    int		height;
    Bool	failed;
    
    if (pDrawable->type != DRAWABLE_PIXMAP)
	pDrawable = (DrawablePtr) fbGetWindowPixmap(pDrawable);
    fbGetStipDrawable(pDrawable, bits, stride, bpp, xoff, yoff);
    first = bits - stride;
    last = bits + stride * pDrawable->height;
    if (!fbValidateBits (first, stride, FB_HEAD_BITS) ||
	!fbValidateBits (last, stride, FB_TAIL_BITS))
	fbInitializeDrawable(pDrawable);
}

void
fbSetBits (FbStip *bits, int stride, FbStip data)
{
    while (stride--)
	*bits++ = data;
}

void
fbInitializeDrawable (DrawablePtr pDrawable)
{
    FbStip  *bits, *first, *last;
    int	    stride, bpp;
    int	    xoff, yoff;
    
    fbGetStipDrawable(pDrawable, bits, stride, bpp, xoff, yoff);
    first = bits - stride;
    last = bits + stride * pDrawable->height;
    fbSetBits (first, stride, FB_HEAD_BITS);
    fbSetBits (last, stride, FB_TAIL_BITS);
}
#endif /* FB_DEBUG */
