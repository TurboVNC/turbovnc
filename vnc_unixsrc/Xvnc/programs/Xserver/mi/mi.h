/* $XConsortium: mi.h,v 1.17 94/04/17 20:27:10 dpw Exp $ */
/* $XFree86: xc/programs/Xserver/mi/mi.h,v 3.1 1997/01/14 22:22:51 dawes Exp $ */
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
#ifndef MI_H
#define MI_H
#include "X11/X.h"
#include "region.h"
#include "validate.h"
#include "window.h"
#include "gc.h"
#include "font.h"
#include "input.h"
#include "cursor.h"

typedef struct _miDash *miDashPtr;
#define EVEN_DASH	0
#define ODD_DASH	~0

/* miarc.c */

extern void miPolyArc(
#if NeedFunctionPrototypes
    DrawablePtr /*pDraw*/,
    GCPtr /*pGC*/,
    int /*narcs*/,
    xArc * /*parcs*/
#endif
);

/* mibitblt.c */

extern RegionPtr miCopyArea(
#if NeedFunctionPrototypes
    DrawablePtr /*pSrcDrawable*/,
    DrawablePtr /*pDstDrawable*/,
    GCPtr /*pGC*/,
    int /*xIn*/,
    int /*yIn*/,
    int /*widthSrc*/,
    int /*heightSrc*/,
    int /*xOut*/,
    int /*yOut*/
#endif
);

extern void miOpqStipDrawable(
#if NeedFunctionPrototypes
    DrawablePtr /*pDraw*/,
    GCPtr /*pGC*/,
    RegionPtr /*prgnSrc*/,
    unsigned long * /*pbits*/,
    int /*srcx*/,
    int /*w*/,
    int /*h*/,
    int /*dstx*/,
    int /*dsty*/
#endif
);

extern RegionPtr miCopyPlane(
#if NeedFunctionPrototypes
    DrawablePtr /*pSrcDrawable*/,
    DrawablePtr /*pDstDrawable*/,
    GCPtr /*pGC*/,
    int /*srcx*/,
    int /*srcy*/,
    int /*width*/,
    int /*height*/,
    int /*dstx*/,
    int /*dsty*/,
    unsigned long /*bitPlane*/
#endif
);

extern void miGetImage(
#if NeedFunctionPrototypes
    DrawablePtr /*pDraw*/,
    int /*sx*/,
    int /*sy*/,
    int /*w*/,
    int /*h*/,
    unsigned int /*format*/,
    unsigned long /*planeMask*/,
    char * /*pdstLine*/
#endif
);

extern void miPutImage(
#if NeedFunctionPrototypes
    DrawablePtr /*pDraw*/,
    GCPtr /*pGC*/,
    int /*depth*/,
    int /*x*/,
    int /*y*/,
    int /*w*/,
    int /*h*/,
    int /*leftPad*/,
    int /*format*/,
    char * /*pImage*/
#endif
);

/* miclipn.c */

extern void miClipNotify(
#if NeedFunctionPrototypes
    void (* /*func*/)(
#if NeedNestedPrototypes
	WindowPtr /* pWin */,
	int /* dx */,
	int /* dy */
#endif
	)
#endif
);

/* micursor.c */

extern void miRecolorCursor(
#if NeedFunctionPrototypes
    ScreenPtr /*pScr*/,
    CursorPtr /*pCurs*/,
    Bool /*displayed*/
#endif
);

/* midash.c */

extern miDashPtr miDashLine(
#if NeedFunctionPrototypes
    int /*npt*/,
    DDXPointPtr /*ppt*/,
    unsigned int /*nDash*/,
    unsigned char * /*pDash*/,
    unsigned int /*offset*/,
    int * /*pnseg*/
#endif
);

extern void miStepDash(
#if NeedFunctionPrototypes
    int /*dist*/,
    int * /*pDashIndex*/,
    unsigned char * /*pDash*/,
    int /*numInDashList*/,
    int * /*pDashOffset*/
#endif
);

/* mieq.c */


#ifndef INPUT_H
typedef struct _DeviceRec *DevicePtr;
#endif

extern Bool mieqInit(
#if NeedFunctionPrototypes
    DevicePtr /*pKbd*/,
    DevicePtr /*pPtr*/
#endif
);

extern void mieqEnqueue(
#if NeedFunctionPrototypes
    xEventPtr /*e*/
#endif
);

extern void mieqSwitchScreen(
#if NeedFunctionPrototypes
    ScreenPtr /*pScreen*/,
    Bool /*fromDIX*/
#endif
);

extern int mieqProcessInputEvents(
#if NeedFunctionPrototypes
    void
#endif
);

/* miexpose.c */

extern RegionPtr miHandleExposures(
#if NeedFunctionPrototypes
    DrawablePtr /*pSrcDrawable*/,
    DrawablePtr /*pDstDrawable*/,
    GCPtr /*pGC*/,
    int /*srcx*/,
    int /*srcy*/,
    int /*width*/,
    int /*height*/,
    int /*dstx*/,
    int /*dsty*/,
    unsigned long /*plane*/
#endif
);

extern void miSendGraphicsExpose(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    RegionPtr /*pRgn*/,
    XID /*drawable*/,
    int /*major*/,
    int /*minor*/
#endif
);

extern void miSendExposures(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    RegionPtr /*pRgn*/,
    int /*dx*/,
    int /*dy*/
#endif
);

extern void miWindowExposures(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    RegionPtr /*prgn*/,
    RegionPtr /*other_exposed*/
#endif
);

extern void miPaintWindow(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    RegionPtr /*prgn*/,
    int /*what*/
#endif
);

extern int miClearDrawable(
#if NeedFunctionPrototypes
    DrawablePtr /*pDraw*/,
    GCPtr /*pGC*/
#endif
);

/* mifillrct.c */

extern void miPolyFillRect(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*nrectFill*/,
    xRectangle * /*prectInit*/
#endif
);

/* miglblt.c */

extern void miPolyGlyphBlt(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*x*/,
    int /*y*/,
    unsigned int /*nglyph*/,
    CharInfoPtr * /*ppci*/,
    pointer /*pglyphBase*/
#endif
);

extern void miImageGlyphBlt(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*x*/,
    int /*y*/,
    unsigned int /*nglyph*/,
    CharInfoPtr * /*ppci*/,
    pointer /*pglyphBase*/
#endif
);

/* mipoly.c */

extern void miFillPolygon(
#if NeedFunctionPrototypes
    DrawablePtr /*dst*/,
    GCPtr /*pgc*/,
    int /*shape*/,
    int /*mode*/,
    int /*count*/,
    DDXPointPtr /*pPts*/
#endif
);

/* mipolycon.c */

extern Bool miFillConvexPoly(
#if NeedFunctionPrototypes
    DrawablePtr /*dst*/,
    GCPtr /*pgc*/,
    int /*count*/,
    DDXPointPtr /*ptsIn*/
#endif
);

/* mipolygen.c */

extern Bool miFillGeneralPoly(
#if NeedFunctionPrototypes
    DrawablePtr /*dst*/,
    GCPtr /*pgc*/,
    int /*count*/,
    DDXPointPtr /*ptsIn*/
#endif
);

/* mipolypnt.c */

extern void miPolyPoint(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*mode*/,
    int /*npt*/,
    xPoint * /*pptInit*/
#endif
);

/* mipolyrect.c */

extern void miPolyRectangle(
#if NeedFunctionPrototypes
    DrawablePtr /*pDraw*/,
    GCPtr /*pGC*/,
    int /*nrects*/,
    xRectangle * /*pRects*/
#endif
);

/* mipolyseg.c */

extern void miPolySegment(
#if NeedFunctionPrototypes
    DrawablePtr /*pDraw*/,
    GCPtr /*pGC*/,
    int /*nseg*/,
    xSegment * /*pSegs*/
#endif
);

/* mipolytext.c */

extern int miPolyText(
#if NeedFunctionPrototypes
    DrawablePtr /*pDraw*/,
    GCPtr /*pGC*/,
    int /*x*/,
    int /*y*/,
    int /*count*/,
    char * /*chars*/,
    FontEncoding /*fontEncoding*/
#endif
);

extern int miPolyText8(
#if NeedFunctionPrototypes
    DrawablePtr /*pDraw*/,
    GCPtr /*pGC*/,
    int /*x*/,
    int /*y*/,
    int /*count*/,
    char * /*chars*/
#endif
);

extern int miPolyText16(
#if NeedFunctionPrototypes
    DrawablePtr /*pDraw*/,
    GCPtr /*pGC*/,
    int /*x*/,
    int /*y*/,
    int /*count*/,
    unsigned short * /*chars*/
#endif
);

extern int miImageText(
#if NeedFunctionPrototypes
    DrawablePtr /*pDraw*/,
    GCPtr /*pGC*/,
    int /*x*/,
    int /*y*/,
    int /*count*/,
    char * /*chars*/,
    FontEncoding /*fontEncoding*/
#endif
);

extern void miImageText8(
#if NeedFunctionPrototypes
    DrawablePtr /*pDraw*/,
    GCPtr /*pGC*/,
    int /*x*/,
    int /*y*/,
    int /*count*/,
    char * /*chars*/
#endif
);

extern void miImageText16(
#if NeedFunctionPrototypes
    DrawablePtr /*pDraw*/,
    GCPtr /*pGC*/,
    int /*x*/,
    int /*y*/,
    int /*count*/,
    unsigned short * /*chars*/
#endif
);

/* mipushpxl.c */

extern void miPushPixels(
#if NeedFunctionPrototypes
    GCPtr /*pGC*/,
    PixmapPtr /*pBitMap*/,
    DrawablePtr /*pDrawable*/,
    int /*dx*/,
    int /*dy*/,
    int /*xOrg*/,
    int /*yOrg*/
#endif
);

/* miregion.c */

/* see also region.h */

extern Bool miRectAlloc(
#if NeedFunctionPrototypes
    RegionPtr /*pRgn*/,
    int /*n*/
#endif
);

extern void miSetExtents(
#if NeedFunctionPrototypes
    RegionPtr /*pReg*/
#endif
);

extern int miFindMaxBand(
#if NeedFunctionPrototypes
    RegionPtr /*prgn*/
#endif
);

#ifdef DEBUG
extern Bool miValidRegion(
#if NeedFunctionPrototypes
    RegionPtr /*prgn*/
#endif
);
#endif

/* miscrinit.c */

extern Bool miModifyPixmapHeader(
#if NeedFunctionPrototypes
    PixmapPtr /*pPixmap*/,
    int /*width*/,
    int /*height*/,
    int /*depth*/,
    int /*bitsPerPixel*/,
    int /*devKind*/,
    pointer /*pPixData*/
#endif
);

extern Bool miCloseScreen(
#if NeedFunctionPrototypes
    int /*index*/,
    ScreenPtr /*pScreen*/
#endif
);

extern Bool miCreateScreenResources(
#if NeedFunctionPrototypes
    ScreenPtr /*pScreen*/
#endif
);

extern Bool miScreenDevPrivateInit(
#if NeedFunctionPrototypes
    ScreenPtr /*pScreen*/,
    int /*width*/,
    pointer /*pbits*/
#endif
);

#ifndef _XTYPEDEF_MIBSFUNCPTR
typedef struct _miBSFuncRec *miBSFuncPtr;
#define _XTYPEDEF_MIBSFUNCPTR
#endif

extern Bool miScreenInit(
#if NeedFunctionPrototypes
    ScreenPtr /*pScreen*/,
    pointer /*pbits*/,
    int /*xsize*/,
    int /*ysize*/,
    int /*dpix*/,
    int /*dpiy*/,
    int /*width*/,
    int /*rootDepth*/,
    int /*numDepths*/,
    DepthPtr /*depths*/,
    VisualID /*rootVisual*/,
    int /*numVisuals*/,
    VisualPtr /*visuals*/,
    miBSFuncPtr /*bsfuncs*/
#endif
);

extern int miAllocateGCPrivateIndex(
#if NeedFunctionPrototypes
    void
#endif
);

/* mivaltree.c */

extern int miShapedWindowIn(
#if NeedFunctionPrototypes
    ScreenPtr /*pScreen*/,
    RegionPtr /*universe*/,
    RegionPtr /*bounding*/,
    BoxPtr /*rect*/,
    int /*x*/,
    int /*y*/
#endif
);

extern int miValidateTree(
#if NeedFunctionPrototypes
    WindowPtr /*pParent*/,
    WindowPtr /*pChild*/,
    VTKind /*kind*/
#endif
);

extern void miWideLine(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*mode*/,
    int /*npt*/,
    DDXPointPtr /*pPts*/
#endif
);

extern void miWideDash(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*mode*/,
    int /*npt*/,
    DDXPointPtr /*pPts*/
#endif
);

extern void miMiter(
#if NeedFunctionPrototypes
    void
#endif
);

extern void miNotMiter(
#if NeedFunctionPrototypes
    void
#endif
);

/* miwindow.c */

extern void miClearToBackground(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    int /*x*/,
    int /*y*/,
    int /*w*/,
    int /*h*/,
    Bool /*generateExposures*/
#endif
);

extern Bool miChangeSaveUnder(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    WindowPtr /*first*/
#endif
);

extern void miPostChangeSaveUnder(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    WindowPtr /*pFirst*/
#endif
);

extern void miMarkWindow(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/
#endif
);

extern Bool miMarkOverlappedWindows(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    WindowPtr /*pFirst*/,
    WindowPtr * /*ppLayerWin*/
#endif
);

extern void miHandleValidateExposures(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/
#endif
);

extern void miMoveWindow(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    int /*x*/,
    int /*y*/,
    WindowPtr /*pNextSib*/,
    VTKind /*kind*/
#endif
);

extern void miSlideAndSizeWindow(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    int /*x*/,
    int /*y*/,
    unsigned int /*w*/,
    unsigned int /*h*/,
    WindowPtr /*pSib*/
#endif
);

extern WindowPtr miGetLayerWindow(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/
#endif
);

extern void miSetShape(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/
#endif
);

extern void miChangeBorderWidth(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    unsigned int /*width*/
#endif
);

extern void miMarkUnrealizedWindow(
#if NeedFunctionPrototypes
    WindowPtr /*pChild*/,
    WindowPtr /*pWin*/,
    Bool /*fromConfigure*/
#endif
);

extern void miZeroPolyArc(
#if NeedFunctionPrototypes
    DrawablePtr /*pDraw*/,
    GCPtr /*pGC*/,
    int /*narcs*/,
    xArc * /*parcs*/
#endif
);

/* mizerline.c */

extern void miZeroLine(
#if NeedFunctionPrototypes
    DrawablePtr /*dst*/,
    GCPtr /*pgc*/,
    int /*mode*/,
    int /*nptInit*/,
    DDXPointRec * /*pptInit*/
#endif
);

extern void miZeroDashLine(
#if NeedFunctionPrototypes
    DrawablePtr /*dst*/,
    GCPtr /*pgc*/,
    int /*mode*/,
    int /*nptInit*/,
    DDXPointRec * /*pptInit*/
#endif
);

extern void miPolyFillArc(
#if NeedFunctionPrototypes
    DrawablePtr /*pDraw*/,
    GCPtr /*pGC*/,
    int /*narcs*/,
    xArc * /*parcs*/
#endif
);

#endif /* MI_H */
