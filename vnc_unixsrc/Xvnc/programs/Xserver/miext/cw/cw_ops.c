/*
 * Copyright © 2004 Eric Anholt
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Eric Anholt not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Eric Anholt makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * ERIC ANHOLT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ERIC ANHOLT BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
/* $Header: /cvs/xorg/xc/programs/Xserver/miext/cw/cw_ops.c,v 1.9 2005/07/03 07:02:01 daniels Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "gcstruct.h"
#include "cw.h"

#define SETUP_BACKING_DST(_pDst, _pGC) \
    cwGCPtr pGCPrivate = getCwGC (_pGC); \
    int dst_off_x, dst_off_y; \
    DrawablePtr pBackingDst = cwGetBackingDrawable(pDst, &dst_off_x, \
	&dst_off_y); \
    GCPtr pBackingGC = pGCPrivate->pBackingGC ? pGCPrivate->pBackingGC : _pGC

#define SETUP_BACKING_SRC(pSrc, pGC) \
    int src_off_x, src_off_y; \
    DrawablePtr pBackingSrc = cwGetBackingDrawable(pSrc, &src_off_x, \
	&src_off_y)

#define PROLOGUE(pGC) do { \
    pGC->funcs = pGCPrivate->wrapFuncs;\
    pGC->ops = pGCPrivate->wrapOps;\
} while (0)

#define EPILOGUE(pGC) do { \
    pGCPrivate->wrapFuncs = (pGC)->funcs; \
    pGCPrivate->wrapOps = (pGC)->ops; \
    (pGC)->funcs = &cwGCFuncs; \
    (pGC)->ops = &cwGCOps; \
} while (0)

extern GCFuncs cwGCFuncs;

/*
 * GC ops -- wrap each GC operation with our own function
 */

static void cwFillSpans(DrawablePtr pDst, GCPtr pGC, int nInit,
			DDXPointPtr pptInit, int *pwidthInit, int fSorted);
static void cwSetSpans(DrawablePtr pDst, GCPtr pGC, char *psrc,
		       DDXPointPtr ppt, int *pwidth, int nspans, int fSorted);
static void cwPutImage(DrawablePtr pDst, GCPtr pGC, int depth,
		       int x, int y, int w, int h, int leftPad, int format,
		       char *pBits);
static RegionPtr cwCopyArea(DrawablePtr pSrc, DrawablePtr pDst, GCPtr pGC,
			    int srcx, int srcy, int w, int h,
			    int dstx, int dsty);
static RegionPtr cwCopyPlane(DrawablePtr pSrc, DrawablePtr pDst, GCPtr pGC,
			     int srcx, int srcy, int w, int h,
			     int dstx, int dsty, unsigned long plane);
static void cwPolyPoint(DrawablePtr pDst, GCPtr pGC, int mode, int npt,
			xPoint *pptInit);
static void cwPolylines(DrawablePtr pDst, GCPtr pGC, int mode, int npt,
			DDXPointPtr pptInit);
static void cwPolySegment(DrawablePtr pDst, GCPtr pGC, int nseg,
			  xSegment *pSegs);
static void cwPolyRectangle(DrawablePtr pDst, GCPtr pGC,
			    int nrects, xRectangle *pRects);
static void cwPolyArc(DrawablePtr pDst, GCPtr pGC, int narcs, xArc *parcs);
static void cwFillPolygon(DrawablePtr pDst, GCPtr pGC, int shape, int mode,
			  int count, DDXPointPtr pPts);
static void cwPolyFillRect(DrawablePtr pDst, GCPtr pGC,
			   int nrectFill, xRectangle *prectInit);
static void cwPolyFillArc(DrawablePtr pDst, GCPtr pGC,
			  int narcs, xArc *parcs);
static int cwPolyText8(DrawablePtr pDrawable, GCPtr pGC, int x, int y,
		       int count, char *chars);
static int cwPolyText16(DrawablePtr pDst, GCPtr pGC, int x, int y,
			int count, unsigned short *chars);
static void cwImageText8(DrawablePtr pDst, GCPtr pGC, int x, int y,
			 int count, char *chars);
static void cwImageText16(DrawablePtr pDst, GCPtr pGC, int x, int y,
			  int count, unsigned short *chars);
static void cwImageGlyphBlt(DrawablePtr pDst, GCPtr pGC, int x, int y,
			    unsigned int nglyph, CharInfoPtr *ppci,
			    pointer pglyphBase);
static void cwPolyGlyphBlt(DrawablePtr pDst, GCPtr pGC, int x, int y,
			   unsigned int nglyph, CharInfoPtr *ppci,
			   pointer pglyphBase);
static void cwPushPixels(GCPtr pGC, PixmapPtr pBitMap, DrawablePtr pDst,
			 int w, int h, int x, int y);

GCOps cwGCOps = {
	cwFillSpans,
	cwSetSpans,
	cwPutImage,
	cwCopyArea,
	cwCopyPlane,
	cwPolyPoint,
	cwPolylines,
	cwPolySegment,
	cwPolyRectangle,
	cwPolyArc,
	cwFillPolygon,
	cwPolyFillRect,
	cwPolyFillArc,
	cwPolyText8,
	cwPolyText16,
	cwImageText8,
	cwImageText16,
	cwImageGlyphBlt,
	cwPolyGlyphBlt,
	cwPushPixels
};

static void
cwFillSpans(DrawablePtr pDst, GCPtr pGC, int nspans, DDXPointPtr ppt,
	    int *pwidth, int fSorted)
{
    SETUP_BACKING_DST(pDst, pGC);

    PROLOGUE(pGC);

    CW_OFFSET_XYPOINTS(ppt, nspans);

    (*pBackingGC->ops->FillSpans)(pBackingDst, pBackingGC, nspans, ppt,
				  pwidth, fSorted);

    EPILOGUE(pGC);
}

static void
cwSetSpans(DrawablePtr pDst, GCPtr pGC, char *psrc, DDXPointPtr ppt,
	   int *pwidth, int nspans, int fSorted)
{
    SETUP_BACKING_DST(pDst, pGC);

    PROLOGUE(pGC);

    CW_OFFSET_XYPOINTS(ppt, nspans);

    (*pBackingGC->ops->SetSpans)(pBackingDst, pBackingGC, psrc, ppt, pwidth,
				 nspans, fSorted);

    EPILOGUE(pGC);
}

static void
cwPutImage(DrawablePtr pDst, GCPtr pGC, int depth, int x, int y, int w, int h,
	   int leftPad, int format, char *pBits)
{
    SETUP_BACKING_DST(pDst, pGC);

    PROLOGUE(pGC);

    CW_OFFSET_XY_DST(x, y);

    (*pBackingGC->ops->PutImage)(pBackingDst, pBackingGC, depth, x, y, w, h,
				 leftPad, format, pBits);

    EPILOGUE(pGC);
}

static RegionPtr
cwCopyArea(DrawablePtr pSrc, DrawablePtr pDst, GCPtr pGC, int srcx, int srcy,
	   int w, int h, int dstx, int dsty)
{
    int		odstx, odsty;
    RegionPtr	exposed = NULL;
    SETUP_BACKING_DST(pDst, pGC);
    SETUP_BACKING_SRC(pSrc, pGC);

    PROLOGUE(pGC);

    odstx = dstx;
    odsty = dsty;
    CW_OFFSET_XY_DST(dstx, dsty);
    CW_OFFSET_XY_SRC(srcx, srcy);

    exposed = (*pBackingGC->ops->CopyArea)(pBackingSrc, pBackingDst,
					   pBackingGC, srcx, srcy, w, h,
					   dstx, dsty);

    if (exposed != NULL)
	REGION_TRANSLATE(pDst->pScreen, exposed, odstx - dstx, odsty - dsty);

    EPILOGUE(pGC);

    return exposed;
}

static RegionPtr
cwCopyPlane(DrawablePtr pSrc, DrawablePtr pDst, GCPtr pGC, int srcx, int srcy,
	    int w, int h, int dstx, int dsty, unsigned long plane)
{
    int		odstx, odsty;
    RegionPtr	exposed = NULL;
    SETUP_BACKING_DST(pDst, pGC);
    SETUP_BACKING_SRC(pSrc, pGC);

    PROLOGUE(pGC);

    odstx = dstx;
    odsty = dsty;
    CW_OFFSET_XY_DST(dstx, dsty);
    CW_OFFSET_XY_SRC(srcx, srcy);

    exposed = (*pBackingGC->ops->CopyPlane)(pBackingSrc, pBackingDst,
					    pBackingGC, srcx, srcy, w, h,
					    dstx, dsty, plane);

    if (exposed != NULL)
	REGION_TRANSLATE(pDst->pScreen, exposed, odstx - dstx, odsty - dsty);

    EPILOGUE(pGC);

    return exposed;
}

static void
cwPolyPoint(DrawablePtr pDst, GCPtr pGC, int mode, int npt, xPoint *ppt)
{
    SETUP_BACKING_DST(pDst, pGC);

    PROLOGUE(pGC);

    if (mode == CoordModeOrigin)
	CW_OFFSET_XYPOINTS(ppt, npt);
    else
	CW_OFFSET_XYPOINTS(ppt, 1);

    (*pBackingGC->ops->PolyPoint)(pBackingDst, pBackingGC, mode, npt, ppt);

    EPILOGUE(pGC);
}

static void
cwPolylines(DrawablePtr pDst, GCPtr pGC, int mode, int npt, DDXPointPtr ppt)
{
    SETUP_BACKING_DST(pDst, pGC);

    PROLOGUE(pGC);

    if (mode == CoordModeOrigin)
	CW_OFFSET_XYPOINTS(ppt, npt);
    else
	CW_OFFSET_XYPOINTS(ppt, 1);

    (*pBackingGC->ops->Polylines)(pBackingDst, pBackingGC, mode, npt, ppt);

    EPILOGUE(pGC);
}

static void
cwPolySegment(DrawablePtr pDst, GCPtr pGC, int nseg, xSegment *pSegs)
{
    SETUP_BACKING_DST(pDst, pGC);

    PROLOGUE(pGC);

    CW_OFFSET_XYPOINTS(pSegs, nseg * 2);

    (*pBackingGC->ops->PolySegment)(pBackingDst, pBackingGC, nseg, pSegs);

    EPILOGUE(pGC);
}

static void
cwPolyRectangle(DrawablePtr pDst, GCPtr pGC, int nrects, xRectangle *pRects)
{
    SETUP_BACKING_DST(pDst, pGC);

    PROLOGUE(pGC);

    CW_OFFSET_RECTS(pRects, nrects);

    (*pBackingGC->ops->PolyRectangle)(pBackingDst, pBackingGC, nrects, pRects);

    EPILOGUE(pGC);
}

static void
cwPolyArc(DrawablePtr pDst, GCPtr pGC, int narcs, xArc *pArcs)
{
    SETUP_BACKING_DST(pDst, pGC);

    PROLOGUE(pGC);

    CW_OFFSET_RECTS(pArcs, narcs);

    (*pBackingGC->ops->PolyArc)(pBackingDst, pBackingGC, narcs, pArcs);

    EPILOGUE(pGC);
}

static void
cwFillPolygon(DrawablePtr pDst, GCPtr pGC, int shape, int mode, int npt,
	      DDXPointPtr ppt)
{
    SETUP_BACKING_DST(pDst, pGC);

    PROLOGUE(pGC);

    if (mode == CoordModeOrigin)
	CW_OFFSET_XYPOINTS(ppt, npt);
    else
	CW_OFFSET_XYPOINTS(ppt, 1);

    (*pBackingGC->ops->FillPolygon)(pBackingDst, pBackingGC, shape, mode, npt,
				    ppt);

    EPILOGUE(pGC);
}

static void
cwPolyFillRect(DrawablePtr pDst, GCPtr pGC, int nrects, xRectangle *pRects)
{
    SETUP_BACKING_DST(pDst, pGC);

    PROLOGUE(pGC);

    CW_OFFSET_RECTS(pRects, nrects);

    (*pBackingGC->ops->PolyFillRect)(pBackingDst, pBackingGC, nrects, pRects);

    EPILOGUE(pGC);
}

static void
cwPolyFillArc(DrawablePtr pDst, GCPtr pGC, int narcs, xArc *parcs)
{
    SETUP_BACKING_DST(pDst, pGC);

    PROLOGUE(pGC);

    CW_OFFSET_RECTS(parcs, narcs);

    (*pBackingGC->ops->PolyFillArc)(pBackingDst, pBackingGC, narcs, parcs);

    EPILOGUE(pGC);
}

static int
cwPolyText8(DrawablePtr pDst, GCPtr pGC, int x, int y, int count, char *chars)
{
    int result;
    SETUP_BACKING_DST(pDst, pGC);

    PROLOGUE(pGC);

    CW_OFFSET_XY_DST(x, y);

    result = (*pBackingGC->ops->PolyText8)(pBackingDst, pBackingGC, x, y,
					   count, chars);

    EPILOGUE(pGC);

    return result;
}

static int
cwPolyText16(DrawablePtr pDst, GCPtr pGC, int x, int y, int count,
	     unsigned short *chars)
{
    int result;
    SETUP_BACKING_DST(pDst, pGC);

    PROLOGUE(pGC);

    CW_OFFSET_XY_DST(x, y);

    result = (*pBackingGC->ops->PolyText16)(pBackingDst, pBackingGC, x, y,
					    count, chars);

    EPILOGUE(pGC);
    return result;
}

static void
cwImageText8(DrawablePtr pDst, GCPtr pGC, int x, int y, int count, char *chars)
{
    SETUP_BACKING_DST(pDst, pGC);

    PROLOGUE(pGC);

    CW_OFFSET_XY_DST(x, y);

    (*pBackingGC->ops->ImageText8)(pBackingDst, pBackingGC, x, y, count,
				   chars);

    EPILOGUE(pGC);
}

static void
cwImageText16(DrawablePtr pDst, GCPtr pGC, int x, int y, int count,
	     unsigned short *chars)
{
    SETUP_BACKING_DST(pDst, pGC);

    PROLOGUE(pGC);

    CW_OFFSET_XY_DST(x, y);

    (*pBackingGC->ops->ImageText16)(pBackingDst, pBackingGC, x, y, count,
				    chars);

    EPILOGUE(pGC);
}

static void
cwImageGlyphBlt(DrawablePtr pDst, GCPtr pGC, int x, int y, unsigned int nglyph,
		CharInfoPtr *ppci, pointer pglyphBase)
{
    SETUP_BACKING_DST(pDst, pGC);

    PROLOGUE(pGC);

    CW_OFFSET_XY_DST(x, y);

    (*pBackingGC->ops->ImageGlyphBlt)(pBackingDst, pBackingGC, x, y, nglyph,
				      ppci, pglyphBase);

    EPILOGUE(pGC);
}

static void
cwPolyGlyphBlt(DrawablePtr pDst, GCPtr pGC, int x, int y, unsigned int nglyph,
	       CharInfoPtr *ppci, pointer pglyphBase)
{
    SETUP_BACKING_DST(pDst, pGC);

    PROLOGUE(pGC);

    CW_OFFSET_XY_DST(x, y);

    (*pBackingGC->ops->PolyGlyphBlt)(pBackingDst, pBackingGC, x, y, nglyph,
				      ppci, pglyphBase);

    EPILOGUE(pGC);
}

static void
cwPushPixels(GCPtr pGC, PixmapPtr pBitMap, DrawablePtr pDst, int w, int h,
	     int x, int y)
{
    SETUP_BACKING_DST(pDst, pGC);

    PROLOGUE(pGC);

    CW_OFFSET_XY_DST(x, y);

    (*pBackingGC->ops->PushPixels)(pBackingGC, pBitMap, pBackingDst, w, h,
				   x, y);

    EPILOGUE(pGC);
}

