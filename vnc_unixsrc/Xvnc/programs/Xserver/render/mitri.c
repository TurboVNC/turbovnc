/*
 * $XFree86: xc/programs/Xserver/render/mitri.c,v 1.5 2002/05/31 16:48:52 keithp Exp $
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

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

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
    PictureScreenPtr    ps = GetPictureScreen(pScreen);
    
    /*
     * Check for solid alpha add
     */
    if (op == PictOpAdd && miIsSolidAlpha (pSrc))
    {
	(*ps->AddTriangles) (pDst, 0, 0, ntri, tris);
    }
    else if (maskFormat)
    {
	BoxRec		bounds;
	PicturePtr	pPicture;
	INT16		xDst, yDst;
	INT16		xRel, yRel;
	
	xDst = tris[0].p1.x >> 16;
	yDst = tris[0].p1.y >> 16;

	miTriangleBounds (ntri, tris, &bounds);
	if (bounds.x2 <= bounds.x1 || bounds.y2 <= bounds.y1)
	    return;
	pPicture = miCreateAlphaPicture (pScreen, pDst, maskFormat,
					 bounds.x2 - bounds.x1,
					 bounds.y2 - bounds.y1);
	if (!pPicture)
	    return;
	(*ps->AddTriangles) (pPicture, -bounds.x1, -bounds.y1, ntri, tris);
	
	xRel = bounds.x1 + xSrc - xDst;
	yRel = bounds.y1 + ySrc - yDst;
	CompositePicture (op, pSrc, pPicture, pDst,
			  xRel, yRel, 0, 0, bounds.x1, bounds.y1,
			  bounds.x2 - bounds.x1, bounds.y2 - bounds.y1);
	FreePicture (pPicture, 0);
    }
    else
    {
	if (pDst->polyEdge == PolyEdgeSharp)
	    maskFormat = PictureMatchFormat (pScreen, 1, PICT_a1);
	else
	    maskFormat = PictureMatchFormat (pScreen, 8, PICT_a8);
	
	for (; ntri; ntri--, tris++)
	    miTriangles (op, pSrc, pDst, maskFormat, xSrc, ySrc, 1, tris);
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
    PictureScreenPtr    ps = GetPictureScreen(pScreen);
    xTriangle		*tris, *tri;
    int			ntri;
    
    if (npoint < 3)
	return;
    ntri = npoint - 2;
    tris = ALLOCATE_LOCAL (ntri & sizeof (xTriangle));
    if (!tris)
	return;
    for (tri = tris; npoint >= 3; npoint--, points++, tri++)
    {
	tri->p1 = points[0];
	tri->p2 = points[1];
	tri->p3 = points[2];
    }
    (*ps->Triangles) (op, pSrc, pDst, maskFormat, xSrc, ySrc, ntri, tris);
    DEALLOCATE_LOCAL (tris);
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
    PictureScreenPtr    ps = GetPictureScreen(pScreen);
    xTriangle		*tris, *tri;
    xPointFixed		*first;
    int			ntri;
    
    if (npoint < 3)
	return;
    ntri = npoint - 2;
    tris = ALLOCATE_LOCAL (ntri & sizeof (xTriangle));
    if (!tris)
	return;
    first = points++;
    for (tri = tris; npoint >= 3; npoint--, points++, tri++)
    {
	tri->p1 = *first;
	tri->p2 = points[0];
	tri->p3 = points[1];
    }
    (*ps->Triangles) (op, pSrc, pDst, maskFormat, xSrc, ySrc, ntri, tris);
    DEALLOCATE_LOCAL (tris);
}
