/* $XConsortium: cfb.h,v 5.37 94/04/17 20:28:38 dpw Exp $ */
/* $XFree86: xc/programs/Xserver/cfb/cfb.h,v 3.3.2.2 1997/05/30 13:50:37 hohndel Exp $ */
/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or X Consortium
not be used in advertising or publicity pertaining to 
distribution  of  the software  without specific prior 
written permission. Sun and X Consortium make no 
representations about the suitability of this software for 
any purpose. It is provided "as is" without any express or 
implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

#include "X.h"
#include "pixmap.h"
#include "region.h"
#include "gc.h"
#include "colormap.h"
#include "miscstruct.h"
#include "servermd.h"
#include "windowstr.h"
#include "mfb.h"
#undef PixelType

#include "cfbmap.h"

/*
   private filed of pixmap
   pixmap.devPrivate = (unsigned int *)pointer_to_bits
   pixmap.devKind = width_of_pixmap_in_bytes
*/

extern int  cfbGCPrivateIndex;
extern int  cfbWindowPrivateIndex;

/* private field of GC */
typedef struct {
    unsigned char       rop;            /* special case rop values */
    /* next two values unused in cfb, included for compatibility with mfb */
    unsigned char       ropOpStip;      /* rop for opaque stipple */
    /* this value is ropFillArea in mfb, usurped for cfb */
    unsigned char       oneRect;	/*  drawable has one clip rect */
    unsigned		fExpose:1;	/* callexposure handling ? */
    unsigned		freeCompClip:1;
    PixmapPtr		pRotatedPixmap;
    RegionPtr		pCompositeClip; /* FREE_CC or REPLACE_CC */
    unsigned long	xor, and;	/* reduced rop values */
    } cfbPrivGC;

typedef cfbPrivGC	*cfbPrivGCPtr;

#define cfbGetGCPrivate(pGC)	((cfbPrivGCPtr)\
	(pGC)->devPrivates[cfbGCPrivateIndex].ptr)

#define cfbGetCompositeClip(pGC) (((cfbPrivGCPtr)\
	(pGC)->devPrivates[cfbGCPrivateIndex].ptr)->pCompositeClip)

/* way to carry RROP info around */
typedef struct {
    unsigned char	rop;
    unsigned long	xor, and;
} cfbRRopRec, *cfbRRopPtr;

/* private field of window */
typedef struct {
    unsigned	char fastBorder; /* non-zero if border is 32 bits wide */
    unsigned	char fastBackground;
    unsigned short unused; /* pad for alignment with Sun compiler */
    DDXPointRec	oldRotate;
    PixmapPtr	pRotatedBackground;
    PixmapPtr	pRotatedBorder;
    } cfbPrivWin;

#define cfbGetWindowPrivate(_pWin) ((cfbPrivWin *)\
	(_pWin)->devPrivates[cfbWindowPrivateIndex].ptr)


/* cfb8bit.c */

extern int cfbSetStipple(
#if NeedFunctionPrototypes
    int /*alu*/,
    unsigned long /*fg*/,
    unsigned long /*planemask*/
#endif
);

extern int cfbSetOpaqueStipple(
#if NeedFunctionPrototypes
    int /*alu*/,
    unsigned long /*fg*/,
    unsigned long /*bg*/,
    unsigned long /*planemask*/
#endif
);

extern int cfbComputeClipMasks32(
#if NeedFunctionPrototypes
    BoxPtr /*pBox*/,
    int /*numRects*/,
    int /*x*/,
    int /*y*/,
    int /*w*/,
    int /*h*/,
    CARD32 * /*clips*/
#endif
);
/* cfb8cppl.c */

extern void cfbCopyImagePlane(
#if NeedFunctionPrototypes
    DrawablePtr /*pSrcDrawable*/,
    DrawablePtr /*pDstDrawable*/,
    int /*rop*/,
    RegionPtr /*prgnDst*/,
    DDXPointPtr /*pptSrc*/,
    unsigned long /*planemask*/
#endif
);

extern void cfbCopyPlane8to1(
#if NeedFunctionPrototypes
    DrawablePtr /*pSrcDrawable*/,
    DrawablePtr /*pDstDrawable*/,
    int /*rop*/,
    RegionPtr /*prgnDst*/,
    DDXPointPtr /*pptSrc*/,
    unsigned long /*planemask*/,
    unsigned long /*bitPlane*/
#endif
);
/* cfb8lineCO.c */

extern int cfb8LineSS1RectCopy(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*mode*/,
    int /*npt*/,
    DDXPointPtr /*pptInit*/,
    DDXPointPtr /*pptInitOrig*/,
    int * /*x1p*/,
    int * /*y1p*/,
    int * /*x2p*/,
    int * /*y2p*/
#endif
);

extern void cfb8LineSS1Rect(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*mode*/,
    int /*npt*/,
    DDXPointPtr /*pptInit*/
#endif
);

extern void cfb8ClippedLineCopy(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*x1*/,
    int /*y1*/,
    int /*x2*/,
    int /*y2*/,
    BoxPtr /*boxp*/,
    Bool /*shorten*/
#endif
);
/* cfb8lineCP.c */

extern int cfb8LineSS1RectPreviousCopy(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*mode*/,
    int /*npt*/,
    DDXPointPtr /*pptInit*/,
    DDXPointPtr /*pptInitOrig*/,
    int * /*x1p*/,
    int * /*y1p*/,
    int * /*x2p*/,
    int * /*y2p*/

#endif
);
/* cfb8lineG.c */

extern int cfb8LineSS1RectGeneral(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*mode*/,
    int /*npt*/,
    DDXPointPtr /*pptInit*/,
    DDXPointPtr /*pptInitOrig*/,
    int * /*x1p*/,
    int * /*y1p*/,
    int * /*x2p*/,
    int * /*y2p*/
#endif
);

extern void cfb8ClippedLineGeneral(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*x1*/,
    int /*y1*/,
    int /*x2*/,
    int /*y2*/,
    BoxPtr /*boxp*/,
    Bool /*shorten*/
#endif
);
/* cfb8lineX.c */

extern int cfb8LineSS1RectXor(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*mode*/,
    int /*npt*/,
    DDXPointPtr /*pptInit*/,
    DDXPointPtr /*pptInitOrig*/,
    int * /*x1p*/,
    int * /*y1p*/,
    int * /*x2p*/,
    int * /*y2p*/
#endif
);

extern void cfb8ClippedLineXor(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*x1*/,
    int /*y1*/,
    int /*x2*/,
    int /*y2*/,
    BoxPtr /*boxp*/,
    Bool /*shorten*/
#endif
);
/* cfb8segC.c */

extern int cfb8SegmentSS1RectCopy(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*nseg*/,
    xSegment * /*pSegInit*/
#endif
);
/* cfb8segCS.c */

extern int cfb8SegmentSS1RectShiftCopy(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*nseg*/,
    xSegment * /*pSegInit*/
#endif
);

extern void cfb8SegmentSS1Rect(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*nseg*/,
    xSegment * /*pSegInit*/
#endif
);
/* cfb8segG.c */

extern int cfb8SegmentSS1RectGeneral(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*nseg*/,
    xSegment * /*pSegInit*/
#endif
);
/* cfbsegX.c */

extern int cfb8SegmentSS1RectXor(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*nseg*/,
    xSegment * /*pSegInit*/
#endif
);
/* cfballpriv.c */

extern Bool cfbAllocatePrivates(
#if NeedFunctionPrototypes
    ScreenPtr /*pScreen*/,
    int * /*window_index*/,
    int * /*gc_index*/
#endif
);
/* cfbbitblt.c */

extern RegionPtr cfbBitBlt(
#if NeedFunctionPrototypes
    DrawablePtr /*pSrcDrawable*/,
    DrawablePtr /*pDstDrawable*/,
    GCPtr/*pGC*/,
    int /*srcx*/,
    int /*srcy*/,
    int /*width*/,
    int /*height*/,
    int /*dstx*/,
    int /*dsty*/,
    void (* /*doBitBlt*/)(
#if NeedNestedPrototypes
	DrawablePtr /*pSrc*/,
	DrawablePtr /*pDst*/,
	int /*alu*/,
	RegionPtr /*prgnDst*/,
	DDXPointPtr /*pptSrc*/,
	unsigned long /*planemask*/,
	unsigned long /*bitPlane*/
#endif
	),
    unsigned long /*bitPlane*/
#endif
);

extern void cfbDoBitblt(
#if NeedFunctionPrototypes
    DrawablePtr /*pSrc*/,
    DrawablePtr /*pDst*/,
    int /*alu*/,
    RegionPtr /*prgnDst*/,
    DDXPointPtr /*pptSrc*/,
    unsigned long /*planemask*/
#endif
);

extern RegionPtr cfbCopyArea(
#if NeedFunctionPrototypes
    DrawablePtr /*pSrcDrawable*/,
    DrawablePtr /*pDstDrawable*/,
    GCPtr/*pGC*/,
    int /*srcx*/,
    int /*srcy*/,
    int /*width*/,
    int /*height*/,
    int /*dstx*/,
    int /*dsty*/
#endif
);

extern void cfbCopyPlane1to8(
#if NeedFunctionPrototypes
    DrawablePtr /*pSrcDrawable*/,
    DrawablePtr /*pDstDrawable*/,
    int /*rop*/,
    RegionPtr /*prgnDst*/,
    DDXPointPtr /*pptSrc*/,
    unsigned long /*planemask*/,
    unsigned long /*bitPlane*/
#endif
);

extern RegionPtr cfbCopyPlane(
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
/* cfbbltC.c */

extern void cfbDoBitbltCopy(
#if NeedFunctionPrototypes
    DrawablePtr /*pSrc*/,
    DrawablePtr /*pDst*/,
    int /*alu*/,
    RegionPtr /*prgnDst*/,
    DDXPointPtr /*pptSrc*/,
    unsigned long /*planemask*/
#endif
);
/* cfbbltG.c */

extern void cfbDoBitbltGeneral(
#if NeedFunctionPrototypes
    DrawablePtr /*pSrc*/,
    DrawablePtr /*pDst*/,
    int /*alu*/,
    RegionPtr /*prgnDst*/,
    DDXPointPtr /*pptSrc*/,
    unsigned long /*planemask*/
#endif
);
/* cfbbltO.c */

extern void cfbDoBitbltOr(
#if NeedFunctionPrototypes
    DrawablePtr /*pSrc*/,
    DrawablePtr /*pDst*/,
    int /*alu*/,
    RegionPtr /*prgnDst*/,
    DDXPointPtr /*pptSrc*/,
    unsigned long /*planemask*/
#endif
);
/* cfbbltX.c */

extern void cfbDoBitbltXor(
#if NeedFunctionPrototypes
    DrawablePtr /*pSrc*/,
    DrawablePtr /*pDst*/,
    int /*alu*/,
    RegionPtr /*prgnDst*/,
    DDXPointPtr /*pptSrc*/,
    unsigned long /*planemask*/
#endif
);
/* cfbbres.c */

extern void cfbBresS(
#if NeedFunctionPrototypes
    int /*rop*/,
    unsigned long /*and*/,
    unsigned long /*xor*/,
    unsigned long * /*addrl*/,
    int /*nlwidth*/,
    int /*signdx*/,
    int /*signdy*/,
    int /*axis*/,
    int /*x1*/,
    int /*y1*/,
    int /*e*/,
    int /*e1*/,
    int /*e2*/,
    int /*len*/
#endif
);
/* cfbbresd.c */

extern void cfbBresD(
#if NeedFunctionPrototypes
    cfbRRopPtr /*rrops*/,
    int * /*pdashIndex*/,
    unsigned char * /*pDash*/,
    int /*numInDashList*/,
    int * /*pdashOffset*/,
    int /*isDoubleDash*/,
    unsigned long * /*addrl*/,
    int /*nlwidth*/,
    int /*signdx*/,
    int /*signdy*/,
    int /*axis*/,
    int /*x1*/,
    int /*y1*/,
    int /*e*/,
    int /*e1*/,
    int /*e2*/,
    int /*len*/
#endif
);
/* cfbbstore.c */

extern void cfbSaveAreas(
#if NeedFunctionPrototypes
    PixmapPtr /*pPixmap*/,
    RegionPtr /*prgnSave*/,
    int /*xorg*/,
    int /*yorg*/,
    WindowPtr /*pWin*/
#endif
);

extern void cfbRestoreAreas(
#if NeedFunctionPrototypes
    PixmapPtr /*pPixmap*/,
    RegionPtr /*prgnRestore*/,
    int /*xorg*/,
    int /*yorg*/,
    WindowPtr /*pWin*/
#endif
);
/* cfbcmap.c */

extern int cfbListInstalledColormaps(
#if NeedFunctionPrototypes
    ScreenPtr	/*pScreen*/,
    Colormap	* /*pmaps*/
#endif
);

extern void cfbInstallColormap(
#if NeedFunctionPrototypes
    ColormapPtr	/*pmap*/
#endif
);

extern void cfbUninstallColormap(
#if NeedFunctionPrototypes
    ColormapPtr	/*pmap*/
#endif
);

extern void cfbResolveColor(
#if NeedFunctionPrototypes
    unsigned short * /*pred*/,
    unsigned short * /*pgreen*/,
    unsigned short * /*pblue*/,
    VisualPtr /*pVisual*/
#endif
);

extern Bool cfbInitializeColormap(
#if NeedFunctionPrototypes
    ColormapPtr /*pmap*/
#endif
);

extern int cfbExpandDirectColors(
#if NeedFunctionPrototypes
    ColormapPtr /*pmap*/,
    int /*ndef*/,
    xColorItem * /*indefs*/,
    xColorItem * /*outdefs*/
#endif
);

extern Bool cfbCreateDefColormap(
#if NeedFunctionPrototypes
    ScreenPtr /*pScreen*/
#endif
);

extern Bool cfbSetVisualTypes(
#if NeedFunctionPrototypes
    int /*depth*/,
    int /*visuals*/,
    int /*bitsPerRGB*/
#endif
);

extern Bool cfbInitVisuals(
#if NeedFunctionPrototypes
    VisualPtr * /*visualp*/,
    DepthPtr * /*depthp*/,
    int * /*nvisualp*/,
    int * /*ndepthp*/,
    int * /*rootDepthp*/,
    VisualID * /*defaultVisp*/,
    unsigned long /*sizes*/,
    int /*bitsPerRGB*/
#endif
);
/* cfbfillarcC.c */

extern void cfbPolyFillArcSolidCopy(
#if NeedFunctionPrototypes
    DrawablePtr /*pDraw*/,
    GCPtr /*pGC*/,
    int /*narcs*/,
    xArc * /*parcs*/
#endif
);
/* cfbfillarcG.c */

extern void cfbPolyFillArcSolidGeneral(
#if NeedFunctionPrototypes
    DrawablePtr /*pDraw*/,
    GCPtr /*pGC*/,
    int /*narcs*/,
    xArc * /*parcs*/
#endif
);
/* cfbfillrct.c */

extern void cfbFillBoxTileOdd(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    int /*n*/,
    BoxPtr /*rects*/,
    PixmapPtr /*tile*/,
    int /*xrot*/,
    int /*yrot*/
#endif
);

extern void cfbFillRectTileOdd(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*nBox*/,
    BoxPtr /*pBox*/
#endif
);

extern void cfbPolyFillRect(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*nrectFill*/,
    xRectangle * /*prectInit*/
#endif
);
/* cfbfillsp.c */

extern void cfbUnnaturalTileFS(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr/*pGC*/,
    int /*nInit*/,
    DDXPointPtr /*pptInit*/,
    int * /*pwidthInit*/,
    int /*fSorted*/
#endif
);

extern void cfbUnnaturalStippleFS(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr/*pGC*/,
    int /*nInit*/,
    DDXPointPtr /*pptInit*/,
    int * /*pwidthInit*/,
    int /*fSorted*/
#endif
);

extern void cfb8Stipple32FS(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*nInit*/,
    DDXPointPtr /*pptInit*/,
    int * /*pwidthInit*/,
    int /*fSorted*/
#endif
);

extern void cfb8OpaqueStipple32FS(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*nInit*/,
    DDXPointPtr /*pptInit*/,
    int * /*pwidthInit*/,
    int /*fSorted*/
#endif
);
/* cfbgc.c */

extern GCOpsPtr cfbMatchCommon(
#if NeedFunctionPrototypes
    GCPtr /*pGC*/,
    cfbPrivGCPtr /*devPriv*/
#endif
);

extern Bool cfbCreateGC(
#if NeedFunctionPrototypes
    GCPtr /*pGC*/
#endif
);

extern void cfbValidateGC(
#if NeedFunctionPrototypes
    GCPtr /*pGC*/,
    unsigned long /*changes*/,
    DrawablePtr /*pDrawable*/
#endif
);

/* cfbgetsp.c */

extern void cfbGetSpans(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    int /*wMax*/,
    DDXPointPtr /*ppt*/,
    int * /*pwidth*/,
    int /*nspans*/,
    char * /*pdstStart*/
#endif
);
/* cfbglblt8.c */

extern void cfbPolyGlyphBlt8(
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
/* cfbglrop8.c */

extern void cfbPolyGlyphRop8(
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
/* cfbhrzvert.c */

extern int cfbHorzS(
#if NeedFunctionPrototypes
    int /*rop*/,
    unsigned long /*and*/,
    unsigned long /*xor*/,
    unsigned long * /*addrl*/,
    int /*nlwidth*/,
    int /*x1*/,
    int /*y1*/,
    int /*len*/
#endif
);

extern void cfbVertS(
#if NeedFunctionPrototypes
    int /*rop*/,
    unsigned long /*and*/,
    unsigned long /*xor*/,
    unsigned long * /*addrl*/,
    int /*nlwidth*/,
    int /*x1*/,
    int /*y1*/,
    int /*len*/
#endif
);
/* cfbigblt8.c */

extern void cfbImageGlyphBlt8(
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
/* cfbimage.c */

extern void cfbPutImage(
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

extern void cfbGetImage(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    int /*sx*/,
    int /*sy*/,
    int /*w*/,
    int /*h*/,
    unsigned int /*format*/,
    unsigned long /*planeMask*/,
    char * /*pdstLine*/
#endif
);
/* cfbline.c */

extern void cfbLineSS(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*mode*/,
    int /*npt*/,
    DDXPointPtr /*pptInit*/
#endif
);

extern void cfbLineSD(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*mode*/,
    int /*npt*/,
    DDXPointPtr /*pptInit*/
#endif
);
/* cfbmskbits.c */
/* cfbpixmap.c */

extern PixmapPtr cfbCreatePixmap(
#if NeedFunctionPrototypes
    ScreenPtr /*pScreen*/,
    int /*width*/,
    int /*height*/,
    int /*depth*/
#endif
);

extern Bool cfbDestroyPixmap(
#if NeedFunctionPrototypes
    PixmapPtr /*pPixmap*/
#endif
);

extern PixmapPtr cfbCopyPixmap(
#if NeedFunctionPrototypes
    PixmapPtr /*pSrc*/
#endif
);

extern void cfbPadPixmap(
#if NeedFunctionPrototypes
    PixmapPtr /*pPixmap*/
#endif
);

extern void cfbXRotatePixmap(
#if NeedFunctionPrototypes
    PixmapPtr /*pPix*/,
    int /*rw*/
#endif
);

extern void cfbYRotatePixmap(
#if NeedFunctionPrototypes
    PixmapPtr /*pPix*/,
    int /*rh*/
#endif
);

extern void cfbCopyRotatePixmap(
#if NeedFunctionPrototypes
    PixmapPtr /*psrcPix*/,
    PixmapPtr * /*ppdstPix*/,
    int /*xrot*/,
    int /*yrot*/
#endif
);
/* cfbply1rctC.c */

extern void cfbFillPoly1RectCopy(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*shape*/,
    int /*mode*/,
    int /*count*/,
    DDXPointPtr /*ptsIn*/
#endif
);
/* cfbply1rctG.c */

extern void cfbFillPoly1RectGeneral(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*shape*/,
    int /*mode*/,
    int /*count*/,
    DDXPointPtr /*ptsIn*/
#endif
);
/* cfbpntwin.c */

extern void cfbPaintWindow(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    RegionPtr /*pRegion*/,
    int /*what*/
#endif
);

extern void cfbFillBoxSolid(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    int /*nBox*/,
    BoxPtr /*pBox*/,
    unsigned long /*pixel*/
#endif
);

extern void cfbFillBoxTile32(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    int /*nBox*/,
    BoxPtr /*pBox*/,
    PixmapPtr /*tile*/
#endif
);
/* cfbpolypnt.c */

extern void cfbPolyPoint(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*mode*/,
    int /*npt*/,
    xPoint * /*pptInit*/
#endif
);
/* cfbpush8.c */

extern void cfbPushPixels8(
#if NeedFunctionPrototypes
    GCPtr /*pGC*/,
    PixmapPtr /*pBitmap*/,
    DrawablePtr /*pDrawable*/,
    int /*dx*/,
    int /*dy*/,
    int /*xOrg*/,
    int /*yOrg*/
#endif
);
/* cfbrctstp8.c */

extern void cfb8FillRectOpaqueStippled32(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*nBox*/,
    BoxPtr /*pBox*/
#endif
);

extern void cfb8FillRectTransparentStippled32(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*nBox*/,
    BoxPtr /*pBox*/
#endif
);

extern void cfb8FillRectStippledUnnatural(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*nBox*/,
    BoxPtr /*pBox*/
#endif
);
/* cfbrrop.c */

extern int cfbReduceRasterOp(
#if NeedFunctionPrototypes
    int /*rop*/,
    unsigned long /*fg*/,
    unsigned long /*pm*/,
    unsigned long * /*andp*/,
    unsigned long * /*xorp*/
#endif
);
/* cfbscrinit.c */

extern Bool cfbCloseScreen(
#if NeedFunctionPrototypes
    int /*index*/,
    ScreenPtr /*pScreen*/
#endif
);

extern Bool cfbSetupScreen(
#if NeedFunctionPrototypes
    ScreenPtr /*pScreen*/,
    pointer /*pbits*/,
    int /*xsize*/,
    int /*ysize*/,
    int /*dpix*/,
    int /*dpiy*/,
    int /*width*/
#endif
);

extern int cfbFinishScreenInit(
#if NeedFunctionPrototypes
    ScreenPtr /*pScreen*/,
    pointer /*pbits*/,
    int /*xsize*/,
    int /*ysize*/,
    int /*dpix*/,
    int /*dpiy*/,
    int /*width*/
#endif
);

extern Bool cfbScreenInit(
#if NeedFunctionPrototypes
    ScreenPtr /*pScreen*/,
    pointer /*pbits*/,
    int /*xsize*/,
    int /*ysize*/,
    int /*dpix*/,
    int /*dpiy*/,
    int /*width*/
#endif
);
/* cfbseg.c */

extern void cfbSegmentSS(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*nseg*/,
    xSegment * /*pSeg*/
#endif
);

extern void cfbSegmentSD(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*nseg*/,
    xSegment * /*pSeg*/
#endif
);
/* cfbsetsp.c */

extern int cfbSetScanline(
#if NeedFunctionPrototypes
    int /*y*/,
    int /*xOrigin*/,
    int /*xStart*/,
    int /*xEnd*/,
    unsigned int * /*psrc*/,
    int /*alu*/,
    int * /*pdstBase*/,
    int /*widthDst*/,
    unsigned long /*planemask*/
#endif
);

extern void cfbSetSpans(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    char * /*psrc*/,
    DDXPointPtr /*ppt*/,
    int * /*pwidth*/,
    int /*nspans*/,
    int /*fSorted*/
#endif
);
/* cfbsolidC.c */

extern void cfbFillRectSolidCopy(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*nBox*/,
    BoxPtr /*pBox*/
#endif
);

extern void cfbSolidSpansCopy(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*nInit*/,
    DDXPointPtr /*pptInit*/,
    int * /*pwidthInit*/,
    int /*fSorted*/
#endif
);
/* cfbsolidG.c */

extern void cfbFillRectSolidGeneral(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*nBox*/,
    BoxPtr /*pBox*/
#endif
);

extern void cfbSolidSpansGeneral(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*nInit*/,
    DDXPointPtr /*pptInit*/,
    int * /*pwidthInit*/,
    int /*fSorted*/
#endif
);
/* cfbsolidX.c */

extern void cfbFillRectSolidXor(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*nBox*/,
    BoxPtr /*pBox*/
#endif
);

extern void cfbSolidSpansXor(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*nInit*/,
    DDXPointPtr /*pptInit*/,
    int * /*pwidthInit*/,
    int /*fSorted*/
#endif
);
/* cfbteblt8.c */

extern void cfbTEGlyphBlt8(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr/*pGC*/,
    int /*xInit*/,
    int /*yInit*/,
    unsigned int /*nglyph*/,
    CharInfoPtr * /*ppci*/,
    pointer /*pglyphBase*/
#endif
);
/* cfbtegblt.c */

extern void cfbTEGlyphBlt(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr/*pGC*/,
    int /*x*/,
    int /*y*/,
    unsigned int /*nglyph*/,
    CharInfoPtr * /*ppci*/,
    pointer /*pglyphBase*/
#endif
);
/* cfbtile32C.c */

extern void cfbFillRectTile32Copy(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*nBox*/,
    BoxPtr /*pBox*/
#endif
);

extern void cfbTile32FSCopy(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*nInit*/,
    DDXPointPtr /*pptInit*/,
    int * /*pwidthInit*/,
    int /*fSorted*/
#endif
);
/* cfbtile32G.c */

extern void cfbFillRectTile32General(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*nBox*/,
    BoxPtr /*pBox*/
#endif
);

extern void cfbTile32FSGeneral(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    GCPtr /*pGC*/,
    int /*nInit*/,
    DDXPointPtr /*pptInit*/,
    int * /*pwidthInit*/,
    int /*fSorted*/
#endif
);
/* cfbtileoddC.c */

extern void cfbFillBoxTileOddCopy(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    int /*nBox*/,
    BoxPtr /*pBox*/,
    PixmapPtr /*tile*/,
    int /*xrot*/,
    int /*yrot*/,
    int /*alu*/,
    unsigned long /*planemask*/
#endif
);

extern void cfbFillSpanTileOddCopy(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    int /*n*/,
    DDXPointPtr /*ppt*/,
    int * /*pwidth*/,
    PixmapPtr /*tile*/,
    int /*xrot*/,
    int /*yrot*/,
    int /*alu*/,
    unsigned long /*planemask*/
#endif
);

extern void cfbFillBoxTile32sCopy(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    int /*nBox*/,
    BoxPtr /*pBox*/,
    PixmapPtr /*tile*/,
    int /*xrot*/,
    int /*yrot*/,
    int /*alu*/,
    unsigned long /*planemask*/
#endif
);

extern void cfbFillSpanTile32sCopy(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    int /*n*/,
    DDXPointPtr /*ppt*/,
    int * /*pwidth*/,
    PixmapPtr /*tile*/,
    int /*xrot*/,
    int /*yrot*/,
    int /*alu*/,
    unsigned long /*planemask*/
#endif
);
/* cfbtileoddG.c */

extern void cfbFillBoxTileOddGeneral(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    int /*nBox*/,
    BoxPtr /*pBox*/,
    PixmapPtr /*tile*/,
    int /*xrot*/,
    int /*yrot*/,
    int /*alu*/,
    unsigned long /*planemask*/
#endif
);

extern void cfbFillSpanTileOddGeneral(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    int /*n*/,
    DDXPointPtr /*ppt*/,
    int * /*pwidth*/,
    PixmapPtr /*tile*/,
    int /*xrot*/,
    int /*yrot*/,
    int /*alu*/,
    unsigned long /*planemask*/
#endif
);

extern void cfbFillBoxTile32sGeneral(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    int /*nBox*/,
    BoxPtr /*pBox*/,
    PixmapPtr /*tile*/,
    int /*xrot*/,
    int /*yrot*/,
    int /*alu*/,
    unsigned long /*planemask*/
#endif
);

extern void cfbFillSpanTile32sGeneral(
#if NeedFunctionPrototypes
    DrawablePtr /*pDrawable*/,
    int /*n*/,
    DDXPointPtr /*ppt*/,
    int * /*pwidth*/,
    PixmapPtr /*tile*/,
    int /*xrot*/,
    int /*yrot*/,
    int /*alu*/,
    unsigned long /*planemask*/
#endif
);
/* cfbwindow.c */

extern Bool cfbCreateWindow(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/
#endif
);

extern Bool cfbDestroyWindow(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/
#endif
);

extern Bool cfbMapWindow(
#if NeedFunctionPrototypes
    WindowPtr /*pWindow*/
#endif
);

extern Bool cfbPositionWindow(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    int /*x*/,
    int /*y*/
#endif
);

extern Bool cfbUnmapWindow(
#if NeedFunctionPrototypes
    WindowPtr /*pWindow*/
#endif
);

extern void cfbCopyWindow(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    DDXPointRec /*ptOldOrg*/,
    RegionPtr /*prgnSrc*/
#endif
);

extern Bool cfbChangeWindowAttributes(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    unsigned long /*mask*/
#endif
);
/* cfbzerarcC.c */

extern void cfbZeroPolyArcSS8Copy(
#if NeedFunctionPrototypes
    DrawablePtr /*pDraw*/,
    GCPtr /*pGC*/,
    int /*narcs*/,
    xArc * /*parcs*/
#endif
);
/* cfbzerarcG.c */

extern void cfbZeroPolyArcSS8General(
#if NeedFunctionPrototypes
    DrawablePtr /*pDraw*/,
    GCPtr /*pGC*/,
    int /*narcs*/,
    xArc * /*parcs*/
#endif
);
/* cfbzerarcX.c */

extern void cfbZeroPolyArcSS8Xor(
#if NeedFunctionPrototypes
    DrawablePtr /*pDraw*/,
    GCPtr /*pGC*/,
    int /*narcs*/,
    xArc * /*parcs*/
#endif
);

/*
 * This is the only completely portable way to
 * compute this info
 */

#define BitsPerPixel(d) (\
    PixmapWidthPaddingInfo[d].notPower2 ? \
    (PixmapWidthPaddingInfo[d].bytesPerPixel * 8) : \
    ((1 << PixmapWidthPaddingInfo[d].padBytesLog2) * 8 / \
    (PixmapWidthPaddingInfo[d].padRoundUp+1)))

/* Common macros for extracting drawing information */

#if !defined(SINGLEDEPTH) && PSZ != 8 || defined(FORCE_SEPARATE_PRIVATE)

#define CFB_NEED_SCREEN_PRIVATE

extern int cfbScreenPrivateIndex;
#define cfbGetScreenPixmap(s)	((PixmapPtr) (s)->devPrivates[cfbScreenPrivateIndex].ptr)
#else
#define cfbGetScreenPixmap(s)	((PixmapPtr) (s)->devPrivate)
#endif

#ifdef PIXMAP_PER_WINDOW
#define cfbGetWindowPixmap(d)	((PixmapPtr) ((WindowPtr) d)->devPrivates[frameWindowPrivateIndex].ptr)
#else
#define cfbGetWindowPixmap(d) cfbGetScreenPixmap((d)->pScreen)
#endif

#define cfbGetTypedWidth(pDrawable,wtype) (\
    (((pDrawable)->type != DRAWABLE_PIXMAP) ? \
     (int) (cfbGetWindowPixmap(pDrawable)->devKind) : \
     (int)(((PixmapPtr)pDrawable)->devKind)) / sizeof (wtype))

#define cfbGetByteWidth(pDrawable) cfbGetTypedWidth(pDrawable, unsigned char)

#define cfbGetPixelWidth(pDrawable) cfbGetTypedWidth(pDrawable, PixelType)

#define cfbGetLongWidth(pDrawable) cfbGetTypedWidth(pDrawable, unsigned long)
    
#define cfbGetTypedWidthAndPointer(pDrawable, width, pointer, wtype, ptype) {\
    PixmapPtr   _pPix; \
    if ((pDrawable)->type != DRAWABLE_PIXMAP) \
	_pPix = cfbGetWindowPixmap(pDrawable); \
    else \
	_pPix = (PixmapPtr) (pDrawable); \
    (pointer) = (ptype *) _pPix->devPrivate.ptr; \
    (width) = ((int) _pPix->devKind) / sizeof (wtype); \
}

#define cfbGetByteWidthAndPointer(pDrawable, width, pointer) \
    cfbGetTypedWidthAndPointer(pDrawable, width, pointer, unsigned char, unsigned char)

#define cfbGetLongWidthAndPointer(pDrawable, width, pointer) \
    cfbGetTypedWidthAndPointer(pDrawable, width, pointer, unsigned long, unsigned long)

#define cfbGetPixelWidthAndPointer(pDrawable, width, pointer) \
    cfbGetTypedWidthAndPointer(pDrawable, width, pointer, PixelType, PixelType)

#define cfbGetWindowTypedWidthAndPointer(pWin, width, pointer, wtype, ptype) {\
    PixmapPtr	_pPix = cfbGetWindowPixmap((DrawablePtr) (pWin)); \
    (pointer) = (ptype *) _pPix->devPrivate.ptr; \
    (width) = ((int) _pPix->devKind) / sizeof (wtype); \
}

#define cfbGetWindowLongWidthAndPointer(pWin, width, pointer) \
    cfbGetWindowTypedWidthAndPointer(pWin, width, pointer, unsigned long, unsigned long)

#define cfbGetWindowByteWidthAndPointer(pWin, width, pointer) \
    cfbGetWindowTypedWidthAndPointer(pWin, width, pointer, unsigned char, unsigned char)

#define cfbGetWindowPixelWidthAndPointer(pDrawable, width, pointer) \
    cfbGetWindowTypedWidthAndPointer(pDrawable, width, pointer, PixelType, PixelType)

/* Macros which handle a coordinate in a single register */

/* Most compilers will convert divide by 65536 into a shift, if signed
 * shifts exist.  If your machine does arithmetic shifts and your compiler
 * can't get it right, add to this line.
 */

/* mips compiler - what a joke - it CSEs the 65536 constant into a reg
 * forcing as to use div instead of shift.  Let's be explicit.
 */

#if defined(mips) || defined(sparc) || defined(__alpha) || defined(__alpha__) || defined(__i386__) || defined(i386)
#define GetHighWord(x) (((int) (x)) >> 16)
#else
#define GetHighWord(x) (((int) (x)) / 65536)
#endif

#if IMAGE_BYTE_ORDER == MSBFirst
#define intToCoord(i,x,y)   (((x) = GetHighWord(i)), ((y) = (int) ((short) (i))))
#define coordToInt(x,y)	(((x) << 16) | (y))
#define intToX(i)	(GetHighWord(i))
#define intToY(i)	((int) ((short) i))
#else
#define intToCoord(i,x,y)   (((x) = (int) ((short) (i))), ((y) = GetHighWord(i)))
#define coordToInt(x,y)	(((y) << 16) | (x))
#define intToX(i)	((int) ((short) (i)))
#define intToY(i)	(GetHighWord(i))
#endif
