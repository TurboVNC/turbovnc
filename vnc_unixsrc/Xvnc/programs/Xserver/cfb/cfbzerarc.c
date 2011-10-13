/* $XFree86: xc/programs/Xserver/cfb/cfbzerarc.c,v 3.4tsi Exp $ */
/************************************************************

Copyright 1989, 1998  The Open Group

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

********************************************************/

/* $Xorg: cfbzerarc.c,v 1.4 2001/02/09 02:04:39 xorgcvs Exp $ */

/* Derived from:
 * "Algorithm for drawing ellipses or hyperbolae with a digital plotter"
 * by M. L. V. Pitteway
 * The Computer Journal, November 1967, Volume 10, Number 3, pp. 282-289
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xprotostr.h>
#include "regionstr.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "cfb.h"
#include "cfbmskbits.h"
#include "mizerarc.h"
#include "cfbrrop.h"
#include "mi.h"

#ifdef PIXEL_ADDR

static void
RROP_NAME(cfbZeroArcSS8)(
    DrawablePtr pDraw,
    GCPtr pGC,
    xArc *arc)
{
    miZeroArcRec info;
    Bool do360;
    register int x;
    PixelType *addrp;
    register PixelType *yorgp, *yorgop;
#if PSZ == 24
    int xorg, xorg3, xorgo, xorgo3;
    register int xtmp;
#endif
    RROP_DECLARE
    register int yoffset;
    int npwidth, dyoffset;
    register int y, a, b, d, mask;
    register int k1, k3, dx, dy;

    cfbGetPixelWidthAndPointer(pDraw,npwidth, addrp)

    RROP_FETCH_GC (pGC);
    do360 = miZeroArcSetup(arc, &info, TRUE);
    yorgp = addrp + ((info.yorg + pDraw->y) * npwidth);
    yorgop = addrp + ((info.yorgo + pDraw->y) * npwidth);
    info.xorg += pDraw->x;
    info.xorgo += pDraw->x;
#if PSZ == 24
    xorg = info.xorg;
    xorg3 = xorg * 3;
    info.xorg = (info.xorg * 3) >> 2;
    xorgo = info.xorgo;
    xorgo3 = xorgo * 3; 
    info.xorgo = (info.xorgo * 3) >> 2;
#endif
    MIARCSETUP();
    yoffset = y ? npwidth : 0;
    dyoffset = 0;
    mask = info.initialMask;
    if (!(arc->width & 1))
    {
#if PSZ == 24
	if (mask & 2)
	    RROP_SOLID24((yorgp + info.xorgo), xorgo);
	if (mask & 8)
	    RROP_SOLID24((yorgop + info.xorgo), xorgo);
#else
	if (mask & 2)
	    RROP_SOLID((yorgp + info.xorgo));
	if (mask & 8)
	    RROP_SOLID((yorgop + info.xorgo));
#endif /* PSZ == 24 */
    }
    if (!info.end.x || !info.end.y)
    {
	mask = info.end.mask;
	info.end = info.altend;
    }
    if (do360 && (arc->width == arc->height) && !(arc->width & 1))
    {
	register int xoffset = npwidth;
#if PSZ == 24
	PixelType *yorghb = yorgp + (info.h * npwidth);
	register int tmp1, tmp2, tmp1_3, tmp2_3;

	tmp1 = xorg + info.h;
	tmp1_3 = tmp1 * 3;
	tmp2 = xorg - info.h;
	tmp2_3 = tmp2 * 3;
	while (1)
	{
	    xtmp = (xorg3 + x * 3) >> 2;
	    RROP_SOLID24(yorgp + yoffset + xtmp, xorg + x);
	    RROP_SOLID24(yorgop - yoffset + xtmp, xorg + x);
	    xtmp = (xorg3 - x * 3) >> 2;
	    RROP_SOLID24(yorgp + yoffset + xtmp, xorg - x);
	    RROP_SOLID24(yorgop - yoffset + xtmp, xorg - x);
	    if (a < 0)
		break;
	    xtmp = (tmp1_3 - y * 3) >> 2;
	    RROP_SOLID24(yorghb - xoffset + xtmp, tmp1 - y);
	    RROP_SOLID24(yorghb + xoffset + xtmp, tmp1 - y);
	    xtmp = (tmp2_3 + y * 3) >> 2;
	    RROP_SOLID24(yorghb - xoffset + xtmp, tmp2 + y);
	    RROP_SOLID24(yorghb + xoffset + xtmp, tmp2 + y);
	    xoffset += npwidth;
	    MIARCCIRCLESTEP(yoffset += npwidth;);
	}
#else
	PixelType *yorghb = yorgp + (info.h * npwidth) + info.xorg;
	PixelType *yorgohb = yorghb - info.h;

	yorgp += info.xorg;
	yorgop += info.xorg;
	yorghb += info.h;
	while (1)
	{
	    RROP_SOLID(yorgp + yoffset + x);
	    RROP_SOLID(yorgp + yoffset - x);
	    RROP_SOLID(yorgop - yoffset - x);
	    RROP_SOLID(yorgop - yoffset + x);
	    if (a < 0)
		break;
	    RROP_SOLID(yorghb - xoffset - y);
	    RROP_SOLID(yorgohb - xoffset + y);
	    RROP_SOLID(yorgohb + xoffset + y);
	    RROP_SOLID(yorghb + xoffset - y);
	    xoffset += npwidth;
	    MIARCCIRCLESTEP(yoffset += npwidth;);
	}
	yorgp -= info.xorg;
	yorgop -= info.xorg;
#endif /* PSZ == 24 */
	x = info.w;
	yoffset = info.h * npwidth;
    }
    else if (do360)
    {
	while (y < info.h || x < info.w)
	{
	    MIARCOCTANTSHIFT(dyoffset = npwidth;);
#if PSZ == 24
	    xtmp = (xorg3 + x * 3) >> 2;
	    RROP_SOLID24(yorgp + yoffset + xtmp, xorg + x);
	    RROP_SOLID24(yorgop - yoffset + xtmp, xorg + x);
	    xtmp = (xorgo3 - x * 3) >> 2;
	    RROP_SOLID24(yorgp + yoffset + xtmp, xorgo - x);
	    RROP_SOLID24(yorgop - yoffset + xtmp, xorgo - x);
#else
	    RROP_SOLID(yorgp + yoffset + info.xorg + x);
	    RROP_SOLID(yorgp + yoffset + info.xorgo - x);
	    RROP_SOLID(yorgop - yoffset + info.xorgo - x);
	    RROP_SOLID(yorgop - yoffset + info.xorg + x);
#endif
	    MIARCSTEP(yoffset += dyoffset;, yoffset += npwidth;);
	}
    }
    else
    {
	while (y < info.h || x < info.w)
	{
	    MIARCOCTANTSHIFT(dyoffset = npwidth;);
	    if ((x == info.start.x) || (y == info.start.y))
	    {
		mask = info.start.mask;
		info.start = info.altstart;
	    }
#if PSZ == 24
	    if (mask & 1){
	      xtmp = (xorg3 + x * 3) >> 2;
	      RROP_SOLID24(yorgp + yoffset + xtmp, xorg + x);
	    }
	    if (mask & 2){
	      xtmp = (xorgo3 - x * 3) >> 2;
	      RROP_SOLID24(yorgp + yoffset + xtmp, xorgo - x);
	    }
	    if (mask & 4){
	      xtmp = (xorgo3 - x * 3) >> 2;
	      RROP_SOLID24(yorgop - yoffset + xtmp, xorgo - x);
	    }
	    if (mask & 8){
	      xtmp = (xorg3 + x * 3) >> 2;
	      RROP_SOLID24(yorgop - yoffset + xtmp, xorg + x);
	    }
#else
	    if (mask & 1)
		RROP_SOLID(yorgp + yoffset + info.xorg + x);
	    if (mask & 2)
		RROP_SOLID(yorgp + yoffset + info.xorgo - x);
	    if (mask & 4)
		RROP_SOLID(yorgop - yoffset + info.xorgo - x);
	    if (mask & 8)
		RROP_SOLID(yorgop - yoffset + info.xorg + x);
#endif /* PSZ == 24 */
	    if ((x == info.end.x) || (y == info.end.y))
	    {
		mask = info.end.mask;
		info.end = info.altend;
	    }
	    MIARCSTEP(yoffset += dyoffset;, yoffset += npwidth;);
	}
    }
    if ((x == info.start.x) || (y == info.start.y))
	mask = info.start.mask;
#if PSZ == 24
    if (mask & 1){
      xtmp = (xorg3 + x * 3) >> 2;
      RROP_SOLID24(yorgp + yoffset + xtmp, xorg + x);
    }
    if (mask & 4){
      xtmp = (xorgo3 - x * 3) >> 2;
      RROP_SOLID24(yorgop - yoffset + xtmp, xorgo - x);
    }
#else
    if (mask & 1)
	RROP_SOLID(yorgp + yoffset + info.xorg + x);
    if (mask & 4)
	RROP_SOLID(yorgop - yoffset + info.xorgo - x);
#endif /* PSZ == 24 */
    if (arc->height & 1)
    {
#if PSZ == 24
	if (mask & 2){
	  xtmp = (xorgo3 - x * 3) >> 2;
	  RROP_SOLID24(yorgp + yoffset + xtmp, xorgo - x);
	}
	if (mask & 8){
	  xtmp = (xorg3 + x * 3) >> 2;
	  RROP_SOLID24(yorgop - yoffset + xtmp, xorg + x);
	}
#else
	if (mask & 2)
	    RROP_SOLID(yorgp + yoffset + info.xorgo - x);
	if (mask & 8)
	    RROP_SOLID(yorgop - yoffset + info.xorg + x);
#endif /* PSZ == 24 */
    }
    RROP_UNDECLARE
}

void
RROP_NAME (cfbZeroPolyArcSS8) (pDraw, pGC, narcs, parcs)
    register DrawablePtr	pDraw;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    register xArc *arc;
    register int i;
    BoxRec box;
    int x2, y2;
    RegionPtr cclip;

    cclip = cfbGetCompositeClip(pGC);
    for (arc = parcs, i = narcs; --i >= 0; arc++)
    {
	if (miCanZeroArc(arc))
	{
	    box.x1 = arc->x + pDraw->x;
	    box.y1 = arc->y + pDraw->y;
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
 	    x2 = box.x1 + (int)arc->width + 1;
 	    box.x2 = x2;
 	    y2 = box.y1 + (int)arc->height + 1;
 	    box.y2 = y2;
 	    if ( (x2 <= MAXSHORT) && (y2 <= MAXSHORT) &&
 		    (RECT_IN_REGION(pDraw->pScreen, cclip, &box) == rgnIN) )
		RROP_NAME (cfbZeroArcSS8) (pDraw, pGC, arc);
	    else
		miZeroPolyArc(pDraw, pGC, 1, arc);
	}
	else
	    miPolyArc(pDraw, pGC, 1, arc);
    }
}

#endif
