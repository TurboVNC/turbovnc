/*
 *  Copyright (C)2013-2017 D. R. Commander.  All Rights Reserved.
 *  Copyright 2012-2015 Pierre Ossman for Cendio AB
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
extern char *rfbAllocateFramebufferMemory(rfbFBInfoPtr);
extern Bool InterframeOn(rfbClientPtr cl);
extern void InterframeOff(rfbClientPtr);

static Bool vncCrtcSet(RRCrtcPtr crtc, RRModePtr mode, int x, int y,
                       Rotation rotation, int numOutputs,
                       RROutputPtr *outputs);
static RRModePtr vncSetModes(rfbScreenInfo *screen);


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


static Bool vncCreateOutput(ScreenPtr pScreen, rfbScreenInfo *screen)
{
  char name[80];
  RRCrtcPtr crtc;
  RRModePtr mode;
  static int index = 0;

  if ((crtc = RRCrtcCreate(pScreen, NULL)) == NULL) return FALSE;
  RRCrtcGammaSetSize(crtc, 256);
  snprintf(name, 80, "TurboVNC-%d", index++);
  if (!(screen->output = RROutputCreate(pScreen, name, strlen(name), NULL)))
    return FALSE;
  RROutputSetCrtcs(screen->output, &crtc, 1);
  RROutputSetConnection(screen->output, RR_Connected);

  if (!(mode = vncSetModes(screen))) return FALSE;

  if (!screen->idAssigned) {
    screen->s.id = screen->output->id;
    screen->idAssigned = TRUE;
  }

  return vncCrtcSet(crtc, mode, screen->s.x, screen->s.y, RR_Rotate_0, 1,
                    &screen->output);
}


static Bool vncCrtcSet(RRCrtcPtr crtc, RRModePtr mode, int x, int y,
                       Rotation rotation, int numOutputs, RROutputPtr *outputs)
{
  int i;

  /*
   * Some applications get confused by a connected output without a
   * mode or CRTC, so we need to fiddle with the connection state as well.
   */
  for (i = 0; i < crtc->numOutputs; i++)
    RROutputSetConnection(crtc->outputs[i], RR_Disconnected);

  for (i = 0; i < numOutputs; i++)
    RROutputSetConnection(outputs[i], mode ? RR_Connected : RR_Disconnected);

  return RRCrtcNotify(crtc, mode, x, y, rotation, NULL, numOutputs, outputs);
}


static Bool vncDisableOutput(RROutputPtr output)
{
  RRCrtcPtr crtc = output->crtc;

  if (crtc == NULL) return TRUE;

  return RRCrtcSet(crtc, NULL, crtc->x, crtc->y, crtc->rotation, 0, NULL);
}


static Bool vncHasOutputClones(ScreenPtr pScreen)
{
  rrScrPrivPtr rp = rrGetScrPriv(pScreen);
  int i;

  for (i = 0; i < rp->numCrtcs; i++)
    if (rp->crtcs[i]->numOutputs > 1) return TRUE;

  return FALSE;
}


static RRModePtr vncModeGet(int width, int height)
{
  xRRModeInfo modeInfo;
  char name[100];

  memset(&modeInfo, 0, sizeof(modeInfo));
  sprintf(name, "%dx%d", width, height);

  modeInfo.width = width;
  modeInfo.height = height;
  modeInfo.hTotal = width;
  modeInfo.vTotal = height;
  modeInfo.dotClock = ((CARD32)width * (CARD32)height * 60);
  modeInfo.nameLength = strlen(name);

  return RRModeGet(&modeInfo, name);
}


static void vncPrintScreenLayout(struct xorg_list *list)
{
  rfbScreenInfo *screen;

  if (!list) return;

  rfbLog("New screen layout:\n");
  xorg_list_for_each_entry(screen, list, entry) {
    if (screen->output->crtc && screen->output->crtc->mode)
      rfbLog("  0x%.8x (output 0x%.8x): %dx%d+%d+%d\n", screen->s.id,
             screen->output->id, screen->s.w, screen->s.h, screen->s.x,
             screen->s.y);
    else
      rfbLog("  0x%.8x (output 0x%.8x): DISABLED\n", screen->s.id,
             screen->output->id);
  }
}


static Bool vncReconfigureOutput(rfbScreenInfo *screen)
{
  RRCrtcPtr crtc = screen->output->crtc;
  RRModePtr mode;
  int i;

  if (crtc == NULL) {
    for (i = 0; i < screen->output->numCrtcs; i++) {
      if (screen->output->crtcs[i]->numOutputs != 0)
        continue;
      crtc = screen->output->crtcs[i];
      break;
    }
    if (crtc == NULL) return FALSE;
  }

  if(!(mode = vncSetModes(screen))) return FALSE;

  return vncCrtcSet(crtc, mode, screen->s.x, screen->s.y, RR_Rotate_0, 1,
                    &screen->output);
}


static RRModePtr vncSetModes(rfbScreenInfo *screen)
{
  Bool found = FALSE;
  int i;
  int numModes = sizeof(vncRRResolutions) / sizeof(Res);
  int preferred = 0;
  RRModePtr modes[numModes];
  Res resolutions[numModes];

  memcpy(resolutions, vncRRResolutions, sizeof(Res) * numModes);
  resolutions[0].w = screen->prefRes.w;
  resolutions[0].h = screen->prefRes.h;

  if (screen->prefRes.w < 1 || screen->prefRes.h < 1) {
    resolutions[0].w = screen->prefRes.w = screen->s.w;
    resolutions[0].h = screen->prefRes.h = screen->s.h;
  }

  for (i = 0; i < numModes; i++) {
    if (resolutions[i].w == screen->s.w && resolutions[i].h == screen->s.h) {
      found = TRUE;
      preferred = i;
      break;
    }
  }

  if (!found) {
    resolutions[0].w = screen->prefRes.w = screen->s.w;
    resolutions[0].h = screen->prefRes.h = screen->s.h;
    preferred = 0;
  }

  for (i = 0; i < numModes; i++) {
    if (!(modes[i] = vncModeGet(resolutions[i].w, resolutions[i].h)))
      return FALSE;
  }

  if (!RROutputSetModes(screen->output, modes, numModes, 1)) return NULL;

  return modes[preferred];
}


static int vncScreenSetSize(ScreenPtr pScreen, CARD16 width, CARD16 height,
                            CARD32 mmWidth, CARD32 mmHeight)
{
  rrScrPrivPtr rp = rrGetScrPriv(pScreen);
  rfbClientPtr cl;
  rfbFBInfo newFB = rfbFB;
  PixmapPtr rootPixmap = pScreen->GetScreenPixmap(pScreen);
  int ret = rfbEDSResultSuccess, i;

  if (width > rfbMaxWidth || height > rfbMaxHeight) {
    width = min(width, rfbMaxWidth);
    height = min(height, rfbMaxHeight);
    rfbLog("NOTICE: desktop size clamped to %dx%d per system policy\n", width,
           height);
  }

  newFB.width = width;
  newFB.height = height;
  newFB.paddedWidthInBytes = PixmapBytePad(newFB.width, newFB.depth);
  newFB.pfbMemory = NULL;
  if (!rfbAllocateFramebufferMemory(&newFB)) {
    rfbLog("ERROR: Could not allocate framebuffer memory\n");
    return rfbEDSResultNoResources;
  }

  rfbFB.blockUpdates = newFB.blockUpdates = TRUE;
  xf86SetRootClip(pScreen, FALSE);

  if (!pScreen->ModifyPixmapHeader(rootPixmap, newFB.width, newFB.height,
                                   newFB.depth, newFB.bitsPerPixel,
                                   newFB.paddedWidthInBytes,
                                   newFB.pfbMemory)) {
    rfbLog("ERROR: Could not modify root pixmap size\n");
    free(newFB.pfbMemory);
    xf86SetRootClip(pScreen, TRUE);
    rfbFB.blockUpdates = FALSE;
    return rfbEDSResultInvalid;
  }
  free(rfbFB.pfbMemory);
  rfbFB = newFB;
  pScreen->width = width;
  pScreen->height = height;
  pScreen->mmWidth = mmWidth;
  pScreen->mmHeight = mmHeight;

  xf86SetRootClip(pScreen, TRUE);

  RRScreenSizeNotify(pScreen);
  update_desktop_dimensions();

  rfbLog("New desktop size: %d x %d\n", pScreen->width, pScreen->height);

  /* Crop all CRTCs to the new screen dimensions */
  for (i = 0; i < rp->numCrtcs; i++) {
    RRCrtcPtr crtc = rp->crtcs[i];
    RRModePtr mode = NULL;
    int j;

    /* Disabled? */
    if (crtc->mode == NULL)
      continue;

    /* Fully inside? */
    if ((crtc->x + crtc->mode->mode.width <= width) &&
        (crtc->y + crtc->mode->mode.height <= height))
      continue;

    /* Fully outside? */
    if ((crtc->x >= width) || (crtc->y >= height)) {
      /* Disable it */
      if (!vncCrtcSet(crtc, NULL, crtc->x, crtc->y, crtc->rotation, 0, NULL))
        rfbLog("WARNING: Could not disable CRTC that is outside of new screen dimensions\n");
      continue;
    }

    /* Just needs to be resized to a new mode */
    for (j = 0; j < crtc->numOutputs; j++) {
      rfbScreenInfo screen;

      screen.output = crtc->outputs[j];
      screen.s.w = min(crtc->mode->mode.width, width - crtc->x);
      screen.s.h = min(crtc->mode->mode.height, height - crtc->y);
      if (!(mode = vncSetModes(&screen))) {
        rfbLog("WARNING: Could not create custom mode for %dx%d\n", screen.s.w,
               screen.s.h);
        continue;
      }
    }
    if (mode && !vncCrtcSet(crtc, mode, crtc->x, crtc->y, crtc->rotation,
                            crtc->numOutputs, crtc->outputs)) {
      rfbLog("ERROR: Could not crop CRTC to new screen dimensions\n");
      rfbFB.blockUpdates = FALSE;
      return rfbEDSResultInvalid;
    }
  }

  rfbFB.blockUpdates = FALSE;

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


static void vncUpdateScreenLayout(struct xorg_list *list)
{
  rfbScreenInfo *screen;

  xorg_list_for_each_entry(screen, list, entry) {
    /* Disabled? */
    if (!screen->output->crtc || !screen->output->crtc->mode) {
      screen->s.id = 0;
      screen->idAssigned = FALSE;
      continue;
    }

    screen->s.x = screen->output->crtc->x;
    screen->s.y = screen->output->crtc->y;
    screen->s.w = screen->output->crtc->mode->mode.width;
    screen->s.h = screen->output->crtc->mode->mode.height;
    if (!screen->idAssigned) {
      screen->s.id = screen->output->id;
      screen->idAssigned = TRUE;
    }
  }
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


/* Called by the X server within the body of RRCrtcSet() */

static Bool vncRRCrtcSet(ScreenPtr pScreen, RRCrtcPtr crtc, RRModePtr mode,
                         int x, int y, Rotation rotation, int numOutputs,
                         RROutputPtr *outputs)
{
  if (!vncCrtcSet(crtc, mode, x, y, rotation, numOutputs, outputs))
    return FALSE;

  vncUpdateScreenLayout(&rfbScreens);
  vncPrintScreenLayout(&rfbScreens);

  return rfbSendDesktopSizeAll(NULL, rfbEDSReasonServer);
}


/* Called by the X server within the body of RRGetInfo() */

static Bool vncRRGetInfo(ScreenPtr pScreen, Rotation *rotations)
{
  return TRUE;
}


/* Called the the X server within the body of RRScreenSizeSet() */

static Bool vncRRScreenSetSize(ScreenPtr pScreen, CARD16 width, CARD16 height,
                               CARD32 mmWidth, CARD32 mmHeight)
{
  if (vncScreenSetSize(pScreen, width, height, mmWidth,
                       mmHeight) != rfbEDSResultSuccess)
    return FALSE;

  vncUpdateScreenLayout(&rfbScreens);

  return rfbSendDesktopSizeAll(NULL, rfbEDSReasonServer);
}


/* Called by the VNC server during initialization */

Bool vncRRInit(ScreenPtr pScreen)
{
  rrScrPrivPtr rp;
  rfbScreenInfo *screen;

  if (!RRScreenInit(pScreen)) return FALSE;

  rp = rrGetScrPriv(pScreen);

  rp->rrGetInfo = vncRRGetInfo;
  rp->rrCrtcSet = vncRRCrtcSet;
  rp->rrScreenSetSize = vncRRScreenSetSize;

  RRScreenSetSizeRange(pScreen, 32, 32, rfbMaxWidth, rfbMaxHeight);
  xorg_list_for_each_entry(screen, &rfbScreens, entry) {
    if (!vncCreateOutput(pScreen, screen))
      return FALSE;
  }

  rfbLog("New desktop size: %d x %d\n", rfbFB.width, rfbFB.height);
  vncPrintScreenLayout(&rfbScreens);

  return TRUE;
}


/* Called by the VNC server when a desktop resize request is received from a
   client */

int ResizeDesktop(ScreenPtr pScreen, rfbClientPtr cl, int w, int h,
                  struct xorg_list *clientScreens)
{
  rrScrPrivPtr rp = rrGetScrPriv(pScreen);
  struct xorg_list serverScreens;
  rfbScreenInfo *clientScreen, *serverScreen, *tmp;
  rfbClientPtr cl2;

  for (cl2 = rfbClientHead; cl2; cl2 = cl2->next) {
    if (!cl2->enableDesktopSize && !cl2->enableExtDesktopSize) {
      rfbLog("ERROR: Not resizing desktop because one or more clients doesn't support it.\n");
      return rfbEDSResultProhibited;
    }
  }

  /* First check that we don't have any active clone modes.  That's just
     too messy to deal with. */
  if (vncHasOutputClones(pScreen)) {
    rfbLog("Desktop resize ERROR: Cannot change screen layout when clone mode is active\n");
    return rfbEDSResultInvalid;
  }

  if (w > rfbMaxWidth || h > rfbMaxHeight) {
    w = min(w, rfbMaxWidth);
    h = min(h, rfbMaxHeight);
    rfbLog("NOTICE: desktop size clamped to %dx%d per system policy\n", w, h);
  }
  rfbClipScreens(clientScreens, w, h);
  if (xorg_list_is_empty(clientScreens)) {
    rfbLog("Desktop resize ERROR: All screens are outside of the framebuffer (%dx%d)\n",
           w, h);
    return rfbEDSResultInvalid;
  }

  rfbDupeScreens(&serverScreens, &rfbScreens);
  xorg_list_for_each_entry(serverScreen, &serverScreens, entry)
    serverScreen->used = FALSE;

  /* Try to match client screen ID with existing server screen ID */
  xorg_list_for_each_entry_safe(clientScreen, tmp, clientScreens, entry) {
    serverScreen = rfbFindScreenID(&serverScreens, clientScreen->s.id);
    if (serverScreen) {
      memcpy(&serverScreen->s, &clientScreen->s, sizeof(rfbScreenDesc));
      serverScreen->idAssigned = serverScreen->used = TRUE;
      rfbRemoveScreen(clientScreen);
      if (!vncReconfigureOutput(serverScreen)) {
        rfbLog("Desktop resize ERROR: Could not reconfigure output for Screen 0x%.8x\n",
               serverScreen->s.id);
        rfbRemoveScreens(&serverScreens);
        return rfbEDSResultInvalid;
      }
    }
  }

  /* No match.  Try to find an unused server screen. */
  xorg_list_for_each_entry_safe(clientScreen, tmp, clientScreens, entry) {
    xorg_list_for_each_entry(serverScreen, &serverScreens, entry) {
      if (!serverScreen->used) {
        memcpy(&serverScreen->s, &clientScreen->s, sizeof(rfbScreenDesc));
        serverScreen->idAssigned = serverScreen->used = TRUE;
        rfbRemoveScreen(clientScreen);
        if (!vncReconfigureOutput(serverScreen)) {
          rfbLog("Desktop resize ERROR: Could not reconfigure output for Screen 0x%.8x\n",
                 serverScreen->s.id);
          rfbRemoveScreens(&serverScreens);
          return rfbEDSResultInvalid;
        }
        break;
      }
    }
  }

  /* Could not match existing screen or find an unused screen to use, so
     create a new one. */
  xorg_list_for_each_entry_safe(clientScreen, tmp, clientScreens, entry) {
    clientScreen->idAssigned = clientScreen->used = TRUE;
    xorg_list_del(&clientScreen->entry);
    rfbAddScreen(&serverScreens, clientScreen);
    if (!(vncCreateOutput(pScreen, clientScreen))) {
      rfbLog("Desktop resize ERROR: Could not create additional output for Screen 0x%.8x\n",
             clientScreen->s.id);
      rfbRemoveScreens(&serverScreens);
      return rfbEDSResultInvalid;
    }
  }

  /* Disable the outputs for the remaining unused screens */
  xorg_list_for_each_entry(serverScreen, &serverScreens, entry) {
    if (!serverScreen->used) {
      if (!vncDisableOutput(serverScreen->output)) {
        rfbLog("Desktop resize ERROR: Could not disable output for Screen 0x%.8x\n",
               serverScreen->s.id);
        rfbRemoveScreens(&serverScreens);
        return rfbEDSResultInvalid;
      }
      serverScreen->idAssigned = FALSE;
      serverScreen->s.id = 0;
    }
  }

  if (pScreen->width != w || pScreen->height != h) {
    int result = vncScreenSetSize(pScreen, w, h, mm(w), mm(h));
    if (result != rfbEDSResultSuccess)
      return result;
    vncUpdateScreenLayout(&serverScreens);
  } else
    RRTellChanged(pScreen);

  /* We have to do this to simulate an RandR-generated screen resize.
     Otherwise the RandR events may not be delivered properly to X clients. */
  rp->lastSetTime = currentTime;

  rfbRemoveScreens(&rfbScreens);
  rfbScreens = serverScreens;
  rfbScreens.prev->next = &rfbScreens;
  rfbScreens.next->prev = &rfbScreens;

  vncPrintScreenLayout(&rfbScreens);

  if (!rfbSendDesktopSizeAll(cl, rfbEDSReasonClient))
    return rfbEDSResultInvalid;

  return rfbEDSResultSuccess;
}

#endif
