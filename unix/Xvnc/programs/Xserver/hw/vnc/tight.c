/*
 * tight.c
 *
 * Routines to implement Tight Encoding
 */

/* Copyright (C) 2010-2012, 2014, 2017, 2022 D. R. Commander.
 *                                           All Rights Reserved.
 * Copyright (C) 2005-2008 Sun Microsystems, Inc.  All Rights Reserved.
 * Copyright (C) 2004 Landmark Graphics Corporation.  All Rights Reserved.
 * Copyright (C) 2000, 2001 Const Kaplinsky.  All Rights Reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include "rfb.h"
#include "turbojpeg.h"


/* Note: The following constant should not be changed. */
#define TIGHT_MIN_TO_COMPRESS 12

/* The parameters below may be adjusted. */
#define MIN_SPLIT_RECT_SIZE     4096
#define MIN_SOLID_SUBRECT_SIZE  2048
#define MAX_SPLIT_TILE_SIZE       16

/* This variable is set on every rfbSendRectEncodingTight() call. */
static Bool usePixelFormat24;


/* ALR stuff */
#define ADD_TO_LOSSY_REGION(x, y, w, h)  \
  if (rfbAutoLosslessRefresh > 0.0) {  \
    RegionRec tmpRegion;  BoxRec box;  \
    box.x1 = x;  box.y1 = y;  \
    box.x2 = x + w;  box.y2 = y + h;  \
    REGION_INIT(pScreen, &tmpRegion, &box, 0);  \
    REGION_UNION(pScreen, &t->lossyRegion, &t->lossyRegion, &tmpRegion);  \
    REGION_UNINIT(pScreen, &tmpRegion);  \
  }

#define ADD_TO_LOSSLESS_REGION(x, y, w, h)  \
  if (rfbAutoLosslessRefresh > 0.0) {  \
    RegionRec tmpRegion;  BoxRec box;  \
    box.x1 = x;  box.y1 = y;  \
    box.x2 = x + w;  box.y2 = y + h;  \
    REGION_INIT(pScreen, &tmpRegion, &box, 0);  \
    REGION_UNION(pScreen, &t->losslessRegion, &t->losslessRegion,  \
                 &tmpRegion);  \
    REGION_UNINIT(pScreen, &tmpRegion);  \
  }


/* Compression level stuff. The following array contains various
   encoder parameters for each of 10 compression levels (0..9).
   Last three parameters correspond to JPEG quality levels (0..9). */

typedef struct TIGHT_CONF_s {
  int maxRectSize, maxRectWidth;
  int monoMinRectSize;
  int idxZlibLevel, monoZlibLevel, rawZlibLevel;
  int idxMaxColorsDivisor;
  int palMaxColorsWithJPEG;
} TIGHT_CONF;

static TIGHT_CONF tightConf[4] = {
  { 65536, 2048,   6, 0, 0, 0,   4, 24 },  /* 0  (used only without JPEG) */
  { 65536, 2048,  32, 1, 1, 1,  96, 24 },  /* 1 */
  { 65536, 2048,  32, 3, 3, 2,  96, 96 },  /* 2  (used only with JPEG) */
  { 65536, 2048,  32, 7, 7, 5,  96, 256 }  /* 9 */
};

static int compressLevel;
static int qualityLevel;
static int subsampLevel;

static const int subsampLevel2tjsubsamp[TVNC_SAMPOPT] = {
  TJ_444, TJ_420, TJ_422, TJ_GRAYSCALE
};


/* Stuff dealing with palettes. */

typedef struct COLOR_LIST_s {
  struct COLOR_LIST_s *next;
  int idx;
  CARD32 rgb;
} COLOR_LIST;

typedef struct PALETTE_ENTRY_s {
  COLOR_LIST *listNode;
  int numPixels;
} PALETTE_ENTRY;

typedef struct PALETTE_s {
  PALETTE_ENTRY entry[256];
  COLOR_LIST *hash[256];
  COLOR_LIST list[256];
} PALETTE;


/* Globals for multi-threading */

static Bool threadInit = FALSE;
static pthread_t thnd[MAX_ENCODING_THREADS] = { 0, 0, 0, 0, 0, 0, 0, 0 };

typedef struct _threadparam {
  rfbClientPtr cl;
  int x, y, w, h, id, _ublen, *ublen;
  char *tightBeforeBuf;
  int tightBeforeBufSize;
  char *tightAfterBuf;
  int tightAfterBufSize;
  char *updateBuf;
  int updateBufSize;
  int paletteNumColors, paletteMaxColors;
  CARD32 monoBackground, monoForeground;
  PALETTE palette;
  tjhandle j;
  int bytessent, rectsent;
  int streamId, baseStreamId, nStreams;
  pthread_mutex_t ready, done;
  Bool status, deadyet;
  RegionRec lossyRegion, losslessRegion;
} threadparam;

static threadparam tparam[MAX_ENCODING_THREADS];


/* Prototypes for static functions. */

static void FindBestSolidArea(rfbClientPtr cl, int x, int y, int w, int h,
                              CARD32 colorValue, int *w_ptr, int *h_ptr);
static void ExtendSolidArea(rfbClientPtr cl, int x, int y, int w, int h,
                            CARD32 colorValue, int *x_ptr, int *y_ptr,
                            int *w_ptr, int *h_ptr);
static Bool CheckSolidTile(rfbClientPtr cl, int x, int y, int w, int h,
                           CARD32 *colorPtr, Bool needSameColor);
static Bool CheckSolidTile8(rfbClientPtr cl, int x, int y, int w, int h,
                            CARD32 *colorPtr, Bool needSameColor);
static Bool CheckSolidTile16(rfbClientPtr cl, int x, int y, int w, int h,
                             CARD32 *colorPtr, Bool needSameColor);
static Bool CheckSolidTile32(rfbClientPtr cl, int x, int y, int w, int h,
                             CARD32 *colorPtr, Bool needSameColor);

static Bool SendRectSimple(threadparam *t, int x, int y, int w, int h);
static Bool SendSubrect(threadparam *t, int x, int y, int w, int h);
static Bool SendTightHeader(threadparam *t, int x, int y, int w, int h);

static Bool SendSolidRect(threadparam *t);
static Bool SendMonoRect(threadparam *t, int w, int h);
static Bool SendIndexedRect(threadparam *t, int w, int h);
static Bool SendFullColorRect(threadparam *t, int w, int h);

static Bool CompressData(threadparam *t, int streamId, int dataLen,
                         int zlibLevel, int zlibStrategy);
static Bool SendCompressedData(threadparam *t, char *buf, int compressedLen);

static void FillPalette8(threadparam *t, int count);
static void FillPalette16(threadparam *t, int count);
static void FillPalette32(threadparam *t, int count);
static void FastFillPalette16(threadparam *t, CARD16 *data, int w, int pitch,
                              int h);
static void FastFillPalette32(threadparam *t, CARD32 *data, int w, int pitch,
                              int h);

static void PaletteReset(threadparam *t);
static int PaletteInsert(threadparam *t, CARD32 rgb, int numPixels, int bpp);

static void Pack24(char *buf, rfbPixelFormat *fmt, int count);

static void EncodeIndexedRect16(threadparam *t, CARD8 *buf, int count);
static void EncodeIndexedRect32(threadparam *t, CARD8 *buf, int count);

static void EncodeMonoRect8(threadparam *t, CARD8 *buf, int w, int h);
static void EncodeMonoRect16(threadparam *t, CARD8 *buf, int w, int h);
static void EncodeMonoRect32(threadparam *t, CARD8 *buf, int w, int h);

static Bool SendJpegRect(threadparam *t, int x, int y, int w, int h,
                         int quality);

static Bool SendRectEncodingTight(threadparam *t, int x, int y, int w, int h);

static void *TightThreadFunc(void *param);
static Bool CheckUpdateBuf(threadparam *t, int bytes);


/*
 * Tight encoding implementation.
 */

int rfbNumCodedRectsTight(rfbClientPtr cl, int x, int y, int w, int h)
{
  int maxRectSize, maxRectWidth;
  int subrectMaxWidth, subrectMaxHeight;

  /* No matter how many rectangles we will send if LastRect markers
     are used to terminate rectangle stream. */
  if (cl->enableLastRectEncoding && w * h >= MIN_SPLIT_RECT_SIZE)
    return 0;

  maxRectSize = tightConf[compressLevel].maxRectSize;
  maxRectWidth = tightConf[compressLevel].maxRectWidth;

  if (w > maxRectWidth || w * h > maxRectSize) {
    subrectMaxWidth = (w > maxRectWidth) ? maxRectWidth : w;
    subrectMaxHeight = maxRectSize / subrectMaxWidth;
    return ((w - 1) / maxRectWidth + 1) * ((h - 1) / subrectMaxHeight + 1);
  } else {
    return 1;
  }
}


/*
 * Translate compression level into Tight compression level
 */

int rfbTightCompressLevel(rfbClientPtr cl)
{
  int tightCompressLevel = cl->tightCompressLevel;

  /* If the interframe comparison engine is set to automatic, then
     interframe comparison will be enabled for compression levels 5 and
     above.  Thus, we map 5-8 internally to 0-4 so that we can get
     interframe-enabled equivalents of all of the documented TurboVNC modes.
     */
  if (tightCompressLevel >= 5 && tightCompressLevel <= 8)
    tightCompressLevel -= 5;

  /* We only allow compression levels that have a demonstrable performance
     benefit.  CL 0 with JPEG reduces CPU usage for workloads that have low
     numbers of unique colors, but the same thing can be accomplished by
     using CL 0 without JPEG (AKA "Lossless Tight.")  For those same
     low-color workloads, CL 2 with JPEG can provide typically 20-40% better
     compression than CL 1 (with a commensurate increase in CPU usage.)  For
     high-color workloads, CL 1 should always be used, as higher compression
     levels increase CPU usage for these workloads without providing any
     significant reduction in bandwidth. */
  if (cl->tightQualityLevel != -1) {
    if (tightCompressLevel < 1) tightCompressLevel = 1;
    if (tightCompressLevel > 2) tightCompressLevel = 2;
  }

  /* With JPEG disabled, CL 2 offers no significant bandwidth savings over
     CL 1, so we don't include it. */
  else if (tightCompressLevel > 1) tightCompressLevel = 1;

  /* CL 9 (which maps internally to CL 3) is included mainly for backward
     compatibility with TightVNC Compression Levels 5-9.  It should be used
     only in extremely low-bandwidth cases in which it can be shown to have a
     benefit.  For low-color workloads, it provides typically only 10-20%
     better compression than CL 2 with JPEG and CL 1 without JPEG, and it
     uses, on average, twice as much CPU time.  Interframe comparison is
     always enabled with this mode. */
  if (cl->tightCompressLevel == 9) tightCompressLevel = 3;

  return tightCompressLevel;
}


static void InitThreads(void)
{
  int err = 0, i;

  if (threadInit) return;

  memset(tparam, 0, sizeof(threadparam) * MAX_ENCODING_THREADS);
  tparam[0].ublen = &ublen;
  tparam[0].updateBuf = updateBuf;
  for (i = 1; i < MAX_ENCODING_THREADS; i++) {
    tparam[i].ublen = &tparam[i]._ublen;
    tparam[i].id = i;
  }
  rfbLog("Using %d thread%s for Tight encoding\n", rfbNumThreads,
         rfbNumThreads == 1 ? "" : "s");
  if (rfbNumThreads > 1) {
    for (i = 1; i < rfbNumThreads; i++) {
      if (!tparam[i].updateBuf) {
        tparam[i].updateBufSize = UPDATE_BUF_SIZE;
        tparam[i].updateBuf = (char *)rfbAlloc(tparam[i].updateBufSize);
      }
      pthread_mutex_init(&tparam[i].ready, NULL);
      pthread_mutex_lock(&tparam[i].ready);
      pthread_mutex_init(&tparam[i].done, NULL);
      pthread_mutex_lock(&tparam[i].done);
      if ((err = pthread_create(&thnd[i], NULL, TightThreadFunc,
                                &tparam[i])) != 0) {
        rfbLog("Could not start thread %d: %s\n", i + 1,
               strerror(err == -1 ? errno : err));
        return;
      }
    }
  }
  threadInit = TRUE;
}

void ShutdownTightThreads(void)
{
  int i;

  if (!threadInit) return;
  if (rfbNumThreads > 1) {
    for (i = 1; i < rfbNumThreads; i++) {
      if (thnd[i]) {
        tparam[i].deadyet = TRUE;
        pthread_mutex_unlock(&tparam[i].ready);
        pthread_join(thnd[i], NULL);
        thnd[i] = 0;
        pthread_mutex_destroy(&tparam[i].ready);
        pthread_mutex_destroy(&tparam[i].done);
      }
    }
  }
  for (i = 0; i < rfbNumThreads; i++) {
    free(tparam[i].tightAfterBuf);
    free(tparam[i].tightBeforeBuf);
    if (i != 0) free(tparam[i].updateBuf);
    if (tparam[i].j) tjDestroy(tparam[i].j);
    if (!REGION_NAR(&tparam[i].losslessRegion))
      REGION_UNINIT(pScreen, &tparam[i].losslessRegion);
    if (!REGION_NAR(&tparam[i].lossyRegion))
      REGION_UNINIT(pScreen, &tparam[i].lossyRegion);
    memset(&tparam[i], 0, sizeof(threadparam));
  }
  threadInit = FALSE;
}

static void *TightThreadFunc(void *param)
{
  threadparam *t = (threadparam *)param;

  while (!t->deadyet) {
    pthread_mutex_lock(&t->ready);
    if (t->deadyet) break;
    t->status = SendRectEncodingTight(t, t->x, t->y, t->w, t->h);
    pthread_mutex_unlock(&t->done);
  }
  return NULL;
}


static Bool CheckUpdateBuf(threadparam *t, int bytes)
{
  rfbClientPtr cl = t->cl;

  if (t->id == 0) {
    if (ublen + bytes > UPDATE_BUF_SIZE) {
      if (!rfbSendUpdateBuf(cl))
        return FALSE;
    }
  } else {
    if ((*t->ublen) + bytes > t->updateBufSize) {
      t->updateBufSize += UPDATE_BUF_SIZE;
      t->updateBuf = (char *)rfbRealloc(t->updateBuf, t->updateBufSize);
    }
  }
  return TRUE;
}


Bool rfbSendRectEncodingTight(rfbClientPtr cl, int x, int y, int w, int h)
{
  Bool status = TRUE;
  int i, nt;

  if (!threadInit) {
    InitThreads();
    if (!threadInit) return FALSE;
  }

  compressLevel = rfbTightCompressLevel(cl);
  qualityLevel = cl->tightQualityLevel;
  subsampLevel = cl->tightSubsampLevel;

  if (cl->format.depth == 24 && cl->format.redMax == 0xFF &&
      cl->format.greenMax == 0xFF && cl->format.blueMax == 0xFF)
    usePixelFormat24 = TRUE;
  else
    usePixelFormat24 = FALSE;

  nt = min(rfbNumThreads, w * h / tightConf[compressLevel].maxRectSize);
  if (nt < 1) nt = 1;

  for (i = 0; i < nt; i++) {
    tparam[i].status = TRUE;
    tparam[i].cl = cl;
    tparam[i].x = x;
    tparam[i].y = h / nt * i + y;
    tparam[i].w = w;
    tparam[i].h = (i == nt - 1) ? (h - (h / nt * i)) : h / nt;
    tparam[i].bytessent = tparam[i].rectsent = 0;
    if (rfbAutoLosslessRefresh > 0.0) {
      REGION_INIT(pScreen, &tparam[i].lossyRegion, NullBox, 0);
      REGION_INIT(pScreen, &tparam[i].losslessRegion, NullBox, 0);
    }
    if (i < 4) {
      int n = min(nt, 4);
      tparam[i].baseStreamId = 4 / n * i;
      if (i == n - 1) tparam[i].nStreams = 4 - tparam[i].baseStreamId;
      else tparam[i].nStreams = 4 / n;
      tparam[i].streamId = tparam[i].baseStreamId;
    }
  }
  if (nt > 1) {
    for (i = 1; i < nt; i++) pthread_mutex_unlock(&tparam[i].ready);
  }

  status &= SendRectEncodingTight(&tparam[0], tparam[0].x, tparam[0].y,
                                  tparam[0].w, tparam[0].h);
  if (!status) return FALSE;
  cl->rfbBytesSent[rfbEncodingTight] += tparam[0].bytessent;
  cl->rfbRectanglesSent[rfbEncodingTight] += tparam[0].rectsent;

  if (nt > 1) {
    for (i = 1; i < nt; i++) {
      pthread_mutex_lock(&tparam[i].done);
      status &= tparam[i].status;
    }
    if (status == FALSE) return FALSE;
    if (ublen > 0) {
      if (!rfbSendUpdateBuf(cl))
        return FALSE;
    }
    for (i = 1; i < nt; i++) {
      if ((*tparam[i].ublen) > 0)
        WRITE_OR_CLOSE(tparam[i].updateBuf, *tparam[i].ublen, return FALSE);
      (*tparam[i].ublen) = 0;
      cl->rfbBytesSent[rfbEncodingTight] += tparam[i].bytessent;
      cl->rfbRectanglesSent[rfbEncodingTight] += tparam[i].rectsent;
    }
  }

  if (rfbAutoLosslessRefresh > 0.0) {
    for (i = 0; i < nt; i++) {
      REGION_UNION(pScreen, &cl->lossyRegion, &cl->lossyRegion,
                   &tparam[i].lossyRegion);
      REGION_UNINIT(pScreen, &tparam[i].lossyRegion);
      memset(&tparam[i].lossyRegion, 0, sizeof(RegionRec));
      REGION_SUBTRACT(pScreen, &cl->lossyRegion, &cl->lossyRegion,
                      &tparam[i].losslessRegion);
      REGION_UNINIT(pScreen, &tparam[i].losslessRegion);
      memset(&tparam[i].losslessRegion, 0, sizeof(RegionRec));
    }
  }

  return status;
}


static Bool SendRectEncodingTight(threadparam *t, int x, int y, int w, int h)
{
  int nMaxRows;
  CARD32 colorValue = 0;
  int dx, dy, dw, dh;
  int x_best, y_best, w_best, h_best;
  char *fbptr;
  rfbClientPtr cl = t->cl;

  if (!cl->enableLastRectEncoding || w * h < MIN_SPLIT_RECT_SIZE)
    return SendRectSimple(t, x, y, w, h);

  /* Make sure we can write at least one pixel into tightBeforeBuf. */

  if (t->tightBeforeBufSize < 4) {
    t->tightBeforeBufSize = 4;
    if (t->tightBeforeBuf == NULL)
      t->tightBeforeBuf = (char *)rfbAlloc(t->tightBeforeBufSize);
    else
      t->tightBeforeBuf = (char *)rfbRealloc(t->tightBeforeBuf,
                                             t->tightBeforeBufSize);
  }

  /* Calculate maximum number of rows in one non-solid rectangle. */

  {
    int maxRectSize, maxRectWidth, nMaxWidth;

    maxRectSize = tightConf[compressLevel].maxRectSize;
    maxRectWidth = tightConf[compressLevel].maxRectWidth;
    nMaxWidth = (w > maxRectWidth) ? maxRectWidth : w;
    nMaxRows = maxRectSize / nMaxWidth;
  }

  /* Try to find large solid-color areas and send them separately. */

  for (dy = y; dy < y + h; dy += MAX_SPLIT_TILE_SIZE) {

    /* If a rectangle becomes too large, send its upper part now. */

    if (dy - y >= nMaxRows) {
      if (!SendRectSimple(t, x, y, w, nMaxRows))
        return 0;
      y += nMaxRows;
      h -= nMaxRows;
    }

    dh =
      (dy + MAX_SPLIT_TILE_SIZE <= y + h) ? MAX_SPLIT_TILE_SIZE : (y + h - dy);

    for (dx = x; dx < x + w; dx += MAX_SPLIT_TILE_SIZE) {

      dw = (dx + MAX_SPLIT_TILE_SIZE <= x + w) ?
           MAX_SPLIT_TILE_SIZE : (x + w - dx);

      if (CheckSolidTile(cl, dx, dy, dw, dh, &colorValue, FALSE)) {

        if (subsampLevel == TJ_GRAYSCALE && qualityLevel != -1) {
          CARD32 r = (colorValue >> 16) & 0xFF;
          CARD32 g = (colorValue >> 8) & 0xFF;
          CARD32 b = (colorValue) & 0xFF;
          double lum = (0.257 * (double)r) + (0.504 * (double)g) +
                       (0.098 * (double)b) + 16.;
          colorValue = (int)lum + (((int)lum) << 8) + (((int)lum) << 16);
        }

        /* Get dimensions of solid-color area. */

        FindBestSolidArea(cl, dx, dy, w - (dx - x), h - (dy - y), colorValue,
                          &w_best, &h_best);

        /* Make sure a solid rectangle is large enough
           (or the whole rectangle is of the same color). */

        if (w_best * h_best != w * h &&
            w_best * h_best < MIN_SOLID_SUBRECT_SIZE)
          continue;

        /* Try to extend solid rectangle to maximum size. */

        x_best = dx;  y_best = dy;
        ExtendSolidArea(cl, x, y, w, h, colorValue, &x_best, &y_best,
                        &w_best, &h_best);

        /* Send rectangles at top and left to solid-color area. */

        if (y_best != y && !SendRectSimple(t, x, y, w, y_best - y))
          return FALSE;
        if (x_best != x &&
            !SendRectEncodingTight(t, x, y_best, x_best - x, h_best))
          return FALSE;

        /* Send solid-color rectangle. */

        if (!SendTightHeader(t, x_best, y_best, w_best, h_best))
          return FALSE;

        fbptr = (cl->fb + (rfbFB.paddedWidthInBytes * y_best) +
                 (x_best * (rfbFB.bitsPerPixel / 8)));

        (*cl->translateFn) (cl->translateLookupTable, &rfbServerFormat,
                            &cl->format, fbptr, t->tightBeforeBuf,
                            rfbFB.paddedWidthInBytes, 1, 1);

        if (!SendSolidRect(t))
          return FALSE;
        ADD_TO_LOSSLESS_REGION(x_best, y_best, w_best, h_best);

        /* Send remaining rectangles (at right and bottom). */

        if (x_best + w_best != x + w &&
            !SendRectEncodingTight(t, x_best + w_best, y_best,
                                   w - (x_best - x) - w_best, h_best))
          return FALSE;
        if (y_best + h_best != y + h &&
            !SendRectEncodingTight(t, x, y_best + h_best,
                                   w, h - (y_best - y) - h_best))
          return FALSE;

        /* Return after all recursive calls are done. */

        return TRUE;
      }

    }

  }

  /* No suitable solid-color rectangles found. */

  return SendRectSimple(t, x, y, w, h);
}


static void FindBestSolidArea(rfbClientPtr cl, int x, int y, int w, int h,
                              CARD32 colorValue, int *w_ptr, int *h_ptr)
{
  int dx, dy, dw, dh;
  int w_prev;
  int w_best = 0, h_best = 0;

  w_prev = w;

  for (dy = y; dy < y + h; dy += MAX_SPLIT_TILE_SIZE) {

    dh = (dy + MAX_SPLIT_TILE_SIZE <= y + h) ?
         MAX_SPLIT_TILE_SIZE : (y + h - dy);
    dw = (w_prev > MAX_SPLIT_TILE_SIZE) ?
         MAX_SPLIT_TILE_SIZE : w_prev;

    if (!CheckSolidTile(cl, x, dy, dw, dh, &colorValue, TRUE))
      break;

    for (dx = x + dw; dx < x + w_prev;) {
      dw = (dx + MAX_SPLIT_TILE_SIZE <= x + w_prev) ?
           MAX_SPLIT_TILE_SIZE : (x + w_prev - dx);
      if (!CheckSolidTile(cl, dx, dy, dw, dh, &colorValue, TRUE))
        break;
      dx += dw;
    }

    w_prev = dx - x;
    if (w_prev * (dy + dh - y) > w_best * h_best) {
      w_best = w_prev;
      h_best = dy + dh - y;
    }
  }

  *w_ptr = w_best;
  *h_ptr = h_best;
}


static void ExtendSolidArea(rfbClientPtr cl, int x, int y, int w, int h,
                            CARD32 colorValue, int *x_ptr, int *y_ptr,
                            int *w_ptr, int *h_ptr)
{
  int cx, cy;

  /* Try to extend the area upwards. */
  for (cy = *y_ptr - 1;
       cy >= y && CheckSolidTile(cl, *x_ptr, cy, *w_ptr, 1, &colorValue, TRUE);
       cy--);
  *h_ptr += *y_ptr - (cy + 1);
  *y_ptr = cy + 1;

  /* ... downwards. */
  for (cy = *y_ptr + *h_ptr;
       cy < y + h &&
       CheckSolidTile(cl, *x_ptr, cy, *w_ptr, 1, &colorValue, TRUE);
       cy++);
  *h_ptr += cy - (*y_ptr + *h_ptr);

  /* ... to the left. */
  for (cx = *x_ptr - 1;
       cx >= x && CheckSolidTile(cl, cx, *y_ptr, 1, *h_ptr, &colorValue, TRUE);
       cx--);
  *w_ptr += *x_ptr - (cx + 1);
  *x_ptr = cx + 1;

  /* ... to the right. */
  for (cx = *x_ptr + *w_ptr;
       cx < x + w &&
       CheckSolidTile(cl, cx, *y_ptr, 1, *h_ptr, &colorValue, TRUE);
       cx++);
  *w_ptr += cx - (*x_ptr + *w_ptr);
}


/*
 * Check if a rectangle is all of the same color. If needSameColor is
 * set to non-zero, then also check that its color equals to the
 * *colorPtr value. The result is 1 if the test is successfull, and in
 * that case new color will be stored in *colorPtr.
 */

static Bool CheckSolidTile(rfbClientPtr cl, int x, int y, int w, int h,
                           CARD32 *colorPtr, Bool needSameColor)
{
  switch (rfbServerFormat.bitsPerPixel) {
    case 32:
      return CheckSolidTile32(cl, x, y, w, h, colorPtr, needSameColor);
    case 16:
      return CheckSolidTile16(cl, x, y, w, h, colorPtr, needSameColor);
    default:
      return CheckSolidTile8(cl, x, y, w, h, colorPtr, needSameColor);
  }
}


#define DEFINE_CHECK_SOLID_FUNCTION(bpp)                                      \
                                                                              \
static Bool CheckSolidTile##bpp(rfbClientPtr cl, int x, int y, int w, int h,  \
                                CARD32 *colorPtr, Bool needSameColor)         \
{                                                                             \
  CARD##bpp *fbptr;                                                           \
  CARD##bpp colorValue;                                                       \
  int dx, dy;                                                                 \
                                                                              \
  fbptr =                                                                     \
    (CARD##bpp *)&cl->fb[y * rfbFB.paddedWidthInBytes + x * (bpp / 8)];       \
                                                                              \
  colorValue = *fbptr;                                                        \
  if (needSameColor && (CARD32)colorValue != *colorPtr)                       \
    return FALSE;                                                             \
                                                                              \
  for (dy = 0; dy < h; dy++) {                                                \
    for (dx = 0; dx < w; dx++) {                                              \
      if (colorValue != fbptr[dx])                                            \
        return FALSE;                                                         \
    }                                                                         \
    fbptr = (CARD##bpp *)((CARD8 *)fbptr + rfbFB.paddedWidthInBytes);         \
  }                                                                           \
                                                                              \
  *colorPtr = (CARD32)colorValue;                                             \
  return TRUE;                                                                \
}

DEFINE_CHECK_SOLID_FUNCTION(8)
DEFINE_CHECK_SOLID_FUNCTION(16)
DEFINE_CHECK_SOLID_FUNCTION(32)


static Bool SendRectSimple(threadparam *t, int x, int y, int w, int h)
{
  int maxBeforeSize, maxAfterSize;
  int maxRectSize, maxRectWidth;
  int subrectMaxWidth, subrectMaxHeight;
  int dx, dy;
  int rw, rh;
  rfbClientPtr cl = t->cl;

  maxRectSize = tightConf[compressLevel].maxRectSize;
  maxRectWidth = tightConf[compressLevel].maxRectWidth;

  maxBeforeSize = maxRectSize * (cl->format.bitsPerPixel / 8);
  maxAfterSize = maxBeforeSize + (maxBeforeSize + 99) / 100 + 12;

  if (t->tightBeforeBufSize < maxBeforeSize) {
    t->tightBeforeBufSize = maxBeforeSize;
    if (t->tightBeforeBuf == NULL)
      t->tightBeforeBuf = (char *)rfbAlloc(t->tightBeforeBufSize);
    else
      t->tightBeforeBuf = (char *)rfbRealloc(t->tightBeforeBuf,
                                             t->tightBeforeBufSize);
  }

  if (t->tightAfterBufSize < maxAfterSize) {
    t->tightAfterBufSize = maxAfterSize;
    if (t->tightAfterBuf == NULL)
      t->tightAfterBuf = (char *)rfbAlloc(t->tightAfterBufSize);
    else
      t->tightAfterBuf = (char *)rfbRealloc(t->tightAfterBuf,
                                            t->tightAfterBufSize);
  }

  if (w > maxRectWidth || w * h > maxRectSize) {
    subrectMaxWidth = (w > maxRectWidth) ? maxRectWidth : w;
    subrectMaxHeight = maxRectSize / subrectMaxWidth;

    for (dy = 0; dy < h; dy += subrectMaxHeight) {
      for (dx = 0; dx < w; dx += maxRectWidth) {
        rw = (dx + maxRectWidth < w) ? maxRectWidth : w - dx;
        rh = (dy + subrectMaxHeight < h) ? subrectMaxHeight : h - dy;
        if (!SendSubrect(t, x + dx, y + dy, rw, rh))
          return FALSE;
      }
    }
  } else {
    if (!SendSubrect(t, x, y, w, h))
      return FALSE;
  }

  return TRUE;
}


static Bool SendSubrect(threadparam *t, int x, int y, int w, int h)
{
  char *fbptr;
  Bool success = FALSE;
  rfbClientPtr cl = t->cl;

  /* Send pending data if there is more than 128 bytes. */
  if (t->id == 0) {
    if (ublen > 128) {
      if (!rfbSendUpdateBuf(cl))
        return FALSE;
    }
  }

  if (!SendTightHeader(t, x, y, w, h))
    return FALSE;

  fbptr =
    (cl->fb + (rfbFB.paddedWidthInBytes * y) + (x * (rfbFB.bitsPerPixel / 8)));

  if (subsampLevel == TJ_GRAYSCALE && qualityLevel != -1 &&
      rfbFB.bitsPerPixel > 8)
    return SendJpegRect(t, x, y, w, h, qualityLevel);

  t->paletteMaxColors = w * h / tightConf[compressLevel].idxMaxColorsDivisor;
  if (qualityLevel != -1)
    t->paletteMaxColors = tightConf[compressLevel].palMaxColorsWithJPEG;
  if (t->paletteMaxColors < 2 &&
      w * h >= tightConf[compressLevel].monoMinRectSize) {
    t->paletteMaxColors = 2;
  }

  if (cl->format.bitsPerPixel == rfbServerFormat.bitsPerPixel &&
      cl->format.redMax == rfbServerFormat.redMax &&
      cl->format.greenMax == rfbServerFormat.greenMax &&
      cl->format.blueMax == rfbServerFormat.blueMax &&
      cl->format.bitsPerPixel >= 16) {

    /* This is so we can avoid translating the pixels when compressing
       with JPEG, since it is unnecessary */
    switch (cl->format.bitsPerPixel) {
      case 16:
        FastFillPalette16(t, (CARD16 *)fbptr, w, rfbFB.paddedWidthInBytes / 2,
                          h);
        break;
      default:
        FastFillPalette32(t, (CARD32 *)fbptr, w, rfbFB.paddedWidthInBytes / 4,
                          h);
    }

    if (t->paletteNumColors != 0 || qualityLevel == -1) {
      (*cl->translateFn) (cl->translateLookupTable, &rfbServerFormat,
                          &cl->format, fbptr, t->tightBeforeBuf,
                          rfbFB.paddedWidthInBytes, w, h);
    }
  } else {
    (*cl->translateFn) (cl->translateLookupTable, &rfbServerFormat,
                        &cl->format, fbptr, t->tightBeforeBuf,
                        rfbFB.paddedWidthInBytes, w, h);

    switch (cl->format.bitsPerPixel) {
      case 8:
        FillPalette8(t, w * h);
        break;
      case 16:
        FillPalette16(t, w * h);
        break;
      default:
        FillPalette32(t, w * h);
    }
  }

  switch (t->paletteNumColors) {
    case 0:
      /* Truecolor image */
      if (qualityLevel != -1) {
        success = SendJpegRect(t, x, y, w, h, qualityLevel);
      } else {
        success = SendFullColorRect(t, w, h);
        ADD_TO_LOSSLESS_REGION(x, y, w, h);
      }
      break;
    case 1:
      /* Solid rectangle */
      success = SendSolidRect(t);
      ADD_TO_LOSSLESS_REGION(x, y, w, h);
      break;
    case 2:
      /* Two-color rectangle */
      success = SendMonoRect(t, w, h);
      ADD_TO_LOSSLESS_REGION(x, y, w, h);
      break;
    default:
      /* Up to 256 different colors */
      success = SendIndexedRect(t, w, h);
      ADD_TO_LOSSLESS_REGION(x, y, w, h);
  }
  return success;
}


static Bool SendTightHeader(threadparam *t, int x, int y, int w, int h)
{
  rfbFramebufferUpdateRectHeader rect;

  if (!CheckUpdateBuf(t, sz_rfbFramebufferUpdateRectHeader))
    return FALSE;

  rect.r.x = Swap16IfLE(x);
  rect.r.y = Swap16IfLE(y);
  rect.r.w = Swap16IfLE(w);
  rect.r.h = Swap16IfLE(h);
  rect.encoding = Swap32IfLE(rfbEncodingTight);

  memcpy(&t->updateBuf[*t->ublen], (char *)&rect,
         sz_rfbFramebufferUpdateRectHeader);
  (*t->ublen) += sz_rfbFramebufferUpdateRectHeader;

  t->rectsent++;
  t->bytessent += sz_rfbFramebufferUpdateRectHeader;

  return TRUE;
}


/*
 * Subencoding implementations.
 */

static Bool SendSolidRect(threadparam *t)
{
  int len;
  rfbClientPtr cl = t->cl;

  if (usePixelFormat24) {
    Pack24(t->tightBeforeBuf, &cl->format, 1);
    len = 3;
  } else
    len = cl->format.bitsPerPixel / 8;

  if (!CheckUpdateBuf(t, 1 + len))
    return FALSE;

  t->updateBuf[(*t->ublen)++] = (char)(rfbTightFill << 4);
  memcpy(&t->updateBuf[*t->ublen], t->tightBeforeBuf, len);
  (*t->ublen) += len;

  t->bytessent += len + 1;

  return TRUE;
}


static Bool SendMonoRect(threadparam *t, int w, int h)
{
  int streamId = t->streamId;
  int paletteLen, dataLen;
  rfbClientPtr cl = t->cl;

  if (!CheckUpdateBuf(t, TIGHT_MIN_TO_COMPRESS + 6 +
                         2 * cl->format.bitsPerPixel / 8))
    return FALSE;

  if (t->nStreams > 0) {
    t->streamId++;
    if (t->streamId >= t->baseStreamId + t->nStreams)
      t->streamId = t->baseStreamId;
  }

  /* Prepare tight encoding header. */
  dataLen = (w + 7) / 8;
  dataLen *= h;

  if (tightConf[compressLevel].monoZlibLevel == 0 || t->id > 3)
    t->updateBuf[(*t->ublen)++] =
      (char)((rfbTightNoZlib | rfbTightExplicitFilter) << 4);
  else
    t->updateBuf[(*t->ublen)++] = (streamId | rfbTightExplicitFilter) << 4;
  t->updateBuf[(*t->ublen)++] = rfbTightFilterPalette;
  t->updateBuf[(*t->ublen)++] = 1;

  /* Prepare palette, convert image. */
  switch (cl->format.bitsPerPixel) {
    case 32:
      EncodeMonoRect32(t, (CARD8 *)t->tightBeforeBuf, w, h);

      ((CARD32 *)t->tightAfterBuf)[0] = t->monoBackground;
      ((CARD32 *)t->tightAfterBuf)[1] = t->monoForeground;
      if (usePixelFormat24) {
        Pack24(t->tightAfterBuf, &cl->format, 2);
        paletteLen = 6;
      } else
        paletteLen = 8;

      memcpy(&t->updateBuf[*t->ublen], t->tightAfterBuf, paletteLen);
      (*t->ublen) += paletteLen;
      t->bytessent += 3 + paletteLen;
      break;

    case 16:
      EncodeMonoRect16(t, (CARD8 *)t->tightBeforeBuf, w, h);

      ((CARD16 *)t->tightAfterBuf)[0] = (CARD16)t->monoBackground;
      ((CARD16 *)t->tightAfterBuf)[1] = (CARD16)t->monoForeground;

      memcpy(&t->updateBuf[*t->ublen], t->tightAfterBuf, 4);
      (*t->ublen) += 4;
      t->bytessent += 7;
      break;

    default:
      EncodeMonoRect8(t, (CARD8 *)t->tightBeforeBuf, w, h);

      t->updateBuf[(*t->ublen)++] = (char)t->monoBackground;
      t->updateBuf[(*t->ublen)++] = (char)t->monoForeground;
      t->bytessent += 5;
  }

  return CompressData(t, streamId, dataLen,
                      tightConf[compressLevel].monoZlibLevel,
                      Z_DEFAULT_STRATEGY);
}


static Bool SendIndexedRect(threadparam *t, int w, int h)
{
  int streamId = t->streamId;
  int i, entryLen;
  rfbClientPtr cl = t->cl;

  if (!CheckUpdateBuf(t, TIGHT_MIN_TO_COMPRESS + 6 +
                         t->paletteNumColors * cl->format.bitsPerPixel / 8))
    return FALSE;

  if (t->nStreams > 0) {
    t->streamId++;
    if (t->streamId >= t->baseStreamId + t->nStreams)
      t->streamId = t->baseStreamId;
  }

  /* Prepare tight encoding header. */
  if (tightConf[compressLevel].idxZlibLevel == 0 || t->id > 3)
    t->updateBuf[(*t->ublen)++] =
      (char)((rfbTightNoZlib | rfbTightExplicitFilter) << 4);
  else
    t->updateBuf[(*t->ublen)++] = (streamId | rfbTightExplicitFilter) << 4;
  t->updateBuf[(*t->ublen)++] = rfbTightFilterPalette;
  t->updateBuf[(*t->ublen)++] = (char)(t->paletteNumColors - 1);

  /* Prepare palette, convert image. */
  switch (cl->format.bitsPerPixel) {
    case 32:
      EncodeIndexedRect32(t, (CARD8 *)t->tightBeforeBuf, w * h);

      for (i = 0; i < t->paletteNumColors; i++)
        ((CARD32 *)t->tightAfterBuf)[i] = t->palette.entry[i].listNode->rgb;
      if (usePixelFormat24) {
        Pack24(t->tightAfterBuf, &cl->format, t->paletteNumColors);
        entryLen = 3;
      } else
        entryLen = 4;

      memcpy(&t->updateBuf[*t->ublen], t->tightAfterBuf,
             t->paletteNumColors * entryLen);
      (*t->ublen) += t->paletteNumColors * entryLen;
      t->bytessent += 3 + t->paletteNumColors * entryLen;
      break;

    case 16:
      EncodeIndexedRect16(t, (CARD8 *)t->tightBeforeBuf, w * h);

      for (i = 0; i < t->paletteNumColors; i++) {
        ((CARD16 *)t->tightAfterBuf)[i] =
          (CARD16)t->palette.entry[i].listNode->rgb;
      }

      memcpy(&t->updateBuf[*t->ublen], t->tightAfterBuf,
             t->paletteNumColors * 2);
      (*t->ublen) += t->paletteNumColors * 2;
      t->bytessent += 3 + t->paletteNumColors * 2;
      break;

    default:
      return FALSE;     /* Should never happen. */
  }

  return CompressData(t, streamId, w * h,
                      tightConf[compressLevel].idxZlibLevel,
                      Z_DEFAULT_STRATEGY);
}


static Bool SendFullColorRect(threadparam *t, int w, int h)
{
  int streamId = t->streamId;
  int len;
  rfbClientPtr cl = t->cl;

  if (!CheckUpdateBuf(t, TIGHT_MIN_TO_COMPRESS + 1))
    return FALSE;

  if (t->nStreams > 0) {
    t->streamId++;
    if (t->streamId >= t->baseStreamId + t->nStreams)
      t->streamId = t->baseStreamId;
  }

  if (tightConf[compressLevel].rawZlibLevel == 0 || t->id > 3)
    t->updateBuf[(*t->ublen)++] = (char)(rfbTightNoZlib << 4);
  else
    t->updateBuf[(*t->ublen)++] = streamId << 4;
  t->bytessent++;

  if (usePixelFormat24) {
    Pack24(t->tightBeforeBuf, &cl->format, w * h);
    len = 3;
  } else
    len = cl->format.bitsPerPixel / 8;

  return CompressData(t, streamId, w * h * len,
                      tightConf[compressLevel].rawZlibLevel,
                      Z_DEFAULT_STRATEGY);
}


static Bool CompressData(threadparam *t, int streamId, int dataLen,
                         int zlibLevel, int zlibStrategy)
{
  z_streamp pz;
  int err;
  rfbClientPtr cl = t->cl;

  if (dataLen < TIGHT_MIN_TO_COMPRESS) {
    memcpy(&t->updateBuf[*t->ublen], t->tightBeforeBuf, dataLen);
    (*t->ublen) += dataLen;
    t->bytessent += dataLen;
    return TRUE;
  }

  /* Tight encoding has only a limited number of Zlib streams (4).  The
     streams must all be left open as long as the client is connected, or
     performance suffers.  Thus, multiple threads can't use the same Zlib
     stream.  We divide the pool of 4 evenly among the available threads (up
     to the first 4 threads), and if each thread has more than one stream, it
     cycles between them in a round-robin fashion.  If we have more than 4
     threads, then threads 5 and beyond must encode their data without Zlib
     compression. */
  if (zlibLevel == 0 || t->id > 3)
    return SendCompressedData(t, t->tightBeforeBuf, dataLen);

  pz = &cl->zsStruct[streamId];

  /* Initialize compression stream if needed. */
  if (!cl->zsActive[streamId]) {
    pz->zalloc = Z_NULL;
    pz->zfree = Z_NULL;
    pz->opaque = Z_NULL;

    err = deflateInit2(pz, zlibLevel, Z_DEFLATED, MAX_WBITS, MAX_MEM_LEVEL,
                       zlibStrategy);
    if (err != Z_OK)
      return FALSE;

    cl->zsActive[streamId] = TRUE;
    cl->zsLevel[streamId] = zlibLevel;
  }

  /* Prepare buffer pointers. */
  pz->next_in = (Bytef *)t->tightBeforeBuf;
  pz->avail_in = dataLen;
  pz->next_out = (Bytef *)t->tightAfterBuf;
  pz->avail_out = t->tightAfterBufSize;

  /* Change compression parameters if needed. */
  if (zlibLevel != cl->zsLevel[streamId]) {
    if (deflateParams(pz, zlibLevel, zlibStrategy) != Z_OK)
      return FALSE;
    cl->zsLevel[streamId] = zlibLevel;
  }

  /* Actual compression. */
  if (deflate(pz, Z_SYNC_FLUSH) != Z_OK || pz->avail_in != 0 ||
      pz->avail_out == 0)
    return FALSE;

  return SendCompressedData(t, t->tightAfterBuf,
                            t->tightAfterBufSize - pz->avail_out);
}


static Bool SendCompressedData(threadparam *t, char *buf, int compressedLen)
{
  int i, portionLen;

  t->updateBuf[(*t->ublen)++] = compressedLen & 0x7F;
  t->bytessent++;
  if (compressedLen > 0x7F) {
    t->updateBuf[(*t->ublen) - 1] |= 0x80;
    t->updateBuf[(*t->ublen)++] = compressedLen >> 7 & 0x7F;
    t->bytessent++;
    if (compressedLen > 0x3FFF) {
      t->updateBuf[(*t->ublen) - 1] |= 0x80;
      t->updateBuf[(*t->ublen)++] = compressedLen >> 14 & 0xFF;
      t->bytessent++;
    }
  }

  portionLen = UPDATE_BUF_SIZE;
  for (i = 0; i < compressedLen; i += portionLen) {
    if (i + portionLen > compressedLen)
      portionLen = compressedLen - i;
    if (!CheckUpdateBuf(t, portionLen))
      return FALSE;
    memcpy(&t->updateBuf[*t->ublen], &buf[i], portionLen);
    (*t->ublen) += portionLen;
  }
  t->bytessent += compressedLen;
  return TRUE;
}


/*
 * Code to determine how many different colors used in rectangle.
 */

static void FillPalette8(threadparam *t, int count)
{
  CARD8 *data = (CARD8 *)t->tightBeforeBuf;
  CARD8 c0, c1;
  int i, n0, n1;

  t->paletteNumColors = 0;

  c0 = data[0];
  for (i = 1; i < count && data[i] == c0; i++);
  if (i == count) {
    t->paletteNumColors = 1;
    return;                     /* Solid rectangle */
  }

  if (t->paletteMaxColors < 2)
    return;

  n0 = i;
  c1 = data[i];
  n1 = 0;
  for (i++; i < count; i++) {
    if (data[i] == c0)
      n0++;
    else if (data[i] == c1)
      n1++;
    else
      break;
  }
  if (i == count) {
    if (n0 > n1) {
      t->monoBackground = (CARD32)c0;
      t->monoForeground = (CARD32)c1;
    } else {
      t->monoBackground = (CARD32)c1;
      t->monoForeground = (CARD32)c0;
    }
    t->paletteNumColors = 2;    /* Two colors */
  }
}


#define DEFINE_FILL_PALETTE_FUNCTION(bpp)                                     \
                                                                              \
static void FillPalette##bpp(threadparam *t, int count)                       \
{                                                                             \
  CARD##bpp *data = (CARD##bpp *)t->tightBeforeBuf;                           \
  CARD##bpp c0, c1, ci;                                                       \
  int i, n0, n1, ni;                                                          \
                                                                              \
  c0 = data[0];                                                               \
  for (i = 1; i < count && data[i] == c0; i++);                               \
  if (i >= count) {                                                           \
    t->paletteNumColors = 1;    /* Solid rectangle */                         \
    return;                                                                   \
  }                                                                           \
                                                                              \
  if (t->paletteMaxColors < 2) {                                              \
    t->paletteNumColors = 0;    /* Full-color encoding preferred */           \
    return;                                                                   \
  }                                                                           \
                                                                              \
  n0 = i;                                                                     \
  c1 = data[i];                                                               \
  n1 = 0;                                                                     \
  for (i++; i < count; i++) {                                                 \
    ci = data[i];                                                             \
    if (ci == c0)                                                             \
      n0++;                                                                   \
    else if (ci == c1)                                                        \
      n1++;                                                                   \
    else                                                                      \
      break;                                                                  \
  }                                                                           \
  if (i >= count) {                                                           \
    if (n0 > n1) {                                                            \
      t->monoBackground = (CARD32)c0;                                         \
      t->monoForeground = (CARD32)c1;                                         \
    } else {                                                                  \
      t->monoBackground = (CARD32)c1;                                         \
      t->monoForeground = (CARD32)c0;                                         \
    }                                                                         \
    t->paletteNumColors = 2;    /* Two colors */                              \
    return;                                                                   \
  }                                                                           \
                                                                              \
  PaletteReset(t);                                                            \
  PaletteInsert(t, c0, (CARD32)n0, bpp);                                      \
  PaletteInsert(t, c1, (CARD32)n1, bpp);                                      \
                                                                              \
  ni = 1;                                                                     \
  for (i++; i < count; i++) {                                                 \
    if (data[i] == ci) {                                                      \
      ni++;                                                                   \
    } else {                                                                  \
      if (!PaletteInsert(t, ci, (CARD32)ni, bpp))                             \
        return;                                                               \
      ci = data[i];                                                           \
      ni = 1;                                                                 \
    }                                                                         \
  }                                                                           \
  PaletteInsert(t, ci, (CARD32)ni, bpp);                                      \
}

DEFINE_FILL_PALETTE_FUNCTION(16)
DEFINE_FILL_PALETTE_FUNCTION(32)


#define DEFINE_FAST_FILL_PALETTE_FUNCTION(bpp)                                \
                                                                              \
static void FastFillPalette##bpp(threadparam *t, CARD##bpp *data,             \
                                 int w, int pitch, int h)                     \
{                                                                             \
  CARD##bpp c0, c1, ci, mask, c0t, c1t, cit;                                  \
  int i, j, i2 = 0, j2, n0, n1, ni;                                           \
  rfbClientPtr cl = t->cl;                                                    \
                                                                              \
  if (cl->translateFn != rfbTranslateNone) {                                  \
    mask = rfbServerFormat.redMax << rfbServerFormat.redShift;                \
    mask |= rfbServerFormat.greenMax << rfbServerFormat.greenShift;           \
    mask |= rfbServerFormat.blueMax << rfbServerFormat.blueShift;             \
  } else mask = ~0;                                                           \
                                                                              \
  c0 = data[0] & mask;                                                        \
  for (j = 0; j < h; j++) {                                                   \
    for (i = 0; i < w; i++) {                                                 \
      if ((data[j * pitch + i] & mask) != c0)                                 \
        goto done;                                                            \
    }                                                                         \
  }                                                                           \
  done:                                                                       \
  if (j >= h) {                                                               \
    t->paletteNumColors = 1;    /* Solid rectangle */                         \
    return;                                                                   \
  }                                                                           \
  if (t->paletteMaxColors < 2) {                                              \
    t->paletteNumColors = 0;    /* Full-color encoding preferred */           \
    return;                                                                   \
  }                                                                           \
                                                                              \
  n0 = j * w + i;                                                             \
  c1 = data[j * pitch + i] & mask;                                            \
  n1 = 0;                                                                     \
  i++;  if (i >= w) { i = 0;  j++; }                                          \
  for (j2 = j; j2 < h; j2++) {                                                \
    for (i2 = i; i2 < w; i2++) {                                              \
      ci = data[j2 * pitch + i2] & mask;                                      \
      if (ci == c0)                                                           \
        n0++;                                                                 \
      else if (ci == c1)                                                      \
        n1++;                                                                 \
      else                                                                    \
        goto done2;                                                           \
    }                                                                         \
    i = 0;                                                                    \
  }                                                                           \
  done2:                                                                      \
  (*cl->translateFn) (cl->translateLookupTable, &rfbServerFormat,             \
                      &cl->format, (char *)&c0, (char *)&c0t, bpp / 8,        \
                      1, 1);                                                  \
  (*cl->translateFn) (cl->translateLookupTable, &rfbServerFormat,             \
                      &cl->format, (char *)&c1, (char *)&c1t, bpp / 8,        \
                      1, 1);                                                  \
  if (j2 >= h) {                                                              \
    if (n0 > n1) {                                                            \
      t->monoBackground = (CARD32)c0t;                                        \
      t->monoForeground = (CARD32)c1t;                                        \
    } else {                                                                  \
      t->monoBackground = (CARD32)c1t;                                        \
      t->monoForeground = (CARD32)c0t;                                        \
    }                                                                         \
    t->paletteNumColors = 2;    /* Two colors */                              \
    return;                                                                   \
  }                                                                           \
                                                                              \
  PaletteReset(t);                                                            \
  PaletteInsert(t, c0t, (CARD32)n0, bpp);                                     \
  PaletteInsert(t, c1t, (CARD32)n1, bpp);                                     \
                                                                              \
  ni = 1;                                                                     \
  i2++;  if (i2 >= w) { i2 = 0;  j2++; }                                      \
  for (j = j2; j < h; j++) {                                                  \
    for (i = i2; i < w; i++) {                                                \
      if ((data[j * pitch + i] & mask) == ci) {                               \
        ni++;                                                                 \
      } else {                                                                \
        (*cl->translateFn) (cl->translateLookupTable, &rfbServerFormat,       \
                            &cl->format, (char *)&ci, (char *)&cit, bpp / 8,  \
                            1, 1);                                            \
        if (!PaletteInsert(t, cit, (CARD32)ni, bpp))                          \
          return;                                                             \
        ci = data[j * pitch + i] & mask;                                      \
        ni = 1;                                                               \
      }                                                                       \
    }                                                                         \
    i2 = 0;                                                                   \
  }                                                                           \
                                                                              \
  (*cl->translateFn) (cl->translateLookupTable, &rfbServerFormat,             \
                      &cl->format, (char *)&ci, (char *)&cit, bpp / 8,        \
                      1, 1);                                                  \
  PaletteInsert(t, cit, (CARD32)ni, bpp);                                     \
}

DEFINE_FAST_FILL_PALETTE_FUNCTION(16)
DEFINE_FAST_FILL_PALETTE_FUNCTION(32)


/*
 * Functions to operate with palette structures.
 */

#define HASH_FUNC16(rgb) ((int)((((rgb) >> 8) + (rgb)) & 0xFF))
#define HASH_FUNC32(rgb) ((int)((((rgb) >> 16) + ((rgb) >> 8)) & 0xFF))


static void PaletteReset(threadparam *t)
{
  t->paletteNumColors = 0;
  memset(t->palette.hash, 0, 256 * sizeof(COLOR_LIST *));
}


static int PaletteInsert(threadparam *t, CARD32 rgb, int numPixels, int bpp)
{
  COLOR_LIST *pnode;
  COLOR_LIST *prev_pnode = NULL;
  int hash_key, idx, new_idx, count;

  hash_key = (bpp == 16) ? HASH_FUNC16(rgb) : HASH_FUNC32(rgb);

  pnode = t->palette.hash[hash_key];

  while (pnode != NULL) {
    if (pnode->rgb == rgb) {
      /* Such palette entry already exists. */
      new_idx = idx = pnode->idx;
      count = t->palette.entry[idx].numPixels + numPixels;
      if (new_idx && t->palette.entry[new_idx - 1].numPixels < count) {
        do {
          t->palette.entry[new_idx] = t->palette.entry[new_idx - 1];
          t->palette.entry[new_idx].listNode->idx = new_idx;
          new_idx--;
        } while (new_idx && t->palette.entry[new_idx - 1].numPixels < count);
        t->palette.entry[new_idx].listNode = pnode;
        pnode->idx = new_idx;
      }
      t->palette.entry[new_idx].numPixels = count;
      return t->paletteNumColors;
    }
    prev_pnode = pnode;
    pnode = pnode->next;
  }

  /* Check if palette is full. */
  if (t->paletteNumColors == 256 ||
      t->paletteNumColors == t->paletteMaxColors) {
    t->paletteNumColors = 0;
    return 0;
  }

  /* Move palette entries with lesser pixel counts. */
  for (idx = t->paletteNumColors;
       idx > 0 && t->palette.entry[idx - 1].numPixels < numPixels;
       idx--) {
    t->palette.entry[idx] = t->palette.entry[idx - 1];
    t->palette.entry[idx].listNode->idx = idx;
  }

  /* Add new palette entry into the freed slot. */
  pnode = &t->palette.list[t->paletteNumColors];
  if (prev_pnode != NULL)
    prev_pnode->next = pnode;
  else
    t->palette.hash[hash_key] = pnode;
  pnode->next = NULL;
  pnode->idx = idx;
  pnode->rgb = rgb;
  t->palette.entry[idx].listNode = pnode;
  t->palette.entry[idx].numPixels = numPixels;

  return ++(t->paletteNumColors);
}


/*
 * Converting 32-bit color samples into 24-bit colors.
 * Should be called only when redMax, greenMax and blueMax are 255.
 * Color components assumed to be byte-aligned.
 */

static void Pack24(char *buf, rfbPixelFormat *fmt, int count)
{
  CARD32 *buf32;
  CARD32 pix;
  int r_shift, g_shift, b_shift;

  buf32 = (CARD32 *)buf;

  if (!rfbServerFormat.bigEndian == !fmt->bigEndian) {
    r_shift = fmt->redShift;
    g_shift = fmt->greenShift;
    b_shift = fmt->blueShift;
  } else {
    r_shift = 24 - fmt->redShift;
    g_shift = 24 - fmt->greenShift;
    b_shift = 24 - fmt->blueShift;
  }

  while (count--) {
    pix = *buf32++;
    *buf++ = (char)(pix >> r_shift);
    *buf++ = (char)(pix >> g_shift);
    *buf++ = (char)(pix >> b_shift);
  }
}


/*
 * Converting truecolor samples into palette indices.
 */

#define DEFINE_IDX_ENCODE_FUNCTION(bpp)                                       \
                                                                              \
static void EncodeIndexedRect##bpp(threadparam *t, CARD8 *buf, int count)     \
{                                                                             \
  COLOR_LIST *pnode;                                                          \
  CARD##bpp *src;                                                             \
  CARD##bpp rgb;                                                              \
  int rep = 0;                                                                \
                                                                              \
  src = (CARD##bpp *)buf;                                                     \
                                                                              \
  while (count--) {                                                           \
    rgb = *src++;                                                             \
    while (count && *src == rgb)                                              \
      rep++, src++, count--;                                                  \
    pnode = t->palette.hash[HASH_FUNC##bpp(rgb)];                             \
    while (pnode != NULL) {                                                   \
      if ((CARD##bpp)pnode->rgb == rgb) {                                     \
        *buf++ = (CARD8)pnode->idx;                                           \
        while (rep) {                                                         \
          *buf++ = (CARD8)pnode->idx;                                         \
          rep--;                                                              \
        }                                                                     \
        break;                                                                \
      }                                                                       \
      pnode = pnode->next;                                                    \
    }                                                                         \
  }                                                                           \
}

DEFINE_IDX_ENCODE_FUNCTION(16)
DEFINE_IDX_ENCODE_FUNCTION(32)


#define DEFINE_MONO_ENCODE_FUNCTION(bpp)                                      \
                                                                              \
static void EncodeMonoRect##bpp(threadparam *t, CARD8 *buf, int w, int h)     \
{                                                                             \
  CARD##bpp *ptr;                                                             \
  CARD##bpp bg;                                                               \
  unsigned int value, mask;                                                   \
  int aligned_width;                                                          \
  int x, y, bg_bits;                                                          \
                                                                              \
  ptr = (CARD##bpp *)buf;                                                     \
  bg = (CARD##bpp)t->monoBackground;                                          \
  aligned_width = w - w % 8;                                                  \
                                                                              \
  for (y = 0; y < h; y++) {                                                   \
    for (x = 0; x < aligned_width; x += 8) {                                  \
      for (bg_bits = 0; bg_bits < 8; bg_bits++) {                             \
        if (*ptr++ != bg)                                                     \
          break;                                                              \
      }                                                                       \
      if (bg_bits == 8) {                                                     \
        *buf++ = 0;                                                           \
        continue;                                                             \
      }                                                                       \
      mask = 0x80 >> bg_bits;                                                 \
      value = mask;                                                           \
      for (bg_bits++; bg_bits < 8; bg_bits++) {                               \
        mask >>= 1;                                                           \
        if (*ptr++ != bg)                                                     \
          value |= mask;                                                      \
      }                                                                       \
      *buf++ = (CARD8)value;                                                  \
    }                                                                         \
                                                                              \
    mask = 0x80;                                                              \
    value = 0;                                                                \
    if (x >= w)                                                               \
      continue;                                                               \
                                                                              \
    for (; x < w; x++) {                                                      \
      if (*ptr++ != bg)                                                       \
        value |= mask;                                                        \
      mask >>= 1;                                                             \
    }                                                                         \
    *buf++ = (CARD8)value;                                                    \
  }                                                                           \
}

DEFINE_MONO_ENCODE_FUNCTION(8)
DEFINE_MONO_ENCODE_FUNCTION(16)
DEFINE_MONO_ENCODE_FUNCTION(32)


#define DEFINE_RGB_CONVERT_FUNCTION(bpp)                                      \
                                                                              \
static unsigned char *ConvertRGB##bpp(rfbClientPtr cl, int x, int y,          \
                                      int w, int h)                           \
{                                                                             \
  CARD##bpp *srcptr, pix;                                                     \
  unsigned char *dst, *tmpbuf;                                                \
  int inRed, inGreen, inBlue, i, j, ps = bpp / 8;                             \
                                                                              \
  tmpbuf = (unsigned char *)rfbAlloc(w * h * 3);                              \
  srcptr = (CARD##bpp *)&cl->fb[y * rfbFB.paddedWidthInBytes + x * ps];       \
  dst = tmpbuf;                                                               \
  for (j = 0; j < h; j++) {                                                   \
    CARD##bpp *srcptr2 = srcptr;                                              \
    unsigned char *dst2 = dst;                                                \
    for (i = 0; i < w; i++) {                                                 \
      pix = *srcptr2++;                                                       \
      inRed =                                                                 \
        (int)(pix >> rfbServerFormat.redShift & rfbServerFormat.redMax);      \
      inGreen =                                                               \
        (int)(pix >> rfbServerFormat.greenShift & rfbServerFormat.greenMax);  \
      inBlue =                                                                \
        (int)(pix >> rfbServerFormat.blueShift & rfbServerFormat.blueMax);    \
      *dst2++ = (CARD8)((inRed * 255 + rfbServerFormat.redMax / 2) /          \
                        rfbServerFormat.redMax);                              \
      *dst2++ = (CARD8)((inGreen * 255 + rfbServerFormat.greenMax / 2) /      \
                        rfbServerFormat.greenMax);                            \
      *dst2++ = (CARD8)((inBlue  * 255 + rfbServerFormat.blueMax / 2) /       \
                        rfbServerFormat.blueMax);                             \
    }                                                                         \
    srcptr += rfbFB.paddedWidthInBytes / ps;                                  \
    dst += w * 3;                                                             \
  }                                                                           \
  return tmpbuf;                                                              \
}

DEFINE_RGB_CONVERT_FUNCTION(16)
DEFINE_RGB_CONVERT_FUNCTION(32)


/*
 * JPEG compression stuff.
 */

static Bool SendJpegRect(threadparam *t, int x, int y, int w, int h,
                         int quality)
{
  unsigned char *srcbuf;
  int ps = rfbServerFormat.bitsPerPixel / 8;
  int subsamp = subsampLevel2tjsubsamp[subsampLevel];
  unsigned long size = 0;
  int flags = 0, pitch;
  unsigned char *tmpbuf = NULL;
  unsigned long jpegDstDataLen;
  rfbClientPtr cl = t->cl;

  if (rfbServerFormat.bitsPerPixel == 8) {
    ADD_TO_LOSSLESS_REGION(x, y, w, h);
    return SendFullColorRect(t, w, h);
  }

  if (ps < 2) {
    rfbLog("Error: JPEG requires 16-bit, 24-bit, or 32-bit pixel format.\n");
    return 0;
  }
  if (!t->j) {
    if ((t->j = tjInitCompress()) == NULL) {
      rfbLog("JPEG Error: %s\n", tjGetErrorStr());
      return 0;
    }
  }

  if (t->tightAfterBufSize < TJBUFSIZE(w, h)) {
    if (t->tightAfterBuf == NULL)
      t->tightAfterBuf = (char *)rfbAlloc(TJBUFSIZE(w, h));
    else
      t->tightAfterBuf = (char *)rfbRealloc(t->tightAfterBuf, TJBUFSIZE(w, h));
    t->tightAfterBufSize = TJBUFSIZE(w, h);
  }

  if (ps == 2) {
    tmpbuf = ConvertRGB16(cl, x, y, w, h);
    srcbuf = tmpbuf;
    pitch = w * 3;
    ps = 3;
  } else if (rfbServerFormat.depth == 30) {
    tmpbuf = ConvertRGB32(cl, x, y, w, h);
    srcbuf = tmpbuf;
    pitch = w * 3;
    ps = 3;
  } else {
    if (rfbServerFormat.bigEndian && ps == 4) flags |= TJ_ALPHAFIRST;
    if (rfbServerFormat.redShift == 16 && rfbServerFormat.blueShift == 0)
      flags |= TJ_BGR;
    if (rfbServerFormat.bigEndian) flags ^= TJ_BGR;
    srcbuf = (unsigned char *)
             &cl->fb[y * rfbFB.paddedWidthInBytes + x * ps];
    pitch = rfbFB.paddedWidthInBytes;
  }

  if (tjCompress(t->j, srcbuf, w, pitch, h, ps,
                 (unsigned char *)t->tightAfterBuf, &size, subsamp, quality,
                 flags) == -1) {
    rfbLog("JPEG Error: %s\n", tjGetErrorStr());
    free(tmpbuf);  tmpbuf = NULL;
    return 0;
  }
  jpegDstDataLen = (int)size;

  free(tmpbuf);  tmpbuf = NULL;

  if (!CheckUpdateBuf(t, TIGHT_MIN_TO_COMPRESS + 1))
    return FALSE;

  t->updateBuf[(*t->ublen)++] = (char)(rfbTightJpeg << 4);
  t->bytessent++;

  ADD_TO_LOSSY_REGION(x, y, w, h);

  return SendCompressedData(t, t->tightAfterBuf, jpegDstDataLen);
}
