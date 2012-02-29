/*
 * $XFree86: xc/programs/Xserver/render/mitri.c,v 1.6 2002/08/12 04:03:21 keithp Exp $
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
#include "mi.h"
#include "picturestr.h"
#include "mipict.h"

void
miPointFixedBounds (int npoint, xPointFixed *points, BoxPtr bounds)
{
    bounds->x1 = xFixedToInt (points->x);
    bounds->x2 = xFixedToInt (xFixedCeil (points->x));
    bounds->y1 = xFixedToInt (points->y);
    bounds->y2 = xFixedToInt (xFixedCeil (points->y));
    points++;
    npoint--;
    while (npoint-- > 0)
    {
	INT16	x1 = xFixedToInt (points->x);
	INT16	x2 = xFixedToInt (xFixedCeil (points->x));
	INT16	y1 = xFixedToInt (points->y);
	INT16	y2 = xFixedToInt (xFixedCeil (points->y));

	if (x1 < bounds->x1)
	    bounds->x1 = x1;
	else if (x2 > bounds->x2)
	    bounds->x2 = x2;
	if (y1 < bounds->y1)
	    bounds->y1 = y1;
	else if (y2 > bounds->y2)
	    bounds->y2 = y2;
	points++;
    }
}

void
miTriangleBounds (int ntri, xTriangle *tris, BoxPtr bounds)
{
    miPointFixedBounds (ntri * 3, (xPointFixed *) tris, bounds);
}

void
miRasterizeTriangle (PicturePtr	pPicture,
		     xTriangle	*tri,
		     int	x_off,
		     int	y_off)
{
    ScreenPtr		pScreen = pPicture->pDrawable->pScreen;
    PictureScreenPtr    ps = GetPictureScreen(pScreen);
    xPointFixed		*top, *left, *right, *t;
    xTrapezoid		trap[2];

    top = &tri->p1;
    left = &tri->p2;
    right = &tri->p3;
    if (left->y < top->y) {
	t = left; left = top; top = t;
    }
    if (right->y < top->y) {
	t = right; right = top; top = t;
    }
    if (right->x < left->x) {
	t = right; right = left; left = t;
    }
    
    /*
     * Two cases:
     *
     *		+		+
     *	       / \             / \
     *	      /   \           /   \
     *	     /     +         +     \
     *      /    --           --    \
     *     /   --               --   \
     *    / ---                   --- \
     *	 +--                         --+
     */
    
    trap[0].top = top->y;
    
    trap[0].left.p1.x = top->x;
    trap[0].left.p1.y = trap[0].top;
    trap[0].left.p2.x = left->x;
    trap[0].left.p2.y = left->y;
    
    trap[0].right.p1 = trap[0].left.p1;
    trap[0].right.p2.x = right->x;
    trap[0].right.p2.y = right->y;
    
    if (right->y < left->y)
    {
	trap[0].bottom = trap[0].right.p2.y;

	trap[1].top = trap[0].bottom;
	trap[1].bottom = trap[0].left.p2.y;
	
	trap[1].left = trap[0].left;
	trap[1].right.p1 = trap[0].right.p2;
	trap[1].right.p2 = trap[0].left.p2;
    }
    else
    {
	trap[0].bottom = trap[0].left.p2.y;
	
	trap[1].top = trap[0].bottom;
	trap[1].bottom = trap[0].right.p2.y;
	
	trap[1].right = trap[0].right;
	trap[1].left.p1 = trap[0].left.p2;
	trap[1].left.p2 = trap[0].right.p2;
    }
    if (trap[0].top != trap[0].bottom)
	(*ps->RasterizeTrapezoid) (pPicture, &trap[0], x_off, y_off);
    if (trap[1].top != trap[1].bottom)
	(*ps->RasterizeTrapezoid) (pPicture, &trap[1], x_off, y_off);
}

void
miTriangles (CARD8	    op,
	     PicturePtr	    pSrc,
	     PicturePtr	    pDst,
	     PictFormatPtr  maskFormat,
	     INT16	    xSrc,
	     INT16	    ySrc,
	     int	    ntri,
	     xTriangle	    *tris)
{
    ScreenPtr		pScreen = pDst->pDrawable->pScreen;
    BoxRec		bounds;
    PicturePtr		pPicture = 0;
    INT16		xDst, yDst;
    INT16		xRel, yRel;
    
    xDst = tris[0].p1.x >> 16;
    yDst = tris[0].p1.y >> 16;
    
    if (maskFormat)
    {
	miTriangleBounds (ntri, tris, &bounds);
	if (bounds.x2 <= bounds.x1 || bounds.y2 <= bounds.y1)
	    return;
	pPicture = miCreateAlphaPicture (pScreen, pDst, maskFormat,
					 bounds.x2 - bounds.x1,
					 bounds.y2 - bounds.y1);
	if (!pPicture)
	    return;
    }
    for (; ntri; ntri--, tris++)
    {
	if (!maskFormat)
	{
	    miTriangleBounds (1, tris, &bounds);
	    if (bounds.x2 <= bounds.x1 || bounds.y2 <= bounds.y1)
		continue;
	    pPicture = miCreateAlphaPicture (pScreen, pDst, maskFormat,
					     bounds.x2 - bounds.x1,
					     bounds.y2 - bounds.y1);
	    if (!pPicture)
		break;
	}
	miRasterizeTriangle (pPicture, tris, -bounds.x1, -bounds.y1);
	if (!maskFormat)
	{
	    xRel = bounds.x1 + xSrc - xDst;
	    yRel = bounds.y1 + ySrc - yDst;
	    CompositePicture (op, pSrc, pPicture, pDst,
			      xRel, yRel, 0, 0, bounds.x1, bounds.y1,
			      bounds.x2 - bounds.x1, bounds.y2 - bounds.y1);
	    FreePicture (pPicture, 0);
	}
	/* XXX adjust xSrc and ySrc */
    }
    if (maskFormat)
    {
	xRel = bounds.x1 + xSrc - xDst;
	yRel = bounds.y1 + ySrc - yDst;
	CompositePicture (op, pSrc, pPicture, pDst,
			  xRel, yRel, 0, 0, bounds.x1, bounds.y1,
			  bounds.x2 - bounds.x1, bounds.y2 - bounds.y1);
	FreePicture (pPicture, 0);
    }
}

void
miTriStrip (CARD8	    op,
	    PicturePtr	    pSrc,
	    PicturePtr	    pDst,
	    PictFormatPtr   maskFormat,
	    INT16	    xSrc,
	    INT16	    ySrc,
	    int		    npoint,
	    xPointFixed	    *points)
{
    ScreenPtr		pScreen = pDst->pDrawable->pScreen;
    xTriangle		tri;
    BoxRec		bounds;
    PicturePtr		pPicture = 0;
    INT16		xDst, yDst;
    INT16		xRel, yRel;
    
    xDst = points[0].x >> 16;
    yDst = points[0].y >> 16;
    
    if (npoint < 3)
	return;
    if (maskFormat)
    {
	miPointFixedBounds (npoint, points, &bounds);
	if (bounds.x2 <= bounds.x1 || bounds.y2 <= bounds.y1)
	    return;
	pPicture = miCreateAlphaPicture (pScreen, pDst, maskFormat,
					 bounds.x2 - bounds.x1,
					 bounds.y2 - bounds.y1);
	if (!pPicture)
	    return;
    }
    for (; npoint >= 3; npoint--, points++)
    {
	tri.p1 = points[0];
	tri.p2 = points[1];
	tri.p3 = points[2];
	if (!maskFormat)
	{
	    miTriangleBounds (1, &tri, &bounds);
	    if (bounds.x2 <= bounds.x1 || bounds.y2 <= bounds.y1)
		continue;
	    pPicture = miCreateAlphaPicture (pScreen, pDst, maskFormat, 
					     bounds.x2 - bounds.x1,
					     bounds.y2 - bounds.y1);
	    if (!pPicture)
		continue;
	}
	miRasterizeTriangle (pPicture, &tri, -bounds.x1, -bounds.y1);
	if (!maskFormat)
	{
	    xRel = bounds.x1 + xSrc - xDst;
	    yRel = bounds.y1 + ySrc - yDst;
	    CompositePicture (op, pSrc, pPicture, pDst,
			      xRel, yRel, 0, 0, bounds.x1, bounds.y1,
			      bounds.x2 - bounds.x1, bounds.y2 - bounds.y1);
	    FreePicture (pPicture, 0);
	}
    }
    if (maskFormat)
    {
	xRel = bounds.x1 + xSrc - xDst;
	yRel = bounds.y1 + ySrc - yDst;
	CompositePicture (op, pSrc, pPicture, pDst,
			  xRel, yRel, 0, 0, bounds.x1, bounds.y1,
			  bounds.x2 - bounds.x1, bounds.y2 - bounds.y1);
	FreePicture (pPicture, 0);
    }
}

void
miTriFan (CARD8		op,
	  PicturePtr	pSrc,
	  PicturePtr	pDst,
	  PictFormatPtr	maskFormat,
	  INT16		xSrc,
	  INT16		ySrc,
	  int		npoint,
	  xPointFixed	*points)
{
    ScreenPtr		pScreen = pDst->pDrawable->pScreen;
    xTriangle		tri;
    BoxRec		bounds;
    PicturePtr		pPicture = 0;
    xPointFixed		*first;
    INT16		xDst, yDst;
    INT16		xRel, yRel;
    
    xDst = points[0].x >> 16;
    yDst = points[0].y >> 16;
    
    if (npoint < 3)
	return;
    if (maskFormat)
    {
	miPointFixedBounds (npoint, points, &bounds);
	if (bounds.x2 <= bounds.x1 || bounds.y2 <= bounds.y1)
	    return;
	pPicture = miCreateAlphaPicture (pScreen, pDst, maskFormat,
					 bounds.x2 - bounds.x1,
					 bounds.y2 - bounds.y1);
	if (!pPicture)
	    return;
    }
    first = points++;
    npoint--;
    for (; npoint >= 2; npoint--, points++)
    {
	tri.p1 = *first;
	tri.p2 = points[0];
	tri.p3 = points[1];
	if (!maskFormat)
	{
	    miTriangleBounds (1, &tri, &bounds);
	    if (bounds.x2 <= bounds.x1 || bounds.y2 <= bounds.y1)
		continue;
	    pPicture = miCreateAlphaPicture (pScreen, pDst, maskFormat, 
					     bounds.x2 - bounds.x1,
					     bounds.y2 - bounds.y1);
	    if (!pPicture)
		continue;
	}
	miRasterizeTriangle (pPicture, &tri, -bounds.x1, -bounds.y1);
	if (!maskFormat)
	{
	    xRel = bounds.x1 + xSrc - xDst;
	    yRel = bounds.y1 + ySrc - yDst;
	    CompositePicture (op, pSrc, pPicture, pDst,
			      xRel, yRel, 0, 0, bounds.x1, bounds.y1,
			      bounds.x2 - bounds.x1, bounds.y2 - bounds.y1);
	    FreePicture (pPicture, 0);
	}
    }
    if (maskFormat)
    {
	xRel = bounds.x1 + xSrc - xDst;
	yRel = bounds.y1 + ySrc - yDst;
	CompositePicture (op, pSrc, pPicture, pDst,
			  xRel, yRel, 0, 0, bounds.x1, bounds.y1,
			  bounds.x2 - bounds.x1, bounds.y2 - bounds.y1);
	FreePicture (pPicture, 0);
    }
}
