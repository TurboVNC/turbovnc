/* Copyright (C) 2023-2024 D. R. Commander
 * Copyright 2024 Pierre Ossman for Cendio AB
 * Copyright (C) 2023 Kasm
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

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#ifdef DRI3

#include <errno.h>
#include <fcntl.h>
#include <glob.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <X11/X.h>
#include <X11/Xmd.h>
#include <dri3.h>
#include <drm_fourcc.h>
#include <fb.h>
#include <gcstruct.h>
#include <gbm.h>
#ifdef MITSHM
#include "shmint.h"
#endif
#ifdef HAVE_XSHMFENCE
#include <misyncshm.h>
#endif

#include "rfb.h"


#define DRI3_WRAP(field, function)  \
  pScreenPriv->field = pScreen->field;  \
  pScreen->field = function;

#define DRI3_UNWRAP(field)  \
  pScreen->field = pScreenPriv->field;


const char *driNode = NULL;

typedef struct {
  struct gbm_device *gbm;
  int fd;
  CloseScreenProcPtr CloseScreen;
  DestroyPixmapProcPtr DestroyPixmap;
} rfbDRI3ScreenRec, *rfbDRI3ScreenPtr;

static DevPrivateKeyRec rfbDRI3PixmapKey, rfbDRI3ScreenKey;

static void rfbDRI3SyncBOToPixmap(PixmapPtr pixmap);
static void rfbDRI3SyncPixmapToBO(PixmapPtr pixmap);


static int rfbDRI3Open(ScreenPtr screen, RRProviderPtr provider, int *pfd)
{
  int fd = open(driNode, O_RDWR | O_CLOEXEC);
  if (fd < 0)
    return BadAlloc;
  *pfd = fd;
  return Success;
}


static uint32_t gbm_format_for_depth(CARD8 depth)
{
  switch (depth) {
  case 16:
    return GBM_FORMAT_RGB565;
  case 24:
    return GBM_FORMAT_XRGB8888;
  case 30:
    return GBM_FORMAT_ARGB2101010;
  default:
    ErrorF("unexpected depth: %d\n", depth);
    /* fallthrough */
  case 32:
    return GBM_FORMAT_ARGB8888;
  }
}


static PixmapPtr rfbDRI3PixmapFromFD(ScreenPtr screen, int fd, CARD16 width,
                                      CARD16 height, CARD16 stride,
                                      CARD8 depth, CARD8 bpp)
{
  struct gbm_import_fd_data data;
  struct gbm_bo *bo = NULL;
  PixmapPtr pixmap;
  rfbDRI3ScreenPtr pScreenPriv =
    dixLookupPrivate(&screen->devPrivates, &rfbDRI3ScreenKey);

  if (width == 0 || height == 0 || depth < 15 || bpp != BitsPerPixel(depth) ||
      stride < width * bpp / 8 || bpp != sizeof(FbBits) * 8 ||
      stride % sizeof(FbBits) != 0 || !pScreenPriv)
    return NULL;

  data.fd = fd;
  data.width = width;
  data.height = height;
  data.stride = stride;
  data.format = gbm_format_for_depth(depth);
  bo = gbm_bo_import(pScreenPriv->gbm, GBM_BO_IMPORT_FD, &data,
                     GBM_BO_USE_RENDERING | GBM_BO_USE_LINEAR);
  if (!bo)
    return NULL;

  pixmap = screen->CreatePixmap(screen, width, height, depth, 0);
  if (pixmap == NULL)
    return NULL;

  dixSetPrivate(&pixmap->devPrivates, &rfbDRI3PixmapKey, bo);

  rfbDRI3SyncBOToPixmap(pixmap);

  return pixmap;
}


static int rfbDRI3FdFromPixmapVisitWindow(WindowPtr window, void *data)
{
  ScreenPtr pScreen = window->drawable.pScreen;
  PixmapPtr pixmap = data;

  if ((*pScreen->GetWindowPixmap) (window) == pixmap)
    window->drawable.serialNumber = NEXT_SERIAL_NUMBER;

  return WT_WALKCHILDREN;
}


static int rfbDRI3FDFromPixmap(ScreenPtr screen, PixmapPtr pixmap,
                               CARD16 *stride, CARD32 *size)
{
  struct gbm_bo *bo =
    dixLookupPrivate(&pixmap->devPrivates, &rfbDRI3PixmapKey);
  rfbDRI3ScreenPtr pScreenPriv =
    dixLookupPrivate(&screen->devPrivates, &rfbDRI3ScreenKey);

  if (!pScreenPriv) return -1;

  if (pixmap->drawable.bitsPerPixel != sizeof(FbBits) * 8) {
    ErrorF("rfbDRI3FDFromPixmap: Unexpected pixmap bits/pixel\n");
    return -1;
  }

  if (!bo) {
    bo = gbm_bo_create(pScreenPriv->gbm, pixmap->drawable.width,
                       pixmap->drawable.height,
                       gbm_format_for_depth(pixmap->drawable.depth),
                       GBM_BO_USE_RENDERING | GBM_BO_USE_LINEAR);
    if (!bo) {
      ErrorF("rfbDRI3FDFromPixmap: Failed to create GBM buffer object from pixmap\n");
      return -1;
    }

    if ((gbm_bo_get_stride(bo) % sizeof(FbBits)) != 0) {
      ErrorF("rfbDRI3FDFromPixmap: Unexpected GBM buffer object stride\n");
      gbm_bo_destroy(bo);
      return -1;
    }

    dixSetPrivate(&pixmap->devPrivates, &rfbDRI3PixmapKey, bo);

    /* Force re-validation of any GCs */
    pixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
    WalkTree(screen, rfbDRI3FdFromPixmapVisitWindow, pixmap);
  }

  rfbDRI3SyncPixmapToBO(pixmap);

  *stride = gbm_bo_get_stride(bo);
  *size = *stride * gbm_bo_get_height(bo);

  return gbm_bo_get_fd(bo);
}


/* This function synchronizes pixels from a pixmap's corresponding GBM buffer
   object into the pixmap itself. */

static void rfbDRI3SyncBOToPixmap(PixmapPtr pixmap)
{
  struct gbm_bo *bo;
  int w, h;
  FbBits *bo_ptr, *pixmap_ptr;
  uint32_t bo_stride, bo_bpp;
  void *opaque_ptr = NULL;
  int pixmap_stride, pixmap_bpp;
  pixman_bool_t ret;

  /* No-op if no DRM render node was specified */
  if (!driNode)
    return;

  /* Also a no-op if the pixmap wasn't created by our DRI3 implementation */
  bo = dixLookupPrivate(&pixmap->devPrivates, &rfbDRI3PixmapKey);
  if (!bo)
    return;

  w = gbm_bo_get_width(bo);
  h = gbm_bo_get_height(bo);

  bo_ptr = gbm_bo_map(bo, 0, 0, w, h, GBM_BO_TRANSFER_READ, &bo_stride,
                      &opaque_ptr);
  if (!bo_ptr) {
    ErrorF("rfbDRI3SyncBOToPixmap: gbm_bo_map() failed (errno %d)\n", errno);
    goto bailout;
  }
  bo_bpp = gbm_bo_get_bpp(bo);
  if (bo_bpp != sizeof(FbBits) * 8) {
    ErrorF("rfbDRI3SyncBOToPixmap: Unexpected GBM buffer object bits/pixel\n");
    goto bailout;
  }
  if (bo_stride % (bo_bpp / 8) != 0) {
    ErrorF("rfbDRI3SyncBOToPixmap: Unexpected GBM buffer object stride\n");
    goto bailout;
  }

  fbGetPixmapBitsData(pixmap, pixmap_ptr, pixmap_stride, pixmap_bpp);
  if (!pixmap_ptr) {
    ErrorF("rfbDRI3SyncBOToPixmap: fbGetPixmapBitsData() failed\n");
    goto bailout;
  }
  if (pixmap_bpp != sizeof(FbBits) * 8 || pixmap_bpp != bo_bpp) {
    ErrorF("rfbDRI3SyncBOToPixmap: Unexpected pixmap bits/pixel\n");
    goto bailout;
  }

  /* Try accelerated pixel copy first */
  ret = pixman_blt((uint32_t *)bo_ptr, (uint32_t *)pixmap_ptr,
                   bo_stride / (bo_bpp / 8), pixmap_stride, bo_bpp, pixmap_bpp,
                   0, 0, 0, 0, w, h);
  if (!ret)
    /* Fall back to slow, pure C version */
    fbBlt(bo_ptr, bo_stride / (bo_bpp / 8), 0, pixmap_ptr, pixmap_stride, 0,
          w * bo_bpp, h, GXcopy, FB_ALLONES, bo_bpp, FALSE, FALSE);

bailout:
  if (opaque_ptr) gbm_bo_unmap(bo, opaque_ptr);
}


void rfbDRI3SyncBOToDrawable(DrawablePtr drawable)
{
  PixmapPtr pixmap;
  int xoff, yoff;
  struct gbm_bo *bo;

  if (!driNode) return;

  fbGetDrawablePixmap(drawable, pixmap, xoff, yoff);
  (void)xoff;
  (void)yoff;

  bo = dixLookupPrivate(&pixmap->devPrivates, &rfbDRI3PixmapKey);
  if (!bo)
    return;

  rfbDRI3SyncBOToPixmap(pixmap);
}


/* This function synchronizes pixels from a pixmap into its corresponding GBM
   buffer object. */

static void rfbDRI3SyncPixmapToBO(PixmapPtr pixmap)
{
  struct gbm_bo *bo;
  int w, h;
  FbBits *bo_ptr, *pixmap_ptr;
  uint32_t bo_stride, bo_bpp;
  void *opaque_ptr = NULL;
  int pixmap_stride, pixmap_bpp;
  pixman_bool_t ret;

  /* No-op if no DRM render node was specified */
  if (!driNode)
    return;

  /* Also a no-op if the pixmap wasn't created by our DRI3 implementation */
  bo = dixLookupPrivate(&pixmap->devPrivates, &rfbDRI3PixmapKey);
  if (!bo)
    return;

  w = gbm_bo_get_width(bo);
  h = gbm_bo_get_height(bo);

  bo_ptr = gbm_bo_map(bo, 0, 0, w, h, GBM_BO_TRANSFER_WRITE, &bo_stride,
                      &opaque_ptr);
  if (!bo_ptr) {
    ErrorF("rfbDRI3SyncPixmapToBO: gbm_bo_map() failed (errno %d)\n", errno);
    goto bailout;
  }
  bo_bpp = gbm_bo_get_bpp(bo);
  if (bo_bpp != sizeof(FbBits) * 8) {
    ErrorF("rfbDRI3SyncPixmapToBO: Unexpected GBM buffer object bits/pixel\n");
    goto bailout;
  }
  if (bo_stride % (bo_bpp / 8) != 0) {
    ErrorF("rfbDRI3SyncPixmapToBO: Unexpected GBM buffer object stride\n");
    goto bailout;
  }

  fbGetPixmapBitsData(pixmap, pixmap_ptr, pixmap_stride, pixmap_bpp);
  if (!pixmap_ptr) {
    ErrorF("rfbDRI3SyncPixmapToBO: fbGetPixmapBitsData() failed\n");
    goto bailout;
  }
  if (pixmap_bpp != sizeof(FbBits) * 8 || pixmap_bpp != bo_bpp) {
    ErrorF("rfbDRI3SyncPixmapToBO: Unexpected pixmap bits/pixel\n");
    goto bailout;
  }

  /* Try accelerated pixel copy first */
  ret = pixman_blt((uint32_t *)pixmap_ptr, (uint32_t *)bo_ptr,
                   pixmap_stride, bo_stride / (bo_bpp / 8), pixmap_bpp,
                   bo_bpp, 0, 0, 0, 0, w, h);
  if (!ret)
    /* Fall back to slow, pure C version */
    fbBlt(pixmap_ptr, pixmap_stride, 0, bo_ptr, bo_stride / (bo_bpp / 8), 0,
          w * bo_bpp, h, GXcopy, FB_ALLONES, bo_bpp, FALSE, FALSE);

bailout:
  if (opaque_ptr) gbm_bo_unmap(bo, opaque_ptr);
}


void rfbDRI3SyncDrawableToBO(DrawablePtr drawable)
{
  PixmapPtr pixmap;
  int xoff, yoff;
  struct gbm_bo *bo;

  if (!driNode) return;

  fbGetDrawablePixmap(drawable, pixmap, xoff, yoff);
  (void)xoff;
  (void)yoff;

  bo = dixLookupPrivate(&pixmap->devPrivates, &rfbDRI3PixmapKey);
  if (!bo)
    return;

  rfbDRI3SyncPixmapToBO(pixmap);
}


static Bool rfbDRI3CloseScreen(ScreenPtr pScreen)
{
  rfbDRI3ScreenPtr pScreenPriv =
    dixLookupPrivate(&pScreen->devPrivates, &rfbDRI3ScreenKey);

  DRI3_UNWRAP(CloseScreen)
  DRI3_UNWRAP(DestroyPixmap)

  if (pScreenPriv->gbm) {
    gbm_device_destroy(pScreenPriv->gbm);
    pScreenPriv->gbm = NULL;
  }
  if (pScreenPriv->fd >= 0) {
    close(pScreenPriv->fd);
    pScreenPriv->fd = -1;
  }

  return (*pScreen->CloseScreen) (pScreen);
}


static Bool rfbDRI3DestroyPixmap(PixmapPtr pixmap)
{
  ScreenPtr pScreen = pixmap->drawable.pScreen;
  rfbDRI3ScreenPtr pScreenPriv =
    dixLookupPrivate(&pScreen->devPrivates, &rfbDRI3ScreenKey);
  Bool ret;

  if (pixmap->refcnt == 1) {
    struct gbm_bo *bo =
      dixLookupPrivate(&pixmap->devPrivates, &rfbDRI3PixmapKey);

    if (bo) {
      gbm_bo_destroy(bo);
      dixSetPrivate(&pixmap->devPrivates, &rfbDRI3PixmapKey, NULL);
    }
  }

  DRI3_UNWRAP(DestroyPixmap)

  ret = pScreen->DestroyPixmap(pixmap);

  DRI3_WRAP(DestroyPixmap, rfbDRI3DestroyPixmap)

  return ret;
}


static const dri3_screen_info_rec rfbDRI3ScreenInfo = {
  .version = 1,
  .open = rfbDRI3Open,
  .pixmap_from_fd = rfbDRI3PixmapFromFD,
  .fd_from_pixmap = rfbDRI3FDFromPixmap,
};


Bool rfbDRI3Initialize(ScreenPtr pScreen)
{
  rfbDRI3ScreenPtr pScreenPriv;

  if (!driNode)
    FatalError("DRM render node not specified");

  /* An empty render node string disables DRI3 */
  if (strlen(driNode) == 0) {
    driNode = NULL;
    return TRUE;
  }

#ifdef MITSHM
  ShmRegisterFbFuncs(pScreen);
#endif
#ifdef HAVE_XSHMFENCE
  if (!miSyncShmScreenInit(pScreen))
    return FALSE;
#endif

  if (!dixRegisterPrivateKey(&rfbDRI3ScreenKey, PRIVATE_SCREEN,
                             sizeof(rfbDRI3ScreenRec))) {
    ErrorF("rfbDRI3Initialize: dixRegisterPrivateKey failed\n");
    return FALSE;
  }

  if (!dixRegisterPrivateKey(&rfbDRI3PixmapKey, PRIVATE_PIXMAP, 0)) {
    ErrorF("rfbDRI3Initialize: dixRegisterPrivateKey failed\n");
    return FALSE;
  }

  pScreenPriv = dixLookupPrivate(&pScreen->devPrivates, &rfbDRI3ScreenKey);
  pScreenPriv->gbm = NULL;
  pScreenPriv->fd = -1;

  if (strcasecmp(driNode, "auto") == 0) {
    glob_t globbuf;
    int ret;
    size_t i;

    ret = glob("/dev/dri/renderD*", 0, NULL, &globbuf);
    if (ret == GLOB_NOMATCH) {
      ErrorF("Could not find any DRM render nodes\n");
      return FALSE;
    }
    if (ret != 0) {
      ErrorF("Could not enumerate DRM render nodes\n");
      return FALSE;
    }

    driNode = NULL;
    for (i = 0; i < globbuf.gl_pathc; i++) {
      if (access(globbuf.gl_pathv[i], R_OK | W_OK) == 0) {
        driNode = strdup(globbuf.gl_pathv[i]);
        break;
      }
    }

    globfree(&globbuf);

    if (driNode == NULL) {
      ErrorF("Could not find any available DRM render nodes\n");
      return FALSE;
    }
  }

  pScreenPriv->fd = open(driNode, O_RDWR | O_CLOEXEC);
  if (pScreenPriv->fd < 0) {
    ErrorF("Failed to open DRM render node %s\n", driNode);
    return FALSE;
  }

  pScreenPriv->gbm = gbm_create_device(pScreenPriv->fd);
  if (!pScreenPriv->gbm) {
    ErrorF("Failed to create GBM device from DRM render node\n");
    return FALSE;
  }

  DRI3_WRAP(CloseScreen, rfbDRI3CloseScreen)
  DRI3_WRAP(DestroyPixmap, rfbDRI3DestroyPixmap)

  rfbLog("Using DRM render node %s for DRI3\n", driNode);

  return dri3_screen_init(pScreen, &rfbDRI3ScreenInfo);
}

#endif  /* DRI3 */
