/*
 * $Xorg: mfbply1rct.c,v 1.4 2001/02/09 02:05:19 xorgcvs Exp $
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

/* $XFree86: xc/programs/Xserver/mfb/mfbply1rct.c,v 1.7tsi Exp $ */

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

#include "mfb.h"
#include "maskbits.h"

#if defined(mips) || defined(sparc)
#define GetHighWord(x) (((int) (x)) >> 16)
#else
#define GetHighWord(x) (((int) (x)) / 65536)
#endif

#if IMAGE_BYTE_ORDER == MSBFirst
#define intToCoord(i,x,y)   (((x) = GetHighWord(i)), ((y) = (int) ((short) (i))))
#define coordToInt(x,y)	(((x) << 16) | ((y) & 0xffff))
#define intToX(i)	(GetHighWord(i))
#define intToY(i)	((int) ((short) i))
#else
#define intToCoord(i,x,y)   (((x) = (int) ((short) (i))), ((y) = GetHighWord(i)))
#define coordToInt(x,y)	(((y) << 16) | ((x) & 0xffff))
#define intToX(i)	((int) ((short) (i)))
#define intToY(i)	(GetHighWord(i))
#endif

void
MFBFILLPOLY1RECT (pDrawable, pGC, shape, mode, count, ptsIn)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		shape;
    int		mode;
    int		count;
    DDXPointPtr	ptsIn;
{
    int		    nlwidth;
    PixelType	    *addrl, *addr;
    int		    maxy;
    int		    origin;
    register int    vertex1, vertex2;
    int		    c;
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
    int		    l, r;
    PixelType	    mask, bits = ~((PixelType)0);
    int		    nmiddle;

    if (mode == CoordModePrevious || shape != Convex ||
	REGION_NUM_RECTS(pGC->pCompositeClip) != 1)
    {
	miFillPolygon (pDrawable, pGC, shape, mode, count, ptsIn);
	return;
    }
    origin = *((int *) &pDrawable->x);
    vertex2 = origin - ((origin & 0x8000) << 1);
    extents = &pGC->pCompositeClip->extents;
    vertex1 = *((int *) &extents->x1) - vertex2;
    vertex2 = *((int *) &extents->x2) - vertex2 - 0x00010001;
    clip = 0;
    y = 32767;
    maxy = 0;
    vertex2p = (int *) ptsIn;
    endp = vertex2p + count;
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
    if (y == maxy)
	return;

    if (clip & 0x80008000)
    {
	miFillPolygon (pDrawable, pGC, shape, mode, vertex2p - (int *) ptsIn, ptsIn);
	return;
    }

    mfbGetPixelWidthAndPointer(pDrawable, nlwidth, addrl);
    addrl = mfbScanlineDelta(addrl, y + pDrawable->y, nlwidth);
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
	    r = x2;
	    nmiddle = x2 - x1;
    	    if (nmiddle < 0)
	    {
	    	nmiddle = -nmiddle;
	    	l = x2;
	    	r = x1;
    	    }
	    c = l & PIM;
	    l -= c;
	    l = l >> PWSH;
	    addr = addrl + l;
	    if (c + nmiddle < PPW)
	    {
	    	mask = SCRRIGHT (bits,c) ^ SCRRIGHT (bits,c+nmiddle);
	    	*addr OPEQ mask;
	    }
	    else
	    {
	    	if (c)
	    	{
	    	    mask = SCRRIGHT(bits, c);
		    *addr OPEQ mask;
	    	    nmiddle += c - PPW;
	    	    addr++;
	    	}
	    	nmiddle >>= PWSH;
		Duff (nmiddle, *addr++ EQWHOLEWORD)
	    	if ((mask = ~SCRRIGHT(bits, r & PIM)))
	    	    *addr OPEQ mask;
	    }
	    if (!--h)
		break;
	    mfbScanlineInc(addrl, nlwidth);
	    Step(x1,dx1,dy1,e1,sign1,step1)
	    Step(x2,dx2,dy2,e2,sign2,step2)
	}
	if (y == maxy)
	    break;
	mfbScanlineInc(addrl, nlwidth);
    }
}
