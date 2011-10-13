/* $XFree86: xc/programs/Xserver/mfb/mfbbresd.c,v 1.4 2001/01/17 22:37:02 dawes Exp $ */
/***********************************************************

Copyright 1987, 1998  The Open Group

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
/* $Xorg: mfbbresd.c,v 1.4 2001/02/09 02:05:18 xorgcvs Exp $ */
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include "misc.h"
#include "mfb.h"
#include "maskbits.h"
#include "miline.h"

/* Dashed bresenham line */

#define StepDash\
    if (!--dashRemaining) { \
	if (++ dashIndex == numInDashList) \
	    dashIndex = 0; \
	dashRemaining = pDash[dashIndex]; \
	rop = fgrop; \
	if (dashIndex & 1) \
	    rop = bgrop; \
    }

void
mfbBresD(fgrop, bgrop,
	 pdashIndex, pDash, numInDashList, pdashOffset, isDoubleDash,
	 addrlbase, nlwidth,
	 signdx, signdy, axis, x1, y1, e, e1, e2, len)
int fgrop, bgrop;
int *pdashIndex;	/* current dash */
unsigned char *pDash;	/* dash list */
int numInDashList;	/* total length of dash list */
int *pdashOffset;	/* offset into current dash */
int isDoubleDash;
PixelType *addrlbase;	/* pointer to base of bitmap */
int nlwidth;		/* width in longwords of bitmap */
int signdx, signdy;	/* signs of directions */
int axis;		/* major axis (Y_AXIS or X_AXIS) */
int x1, y1;		/* initial point */
register int e;		/* error accumulator */
register int e1;	/* bresenham increments */
int e2;
int len;		/* length of line */
{
    register int yinc;	/* increment to next scanline, in bytes */
    register PixelType *addrl;
    register int e3 = e2-e1;
    register MfbBits bit;
    PixelType leftbit = mask[0]; /* leftmost bit to process in new word */
    PixelType rightbit = mask[PPW-1]; /* rightmost bit to process in new word */
    int dashIndex;
    int dashOffset;
    int dashRemaining;
    int	rop;

    dashOffset = *pdashOffset;
    dashIndex = *pdashIndex;
    dashRemaining = pDash[dashIndex] - dashOffset;
    rop = fgrop;
    if (!isDoubleDash)
	bgrop = -1;
    if (dashIndex & 1)
	rop = bgrop;

    /* point to longword containing first point */
    addrl = mfbScanline(addrlbase, x1, y1, nlwidth);
    yinc = signdy * nlwidth;
    e = e-e1;			/* to make looping easier */
    bit = mask[x1 & PIM];
    if (axis == X_AXIS)
    {
	if (signdx > 0)
	{
	    while(len--)
	    { 
		if (rop == RROP_BLACK)
		    *addrl &= ~bit;
		else if (rop == RROP_WHITE)
		    *addrl |= bit;
		else if (rop == RROP_INVERT)
		    *addrl ^= bit;
		e += e1;
		if (e >= 0)
		{
		    mfbScanlineInc(addrl, yinc);
		    e += e3;
		}
		bit = SCRRIGHT(bit,1);
		if (!bit) { bit = leftbit;addrl ++; }
		StepDash
	    }
	}
	else
	{
	    while(len--)
	    { 
		if (rop == RROP_BLACK)
		    *addrl &= ~bit;
		else if (rop == RROP_WHITE)
		    *addrl |= bit;
		else if (rop == RROP_INVERT)
		    *addrl ^= bit;
		e += e1;
		if (e >= 0)
		{
		    mfbScanlineInc(addrl, yinc);
		    e += e3;
		}
		bit = SCRLEFT(bit,1);
		if (!bit) { bit = rightbit;addrl --; }
		StepDash
	    }
	}
    } /* if X_AXIS */
    else
    {
	if (signdx > 0)
	{
	    while(len--)
	    {
		if (rop == RROP_BLACK)
		    *addrl &= ~bit;
		else if (rop == RROP_WHITE)
		    *addrl |= bit;
		else if (rop == RROP_INVERT)
		    *addrl ^= bit;
		e += e1;
		if (e >= 0)
		{
		    bit = SCRRIGHT(bit,1);
		    if (!bit) { bit = leftbit;addrl ++; }
		    e += e3;
		}
		mfbScanlineInc(addrl, yinc);
		StepDash
	    }
	}
	else
	{
	    while(len--)
	    {
		if (rop == RROP_BLACK)
		    *addrl &= ~bit;
		else if (rop == RROP_WHITE)
		    *addrl |= bit;
		else if (rop == RROP_INVERT)
		    *addrl ^= bit;
		e += e1;
		if (e >= 0)
		{
		    bit = SCRLEFT(bit,1);
		    if (!bit) { bit = rightbit;addrl --; }
		    e += e3;
		}
		mfbScanlineInc(addrl, yinc);
		StepDash
	    }
	}
    } /* else Y_AXIS */
    *pdashIndex = dashIndex;
    *pdashOffset = pDash[dashIndex] - dashRemaining;
} 
