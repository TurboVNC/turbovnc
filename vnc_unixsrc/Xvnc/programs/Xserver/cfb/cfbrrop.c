/*
 * $XConsortium: cfbrrop.c,v 1.8 94/04/17 20:28:59 dpw Exp $
 *
Copyright (c) 1989  X Consortium

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
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/* cfb reduced rasterop computations */

#include "X.h"
#include "Xmd.h"
#include "Xproto.h"
#include "cfb.h"
#include "cfbmskbits.h"

/* A description:
 *
 * There are four possible operations on each bit in the destination word,
 *
 *	    1	2   3	4
 *
 *    0	    0	0   1	1
 *    1	    0	1   0	1
 *
 * On examination of the reduced rop equation (dst = (dst & and) ^ xor),
 * these four fall to reduced rops as follows:
 *
 *  and	    0	1   1	0
 *  xor	    0	0   1	1
 *
 * or, (if 'and' is expensive) (dst = (dst | or) ^ xor)
 *
 *  or	    1	0   0	1
 *  xor	    1	0   1	0
 *
 * The trouble with using this later equation is that trivial
 * rasterop reduction is more difficult; some common rasterops
 * use complicated expressions of xor/and instead of the simple
 * ones while other common rasterops are not made any simpler:
 *
 * GXcopy:	*dst = ~xor		instead of  *dst = xor
 * GXand:	*dst = *dst & ~or	instead of  *dst = *dst & and
 * GXor:	*dst = *dst | or	instead of  *dst = *dst | xor
 * GXxor:	*dst = *dst ^ xor	instead of  *dst = *dst ^ xor
 *
 * If you're really set on using this second mechanism, the changes
 * are pretty simple.
 *
 * All that remains is to provide a mechanism for computing and/xor values
 * based on the raster op and foreground value.
 *
 * The 16 rops fall as follows, with the associated reduced
 * rop and/xor and or/xor values.  The values in parenthesis following the
 * reduced values gives an equation using the source value for
 * the reduced value, and is one of {0, src, ~src, 1} as appropriate.
 *
 *	clear		and		andReverse	copy
 *     src  0	1	    0   1	    0	1	    0	1
 *  dst	0   0	0	0   0   0	0   0	1	0   0	1
 *	1   0	0	1   0   1	1   0	0	1   0	1
 *
 *  and	    0	0 (0)	    0   1 (src)	    0	1 (src)	    0	0 (0)
 *  xor	    0	0 (0)	    0   0 (0)	    0	1 (src)	    0	1 (src)
 *
 *  or	    1	1 (1)	    1	0 (~src)    1	0 (~src)    1	1 (1)
 *  xor	    1	1 (1)	    1	0 (~src)    1	1 (1)	    1	0 (~src)
 *
 *	andInverted	noop		xor		or
 *     src  0	1	    0   1	    0	1	    0	1
 *  dst	0   0	0	0   0   0	0   0	1	0   0	1
 *	1   1	0	1   1   1	1   1	0	1   1	1
 *
 *  and	    1	0 (~src)    1   1 (1)	    1	1 (1)	    1	0 (~src)
 *  xor	    0	0 (0)	    0   0 (0)	    0	1 (src)	    0	1 (src)
 *
 *  or	    0	1 (src)	    0	0 (0)	    0	0 (0)	    0	1 (src)
 *  xor	    0	1 (src)	    0	0 (0)	    0	1 (src)	    0	0 (0)
 *
 *	nor		equiv		invert		orReverse
 *     src  0	1	    0   1	    0	1	    0	1
 *  dst	0   1	0	0   1   0	0   1	1	0   1	1
 *	1   0	0	1   0   1	1   0	0	1   0	1
 *
 *  and	    1	0 (~src)    1   1 (1)	    1	1 (1)	    1	0 (~src)
 *  xor	    1	0 (~src)    1   0 (~src)    1	1 (1)	    1	1 (1)
 *
 *  or	    0	1 (src)	    0	0 (0)	    0	0 (0)	    0	1 (src)
 *  xor	    1	1 (1)	    1	0 (~src)    1	1 (1)	    1	0 (~src)
 *
 *	copyInverted	orInverted	nand		set
 *     src  0	1	    0   1	    0	1	    0	1
 *  dst	0   1	0	0   1   0	0   1	1	0   1	1
 *	1   1	0	1   1   1	1   1	0	1   1	1
 *
 *  and	    0	0 (0)	    0   1 (src)	    0	1 (src)	    0	0 (0)
 *  xor	    1	0 (~src)    1   0 (~src)    1	1 (1)	    1	1 (1)
 *
 *  or	    1	1 (1)	    1	0 (~src)    1	0 (~src)    1	1 (1)
 *  xor	    0	1 (src)	    0	0 (0)	    0	1 (src)	    0	0 (0)
 */

int
cfbReduceRasterOp (rop, fg, pm, andp, xorp)
    int		    rop;
    unsigned long   fg, pm;
    unsigned long   *andp, *xorp;
{
    unsigned long   and, xor;
    int		    rrop;

    fg = PFILL (fg);
    pm = PFILL (pm);
    switch (rop)
    {
    case GXclear:
    	and = 0;
    	xor = 0;
	break;
    case GXand:
	and = fg;
	xor = 0;
	break;
    case GXandReverse:
	and = fg;
	xor = fg;
	break;
    case GXcopy:
	and = 0;
	xor = fg;
	break;
    case GXandInverted:
	and = ~fg;
	xor = 0;
	break;
    case GXnoop:
	and = ~0;
	xor = 0;
	break;
    case GXxor:
	and = ~0;
	xor = fg;
	break;
    case GXor:
	and = ~fg;
	xor = fg;
	break;
    case GXnor:
	and = ~fg;
	xor = ~fg;
	break;
    case GXequiv:
	and = ~0;
	xor = ~fg;
	break;
    case GXinvert:
	and = ~0;
	xor = ~0;
	break;
    case GXorReverse:
	and = ~fg;
	xor = ~0;
	break;
    case GXcopyInverted:
	and = 0;
	xor = ~fg;
	break;
    case GXorInverted:
	and = fg;
	xor = ~fg;
	break;
    case GXnand:
	and = fg;
	xor = ~0;
	break;
    case GXset:
	and = 0;
	xor = ~0;
	break;
    }
    and |= ~pm;
    xor &= pm;
    *andp = and;
    *xorp = xor;
    /* check for some special cases to reduce computation */
    if (and == 0)
	rrop = GXcopy;
    /* nothing checks for GXnoop
    else if (and == ~0 && xor == 0)
	rrop = GXnoop;
    */
    else if (and == ~0)
	rrop = GXxor;
    else if (xor == 0)
	rrop = GXand;
    else if ( (and ^ xor) == ~0) /* fix XBUG 6541 */
	rrop = GXor;
    else
	rrop = GXset;   /* rop not reduced */
    return rrop;
}
