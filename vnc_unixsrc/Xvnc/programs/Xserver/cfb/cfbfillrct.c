/*
 * Fill rectangles.
 */

/*

Copyright (c) 1989  X Consortium

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

/* $XConsortium: cfbfillrct.c,v 5.18 94/04/17 20:28:47 dpw Exp $ */
/* $XFree86: xc/programs/Xserver/cfb/cfbfillrct.c,v 3.1.4.1 1997/05/10 07:02:48 hohndel Exp $ */

#include "X.h"
#include "Xmd.h"
#include "servermd.h"
#include "gcstruct.h"
#include "window.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "windowstr.h"

#include "cfb.h"
#include "cfbmskbits.h"
#include "mergerop.h"


void
cfbFillBoxTileOdd (pDrawable, n, rects, tile, xrot, yrot)
    DrawablePtr	pDrawable;
    int		n;
    BoxPtr	rects;
    PixmapPtr	tile;
    int		xrot, yrot;
{
#if PSZ == 24
    if (tile->drawable.width & 3)
#else
    if (tile->drawable.width & PIM)
#endif
	cfbFillBoxTileOddCopy (pDrawable, n, rects, tile, xrot, yrot, GXcopy, ~0L);
    else
	cfbFillBoxTile32sCopy (pDrawable, n, rects, tile, xrot, yrot, GXcopy, ~0L);
}

void
cfbFillRectTileOdd (pDrawable, pGC, nBox, pBox)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		nBox;
    BoxPtr	pBox;
{
    int	xrot, yrot;
    void    (*fill)();

    xrot = pDrawable->x + pGC->patOrg.x;
    yrot = pDrawable->y + pGC->patOrg.y;
#if PSZ == 24
    if (pGC->tile.pixmap->drawable.width & 3)
#else
    if (pGC->tile.pixmap->drawable.width & PIM)
#endif
    {
    	fill = cfbFillBoxTileOddGeneral;
    	if ((pGC->planemask & PMSK) == PMSK)
    	{
	    if (pGC->alu == GXcopy)
	    	fill = cfbFillBoxTileOddCopy;
    	}
    }
    else
    {
    	fill = cfbFillBoxTile32sGeneral;
    	if ((pGC->planemask & PMSK) == PMSK)
    	{
	    if (pGC->alu == GXcopy)
	    	fill = cfbFillBoxTile32sCopy;
    	}
    }
    (*fill) (pDrawable, nBox, pBox, pGC->tile.pixmap, xrot, yrot, pGC->alu, pGC->planemask);
}

#define NUM_STACK_RECTS	1024

void
cfbPolyFillRect(pDrawable, pGC, nrectFill, prectInit)
    DrawablePtr pDrawable;
    register GCPtr pGC;
    int		nrectFill; 	/* number of rectangles to fill */
    xRectangle	*prectInit;  	/* Pointer to first rectangle to fill */
{
    xRectangle	    *prect;
    RegionPtr	    prgnClip;
    register BoxPtr pbox;
    register BoxPtr pboxClipped;
    BoxPtr	    pboxClippedBase;
    BoxPtr	    pextent;
    BoxRec	    stackRects[NUM_STACK_RECTS];
    cfbPrivGC	    *priv;
    int		    numRects;
    void	    (*BoxFill)();
    int		    n;
    int		    xorg, yorg;

#if PSZ != 8
    if ((pGC->fillStyle == FillStippled) ||
	(pGC->fillStyle == FillOpaqueStippled)) {
       miPolyFillRect(pDrawable, pGC, nrectFill, prectInit);
       return;
    }
#endif

    priv = cfbGetGCPrivate(pGC);
    prgnClip = priv->pCompositeClip;

    BoxFill = 0;
    switch (pGC->fillStyle)
    {
    case FillSolid:
	switch (priv->rop) {
	case GXcopy:
	    BoxFill = cfbFillRectSolidCopy;
	    break;
	case GXxor:
	    BoxFill = cfbFillRectSolidXor;
	    break;
	default:
	    BoxFill = cfbFillRectSolidGeneral;
	    break;
	}
	break;
    case FillTiled:
	if (!cfbGetGCPrivate(pGC)->pRotatedPixmap)
	    BoxFill = cfbFillRectTileOdd;
	else
	{
	    if (pGC->alu == GXcopy && (pGC->planemask & PMSK) == PMSK)
		BoxFill = cfbFillRectTile32Copy;
	    else
		BoxFill = cfbFillRectTile32General;
	}
	break;
#if PSZ == 8
    case FillStippled:
	if (!cfbGetGCPrivate(pGC)->pRotatedPixmap)
	    BoxFill = cfb8FillRectStippledUnnatural;
	else
	    BoxFill = cfb8FillRectTransparentStippled32;
	break;
    case FillOpaqueStippled:
	if (!cfbGetGCPrivate(pGC)->pRotatedPixmap)
	    BoxFill = cfb8FillRectStippledUnnatural;
	else
	    BoxFill = cfb8FillRectOpaqueStippled32;
	break;
#endif
    }
    prect = prectInit;
    xorg = pDrawable->x;
    yorg = pDrawable->y;
    if (xorg || yorg)
    {
	prect = prectInit;
	n = nrectFill;
	while(n--)
	{
	    prect->x += xorg;
	    prect->y += yorg;
	    prect++;
	}
    }

    prect = prectInit;

    numRects = REGION_NUM_RECTS(prgnClip) * nrectFill;
    if (numRects > NUM_STACK_RECTS)
    {
	pboxClippedBase = (BoxPtr)ALLOCATE_LOCAL(numRects * sizeof(BoxRec));
	if (!pboxClippedBase)
	    return;
    }
    else
	pboxClippedBase = stackRects;

    pboxClipped = pboxClippedBase;
	
    if (REGION_NUM_RECTS(prgnClip) == 1)
    {
	int x1, y1, x2, y2, bx2, by2;

	pextent = REGION_RECTS(prgnClip);
	x1 = pextent->x1;
	y1 = pextent->y1;
	x2 = pextent->x2;
	y2 = pextent->y2;
    	while (nrectFill--)
    	{
	    if ((pboxClipped->x1 = prect->x) < x1)
		pboxClipped->x1 = x1;
    
	    if ((pboxClipped->y1 = prect->y) < y1)
		pboxClipped->y1 = y1;
    
	    bx2 = (int) prect->x + (int) prect->width;
	    if (bx2 > x2)
		bx2 = x2;
	    pboxClipped->x2 = bx2;
    
	    by2 = (int) prect->y + (int) prect->height;
	    if (by2 > y2)
		by2 = y2;
	    pboxClipped->y2 = by2;

	    prect++;
	    if ((pboxClipped->x1 < pboxClipped->x2) &&
		(pboxClipped->y1 < pboxClipped->y2))
	    {
		pboxClipped++;
	    }
    	}
    }
    else
    {
	int x1, y1, x2, y2, bx2, by2;

	pextent = REGION_EXTENTS(pGC->pScreen, prgnClip);
	x1 = pextent->x1;
	y1 = pextent->y1;
	x2 = pextent->x2;
	y2 = pextent->y2;
    	while (nrectFill--)
    	{
	    BoxRec box;
    
	    if ((box.x1 = prect->x) < x1)
		box.x1 = x1;
    
	    if ((box.y1 = prect->y) < y1)
		box.y1 = y1;
    
	    bx2 = (int) prect->x + (int) prect->width;
	    if (bx2 > x2)
		bx2 = x2;
	    box.x2 = bx2;
    
	    by2 = (int) prect->y + (int) prect->height;
	    if (by2 > y2)
		by2 = y2;
	    box.y2 = by2;
    
	    prect++;
    
	    if ((box.x1 >= box.x2) || (box.y1 >= box.y2))
	    	continue;
    
	    n = REGION_NUM_RECTS (prgnClip);
	    pbox = REGION_RECTS(prgnClip);
    
	    /* clip the rectangle to each box in the clip region
	       this is logically equivalent to calling Intersect()
	    */
	    while(n--)
	    {
		pboxClipped->x1 = max(box.x1, pbox->x1);
		pboxClipped->y1 = max(box.y1, pbox->y1);
		pboxClipped->x2 = min(box.x2, pbox->x2);
		pboxClipped->y2 = min(box.y2, pbox->y2);
		pbox++;

		/* see if clipping left anything */
		if(pboxClipped->x1 < pboxClipped->x2 && 
		   pboxClipped->y1 < pboxClipped->y2)
		{
		    pboxClipped++;
		}
	    }
    	}
    }
    if (pboxClipped != pboxClippedBase)
	(*BoxFill) (pDrawable, pGC,
		    pboxClipped-pboxClippedBase, pboxClippedBase);
    if (pboxClippedBase != stackRects)
    	DEALLOCATE_LOCAL(pboxClippedBase);
}
