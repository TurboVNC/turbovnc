/* $XConsortium: miwideline.h,v 1.11 94/04/17 20:28:02 dpw Exp $ */
/*

Copyright (c) 1988  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/

/* Author:  Keith Packard, MIT X Consortium */

#include "mispans.h"

/* 
 * interface data to span-merging polygon filler
 */

typedef struct _SpanData {
    SpanGroup	fgGroup, bgGroup;
} SpanDataRec, *SpanDataPtr;

#define AppendSpanGroup(pGC, pixel, spanPtr, spanData) { \
	SpanGroup   *group, *othergroup = NULL; \
	if (pixel == pGC->fgPixel) \
	{ \
	    group = &spanData->fgGroup; \
	    if (pGC->lineStyle == LineDoubleDash) \
		othergroup = &spanData->bgGroup; \
	} \
	else \
	{ \
	    group = &spanData->bgGroup; \
	    othergroup = &spanData->fgGroup; \
	} \
	miAppendSpans (group, othergroup, spanPtr); \
}

/*
 * Polygon edge description for integer wide-line routines
 */

typedef struct _PolyEdge {
    int	    height;	/* number of scanlines to process */
    int	    x;		/* starting x coordinate */
    int	    stepx;	/* fixed integral dx */
    int	    signdx;	/* variable dx sign */
    int	    e;		/* initial error term */
    int	    dy;
    int	    dx;
} PolyEdgeRec, *PolyEdgePtr;

#define SQSECANT 108.856472512142 /* 1/sin^2(11/2) - miter limit constant */

/*
 * types for general polygon routines
 */

typedef struct _PolyVertex {
    double  x, y;
} PolyVertexRec, *PolyVertexPtr;

typedef struct _PolySlope {
    int	    dx, dy;
    double  k;	    /* x0 * dy - y0 * dx */
} PolySlopeRec, *PolySlopePtr;

/*
 * Line face description for caps/joins
 */

typedef struct _LineFace {
    double  xa, ya;
    int	    dx, dy;
    int	    x, y;
    double  k;
} LineFaceRec, *LineFacePtr;

/*
 * macros for polygon fillers
 */

#define MIPOLYRELOADLEFT    if (!left_height && left_count) { \
	    	    	    	left_height = left->height; \
	    	    	    	left_x = left->x; \
	    	    	    	left_stepx = left->stepx; \
	    	    	    	left_signdx = left->signdx; \
	    	    	    	left_e = left->e; \
	    	    	    	left_dy = left->dy; \
	    	    	    	left_dx = left->dx; \
	    	    	    	--left_count; \
	    	    	    	++left; \
			    }

#define MIPOLYRELOADRIGHT   if (!right_height && right_count) { \
	    	    	    	right_height = right->height; \
	    	    	    	right_x = right->x; \
	    	    	    	right_stepx = right->stepx; \
	    	    	    	right_signdx = right->signdx; \
	    	    	    	right_e = right->e; \
	    	    	    	right_dy = right->dy; \
	    	    	    	right_dx = right->dx; \
	    	    	    	--right_count; \
	    	    	    	++right; \
			}

#define MIPOLYSTEPLEFT  left_x += left_stepx; \
    	    	    	left_e += left_dx; \
    	    	    	if (left_e > 0) \
    	    	    	{ \
	    	    	    left_x += left_signdx; \
	    	    	    left_e -= left_dy; \
    	    	    	}

#define MIPOLYSTEPRIGHT right_x += right_stepx; \
    	    	    	right_e += right_dx; \
    	    	    	if (right_e > 0) \
    	    	    	{ \
	    	    	    right_x += right_signdx; \
	    	    	    right_e -= right_dy; \
    	    	    	}

#define MILINESETPIXEL(pDrawable, pGC, pixel, oldPixel) { \
    oldPixel = pGC->fgPixel; \
    if (pixel != oldPixel) { \
	DoChangeGC (pGC, GCForeground, (XID *) &pixel, FALSE); \
	ValidateGC (pDrawable, pGC); \
    } \
}
#define MILINERESETPIXEL(pDrawable, pGC, pixel, oldPixel) { \
    if (pixel != oldPixel) { \
	DoChangeGC (pGC, GCForeground, (XID *) &oldPixel, FALSE); \
	ValidateGC (pDrawable, pGC); \
    } \
}

#ifdef NOINLINEICEIL
#define ICEIL(x) ((int)ceil(x))
#else
#ifdef __GNUC__
static __inline int ICEIL(x)
    double x;
{
    int _cTmp = x;
    return ((x == _cTmp) || (x < 0.0)) ? _cTmp : _cTmp+1;
}
#else
#define ICEIL(x) ((((x) == (_cTmp = (x))) || ((x) < 0.0)) ? _cTmp : _cTmp+1)
#define ICEILTEMPDECL static int _cTmp;
#endif
#endif

extern void miFillPolyHelper(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    unsigned long /*pixel*/,
    SpanDataPtr /*spanData*/,
    int /*y*/,
    int /*overall_height*/,
    PolyEdgePtr /*left*/,
    PolyEdgePtr /*right*/,
    int /*left_count*/,
    int /*right_count*/
#endif
);
extern int miRoundJoinFace(
#if NeedFunctionPrototypes
    LineFacePtr /*face*/,
    PolyEdgePtr /*edge*/,
    Bool * /*leftEdge*/
#endif
);

extern void miRoundJoinClip(
#if NeedFunctionPrototypes
    LineFacePtr /*pLeft*/,
    LineFacePtr /*pRight*/,
    PolyEdgePtr /*edge1*/,
    PolyEdgePtr /*edge2*/,
    int * /*y1*/,
    int * /*y2*/,
    Bool * /*left1*/,
    Bool * /*left2*/
#endif
);

extern int miRoundCapClip(
#if NeedFunctionPrototypes
    LineFacePtr /*face*/,
    Bool /*isInt*/,
    PolyEdgePtr /*edge*/,
    Bool * /*leftEdge*/
#endif
);

extern void miLineProjectingCap(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    unsigned long /*pixel*/,
    SpanDataPtr /*spanData*/,
    LineFacePtr /*face*/,
    Bool /*isLeft*/,
    double /*xorg*/,
    double /*yorg*/,
    Bool /*isInt*/
#endif
);

extern SpanDataPtr miSetupSpanData(
#if NeedFunctionPrototypes
    GCPtr /*pGC*/,
    SpanDataPtr /*spanData*/,
    int /*npt*/
#endif
);

extern void miCleanupSpanData(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    SpanDataPtr /*spanData*/
#endif
);
