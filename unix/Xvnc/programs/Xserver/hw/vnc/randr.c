/*
 *  Copyright (C)2013, D. R. Commander.  All Rights Reserved.
 *
 *  This is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this software; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 *  USA.
 *
 * Copyright (c) 1997-2003 by The XFree86 Project, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the copyright holder(s)
 * and author(s) shall not be used in advertising or otherwise to promote
 * the sale, use or other dealings in this Software without prior written
 * authorization from the copyright holder(s) and author(s).
 *
 * Copyright © 2000, Compaq Computer Corporation,
 * Copyright © 2002, Hewlett Packard, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Compaq or HP not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  HP makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * HP DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL HP
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef RANDR

#include "windowstr.h"
#include "randrstr.h"
#include "fb.h"
#include "servermd.h"
#include "rfb.h"
#include "mivalidate.h"


typedef struct _Res
{
  int w, h;
} Res;

Res vncRRResolutions[] = {
  {  -1,   -1},  /* Original resolution of the VNC server          */
  {3200, 1800},  /* WQXGA+  16:9                                   */
  {2880, 1800},  /*          8:5 (Mac)                             */
  {2560, 1600},  /* WQXGA    8:5                                   */
  {2560, 1440},  /* QHD     16:9                                   */
  {2048, 1536},  /* QXGA     4:3 (iPad)                            */
  {2048, 1152},  /* QWXGA   16:9                                   */
  {1920, 1200},  /* WUXGA    8:5                                   */
  {1920, 1080},  /* FHD     16:9                                   */
  {1680, 1050},  /* WSXGA+   8:5                                   */
  {1600, 1200},  /* UXGA     4:3                                   */
  {1600, 1000},  /*          8:5 (Mac)                             */
  {1600,  900},  /* HD+     16:9                                   */
  {1440,  900},  /* WXGA+    8:5                                   */
  {1400, 1050},  /* SXGA+    4:3                                   */
  {1366,  768},  /* WXGA    16:9                                   */
  {1360,  768},  /* WXGA    16:9                                   */
  {1344, 1008},  /*          4:3 (Mac)                             */
  {1344,  840},  /*          8:5 (Mac)                             */
  {1280, 1024},  /* SXGA     5:4                                   */
  {1280,  960},  /* SXGA-    4:3                                   */
  {1280,  800},  /* WXGA     8:5                                   */
  {1280,  720},  /* HD      16:9                                   */
  {1152,  864},  /* XGA+     4:3                                   */
  {1136,  640},  /*         16:9 (iPhone 5)                        */
  {1024,  768},  /* XGA      4:3                                   */
  {1024,  640},  /*          8:5 (laptops)                         */
  { 960,  640},  /* DVGA     3:2 (iPhone 4S)                       */
  { 960,  600},  /*          8:5 (Mac)                             */
  { 960,  540},  /* qHD     16:9                                   */
  { 854,  480},  /* FWVGA   16:9 (mobile devices)                  */
  { 800,  600},  /* SVGA     4:3                                   */
  { 800,  480},  /* WVGA     5:3 (mobile devices)                  */
  { 640,  480},  /* VGA      4:3                                   */
  { 640,  360},  /* nHD     16:9 (mobile devices)                  */
  { 480,  320},  /* HVGA     3:2 (iPhone and other mobile devices) */
};

extern int monitorResolution;
extern char *rfbAllocateFramebufferMemory(rfbScreenInfoPtr);
extern Bool InterframeOn(rfbClientPtr cl);
extern void InterframeOff(rfbClientPtr);


/*
 * xf86SetRootClip --
 *        Enable or disable rendering to the screen by
 *        setting the root clip list and revalidating
 *        all of the windows
 */

static void xf86SetRootClip (ScreenPtr pScreen, Bool enable)
{
  WindowPtr pWin = WindowTable[pScreen->myNum];
  WindowPtr pChild;
  Bool WasViewable = (Bool)(pWin->viewable);
  Bool anyMarked = FALSE;
  RegionPtr pOldClip = NULL, bsExposed;
#ifdef DO_SAVE_UNDERS
  Bool dosave = FALSE;
#endif
  WindowPtr pLayerWin;
  BoxRec box;

  if (WasViewable) {
    for (pChild = pWin->firstChild; pChild; pChild = pChild->nextSib) {
      (void) (*pScreen->MarkOverlappedWindows)(pChild, pChild, &pLayerWin);
    }
    (*pScreen->MarkWindow)(pWin);
    anyMarked = TRUE;
    if (pWin->valdata) {
      if (HasBorder (pWin)) {
        RegionPtr borderVisible;

        borderVisible = REGION_CREATE(pScreen, NullBox, 1);
        REGION_SUBTRACT(pScreen, borderVisible, &pWin->borderClip,
                        &pWin->winSize);
        pWin->valdata->before.borderVisible = borderVisible;
      }
      pWin->valdata->before.resized = TRUE;
    }
  }

  /*
   * Use REGION_BREAK to avoid optimizations in ValidateTree
   * that assume the root borderClip can't change well, normally
   * it doesn't...)
   */
  if (enable) {
    box.x1 = 0;
    box.y1 = 0;
    box.x2 = pScreen->width;
    box.y2 = pScreen->height;
    REGION_INIT (pScreen, &pWin->winSize, &box, 1);
    REGION_INIT (pScreen, &pWin->borderSize, &box, 1);
    if (WasViewable)
      REGION_RESET(pScreen, &pWin->borderClip, &box);
    pWin->drawable.width = pScreen->width;
    pWin->drawable.height = pScreen->height;
    REGION_BREAK (pWin->drawable.pScreen, &pWin->clipList);
  } else {
    REGION_EMPTY(pScreen, &pWin->borderClip);
    REGION_BREAK (pWin->drawable.pScreen, &pWin->clipList);
  }

  ResizeChildrenWinSize (pWin, 0, 0, 0, 0);

  if (WasViewable) {
    if (pWin->backStorage) {
      pOldClip = REGION_CREATE(pScreen, NullBox, 1);
      REGION_COPY(pScreen, pOldClip, &pWin->clipList);
    }

    if (pWin->firstChild) {
      anyMarked |= (*pScreen->MarkOverlappedWindows)(pWin->firstChild,
                                                     pWin->firstChild,
                                                     (WindowPtr *)NULL);
    } else {
      (*pScreen->MarkWindow) (pWin);
      anyMarked = TRUE;
    }

#ifdef DO_SAVE_UNDERS
    if (DO_SAVE_UNDERS(pWin)) {
      dosave = (*pScreen->ChangeSaveUnder)(pLayerWin, pLayerWin);
    }
#endif /* DO_SAVE_UNDERS */

    if (anyMarked)
      (*pScreen->ValidateTree)(pWin, NullWindow, VTOther);
  }

  if (pWin->backStorage && ((pWin->backingStore == Always) || WasViewable)) {
    if (!WasViewable)
      pOldClip = &pWin->clipList; /* a convenient empty region */
    bsExposed = (*pScreen->TranslateBackingStore)(pWin, 0, 0, pOldClip,
                                                  pWin->drawable.x,
                                                  pWin->drawable.y);
    if (WasViewable)
      REGION_DESTROY(pScreen, pOldClip);
    if (bsExposed) {
      RegionPtr valExposed = NullRegion;

      if (pWin->valdata)
        valExposed = &pWin->valdata->after.exposed;
      (*pScreen->WindowExposures)(pWin, valExposed, bsExposed);
      if (valExposed)
        REGION_EMPTY(pScreen, valExposed);
      REGION_DESTROY(pScreen, bsExposed);
    }
  }
  if (WasViewable) {
    if (anyMarked)
      (*pScreen->HandleExposures)(pWin);
#ifdef DO_SAVE_UNDERS
    if (dosave)
      (*pScreen->PostChangeSaveUnder)(pLayerWin, pLayerWin);
#endif /* DO_SAVE_UNDERS */
    if (anyMarked && pScreen->PostValidateTree)
      (*pScreen->PostValidateTree)(pWin, NullWindow, VTOther);
  }
  if (pWin->realized)
    WindowsRestructured();
  FlushAllOutput();
}


int mm(int dimension)
{
  int dpi = 75;
  if (monitorResolution != 0) {
    dpi = monitorResolution;
  }
  return (dimension * 254 + dpi * 5) / (dpi * 10);
}


Bool vncRRGetInfo(ScreenPtr pScreen, Rotation *rotations)
{
  int i;
  Bool setConfig = FALSE;
  RRScreenSizePtr pSize;

  *rotations = RR_Rotate_0;
  for (i = 0; i < pScreen->numDepths; i++) {
    if (pScreen->allowedDepths[i].numVids) {
      if (vncRRResolutions[0].w < 0 && vncRRResolutions[0].h < 0) {
        vncRRResolutions[0].w = pScreen->width;
        vncRRResolutions[0].h = pScreen->height;
      }
      pSize = RRRegisterSize(pScreen, pScreen->width, pScreen->height,
                             pScreen->mmWidth, pScreen->mmHeight);
      if (!pSize) return FALSE;
      RRRegisterRate (pScreen, pSize, 60);
      if (!setConfig) {
        RRSetCurrentConfig (pScreen, RR_Rotate_0, 0, pSize);
        setConfig = TRUE;
      }
    }
  }

  for (i = 0; i < sizeof(vncRRResolutions) / sizeof(Res); i++) {
    int w = vncRRResolutions[i].w, h = vncRRResolutions[i].h;
    pSize = RRRegisterSize(pScreen, w, h, mm(w), mm(h));
    if (!pSize) return FALSE;
    RRRegisterRate (pScreen, pSize, 60);
  }

  return TRUE;
}


Bool vncRRSetConfig(ScreenPtr pScreen, Rotation rotation, int rate,
                    RRScreenSizePtr pSize)
{
  rfbClientPtr cl;
  rfbScreenInfo newScreen = rfbScreen;
  PixmapPtr rootPixmap = fbGetScreenPixmap(pScreen);
  Bool ret = TRUE;

  newScreen.width = pSize->width;
  newScreen.height = pSize->height;
  newScreen.paddedWidthInBytes = PixmapBytePad(newScreen.width,
                                               newScreen.depth);
  newScreen.pfbMemory = NULL;
  if (!rfbAllocateFramebufferMemory(&newScreen)) return FALSE;

  xf86SetRootClip(pScreen, FALSE);
  Xfree(rfbScreen.pfbMemory);
  rfbScreen = newScreen;
  pScreen->width = pSize->width;
  pScreen->height = pSize->height;
  pScreen->mmWidth = pSize->mmWidth;
  pScreen->mmHeight = pSize->mmHeight;
  ret = pScreen->ModifyPixmapHeader(rootPixmap, rfbScreen.width,
                                    rfbScreen.height, rfbScreen.depth,
                                    rfbScreen.bitsPerPixel,
                                    rfbScreen.paddedWidthInBytes,
                                    rfbScreen.pfbMemory);
  xf86SetRootClip(pScreen, TRUE);

  rfbLog("New desktop size: %d x %d\n", pScreen->width, pScreen->height);

  for (cl = rfbClientHead; cl; cl = cl->next) {
    RegionRec tmpRegion;  BoxRec box;
    Bool reEnableInterframe = (cl->compareFB != NULL);
    InterframeOff(cl);
    if (reEnableInterframe) {
      if (!InterframeOn(cl)) {
        rfbCloseSock(cl->sock);
        ret = FALSE;
      }
    }
    cl->pendingDesktopResize = TRUE;
    cl->reason = rfbEDSReasonServer;
    // Reset all of the regions, so the next FBU will behave as if it
    // was the first.
    box.x1 = box.y1 = 0;
    box.x2 = pScreen->width;  box.y2 = pScreen->height;
    SAFE_REGION_INIT(pScreen, &tmpRegion, &box, 0);
    REGION_EMPTY(pScreen, &cl->modifiedRegion);
    REGION_UNION(pScreen, &cl->modifiedRegion, &cl->modifiedRegion,
                 &tmpRegion);
    REGION_EMPTY(pScreen, &cl->copyRegion);
    REGION_EMPTY(pScreen, &cl->ifRegion);
    REGION_UNION(pScreen, &cl->ifRegion, &cl->ifRegion, &tmpRegion);
    if (rfbAutoLosslessRefresh > 0.0) {
        REGION_EMPTY(pScreen, &cl->alrRegion);
        REGION_EMPTY(pScreen, &cl->alrEligibleRegion);
        REGION_EMPTY(pScreen, &cl->lossyRegion);
        cl->firstUpdate = TRUE;
    }
    if (cl->continuousUpdates) {
        REGION_EMPTY(pScreen, &cl->cuRegion);
        REGION_UNION(pScreen, &cl->cuRegion, &cl->cuRegion, &tmpRegion);
    } else {
        REGION_EMPTY(pScreen, &cl->requestedRegion);
        REGION_UNION(pScreen, &cl->requestedRegion, &cl->requestedRegion,
                     &tmpRegion);
    }
    REGION_UNINIT(pScreen, &tmpRegion);
  }

  return ret;
}


Bool vncRRInit(ScreenPtr pScreen)
{
  rrScrPrivPtr rp;

  if (!RRScreenInit(pScreen)) return FALSE;

  rp = rrGetScrPriv(pScreen);
  rp->rrGetInfo = vncRRGetInfo;
  rp->rrSetConfig = vncRRSetConfig;
  return TRUE;
}


Bool ResizeDesktop(ScreenPtr pScreen, int w, int h)
{
  int i;  Bool found = FALSE, setConfig = FALSE;
  RRScreenSizePtr pSize = NULL;

  for (i = 0; i < sizeof(vncRRResolutions) / sizeof(Res); i++) {
    if (vncRRResolutions[i].w == w && vncRRResolutions[i].h == h) {
      found = TRUE;
      break;
    }
  }
  for (i = 0; i < pScreen->numDepths; i++) {
    if (pScreen->allowedDepths[i].numVids) {
      if (!found) {
        int w0 = vncRRResolutions[0].w, h0 = vncRRResolutions[0].h;
        // Resolution 0 always represents the "custom" VNC resolution
        pSize = RRRegisterSize(pScreen, w0, h0, mm(w0), mm(h0));
        if (!pSize) return FALSE;
        pSize->width = vncRRResolutions[0].w = w;
        pSize->height = vncRRResolutions[0].h = h;
        pSize->mmWidth = mm(w);
        pSize->mmHeight = mm(h);
      } else {
        pSize = RRRegisterSize(pScreen, w, h, mm(w), mm(h));
        if (!pSize) return FALSE;
      }
      RRRegisterRate (pScreen, pSize, 60);

      if (!setConfig) {
        RRSetCurrentConfig (pScreen, RR_Rotate_0, 0, pSize);
        setConfig = TRUE;
      }
    }
  }

  if (RRSetScreenConfig(pScreen, RR_Rotate_0, 0, pSize) != Success)
    return FALSE;

  return TRUE;
}

#endif
