/*
 * $XFree86: xc/programs/Xserver/render/mitrap.c,v 1.9 2002/11/05 23:39:16 keithp Exp $
 *
 * Copyright © 2002 Keith Packard, member of The XFree86 Project, Inc.
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

#include "scrnintstr.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "windowstr.h"
#include "servermd.h"
#include "mi.h"
#include "picturestr.h"
#include "mipict.h"

PicturePtr
miCreateAlphaPicture (ScreenPtr	    pScreen, 
		      PicturePtr    pDst,
		      PictFormatPtr pPictFormat,
		      CARD16	    width,
		      CARD16	    height)
{
    PixmapPtr	    pPixmap;
    PicturePtr	    pPicture;
    GCPtr	    pGC;
    int		    error;
    xRectangle	    rect;

    if (width > 32767 || height > 32767)
	return 0;

    if (!pPictFormat)
    {
	if (pDst->polyEdge == PolyEdgeSharp)
	    pPictFormat = PictureMatchFormat (pScreen, 1, PICT_a1);
	else
	    pPictFormat = PictureMatchFormat (pScreen, 8, PICT_a8);
	if (!pPictFormat)
	    return 0;
    }

    pPixmap = (*pScreen->CreatePixmap) (pScreen, width, height, 
					pPictFormat->depth);
    if (!pPixmap)
	return 0;
    pGC = GetScratchGC (pPixmap->drawable.depth, pScreen);
    if (!pGC)
    {
	(*pScreen->DestroyPixmap) (pPixmap);
	return 0;
    }
    ValidateGC (&pPixmap->drawable, pGC);
    rect.x = 0;
    rect.y = 0;
    rect.width = width;
    rect.height = height;
    (*pGC->ops->PolyFillRect)(&pPixmap->drawable, pGC, 1, &rect);
    FreeScratchGC (pGC);
    pPicture = CreatePicture (0, &pPixmap->drawable, pPictFormat,
			      0, 0, serverClient, &error);
    (*pScreen->DestroyPixmap) (pPixmap);
    return pPicture;
}

static xFixed
miLineFixedX (xLineFixed *l, xFixed y, Bool ceil)
{
    xFixed	    dx = l->p2.x - l->p1.x;
    xFixed_32_32    ex = (xFixed_32_32) (y - l->p1.y) * dx;
    xFixed	    dy = l->p2.y - l->p1.y;
    if (ceil)
	ex += (dy - 1);
    return l->p1.x + (xFixed) (ex / dy);
}

void
miTrapezoidBounds (int ntrap, xTrapezoid *traps, BoxPtr box)
{
    box->y1 = MAXSHORT;
    box->y2 = MINSHORT;
    box->x1 = MAXSHORT;
    box->x2 = MINSHORT;
    for (; ntrap; ntrap--, traps++)
    {
	INT16 x1, y1, x2, y2;

	if (!xTrapezoidValid(traps))
	    continue;
	y1 = xFixedToInt (traps->top);
	if (y1 < box->y1)
	    box->y1 = y1;
	
	y2 = xFixedToInt (xFixedCeil (traps->bottom));
	if (y2 > box->y2)
	    box->y2 = y2;
	
	x1 = xFixedToInt (min (miLineFixedX (&traps->left, traps->top, FALSE),
			       miLineFixedX (&traps->left, traps->bottom, FALSE)));
	if (x1 < box->x1)
	    box->x1 = x1;
	
	x2 = xFixedToInt (xFixedCeil (max (miLineFixedX (&traps->right, traps->top, TRUE),
					   miLineFixedX (&traps->right, traps->bottom, TRUE))));
	if (x2 > box->x2)
	    box->x2 = x2;
    }
}

void
miTrapezoids (CARD8	    op,
	      PicturePtr    pSrc,
	      PicturePtr    pDst,
	      PictFormatPtr maskFormat,
	      INT16	    xSrc,
	      INT16	    ySrc,
	      int	    ntrap,
	      xTrapezoid    *traps)
{
    ScreenPtr		pScreen = pDst->pDrawable->pScreen;
    PictureScreenPtr    ps = GetPictureScreen(pScreen);
    PicturePtr		pPicture = 0;
    BoxRec		bounds;
    INT16		xDst, yDst;
    INT16		xRel, yRel;
    
    xDst = traps[0].left.p1.x >> 16;
    yDst = traps[0].left.p1.y >> 16;
    
    if (maskFormat)
    {
	miTrapezoidBounds (ntrap, traps, &bounds);
	if (bounds.y1 >= bounds.y2 || bounds.x1 >= bounds.x2)
	    return;
	pPicture = miCreateAlphaPicture (pScreen, pDst, maskFormat,
					 bounds.x2 - bounds.x1,
					 bounds.y2 - bounds.y1);
	if (!pPicture)
	    return;
    }
    for (; ntrap; ntrap--, traps++)
    {
	if (!xTrapezoidValid(traps))
	    continue;
	if (!maskFormat)
	{
	    miTrapezoidBounds (1, traps, &bounds);
	    if (bounds.y1 >= bounds.y2 || bounds.x1 >= bounds.x2)
		continue;
	    pPicture = miCreateAlphaPicture (pScreen, pDst, maskFormat,
					     bounds.x2 - bounds.x1,
					     bounds.y2 - bounds.y1);
	    if (!pPicture)
		continue;
	}
	(*ps->RasterizeTrapezoid) (pPicture, traps, 
				   -bounds.x1, -bounds.y1);
	if (!maskFormat)
	{
	    xRel = bounds.x1 + xSrc - xDst;
	    yRel = bounds.y1 + ySrc - yDst;
	    CompositePicture (op, pSrc, pPicture, pDst,
			      xRel, yRel, 0, 0, bounds.x1, bounds.y1,
			      bounds.x2 - bounds.x1,
			      bounds.y2 - bounds.y1);
	    FreePicture (pPicture, 0);
	}
    }
    if (maskFormat)
    {
	xRel = bounds.x1 + xSrc - xDst;
	yRel = bounds.y1 + ySrc - yDst;
	CompositePicture (op, pSrc, pPicture, pDst,
			  xRel, yRel, 0, 0, bounds.x1, bounds.y1,
			  bounds.x2 - bounds.x1,
			  bounds.y2 - bounds.y1);
	FreePicture (pPicture, 0);
    }
}
