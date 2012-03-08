/*
 * $XFree86: xc/programs/Xserver/fb/fbpict.h,v 1.11 2002/09/26 02:56:48 keithp Exp $
 *
 * Copyright © 2000 Keith Packard, member of The XFree86 Project, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _FBPICT_H_
#define _FBPICT_H_

#define FbIntMult(a,b,t) ( (t) = (a) * (b) + 0x80, ( ( ( (t)>>8 ) + (t) )>>8 ) )
#define FbIntDiv(a,b)	 (((CARD16) (a) * 255) / (b))

#define FbGet8(v,i)   ((CARD16) (CARD8) ((v) >> i))

/*
 * There are two ways of handling alpha -- either as a single unified value or
 * a separate value for each component, hence each macro must have two
 * versions.  The unified alpha version has a 'U' at the end of the name,
 * the component version has a 'C'.  Similarly, functions which deal with
 * this difference will have two versions using the same convention.
 */

#define FbOverU(x,y,i,a,t) ((t) = FbIntMult(FbGet8(y,i),(a),(t)) + FbGet8(x,i),\
			   (CARD32) ((CARD8) ((t) | (0 - ((t) >> 8)))) << (i))

#define FbOverC(x,y,i,a,t) ((t) = FbIntMult(FbGet8(y,i),FbGet8(a,i),(t)) + FbGet8(x,i),\
			    (CARD32) ((CARD8) ((t) | (0 - ((t) >> 8)))) << (i))

#define FbInU(x,i,a,t) ((CARD32) FbIntMult(FbGet8(x,i),(a),(t)) << (i))

#define FbInC(x,i,a,t) ((CARD32) FbIntMult(FbGet8(x,i),FbGet8(a,i),(t)) << (i))

#define FbGen(x,y,i,ax,ay,t,u,v) ((t) = (FbIntMult(FbGet8(y,i),ay,(u)) + \
					 FbIntMult(FbGet8(x,i),ax,(v))),\
				  (CARD32) ((CARD8) ((t) | \
						     (0 - ((t) >> 8)))) << (i))

#define FbAdd(x,y,i,t)	((t) = FbGet8(x,i) + FbGet8(y,i), \
			 (CARD32) ((CARD8) ((t) | (0 - ((t) >> 8)))) << (i))


typedef void	(*CompositeFunc) (CARD8      op,
				  PicturePtr pSrc,
				  PicturePtr pMask,
				  PicturePtr pDst,
				  INT16      xSrc,
				  INT16      ySrc,
				  INT16      xMask,
				  INT16      yMask,
				  INT16      xDst,
				  INT16      yDst,
				  CARD16     width,
				  CARD16     height);

typedef struct _FbCompositeOperand FbCompositeOperand;

typedef CARD32 (*FbCompositeFetch)(FbCompositeOperand *op);
typedef void (*FbCompositeStore) (FbCompositeOperand *op, CARD32 value);

typedef void (*FbCompositeStep) (FbCompositeOperand *op);
typedef void (*FbCompositeSet) (FbCompositeOperand *op, int x, int y);

struct _FbCompositeOperand {
    union {
	struct {
	    FbBits		*top_line;
	    int			left_offset;
	    
	    int			start_offset;
	    FbBits		*line;
	    CARD32		offset;
	    FbStride		stride;
	    int			bpp;
	} drawable;
	struct {
	    int			alpha_dx;
	    int			alpha_dy;
	} external;
	struct {
	    int			top_y;
	    int			left_x;
	    int			start_x;
	    int			x;
	    int			y;
	    PictTransformPtr	transform;
	    int			filter;
	} transform;
    } u;
    FbCompositeFetch	fetch;
    FbCompositeFetch	fetcha;
    FbCompositeStore	store;
    FbCompositeStep	over;
    FbCompositeStep	down;
    FbCompositeSet	set;
    miIndexedPtr	indexed;
    RegionPtr		clip;
};

typedef void (*FbCombineFunc) (FbCompositeOperand	*src,
			       FbCompositeOperand	*msk,
			       FbCompositeOperand	*dst);

/*
 * indexed by op
 */
extern FbCombineFunc	fbCombineFunc[];

typedef struct _FbAccessMap {
    CARD32		format;
    FbCompositeFetch	fetch;
    FbCompositeFetch	fetcha;
    FbCompositeStore	store;
} FbAccessMap;

/*
 * search on format
 */
extern FbAccessMap  fbAccessMap[];

/* fbcompose.c */

typedef struct _fbCompSrc {
    CARD32	value;
    CARD32	alpha;
} FbCompSrc;

/*
 * All compositing operators *
 */

CARD32
fbCombineMaskU (FbCompositeOperand   *src,
		FbCompositeOperand   *msk);

FbCompSrc
fbCombineMaskC (FbCompositeOperand   *src,
		FbCompositeOperand   *msk);

CARD32
fbCombineMaskValueC (FbCompositeOperand   *src,
		     FbCompositeOperand   *msk);

CARD32
fbCombineMaskAlphaU (FbCompositeOperand   *src,
		     FbCompositeOperand   *msk);

CARD32
fbCombineMaskAlphaC (FbCompositeOperand   *src,
		     FbCompositeOperand   *msk);


#if 0
CARD32
FbCombineMask (FbCompositeOperand   *src,
	       FbCompositeOperand   *msk);
#endif

void
fbCombineClear (FbCompositeOperand   *src,
		FbCompositeOperand   *msk,
		FbCompositeOperand   *dst);

void
fbCombineSrcU (FbCompositeOperand    *src,
	       FbCompositeOperand    *msk,
	       FbCompositeOperand    *dst);

void
fbCombineSrcC (FbCompositeOperand    *src,
	       FbCompositeOperand    *msk,
	       FbCompositeOperand    *dst);

void
fbCombineDst (FbCompositeOperand    *src,
	      FbCompositeOperand    *msk,
	      FbCompositeOperand    *dst);

void
fbCombineOverU (FbCompositeOperand   *src,
		FbCompositeOperand   *msk,
		FbCompositeOperand   *dst);

void
fbCombineOverC (FbCompositeOperand   *src,
		FbCompositeOperand   *msk,
		FbCompositeOperand   *dst);

void
fbCombineOverReverseU (FbCompositeOperand    *src,
		       FbCompositeOperand    *msk,
		       FbCompositeOperand    *dst);

void
fbCombineOverReverseC (FbCompositeOperand    *src,
		       FbCompositeOperand    *msk,
		       FbCompositeOperand    *dst);

void
fbCombineInU (FbCompositeOperand	    *src,
	      FbCompositeOperand	    *msk,
	      FbCompositeOperand	    *dst);

void
fbCombineInC (FbCompositeOperand	    *src,
	      FbCompositeOperand	    *msk,
	      FbCompositeOperand	    *dst);

void
fbCombineInReverseU (FbCompositeOperand  *src,
		     FbCompositeOperand  *msk,
		     FbCompositeOperand  *dst);

void
fbCombineInReverseC (FbCompositeOperand  *src,
		     FbCompositeOperand  *msk,
		     FbCompositeOperand  *dst);

void
fbCombineOutU (FbCompositeOperand    *src,
	       FbCompositeOperand    *msk,
	       FbCompositeOperand    *dst);

void
fbCombineOutC (FbCompositeOperand    *src,
	       FbCompositeOperand    *msk,
	       FbCompositeOperand    *dst);

void
fbCombineOutReverseU (FbCompositeOperand *src,
		      FbCompositeOperand *msk,
		      FbCompositeOperand *dst);

void
fbCombineOutReverseC (FbCompositeOperand *src,
		      FbCompositeOperand *msk,
		      FbCompositeOperand *dst);

void
fbCombineAtopU (FbCompositeOperand   *src,
		FbCompositeOperand   *msk,
		FbCompositeOperand   *dst);


void
fbCombineAtopC (FbCompositeOperand   *src,
		FbCompositeOperand   *msk,
		FbCompositeOperand   *dst);

void
fbCombineAtopReverseU (FbCompositeOperand    *src,
		       FbCompositeOperand    *msk,
		       FbCompositeOperand    *dst);

void
fbCombineAtopReverseC (FbCompositeOperand    *src,
		       FbCompositeOperand    *msk,
		       FbCompositeOperand    *dst);

void
fbCombineXorU (FbCompositeOperand    *src,
	       FbCompositeOperand    *msk,
	       FbCompositeOperand    *dst);

void
fbCombineXorC (FbCompositeOperand    *src,
	       FbCompositeOperand    *msk,
	       FbCompositeOperand    *dst);


void
fbCombineAddU (FbCompositeOperand    *src,
	       FbCompositeOperand    *msk,
	       FbCompositeOperand    *dst);

void
fbCombineAddC (FbCompositeOperand    *src,
	       FbCompositeOperand    *msk,
	       FbCompositeOperand    *dst);

void
fbCombineSaturateU (FbCompositeOperand   *src,
		    FbCompositeOperand   *msk,
		    FbCompositeOperand   *dst);

void
fbCombineSaturateC (FbCompositeOperand   *src,
		    FbCompositeOperand   *msk,
		    FbCompositeOperand   *dst);

CARD8
fbCombineDisjointOutPart (CARD8 a, CARD8 b);

CARD8
fbCombineDisjointInPart (CARD8 a, CARD8 b);

void
fbCombineDisjointGeneralU (FbCompositeOperand   *src,
			   FbCompositeOperand   *msk,
			   FbCompositeOperand   *dst,
			   CARD8		combine);

void
fbCombineDisjointGeneralC (FbCompositeOperand   *src,
			   FbCompositeOperand   *msk,
			   FbCompositeOperand   *dst,
			   CARD8		combine);

void
fbCombineDisjointOverU (FbCompositeOperand   *src,
			FbCompositeOperand   *msk,
			FbCompositeOperand   *dst);

void
fbCombineDisjointOverC (FbCompositeOperand   *src,
			FbCompositeOperand   *msk,
			FbCompositeOperand   *dst);

void
fbCombineDisjointOverReverseU (FbCompositeOperand    *src,
			       FbCompositeOperand    *msk,
			       FbCompositeOperand    *dst);

void
fbCombineDisjointOverReverseC (FbCompositeOperand    *src,
			       FbCompositeOperand    *msk,
			       FbCompositeOperand    *dst);

void
fbCombineDisjointInU (FbCompositeOperand	    *src,
		      FbCompositeOperand	    *msk,
		      FbCompositeOperand	    *dst);

void
fbCombineDisjointInC (FbCompositeOperand	    *src,
		      FbCompositeOperand	    *msk,
		      FbCompositeOperand	    *dst);

void
fbCombineDisjointInReverseU (FbCompositeOperand  *src,
                             FbCompositeOperand  *msk,
                             FbCompositeOperand  *dst);

void
fbCombineDisjointInReverseC (FbCompositeOperand  *src,
                             FbCompositeOperand  *msk,
                             FbCompositeOperand  *dst);

void
fbCombineDisjointOutU (FbCompositeOperand    *src,
                       FbCompositeOperand    *msk,
                       FbCompositeOperand    *dst);

void
fbCombineDisjointOutC (FbCompositeOperand    *src,
                       FbCompositeOperand    *msk,
                       FbCompositeOperand    *dst);
void
fbCombineDisjointOutReverseU (FbCompositeOperand *src,
                              FbCompositeOperand *msk,
                              FbCompositeOperand *dst);

void
fbCombineDisjointOutReverseC (FbCompositeOperand *src,
                              FbCompositeOperand *msk,
                              FbCompositeOperand *dst);

void
fbCombineDisjointAtopU (FbCompositeOperand   *src,
                        FbCompositeOperand   *msk,
                        FbCompositeOperand   *dst);

void
fbCombineDisjointAtopC (FbCompositeOperand   *src,
                        FbCompositeOperand   *msk,
                        FbCompositeOperand   *dst);

void
fbCombineDisjointAtopReverseU (FbCompositeOperand    *src,
                               FbCompositeOperand    *msk,
                               FbCompositeOperand    *dst);

void
fbCombineDisjointAtopReverseC (FbCompositeOperand    *src,
                               FbCompositeOperand    *msk,
                               FbCompositeOperand    *dst);

void
fbCombineDisjointXorU (FbCompositeOperand    *src,
                       FbCompositeOperand    *msk,
                       FbCompositeOperand    *dst);

void
fbCombineDisjointXorC (FbCompositeOperand    *src,
                       FbCompositeOperand    *msk,
                       FbCompositeOperand    *dst);

CARD8
fbCombineConjointOutPart (CARD8 a, CARD8 b);

CARD8
fbCombineConjointInPart (CARD8 a, CARD8 b);


void
fbCombineConjointGeneralU (FbCompositeOperand   *src,
                           FbCompositeOperand   *msk,
                           FbCompositeOperand   *dst,
                           CARD8                combine);

void
fbCombineConjointGeneralC (FbCompositeOperand   *src,
                           FbCompositeOperand   *msk,
                           FbCompositeOperand   *dst,
                           CARD8                combine);

void
fbCombineConjointOverU (FbCompositeOperand   *src,
                        FbCompositeOperand   *msk,
                        FbCompositeOperand   *dst);

void
fbCombineConjointOverC (FbCompositeOperand   *src,
                        FbCompositeOperand   *msk,
                        FbCompositeOperand   *dst);
void
fbCombineConjointOverReverseU (FbCompositeOperand    *src,
                               FbCompositeOperand    *msk,
                               FbCompositeOperand    *dst);

void
fbCombineConjointOverReverseC (FbCompositeOperand    *src,
                               FbCompositeOperand    *msk,
                               FbCompositeOperand    *dst);

void
fbCombineConjointInU (FbCompositeOperand            *src,
                      FbCompositeOperand            *msk,
                      FbCompositeOperand            *dst);

void
fbCombineConjointInC (FbCompositeOperand            *src,
                      FbCompositeOperand            *msk,
                      FbCompositeOperand            *dst);

void
fbCombineConjointInReverseU (FbCompositeOperand  *src,
                             FbCompositeOperand  *msk,
                             FbCompositeOperand  *dst);


void
fbCombineConjointInReverseC (FbCompositeOperand  *src,
                             FbCompositeOperand  *msk,
                             FbCompositeOperand  *dst);

void
fbCombineConjointOutU (FbCompositeOperand    *src,
                       FbCompositeOperand    *msk,
                       FbCompositeOperand    *dst);

void
fbCombineConjointOutC (FbCompositeOperand    *src,
                       FbCompositeOperand    *msk,
                       FbCompositeOperand    *dst);

void
fbCombineConjointOutReverseU (FbCompositeOperand *src,
                              FbCompositeOperand *msk,
                              FbCompositeOperand *dst);

void
fbCombineConjointOutReverseC (FbCompositeOperand *src,
                              FbCompositeOperand *msk,
                              FbCompositeOperand *dst);

void
fbCombineConjointAtopU (FbCompositeOperand   *src,
                        FbCompositeOperand   *msk,
                        FbCompositeOperand   *dst);

void
fbCombineConjointAtopC (FbCompositeOperand   *src,
                        FbCompositeOperand   *msk,
                        FbCompositeOperand   *dst);

void
fbCombineConjointAtopReverseU (FbCompositeOperand    *src,
                               FbCompositeOperand    *msk,
                               FbCompositeOperand    *dst);
void
fbCombineConjointAtopReverseC (FbCompositeOperand    *src,
                               FbCompositeOperand    *msk,
                               FbCompositeOperand    *dst);

void
fbCombineConjointXorU (FbCompositeOperand    *src,
                       FbCompositeOperand    *msk,
                       FbCompositeOperand    *dst);

void
fbCombineConjointXorC (FbCompositeOperand    *src,
                       FbCompositeOperand    *msk,
                       FbCompositeOperand    *dst);

/*
 * All fetch functions
 */

CARD32
fbFetch_a8r8g8b8 (FbCompositeOperand *op);

CARD32
fbFetch_x8r8g8b8 (FbCompositeOperand *op);

CARD32
fbFetch_a8b8g8r8 (FbCompositeOperand *op);

CARD32
fbFetch_x8b8g8r8 (FbCompositeOperand *op);

CARD32
fbFetch_r8g8b8 (FbCompositeOperand *op);

CARD32
fbFetch_b8g8r8 (FbCompositeOperand *op);

CARD32
fbFetch_r5g6b5 (FbCompositeOperand *op);

CARD32
fbFetch_b5g6r5 (FbCompositeOperand *op);

CARD32
fbFetch_a1r5g5b5 (FbCompositeOperand *op);

CARD32
fbFetch_x1r5g5b5 (FbCompositeOperand *op);

CARD32
fbFetch_a1b5g5r5 (FbCompositeOperand *op);

CARD32
fbFetch_x1b5g5r5 (FbCompositeOperand *op);

CARD32
fbFetch_a4r4g4b4 (FbCompositeOperand *op);

CARD32
fbFetch_x4r4g4b4 (FbCompositeOperand *op);

CARD32
fbFetch_a4b4g4r4 (FbCompositeOperand *op);

CARD32
fbFetch_x4b4g4r4 (FbCompositeOperand *op);

CARD32
fbFetch_a8 (FbCompositeOperand *op);

CARD32
fbFetcha_a8 (FbCompositeOperand *op);

CARD32
fbFetch_r3g3b2 (FbCompositeOperand *op);

CARD32
fbFetch_b2g3r3 (FbCompositeOperand *op);

CARD32
fbFetch_a2r2g2b2 (FbCompositeOperand *op);

CARD32
fbFetch_a2b2g2r2 (FbCompositeOperand *op);

CARD32
fbFetch_c8 (FbCompositeOperand *op);

CARD32
fbFetch_a4 (FbCompositeOperand *op);

CARD32
fbFetcha_a4 (FbCompositeOperand *op);

CARD32
fbFetch_r1g2b1 (FbCompositeOperand *op);

CARD32
fbFetch_b1g2r1 (FbCompositeOperand *op);

CARD32
fbFetch_a1r1g1b1 (FbCompositeOperand *op);

CARD32
fbFetch_a1b1g1r1 (FbCompositeOperand *op);

CARD32
fbFetch_c4 (FbCompositeOperand *op);

CARD32
fbFetch_a1 (FbCompositeOperand *op);

CARD32
fbFetcha_a1 (FbCompositeOperand *op);

CARD32
fbFetch_g1 (FbCompositeOperand *op);

void
fbStore_a8r8g8b8 (FbCompositeOperand *op, CARD32 value);

void
fbStore_x8r8g8b8 (FbCompositeOperand *op, CARD32 value);

void
fbStore_a8b8g8r8 (FbCompositeOperand *op, CARD32 value);

void
fbStore_x8b8g8r8 (FbCompositeOperand *op, CARD32 value);

void
fbStore_r8g8b8 (FbCompositeOperand *op, CARD32 value);

void
fbStore_b8g8r8 (FbCompositeOperand *op, CARD32 value);

void
fbStore_r5g6b5 (FbCompositeOperand *op, CARD32 value);

void
fbStore_b5g6r5 (FbCompositeOperand *op, CARD32 value);

void
fbStore_a1r5g5b5 (FbCompositeOperand *op, CARD32 value);

void
fbStore_x1r5g5b5 (FbCompositeOperand *op, CARD32 value);

void
fbStore_a1b5g5r5 (FbCompositeOperand *op, CARD32 value);

void
fbStore_x1b5g5r5 (FbCompositeOperand *op, CARD32 value);

void
fbStore_a4r4g4b4 (FbCompositeOperand *op, CARD32 value);

void
fbStore_x4r4g4b4 (FbCompositeOperand *op, CARD32 value);

void
fbStore_a4b4g4r4 (FbCompositeOperand *op, CARD32 value);

void
fbStore_x4b4g4r4 (FbCompositeOperand *op, CARD32 value);

void
fbStore_a8 (FbCompositeOperand *op, CARD32 value);

void
fbStore_r3g3b2 (FbCompositeOperand *op, CARD32 value);

void
fbStore_b2g3r3 (FbCompositeOperand *op, CARD32 value);

void
fbStore_a2r2g2b2 (FbCompositeOperand *op, CARD32 value);

void
fbStore_c8 (FbCompositeOperand *op, CARD32 value);

void
fbStore_g8 (FbCompositeOperand *op, CARD32 value);


void
fbStore_a4 (FbCompositeOperand *op, CARD32 value);

void
fbStore_r1g2b1 (FbCompositeOperand *op, CARD32 value);

void
fbStore_b1g2r1 (FbCompositeOperand *op, CARD32 value);

void
fbStore_a1r1g1b1 (FbCompositeOperand *op, CARD32 value);

void
fbStore_a1b1g1r1 (FbCompositeOperand *op, CARD32 value);

void
fbStore_c4 (FbCompositeOperand *op, CARD32 value);

void
fbStore_g4 (FbCompositeOperand *op, CARD32 value);

void
fbStore_a1 (FbCompositeOperand *op, CARD32 value);

void
fbStore_g1 (FbCompositeOperand *op, CARD32 value);

CARD32
fbFetch_external (FbCompositeOperand *op);

CARD32
fbFetch_transform (FbCompositeOperand *op);

CARD32
fbFetcha_transform (FbCompositeOperand *op);

CARD32
fbFetcha_external (FbCompositeOperand *op);

void
fbStore_external (FbCompositeOperand *op, CARD32 value);

Bool
fbBuildOneCompositeOperand (PicturePtr		pPict,
			    FbCompositeOperand	*op,
			    INT16		x,
			    INT16		y);

Bool
fbBuildCompositeOperand (PicturePtr	    pPict,
			 FbCompositeOperand *op,
			 INT16		    x,
			 INT16		    y,
			 Bool		    transform,
			 Bool		    alpha);
void
fbCompositeGeneral (CARD8	op,
		    PicturePtr	pSrc,
		    PicturePtr	pMask,
		    PicturePtr	pDst,
		    INT16	xSrc,
		    INT16	ySrc,
		    INT16	xMask,
		    INT16	yMask,
		    INT16	xDst,
		    INT16	yDst,
		    CARD16	width,
		    CARD16	height);


/* fbpict.c */
CARD32
fbOver (CARD32 x, CARD32 y);

CARD32
fbOver24 (CARD32 x, CARD32 y);

CARD32
fbIn (CARD32 x, CARD8 y);

void
fbCompositeSolidMask_nx8x8888 (CARD8      op,
			       PicturePtr pSrc,
			       PicturePtr pMask,
			       PicturePtr pDst,
			       INT16      xSrc,
			       INT16      ySrc,
			       INT16      xMask,
			       INT16      yMask,
			       INT16      xDst,
			       INT16      yDst,
			       CARD16     width,
			       CARD16     height);

void
fbCompositeSolidMask_nx8x0888 (CARD8      op,
			       PicturePtr pSrc,
			       PicturePtr pMask,
			       PicturePtr pDst,
			       INT16      xSrc,
			       INT16      ySrc,
			       INT16      xMask,
			       INT16      yMask,
			       INT16      xDst,
			       INT16      yDst,
			       CARD16     width,
			       CARD16     height);

void
fbCompositeSolidMask_nx8888x8888C (CARD8      op,
				   PicturePtr pSrc,
				   PicturePtr pMask,
				   PicturePtr pDst,
				   INT16      xSrc,
				   INT16      ySrc,
				   INT16      xMask,
				   INT16      yMask,
				   INT16      xDst,
				   INT16      yDst,
				   CARD16     width,
				   CARD16     height);

void
fbCompositeSolidMask_nx8x0565 (CARD8      op,
			       PicturePtr pSrc,
			       PicturePtr pMask,
			       PicturePtr pDst,
			       INT16      xSrc,
			       INT16      ySrc,
			       INT16      xMask,
			       INT16      yMask,
			       INT16      xDst,
			       INT16      yDst,
			       CARD16     width,
			       CARD16     height);

void
fbCompositeSolidMask_nx8888x0565C (CARD8      op,
				   PicturePtr pSrc,
				   PicturePtr pMask,
				   PicturePtr pDst,
				   INT16      xSrc,
				   INT16      ySrc,
				   INT16      xMask,
				   INT16      yMask,
				   INT16      xDst,
				   INT16      yDst,
				   CARD16     width,
				   CARD16     height);

void
fbCompositeSrc_8888x8888 (CARD8      op,
			  PicturePtr pSrc,
			  PicturePtr pMask,
			  PicturePtr pDst,
			  INT16      xSrc,
			  INT16      ySrc,
			  INT16      xMask,
			  INT16      yMask,
			  INT16      xDst,
			  INT16      yDst,
			  CARD16     width,
			  CARD16     height);

void
fbCompositeSrc_8888x0888 (CARD8      op,
			 PicturePtr pSrc,
			 PicturePtr pMask,
			 PicturePtr pDst,
			 INT16      xSrc,
			 INT16      ySrc,
			 INT16      xMask,
			 INT16      yMask,
			 INT16      xDst,
			 INT16      yDst,
			 CARD16     width,
			 CARD16     height);

void
fbCompositeSrc_8888x0565 (CARD8      op,
			  PicturePtr pSrc,
			  PicturePtr pMask,
			  PicturePtr pDst,
			  INT16      xSrc,
			  INT16      ySrc,
			  INT16      xMask,
			  INT16      yMask,
			  INT16      xDst,
			  INT16      yDst,
			  CARD16     width,
			  CARD16     height);

void
fbCompositeSrc_0565x0565 (CARD8      op,
			  PicturePtr pSrc,
			  PicturePtr pMask,
			  PicturePtr pDst,
			  INT16      xSrc,
			  INT16      ySrc,
			  INT16      xMask,
			  INT16      yMask,
			  INT16      xDst,
			  INT16      yDst,
			  CARD16     width,
			  CARD16     height);

void
fbCompositeSrcAdd_8000x8000 (CARD8	op,
			     PicturePtr pSrc,
			     PicturePtr pMask,
			     PicturePtr pDst,
			     INT16      xSrc,
			     INT16      ySrc,
			     INT16      xMask,
			     INT16      yMask,
			     INT16      xDst,
			     INT16      yDst,
			     CARD16     width,
			     CARD16     height);

void
fbCompositeSrcAdd_8888x8888 (CARD8	op,
			     PicturePtr pSrc,
			     PicturePtr pMask,
			     PicturePtr pDst,
			     INT16      xSrc,
			     INT16      ySrc,
			     INT16      xMask,
			     INT16      yMask,
			     INT16      xDst,
			     INT16      yDst,
			     CARD16     width,
			     CARD16     height);

void
fbCompositeSrcAdd_1000x1000 (CARD8	op,
			     PicturePtr pSrc,
			     PicturePtr pMask,
			     PicturePtr pDst,
			     INT16      xSrc,
			     INT16      ySrc,
			     INT16      xMask,
			     INT16      yMask,
			     INT16      xDst,
			     INT16      yDst,
			     CARD16     width,
			     CARD16     height);

void
fbCompositeSolidMask_nx1xn (CARD8      op,
			    PicturePtr pSrc,
			    PicturePtr pMask,
			    PicturePtr pDst,
			    INT16      xSrc,
			    INT16      ySrc,
			    INT16      xMask,
			    INT16      yMask,
			    INT16      xDst,
			    INT16      yDst,
			    CARD16     width,
			    CARD16     height);

void
fbComposite (CARD8      op,
	     PicturePtr pSrc,
	     PicturePtr pMask,
	     PicturePtr pDst,
	     INT16      xSrc,
	     INT16      ySrc,
	     INT16      xMask,
	     INT16      yMask,
	     INT16      xDst,
	     INT16      yDst,
	     CARD16     width,
	     CARD16     height);

/* fbtrap.c */
void
fbRasterizeTrapezoid (PicturePtr    alpha,
		      xTrapezoid    *trap,
		      int	    x_off,
		      int	    y_off);

#endif /* _FBPICT_H_ */
