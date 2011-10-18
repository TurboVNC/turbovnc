/* Combined Purdue/PurduePlus patches, level 2.0, 1/17/89 */
/***********************************************************

Copyright (c) 1987  X Consortium

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


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/* $XConsortium: mfbhrzvert.c,v 1.15 94/04/17 20:28:24 dpw Exp $ */
#include "X.h"

#include "gc.h"
#include "window.h"
#include "pixmap.h"
#include "region.h"

#include "mfb.h"
#include "maskbits.h"

/* horizontal solid line
   abs(len) > 1
*/
mfbHorzS(rop, addrl, nlwidth, x1, y1, len)
int rop;		/* a reduced rasterop */
register PixelType *addrl;	/* pointer to base of bitmap */
register int nlwidth;	/* width in longwords of bitmap */
int x1;			/* initial point */ 
int y1;
int len;		/* length of line */
{
    register PixelType startmask;
    register PixelType endmask;
    register int nlmiddle;


    /* force the line to go left to right
       but don't draw the last point
    */
    if (len < 0)
    {
	x1 += len;
	x1 += 1;
	len = -len;
    }

    addrl = mfbScanline(addrl, x1, y1, nlwidth);

    /* all bits inside same longword */
    if ( ((x1 & PIM) + len) < PPW)
    {
	maskpartialbits(x1, len, startmask);
        if (rop == RROP_BLACK)
        {
	    *addrl &= ~startmask;
        }
        else if (rop == RROP_WHITE)
        {
	    *addrl |= startmask;
        }
        else if (rop == RROP_INVERT)
        {
	    *addrl ^= startmask;
        }
    }
    else
    {
	maskbits(x1, len, startmask, endmask, nlmiddle);
        if (rop == RROP_BLACK)
        {
	    if (startmask)
		*addrl++ &= ~startmask;
	    Duff (nlmiddle, *addrl++ = 0x0);
	    if (endmask)
		*addrl &= ~endmask;
        }
        else if (rop == RROP_WHITE)
        {
	    if (startmask)
		*addrl++ |= startmask;
	    Duff (nlmiddle, *addrl++ = ~0);
	    if (endmask)
		*addrl |= endmask;
        }
        else if (rop == RROP_INVERT)
        {
	    if (startmask)
		*addrl++ ^= startmask;
	    Duff (nlmiddle, *addrl++ ^= ~0);
	    if (endmask)
		*addrl ^= endmask;
        }
    }
}

/* vertical solid line
   this uses do loops because pcc (Ultrix 1.2, bsd 4.2) generates
   better code.  sigh.  we know that len will never be 0 or 1, so
   it's OK to use it.
*/

mfbVertS(rop, addrl, nlwidth, x1, y1, len)
int rop;		/* a reduced rasterop */
register PixelType *addrl;	/* pointer to base of bitmap */
register int nlwidth;	/* width in longwords of bitmap */
int x1, y1;		/* initial point */
register int len;	/* length of line */
{
    register PixelType bitmask;

    addrl = mfbScanline(addrl, x1, y1, nlwidth);

    if (len < 0)
    {
	nlwidth = -nlwidth;
	len = -len;
    }
 
    if (rop == RROP_BLACK)
    {
	bitmask = rmask[x1 & PIM];
        Duff(len, *addrl &= bitmask; mfbScanlineInc(addrl, nlwidth) );
    }
    else if (rop == RROP_WHITE)
    {
	bitmask = mask[x1 & PIM];
        Duff(len, *addrl |= bitmask; mfbScanlineInc(addrl, nlwidth) );
    }
    else if (rop == RROP_INVERT)
    {
	bitmask = mask[x1 & PIM];
        Duff(len, *addrl ^= bitmask; mfbScanlineInc(addrl, nlwidth) );
    }
}
