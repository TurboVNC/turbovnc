/*
 * draw.c - drawing routines for the RFB X server.  This is a set of
 * wrappers around the standard MI/MFB/CFB drawing routines which work out
 * to a fair approximation the region of the screen being modified by the
 * drawing.  If the RFB client is ready then the modified region of the screen
 * is sent to the client, otherwise the modified region will simply grow with
 * each drawing request until the client is ready.
 *
 * Modified for XFree86 4.x by Alan Hourihane <alanh@fairlite.demon.co.uk>
 */

/* Copyright (C) 2010-2012, 2014, 2016-2017, 2019-2021, 2024
 *           D. R. Commander.  All Rights Reserved.
 * Copyright (C) 2021 AnatoScope SA.  All Rights Reserved.
 * Copyright (C) 2012-2013, 2016 Pierre Ossman for Cendio AB.
 *                               All Rights Reserved.
 * Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

/*

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
*/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdio.h>
#include "scrnintstr.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "regionstr.h"
#include "dixfontstr.h"
#include "rfb.h"
#include "fb.h"
#include "misc.h"

extern WindowPtr *WindowTable;  /* Why isn't this in a header file? */

int rfbDeferUpdateTime = DEFAULT_DEFER_UPDATE_TIME;  /* ms */


static inline Bool is_visible(DrawablePtr drawable)
{
  PixmapPtr scrPixmap;

  scrPixmap = drawable->pScreen->GetScreenPixmap(drawable->pScreen);

  if (drawable->type == DRAWABLE_WINDOW) {
    WindowPtr window;
    PixmapPtr winPixmap;

    window = (WindowPtr)drawable;
    winPixmap = drawable->pScreen->GetWindowPixmap(window);

    if (!window->viewable)
      return FALSE;

    if (winPixmap != scrPixmap)
      return FALSE;

    return TRUE;
  }

  if (drawable != &scrPixmap->drawable)
    return FALSE;

  return TRUE;
}


/****************************************************************************/
/*
 * Macro definitions
 */
/****************************************************************************/

#define TRC(...)  /* rfbLog(__VA_ARGS__) */
#define TRC_REGION(...)  /* PrintRegion(__VA_ARGS__, FALSE) */

#define XID(drawable)  (drawable ? drawable->id : 0)

/* ADD_TO_MODIFIED_REGION adds the given region to the modified region for each
   client */

#define ADD_TO_MODIFIED_REGION(pScreen, reg) {  \
  rfbClientPtr clTemp;  \
  BoxRec *boxTemp = REGION_EXTENTS(pScreen, reg);  \
  if ((boxTemp->x2 - boxTemp->x1) * (boxTemp->y2 - boxTemp->y1) != 0)  \
    for (clTemp = rfbClientHead; clTemp; clTemp = clTemp->next) {  \
      if (!prfb->dontSendFramebufferUpdate || pointerOwner != clTemp ||  \
          !clTemp->enableCursorShapeUpdates)  \
        REGION_UNION((pScreen), &clTemp->modifiedRegion,  \
                     &clTemp->modifiedRegion, reg);  \
    }  \
}

/* ADD_TO_ALR_REGION adds the given region to the ALR-eligible region for each
   client */

#define ADD_TO_ALR_REGION(pScreen, reg) {  \
  rfbClientPtr clTemp;  \
  BoxRec *boxTemp = REGION_EXTENTS(pScreen, reg);  \
  if ((boxTemp->x2 - boxTemp->x1) * (boxTemp->y2 - boxTemp->y1) != 0)  \
    for (clTemp = rfbClientHead; clTemp; clTemp = clTemp->next) {  \
      if (!prfb->dontSendFramebufferUpdate ||  \
          !clTemp->enableCursorShapeUpdates)  \
        REGION_UNION((pScreen), &clTemp->alrEligibleRegion,  \
                     &clTemp->alrEligibleRegion, reg);  \
    }  \
}

/* SCHEDULE_FB_UPDATE is used at the end of each drawing routine to schedule an
   update to be sent to each client if there is one pending and the client is
   ready for it.  */

#define SCHEDULE_FB_UPDATE(pScreen, prfb)  \
  if (!prfb->dontSendFramebufferUpdate && !prfb->blockUpdates) {  \
    rfbClientPtr clTemp, nextCl;  \
    for (clTemp = rfbClientHead; clTemp; clTemp = nextCl) {  \
      nextCl = clTemp->next;  \
      if (!clTemp->deferredUpdateScheduled && FB_UPDATE_PENDING(clTemp))  \
        rfbScheduleDeferredUpdate(clTemp);  \
    }  \
  }

/* function prototypes */

static void rfbScheduleDeferredUpdate(rfbClientPtr cl);
static void rfbCopyRegion(ScreenPtr pScreen, rfbClientPtr cl,
                          RegionPtr src, RegionPtr dst, int dx, int dy);

/* GC funcs */

static void rfbValidateGC(GCPtr pGC, unsigned long changes,
                          DrawablePtr pDrawable);
static void rfbChangeGC(GCPtr pGC, unsigned long mask);
static void rfbCopyGC(GCPtr pGCSrc, unsigned long mask, GCPtr pGCDst);
static void rfbDestroyGC(GCPtr pGC);
static void rfbChangeClip(GCPtr pGC, int type, pointer pvalue, int nrects);
static void rfbDestroyClip(GCPtr pGC);
static void rfbCopyClip(GCPtr pgcDst, GCPtr pgcSrc);

/* GC ops */

static void rfbFillSpans(DrawablePtr pDrawable, GCPtr pGC, int nInit,
                         DDXPointPtr pptInit, int *pwidthInit, int fSorted);
static void rfbSetSpans(DrawablePtr pDrawable, GCPtr pGC, char *psrc,
                        register DDXPointPtr ppt, int *pwidth, int nspans,
                        int fSorted);
static void rfbPutImage(DrawablePtr pDrawable, GCPtr pGC, int depth, int x,
                        int y, int w, int h, int leftPad, int format,
                        char *pBits);
static RegionPtr rfbCopyArea(DrawablePtr pSrc, DrawablePtr pDst, GCPtr pGC,
                             int srcx, int srcy, int w, int h, int dstx,
                             int dsty);
static RegionPtr rfbCopyPlane(DrawablePtr pSrc, DrawablePtr pDst,
                              register GCPtr pGC, int srcx, int srcy, int w,
                              int h, int dstx, int dsty, unsigned long plane);
static void rfbPolyPoint(DrawablePtr pDrawable, GCPtr pGC, int mode, int npt,
                         xPoint *pts);
static void rfbPolylines(DrawablePtr pDrawable, GCPtr pGC, int mode, int npt,
                         DDXPointPtr ppts);
static void rfbPolySegment(DrawablePtr pDrawable, GCPtr pGC, int nseg,
                           xSegment *segs);
static void rfbPolyRectangle(DrawablePtr pDrawable, GCPtr pGC, int nrects,
                             xRectangle *rects);
static void rfbPolyArc(DrawablePtr pDrawable, register GCPtr pGC, int narcs,
                       xArc *arcs);
static void rfbFillPolygon(register DrawablePtr pDrawable, register GCPtr pGC,
                           int shape, int mode, int count, DDXPointPtr pts);
static void rfbPolyFillRect(DrawablePtr pDrawable, GCPtr pGC, int nrects,
                            xRectangle *rects);
static void rfbPolyFillArc(DrawablePtr pDrawable, GCPtr pGC, int narcs,
                           xArc *arcs);
static void GetTextBoundingBox(DrawablePtr pDrawable, FontPtr font, int x,
                               int y, int n, BoxPtr pbox);
static int rfbPolyText8(DrawablePtr pDrawable, GCPtr pGC, int x, int y,
                        int count, char *chars);
static int rfbPolyText16(DrawablePtr pDrawable, GCPtr pGC, int x, int y,
                         int count, unsigned short *chars);
static void rfbImageText8(DrawablePtr pDrawable, GCPtr pGC, int x, int y,
                          int count, char *chars);
static void rfbImageText16(DrawablePtr pDrawable, GCPtr pGC, int x, int y,
                           int count, unsigned short *chars);
static void rfbImageGlyphBlt(DrawablePtr pDrawable, GCPtr pGC, int x, int y,
                             unsigned int nglyph, CharInfoPtr *ppci,
                             pointer pglyphBase);
static void rfbPolyGlyphBlt(DrawablePtr pDrawable, GCPtr pGC, int x, int y,
                            unsigned int nglyph, CharInfoPtr *ppci,
                            pointer pglyphBase);
static void rfbPushPixels(GCPtr pGC, PixmapPtr pBitMap, DrawablePtr pDrawable,
                          int w, int h, int x, int y);


static const GCFuncs rfbGCFuncs = {
  rfbValidateGC,
  rfbChangeGC,
  rfbCopyGC,
  rfbDestroyGC,
  rfbChangeClip,
  rfbDestroyClip,
  rfbCopyClip,
};


static const GCOps rfbGCOps = {
  rfbFillSpans,     rfbSetSpans,     rfbPutImage,
  rfbCopyArea,      rfbCopyPlane,    rfbPolyPoint,
  rfbPolylines,     rfbPolySegment,  rfbPolyRectangle,
  rfbPolyArc,       rfbFillPolygon,  rfbPolyFillRect,
  rfbPolyFillArc,   rfbPolyText8,    rfbPolyText16,
  rfbImageText8,    rfbImageText16,  rfbImageGlyphBlt,
  rfbPolyGlyphBlt,  rfbPushPixels
};


void ClipToScreen(ScreenPtr pScreen, RegionPtr pRegion)
{
  RegionRec screenRegion;
  BoxRec box = { 0, 0, pScreen->width, pScreen->height };

  REGION_INIT(pScreen, &screenRegion, &box, 0);
  REGION_INTERSECT(pScreen, pRegion, pRegion, &screenRegion);
  REGION_UNINIT(pScreen, &screenRegion);
}


/****************************************************************************/
/*
 * Screen functions wrapper stuff
 */
/****************************************************************************/

#define SCREEN_PROLOGUE(scrn, field)  \
  ScreenPtr pScreen = scrn;  \
  rfbFBInfoPtr prfb = &rfbFB;  \
  pScreen->field = prfb->field;

#define SCREEN_EPILOGUE(field, wrapper)  \
  pScreen->field = wrapper;


/*
 * CloseScreen wrapper -- unwrap everything, free the private data
 * and call the wrapped CloseScreen function.
 */

Bool rfbCloseScreen(ScreenPtr pScreen)
{
  rfbFBInfoPtr prfb = &rfbFB;
#ifdef RENDER
  PictureScreenPtr ps;
#endif

  pScreen->CloseScreen = prfb->CloseScreen;
#ifdef DRI3
  if (driNode)
    pScreen->SourceValidate = prfb->SourceValidate;
#endif
  pScreen->CreateGC = prfb->CreateGC;
  pScreen->CopyWindow = prfb->CopyWindow;
  pScreen->ClearToBackground = prfb->ClearToBackground;
#ifdef RENDER
  ps = GetPictureScreenIfSet(pScreen);
  if (ps) {
    ps->Composite = prfb->Composite;
    ps->Glyphs = prfb->Glyphs;
#ifdef DRI3
    if (driNode) {
      ps->CompositeRects = prfb->CompositeRects;
      ps->Trapezoids = prfb->Trapezoids;
      ps->Triangles = prfb->Triangles;
      ps->TriStrip = prfb->TriStrip;
      ps->TriFan = prfb->TriFan;
    }
#endif
  }
#endif
  pScreen->InstallColormap = prfb->InstallColormap;
  pScreen->UninstallColormap = prfb->UninstallColormap;
  pScreen->ListInstalledColormaps = prfb->ListInstalledColormaps;
  pScreen->StoreColors = prfb->StoreColors;
  pScreen->SaveScreen = prfb->SaveScreen;

  TRC("rfbCloseScreen(): Unwrapped screen functions\n");

  return (*pScreen->CloseScreen) (pScreen);
}


#ifdef DRI3

void rfbSourceValidate(DrawablePtr pDrawable, int x, int y, int width,
                       int height, unsigned int subWindowMode)
{
  SCREEN_PROLOGUE(pDrawable->pScreen, SourceValidate);

  TRC("rfbSourceValidate() Drawable=0x%.8x x=%d y=%d width=%d height=%d subWindowMode=%u\n",
      XID(pDrawable), x, y, width, height, subWindowMode);

  rfbDRI3SyncBOToDrawable(pDrawable);

  (*pScreen->SourceValidate) (pDrawable, x, y, width, height, subWindowMode);

  SCREEN_EPILOGUE(SourceValidate, rfbSourceValidate);
}

#endif


/*
 * CreateGC - wrap the GC funcs (the GC ops will be wrapped when the GC
 * func "ValidateGC" is called).
 */

Bool rfbCreateGC(GCPtr pGC)
{
  Bool ret;
  rfbGCPtr pGCPriv;

  SCREEN_PROLOGUE(pGC->pScreen, CreateGC);

  pGCPriv = (rfbGCPtr)dixLookupPrivate(&pGC->devPrivates, &rfbGCKey);

  ret = (*pScreen->CreateGC) (pGC);

  TRC("rfbCreateGC() pGC=0x%.8lx\n", (unsigned long)pGC);

  pGCPriv->wrapOps = NULL;
  pGCPriv->wrapFuncs = pGC->funcs;
  pGC->funcs = &rfbGCFuncs;

  SCREEN_EPILOGUE(CreateGC, rfbCreateGC);

  return ret;
}


/*
 * CopyWindow - the region being modified is the translation of the old
 * region, clipped to the border clip region of the window.  Note that any
 * parts of the window which have become newly-visible will not be affected by
 * this call - a separate PaintWindowBackground/Border will be called to do
 * that.  If the client will accept CopyRect messages then use rfbCopyRegion to
 * optimise the pending screen changes into a single "copy region" plus the
 * ordinary modified region.
 */

void rfbCopyWindow(WindowPtr pWin, DDXPointRec ptOldOrg, RegionPtr pOldRegion)
{
  int dx, dy;
  rfbClientPtr cl;
  RegionRec srcRegion, dstRegion;

  SCREEN_PROLOGUE(pWin->drawable.pScreen, CopyWindow);

  TRC("rfbCopyWindow() Win=0x%.8x ptOldOrg={%d, %d}\n", pWin->drawable.id,
      ptOldOrg.x, ptOldOrg.y);
  TRC_REGION(pScreen, pOldRegion, "rfbCopyWindow() pOldRegion");

  dx = pWin->drawable.x - ptOldOrg.x;
  dy = pWin->drawable.y - ptOldOrg.y;

  REGION_INIT(pScreen, &dstRegion, NullBox, 0);
  REGION_COPY(pScreen, &dstRegion, pOldRegion);
  ClipToScreen(pScreen, &dstRegion);
  REGION_TRANSLATE(pScreen, &dstRegion, dx, dy);
  ClipToScreen(pScreen, &dstRegion);
  REGION_INTERSECT(pScreen, &dstRegion, &dstRegion, &pWin->borderClip);

  for (cl = rfbClientHead; cl; cl = cl->next) {
    if (cl->useCopyRect) {
      REGION_INIT(pScreen, &srcRegion, NullBox, 0);
      REGION_COPY(pScreen, &srcRegion, pOldRegion);

      if (!prfb->dontSendFramebufferUpdate || !cl->enableCursorShapeUpdates)
        rfbCopyRegion(pScreen, cl, &srcRegion, &dstRegion, dx, dy);

      REGION_UNINIT(pSrc->pScreen, &srcRegion);

    } else {

      REGION_UNION(pScreen, &cl->modifiedRegion, &cl->modifiedRegion,
                   &dstRegion);
    }
  }

  TRC_REGION(pScreen, &dstRegion, "rfbCopyWindow() mod");

  REGION_UNINIT(pSrc->pScreen, &dstRegion);

  (*pScreen->CopyWindow) (pWin, ptOldOrg, pOldRegion);

  SCHEDULE_FB_UPDATE(pScreen, prfb);

  SCREEN_EPILOGUE(CopyWindow, rfbCopyWindow);
}


/*
 * ClearToBackground - when generateExposures is false, the region being
 * modified is the given rectangle (clipped to the "window clip region").
 */

void rfbClearToBackground(WindowPtr pWin, int x, int y, int w, int h,
                          Bool generateExposures)
{
  RegionRec tmpRegion;
  BoxRec box;

  SCREEN_PROLOGUE(pWin->drawable.pScreen, ClearToBackground);

  TRC("rfbClearToBackground() Win=0x%.8x x=%d y=%d w=%d h=%d generateExposures=%d\n",
      pWin->drawable.id, x, y, w, h, generateExposures);

  if (!generateExposures) {
    box.x1 = x + pWin->drawable.x;
    box.y1 = y + pWin->drawable.y;
    box.x2 = w ? (box.x1 + w) : (pWin->drawable.x + pWin->drawable.width);
    box.y2 = h ? (box.y1 + h) : (pWin->drawable.y + pWin->drawable.height);

    SAFE_REGION_INIT(pScreen, &tmpRegion, &box, 0);

    REGION_INTERSECT(pScreen, &tmpRegion, &tmpRegion, &pWin->clipList);

    ADD_TO_MODIFIED_REGION(pScreen, &tmpRegion);

    TRC_REGION(pScreen, &tmpRegion, "rfbClearToBackground() mod");

    REGION_UNINIT(pScreen, &tmpRegion);
  }

  (*pScreen->ClearToBackground) (pWin, x, y, w, h, generateExposures);

  if (!generateExposures)
    SCHEDULE_FB_UPDATE(pScreen, prfb);

  SCREEN_EPILOGUE(ClearToBackground, rfbClearToBackground);
}


/****************************************************************************/
/*
 * GC funcs wrapper stuff
 *
 * We only really want to wrap the GC ops, but to do this we need to wrap
 * ValidateGC and so all the other GC funcs must be wrapped as well.
 */
/****************************************************************************/

#define GC_FUNC_PROLOGUE(pGC)  \
  rfbGCPtr pGCPriv =  \
    (rfbGCPtr)dixLookupPrivate(&(pGC)->devPrivates, &rfbGCKey);  \
  (pGC)->funcs = pGCPriv->wrapFuncs;  \
  if (pGCPriv->wrapOps)  \
    (pGC)->ops = pGCPriv->wrapOps;

#define GC_FUNC_EPILOGUE(pGC)  \
  pGCPriv->wrapFuncs = (pGC)->funcs;  \
  (pGC)->funcs = &rfbGCFuncs;  \
  if (pGCPriv->wrapOps) {  \
    pGCPriv->wrapOps = (pGC)->ops;  \
    (pGC)->ops = &rfbGCOps;  \
  }


/*
 * ValidateGC - call the wrapped ValidateGC, then wrap the resulting GC ops if
 * the drawing will be to a viewable window or the screen pixmap.
 */

static void rfbValidateGC(GCPtr pGC, unsigned long changes,
                          DrawablePtr pDrawable)
{
  GC_FUNC_PROLOGUE(pGC);

  TRC("rfbValidateGC() pGC=0x%.8lx changes=%ul Drawable=0x%.8x\n",
      (unsigned long)pGC, changes, XID(pDrawable));

  (*pGC->funcs->ValidateGC) (pGC, changes, pDrawable);

  pGCPriv->wrapOps = NULL;
  if (pDrawable->type == DRAWABLE_WINDOW && ((WindowPtr)pDrawable)->viewable) {
    WindowPtr pWin = (WindowPtr)pDrawable;
    RegionPtr pRegion = &pWin->clipList;

    if (pGC->subWindowMode == IncludeInferiors)
      pRegion = &pWin->borderClip;
    if (REGION_NOTEMPTY(pDrawable->pScreen, pRegion)) {
      pGCPriv->wrapOps = pGC->ops;
      TRC("rfbValidateGC(): wrapped GC ops\n");
    }
  } else if (pDrawable ==
             &pGC->pScreen->GetScreenPixmap(pGC->pScreen)->drawable) {
    pGCPriv->wrapOps = pGC->ops;
    TRC("rfbValidateGC(): wrapped GC ops\n");
  }

  GC_FUNC_EPILOGUE(pGC);
}


/*
 * All other GC funcs simply unwrap the GC funcs and ops, call the wrapped
 * function and then rewrap the funcs and ops.
 */

static void rfbChangeGC(GCPtr pGC, unsigned long mask)
{
  GC_FUNC_PROLOGUE(pGC);
  (*pGC->funcs->ChangeGC) (pGC, mask);
  GC_FUNC_EPILOGUE(pGC);
}

static void rfbCopyGC(GCPtr pGCSrc, unsigned long mask, GCPtr pGCDst)
{
  GC_FUNC_PROLOGUE(pGCDst);
  (*pGCDst->funcs->CopyGC) (pGCSrc, mask, pGCDst);
  GC_FUNC_EPILOGUE(pGCDst);
}

static void rfbDestroyGC(GCPtr pGC)
{
  GC_FUNC_PROLOGUE(pGC);
  (*pGC->funcs->DestroyGC) (pGC);
  GC_FUNC_EPILOGUE(pGC);
}

static void rfbChangeClip(GCPtr pGC, int type, pointer pvalue, int nrects)
{
  GC_FUNC_PROLOGUE(pGC);
  (*pGC->funcs->ChangeClip) (pGC, type, pvalue, nrects);
  GC_FUNC_EPILOGUE(pGC);
}

static void rfbDestroyClip(GCPtr pGC)
{
  GC_FUNC_PROLOGUE(pGC);
  (*pGC->funcs->DestroyClip) (pGC);
  GC_FUNC_EPILOGUE(pGC);
}

static void rfbCopyClip(GCPtr pgcDst, GCPtr pgcSrc)
{
  GC_FUNC_PROLOGUE(pgcDst);
  (*pgcDst->funcs->CopyClip) (pgcDst, pgcSrc);
  GC_FUNC_EPILOGUE(pgcDst);
}


/****************************************************************************/
/*
 * GC ops wrapper stuff
 *
 * Note that these routines will only have been wrapped for drawing to
 * viewable windows so we don't need to check each time that the drawable
 * is a viewable window.
 */
/****************************************************************************/

#define GC_OP_PROLOGUE(pDrawable, pGC)  \
  rfbFBInfoPtr prfb = &rfbFB;  \
  rfbGCPtr pGCPrivate =  \
    (rfbGCPtr)dixLookupPrivate(&(pGC)->devPrivates, &rfbGCKey);  \
  const GCFuncs *oldFuncs = pGC->funcs;  \
  (pGC)->funcs = pGCPrivate->wrapFuncs;  \
  (pGC)->ops = pGCPrivate->wrapOps;

#define GC_OP_EPILOGUE(pGC)  \
  pGCPrivate->wrapOps = (pGC)->ops;  \
  (pGC)->funcs = oldFuncs;  \
  (pGC)->ops = &rfbGCOps;


/*
 * FillSpans - being very safe - assume the entire clip region is damaged.
 */

static void rfbFillSpans(DrawablePtr pDrawable, GCPtr pGC,
                         int nInit,            /* number of spans to fill */
                         DDXPointPtr pptInit,  /* pointer to list of start points */
                         int *pwidthInit,      /* pointer to list of n widths */
                         int fSorted)
{
  GC_OP_PROLOGUE(pDrawable, pGC);

  TRC("rfbFillSpans() Drawable=0x%.8x pGC=0x%.8lx\n", XID(pDrawable),
      (unsigned long)pGC);

  if (pDrawable->type == DRAWABLE_WINDOW) {
    RegionRec changed;
    REGION_INIT(pDrawable->pScreen, &changed, NullBox, 0);
    REGION_COPY(pDrawable->pScreen, &changed, pGC->pCompositeClip);
    REGION_INTERSECT(pDrawable->pScreen, &changed, &changed,
                     &((WindowPtr)pDrawable)->borderClip);
    ADD_TO_MODIFIED_REGION(pDrawable->pScreen, &changed);
    TRC_REGION(pDrawable->pScreen, &changed, "rfbFillSpans() mod");
    REGION_UNINIT(pDrawable->pScreen, &changed);
  } else {
    ADD_TO_MODIFIED_REGION(pDrawable->pScreen, pGC->pCompositeClip);
    TRC_REGION(pDrawable->pScreen, pGC->pCompositeClip, "rfbFillSpans() mod");
  }

#ifdef DRI3
  rfbDRI3SyncBOToDrawable(pDrawable);
#endif

  (*pGC->ops->FillSpans) (pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted);

#ifdef DRI3
  rfbDRI3SyncDrawableToBO(pDrawable);
#endif

  SCHEDULE_FB_UPDATE(pDrawable->pScreen, prfb);

  GC_OP_EPILOGUE(pGC);
}


/*
 * SetSpans - being very safe - assume the entire clip region is damaged.
 */

static void rfbSetSpans(DrawablePtr pDrawable, GCPtr pGC, char *psrc,
                        register DDXPointPtr ppt, int *pwidth, int nspans,
                        int fSorted)
{
  GC_OP_PROLOGUE(pDrawable, pGC);

  TRC("rfbSetSpans() Drawable=0x%.8x pGC=0x%.8lx\n", XID(pDrawable),
      (unsigned long)pGC);

  if (pDrawable->type == DRAWABLE_WINDOW) {
    RegionRec changed;
    REGION_INIT(pDrawable->pScreen, &changed, NullBox, 0);
    REGION_COPY(pDrawable->pScreen, &changed, pGC->pCompositeClip);
    REGION_INTERSECT(pDrawable->pScreen, &changed, &changed,
                     &((WindowPtr)pDrawable)->borderClip);
    ADD_TO_MODIFIED_REGION(pDrawable->pScreen, &changed);
    TRC_REGION(pDrawable->pScreen, &changed, "rfbSetSpans() mod");
    REGION_UNINIT(pDrawable->pScreen, &changed);
  } else {
    ADD_TO_MODIFIED_REGION(pDrawable->pScreen, pGC->pCompositeClip);
    TRC_REGION(pDrawable->pScreen, pGC->pCompositeClip, "rfbSetSpans() mod");
  }

#ifdef DRI3
  rfbDRI3SyncBOToDrawable(pDrawable);
#endif

  (*pGC->ops->SetSpans) (pDrawable, pGC, psrc, ppt, pwidth, nspans, fSorted);

#ifdef DRI3
  rfbDRI3SyncDrawableToBO(pDrawable);
#endif

  SCHEDULE_FB_UPDATE(pDrawable->pScreen, prfb);

  GC_OP_EPILOGUE(pGC);
}


/*
 * PutImage - the region being modified is the rectangle of the
 * PutImage (clipped to the window clip region).
 */

static void rfbPutImage(DrawablePtr pDrawable, GCPtr pGC, int depth,
                        int x, int y, int w, int h, int leftPad, int format,
                        char *pBits)
{
  RegionRec tmpRegion;
  BoxRec box;

  GC_OP_PROLOGUE(pDrawable, pGC);

  TRC("rfbPutImage() Drawable=0x%.8x pGC=0x%.8lx depth=%d x=%d y=%d w=%d h=%d leftPad=%d format=%d\n",
      XID(pDrawable), (unsigned long)pGC, depth, x, y, w, h, leftPad, format);

  box.x1 = x + pDrawable->x;
  box.y1 = y + pDrawable->y;
  box.x2 = box.x1 + w;
  box.y2 = box.y1 + h;

  SAFE_REGION_INIT(pDrawable->pScreen, &tmpRegion, &box, 0);

  REGION_INTERSECT(pDrawable->pScreen, &tmpRegion, &tmpRegion,
                   pGC->pCompositeClip);

  ADD_TO_MODIFIED_REGION(pDrawable->pScreen, &tmpRegion);
  ADD_TO_ALR_REGION(pDrawable->pScreen, &tmpRegion);

  TRC_REGION(pDrawable->pScreen, &tmpRegion, "rfbPutImage() mod");

  REGION_UNINIT(pDrawable->pScreen, &tmpRegion);

#ifdef DRI3
  rfbDRI3SyncBOToDrawable(pDrawable);
#endif

  (*pGC->ops->PutImage) (pDrawable, pGC, depth, x, y, w, h, leftPad, format,
                         pBits);

#ifdef DRI3
  rfbDRI3SyncDrawableToBO(pDrawable);
#endif

  SCHEDULE_FB_UPDATE(pDrawable->pScreen, prfb);

  GC_OP_EPILOGUE(pGC);
}


/*
 * CopyArea - the region being modified is the destination rectangle (clipped
 * to the window clip region).
 * If the client will accept CopyRect messages then use rfbCopyRegion
 * to optimise the pending screen changes into a single "copy region" plus
 * the ordinary modified region.
 */

static RegionPtr rfbCopyArea(DrawablePtr pSrc, DrawablePtr pDst, GCPtr pGC,
                             int srcx, int srcy, int w, int h,
                             int dstx, int dsty)
{
  rfbClientPtr cl;
  RegionPtr rgn;
  RegionRec srcRegion, dstRegion;
  BoxRec box;

  GC_OP_PROLOGUE(pDst, pGC);

  TRC("rfbCopyArea() Src=0x%.8x Dst=0x%.8x pGC=0x%.8lx srcx=%d srcy=%d w=%d h=%d dstx=%d dsty=%d\n",
      XID(pSrc), XID(pDst), (unsigned long)pGC, srcx, srcy, w, h, dstx, dsty);

  box.x1 = dstx + pDst->x;
  box.y1 = dsty + pDst->y;
  box.x2 = box.x1 + w;
  box.y2 = box.y1 + h;

  SAFE_REGION_INIT(pDst->pScreen, &dstRegion, &box, 0);
  REGION_INTERSECT(pDst->pScreen, &dstRegion, &dstRegion, pGC->pCompositeClip);

  if (is_visible(pSrc)) {
    box.x1 = srcx + pSrc->x;
    box.y1 = srcy + pSrc->y;
    box.x2 = box.x1 + w;
    box.y2 = box.y1 + h;

    for (cl = rfbClientHead; cl; cl = cl->next) {
      if (cl->useCopyRect) {
        SAFE_REGION_INIT(pSrc->pScreen, &srcRegion, &box, 0);
        if (pSrc->type == DRAWABLE_WINDOW &&
            REGION_NOTEMPTY(pScreen, &((WindowPtr)pSrc)->clipList)) {
          REGION_INTERSECT(pSrc->pScreen, &srcRegion, &srcRegion,
                           &((WindowPtr)pSrc)->clipList);
        }

        if (!prfb->dontSendFramebufferUpdate || !cl->enableCursorShapeUpdates)
          rfbCopyRegion(pSrc->pScreen, cl, &srcRegion, &dstRegion,
                        dstx + pDst->x - srcx - pSrc->x,
                        dsty + pDst->y - srcy - pSrc->y);

        REGION_UNINIT(pSrc->pScreen, &srcRegion);

      } else {

        REGION_UNION(pScreen, &cl->modifiedRegion, &cl->modifiedRegion,
                     &dstRegion);
      }
    }

  } else {

    ADD_TO_MODIFIED_REGION(pDst->pScreen, &dstRegion);
  }

  TRC_REGION(pDst->pScreen, &dstRegion, "rfbCopyArea() mod");

  REGION_UNINIT(pDst->pScreen, &dstRegion);

#ifdef DRI3
  rfbDRI3SyncBOToDrawable(pDst);
#endif

  rgn = (*pGC->ops->CopyArea) (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty);

#ifdef DRI3
  rfbDRI3SyncDrawableToBO(pDst);
#endif

  SCHEDULE_FB_UPDATE(pDst->pScreen, prfb);

  GC_OP_EPILOGUE(pGC);

  return rgn;
}


/*
 * CopyPlane - the region being modified is the destination rectangle (clipped
 * to the window clip region).
 */

static RegionPtr rfbCopyPlane(DrawablePtr pSrc, DrawablePtr pDst,
                              register GCPtr pGC, int srcx, int srcy,
                              int w, int h, int dstx, int dsty,
                              unsigned long plane)
{
  RegionPtr rgn;
  RegionRec tmpRegion;
  BoxRec box;

  GC_OP_PROLOGUE(pDst, pGC);

  TRC("rfbCopyPlane() Src=0x%.8x Dst=0x%.8x pGC=0x%.8lx srcx=%d srcy=%d w=%d h=%d dstx=%d dsty=%d plane=%lu\n",
      XID(pSrc), XID(pDst), (unsigned long)pGC, srcx, srcy, w, h, dstx, dsty,
      plane);

  box.x1 = dstx + pDst->x;
  box.y1 = dsty + pDst->y;
  box.x2 = box.x1 + w;
  box.y2 = box.y1 + h;

  SAFE_REGION_INIT(pDst->pScreen, &tmpRegion, &box, 0);

  REGION_INTERSECT(pDst->pScreen, &tmpRegion, &tmpRegion, pGC->pCompositeClip);

  ADD_TO_MODIFIED_REGION(pDst->pScreen, &tmpRegion);

  TRC_REGION(pDst->pScreen, &tmpRegion, "rfbCopyPlane() mod");

  REGION_UNINIT(pDst->pScreen, &tmpRegion);

#ifdef DRI3
  rfbDRI3SyncBOToDrawable(pDst);
#endif

  rgn = (*pGC->ops->CopyPlane) (pSrc, pDst, pGC, srcx, srcy, w, h,
                                dstx, dsty, plane);

#ifdef DRI3
  rfbDRI3SyncDrawableToBO(pDst);
#endif

  SCHEDULE_FB_UPDATE(pDst->pScreen, prfb);

  GC_OP_EPILOGUE(pGC);

  return rgn;
}


/*
 * PolyPoint - find the smallest rectangle which encloses the points drawn
 * (and clip).
 */

static void rfbPolyPoint(DrawablePtr pDrawable, GCPtr pGC,
                         int mode,  /* Origin or Previous */
                         int npt, xPoint *pts)
{
  int i;
  RegionRec tmpRegion;
  BoxRec box;

  GC_OP_PROLOGUE(pDrawable, pGC);

  TRC("rfbPolyPoint() Drawable=0x%.8x pGC=0x%.8lx\n", XID(pDrawable),
      (unsigned long)pGC);

  if (npt) {
    int minX = pts[0].x, maxX = pts[0].x;
    int minY = pts[0].y, maxY = pts[0].y;

    if (mode == CoordModePrevious) {
      int x = pts[0].x, y = pts[0].y;

      for (i = 1; i < npt; i++) {
        x += pts[i].x;
        y += pts[i].y;
        if (x < minX) minX = x;
        if (x > maxX) maxX = x;
        if (y < minY) minY = y;
        if (y > maxY) maxY = y;
      }
    } else {
      for (i = 1; i < npt; i++) {
        if (pts[i].x < minX) minX = pts[i].x;
        if (pts[i].x > maxX) maxX = pts[i].x;
        if (pts[i].y < minY) minY = pts[i].y;
        if (pts[i].y > maxY) maxY = pts[i].y;
      }
    }

    box.x1 = minX + pDrawable->x;
    box.y1 = minY + pDrawable->y;
    box.x2 = maxX + 1 + pDrawable->x;
    box.y2 = maxY + 1 + pDrawable->y;

    SAFE_REGION_INIT(pDrawable->pScreen, &tmpRegion, &box, 0);

    REGION_INTERSECT(pDrawable->pScreen, &tmpRegion, &tmpRegion,
                     pGC->pCompositeClip);

    ADD_TO_MODIFIED_REGION(pDrawable->pScreen, &tmpRegion);

    TRC_REGION(pDrawable->pScreen, &tmpRegion, "rfbPolyPoint() mod");

    REGION_UNINIT(pDrawable->pScreen, &tmpRegion);
#ifdef DRI3
    rfbDRI3SyncBOToDrawable(pDrawable);
#endif
  }

  (*pGC->ops->PolyPoint) (pDrawable, pGC, mode, npt, pts);

  if (npt) {
#ifdef DRI3
    rfbDRI3SyncDrawableToBO(pDrawable);
#endif
    SCHEDULE_FB_UPDATE(pDrawable->pScreen, prfb);
  }

  GC_OP_EPILOGUE(pGC);
}


/*
 * PolyLines - take the union of bounding boxes around each line (and clip).
 */

static void rfbPolylines(DrawablePtr pDrawable, GCPtr pGC, int mode, int npt,
                         DDXPointPtr ppts)
{
  RegionPtr tmpRegion;
  xRectangle *rects;
  int i, extra, nlines, lw;
  int x1, x2, y1, y2;

  GC_OP_PROLOGUE(pDrawable, pGC);

  TRC("rfbPolyLines() Drawable=0x%.8x pGC=0x%.8lx\n", XID(pDrawable),
      (unsigned long)pGC);

  if (npt) {
    lw = pGC->lineWidth;
    if (lw == 0)
      lw = 1;

    if (npt == 1) {
      nlines = 1;
      rects = (xRectangle *)rfbAlloc(sizeof(xRectangle));

      rects[0].x = ppts[0].x - lw + pDrawable->x;  /* being safe here */
      rects[0].y = ppts[0].y - lw + pDrawable->y;
      rects[0].width = 2 * lw;
      rects[0].height = 2 * lw;
    } else {
      nlines = npt - 1;
      rects = (xRectangle *)rfbAlloc(nlines * sizeof(xRectangle));

      /*
       * mitered joins can project quite a way from
       * the line end; the 11 degree miter limit limits
       * this extension to lw / (2 * tan(11/2)), rounded up
       * and converted to int yields 6 * lw
       */

      if (pGC->joinStyle == JoinMiter)
        extra = 6 * lw;
      else
        extra = lw / 2;

      x1 = ppts[0].x + pDrawable->x;
      y1 = ppts[0].y + pDrawable->y;

      for (i = 0; i < nlines; i++) {
        if (mode == CoordModeOrigin) {
          x2 = pDrawable->x + ppts[i + 1].x;
          y2 = pDrawable->y + ppts[i + 1].y;
        } else {
          x2 = x1 + ppts[i + 1].x;
          y2 = y1 + ppts[i + 1].y;
        }

        if (x1 > x2) {
          rects[i].x = x2 - extra;
          rects[i].width = x1 - x2 + 1 + 2 * extra;
        } else {
          rects[i].x = x1 - extra;
          rects[i].width = x2 - x1 + 1 + 2 * extra;
        }

        if (y1 > y2) {
          rects[i].y = y2 - extra;
          rects[i].height = y1 - y2 + 1 + 2 * extra;
        } else {
          rects[i].y = y1 - extra;
          rects[i].height = y2 - y1 + 1 + 2 * extra;
        }

        x1 = x2;
        y1 = y2;
      }
    }
    tmpRegion = RECTS_TO_REGION(pDrawable->pScreen, nlines, rects, CT_NONE);
    REGION_INTERSECT(pDrawable->pScreen, tmpRegion, tmpRegion,
                     pGC->pCompositeClip);

    ADD_TO_MODIFIED_REGION(pDrawable->pScreen, tmpRegion);

    TRC_REGION(pDrawable->pScreen, tmpRegion, "rfbPolyLines() mod");

    REGION_DESTROY(pDrawable->pScreen, tmpRegion);
    free((char *)rects);
#ifdef DRI3
    rfbDRI3SyncBOToDrawable(pDrawable);
#endif
  }

  (*pGC->ops->Polylines) (pDrawable, pGC, mode, npt, ppts);

  if (npt) {
#ifdef DRI3
    rfbDRI3SyncDrawableToBO(pDrawable);
#endif
    SCHEDULE_FB_UPDATE(pDrawable->pScreen, prfb);
  }

  GC_OP_EPILOGUE(pGC);
}


/*
 * PolySegment - take the union of bounding boxes around each segment (and
 * clip).
 */

static void rfbPolySegment(DrawablePtr pDrawable, GCPtr pGC, int nseg,
                           xSegment *segs)
{
  RegionPtr tmpRegion;
  xRectangle *rects;
  int i, extra, lw;

  GC_OP_PROLOGUE(pDrawable, pGC);

  TRC("rfbPolySegment() Drawable=0x%.8x pGC=0x%.8lx\n", XID(pDrawable),
      (unsigned long)pGC);

  if (nseg) {
    rects = (xRectangle *)rfbAlloc(nseg * sizeof(xRectangle));

    lw = pGC->lineWidth;
    if (lw == 0)
      lw = 1;

    extra = lw / 2;

    for (i = 0; i < nseg; i++) {
      if (segs[i].x1 > segs[i].x2) {
        rects[i].x = segs[i].x2 - extra + pDrawable->x;
        rects[i].width = segs[i].x1 - segs[i].x2 + 1 + 2 * extra;
      } else {
        rects[i].x = segs[i].x1 - extra + pDrawable->x;
        rects[i].width = segs[i].x2 - segs[i].x1 + 1 + 2 * extra;
      }

      if (segs[i].y1 > segs[i].y2) {
        rects[i].y = segs[i].y2 - extra + pDrawable->y;
        rects[i].height = segs[i].y1 - segs[i].y2 + 1 + 2 * extra;
      } else {
        rects[i].y = segs[i].y1 - extra + pDrawable->y;
        rects[i].height = segs[i].y2 - segs[i].y1 + 1 + 2 * extra;
      }
    }

    tmpRegion = RECTS_TO_REGION(pDrawable->pScreen, nseg, rects, CT_NONE);
    REGION_INTERSECT(pDrawable->pScreen, tmpRegion, tmpRegion,
                     pGC->pCompositeClip);

    ADD_TO_MODIFIED_REGION(pDrawable->pScreen, tmpRegion);

    TRC_REGION(pDrawable->pScreen, tmpRegion, "rfbPolySegment() mod");

    REGION_DESTROY(pDrawable->pScreen, tmpRegion);
    free((char *)rects);
#ifdef DRI3
    rfbDRI3SyncBOToDrawable(pDrawable);
#endif
  }

  (*pGC->ops->PolySegment) (pDrawable, pGC, nseg, segs);

  if (nseg) {
#ifdef DRI3
    rfbDRI3SyncDrawableToBO(pDrawable);
#endif
    SCHEDULE_FB_UPDATE(pDrawable->pScreen, prfb);
  }

  GC_OP_EPILOGUE(pGC);
}


/*
 * PolyRectangle (rectangle outlines) - take the union of bounding boxes
 * around each line (and clip).
 */

static void rfbPolyRectangle(DrawablePtr pDrawable, GCPtr pGC, int nrects,
                             xRectangle *rects)
{
  int i, extra, lw;
  RegionPtr tmpRegion;
  xRectangle *regRects;

  GC_OP_PROLOGUE(pDrawable, pGC);

  TRC("rfbPolyRectangle() Drawable=0x%.8x pGC=0x%.8lx\n", XID(pDrawable),
      (unsigned long)pGC);

  if (nrects) {
    regRects = (xRectangle *)rfbAlloc(nrects * 4 * sizeof(xRectangle));

    lw = pGC->lineWidth;
    if (lw == 0)
      lw = 1;

    extra = lw / 2;

    for (i = 0; i < nrects; i++) {
      regRects[i * 4].x = rects[i].x - extra + pDrawable->x;
      regRects[i * 4].y = rects[i].y - extra + pDrawable->y;
      regRects[i * 4].width = rects[i].width + 1 + 2 * extra;
      regRects[i * 4].height = 1 + 2 * extra;

      regRects[i * 4 + 1].x = rects[i].x - extra + pDrawable->x;
      regRects[i * 4 + 1].y = rects[i].y - extra + pDrawable->y;
      regRects[i * 4 + 1].width = 1 + 2 * extra;
      regRects[i * 4 + 1].height = rects[i].height + 1 + 2 * extra;

      regRects[i * 4 + 2].x =
        rects[i].x + rects[i].width - extra + pDrawable->x;
      regRects[i * 4 + 2].y = rects[i].y - extra + pDrawable->y;
      regRects[i * 4 + 2].width = 1 + 2 * extra;
      regRects[i * 4 + 2].height = rects[i].height + 1 + 2 * extra;

      regRects[i * 4 + 3].x = rects[i].x - extra + pDrawable->x;
      regRects[i * 4 + 3].y =
        rects[i].y + rects[i].height - extra + pDrawable->y;
      regRects[i * 4 + 3].width = rects[i].width + 1 + 2 * extra;
      regRects[i * 4 + 3].height = 1 + 2 * extra;
    }

    tmpRegion = RECTS_TO_REGION(pDrawable->pScreen, nrects * 4, regRects,
                                CT_NONE);
    REGION_INTERSECT(pDrawable->pScreen, tmpRegion, tmpRegion,
                     pGC->pCompositeClip);

    ADD_TO_MODIFIED_REGION(pDrawable->pScreen, tmpRegion);

    TRC_REGION(pDrawable->pScreen, tmpRegion, "rfbPolyRectangle() mod");

    REGION_DESTROY(pDrawable->pScreen, tmpRegion);
    free((char *)regRects);
#ifdef DRI3
    rfbDRI3SyncBOToDrawable(pDrawable);
#endif
  }

  (*pGC->ops->PolyRectangle) (pDrawable, pGC, nrects, rects);

  if (nrects) {
#ifdef DRI3
    rfbDRI3SyncDrawableToBO(pDrawable);
#endif
    SCHEDULE_FB_UPDATE(pDrawable->pScreen, prfb);
  }

  GC_OP_EPILOGUE(pGC);
}


/*
 * PolyArc - take the union of bounding boxes around each arc (and clip).
 * Bounding boxes assume each is a full circle / ellipse.
 */

static void rfbPolyArc(DrawablePtr pDrawable, register GCPtr pGC, int narcs,
                       xArc *arcs)
{
  int i, extra, lw;
  RegionPtr tmpRegion;
  xRectangle *rects;

  GC_OP_PROLOGUE(pDrawable, pGC);

  TRC("rfbPolyArc() Drawable=0x%.8x pGC=0x%.8lx\n", XID(pDrawable),
      (unsigned long)pGC);

  if (narcs) {
    rects = (xRectangle *)rfbAlloc(narcs * sizeof(xRectangle));

    lw = pGC->lineWidth;
    if (lw == 0)
      lw = 1;

    extra = lw / 2;

    for (i = 0; i < narcs; i++) {
      rects[i].x = arcs[i].x - extra + pDrawable->x;
      rects[i].y = arcs[i].y - extra + pDrawable->y;
      rects[i].width = arcs[i].width + lw;
      rects[i].height = arcs[i].height + lw;
    }

    tmpRegion = RECTS_TO_REGION(pDrawable->pScreen, narcs, rects, CT_NONE);
    REGION_INTERSECT(pDrawable->pScreen, tmpRegion, tmpRegion,
                     pGC->pCompositeClip);

    ADD_TO_MODIFIED_REGION(pDrawable->pScreen, tmpRegion);

    TRC_REGION(pDrawable->pScreen, tmpRegion, "rfbPolyArc() mod");

    REGION_DESTROY(pDrawable->pScreen, tmpRegion);
    free((char *)rects);
#ifdef DRI3
    rfbDRI3SyncBOToDrawable(pDrawable);
#endif
  }

  (*pGC->ops->PolyArc) (pDrawable, pGC, narcs, arcs);

  if (narcs) {
#ifdef DRI3
    rfbDRI3SyncDrawableToBO(pDrawable);
#endif
    SCHEDULE_FB_UPDATE(pDrawable->pScreen, prfb);
  }

  GC_OP_EPILOGUE(pGC);
}


/*
 * FillPolygon - take bounding box around polygon (and clip).
 */

static void rfbFillPolygon(register DrawablePtr pDrawable, register GCPtr pGC,
                           int shape, int mode, int count, DDXPointPtr pts)
{
  int i;
  RegionRec tmpRegion;
  BoxRec box;

  GC_OP_PROLOGUE(pDrawable, pGC);

  TRC("rfbFillPolygon() Drawable=0x%.8x pGC=0x%.8lx\n", XID(pDrawable),
      (unsigned long)pGC);

  if (count) {
    int minX = pts[0].x, maxX = pts[0].x;
    int minY = pts[0].y, maxY = pts[0].y;

    if (mode == CoordModePrevious) {
      int x = pts[0].x, y = pts[0].y;

      for (i = 1; i < count; i++) {
        x += pts[i].x;
        y += pts[i].y;
        if (x < minX) minX = x;
        if (x > maxX) maxX = x;
        if (y < minY) minY = y;
        if (y > maxY) maxY = y;
      }
    } else {
      for (i = 1; i < count; i++) {
        if (pts[i].x < minX) minX = pts[i].x;
        if (pts[i].x > maxX) maxX = pts[i].x;
        if (pts[i].y < minY) minY = pts[i].y;
        if (pts[i].y > maxY) maxY = pts[i].y;
      }
    }

    box.x1 = minX + pDrawable->x;
    box.y1 = minY + pDrawable->y;
    box.x2 = maxX + 1 + pDrawable->x;
    box.y2 = maxY + 1 + pDrawable->y;

    SAFE_REGION_INIT(pDrawable->pScreen, &tmpRegion, &box, 0);

    REGION_INTERSECT(pDrawable->pScreen, &tmpRegion, &tmpRegion,
                     pGC->pCompositeClip);

    ADD_TO_MODIFIED_REGION(pDrawable->pScreen, &tmpRegion);

    TRC_REGION(pDrawable->pScreen, &tmpRegion, "rfbFillPolygon() mod");

    REGION_UNINIT(pDrawable->pScreen, &tmpRegion);
#ifdef DRI3
    rfbDRI3SyncBOToDrawable(pDrawable);
#endif
  }

  (*pGC->ops->FillPolygon) (pDrawable, pGC, shape, mode, count, pts);

  if (count) {
#ifdef DRI3
    rfbDRI3SyncDrawableToBO(pDrawable);
#endif
    SCHEDULE_FB_UPDATE(pDrawable->pScreen, prfb);
  }

  GC_OP_EPILOGUE(pGC);
}


/*
 * PolyFillRect - take the union of the given rectangles (and clip).
 */

static void rfbPolyFillRect(DrawablePtr pDrawable, GCPtr pGC, int nrects,
                            xRectangle *rects)
{
  RegionPtr tmpRegion;
  xRectangle *regRects;
  int i;

  GC_OP_PROLOGUE(pDrawable, pGC);

  TRC("rfbPolyFillRect() Drawable=0x%.8x pGC=0x%.8lx\n", XID(pDrawable),
      (unsigned long)pGC);

  if (nrects) {
    regRects = (xRectangle *)rfbAlloc(nrects * sizeof(xRectangle));

    for (i = 0; i < nrects; i++) {
      regRects[i].x = rects[i].x + pDrawable->x;
      regRects[i].y = rects[i].y + pDrawable->y;
      regRects[i].width = rects[i].width;
      regRects[i].height = rects[i].height;
    }

    tmpRegion = RECTS_TO_REGION(pDrawable->pScreen, nrects, regRects, CT_NONE);
    REGION_INTERSECT(pDrawable->pScreen, tmpRegion, tmpRegion,
                     pGC->pCompositeClip);

    ADD_TO_MODIFIED_REGION(pDrawable->pScreen, tmpRegion);

    TRC_REGION(pDrawable->pScreen, tmpRegion, "rfbPolyFillRect() mod");

    REGION_DESTROY(pDrawable->pScreen, tmpRegion);
    free((char *)regRects);
#ifdef DRI3
    rfbDRI3SyncBOToDrawable(pDrawable);
#endif
  }

  (*pGC->ops->PolyFillRect) (pDrawable, pGC, nrects, rects);

  if (nrects) {
#ifdef DRI3
    rfbDRI3SyncDrawableToBO(pDrawable);
#endif
    SCHEDULE_FB_UPDATE(pDrawable->pScreen, prfb);
  }

  GC_OP_EPILOGUE(pGC);
}


/*
 * PolyFillArc - take the union of bounding boxes around each arc (and clip).
 * Bounding boxes assume each is a full circle / ellipse.
 */

static void rfbPolyFillArc(DrawablePtr pDrawable, GCPtr pGC, int narcs,
                           xArc *arcs)
{
  int i, extra, lw;
  RegionPtr tmpRegion;
  xRectangle *rects;

  GC_OP_PROLOGUE(pDrawable, pGC);

  TRC("rfbPolyFillArc() Drawable=0x%.8x pGC=0x%.8lx\n", XID(pDrawable),
      (unsigned long)pGC);

  if (narcs) {
    rects = (xRectangle *)rfbAlloc(narcs * sizeof(xRectangle));

    lw = pGC->lineWidth;
    if (lw == 0)
      lw = 1;

    extra = lw / 2;

    for (i = 0; i < narcs; i++) {
      rects[i].x = arcs[i].x - extra + pDrawable->x;
      rects[i].y = arcs[i].y - extra + pDrawable->y;
      rects[i].width = arcs[i].width + lw;
      rects[i].height = arcs[i].height + lw;
    }

    tmpRegion = RECTS_TO_REGION(pDrawable->pScreen, narcs, rects, CT_NONE);
    REGION_INTERSECT(pDrawable->pScreen, tmpRegion, tmpRegion,
                     pGC->pCompositeClip);

    ADD_TO_MODIFIED_REGION(pDrawable->pScreen, tmpRegion);

    TRC_REGION(pDrawable->pScreen, tmpRegion, "rfbPolyFillArc() mod");

    REGION_DESTROY(pDrawable->pScreen, tmpRegion);
    free((char *)rects);
#ifdef DRI3
    rfbDRI3SyncBOToDrawable(pDrawable);
#endif
  }

  (*pGC->ops->PolyFillArc) (pDrawable, pGC, narcs, arcs);

  if (narcs) {
#ifdef DRI3
    rfbDRI3SyncDrawableToBO(pDrawable);
#endif
    SCHEDULE_FB_UPDATE(pDrawable->pScreen, prfb);
  }

  GC_OP_EPILOGUE(pGC);
}


/*
 * Get a rough bounding box around n characters of the given font.
 */

static void GetTextBoundingBox(DrawablePtr pDrawable, FontPtr font, int x,
                               int y, int n, BoxPtr pbox)
{
  int maxAscent, maxDescent, maxCharWidth;

  if (FONTASCENT(font) > FONTMAXBOUNDS(font, ascent))
    maxAscent = FONTASCENT(font);
  else
    maxAscent = FONTMAXBOUNDS(font, ascent);

  if (FONTDESCENT(font) > FONTMAXBOUNDS(font, descent))
    maxDescent = FONTDESCENT(font);
  else
    maxDescent = FONTMAXBOUNDS(font, descent);

  if (FONTMAXBOUNDS(font, rightSideBearing) >
      FONTMAXBOUNDS(font, characterWidth))
    maxCharWidth = FONTMAXBOUNDS(font, rightSideBearing);
  else
    maxCharWidth = FONTMAXBOUNDS(font, characterWidth);

  pbox->x1 = pDrawable->x + x;
  pbox->y1 = pDrawable->y + y - maxAscent;
  pbox->x2 = pbox->x1 + maxCharWidth * n;
  pbox->y2 = pbox->y1 + maxAscent + maxDescent;

  if (FONTMINBOUNDS(font, leftSideBearing) < 0)
    pbox->x1 += FONTMINBOUNDS(font, leftSideBearing);
}


/*
 * PolyText8 - use rough bounding box.
 */

static int rfbPolyText8(DrawablePtr pDrawable, GCPtr pGC, int x, int y,
                        int count, char *chars)
{
  int ret;
  RegionRec tmpRegion;
  BoxRec box;

  GC_OP_PROLOGUE(pDrawable, pGC);

  TRC("rfbPolyText8() Drawable=0x%.8x pGC=0x%.8lx x=%d y=%d '%.*s'\n",
      XID(pDrawable), (unsigned long)pGC, x, y, count, chars);

  if (count) {
    GetTextBoundingBox(pDrawable, pGC->font, x, y, count, &box);

    SAFE_REGION_INIT(pDrawable->pScreen, &tmpRegion, &box, 0);

    REGION_INTERSECT(pDrawable->pScreen, &tmpRegion, &tmpRegion,
                     pGC->pCompositeClip);

    ADD_TO_MODIFIED_REGION(pDrawable->pScreen, &tmpRegion);

    TRC_REGION(pDrawable->pScreen, &tmpRegion, "rfbPolyText8() mod");

    REGION_UNINIT(pDrawable->pScreen, &tmpRegion);
#ifdef DRI3
    rfbDRI3SyncBOToDrawable(pDrawable);
#endif
  }

  ret = (*pGC->ops->PolyText8) (pDrawable, pGC, x, y, count, chars);

  if (count) {
#ifdef DRI3
    rfbDRI3SyncDrawableToBO(pDrawable);
#endif
    SCHEDULE_FB_UPDATE(pDrawable->pScreen, prfb);
  }

  GC_OP_EPILOGUE(pGC);
  return ret;
}


/*
 * PolyText16 - use rough bounding box.
 */

static int rfbPolyText16(DrawablePtr pDrawable, GCPtr pGC, int x, int y,
                         int count, unsigned short *chars)
{
  int ret;
  RegionRec tmpRegion;
  BoxRec box;

  GC_OP_PROLOGUE(pDrawable, pGC);

  TRC("rfbPolyText16() Drawable=0x%.8x pGC=0x%.8lx x=%d y=%d count=%d\n",
      XID(pDrawable), (unsigned long)pGC, x, y, count);

  if (count) {
    GetTextBoundingBox(pDrawable, pGC->font, x, y, count, &box);

    SAFE_REGION_INIT(pDrawable->pScreen, &tmpRegion, &box, 0);

    REGION_INTERSECT(pDrawable->pScreen, &tmpRegion, &tmpRegion,
                     pGC->pCompositeClip);

    ADD_TO_MODIFIED_REGION(pDrawable->pScreen, &tmpRegion);

    TRC_REGION(pDrawable->pScreen, &tmpRegion, "rfbPolyText16() mod");

    REGION_UNINIT(pDrawable->pScreen, &tmpRegion);
#ifdef DRI3
    rfbDRI3SyncBOToDrawable(pDrawable);
#endif
  }

  ret = (*pGC->ops->PolyText16) (pDrawable, pGC, x, y, count, chars);

  if (count) {
#ifdef DRI3
    rfbDRI3SyncDrawableToBO(pDrawable);
#endif
    SCHEDULE_FB_UPDATE(pDrawable->pScreen, prfb);
  }

  GC_OP_EPILOGUE(pGC);
  return ret;
}


/*
 * ImageText8 - use rough bounding box.
 */

static void rfbImageText8(DrawablePtr pDrawable, GCPtr pGC, int x, int y,
                          int count, char *chars)
{
  RegionRec tmpRegion;
  BoxRec box;

  GC_OP_PROLOGUE(pDrawable, pGC);

  TRC("rfbImageText8() Drawable=0x%.8x pGC=0x%.8lx x=%d y=%d '%.*s'\n",
      XID(pDrawable), (unsigned long)pGC, x, y, count, chars);

  if (count) {
    GetTextBoundingBox(pDrawable, pGC->font, x, y, count, &box);

    SAFE_REGION_INIT(pDrawable->pScreen, &tmpRegion, &box, 0);

    REGION_INTERSECT(pDrawable->pScreen, &tmpRegion, &tmpRegion,
                     pGC->pCompositeClip);

    ADD_TO_MODIFIED_REGION(pDrawable->pScreen, &tmpRegion);

    TRC_REGION(pDrawable->pScreen, &tmpRegion, "rfbImageText8() mod");

    REGION_UNINIT(pDrawable->pScreen, &tmpRegion);
#ifdef DRI3
    rfbDRI3SyncBOToDrawable(pDrawable);
#endif
  }

  (*pGC->ops->ImageText8) (pDrawable, pGC, x, y, count, chars);

  if (count) {
#ifdef DRI3
    rfbDRI3SyncDrawableToBO(pDrawable);
#endif
    SCHEDULE_FB_UPDATE(pDrawable->pScreen, prfb);
  }

  GC_OP_EPILOGUE(pGC);
}


/*
 * ImageText16 - use rough bounding box.
 */

static void rfbImageText16(DrawablePtr pDrawable, GCPtr pGC, int x, int y,
                           int count, unsigned short *chars)
{
  RegionRec tmpRegion;
  BoxRec box;

  GC_OP_PROLOGUE(pDrawable, pGC);

  TRC("rfbImageText16() Drawable=0x%.8x pGC=0x%.8lx x=%d y=%d count=%d\n",
      XID(pDrawable), (unsigned long)pGC, x, y, count);

  if (count) {
    GetTextBoundingBox(pDrawable, pGC->font, x, y, count, &box);

    SAFE_REGION_INIT(pDrawable->pScreen, &tmpRegion, &box, 0);

    REGION_INTERSECT(pDrawable->pScreen, &tmpRegion, &tmpRegion,
                     pGC->pCompositeClip);

    ADD_TO_MODIFIED_REGION(pDrawable->pScreen, &tmpRegion);

    TRC_REGION(pDrawable->pScreen, &tmpRegion, "rfbImageText16() mod");

    REGION_UNINIT(pDrawable->pScreen, &tmpRegion);
#ifdef DRI3
    rfbDRI3SyncBOToDrawable(pDrawable);
#endif
  }

  (*pGC->ops->ImageText16) (pDrawable, pGC, x, y, count, chars);

  if (count) {
#ifdef DRI3
    rfbDRI3SyncDrawableToBO(pDrawable);
#endif
    SCHEDULE_FB_UPDATE(pDrawable->pScreen, prfb);
  }

  GC_OP_EPILOGUE(pGC);
}


/*
 * ImageGlyphBlt - use rough bounding box.
 */

static void rfbImageGlyphBlt(DrawablePtr pDrawable, GCPtr pGC, int x, int y,
                             unsigned int nglyph,
                             CharInfoPtr *ppci,   /* array of character info */
                             pointer pglyphBase)  /* start of array of glyphs */
{
  RegionRec tmpRegion;
  BoxRec box;

  GC_OP_PROLOGUE(pDrawable, pGC);

  TRC("rfbImageGlyphBlt() Drawable=0x%.8x pGC=0x%.8lx x=%d y=%d nglyph=%d\n",
      XID(pDrawable), (unsigned long)pGC, x, y, nglyph);

  if (nglyph) {
    GetTextBoundingBox(pDrawable, pGC->font, x, y, nglyph, &box);

    SAFE_REGION_INIT(pDrawable->pScreen, &tmpRegion, &box, 0);

    REGION_INTERSECT(pDrawable->pScreen, &tmpRegion, &tmpRegion,
                     pGC->pCompositeClip);

    ADD_TO_MODIFIED_REGION(pDrawable->pScreen, &tmpRegion);

    TRC_REGION(pDrawable->pScreen, &tmpRegion, "rfbImageGlyphBlt() mod");

    REGION_UNINIT(pDrawable->pScreen, &tmpRegion);
#ifdef DRI3
    rfbDRI3SyncBOToDrawable(pDrawable);
#endif
  }

  (*pGC->ops->ImageGlyphBlt) (pDrawable, pGC, x, y, nglyph, ppci, pglyphBase);

  if (nglyph) {
#ifdef DRI3
    rfbDRI3SyncDrawableToBO(pDrawable);
#endif
    SCHEDULE_FB_UPDATE(pDrawable->pScreen, prfb);
  }

  GC_OP_EPILOGUE(pGC);
}


/*
 * PolyGlyphBlt - use rough bounding box.
 */

static void rfbPolyGlyphBlt(DrawablePtr pDrawable, GCPtr pGC, int x, int y,
                            unsigned int nglyph,
                            CharInfoPtr *ppci,   /* array of character info */
                            pointer pglyphBase)  /* start of array of glyphs */
{
  RegionRec tmpRegion;
  BoxRec box;

  GC_OP_PROLOGUE(pDrawable, pGC);

  TRC("rfbPolyGlyphBlt() Drawable=0x%.8x pGC=0x%.8lx x=%d y=%d nglyph=%d\n",
      XID(pDrawable), (unsigned long)pGC, x, y, nglyph);

  if (nglyph) {
    GetTextBoundingBox(pDrawable, pGC->font, x, y, nglyph, &box);

    SAFE_REGION_INIT(pDrawable->pScreen, &tmpRegion, &box, 0);

    REGION_INTERSECT(pDrawable->pScreen, &tmpRegion, &tmpRegion,
                     pGC->pCompositeClip);

    ADD_TO_MODIFIED_REGION(pDrawable->pScreen, &tmpRegion);

    TRC_REGION(pDrawable->pScreen, &tmpRegion, "rfbPolyGlyphBlt() mod");

    REGION_UNINIT(pDrawable->pScreen, &tmpRegion);
#ifdef DRI3
    rfbDRI3SyncBOToDrawable(pDrawable);
#endif
  }

  (*pGC->ops->PolyGlyphBlt) (pDrawable, pGC, x, y, nglyph, ppci, pglyphBase);

  if (nglyph) {
#ifdef DRI3
    rfbDRI3SyncDrawableToBO(pDrawable);
#endif
    SCHEDULE_FB_UPDATE(pDrawable->pScreen, prfb);
  }

  GC_OP_EPILOGUE(pGC);
}


/*
 * PushPixels - be fairly safe - region modified is intersection of the given
 * rectangle with the window clip region.
 */

static void rfbPushPixels(GCPtr pGC, PixmapPtr pBitMap, DrawablePtr pDrawable,
                          int w, int h, int x, int y)
{
  RegionRec tmpRegion;
  BoxRec box;

  GC_OP_PROLOGUE(pDrawable, pGC);

  TRC("rfbPushPixels() pGC=0x%.8lx Drawable=0x%.8x w=%d h=%d x=%d y=%d\n",
      (unsigned long)pGC, XID(pDrawable), w, h, x, y);

  box.x1 = x + pDrawable->x;
  box.y1 = y + pDrawable->y;
  box.x2 = box.x1 + w;
  box.y2 = box.y1 + h;

  SAFE_REGION_INIT(pDrawable->pScreen, &tmpRegion, &box, 0);

  REGION_INTERSECT(pDrawable->pScreen, &tmpRegion, &tmpRegion,
                   pGC->pCompositeClip);

  ADD_TO_MODIFIED_REGION(pDrawable->pScreen, &tmpRegion);

  TRC_REGION(pDrawable->pScreen, &tmpRegion, "rfbPushPixels() mod");

  REGION_UNINIT(pDrawable->pScreen, &tmpRegion);

#ifdef DRI3
  rfbDRI3SyncBOToDrawable(pDrawable);
#endif

  (*pGC->ops->PushPixels) (pGC, pBitMap, pDrawable, w, h, x, y);

#ifdef DRI3
  rfbDRI3SyncDrawableToBO(pDrawable);
#endif

  SCHEDULE_FB_UPDATE(pDrawable->pScreen, prfb);

  GC_OP_EPILOGUE(pGC);
}


#ifdef RENDER

void rfbComposite(CARD8 op, PicturePtr pSrc, PicturePtr pMask, PicturePtr pDst,
                  INT16 xSrc, INT16 ySrc, INT16 xMask, INT16 yMask,
                  INT16 xDst, INT16 yDst, CARD16 width, CARD16 height)
{
  ScreenPtr pScreen = pDst->pDrawable->pScreen;
  rfbFBInfoPtr prfb = &rfbFB;
  RegionRec tmpRegion, fbRegion;
  BoxRec box;
  PictureScreenPtr ps = GetPictureScreen(pScreen);

  TRC("rfbComposite() op=%d Src=0x%.8x Dst=0x%.8x xSrc=%d ySrc=%d xMask=0x%.4x yMask=0x%.4x xDst=%d yDst=%d width=%d height=%d\n",
      op, XID(pSrc->pDrawable), XID(pDst->pDrawable), xSrc, ySrc, xMask, yMask,
      xDst, yDst, width, height);

  if (is_visible(pDst->pDrawable)) {
    box.x1 = max(pDst->pDrawable->x + xDst, 0);
    box.y1 = max(pDst->pDrawable->y + yDst, 0);
    box.x2 = box.x1 + width;
    box.y2 = box.y1 + height;

    REGION_INIT(pScreen, &tmpRegion, &box, 0);

    box.x1 = 0;
    box.y1 = 0;
    box.x2 = pScreen->width;
    box.y2 = pScreen->height;

    REGION_INIT(pScreen, &fbRegion, &box, 0);

    REGION_INTERSECT(pScreen, &tmpRegion, &tmpRegion, &fbRegion);

    ADD_TO_MODIFIED_REGION(pScreen, &tmpRegion);

    TRC_REGION(pScreen, &tmpRegion, "rfbComposite() mod");
  }

#ifdef DRI3
  rfbDRI3SyncBOToDrawable(pDst->pDrawable);
#endif

  ps->Composite = prfb->Composite;
  (*ps->Composite) (op, pSrc, pMask, pDst, xSrc, ySrc, xMask, yMask,
                    xDst, yDst, width, height);
  ps->Composite = rfbComposite;

#ifdef DRI3
  rfbDRI3SyncDrawableToBO(pDst->pDrawable);
#endif

  if (is_visible(pDst->pDrawable)) {
    SCHEDULE_FB_UPDATE(pScreen, prfb);

    REGION_UNINIT(pScreen, &tmpRegion);
  }
}


static int GlyphCount(int nlist, GlyphListPtr list, GlyphPtr *glyphs)
{
  int count;

  count = 0;
  while (nlist--) {
    count += list->len;
    list++;
  }

  return count;
}


static RegionPtr GlyphsToRegion(ScreenPtr pScreen, int nlist,
                                GlyphListPtr list, GlyphPtr *glyphs)
{
  int n;
  GlyphPtr glyph;
  int x, y;

  int nrects = GlyphCount(nlist, list, glyphs);
  xRectangle rects[nrects];
  xRectanglePtr rect;

  x = 0;
  y = 0;
  rect = &rects[0];
  while (nlist--) {
    x += list->xOff;
    y += list->yOff;
    n = list->len;
    list++;
    while (n--) {
      glyph = *glyphs++;
      rect->x = x - glyph->info.x;
      rect->y = y - glyph->info.y;
      rect->width = glyph->info.width;
      rect->height = glyph->info.height;
      x += glyph->info.xOff;
      y += glyph->info.yOff;
      rect++;
    }
  }

  return RECTS_TO_REGION(pScreen, nrects, rects, CT_NONE);
}


/* Glyphs - Glyph-specific version of Composite (caches and whatnot) */

void rfbGlyphs(CARD8 op, PicturePtr pSrc, PicturePtr pDst,
               PictFormatPtr maskFormat, INT16 xSrc, INT16 ySrc, int nlists,
               GlyphListPtr lists, GlyphPtr *glyphs)
{
  ScreenPtr pScreen = pDst->pDrawable->pScreen;
  rfbFBInfoPtr prfb = &rfbFB;
  RegionRec *tmpRegion = NULL;
  PictureScreenPtr ps = GetPictureScreen(pScreen);

  TRC("rfbGlyphs() op=%d Src=0x%.8x Dst=0x%.8x xSrc=%d ySrc=%d nlists=%d\n",
      op, XID(pSrc->pDrawable), XID(pDst->pDrawable), xSrc, ySrc, nlists);

  if (is_visible(pDst->pDrawable)) {
    BoxRec fbBox;
    RegionRec fbRegion;

    tmpRegion = GlyphsToRegion(pScreen, nlists, lists, glyphs);
    REGION_TRANSLATE(pScreen, tmpRegion, pDst->pDrawable->x,
                     pDst->pDrawable->y);

    fbBox.x1 = 0;
    fbBox.y1 = 0;
    fbBox.x2 = rfbFB.width;
    fbBox.y2 = rfbFB.height;
    REGION_INIT(pScreen, &fbRegion, &fbBox, 0);

    REGION_INTERSECT(pScreen, tmpRegion, tmpRegion, &fbRegion);

    REGION_UNINIT(pScreen, &fbRegion);

    ADD_TO_MODIFIED_REGION(pScreen, tmpRegion);

    TRC_REGION(pScreen, tmpRegion, "rfbGlyphs() mod");

    REGION_DESTROY(pScreen, tmpRegion);
  }

#ifdef DRI3
  rfbDRI3SyncBOToDrawable(pDst->pDrawable);
#endif

  ps->Glyphs = prfb->Glyphs;
  (*ps->Glyphs) (op, pSrc, pDst, maskFormat, xSrc, ySrc, nlists, lists,
                 glyphs);
  ps->Glyphs = rfbGlyphs;

#ifdef DRI3
  rfbDRI3SyncDrawableToBO(pDst->pDrawable);
#endif

  if (is_visible(pDst->pDrawable))
    SCHEDULE_FB_UPDATE(pScreen, prfb);
}


#ifdef DRI3

void rfbCompositeRects(CARD8 op, PicturePtr pDst, xRenderColor *color,
                       int nRect, xRectangle *rects)
{
  ScreenPtr pScreen = pDst->pDrawable->pScreen;
  rfbFBInfoPtr prfb = &rfbFB;
  PictureScreenPtr ps = GetPictureScreen(pScreen);

  TRC("rfbCompositeRects() op=%d Dst=0x%.8x nRect=%d\n", op,
      XID(pDst->pDrawable), nRect);

  if (nRect)
    rfbDRI3SyncBOToDrawable(pDst->pDrawable);

  ps->CompositeRects = prfb->CompositeRects;
  (*ps->CompositeRects) (op, pDst, color, nRect, rects);
  ps->CompositeRects = rfbCompositeRects;

  if (nRect)
    rfbDRI3SyncDrawableToBO(pDst->pDrawable);
}


void rfbTrapezoids(CARD8 op, PicturePtr pSrc, PicturePtr pDst,
                   PictFormatPtr maskFormat, INT16 xSrc, INT16 ySrc, int ntrap,
                   xTrapezoid *traps)
{
  ScreenPtr pScreen = pDst->pDrawable->pScreen;
  rfbFBInfoPtr prfb = &rfbFB;
  PictureScreenPtr ps = GetPictureScreen(pScreen);

  TRC("rfbTrapezoids() op=%d Src=0x%.8x Dst=0x%.8x xSrc=%d ySrc=%d ntrap=%d\n",
      op, XID(pSrc->pDrawable), XID(pDst->pDrawable), xSrc, ySrc, ntrap);

  if (ntrap)
    rfbDRI3SyncBOToDrawable(pDst->pDrawable);

  ps->Trapezoids = prfb->Trapezoids;
  (*ps->Trapezoids) (op, pSrc, pDst, maskFormat, xSrc, ySrc, ntrap, traps);
  ps->Trapezoids = rfbTrapezoids;

  if (ntrap)
    rfbDRI3SyncDrawableToBO(pDst->pDrawable);
}


void rfbTriangles(CARD8 op, PicturePtr pSrc, PicturePtr pDst,
                  PictFormatPtr maskFormat, INT16 xSrc, INT16 ySrc, int ntri,
                  xTriangle *tris)
{
  ScreenPtr pScreen = pDst->pDrawable->pScreen;
  rfbFBInfoPtr prfb = &rfbFB;
  PictureScreenPtr ps = GetPictureScreen(pScreen);

  TRC("rfbTriangles() op=%d Src=0x%.8x Dst=0x%.8x xSrc=%d ySrc=%d ntri=%d\n",
      op, XID(pSrc->pDrawable), XID(pDst->pDrawable), xSrc, ySrc, ntri);

  if (ntri)
    rfbDRI3SyncBOToDrawable(pDst->pDrawable);

  ps->Triangles = prfb->Triangles;
  (*ps->Triangles) (op, pSrc, pDst, maskFormat, xSrc, ySrc, ntri, tris);
  ps->Triangles = rfbTriangles;

  if (ntri)
    rfbDRI3SyncDrawableToBO(pDst->pDrawable);
}


void rfbTriStrip(CARD8 op, PicturePtr pSrc, PicturePtr pDst,
                 PictFormatPtr maskFormat, INT16 xSrc, INT16 ySrc, int npoint,
                 xPointFixed *points)
{
  ScreenPtr pScreen = pDst->pDrawable->pScreen;
  rfbFBInfoPtr prfb = &rfbFB;
  PictureScreenPtr ps = GetPictureScreen(pScreen);

  TRC("rfbTriStrip() op=%d Src=0x%.8x Dst=0x%.8x xSrc=%d ySrc=%d npoint=%d\n",
      op, XID(pSrc->pDrawable), XID(pDst->pDrawable), xSrc, ySrc, npoint);

  if (npoint)
    rfbDRI3SyncBOToDrawable(pDst->pDrawable);

  ps->TriStrip = prfb->TriStrip;
  (*ps->TriStrip) (op, pSrc, pDst, maskFormat, xSrc, ySrc, npoint, points);
  ps->TriStrip = rfbTriStrip;

  if (npoint)
    rfbDRI3SyncDrawableToBO(pDst->pDrawable);
}


void rfbTriFan(CARD8 op, PicturePtr pSrc, PicturePtr pDst,
               PictFormatPtr maskFormat, INT16 xSrc, INT16 ySrc, int npoint,
               xPointFixed *points)
{
  ScreenPtr pScreen = pDst->pDrawable->pScreen;
  rfbFBInfoPtr prfb = &rfbFB;
  PictureScreenPtr ps = GetPictureScreen(pScreen);

  TRC("rfbTriFan() op=%d Src=0x%.8x Dst=0x%.8x xSrc=%d ySrc=%d npoint=%d\n",
      op, XID(pSrc->pDrawable), XID(pDst->pDrawable), xSrc, ySrc, npoint);

  if (npoint)
    rfbDRI3SyncBOToDrawable(pDst->pDrawable);

  ps->TriFan = prfb->TriFan;
  (*ps->TriFan) (op, pSrc, pDst, maskFormat, xSrc, ySrc, npoint, points);
  ps->TriFan = rfbTriFan;

  if (npoint)
    rfbDRI3SyncDrawableToBO(pDst->pDrawable);
}

#endif  /* DRI3 */

#endif  /* RENDER */


/****************************************************************************/
/*
 * Other functions
 */
/****************************************************************************/

/*
 * rfbCopyRegion.  Args are src and dst regions plus a translation (dx, dy).
 * Takes these args together with the existing modified region and possibly an
 * existing copy region and translation.  Produces a combined modified region
 * plus copy region and translation.  Note that the copy region is the
 * destination of the copy.
 *
 * First we trim parts of src which are invalid (ie in the modified region).
 * Then we see if there is any overlap between the src and the existing copy
 * region.  If not then the two copies cannot be combined, so we choose
 * whichever is bigger to form the basis of a new copy, while the other copy is
 * just done the hard way by being added to the modified region.  So if the
 * existing copy is bigger then we simply add the destination of the new copy
 * to the modified region and we're done.  If the new copy is bigger, we add
 * the old copy region to the modified region and behave as though there is no
 * existing copy region.
 *
 * At this stage we now know that either the two copies can be combined, or
 * that there is no existing copy.  We temporarily add both the existing copy
 * region and dst to the modified region (this is the entire area of the screen
 * affected in any way).  Finally we calculate the new copy region, and remove
 * it from the modified region.
 *
 * Note:
 *   1. The src region is modified by this routine.
 *   2. When the copy region is empty, copyDX and copyDY MUST be set to zero.
 */

static void rfbCopyRegion(ScreenPtr pScreen, rfbClientPtr cl, RegionPtr src,
                          RegionPtr dst, int dx, int dy)
{
  RegionRec tmp;

  TRC("%d: rfbCopyRegion() dx=%d dy=%d\n", cl->id, dx, dy);
  TRC_REGION(pScreen, src, "rfbCopyRegion() src");
  TRC_REGION(pScreen, dst, "rfbCopyRegion() dst");

  /* src = src - modifiedRegion */

  REGION_SUBTRACT(pScreen, src, src, &cl->modifiedRegion);

  if (REGION_NOTEMPTY(pScreen, &cl->copyRegion)) {

    REGION_INIT(pScreen, &tmp, NullBox, 0);
    REGION_INTERSECT(pScreen, &tmp, src, &cl->copyRegion);

    if (REGION_NOTEMPTY(pScreen, &tmp)) {

      /* if src and copyRegion overlap:
           src = src intersect copyRegion */

      REGION_COPY(pScreen, src, &tmp);

    } else {

      /* if no overlap, find bigger region */

      int newArea = (((REGION_EXTENTS(pScreen, src))->x2 -
                      (REGION_EXTENTS(pScreen, src))->x1) *
                     ((REGION_EXTENTS(pScreen, src))->y2 -
                      (REGION_EXTENTS(pScreen, src))->y1));

      int oldArea = (((REGION_EXTENTS(pScreen, &cl->copyRegion))->x2 -
                      (REGION_EXTENTS(pScreen, &cl->copyRegion))->x1) *
                     ((REGION_EXTENTS(pScreen, &cl->copyRegion))->y2 -
                      (REGION_EXTENTS(pScreen, &cl->copyRegion))->y1));

      if (oldArea > newArea) {

        /* existing copy is bigger:
             modifiedRegion = modifiedRegion union dst
             copyRegion = copyRegion - dst
             return */

        REGION_UNION(pScreen, &cl->modifiedRegion, &cl->modifiedRegion, dst);
        REGION_SUBTRACT(pScreen, &cl->copyRegion, &cl->copyRegion, dst);
        if (!REGION_NOTEMPTY(pScreen, &cl->copyRegion)) {
          cl->copyDX = 0;
          cl->copyDY = 0;
        }
        return;
      }

      /* new copy is bigger:
           modifiedRegion = modifiedRegion union copyRegion
           copyRegion = empty */

      REGION_UNION(pScreen, &cl->modifiedRegion, &cl->modifiedRegion,
                   &cl->copyRegion);
      REGION_EMPTY(pScreen, &cl->copyRegion);
      cl->copyDX = cl->copyDY = 0;
    }
  }

  /* modifiedRegion = modifiedRegion union dst union copyRegion */

  REGION_UNION(pScreen, &cl->modifiedRegion, &cl->modifiedRegion, dst);
  REGION_UNION(pScreen, &cl->modifiedRegion, &cl->modifiedRegion,
               &cl->copyRegion);

  /* copyRegion = T(src) intersect dst */

  REGION_TRANSLATE(pScreen, src, dx, dy);
  REGION_INTERSECT(pScreen, &cl->copyRegion, src, dst);

  /* modifiedRegion = modifiedRegion - copyRegion */

  REGION_SUBTRACT(pScreen, &cl->modifiedRegion, &cl->modifiedRegion,
                  &cl->copyRegion);

  /* combine new translation T with existing translation */

  if (REGION_NOTEMPTY(pScreen, &cl->copyRegion)) {
    cl->copyDX += dx;
    cl->copyDY += dy;
  } else {
    cl->copyDX = 0;
    cl->copyDY = 0;
  }
}


/*
 * rfbDeferredUpdateCallback() is called when a client's deferredUpdateTimer
 * goes off.
 */

static CARD32 rfbDeferredUpdateCallback(OsTimerPtr timer, CARD32 now,
                                        pointer arg)
{
  rfbClientPtr cl = (rfbClientPtr)arg;
  BOOL status = TRUE;

  if (cl->deferredUpdateScheduled && FB_UPDATE_PENDING(cl))
    status = rfbSendFramebufferUpdate(cl);

  if (status) cl->deferredUpdateScheduled = FALSE;
  return 0;
}


/*
 * rfbScheduleDeferredUpdate() is called from the SCHEDULE_FB_UPDATE macro
 * to schedule an update.
 */

static void rfbScheduleDeferredUpdate(rfbClientPtr cl)
{
  if (rfbDeferUpdateTime != 0) {
    cl->deferredUpdateTimer = TimerSet(cl->deferredUpdateTimer, 0,
                                       rfbDeferUpdateTime,
                                       rfbDeferredUpdateCallback, cl);
    cl->deferredUpdateScheduled = TRUE;
    cl->deferredUpdateStart = gettime();
  } else {
    rfbSendFramebufferUpdate(cl);
  }
}


/*
 * PrintRegion is useful for debugging.
 */

void PrintRegion(ScreenPtr pScreen, RegionPtr reg, const char *msg,
                 Bool printRects)
{
  int nrects = REGION_NUM_RECTS(reg);
  int i;

  rfbLog("%s: num rects %d extents %d,%d %d,%d\n", msg, nrects,
         (REGION_EXTENTS(pScreen, reg))->x1,
         (REGION_EXTENTS(pScreen, reg))->y1,
         (REGION_EXTENTS(pScreen, reg))->x2,
         (REGION_EXTENTS(pScreen, reg))->y2);

  if (!printRects) return;

  for (i = 0; i < nrects; i++) {
    rfbLog("    rect %d,%d %dx%d\n",
           REGION_RECTS(reg)[i].x1,
           REGION_RECTS(reg)[i].y1,
           REGION_RECTS(reg)[i].x2 - REGION_RECTS(reg)[i].x1,
           REGION_RECTS(reg)[i].y2 - REGION_RECTS(reg)[i].y1);
  }
}
