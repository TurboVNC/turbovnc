/*
 * $Xorg: cfbply1rct.c,v 1.4 2001/02/09 02:04:38 xorgcvs Exp $
 *
Copyright 1990, 1998  The Open Group

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
 *
 * Author:  Keith Packard, MIT X Consortium
 */
/* $XFree86: xc/programs/Xserver/cfb/cfbply1rct.c,v 3.10 2003/10/29 22:44:53 tsi Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>

#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "mistruct.h"

#include "cfb.h"
#include "cfbmskbits.h"
#include "cfbrrop.h"

void
RROP_NAME(cfbFillPoly1Rect) (pDrawable, pGC, shape, mode, count, ptsIn)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		shape;
    int		mode;
    int		count;
    DDXPointPtr	ptsIn;
{
    cfbPrivGCPtr    devPriv;
    int		    nwidth;
    CfbBits	    *addrl, *addr;
#if PSZ == 24
    CfbBits	    startmask, endmask;
    register int    pidx;
#else
#if PPW > 1
    CfbBits	    mask, bits = ~((CfbBits)0);
#endif
#endif
    int		    maxy;
    int		    origin;
    register int    vertex1, vertex2;
    int		    c = 0;
    BoxPtr	    extents;
    int		    clip;
    int		    y;
    int		    *vertex1p = NULL, *vertex2p;
    int		    *endp;
    int		    x1 = 0, x2 = 0;
    int		    dx1 = 0, dx2 = 0;
    int		    dy1 = 0, dy2 = 0;
    int		    e1 = 0, e2 = 0;
    int		    step1 = 0, step2 = 0;
    int		    sign1 = 0, sign2 = 0;
    int		    h;
    int		    l;
#if PSZ != 24 && PPW > 1
    int		    r;
#endif
    int		    nmiddle;
    RROP_DECLARE

    if (mode == CoordModePrevious)
    {
	miFillPolygon (pDrawable, pGC, shape, mode, count, ptsIn);
	return;
    }
    
    devPriv = cfbGetGCPrivate(pGC);
#ifdef NO_ONE_RECT
    if (REGION_NUM_RECTS(pGC->pCompositeClip) != 1)
    {
	miFillPolygon (pDrawable, pGC, shape, mode, count, ptsIn);
	return;
    }
#endif
    origin = *((int *) &pDrawable->x);
    vertex2 = origin - ((origin & 0x8000) << 1);
    extents = &pGC->pCompositeClip->extents;
    RROP_FETCH_GCPRIV(devPriv);
    vertex1 = *((int *) &extents->x1) - vertex2;
    vertex2 = *((int *) &extents->x2) - vertex2 - 0x00010001;
    clip = 0;
    y = 32767;
    maxy = 0;
    vertex2p = (int *) ptsIn;
    endp = vertex2p + count;
    if (shape == Convex)
    {
    	while (count--)
    	{
	    c = *vertex2p;
	    clip |= (c - vertex1) | (vertex2 - c);
	    c = intToY(c);
	    if (c < y) 
	    {
	    	y = c;
	    	vertex1p = vertex2p;
	    }
	    vertex2p++;
	    if (c > maxy)
	    	maxy = c;
    	}
    }
    else
    {
	int yFlip = 0;
	dx1 = 1;
	x2 = -1;
	x1 = -1;
    	while (count--)
    	{
	    c = *vertex2p;
	    clip |= (c - vertex1) | (vertex2 - c);
	    c = intToY(c);
	    if (c < y) 
	    {
	    	y = c;
	    	vertex1p = vertex2p;
	    }
	    vertex2p++;
	    if (c > maxy)
	    	maxy = c;
	    if (c == x1)
		continue;
	    if (dx1 > 0)
	    {
		if (x2 < 0)
		    x2 = c;
		else
		    dx2 = dx1 = (c - x1) >> 31;
	    }
	    else
		if ((c - x1) >> 31 != dx1) 
		{
		    dx1 = ~dx1;
		    yFlip++;
		}
	    x1 = c;
       	}
	x1 = (x2 - c) >> 31;
	if (x1 != dx1)
	    yFlip++;
	if (x1 != dx2)
	    yFlip++;
	if (yFlip != 2) 
	    clip = 0x8000;
    }
    if (y == maxy)
	return;

    if (clip & 0x80008000)
    {
	miFillPolygon (pDrawable, pGC, shape, mode, vertex2p - (int *) ptsIn, ptsIn);
	return;
    }

#define AddrYPlus(a,y)  (CfbBits *) (((unsigned char *) (a)) + (y) * nwidth)

    cfbGetTypedWidthAndPointer(pDrawable, nwidth, addrl, unsigned char, CfbBits);
    addrl = AddrYPlus(addrl,y + pDrawable->y);
    origin = intToX(origin);
    vertex2p = vertex1p;
    vertex2 = vertex1 = *vertex2p++;
    if (vertex2p == endp)
	vertex2p = (int *) ptsIn;
#define Setup(c,x,vertex,dx,dy,e,sign,step) {\
    x = intToX(vertex); \
    if ((dy = intToY(c) - y)) { \
    	dx = intToX(c) - x; \
	step = 0; \
    	if (dx >= 0) \
    	{ \
	    e = 0; \
	    sign = 1; \
	    if (dx >= dy) {\
	    	step = dx / dy; \
	    	dx = dx % dy; \
	    } \
    	} \
    	else \
    	{ \
	    e = 1 - dy; \
	    sign = -1; \
	    dx = -dx; \
	    if (dx >= dy) { \
		step = - (dx / dy); \
		dx = dx % dy; \
	    } \
    	} \
    } \
    x += origin; \
    vertex = c; \
}

#define Step(x,dx,dy,e,sign,step) {\
    x += step; \
    if ((e += dx) > 0) \
    { \
	x += sign; \
	e -= dy; \
    } \
}
    for (;;)
    {
	if (y == intToY(vertex1))
	{
	    do
	    {
	    	if (vertex1p == (int *) ptsIn)
		    vertex1p = endp;
	    	c = *--vertex1p;
	    	Setup (c,x1,vertex1,dx1,dy1,e1,sign1,step1)
	    } while (y >= intToY(vertex1));
	    h = dy1;
	}
	else
	{
	    Step(x1,dx1,dy1,e1,sign1,step1)
	    h = intToY(vertex1) - y;
	}
	if (y == intToY(vertex2))
	{
	    do
	    {
	    	c = *vertex2p++;
	    	if (vertex2p == endp)
		    vertex2p = (int *) ptsIn;
	    	Setup (c,x2,vertex2,dx2,dy2,e2,sign2,step2)
	    } while (y >= intToY(vertex2));
	    if (dy2 < h)
		h = dy2;
	}
	else
	{
	    Step(x2,dx2,dy2,e2,sign2,step2)
	    if ((c = (intToY(vertex2) - y)) < h)
		h = c;
	}
	/* fill spans for this segment */
	y += h;
	for (;;)
	{
	    l = x1;
#if PSZ != 24 && PPW > 1
	    r = x2;
#endif
	    nmiddle = x2 - x1;
    	    if (nmiddle < 0)
	    {
	    	nmiddle = -nmiddle;
	    	l = x2;
#if PSZ != 24 && PPW > 1
	    	r = x1;
#endif
    	    }
#if PPW > 1
	    c = l & PIM;
	    l -= c;
#endif

#if PGSZ == 32
#define LWRD_SHIFT 2
#else /* PGSZ == 64 */
#define LWRD_SHIFT 3
#endif /* PGSZ */

#if PSZ == 24
	    addr = (CfbBits *)((char *)addrl + ((l * 3) & ~0x03));
	    if (nmiddle <= 1){
	      if (nmiddle)
	        RROP_SOLID24(addr, l);
	    } else {
	      maskbits(l, nmiddle, startmask, endmask, nmiddle);
	      pidx = l & 3;
	      if (startmask){
		RROP_SOLID_MASK(addr, startmask, pidx-1);
		addr++;
		if (pidx == 3)
		  pidx = 0;
	      }
	      while (--nmiddle >= 0){
		RROP_SOLID(addr, pidx);
		addr++;
		if (++pidx == 3)
		  pidx = 0;
	      }
	      if (endmask)
		RROP_SOLID_MASK(addr, endmask, pidx);
	    }
#else /* PSZ == 24 */
#if PWSH > LWRD_SHIFT
	    l = l >> (PWSH - LWRD_SHIFT);
#endif
#if PWSH < LWRD_SHIFT
	    l = l << (LWRD_SHIFT - PWSH);
#endif
	    addr = (CfbBits *) (((char *) addrl) + l);
#if PPW > 1
	    if (c + nmiddle < PPW)
	    {
	    	mask = SCRRIGHT (bits,c) ^ SCRRIGHT (bits,c+nmiddle);
	    	RROP_SOLID_MASK(addr,mask);
	    }
	    else
	    {
	    	if (c)
	    	{
	    	    mask = SCRRIGHT(bits, c);
	    	    RROP_SOLID_MASK(addr,mask);
	    	    nmiddle += c - PPW;
	    	    addr++;
	    	}
#endif
	    	nmiddle >>= PWSH;
		while (--nmiddle >= 0) {
		    RROP_SOLID(addr); addr++;
		}
#if PPW > 1
	    	if ((mask = ~SCRRIGHT(bits, r & PIM)))
	    	    RROP_SOLID_MASK(addr,mask);
	    }
#endif
#endif /* PSZ == 24 */
	    if (!--h)
		break;
	    addrl = AddrYPlus (addrl, 1);
	    Step(x1,dx1,dy1,e1,sign1,step1)
	    Step(x2,dx2,dy2,e2,sign2,step2)
	}
	if (y == maxy)
	    break;
	addrl = AddrYPlus (addrl, 1);
    }
    RROP_UNDECLARE
}
