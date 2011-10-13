/* $XFree86: xc/programs/Xserver/mfb/mfbfillarc.c,v 1.5 2001/12/14 20:00:06 dawes Exp $ */
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

/* $Xorg: mfbfillarc.c,v 1.4 2001/02/09 02:05:18 xorgcvs Exp $ */

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
#include "mifillarc.h"
#include "mi.h"

static void
mfbFillEllipseSolid(
    DrawablePtr pDraw,
    xArc *arc,
    register int rop)
{
    int x, y, e;
    int yk, xk, ym, xm, dx, dy, xorg, yorg;
    register int slw;
    miFillArcRec info;
    PixelType *addrlt, *addrlb;
    register PixelType *addrl;
    register int n;
    int nlwidth;
    register int xpos;
    PixelType startmask, endmask;
    int nlmiddle;

    mfbGetPixelWidthAndPointer(pDraw, nlwidth, addrlt);
    miFillArcSetup(arc, &info);
    MIFILLARCSETUP();
    xorg += pDraw->x;
    yorg += pDraw->y;
    addrlb = addrlt;
    addrlt += nlwidth * (yorg - y);
    addrlb += nlwidth * (yorg + y + dy);
    while (y)
    {
	addrlt += nlwidth;
	addrlb -= nlwidth;
	MIFILLARCSTEP(slw);
	if (!slw)
	    continue;
	xpos = xorg - x;
	addrl = mfbScanlineOffset(addrlt, (xpos >> PWSH));
	if (((xpos & PIM) + slw) < PPW)
	{
	    maskpartialbits(xpos, slw, startmask);
	    if (rop == RROP_BLACK)
		*addrl &= ~startmask;
	    else if (rop == RROP_WHITE)
		*addrl |= startmask;
	    else
		*addrl ^= startmask;
	    if (miFillArcLower(slw))
	    {
		addrl = mfbScanlineOffset(addrlb, (xpos >> PWSH));
		if (rop == RROP_BLACK)
		    *addrl &= ~startmask;
		else if (rop == RROP_WHITE)
		    *addrl |= startmask;
		else
		    *addrl ^= startmask;
	    }
	    continue;
	}
	maskbits(xpos, slw, startmask, endmask, nlmiddle);
	if (startmask)
	{
	    if (rop == RROP_BLACK)
		*addrl++ &= ~startmask;
	    else if (rop == RROP_WHITE)
		*addrl++ |= startmask;
	    else
		*addrl++ ^= startmask;
	}
	n = nlmiddle;
	if (rop == RROP_BLACK)
	    while (n--)
		*addrl++ = 0;
	else if (rop == RROP_WHITE)
	    while (n--)
		*addrl++ = ~0;
	else
	    while (n--)
		*addrl++ ^= ~0;
	if (endmask)
	{
	    if (rop == RROP_BLACK)
		*addrl &= ~endmask;
	    else if (rop == RROP_WHITE)
		*addrl |= endmask;
	    else
		*addrl ^= endmask;
	}
	if (!miFillArcLower(slw))
	    continue;
	addrl = mfbScanlineOffset(addrlb, (xpos >> PWSH));
	if (startmask)
	{
	    if (rop == RROP_BLACK)
		*addrl++ &= ~startmask;
	    else if (rop == RROP_WHITE)
		*addrl++ |= startmask;
	    else
		*addrl++ ^= startmask;
	}
	n = nlmiddle;
	if (rop == RROP_BLACK)
	    while (n--)
		*addrl++ = 0;
	else if (rop == RROP_WHITE)
	    while (n--)
		*addrl++ = ~0;
	else
	    while (n--)
		*addrl++ ^= ~0;
	if (endmask)
	{
	    if (rop == RROP_BLACK)
		*addrl &= ~endmask;
	    else if (rop == RROP_WHITE)
		*addrl |= endmask;
	    else
		*addrl ^= endmask;
	}
    }
}

#define FILLSPAN(xl,xr,addr) \
    if (xr >= xl) \
    { \
	width = xr - xl + 1; \
	addrl = mfbScanlineOffset(addr, (xl >> PWSH)); \
	if (((xl & PIM) + width) < PPW) \
	{ \
	    maskpartialbits(xl, width, startmask); \
	    if (rop == RROP_BLACK) \
		*addrl &= ~startmask; \
	    else if (rop == RROP_WHITE) \
		*addrl |= startmask; \
	    else \
		*addrl ^= startmask; \
	} \
	else \
	{ \
	    maskbits(xl, width, startmask, endmask, nlmiddle); \
	    if (startmask) \
	    { \
		if (rop == RROP_BLACK) \
		    *addrl++ &= ~startmask; \
		else if (rop == RROP_WHITE) \
		    *addrl++ |= startmask; \
		else \
		    *addrl++ ^= startmask; \
	    } \
	    n = nlmiddle; \
	    if (rop == RROP_BLACK) \
		while (n--) \
		    *addrl++ = 0; \
	    else if (rop == RROP_WHITE) \
		while (n--) \
		    *addrl++ = ~0; \
	    else \
		while (n--) \
		    *addrl++ ^= ~0; \
	    if (endmask) \
	    { \
		if (rop == RROP_BLACK) \
		    *addrl &= ~endmask; \
		else if (rop == RROP_WHITE) \
		    *addrl |= endmask; \
		else \
		    *addrl ^= endmask; \
	    } \
	} \
    }

#define FILLSLICESPANS(flip,addr) \
    if (!flip) \
    { \
	FILLSPAN(xl, xr, addr); \
    } \
    else \
    { \
	xc = xorg - x; \
	FILLSPAN(xc, xr, addr); \
	xc += slw - 1; \
	FILLSPAN(xl, xc, addr); \
    }

static void
mfbFillArcSliceSolidCopy(
    DrawablePtr pDraw,
    GCPtr pGC,
    xArc *arc,
    register int rop)
{
    register PixelType *addrl;
    register int n;
    int yk, xk, ym, xm, dx, dy, xorg, yorg, slw;
    register int x, y, e;
    miFillArcRec info;
    miArcSliceRec slice;
    int xl, xr, xc;
    PixelType *addrlt, *addrlb;
    int nlwidth;
    int width;
    PixelType startmask, endmask;
    int nlmiddle;

    mfbGetPixelWidthAndPointer(pDraw, nlwidth, addrlt);
    miFillArcSetup(arc, &info);
    miFillArcSliceSetup(arc, &slice, pGC);
    MIFILLARCSETUP();
    xorg += pDraw->x;
    yorg += pDraw->y;
    addrlb = addrlt;
    addrlt = mfbScanlineDeltaNoBankSwitch(addrlt, yorg - y, nlwidth);
    addrlb = mfbScanlineDeltaNoBankSwitch(addrlb, yorg + y + dy, nlwidth);
    slice.edge1.x += pDraw->x;
    slice.edge2.x += pDraw->x;
    while (y > 0)
    {
	mfbScanlineIncNoBankSwitch(addrlt, nlwidth);
	mfbScanlineIncNoBankSwitch(addrlb, -nlwidth);
	MIFILLARCSTEP(slw);
	MIARCSLICESTEP(slice.edge1);
	MIARCSLICESTEP(slice.edge2);
	if (miFillSliceUpper(slice))
	{
	    MIARCSLICEUPPER(xl, xr, slice, slw);
	    FILLSLICESPANS(slice.flip_top, addrlt);
	}
	if (miFillSliceLower(slice))
	{
	    MIARCSLICELOWER(xl, xr, slice, slw);
	    FILLSLICESPANS(slice.flip_bot, addrlb);
	}
    }
}

void
mfbPolyFillArcSolid(pDraw, pGC, narcs, parcs)
    register DrawablePtr pDraw;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    mfbPrivGC *priv;
    register xArc *arc;
    register int i;
    BoxRec box;
    int x2, y2;
    RegionPtr cclip;
    int rop;

    priv = (mfbPrivGC *) pGC->devPrivates[mfbGCPrivateIndex].ptr;
    rop = priv->rop;
    if ((rop == RROP_NOP) || !(pGC->planemask & 1))
	return;
    cclip = pGC->pCompositeClip;
    for (arc = parcs, i = narcs; --i >= 0; arc++)
    {
	if (miFillArcEmpty(arc))
	    continue;
	if (miCanFillArc(arc))
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
	    {
		if ((arc->angle2 >= FULLCIRCLE) ||
		    (arc->angle2 <= -FULLCIRCLE))
		    mfbFillEllipseSolid(pDraw, arc, rop);
		else
		    mfbFillArcSliceSolidCopy(pDraw, pGC, arc, rop);
		continue;
	    }
	}
	miPolyFillArc(pDraw, pGC, 1, arc);
    }
}
