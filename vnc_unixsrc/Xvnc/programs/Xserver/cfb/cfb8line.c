/*
 * $Xorg: cfb8line.c,v 1.4 2001/02/09 02:04:37 xorgcvs Exp $
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
 *
 * $XFree86: xc/programs/Xserver/cfb/cfb8line.c,v 3.18tsi Exp $
 * Jeff Anton'x fixes: cfb8line.c   97/02/07
 */

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
#include "miline.h"

#ifdef PIXEL_ADDR

#if defined(__GNUC__) && defined(mc68020)
#define STUPID volatile
#define REARRANGE
#else
#define STUPID
#endif

#ifdef __GNUC__
/* lame compiler doesn't even look at 'register' attributes */
#define I_H do{
#define I_T }while(0);
#define IMPORTANT_START I_H I_H I_H I_H I_H I_H I_H I_H I_H I_H
#define IMPORTANT_END	I_T I_T I_T I_T I_T I_T I_T I_T I_T I_T
#else
#define IMPORTANT_START
#define IMPORTANT_END
#endif

#define isClipped(c,ul,lr)  ((((c) - (ul)) | ((lr) - (c))) & ClipMask)

#ifdef POLYSEGMENT

# if (defined(sun) || defined(__bsdi__)) && \
     (defined(sparc) || defined(__sparc__))
#  define WIDTH_FAST  1152
# endif

# ifdef ultrix
#  define WIDTH_FAST  1024
# endif

# ifdef Mips
#  define WIDTH_FAST 4096
# endif
# ifdef WIDTH_FAST
#  if WIDTH_FAST == 1024
#   define FAST_MUL(y)	((y) << 10)
#  endif

#  if WIDTH_FAST == 1152
#   define FAST_MUL(y)	(((y) << 10) + ((y) << 7))
#  endif

#  if WIDTH_FAST == 1280
#   define FAST_MUL(y)	(((y) << 10) + ((y) << 8))
#  endif

#  if WIDTH_FAST == 2048
#   define FAST_MUL(y)	((y) << 11)
#  endif

#  if WIDTH_FAST == 4096
#   define FAST_MUL(y)	((y) << 12)
#  endif
# endif

# if defined(WIDTH_SHIFT)
#  ifdef FAST_MUL
#   define FUNC_NAME(e)	    RROP_NAME(RROP_NAME_CAT(e,Shift))
#   if RROP == GXcopy
#    define INCLUDE_OTHERS
#    define SERIOUS_UNROLLING
#   endif
#   define INCLUDE_DRAW
#   define NWIDTH(nwidth)   WIDTH_FAST
#   define WIDTH_MUL(y,w)   FAST_MUL(y)
#  endif
# else
#  define FUNC_NAME(e)	    RROP_NAME(e)
#  define WIDTH_MUL(y,w)    ((y) * (w))
#  define NWIDTH(nwidth)    (nwidth)
#  define INCLUDE_DRAW
#  if !defined (FAST_MUL) && RROP == GXcopy
#   define INCLUDE_OTHERS
#   define SERIOUS_UNROLLING
#  endif
# endif
#else

# define INCLUDE_DRAW
# define WIDTH_MUL(y,w)	((y) * (w))
# define NWIDTH(nwidth)	nwidth
# ifdef PREVIOUS
#  define FUNC_NAME(e)	RROP_NAME(RROP_NAME_CAT(e,Previous))
# else
#  define FUNC_NAME(e)	RROP_NAME(e)
#  if RROP == GXcopy
#   define INCLUDE_OTHERS
#   ifdef PLENTIFUL_REGISTERS
#    define SAVE_X2Y2
#   endif
#   define ORIGIN
#   define SERIOUS_UNROLLING
#  else
#   define EITHER_MODE
#  endif
# endif
#endif

#if PSZ == 24
#define PXL2ADR(x)  ((x)*3 >> 2)

#if RROP == GXcopy
#define body_rop \
	    addrp = (PixelType *)((unsigned long)addrb & ~0x03); \
	    switch((unsigned long)addrb & 3){ \
	    case 0: \
	      *addrp = (*addrp & 0xFF000000)|(piQxelXor[0] & 0xFFFFFF); \
	      break; \
	    case 1: \
	      *addrp = (*addrp & 0xFF)|(piQxelXor[2] & 0xFFFFFF00); \
	      break; \
	    case 3: \
	      *addrp = (*addrp & 0xFFFFFF)|(piQxelXor[0] & 0xFF000000); \
	      *(addrp+1)=(*(addrp+1) & 0xFFFF0000)|(piQxelXor[1] & 0xFFFF); \
	      break; \
	    case 2: \
	      *addrp = (*addrp & 0xFFFF)|(piQxelXor[1] & 0xFFFF0000); \
	      *(addrp+1)=(*(addrp+1) & 0xFFFFFF00)|(piQxelXor[2] & 0xFF); \
	      break; \
	    }
#endif
#if RROP == GXxor
#define body_rop \
	    addrp = (PixelType *)((unsigned long)addrb & ~0x03); \
	    switch((unsigned long)addrb & 3){ \
	    case 0: \
	      *addrp ^= piQxelXor[0] & 0xFFFFFF; \
	      break; \
	    case 1: \
	      *addrp ^= piQxelXor[2] & 0xFFFFFF00; \
	      break; \
	    case 3: \
	      *addrp ^= piQxelXor[0] & 0xFF000000; \
	      *(addrp+1) ^= piQxelXor[1] & 0xFFFF; \
	      break; \
	    case 2: \
	      *addrp ^= piQxelXor[1] & 0xFFFF0000; \
	      *(addrp+1) ^= piQxelXor[2] & 0xFF; \
	      break; \
	    }
#endif
#if RROP == GXand
#define body_rop \
	    addrp = (PixelType *)((unsigned long)addrb & ~0x03); \
	    switch((unsigned long)addrb & 3){ \
	    case 0: \
	      *addrp &= piQxelAnd[0] | 0xFF000000; \
	      break; \
	    case 1: \
	      *addrp &= piQxelAnd[2] | 0xFF; \
	      break; \
	    case 3: \
	      *addrp &= 0xFFFFFF | piQxelAnd[0]; \
	      *(addrp+1) &= 0xFFFF0000 | piQxelAnd[1]; \
	      break; \
	    case 2: \
	      *addrp &= 0xFFFF | piQxelAnd[1]; \
	      *(addrp+1) &= 0xFFFFFF00 | piQxelAnd[2]; \
	      break; \
	    }
#endif
#if RROP == GXor
#define body_rop \
	    addrp = (PixelType *)((unsigned long)addrb & ~0x03); \
	    switch((unsigned long)addrb & 3){ \
	    case 0: \
	      *addrp |= piQxelOr[0] & 0xFFFFFF; \
	      break; \
	    case 1: \
	      *addrp |= piQxelOr[2] & 0xFFFFFF00; \
	      break; \
	    case 3: \
	      *addrp |= piQxelOr[0] & 0xFF000000; \
	      *(addrp+1) |= piQxelOr[1] & 0xFFFF; \
	      break; \
	    case 2: \
	      *addrp |= piQxelOr[1] & 0xFFFF0000; \
	      *(addrp+1) |= piQxelOr[2] & 0xFF; \
	      break; \
	    }
#endif
#if RROP == GXset
#define body_rop \
	    addrp = (PixelType *)((unsigned long)addrb & ~0x03); \
	    switch((unsigned long)addrb & 3){ \
	    case 0: \
	      *addrp = (*addrp & (piQxelAnd[0]|0xFF000000)) \
			^ (piQxelXor[0] & 0xFFFFFF); \
	      break; \
	    case 1: \
	      *addrp = (*addrp & (piQxelAnd[2]|0xFF)) \
			^ (piQxelXor[2] & 0xFFFFFF00); \
	      break; \
	    case 3: \
	      *addrp = (*addrp & (piQxelAnd[0]|0xFFFFFF)) \
			^ (piQxelXor[0] & 0xFF000000); \
	      *(addrp+1) = (*(addrp+1) & (piQxelAnd[1]|0xFFFF0000)) \
			^ (piQxelXor[1] & 0xFFFF); \
	      break; \
	    case 2: \
	      *addrp = (*addrp & (piQxelAnd[1]|0xFFFF)) \
			^ (piQxelXor[1] & 0xFFFF0000); \
	      *(addrp+1) = (*(addrp+1) & (piQxelAnd[2]|0xFFFFFF00)) \
			^ (piQxelXor[2] & 0xFF); \
	      break; \
	    }
#endif
#endif /* PSZ == 24 */

#define BUGFIX_clip

#ifdef INCLUDE_DRAW

int
#ifdef POLYSEGMENT
FUNC_NAME(cfb8SegmentSS1Rect) (pDrawable, pGC, nseg, pSegInit)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		nseg;
    xSegment	*pSegInit;
#else
FUNC_NAME(cfb8LineSS1Rect) (pDrawable, pGC, mode, npt, pptInit, pptInitOrig,
			    x1p,y1p,x2p,y2p)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int	mode;		/* Origin or Previous */
    int	npt;		/* number of points */
    DDXPointPtr pptInit, pptInitOrig;
    int	*x1p, *y1p, *x2p, *y2p;
#endif /* POLYSEGMENT */
{
    register long   e;
    register int    y1_or_e1;
    register PixelType   *addrp;
    register int    stepmajor;
    register int    stepminor;
#ifndef REARRANGE
    register long   e3;
#endif
#ifdef mc68000
    register short  x1_or_len;
#else
    register int    x1_or_len;
#endif
    RROP_DECLARE

#ifdef SAVE_X2Y2
# define c2 y2
#else
    register int    c2;
#endif
#if !defined(ORIGIN) && !defined(POLYSEGMENT)
    register int _x1 = 0, _y1 = 0, _x2 = 0, _y2 = 0;
    int extents_x1, extents_y1, extents_x2, extents_y2;
#endif /* !ORIGIN */
#ifndef PREVIOUS
    register int upperleft, lowerright;
    CARD32	 ClipMask = 0x80008000;
#endif /* !PREVIOUS */
#ifdef POLYSEGMENT
    register int    capStyle;
#endif /* POLYSEGMENT */
#ifdef SAVE_X2Y2
    register int    x2, y2;
# define X1  x1_or_len
# define Y1  y1_or_e1
# define X2  x2
# define Y2  y2
#else
# ifdef POLYSEGMENT
#  define X1  x1_or_len
#  define Y1  y1_or_e1
# else
#  define X1  intToX(y1_or_e1)
#  define Y1  intToY(y1_or_e1)
# endif /* POLYSEGMENT */
# define X2  intToX(c2)
# define Y2  intToY(c2)
#endif /* SAVE_X2Y2 */
    PixelType   *addr;
    int		    nwidth;
    cfbPrivGCPtr    devPriv;
    BoxPtr	    extents;
    int		    *ppt;
#if PSZ == 24
    int xBase;     /* x of addr */
    int xOffset;   /* x of addrp */
    PixelType   *addrLineEnd;
    char *addrb;
    int stepmajor3, stepminor3, majordx, minordx;
#endif
#ifndef POLYSEGMENT
#ifndef ORIGIN
#ifdef BUGFIX_clip
    int ex_x1, ex_y1, ex_x2, ex_y2;
#endif
#endif
#endif
    int		    octant;
    unsigned int    bias = miGetZeroLineBias(pDrawable->pScreen);

    devPriv = cfbGetGCPrivate(pGC);
    cfbGetPixelWidthAndPointer (pDrawable, nwidth, addr);
#ifndef REARRANGE
    RROP_FETCH_GCPRIV(devPriv);
#endif
    extents = &pGC->pCompositeClip->extents;
#ifndef PREVIOUS
    c2 = *((int *) &pDrawable->x);
    c2 -= (c2 & 0x8000) << 1;
    upperleft = *((int *) &extents->x1) - c2;
    lowerright = *((int *) &extents->x2) - c2 - 0x00010001;
#endif /* !PREVIOUS */
#ifndef POLYSEGMENT
#ifndef ORIGIN
#ifdef BUGFIX_clip
    ex_x1 = extents->x1 - pDrawable->x;
    ex_y1 = extents->y1 - pDrawable->y;
    ex_x2 = extents->x2 - pDrawable->x;
    ex_y2 = extents->y2 - pDrawable->y;
#endif
#endif
#endif
#if PSZ == 24
    xBase = pDrawable->x;
    addr += WIDTH_MUL(pDrawable->y,nwidth);
#else
    addr = addr + WIDTH_MUL(pDrawable->y,nwidth) + pDrawable->x;
#endif
#ifdef POLYSEGMENT
    capStyle = pGC->capStyle - CapNotLast;
    ppt = (int *) pSegInit;
    while (nseg--)
#else /* POLYSEGMENT */
#ifdef EITHER_MODE
    mode -= CoordModePrevious;
    if (!mode)
#endif /* EITHER_MODE */	
#ifndef ORIGIN
    {	/* CoordModePrevious */
	ppt = (int *)pptInit + 1;
	_x1 = *x1p;
	_y1 = *y1p;
	extents_x1 = extents->x1 - pDrawable->x;
	extents_x2 = extents->x2 - pDrawable->x;
	extents_y1 = extents->y1 - pDrawable->y;
	extents_y2 = extents->y2 - pDrawable->y;
	if (_x1 < extents_x1 || _x1 >= extents_x2 ||
	    _y1 < extents_y1 || _y1 >= extents_y2)
	{
	    c2 = *ppt++;
	    intToCoord(c2, _x2, _y2);
	    *x2p = _x1 + _x2;
	    *y2p = _y1 + _y2;
	    return 1;
	}
#if PSZ == 24
	addrLineEnd = addr + WIDTH_MUL(_y1, nwidth);
	xOffset = xBase + _x1;
	addrb = (char *)addrLineEnd + xOffset * 3;
	addrp = (PixelType *)((unsigned long)addrb & ~0x03);
#else
	addrp = addr + WIDTH_MUL(_y1, nwidth) + _x1;
#endif
	_x2 = _x1;
	_y2 = _y1;	
    }
#endif /* !ORIGIN */
#ifdef EITHER_MODE
    else
#endif /* EITHER_MODE */
#ifndef PREVIOUS
    {
	ppt = (int *) pptInit;
	c2 = *ppt++;
	if (isClipped (c2, upperleft, lowerright))
	{
	    return 1;
	}
#ifdef SAVE_X2Y2
	intToCoord(c2,x2,y2);
#endif
#if PSZ == 24
	addrLineEnd = addr + WIDTH_MUL(Y2, nwidth);
	xOffset = xBase + X2;
	addrb = (char *)addrLineEnd + xOffset * 3;
	addrp = (PixelType *)((unsigned long)addrb & ~0x03);
#else
	addrp = addr + WIDTH_MUL(Y2, nwidth) + X2;
#endif
    }
#endif /* !PREVIOUS */    
    while (--npt)
#endif /* POLYSEGMENT */
    {
#ifdef POLYSEGMENT
	y1_or_e1 = ppt[0];
	c2 = ppt[1];
	ppt += 2;
	if (isClipped(y1_or_e1,upperleft,lowerright)|isClipped(c2,upperleft,lowerright))
	    break;
	intToCoord(y1_or_e1,x1_or_len,y1_or_e1);
	/* compute now to avoid needing x1, y1 later */
#if PSZ == 24
	addrLineEnd = addr + WIDTH_MUL(y1_or_e1, nwidth);
	xOffset = xBase + x1_or_len;
	addrb = (char *)addrLineEnd + xOffset * 3;
	addrp = (PixelType *)((unsigned long)addrb & ~0x03);
#else
	addrp = addr + WIDTH_MUL(y1_or_e1, nwidth) + x1_or_len;
#endif
#else /* !POLYSEGMENT */
#ifdef EITHER_MODE
	if (!mode)
#endif /* EITHER_MODE */	
#ifndef ORIGIN
	{	
	    /* CoordModePrevious */
	    _x1 = _x2;
	    _y1 = _y2;
	    c2 = *ppt++;
	    intToCoord(c2, _x2, _y2);
	    _x2 = _x1 + _x2;
	    _y2 = _y1 + _y2;

#ifdef BUGFIX_clip
	    if (_x2 < ex_x1 || _x2 >= ex_x2 ||
		_y2 < ex_y1 || _y2 >= ex_y2)
#else
	    if (_x2 < extents_x1 || _x2 >= extents_x2 ||
		_y2 < extents_y1 || _y2 >= extents_y2)
#endif
	    {
		break;
	    }
	    CalcLineDeltas(_x1, _y1, _x2, _y2, x1_or_len, y1_or_e1,
			   stepmajor, stepminor, 1, NWIDTH(nwidth), octant);
	}
#endif /* !ORIGIN */
#ifdef EITHER_MODE
	else
#endif /* EITHER_MODE */
#ifndef PREVIOUS
        {
#ifndef SAVE_X2Y2
	    y1_or_e1 = c2;
#else
	    y1_or_e1 = y2;
	    x1_or_len = x2;
#endif /* SAVE_X2Y2 */
	    c2 = *ppt++;

	    if (isClipped (c2, upperleft, lowerright))
		break;
#ifdef SAVE_X2Y2
	    intToCoord(c2,x2,y2);
#endif
	    CalcLineDeltas(X1, Y1, X2, Y2, x1_or_len, y1_or_e1,
			   stepmajor, stepminor, 1, NWIDTH(nwidth), octant);
	}
#endif /* !PREVIOUS */
#endif /* POLYSEGMENT */

#ifdef POLYSEGMENT
	CalcLineDeltas(X1, Y1, X2, Y2, x1_or_len, y1_or_e1,
		       stepmajor, stepminor, 1, NWIDTH(nwidth), octant);
	/*
	 * although the horizontal code works for polyline, it
	 * slows down 10 pixel lines by 15%.  Thus, this
	 * code is optimized for horizontal segments and
	 * random orientation lines, which seems like a reasonable
	 * assumption
	 */
	if (y1_or_e1 != 0)
	{
#endif /* POLYSEGMENT */
	if (x1_or_len < y1_or_e1)
	{
#ifdef REARRANGE
	    register int	e3;
#endif

	    e3 = x1_or_len;
	    x1_or_len = y1_or_e1;
	    y1_or_e1 = e3;

	    e3 = stepminor;
	    stepminor = stepmajor;
	    stepmajor = e3;
	    SetYMajorOctant(octant);
	}

	e = -x1_or_len;
#ifdef POLYSEGMENT
	if (!capStyle)
	    x1_or_len--;
#endif

	{
#ifdef REARRANGE
	register int e3;
	RROP_DECLARE
	RROP_FETCH_GCPRIV(devPriv);
#endif

	y1_or_e1 = y1_or_e1 << 1;
	e3 = e << 1;

	FIXUP_ERROR(e, octant, bias);

#if PSZ == 24
 	if (stepmajor == 1  ||  stepmajor == -1){
 	    stepmajor3 = stepmajor * 3;
 	    stepminor3 = stepminor * sizeof (CfbBits);
 	    majordx = stepmajor; minordx = 0;
         } else {
 	    stepmajor3 = stepmajor * sizeof (CfbBits);
 	    stepminor3 = stepminor * 3;
 	    majordx = 0; minordx = stepminor;
         }
#endif
 
#if PSZ == 24
#define body {\
 	    body_rop \
 	    addrb += stepmajor3; \
             xOffset += majordx; \
 	    e += y1_or_e1; \
 	    if (e >= 0){ \
 	        addrb += stepminor3; \
                 xOffset += minordx; \
 		e += e3; \
 	    } \
 	}
#else /* PSZ == 24 */

#define body {\
	    RROP_SOLID(addrp); \
	    addrp += stepmajor; \
	    e += y1_or_e1; \
	    if (e >= 0) \
	    { \
		addrp += stepminor; \
		e += e3; \
	     } \
	}
#endif /* PSZ == 24 */

#ifdef LARGE_INSTRUCTION_CACHE

# ifdef SERIOUS_UNROLLING
#  define UNROLL	16
# else
#  define UNROLL	4
# endif
#define CASE(n)	case -n: body

	while ((x1_or_len -= UNROLL) >= 0)
	{
	    body body body body
# if UNROLL >= 8
	    body body body body
# endif
# if UNROLL >= 12
	    body body body body
# endif
# if UNROLL >= 16
	    body body body body
# endif
	}
	switch (x1_or_len)
	{
	CASE(1) CASE(2) CASE(3)
# if UNROLL >= 8
	CASE(4) CASE(5) CASE(6) CASE(7)
# endif
# if UNROLL >= 12
	CASE(8) CASE(9) CASE(10) CASE(11)
# endif
# if UNROLL >= 16
	CASE(12) CASE(13) CASE(14) CASE(15)
# endif
	}
#else /* !LARGE_INSTRUCTION_CACHE */

	IMPORTANT_START
	IMPORTANT_START

	if (x1_or_len & 1)
	    body
	x1_or_len >>= 1;
	while (x1_or_len--) {
	    body body
	}

	IMPORTANT_END
	IMPORTANT_END
#endif /* LARGE_INSTRUCTION_CACHE */

#ifdef POLYSEGMENT
#if PSZ == 24
	body_rop
#else
	RROP_SOLID(addrp);
#endif
#endif
#if PSZ == 24
	addrp = (PixelType *)((unsigned long)addrb & ~0x03);
#endif
	}
#undef body
#ifdef POLYSEGMENT
	}
	else /* Polysegment horizontal line optimization */
	{
# ifdef REARRANGE
	    register int    e3;
	    RROP_DECLARE
	    RROP_FETCH_GCPRIV(devPriv);
# endif /* REARRANGE */
	    if (stepmajor < 0)
	    {
#if PSZ == 24
		xOffset -= x1_or_len;
		addrp = addrLineEnd + PXL2ADR(xOffset);
#else
		addrp -= x1_or_len;
#endif
		if (capStyle)
		    x1_or_len++;
		else
#if PSZ == 24
		  xOffset++;
		addrp = addrLineEnd + PXL2ADR(xOffset);
#else
		    addrp++;
#endif
	    }
	    else
	    {
#if PSZ == 24
		addrp = addrLineEnd + PXL2ADR(xOffset);
#endif
		if (capStyle)
		    x1_or_len++;
	    }
# if PSZ == 24
	    y1_or_e1 = xOffset & 3;
# else
#  if PGSZ == 64 /* PIM value from <cfbmskbits.h> is not it! (for 16/32 PSZ)*/
	    y1_or_e1 = ((long) addrp) & 0x7;
	    addrp = (PixelType *) (((unsigned char *) addrp) - y1_or_e1);
#  else
	    y1_or_e1 = ((long) addrp) & PIM;
	    addrp = (PixelType *) (((unsigned char *) addrp) - y1_or_e1);
#  endif
#if PGSZ == 32
#  if PWSH != 2
	    y1_or_e1 >>= (2 - PWSH);
#  endif
#else /* PGSZ == 64 */
#  if PWSH != 3
	    y1_or_e1 >>= (3 - PWSH);
#  endif
#endif /* PGSZ */
# endif /* PSZ == 24 */
#if PSZ == 24
	    {
#if RROP == GXcopy
	      register int nlmiddle;
	      int leftIndex = xOffset & 3;
	      int rightIndex = (xOffset + x1_or_len) & 3;
#else
	      register int pidx;
#endif

#if RROP == GXcopy
	      nlmiddle = x1_or_len;
	      if(leftIndex){
		nlmiddle -= (4 - leftIndex);
	      }
	      if(rightIndex){
		nlmiddle -= rightIndex;
	      }
	      
	      nlmiddle >>= 2;
	      switch(leftIndex+x1_or_len){
	      case 4:
		switch(leftIndex){
		case 0:
		  *addrp++ = piQxelXor[0];
		  *addrp++ = piQxelXor[1];
		  *addrp   = piQxelXor[2];
		  break;
		case 1:
		  *addrp = ((*addrp) & 0xFFFFFF) | (piQxelXor[0] & 0xFF000000);
		  addrp++;
		  *addrp = piQxelXor[1];
		  addrp++;
		  *addrp = piQxelXor[2];
		  break;
		case 2:
		  *addrp = ((*addrp) & 0xFFFF) | (piQxelXor[1] & 0xFFFF0000);
		  addrp++;
		  *addrp = piQxelXor[2];
		  break;
		case 3:
		  *addrp = ((*addrp) & 0xFF) | (piQxelXor[2] & 0xFFFFFF00);
		  break;
		}
		break;
	      case 3:
		switch(leftIndex){
		case 0:
		  *addrp++ = piQxelXor[0];
		  *addrp++ = piQxelXor[1];
		  *addrp = ((*addrp) & 0xFFFFFF00) | (piQxelXor[2] & 0xFF);
		  break;
		case 1:
		  *addrp = ((*addrp) & 0xFFFFFF) | (piQxelXor[0] & 0xFF000000);
		  addrp++;
		  *addrp = piQxelXor[1];
		  addrp++;
		  *addrp = ((*addrp) & 0xFFFFFF00) | (piQxelXor[2] & 0xFF);
		  break;
		case 2:
		  *addrp = ((*addrp) & 0xFFFF) | (piQxelXor[1] & 0xFFFF0000);
		  addrp++;
		  *addrp = ((*addrp) & 0xFFFFFF00) | (piQxelXor[2] & 0xFF);
		  break;
		}
		break;
	      case 2:
		switch(leftIndex){
/*
		case 2:
		  *addrp = ((*addrp) & 0xFFFF) | (piQxelXor[1] & 0xFFFF0000);
		  addrp++;
		  *addrp = ((*addrp) & 0xFFFFFF00) | (piQxelXor[2] & 0xFF);
		  break;
*/
		case 1:
		  *addrp = ((*addrp) & 0xFFFFFF) | (piQxelXor[0] & 0xFF000000);
		  addrp++;
		  *addrp = ((*addrp) & 0xFFFF0000) | (piQxelXor[1] & 0xFFFF);
		  break;
		case 0:
		  *addrp++ =  piQxelXor[0];
		  *addrp = ((*addrp) & 0xFFFF0000) | (piQxelXor[1] & 0xFFFF);
		  break;
		}
		break;
	      case 1: /*only if leftIndex = 0 and w = 1*/
		if(x1_or_len){
		*addrp =  ((*addrp) & 0xFF000000) | (piQxelXor[0] & 0xFFFFFF);
		}
/*
		else{
		*addrp =  ((*addrp) & 0xFFFFFF) | (piQxelXor[0] & 0xFF000000);
		addrp++;
		*addrp =  ((*addrp) & 0xFFFF0000) | (piQxelXor[1] & 0xFFFF);
		}
*/
		break;
	      case 0: /*never*/
		break;
	      default:
		{
/*
		  maskbits(y1_or_e1, x1_or_len, e, e3, x1_or_len)
*/
		  switch(leftIndex){
		  case 0:
		    break;
		  case 1:
		    *addrp = ((*addrp) & 0xFFFFFF) | (piQxelXor[0] & 0xFF000000);
		    addrp++;
		    *addrp = piQxelXor[1];
		    addrp++;
		    *addrp = piQxelXor[2];
		    addrp++;
		    break;
		  case 2:
		    *addrp = ((*addrp) & 0xFFFF) | (piQxelXor[1] & 0xFFFF0000);
		    addrp++;
		    *addrp = piQxelXor[2];
		    addrp++;
		    break;
		  case 3:
		    *addrp = ((*addrp) & 0xFF) | (piQxelXor[2] & 0xFFFFFF00);
		    addrp++;
		    break;
		  }
		  while(nlmiddle--){
		    *addrp++ = piQxelXor[0];
		    *addrp++ = piQxelXor[1];
		    *addrp++ = piQxelXor[2];
		  }
		  switch(rightIndex++){
		  case 0:
		    break;
		  case 1:
		    *addrp = ((*addrp) & 0xFF000000) | (piQxelXor[0] & 0xFFFFFF);
		    break;
		  case 2:
		    *addrp++ = piQxelXor[0];
		    *addrp = ((*addrp) & 0xFFFF0000) | (piQxelXor[1] & 0xFFFF);
		    break;
		  case 3:
		    *addrp++ = piQxelXor[0];
		    *addrp++ = piQxelXor[1];
		    *addrp = ((*addrp) & 0xFFFFFF00) | (piQxelXor[2] & 0xFF);
		    break;
		  }
/*
		  if (e3){
		    e3 &= 0xFFFFFF;
		    switch(rightIndex&3){
		    case 0:
		      *addrp = ((*addrp) & (0xFF000000 | ~e3))
			| (piQxelXor[0] & 0xFFFFFF & e3);
		      break;
		    case 1:
		      *addrp = ((*addrp) & (0xFFFFFF | ~(e3<<24)))
				  + (piQxelXor[0] & 0xFF000000 & (e3<<24));
		      addrp++;
		      *addrp = ((*addrp) & (0xFFFF0000|~(e3 >> 8)))
				  | (piQxelXor[1] & 0xFFFF & (e3 >> 8));
		      break;
		    case 2:
		      *addrp = ((*addrp) & (0xFFFF|~(e3 << 16)))
				  | (piQxelXor[1] & 0xFFFF0000 & (e3 << 16));
		      addrp++;
		      *addrp = ((*addrp) & (0xFFFFFF00|~(e3>>16)))
				  | (piQxelXor[2] & 0xFF & (e3 >> 16));
		      break;
		    case 3:
		      *addrp = ((*addrp) & (0xFF|~(e3<<8)))
				  | (piQxelXor[2] & 0xFFFFFF00 & (e3<<8));
		      addrp++;
		      break;
		    }
		  }
*/
		}
	      }
#else /* GXcopy */
	      addrp = (PixelType *)((char *)addrLineEnd + ((xOffset * 3) & ~0x03));
	      if (x1_or_len <= 1){
		if (x1_or_len)
		  RROP_SOLID24(addrp, xOffset);
	      } else {
		maskbits(xOffset, x1_or_len, e, e3, x1_or_len);
		pidx = xOffset & 3;
		if (e){
		  RROP_SOLID_MASK(addrp, e, pidx-1);
		  addrp++;
		  if (pidx == 3)
		    pidx = 0;
		}
		while (--x1_or_len >= 0){
		  RROP_SOLID(addrp, pidx);
		  addrp++;
		  if (++pidx == 3)
		    pidx = 0;
		}
		if (e3)
		  RROP_SOLID_MASK(addrp, e3, pidx);
	      }
#endif /* GXcopy */
	    }
#else /* PSZ == 24 */
	    if (y1_or_e1 + x1_or_len <= PPW)
	    {
		if (x1_or_len)
		{
		    maskpartialbits(y1_or_e1, x1_or_len, e)
		    RROP_SOLID_MASK((CfbBits *) addrp, e);
		}
	    }
	    else
	    {
	    	maskbits(y1_or_e1, x1_or_len, e, e3, x1_or_len)
	    	if (e)
	    	{
		    RROP_SOLID_MASK((CfbBits *) addrp, e);
		    addrp += PPW;
	    	}
		RROP_SPAN(addrp, x1_or_len)
	    	if (e3)
		    RROP_SOLID_MASK((CfbBits *) addrp, e3);
	    }
#endif /* PSZ == 24 */
	}
#endif /* POLYSEGMENT */
    }
#ifdef POLYSEGMENT
    if (nseg >= 0)
	return (xSegment *) ppt - pSegInit;
#else
    if (npt)
    {
#ifdef EITHER_MODE
	if (!mode)
#endif /* EITHER_MODE */
#ifndef ORIGIN
	{
	    *x1p = _x1;
	    *y1p = _y1;		
	    *x2p = _x2;
	    *y2p = _y2;
	}
#endif /* !ORIGIN */	    
	return ((DDXPointPtr) ppt - pptInit) - 1;
    }

# ifndef ORIGIN
#  define C2  c2
# else
#  define C2  ppt[-1]
# endif
#ifdef EITHER_MODE
    if (pGC->capStyle != CapNotLast &&
	((mode ? (C2 != *((int *) pptInitOrig))
	       : ((_x2 != pptInitOrig->x) ||
		  (_y2 != pptInitOrig->y)))
	 || (ppt == ((int *)pptInitOrig) + 2)))
#endif /* EITHER_MODE */
#ifdef PREVIOUS
    if (pGC->capStyle != CapNotLast &&
	((_x2 != pptInitOrig->x) ||
	 (_y2 != pptInitOrig->y) ||
	 (ppt == ((int *)pptInitOrig) + 2)))
#endif /* PREVIOUS */
#ifdef ORIGIN
    if (pGC->capStyle != CapNotLast &&
	((C2 != *((int *) pptInitOrig)) ||
	 (ppt == ((int *)pptInitOrig) + 2)))
#endif /* !PREVIOUS */
    {
# ifdef REARRANGE
	RROP_DECLARE

	RROP_FETCH_GCPRIV(devPriv);
# endif
#if PSZ == 24
#if RROP == GXcopy
	    switch(xOffset & 3){
	    case 0:
	      *addrp = ((*addrp)&0xFF000000)|(piQxelXor[0] & 0xFFFFFF);
	      break;
	    case 3:
	      *addrp = ((*addrp)&0xFF)|(piQxelXor[2] & 0xFFFFFF00);
	      break;
	    case 1:
	      *addrp = ((*addrp)&0xFFFFFF)|(piQxelXor[0] & 0xFF000000);
	      *(addrp+1) = ((*(addrp+1))&0xFFFF0000)|(piQxelXor[1] & 0xFFFF);
	      break;
	    case 2:
	      *addrp = ((*addrp)&0xFFFF)|(piQxelXor[1] & 0xFFFF0000);
	      *(addrp+1) = ((*(addrp+1))&0xFFFFFF00)|(piQxelXor[2] & 0xFF);
	      break;
	    }
#endif
#if RROP == GXxor
	    switch(xOffset & 3){
	    case 0:
	      *addrp ^= (piQxelXor[0] & 0xFFFFFF);
	      break;
	    case 3:
	      *addrp ^= (piQxelXor[2] & 0xFFFFFF00);
	      break;
	    case 1:
	      *addrp ^= (piQxelXor[0] & 0xFF000000);
	      *(addrp+1) ^= (piQxelXor[1] & 0xFFFF);
	      break;
	    case 2:
	      *addrp ^= (piQxelXor[1] & 0xFFFF0000);
	      *(addrp+1) ^= (piQxelXor[2] & 0xFF);
	      break;
	    }
#endif
#if RROP == GXand
	    switch(xOffset & 3){
	    case 0:
	      *addrp &= (piQxelAnd[0] | 0xFF000000);
	      break;
	    case 3:
	      *addrp &= (piQxelAnd[2] | 0xFF);
	      break;
	    case 1:
	      *addrp &= (0xFFFFFF|piQxelAnd[0]);
	      *(addrp+1) &= (0xFFFF0000|piQxelAnd[1]);
	      break;
	    case 2:
	      *addrp &= (0xFFFF|piQxelAnd[1]);
	      *(addrp+1) &= (0xFFFFFF00|piQxelAnd[2]);
	      break;
	    }
#endif
#if RROP == GXor
	    switch(xOffset & 3){
	    case 0:
	      *addrp |= (piQxelOr[0] & 0xFFFFFF);
	      break;
	    case 3:
	      *addrp |= (piQxelOr[2] & 0xFFFFFF00);
	      break;
	    case 1:
	      *addrp |= (piQxelOr[0] & 0xFF000000);
	      *(addrp+1) |= (piQxelOr[1] & 0xFFFF);
	      break;
	    case 2:
	      *addrp |= (piQxelOr[1] & 0xFFFF0000);
	      *(addrp+1) |= (piQxelOr[2] & 0xFF);
	      break;
	    }
#endif
#if RROP == GXset
	    switch(xOffset & 3){
	    case 0:
	      *addrp = (((*addrp)&(piQxelAnd[0] |0xFF000000))^(piQxelXor[0] & 0xFFFFFF));
	      break;
	    case 3:
	      *addrp = (((*addrp)&(piQxelAnd[2]|0xFF))^(piQxelXor[2] & 0xFFFFFF00));
	      break;
	    case 1:
	      *addrp = (((*addrp)&(piQxelAnd[0]|0xFFFFFF))^(piQxelXor[0] & 0xFF000000));
	      *(addrp+1) = (((*(addrp+1))&(piQxelAnd[1]|0xFFFF0000))^(piQxelXor[1] & 0xFFFF));
	      break;
	    case 2:
	      *addrp = (((*addrp)&(piQxelAnd[1]|0xFFFF))^(piQxelXor[1] & 0xFFFF0000));
	      *(addrp+1) = (((*(addrp+1))&(piQxelAnd[2]|0xFFFFFF00))^(piQxelXor[2] & 0xFF));
	      break;
	    } 
#endif
#else
	RROP_SOLID (addrp);
# endif
    }
#endif /* !POLYSEGMENT */
    RROP_UNDECLARE;
    return -1;
}

#endif /* INCLUDE_DRAW */


#ifdef INCLUDE_OTHERS

#ifdef POLYSEGMENT

void
cfb8SegmentSS1Rect (pDrawable, pGC, nseg, pSegInit)
    DrawablePtr	    pDrawable;
    GCPtr	    pGC;
    int		    nseg;
    xSegment	    *pSegInit;
{
    int	    (*func)(DrawablePtr, GCPtr, int, xSegment *);
    void    (*clip)(DrawablePtr, GCPtr, int, int, int, int, BoxPtr, Bool);
    int	    drawn;
    cfbPrivGCPtr    devPriv;

#if defined(__arm32__) && PSZ != 8
    /* XXX -JJK */
    /* There is a painting bug when PSZ != 8; I need to track it down! */
    cfbSegmentSS(pDrawable, pGC, nseg, pSegInit);
    return;
#endif

    devPriv = cfbGetGCPrivate(pGC);
#ifdef NO_ONE_RECT
    if (REGION_NUM_RECTS(pGC->pCompositeClip) != 1)
    {
       cfbSegmentSS(pDrawable, pGC, nseg, pSegInit);
       return;
    }
#endif
    switch (devPriv->rop)
    {
    case GXcopy:
	func = cfb8SegmentSS1RectCopy;
	clip = cfb8ClippedLineCopy;
#ifdef FAST_MUL
	if (cfbGetPixelWidth (pDrawable) == WIDTH_FAST)
	    func = cfb8SegmentSS1RectShiftCopy;
#endif
	break;
    case GXxor:
	func = cfb8SegmentSS1RectXor;
	clip = cfb8ClippedLineXor;
	break;
    default:
	func = cfb8SegmentSS1RectGeneral;
	clip = cfb8ClippedLineGeneral;
	break;
    }
    while (nseg)
    {
	drawn = (*func) (pDrawable, pGC, nseg, pSegInit);
	if (drawn == -1)
	    break;
	(*clip) (pDrawable, pGC,
			 pSegInit[drawn-1].x1, pSegInit[drawn-1].y1,
			 pSegInit[drawn-1].x2, pSegInit[drawn-1].y2,
			 &pGC->pCompositeClip->extents,
			 pGC->capStyle == CapNotLast);
	pSegInit += drawn;
	nseg -= drawn;
    }
}

#else /* POLYSEGMENT */

void
cfb8LineSS1Rect (pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		mode;
    int		npt;
    DDXPointPtr	pptInit;
{
    int	    (*func)(DrawablePtr, GCPtr, int, int, 
		    DDXPointPtr, DDXPointPtr,
		    int *, int *, int *, int *);
    void    (*clip)(DrawablePtr, GCPtr, int, int, int, int, BoxPtr, Bool);
    int	    drawn;
    cfbPrivGCPtr    devPriv;
    int x1, y1, x2, y2;
    DDXPointPtr pptInitOrig = pptInit;

#if defined(__arm32__) && PSZ != 8
    /* XXX -JJK */
    /* There is a painting bug when PSZ != 8; I need to track it down! */
    cfbLineSS(pDrawable, pGC, mode, npt, pptInit);
    return;
#endif

    devPriv = cfbGetGCPrivate(pGC);
#ifdef NO_ONE_RECT
    if (REGION_NUM_RECTS(pGC->pCompositeClip) != 1)
    {
       cfbLineSS(pDrawable, pGC, mode, npt, pptInit);
       return;
    }
#endif
    switch (devPriv->rop)
    {
    case GXcopy:
	func = cfb8LineSS1RectCopy;
	clip = cfb8ClippedLineCopy;
	if (mode == CoordModePrevious)
	    func = cfb8LineSS1RectPreviousCopy;
	break;
    case GXxor:
	func = cfb8LineSS1RectXor;
	clip = cfb8ClippedLineXor;
	break;
    default:
	func = cfb8LineSS1RectGeneral;
	clip = cfb8ClippedLineGeneral;
	break;
    }
    if (mode == CoordModePrevious)
    {
	x1 = pptInit->x;
	y1 = pptInit->y;
	while (npt > 1)
	{
	    drawn = (*func) (pDrawable, pGC, mode, npt, pptInit, pptInitOrig,
			     &x1, &y1, &x2, &y2);
	    if (drawn == -1)
		break;
	    (*clip) (pDrawable, pGC, x1, y1, x2, y2,
		     &pGC->pCompositeClip->extents,
		     drawn != npt - 1 || pGC->capStyle == CapNotLast);
	    pptInit += drawn;
	    npt -= drawn;
	    x1 = x2;
	    y1 = y2;
	}
    }
    else
    {
	while (npt > 1)
	{
	    drawn = (*func) (pDrawable, pGC, mode, npt, pptInit, pptInitOrig,
			     &x1, &y1, &x2, &y2);
	    if (drawn == -1)
		break;
	    (*clip) (pDrawable, pGC,
		     pptInit[drawn-1].x, pptInit[drawn-1].y,
		     pptInit[drawn].x, pptInit[drawn].y,
		     &pGC->pCompositeClip->extents,
		     drawn != npt - 1 || pGC->capStyle == CapNotLast);
	    pptInit += drawn;
	    npt -= drawn;
	}
    }
}

#endif /* else POLYSEGMENT */
#endif /* INCLUDE_OTHERS */

#if !defined(POLYSEGMENT) && !defined (PREVIOUS)

void
RROP_NAME (cfb8ClippedLine) (pDrawable, pGC, x1, y1, x2, y2, boxp, shorten)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		x1, y1, x2, y2;
    BoxPtr	boxp;
    Bool	shorten;
{
    int		    oc1, oc2;
    int		    e, e1, e3, len;
    int		    adx, ady;

    PixelType	    *addr;
    int		    nwidth;
    int		    stepx, stepy;
    int		    xorg, yorg;
    int             new_x1, new_y1, new_x2, new_y2;
    Bool	    pt1_clipped, pt2_clipped;
    int		    changex, changey, result;
#if PSZ == 24
    PixelType   *addrLineEnd;
    char *addrb;
    int stepx3, stepy3;
#endif
    int		    octant;
    unsigned int    bias = miGetZeroLineBias(pDrawable->pScreen);

    cfbGetPixelWidthAndPointer(pDrawable, nwidth, addr);

    xorg = pDrawable->x;
    yorg = pDrawable->y;
    x1 += xorg;
    y1 += yorg;
    x2 += xorg;
    y2 += yorg;
    oc1 = 0;
    oc2 = 0;
    OUTCODES (oc1, x1, y1, boxp);
    OUTCODES (oc2, x2, y2, boxp);

    if (oc1 & oc2)
	return;

    CalcLineDeltas(x1, y1, x2, y2, adx, ady, stepx, stepy, 1, nwidth, octant);

    if (adx <= ady)
    {
	int	t;

	t = adx;
	adx = ady;
	ady = t;

	t = stepx;
	stepx = stepy;
	stepy = t;
	
	SetYMajorOctant(octant);
    }
    e = - adx;
    e1 = ady << 1;
    e3 = - (adx << 1);

    FIXUP_ERROR(e, octant, bias);

    new_x1 = x1;
    new_y1 = y1;
    new_x2 = x2;
    new_y2 = y2;
    pt1_clipped = 0;
    pt2_clipped = 0;

    if (IsXMajorOctant(octant))
    {
	result = miZeroClipLine(boxp->x1, boxp->y1, boxp->x2 - 1, boxp->y2 - 1,
				&new_x1, &new_y1, &new_x2, &new_y2,
				adx, ady,
				&pt1_clipped, &pt2_clipped,
				octant, bias, oc1, oc2);
	if (result == -1)
	    return;
	
	len = abs(new_x2 - new_x1) - 1; /* this routine needs the "-1" */
	
	/* if we've clipped the endpoint, always draw the full length
	 * of the segment, because then the capstyle doesn't matter 
	 * if x2,y2 isn't clipped, use the capstyle
	 * (shorten == TRUE <--> CapNotLast)
	 */
	if (pt2_clipped || !shorten)
	    len++;
	
	if (pt1_clipped)
	{
	    /* must calculate new error terms */
	    changex = abs(new_x1 - x1);
	    changey = abs(new_y1 - y1);
	    e = e + changey * e3 + changex * e1;	    
	}
    }
    else /* Y_AXIS */
    {
	result = miZeroClipLine(boxp->x1, boxp->y1, boxp->x2 - 1, boxp->y2 - 1,
				&new_x1, &new_y1, &new_x2, &new_y2,
				ady, adx,
				&pt1_clipped, &pt2_clipped,
				octant, bias, oc1, oc2);
	if (result == -1)
	    return;
	
	len = abs(new_y2 - new_y1) - 1; /* this routine needs the "-1" */
	
	/* if we've clipped the endpoint, always draw the full length
	 * of the segment, because then the capstyle doesn't matter 
	 * if x2,y2 isn't clipped, use the capstyle
	 * (shorten == TRUE <--> CapNotLast)
	 */
	if (pt2_clipped || !shorten)
	    len++;
	
	if (pt1_clipped)
	{
	    /* must calculate new error terms */
	    changex = abs(new_x1 - x1);
	    changey = abs(new_y1 - y1);
	    e = e + changex * e3 + changey * e1;
	}
    }
    x1 = new_x1;
    y1 = new_y1;
    {
    register PixelType	*addrp;
    RROP_DECLARE

    RROP_FETCH_GC(pGC);

#if PSZ == 24
    addrLineEnd = addr + (y1 * nwidth);
    addrb = (char *)addrLineEnd + x1 * 3;
    if (stepx == 1  ||  stepx == -1){
      stepx3 = stepx * 3;
      stepy3 = stepy * sizeof (CfbBits);
    } else {
      stepx3 = stepx * sizeof (CfbBits);
      stepy3 = stepy * 3;
    }
#else
    addrp = addr + (y1 * nwidth) + x1;
#endif

#ifndef REARRANGE
    if (!ady)
    {
#if PSZ == 24
#define body {\
	    body_rop \
	    addrb += stepx3; \
	}
#else
#define body	{ RROP_SOLID(addrp); addrp += stepx; }
#endif
	while (len >= PGSZB)
	{
	    body body body body
#if PGSZ == 64
	    body body body body
#endif
	    len -= PGSZB;
	}
	switch (len)
	{
#if PGSZ == 64
	case  7: body case 6: body case 5: body case 4: body
#endif
	case  3: body case 2: body case 1: body
	}
#undef body
    }
    else
#endif /* !REARRANGE */
    {
#if PSZ == 24
#define body {\
	    body_rop \
	    addrb += stepx3; \
	    e += e1; \
	    if (e >= 0) \
	    { \
		addrb += stepy3; \
		e += e3; \
	    } \
	}
#else
#define body {\
	    RROP_SOLID(addrp); \
	    addrp += stepx; \
	    e += e1; \
	    if (e >= 0) \
	    { \
		addrp += stepy; \
		e += e3; \
	     } \
	}
#endif

#ifdef LARGE_INSTRUCTION_CACHE
	while ((len -= PGSZB) >= 0)
	{
	    body body body body
#if PGSZ == 64
	    body body body body
#endif
	}
	switch (len)
	{
	case  -1: body case -2: body case -3: body
#if PGSZ == 64
	case  -4: body case -5: body case -6: body case -7: body
#endif
	}
#else /* !LARGE_INSTRUCTION_CACHE */
	IMPORTANT_START;

	while ((len -= 2) >= 0)
	{
	    body body;
	}
	if (len & 1)
	    body;

	IMPORTANT_END;
#endif /* LARGE_INSTRUCTION_CACHE */
    }
#if PSZ == 24
    body_rop
#else
    RROP_SOLID(addrp);
#endif
#undef body
    RROP_UNDECLARE
    }
}

#endif /* !POLYSEGMENT && !PREVIOUS */
#endif /* PIXEL_ADDR */
