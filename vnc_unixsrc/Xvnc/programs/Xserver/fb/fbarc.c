/*
 * Id: fbarc.c,v 1.1 1999/11/02 03:54:45 keithp Exp $
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
/* $XFree86: xc/programs/Xserver/fb/fbarc.c,v 1.8 2002/11/09 04:59:29 tsi Exp $ */

#include "fb.h"
#include "mizerarc.h"
#ifdef IN_MODULE
#include "xf86_ansic.h"
#endif

typedef void	(*FbArc) (FbBits    *dst, 
			  FbStride  dstStride, 
			  int	    dstBpp, 
			  xArc	    *arc, 
			  int	    dx,
			  int	    dy,
			  FbBits    and, 
			  FbBits    xor);

void
fbPolyArc (DrawablePtr	pDrawable,
	   GCPtr	pGC,
	   int		narcs,
	   xArc		*parcs)
{
    FbArc	arc;
    
    if (pGC->lineWidth == 0)
    {
#ifndef FBNOPIXADDR
	arc = 0;
	if (pGC->lineStyle == LineSolid && pGC->fillStyle == FillSolid)
	{
	    switch (pDrawable->bitsPerPixel)
	    {
	    case 8:	arc = fbArc8; break;
	    case 16:    arc = fbArc16; break;
#ifdef FB_24BIT
	    case 24:	arc = fbArc24; break;
#endif
	    case 32:    arc = fbArc32; break;
	    }
	}
	if (arc)
	{
	    FbGCPrivPtr	pPriv = fbGetGCPrivate (pGC);
	    FbBits	*dst;
	    FbStride	dstStride;
	    int		dstBpp;
	    int		dstXoff, dstYoff;
	    BoxRec	box;
	    int		x2, y2;
	    RegionPtr	cclip;
	    
	    cclip = fbGetCompositeClip (pGC);
	    fbGetDrawable (pDrawable, dst, dstStride, dstBpp, dstXoff, dstYoff);
	    while (narcs--)
	    {
		if (miCanZeroArc (parcs))
		{
		    box.x1 = parcs->x + pDrawable->x;
		    box.y1 = parcs->y + pDrawable->y;
		    /*
		     * Because box.x2 and box.y2 get truncated to 16 bits, and the
		     * RECT_IN_REGION test treats the resulting number as a signed
		     * integer, the RECT_IN_REGION test alone can go the wrong way.
		     * This can result in a server crash because the rendering
		     * routines in this file deal directly with cpu addresses
		     * of pixels to be stored, and do not clip or otherwise check
		     * that all such addresses are within their respective pixmaps.
		     * So we only allow the RECT_IN_REGION test to be used for
		     * values that can be expressed correctly in a signed short.
		     */
		    x2 = box.x1 + (int)parcs->width + 1;
		    box.x2 = x2;
		    y2 = box.y1 + (int)parcs->height + 1;
		    box.y2 = y2;
		    if ( (x2 <= MAXSHORT) && (y2 <= MAXSHORT) &&
			(RECT_IN_REGION(pDrawable->pScreen, cclip, &box) == rgnIN) )
			(*arc) (dst, dstStride, dstBpp, 
				parcs, pDrawable->x + dstXoff, pDrawable->y + dstYoff, 
				pPriv->and, pPriv->xor);
		    else
			miZeroPolyArc(pDrawable, pGC, 1, parcs);
		}
		else
		    miPolyArc(pDrawable, pGC, 1, parcs);
		parcs++;
	    }
	}
	else
#endif
	    miZeroPolyArc (pDrawable, pGC, narcs, parcs);
    }
    else
	miPolyArc (pDrawable, pGC, narcs, parcs);
}
