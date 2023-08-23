/* Copyright (C) 2023 Kasm
 * Copyright (C) 2023-2024 D. R. Commander
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

#include "rfb.h"

const char *driNode;

static struct gbm_device *gbm = NULL;
static int dri_fd = 0;

static DevPrivateKeyRec dri3_pixmap_private_key;

typedef struct {
  PixmapPtr pixmap;
  struct xorg_list entry;
} dri3_pixmap;

struct xorg_list dri3_pixmaps;
static CARD32 update_dri3_pixmaps(OsTimerPtr timer, CARD32 time, void *arg);
static OsTimerPtr dri3_pixmap_timer;


static int xvnc_dri3_open_client(ClientPtr client, ScreenPtr screen,
                                 RRProviderPtr provider, int *pfd)
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


static void add_dri3_pixmap(PixmapPtr pixmap)
{
  dri3_pixmap *dri3pm;

  xorg_list_for_each_entry(dri3pm, &dri3_pixmaps, entry) {
    if (dri3pm->pixmap == pixmap) {
      return;
    }
  }

  dri3pm = (dri3_pixmap *)rfbAlloc0(sizeof(dri3_pixmap));
  dri3pm->pixmap = pixmap;
  pixmap->refcnt++;
  xorg_list_append(&dri3pm->entry, &dri3_pixmaps);

  /* Start timer if it isn't already started */
  if (!dri3_pixmap_timer)
    dri3_pixmap_timer = TimerSet(NULL, 0, 16, update_dri3_pixmaps, NULL);
}


static PixmapPtr xvnc_dri3_pixmap_from_fds(ScreenPtr screen, CARD8 num_fds,
                                           const int *fds, CARD16 width,
                                           CARD16 height,
                                           const CARD32 *strides,
                                           const CARD32 *offsets, CARD8 depth,
                                           CARD8 bpp, uint64_t modifier)
{
  struct gbm_bo *bo = NULL;
  PixmapPtr pixmap;

  if (width == 0 || height == 0 || num_fds == 0 || depth < 15 ||
      bpp != BitsPerPixel(depth) || strides[0] < width * bpp / 8)
    return NULL;

  if (num_fds == 1) {
    struct gbm_import_fd_data data;

    data.fd = fds[0];
    data.width = width;
    data.height = height;
    data.stride = strides[0];
    data.format = gbm_format_for_depth(depth);
    bo = gbm_bo_import(gbm, GBM_BO_IMPORT_FD, &data, GBM_BO_USE_RENDERING);
    if (!bo)
      return NULL;
  } else {
    return NULL;
  }

  pixmap = screen->CreatePixmap(screen, gbm_bo_get_width(bo),
                                gbm_bo_get_height(bo), depth,
                                CREATE_PIXMAP_USAGE_SCRATCH);
  if (pixmap == NULL) {
    gbm_bo_destroy(bo);
    return NULL;
  }

  dixSetPrivate(&pixmap->devPrivates, &dri3_pixmap_private_key, bo);

  return pixmap;
}


static int xvnc_dri3_fds_from_pixmap(ScreenPtr screen, PixmapPtr pixmap,
                                     int *fds, uint32_t *strides,
                                     uint32_t *offsets, uint64_t *modifier)
{
  struct gbm_bo *bo =
    dixLookupPrivate(&pixmap->devPrivates, &dri3_pixmap_private_key);

  if (!bo) {
    bo = gbm_bo_create(gbm, pixmap->drawable.width, pixmap->drawable.height,
                       gbm_format_for_depth(pixmap->drawable.depth),
                       (pixmap->usage_hint == CREATE_PIXMAP_USAGE_SHARED ?
                        GBM_BO_USE_LINEAR : 0) |
                       GBM_BO_USE_RENDERING | GBM_BO_USE_SCANOUT);
    if (!bo) {
      ErrorF("Failed to create GBM buffer object from pixmap");
      return 0;
    }

    dixSetPrivate(&pixmap->devPrivates, &dri3_pixmap_private_key, bo);
  }

  fds[0] = gbm_bo_get_fd(bo);
  strides[0] = gbm_bo_get_stride(bo);
  offsets[0] = 0;
  *modifier = DRM_FORMAT_MOD_INVALID;

  add_dri3_pixmap(pixmap);

  return 1;
}


static Bool xvnc_dri3_get_formats(ScreenPtr screen, CARD32 *num_formats,
                                  CARD32 **formats)
{
  ErrorF("xvnc_dri3_get_formats() not implemented\n");
  return FALSE;
}


static Bool xvnc_dri3_get_modifiers(ScreenPtr screen, uint32_t format,
                                    uint32_t *num_modifiers,
                                    uint64_t **modifiers)
{
  ErrorF("xvnc_dri3_get_modifiers() not implemented\n");
  return FALSE;
}


static Bool xvnc_dri3_get_drawable_modifiers(DrawablePtr draw, uint32_t format,
                                             uint32_t *num_modifiers,
                                             uint64_t **modifiers)
{
  ErrorF("xvnc_dri3_get_drawable_modifiers() not implemented\n");
  return FALSE;
}


static const dri3_screen_info_rec xvnc_dri3_info = {
  .version = 2,
  .open = NULL,
  .pixmap_from_fds = xvnc_dri3_pixmap_from_fds,
  .fds_from_pixmap = xvnc_dri3_fds_from_pixmap,
  .open_client = xvnc_dri3_open_client,
  .get_formats = xvnc_dri3_get_formats,
  .get_modifiers = xvnc_dri3_get_modifiers,
  .get_drawable_modifiers = xvnc_dri3_get_drawable_modifiers,
};


/* This function synchronizes pixels from a pixmap's corresponding GBM buffer
   object into the pixmap itself.  There doesn't seem to be a good hook or sync
   point, so we do it manually, right before Present copies from the pixmap. */

void xvnc_dri3_sync_bo_to_pixmap(PixmapPtr pixmap)
{
  DrawablePtr pDraw;
  GCPtr gc;
  void *ptr;
  uint32_t stride;
  int w, h;
  void *opaque = NULL;
  struct gbm_bo *bo;

  /* No-op if no DRM render node was specified */
  if (!driNode)
    return;

  /* Also a no-op if the pixmap wasn't created by our DRI3 implementation */
  bo = dixLookupPrivate(&pixmap->devPrivates, &dri3_pixmap_private_key);
  if (!bo)
    return;

  w = gbm_bo_get_width(bo);
  h = gbm_bo_get_height(bo);

  ptr = gbm_bo_map(bo, 0, 0, w, h, GBM_BO_TRANSFER_READ, &stride, &opaque);
  if (!ptr) {
    ErrorF("xvnc_dri3_sync_bo_to_pixmap: gbm_bo_map() failed (errno %d)\n",
           errno);
    return;
  }

  pDraw = &pixmap->drawable;
  if ((gc = GetScratchGC(pDraw->depth, pDraw->pScreen))) {
    ValidateGC(pDraw, gc);
    fbPutZImage(pDraw, fbGetCompositeClip(gc), gc->alu, fbGetGCPrivate(gc)->pm,
                0, 0, w, h, ptr, stride / sizeof(FbStip));
    FreeScratchGC(gc);
  }

  gbm_bo_unmap(bo, opaque);
}


/* This function synchronizes pixels from all DRI3-created pixmaps into their
   corresponding GBM buffer objects.  Since we don't know when the pixmaps have
   changed or when the buffer objects will be read, we call this function both
   from the X Damage Extension's notification handler as well as from a
   timer (to account for applications that don't use the X Damage
   Extension.) */

void xvnc_dri3_sync_pixmaps_to_bos(void)
{
  uint32_t y;
  uint8_t *src, *dst;
  uint32_t srcstride, dststride;
  void *opaque = NULL;
  dri3_pixmap *dri3pm, *tmp;

  xorg_list_for_each_entry_safe(dri3pm, tmp, &dri3_pixmaps, entry) {
    struct gbm_bo *bo;

    if (dri3pm->pixmap->refcnt == 1) {
      /* We are the only user left, so delete the pixmap */
      dri3pm->pixmap->drawable.pScreen->DestroyPixmap(dri3pm->pixmap);
      xorg_list_del(&dri3pm->entry);
      free(dri3pm);
      continue;
    }

    bo =
      dixLookupPrivate(&dri3pm->pixmap->devPrivates, &dri3_pixmap_private_key);
    opaque = NULL;
    dst = gbm_bo_map(bo, 0, 0, dri3pm->pixmap->drawable.width,
                     dri3pm->pixmap->drawable.height, GBM_BO_TRANSFER_WRITE,
                     &dststride, &opaque);
    if (!dst) {
      ErrorF("xvnc_dri3_sync_pixmaps_to_bos: gbm_bo_map() failed (errno %d)\n",
             errno);
      continue;
    }

    srcstride = dri3pm->pixmap->devKind;
    src = dri3pm->pixmap->devPrivate.ptr;

    for (y = 0; y < dri3pm->pixmap->drawable.height; y++) {
      memcpy(dst, src, srcstride);
      dst += dststride;
      src += srcstride;
    }

    gbm_bo_unmap(bo, opaque);
  }
}


static CARD32 update_dri3_pixmaps(OsTimerPtr timer, CARD32 time, void *arg)
{
  int num_dri3_pixmaps = 0;
  dri3_pixmap *dri3pm;

  xvnc_dri3_sync_pixmaps_to_bos();

  xorg_list_for_each_entry(dri3pm, &dri3_pixmaps, entry)
    num_dri3_pixmaps++;
  if (!num_dri3_pixmaps) {
    TimerFree(dri3_pixmap_timer);
    dri3_pixmap_timer = NULL;
    return 0;
  }

  return 16;  /* Re-schedule the timer */
}


void xvnc_dri3_init(void)
{
  if (!dixRegisterPrivateKey(&dri3_pixmap_private_key, PRIVATE_PIXMAP, 0))
    FatalError("xvnc_dri3_init: dixRegisterPrivateKey failed");

  if (!driNode)
    FatalError("DRM render node not specified");

  dri_fd = open(driNode, O_RDWR | O_CLOEXEC);
  if (!dri_fd)
    FatalError("Failed to open DRM render node %s", driNode);

  gbm = gbm_create_device(dri_fd);
  if (!gbm)
    FatalError("Failed to create GBM device from DRM render node");

  xorg_list_init(&dri3_pixmaps);

  if (!dri3_screen_init(screenInfo.screens[0], &xvnc_dri3_info))
    FatalError("Cannot initialize DRI3 extension");
}

#endif  /* DRI3 */
