/* $Xorg: cfb8bit.c,v 1.4 2001/02/09 02:04:37 xorgcvs Exp $ */
/*

Copyright 1989, 1994, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/
/* $XFree86: xc/programs/Xserver/cfb/cfb8bit.c,v 1.4 2001/01/17 22:36:34 dawes Exp $ */

/*
 * cfb8bit.c
 *
 * 8 bit color frame buffer utility routines
 */


#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#if PSZ == 8

#include	<X11/X.h>
#include	<X11/Xmd.h>
#include	<X11/Xproto.h>
#include	"gcstruct.h"
#include	"windowstr.h"
#include	"scrnintstr.h"
#include	"pixmapstr.h"
#include	"regionstr.h"
#include	"cfb.h"
#include	"cfbmskbits.h"
#include	"cfb8bit.h"

PixelGroup cfb8StippleMasks[NUM_MASKS] = {
#if NUM_MASKS == 16
    0x00000000, 0x000000ff, 0x0000ff00, 0x0000ffff,
    0x00ff0000, 0x00ff00ff, 0x00ffff00, 0x00ffffff,
    0xff000000, 0xff0000ff, 0xff00ff00, 0xff00ffff,
    0xffff0000, 0xffff00ff, 0xffffff00, 0xffffffff
#else /* NUM_MASKS == 256 */
    0x0000000000000000,	0x00000000000000ff,
    0x000000000000ff00,	0x000000000000ffff,
    0x0000000000ff0000,	0x0000000000ff00ff,
    0x0000000000ffff00,	0x0000000000ffffff,
    0x00000000ff000000,	0x00000000ff0000ff,
    0x00000000ff00ff00,	0x00000000ff00ffff,
    0x00000000ffff0000,	0x00000000ffff00ff,
    0x00000000ffffff00,	0x00000000ffffffff,
    0x000000ff00000000,	0x000000ff000000ff,
    0x000000ff0000ff00,	0x000000ff0000ffff,
    0x000000ff00ff0000,	0x000000ff00ff00ff,
    0x000000ff00ffff00,	0x000000ff00ffffff,
    0x000000ffff000000,	0x000000ffff0000ff,
    0x000000ffff00ff00,	0x000000ffff00ffff,
    0x000000ffffff0000,	0x000000ffffff00ff,
    0x000000ffffffff00,	0x000000ffffffffff,
    0x0000ff0000000000,	0x0000ff00000000ff,
    0x0000ff000000ff00,	0x0000ff000000ffff,
    0x0000ff0000ff0000,	0x0000ff0000ff00ff,
    0x0000ff0000ffff00,	0x0000ff0000ffffff,
    0x0000ff00ff000000,	0x0000ff00ff0000ff,
    0x0000ff00ff00ff00,	0x0000ff00ff00ffff,
    0x0000ff00ffff0000,	0x0000ff00ffff00ff,
    0x0000ff00ffffff00,	0x0000ff00ffffffff,
    0x0000ffff00000000,	0x0000ffff000000ff,
    0x0000ffff0000ff00,	0x0000ffff0000ffff,
    0x0000ffff00ff0000,	0x0000ffff00ff00ff,
    0x0000ffff00ffff00,	0x0000ffff00ffffff,
    0x0000ffffff000000,	0x0000ffffff0000ff,
    0x0000ffffff00ff00,	0x0000ffffff00ffff,
    0x0000ffffffff0000,	0x0000ffffffff00ff,
    0x0000ffffffffff00,	0x0000ffffffffffff,
    0x00ff000000000000,	0x00ff0000000000ff,
    0x00ff00000000ff00,	0x00ff00000000ffff,
    0x00ff000000ff0000,	0x00ff000000ff00ff,
    0x00ff000000ffff00,	0x00ff000000ffffff,
    0x00ff0000ff000000,	0x00ff0000ff0000ff,
    0x00ff0000ff00ff00,	0x00ff0000ff00ffff,
    0x00ff0000ffff0000,	0x00ff0000ffff00ff,
    0x00ff0000ffffff00,	0x00ff0000ffffffff,
    0x00ff00ff00000000,	0x00ff00ff000000ff,
    0x00ff00ff0000ff00,	0x00ff00ff0000ffff,
    0x00ff00ff00ff0000,	0x00ff00ff00ff00ff,
    0x00ff00ff00ffff00,	0x00ff00ff00ffffff,
    0x00ff00ffff000000,	0x00ff00ffff0000ff,
    0x00ff00ffff00ff00,	0x00ff00ffff00ffff,
    0x00ff00ffffff0000,	0x00ff00ffffff00ff,
    0x00ff00ffffffff00,	0x00ff00ffffffffff,
    0x00ffff0000000000,	0x00ffff00000000ff,
    0x00ffff000000ff00,	0x00ffff000000ffff,
    0x00ffff0000ff0000,	0x00ffff0000ff00ff,
    0x00ffff0000ffff00,	0x00ffff0000ffffff,
    0x00ffff00ff000000,	0x00ffff00ff0000ff,
    0x00ffff00ff00ff00,	0x00ffff00ff00ffff,
    0x00ffff00ffff0000,	0x00ffff00ffff00ff,
    0x00ffff00ffffff00,	0x00ffff00ffffffff,
    0x00ffffff00000000,	0x00ffffff000000ff,
    0x00ffffff0000ff00,	0x00ffffff0000ffff,
    0x00ffffff00ff0000,	0x00ffffff00ff00ff,
    0x00ffffff00ffff00,	0x00ffffff00ffffff,
    0x00ffffffff000000,	0x00ffffffff0000ff,
    0x00ffffffff00ff00,	0x00ffffffff00ffff,
    0x00ffffffffff0000,	0x00ffffffffff00ff,
    0x00ffffffffffff00,	0x00ffffffffffffff,
    0xff00000000000000,	0xff000000000000ff,
    0xff0000000000ff00,	0xff0000000000ffff,
    0xff00000000ff0000,	0xff00000000ff00ff,
    0xff00000000ffff00,	0xff00000000ffffff,
    0xff000000ff000000,	0xff000000ff0000ff,
    0xff000000ff00ff00,	0xff000000ff00ffff,
    0xff000000ffff0000,	0xff000000ffff00ff,
    0xff000000ffffff00,	0xff000000ffffffff,
    0xff0000ff00000000,	0xff0000ff000000ff,
    0xff0000ff0000ff00,	0xff0000ff0000ffff,
    0xff0000ff00ff0000,	0xff0000ff00ff00ff,
    0xff0000ff00ffff00,	0xff0000ff00ffffff,
    0xff0000ffff000000,	0xff0000ffff0000ff,
    0xff0000ffff00ff00,	0xff0000ffff00ffff,
    0xff0000ffffff0000,	0xff0000ffffff00ff,
    0xff0000ffffffff00,	0xff0000ffffffffff,
    0xff00ff0000000000,	0xff00ff00000000ff,
    0xff00ff000000ff00,	0xff00ff000000ffff,
    0xff00ff0000ff0000,	0xff00ff0000ff00ff,
    0xff00ff0000ffff00,	0xff00ff0000ffffff,
    0xff00ff00ff000000,	0xff00ff00ff0000ff,
    0xff00ff00ff00ff00,	0xff00ff00ff00ffff,
    0xff00ff00ffff0000,	0xff00ff00ffff00ff,
    0xff00ff00ffffff00,	0xff00ff00ffffffff,
    0xff00ffff00000000,	0xff00ffff000000ff,
    0xff00ffff0000ff00,	0xff00ffff0000ffff,
    0xff00ffff00ff0000,	0xff00ffff00ff00ff,
    0xff00ffff00ffff00,	0xff00ffff00ffffff,
    0xff00ffffff000000,	0xff00ffffff0000ff,
    0xff00ffffff00ff00,	0xff00ffffff00ffff,
    0xff00ffffffff0000,	0xff00ffffffff00ff,
    0xff00ffffffffff00,	0xff00ffffffffffff,
    0xffff000000000000,	0xffff0000000000ff,
    0xffff00000000ff00,	0xffff00000000ffff,
    0xffff000000ff0000,	0xffff000000ff00ff,
    0xffff000000ffff00,	0xffff000000ffffff,
    0xffff0000ff000000,	0xffff0000ff0000ff,
    0xffff0000ff00ff00,	0xffff0000ff00ffff,
    0xffff0000ffff0000,	0xffff0000ffff00ff,
    0xffff0000ffffff00,	0xffff0000ffffffff,
    0xffff00ff00000000,	0xffff00ff000000ff,
    0xffff00ff0000ff00,	0xffff00ff0000ffff,
    0xffff00ff00ff0000,	0xffff00ff00ff00ff,
    0xffff00ff00ffff00,	0xffff00ff00ffffff,
    0xffff00ffff000000,	0xffff00ffff0000ff,
    0xffff00ffff00ff00,	0xffff00ffff00ffff,
    0xffff00ffffff0000,	0xffff00ffffff00ff,
    0xffff00ffffffff00,	0xffff00ffffffffff,
    0xffffff0000000000,	0xffffff00000000ff,
    0xffffff000000ff00,	0xffffff000000ffff,
    0xffffff0000ff0000,	0xffffff0000ff00ff,
    0xffffff0000ffff00,	0xffffff0000ffffff,
    0xffffff00ff000000,	0xffffff00ff0000ff,
    0xffffff00ff00ff00,	0xffffff00ff00ffff,
    0xffffff00ffff0000,	0xffffff00ffff00ff,
    0xffffff00ffffff00,	0xffffff00ffffffff,
    0xffffffff00000000,	0xffffffff000000ff,
    0xffffffff0000ff00,	0xffffffff0000ffff,
    0xffffffff00ff0000,	0xffffffff00ff00ff,
    0xffffffff00ffff00,	0xffffffff00ffffff,
    0xffffffffff000000,	0xffffffffff0000ff,
    0xffffffffff00ff00,	0xffffffffff00ffff,
    0xffffffffffff0000,	0xffffffffffff00ff,
    0xffffffffffffff00,	0xffffffffffffffff
#endif
};

int	cfb8StippleMode, cfb8StippleAlu, cfb8StippleRRop;
PixelGroup cfb8StippleFg, cfb8StippleBg, cfb8StipplePm;
PixelGroup cfb8StippleAnd[NUM_MASKS], cfb8StippleXor[NUM_MASKS];

int
cfb8SetStipple (alu, fg, planemask)
int		alu;
CfbBits	fg, planemask;
{
    CfbBits   and, xor, rrop;
    int	s;
    CfbBits   c;

    cfb8StippleMode = FillStippled;
    cfb8StippleAlu = alu;
    cfb8StippleFg = fg & PMSK;
    cfb8StipplePm = planemask & PMSK;
    rrop = cfbReduceRasterOp (alu, fg, planemask, &and, &xor);
    cfb8StippleRRop = rrop;
    /*
     * create the appropriate pixel-fill bits for current
     * foreground
     */
    for (s = 0; s < NUM_MASKS; s++)
    {
	c = cfb8StippleMasks[s];
	cfb8StippleAnd[s] = and | ~c;
	cfb8StippleXor[s] = xor & c;
    }
    return TRUE;
}


int
cfb8SetOpaqueStipple (alu, fg, bg, planemask)
int		alu;
CfbBits	fg, bg, planemask;
{
    CfbBits   andfg, xorfg, andbg, xorbg, rropfg, rropbg;
    int	s;
    CfbBits   c;

    cfb8StippleMode = FillOpaqueStippled;
    cfb8StippleAlu = alu;
    cfb8StippleFg = fg & PMSK;
    cfb8StippleBg = bg & PMSK;
    cfb8StipplePm = planemask & PMSK;
    rropfg = cfbReduceRasterOp (alu, cfb8StippleFg, cfb8StipplePm, &andfg, &xorfg);
    rropbg = cfbReduceRasterOp (alu, cfb8StippleBg, cfb8StipplePm, &andbg, &xorbg);
    if (rropfg == rropbg)
	cfb8StippleRRop = rropfg;
    else
	cfb8StippleRRop = GXset;
    /*
     * create the appropriate pixel-fill bits for current
     * foreground
     */
    for (s = 0; s < NUM_MASKS; s++)
    {
	c = cfb8StippleMasks[s];
	cfb8StippleAnd[s] = (andfg | ~c) & (andbg | c);
	cfb8StippleXor[s] = (xorfg & c) | (xorbg & ~c);
    }
    return TRUE;
}

/*
 * a grungy little routine.  This computes clip masks
 * for partial character blts.  Returns rgnOUT if the
 * entire character is clipped; returns rgnIN if the entire
 * character is unclipped; returns rgnPART if a portion of
 * the character is visible.  Computes clip masks for each
 * longword of the character -- and those with the
 * contents of the glyph to compute the visible bits.
 */

#if PGSZ == 32
#if (BITMAP_BIT_ORDER == MSBFirst)
PixelGroup cfb8BitLenMasks[PGSZ] = {
    0xffffffff, 0x7fffffff, 0x3fffffff, 0x1fffffff,
    0x0fffffff, 0x07ffffff, 0x03ffffff, 0x01ffffff,
    0x00ffffff, 0x007fffff, 0x003fffff, 0x001fffff,
    0x000fffff, 0x0007ffff, 0x0003ffff, 0x0001ffff,
    0x0000ffff, 0x00007fff, 0x00003fff, 0x00001fff,
    0x00000fff, 0x000007ff, 0x000003ff, 0x000001ff,
    0x000000ff, 0x0000007f, 0x0000003f, 0x0000001f,
    0x0000000f, 0x00000007, 0x00000003, 0x00000001,
};
#else
PixelGroup cfb8BitLenMasks[PGSZ] = {
    0xffffffff, 0xfffffffe, 0xfffffffc, 0xfffffff8,
    0xfffffff0, 0xffffffe0, 0xffffffc0, 0xffffff80,
    0xffffff00, 0xfffffe00, 0xfffffc00, 0xfffff800,
    0xfffff000, 0xffffe000, 0xffffc000, 0xffff8000,
    0xffff0000, 0xfffe0000, 0xfffc0000, 0xfff80000,
    0xfff00000, 0xffe00000, 0xffc00000, 0xff800000,
    0xff000000, 0xfe000000, 0xfc000000, 0xf8000000,
    0xf0000000, 0xe0000000, 0xc0000000, 0x80000000,
};
#endif /* BITMAP_BIT_ORDER */
#else /* PGSZ == 64 */
#if (BITMAP_BIT_ORDER == MSBFirst)
PixelGroup cfb8BitLenMasks[PGSZ] = {
    0xffffffffffffffff,    0x7fffffffffffffff,
    0x3fffffffffffffff,    0x1fffffffffffffff,
    0x0fffffffffffffff,    0x07ffffffffffffff,
    0x03ffffffffffffff,    0x01ffffffffffffff,
    0x00ffffffffffffff,    0x007fffffffffffff,
    0x003fffffffffffff,    0x001fffffffffffff,
    0x000fffffffffffff,    0x0007ffffffffffff,
    0x0003ffffffffffff,    0x0001ffffffffffff,
    0x0000ffffffffffff,    0x00007fffffffffff,
    0x00003fffffffffff,    0x00001fffffffffff,
    0x00000fffffffffff,    0x000007ffffffffff,
    0x000003ffffffffff,    0x000001ffffffffff,
    0x000000ffffffffff,    0x0000007fffffffff,
    0x0000003fffffffff,    0x0000001fffffffff,
    0x0000000fffffffff,    0x00000007ffffffff,
    0x00000003ffffffff,    0x00000001ffffffff,
    0x00000000ffffffff,    0x000000007fffffff,
    0x000000003fffffff,    0x000000001fffffff,
    0x000000000fffffff,    0x0000000007ffffff,
    0x0000000003ffffff,    0x0000000001ffffff,
    0x0000000000ffffff,    0x00000000007fffff,
    0x00000000003fffff,    0x00000000001fffff,
    0x00000000000fffff,    0x000000000007ffff,
    0x000000000003ffff,    0x000000000001ffff,
    0x000000000000ffff,    0x0000000000007fff,
    0x0000000000003fff,    0x0000000000001fff,
    0x0000000000000fff,    0x00000000000007ff,
    0x00000000000003ff,    0x00000000000001ff,
    0x00000000000000ff,    0x000000000000007f,
    0x000000000000003f,    0x000000000000001f,
    0x000000000000000f,    0x0000000000000007,
    0x0000000000000003,    0x0000000000000001
};
#else
PixelGroup cfb8BitLenMasks[PGSZ] = {
    0xffffffffffffffff,    0xfffffffffffffffe,
    0xfffffffffffffffc,    0xfffffffffffffff8,
    0xfffffffffffffff0,    0xffffffffffffffe0,
    0xffffffffffffffc0,    0xffffffffffffff80,
    0xffffffffffffff00,    0xfffffffffffffe00,
    0xfffffffffffffc00,    0xfffffffffffff800,
    0xfffffffffffff000,    0xffffffffffffe000,
    0xffffffffffffc000,    0xffffffffffff8000,
    0xffffffffffff0000,    0xfffffffffffe0000,
    0xfffffffffffc0000,    0xfffffffffff80000,
    0xfffffffffff00000,    0xffffffffffe00000,
    0xffffffffffc00000,    0xffffffffff800000,
    0xffffffffff000000,    0xfffffffffe000000,
    0xfffffffffc000000,    0xfffffffff8000000,
    0xfffffffff0000000,    0xffffffffe0000000,
    0xffffffffc0000000,    0xffffffff80000000,
    0xffffffff00000000,    0xfffffffe00000000,
    0xfffffffc00000000,    0xfffffff800000000,
    0xfffffff000000000,    0xffffffe000000000,
    0xffffffc000000000,    0xffffff8000000000,
    0xffffff0000000000,    0xfffffe0000000000,
    0xfffffc0000000000,    0xfffff80000000000,
    0xfffff00000000000,    0xffffe00000000000,
    0xffffc00000000000,    0xffff800000000000,
    0xffff000000000000,    0xfffe000000000000,
    0xfffc000000000000,    0xfff8000000000000,
    0xfff0000000000000,    0xffe0000000000000,
    0xffc0000000000000,    0xff80000000000000,
    0xff00000000000000,    0xfe00000000000000,
    0xfc00000000000000,    0xf800000000000000,
    0xf000000000000000,    0xe000000000000000,
    0xc000000000000000,    0x8000000000000000
};
#endif /* BITMAP_BIT_ORDER */
#endif /* PGSZ */



int
cfb8ComputeClipMasks32 (pBox, numRects, x, y, w, h, clips)
    BoxPtr	pBox;
    int		numRects;
    int		x, y, w, h;
    CARD32      *clips;
{
    int	    yBand, yBandBot;
    int	    ch;
    CfbBits	    clip;
    int	    partIN = FALSE, partOUT = FALSE;
    int	    result;

    if (numRects == 0)
	return rgnOUT;
    while (numRects && pBox->y2 <= y)
    {
	--numRects;
	++pBox;
    }
    if (!numRects || pBox->y1 >= y + h)
	return rgnOUT;
    yBand = pBox->y1;
    while (numRects && pBox->y1 == yBand && pBox->x2 <= x)
    {
	--numRects;
	++pBox;
    }
    if (!numRects || pBox->y1 >= y + h)
	return rgnOUT;
    if (numRects &&
	x >= pBox->x1 &&
	x + w <= pBox->x2 &&
	y >= pBox->y1 &&
	y + h <= pBox->y2)
    {
	return rgnIN;
    }
    ch = 0;
    while (numRects && pBox->y1 < y + h)
    {
	yBand = pBox->y1;
	yBandBot = pBox->y2;
 	while (ch < h && y + ch < yBand)
 	{
 	    partOUT = TRUE;
 	    clips[ch++] = 0;
 	}
 	if (ch >= h)
 	    break;
    	while (numRects && pBox->y1 == yBand && pBox->x2 <= x)
    	{
	    --numRects;
	    ++pBox;
    	}
    	if (!numRects)
	    break;
	clip = 0;
    	while (numRects && pBox->y1 == yBand && pBox->x1 < x + w)
    	{
	    if (x < pBox->x1)
		if (pBox->x2 < x + w)
		    clip |= cfb8BitLenMasks[pBox->x1 - x] & ~cfb8BitLenMasks[pBox->x2 - x];
		else
		    clip |= cfb8BitLenMasks[pBox->x1 - x];
 	    else
		if (pBox->x2 < x + w)
		    clip |= ~cfb8BitLenMasks[pBox->x2 - x];
		else
		    clip = ~0;
	    --numRects;
	    ++pBox;
    	}
	if (clip != 0)
		partIN = TRUE;
	if (clip != ~0)
		partOUT = TRUE;
	while (ch < h && y + ch < yBandBot)
	    clips[ch++] = clip;
	while (numRects && pBox->y1 == yBand)
	{
	    --numRects;
	    ++pBox;
	}
    }
    while (ch < h)
    {
	partOUT = TRUE;
	clips[ch++] = 0;
    }
    result = rgnOUT;
    if (partIN)
    {
	if (partOUT)
	    result = rgnPART;
	else
	    result = rgnIN;
    }
    return result;
}

#endif /* PSZ == 8 */
