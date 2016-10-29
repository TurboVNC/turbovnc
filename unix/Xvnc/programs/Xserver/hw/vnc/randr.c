/*
 *  Copyright (C)2013-2016 D. R. Commander.  All Rights Reserved.
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
 *
 * Author:  Jim Gettys, HP Labs, Hewlett-Packard, Inc.
 */

#ifdef RANDR

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

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

static Res vncRRResolutions[] = {
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
  WindowPtr pWin = pScreen->root;
  WindowPtr pChild;
  Bool WasViewable = (Bool)(pWin->viewable);
  Bool anyMarked = FALSE;
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
    if (pWin->firstChild) {
      anyMarked |= (*pScreen->MarkOverlappedWindows)(pWin->firstChild,
                                                     pWin->firstChild,
                                                     (WindowPtr *)NULL);
    } else {
      (*pScreen->MarkWindow) (pWin);
      anyMarked = TRUE;
    }

    if (anyMarked) {
      (*pScreen->ValidateTree)(pWin, NullWindow, VTOther);
      (*pScreen->HandleExposures)(pWin);
      if (pScreen->PostValidateTree)
        (*pScreen->PostValidateTree)(pWin, NullWindow, VTOther);
    }
  }
  if (pWin->realized)
    WindowsRestructured();
  FlushAllOutput();
}


static int mm(int dimension)
{
  int dpi = 96;
  if (monitorResolution != 0) {
    dpi = monitorResolution;
  }
  return (dimension * 254 + dpi * 5) / (dpi * 10);
}


static Bool vncSetModes(ScreenPtr pScreen, int w, int h)
{
  Bool found = FALSE;
  int i;
  rrScrPrivPtr rp = rrGetScrPriv(pScreen);
  int numModes = sizeof(vncRRResolutions) / sizeof(Res);
  int preferred = 0;
  RRModePtr modes[numModes];

  if (vncRRResolutions[0].w < 0 && vncRRResolutions[0].h < 0) {
    vncRRResolutions[0].w = w;
    vncRRResolutions[0].h = h;
  }

  for (i = 0; i < sizeof(vncRRResolutions) / sizeof(Res); i++) {
    if (vncRRResolutions[i].w == w && vncRRResolutions[i].h == h) {
      found = TRUE;
      preferred = i;
      break;
    }
  }

  if (!found) {
    vncRRResolutions[0].w = w;
    vncRRResolutions[0].h = h;
    preferred = 0;
  }

  for (i = 0; i < numModes; i++) {
    int w = vncRRResolutions[i].w, h = vncRRResolutions[i].h;
    xRRModeInfo	modeInfo;
    char name[100];

    memset(&modeInfo, 0, sizeof(modeInfo));
    snprintf(name, 100, "%dx%d", w, h);

    modeInfo.width = w;
    modeInfo.height = h;
    modeInfo.hTotal = w;
    modeInfo.vTotal = h;
    modeInfo.dotClock = ((CARD32)w * (CARD32)h * 60);
    modeInfo.nameLength = strlen(name);
    if ((modes[i] = RRModeGet(&modeInfo, name)) == NULL) return FALSE;
  }

  if (rp->numOutputs < 1) return FALSE;
  for (i = 0; i < rp->numOutputs; i++) {
    if (rp->outputs[i] == NULL) return FALSE;
    if (!RROutputSetModes(rp->outputs[i], modes, numModes, 1))
      return FALSE;
  }

  if (rp->numCrtcs < 1) return FALSE;
  for (i = 0; i < rp->numCrtcs; i++) {
    if (rp->crtcs[i] == NULL) return FALSE;
    if (!RRCrtcNotify(rp->crtcs[i], modes[preferred], 0, 0, RR_Rotate_0, NULL,
                      rp->numOutputs, rp->outputs))
      return FALSE;
  }

  return TRUE;
}


static Bool vncRRGetInfo(ScreenPtr pScreen, Rotation *rotations)
{
  return TRUE;
}


/* Called by X server */

static int vncScreenSetSize(ScreenPtr pScreen, CARD16 width, CARD16 height,
                            CARD32 mmWidth, CARD32 mmHeight)
{
  rfbClientPtr cl;
  rfbScreenInfo newScreen = rfbScreen;
  PixmapPtr rootPixmap = pScreen->GetScreenPixmap(pScreen);
  int ret = rfbEDSResultSuccess;

  if ((width > rfbMaxWidth && rfbMaxWidth > 0) ||
      (height > rfbMaxHeight && rfbMaxHeight > 0)) {
    width = min(width, rfbMaxWidth);
    height = min(height, rfbMaxHeight);
    rfbLog("NOTICE: desktop size clamped to %dx%d per system policy\n", width,
           height);
  }

  for (cl = rfbClientHead; cl; cl = cl->next) {
    if (!cl->enableDesktopSize && !cl->enableExtDesktopSize) {
      rfbLog("ERROR: Not resizing desktop because one or more clients doesn't support it.\n");
      vncSetModes(pScreen, pScreen->width, pScreen->height);
      return rfbEDSResultProhibited;
    }
  }

  newScreen.width = width;
  newScreen.height = height;
  newScreen.paddedWidthInBytes = PixmapBytePad(newScreen.width,
                                               newScreen.depth);
  newScreen.pfbMemory = NULL;
  if (!rfbAllocateFramebufferMemory(&newScreen)) {
    rfbLog("ERROR: Could not allocate framebuffer memory\n");
    return rfbEDSResultNoResources;
  }

  rfbScreen.blockUpdates = TRUE;
  xf86SetRootClip(pScreen, FALSE);

  if (!pScreen->ModifyPixmapHeader(rootPixmap, newScreen.width,
                                   newScreen.height, newScreen.depth,
                                   newScreen.bitsPerPixel,
                                   newScreen.paddedWidthInBytes,
                                   newScreen.pfbMemory)) {
    rfbLog("ERROR: Could not modify root pixmap size\n");
    free(newScreen.pfbMemory);
    xf86SetRootClip(pScreen, TRUE);
    rfbScreen.blockUpdates = FALSE;
    return rfbEDSResultInvalid;
  }
  free(rfbScreen.pfbMemory);
  rfbScreen = newScreen;
  pScreen->width = width;
  pScreen->height = height;
  pScreen->mmWidth = mmWidth;
  pScreen->mmHeight = mmHeight;

  xf86SetRootClip(pScreen, TRUE);

  RRScreenSizeNotify(pScreen);
  update_desktop_dimensions();

  if (!vncSetModes(pScreen, width, height)) {
    rfbLog("ERROR: Could not set screen modes\n");
    rfbScreen.blockUpdates = FALSE;
    return rfbEDSResultInvalid;
  }

  rfbLog("New desktop size: %d x %d\n", pScreen->width, pScreen->height);

  rfbScreen.blockUpdates = FALSE;

  for (cl = rfbClientHead; cl; cl = cl->next) {
    RegionRec tmpRegion;  BoxRec box;
    Bool reEnableInterframe = (cl->compareFB != NULL);
    InterframeOff(cl);
    if (reEnableInterframe) {
      if (!InterframeOn(cl)) {
        rfbCloseClient(cl);
        ret = rfbEDSResultInvalid;
        continue;
      }
    }
    cl->deferredUpdateScheduled = FALSE;
    /* Reset all of the regions, so the next FBU will behave as if it
       was the first. */
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


static Bool rfbSendDesktopSizeAll(rfbClientPtr reqClient, int reason)
{
  rfbClientPtr cl;

  for (cl = rfbClientHead; cl; cl = cl->next) {
    cl->pendingDesktopResize = TRUE;
    if (cl != reqClient && reason == rfbEDSReasonClient)
      cl->reason = rfbEDSReasonOtherClient;
    else
      cl->reason = reason;
    cl->result = rfbEDSResultSuccess;
    if (!rfbSendFramebufferUpdate(cl))
      return FALSE;
  }

  return TRUE;
}


static Bool vncRRCrtcSet(ScreenPtr pScreen, RRCrtcPtr crtc, RRModePtr mode,
                         int x, int y, Rotation rotation, int numOutputs,
                         RROutputPtr *outputs)
{
  return TRUE;
}


static Bool vncRRScreenSetSize(ScreenPtr pScreen, CARD16 width, CARD16 height,
                               CARD32 mmWidth, CARD32 mmHeight)
{
  int result = vncScreenSetSize(pScreen, width, height, mmWidth, mmHeight);
  if (result == rfbEDSResultSuccess)
    return rfbSendDesktopSizeAll(NULL, rfbEDSReasonServer);
  else
    return FALSE;
}


Bool vncRRInit(ScreenPtr pScreen)
{
  rrScrPrivPtr rp;
  RRCrtcPtr crtc;
  RROutputPtr output;

  if (!RRScreenInit(pScreen)) return FALSE;

  rp = rrGetScrPriv(pScreen);

  rp->rrGetInfo = vncRRGetInfo;
  rp->rrCrtcSet = vncRRCrtcSet;
  rp->rrScreenSetSize = vncRRScreenSetSize;

  RRScreenSetSizeRange(pScreen, 32, 32, 32768, 32768);
  if ((crtc = RRCrtcCreate(pScreen, NULL)) == NULL) return FALSE;
  RRCrtcGammaSetSize(crtc, 256);
  if ((output = RROutputCreate(pScreen, "TurboVNC", 8, NULL)) == NULL)
    return FALSE;
  RROutputSetCrtcs(output, &crtc, 1);
  RROutputSetConnection(output, RR_Connected);

  if (!vncSetModes(pScreen, pScreen->width, pScreen->height))
    return FALSE;

  return TRUE;
}


/* Called by VNC */

Bool ResizeDesktop(ScreenPtr pScreen, rfbClientPtr cl, int w, int h)
{
  rfbClientPtr cl2;
  int result = rfbEDSResultSuccess;
  rrScrPrivPtr rp = rrGetScrPriv(pScreen);

  if (w < 1 || h < 1) {
    rfbLog("ERROR: Not resizing desktop because requested dimensions %d x %d are invalid.\n",
           w, h);
    result = rfbEDSResultInvalid;
    goto error;
  }

  if (cl->viewOnly) {
    rfbLog("NOTICE: Ignoring remote desktop resize request from a view-only client.\n");
    result = rfbEDSResultProhibited;
    goto error;
  }

  if (pScreen->width != w || pScreen->height != h) {
    result = vncScreenSetSize(pScreen, w, h, mm(w), mm(h));
    /* We have to do this to simulate an RandR-generated screen resize.
       Otherwise the RandR events may not be delivered properly to X
       clients. */
    if (result == rfbEDSResultSuccess)
      rp->lastSetTime = currentTime;
  }

  if (result == rfbEDSResultSuccess)
    return rfbSendDesktopSizeAll(cl, rfbEDSReasonClient);

  error:
  /* Send back the error only to the requesting client.  This loop is
     necessary because the client may have been shut down as a result of an
     error in vncScreenSetSize(). */
  for (cl2 = rfbClientHead; cl2; cl2 = cl2->next) {
    if (cl2 == cl) {
      cl2->pendingDesktopResize = TRUE;
      cl2->reason = rfbEDSReasonClient;
      cl2->result = result;
      rfbSendFramebufferUpdate(cl2);
      break;
    }
  }
  return FALSE;
}

#endif
