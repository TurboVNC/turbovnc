/* $XFree86: xc/programs/Xserver/mfb/mfbzerarc.c,v 3.7 2002/09/27 01:57:47 dawes Exp $ */
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

/* $Xorg: mfbzerarc.c,v 1.4 2001/02/09 02:05:19 xorgcvs Exp $ */

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
#include "mfb.h"
#include "maskbits.h"
#include "mizerarc.h"
#include "mi.h"

/*
 * Note: LEFTMOST must be the bit leftmost in the actual screen
 * representation.  This depends also on the IMAGE_BYTE_ORDER.
 * LONG2CHARS() takes care of the re-ordering as required. (DHD)
 */
#if (BITMAP_BIT_ORDER == MSBFirst)
#define LEFTMOST	((PixelType) LONG2CHARS(((MfbBits)1 << PLST)))
#else
#define LEFTMOST	((PixelType) LONG2CHARS(1))
#endif

#define PixelateWhite(addr,yoff,xoff) \
    *mfbScanlineOffset(addr, (yoff)+((xoff)>>PWSH)) |= \
	SCRRIGHT (LEFTMOST, ((xoff) & PIM))
#define PixelateBlack(addr,yoff,xoff) \
    *mfbScanlineOffset(addr, (yoff)+((xoff)>>PWSH)) &= \
	~(SCRRIGHT (LEFTMOST, ((xoff) & PIM)))

#define Pixelate(base,yoff,xoff) \
{ \
    paddr = mfbScanlineOffset(base, (yoff) + ((xoff)>>PWSH)); \
    pmask = SCRRIGHT(LEFTMOST, (xoff) & PIM); \
    *paddr = (*paddr & ~pmask) | (pixel & pmask); \
}

#define DoPix(bit,base,yoff,xoff) if (mask & bit) Pixelate(base,yoff,xoff);

static void
mfbZeroArcSS(
    DrawablePtr pDraw,
    GCPtr pGC,
    xArc *arc)
{
    miZeroArcRec info;
    Bool do360;
    register int x, y, a, b, d, mask;
    register int k1, k3, dx, dy;
    PixelType *addrl;
    PixelType *yorgl, *yorgol;
    PixelType pixel;
    int nlwidth, yoffset, dyoffset;
    PixelType pmask;
    register PixelType *paddr;

    if (((mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->rop ==
	RROP_BLACK)
	pixel = 0;
    else
	pixel = ~0;

    mfbGetPixelWidthAndPointer(pDraw, nlwidth, addrl);
    do360 = miZeroArcSetup(arc, &info, TRUE);
    yorgl = addrl + ((info.yorg + pDraw->y) * nlwidth);
    yorgol = addrl + ((info.yorgo + pDraw->y) * nlwidth);
    info.xorg += pDraw->x;
    info.xorgo += pDraw->x;
    MIARCSETUP();
    yoffset = y ? nlwidth : 0;
    dyoffset = 0;
    mask = info.initialMask;
    if (!(arc->width & 1))
    {
	DoPix(2, yorgl, 0, info.xorgo);
	DoPix(8, yorgol, 0, info.xorgo);
    }
    if (!info.end.x || !info.end.y)
    {
	mask = info.end.mask;
	info.end = info.altend;
    }
    if (do360 && (arc->width == arc->height) && !(arc->width & 1))
    {
	int xoffset = nlwidth;
	PixelType *yorghl = mfbScanlineDeltaNoBankSwitch(yorgl, info.h, nlwidth);
	int xorghp = info.xorg + info.h;
	int xorghn = info.xorg - info.h;

	if (pixel)
	{
	    while (1)
	    {
		PixelateWhite(yorgl, yoffset, info.xorg + x);
		PixelateWhite(yorgl, yoffset, info.xorg - x);
		PixelateWhite(yorgol, -yoffset, info.xorg - x);
		PixelateWhite(yorgol, -yoffset, info.xorg + x);
		if (a < 0)
		    break;
		PixelateWhite(yorghl, -xoffset, xorghp - y);
		PixelateWhite(yorghl, -xoffset, xorghn + y);
		PixelateWhite(yorghl, xoffset, xorghn + y);
		PixelateWhite(yorghl, xoffset, xorghp - y);
		xoffset += nlwidth;
		MIARCCIRCLESTEP(yoffset += nlwidth;);
	    }
	}
	else
	{
	    while (1)
	    {
		PixelateBlack(yorgl, yoffset, info.xorg + x);
		PixelateBlack(yorgl, yoffset, info.xorg - x);
		PixelateBlack(yorgol, -yoffset, info.xorg - x);
		PixelateBlack(yorgol, -yoffset, info.xorg + x);
		if (a < 0)
		    break;
		PixelateBlack(yorghl, -xoffset, xorghp - y);
		PixelateBlack(yorghl, -xoffset, xorghn + y);
		PixelateBlack(yorghl, xoffset, xorghn + y);
		PixelateBlack(yorghl, xoffset, xorghp - y);
		xoffset += nlwidth;
		MIARCCIRCLESTEP(yoffset += nlwidth;);
	    }
	}
	x = info.w;
	yoffset = info.h * nlwidth;
    }
    else if (do360)
    {
	while (y < info.h || x < info.w)
	{
	    MIARCOCTANTSHIFT(dyoffset = nlwidth;);
	    Pixelate(yorgl, yoffset, info.xorg + x);
	    Pixelate(yorgl, yoffset, info.xorgo - x);
	    Pixelate(yorgol, -yoffset, info.xorgo - x);
	    Pixelate(yorgol, -yoffset, info.xorg + x);
	    MIARCSTEP(yoffset += dyoffset;, yoffset += nlwidth;);
	}
    }
    else
    {
	while (y < info.h || x < info.w)
	{
	    MIARCOCTANTSHIFT(dyoffset = nlwidth;);
	    if ((x == info.start.x) || (y == info.start.y))
	    {
		mask = info.start.mask;
		info.start = info.altstart;
	    }
	    DoPix(1, yorgl, yoffset, info.xorg + x);
	    DoPix(2, yorgl, yoffset, info.xorgo - x);
	    DoPix(4, yorgol, -yoffset, info.xorgo - x);
	    DoPix(8, yorgol, -yoffset, info.xorg + x);
	    if ((x == info.end.x) || (y == info.end.y))
	    {
		mask = info.end.mask;
		info.end = info.altend;
	    }
	    MIARCSTEP(yoffset += dyoffset;, yoffset += nlwidth;);
	}
    }
    if ((x == info.start.x) || (y == info.start.y))
	mask = info.start.mask;
    DoPix(1, yorgl, yoffset, info.xorg + x);
    DoPix(4, yorgol, -yoffset, info.xorgo - x);
    if (arc->height & 1)
    {
	DoPix(2, yorgl, yoffset, info.xorgo - x);
	DoPix(8, yorgol, -yoffset, info.xorg + x);
    }
}

void
mfbZeroPolyArcSS(pDraw, pGC, narcs, parcs)
    DrawablePtr	pDraw;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    register xArc *arc;
    register int i;
    BoxRec box;
    int x2, y2;
    RegionPtr cclip;

    if (!(pGC->planemask & 1))
	return;
    cclip = pGC->pCompositeClip;
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
		mfbZeroArcSS(pDraw, pGC, arc);
	    else
		miZeroPolyArc(pDraw, pGC, 1, arc);
	}
	else
	    miPolyArc(pDraw, pGC, 1, arc);
    }
}
