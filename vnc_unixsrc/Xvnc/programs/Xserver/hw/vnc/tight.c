/*
 * tight.c
 *
 * Routines to implement Tight Encoding
 */

/*
 *  Copyright (C) 2000, 2001 Const Kaplinsky.  All Rights Reserved.
 *  Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 *  USA.
 */

#include <stdio.h>
#include "rfb.h"
#include "hpjpeg.h"


/* Note: The following constant should not be changed. */
#define TIGHT_MIN_TO_COMPRESS 12

/* The parameters below may be adjusted. */
#define MIN_SPLIT_RECT_SIZE     4096
#define MIN_SOLID_SUBRECT_SIZE  2048
#define MAX_SPLIT_TILE_SIZE       16

/* This variable is set on every rfbSendRectEncodingTight() call. */
static Bool usePixelFormat24;


/* Compression level stuff. The following array contains various
   encoder parameters for each of 10 compression levels (0..9).
   Last three parameters correspond to JPEG quality levels (0..9). */

typedef struct TIGHT_CONF_s {
    int maxRectSize, maxRectWidth;
} TIGHT_CONF;

static TIGHT_CONF tightConf[1] = {
#if 0
    {   512,   32 },
    {  2048,  128 },
    {  6144,  256 },
    { 10240, 1024 },
    { 16384, 2048 },
    { 32768, 2048 },
    { 65536, 2048 },
    { 65536, 2048 },
    { 65536, 2048 },
#endif
    { 65536, 2048 }
};

static int compressLevel;
static int qualityLevel;

/* Pointers to dynamically-allocated buffers. */

static int tightBeforeBufSize = 0;
static char *tightBeforeBuf = NULL;

static int tightAfterBufSize = 0;
static char *tightAfterBuf = NULL;

static int *prevRowBuf = NULL;


/* Prototypes for static functions. */

static void FindBestSolidArea (int x, int y, int w, int h,
                               CARD32 colorValue, int *w_ptr, int *h_ptr);
static void ExtendSolidArea   (int x, int y, int w, int h,
                               CARD32 colorValue,
                               int *x_ptr, int *y_ptr, int *w_ptr, int *h_ptr);
static Bool CheckSolidTile    (int x, int y, int w, int h,
                               CARD32 *colorPtr, Bool needSameColor);
static Bool CheckSolidTile8   (int x, int y, int w, int h,
                               CARD32 *colorPtr, Bool needSameColor);
static Bool CheckSolidTile16  (int x, int y, int w, int h,
                               CARD32 *colorPtr, Bool needSameColor);
static Bool CheckSolidTile32  (int x, int y, int w, int h,
                               CARD32 *colorPtr, Bool needSameColor);

static Bool SendRectSimple    (rfbClientPtr cl, int x, int y, int w, int h);
static Bool SendSubrect       (rfbClientPtr cl, int x, int y, int w, int h);
static Bool SendTightHeader   (rfbClientPtr cl, int x, int y, int w, int h);

static Bool SendSolidRect     (rfbClientPtr cl);

static Bool SendCompressedData(rfbClientPtr cl, int compressedLen);

static void Pack24(char *buf, rfbPixelFormat *fmt, int count);

static Bool SendJpegRect(rfbClientPtr cl, int x, int y, int w, int h,
                         int quality);

/*
 * Tight encoding implementation.
 */

int
rfbNumCodedRectsTight(cl, x, y, w, h)
    rfbClientPtr cl;
    int x, y, w, h;
{
    int maxRectSize, maxRectWidth;
    int subrectMaxWidth, subrectMaxHeight;

    /* No matter how many rectangles we will send if LastRect markers
       are used to terminate rectangle stream. */
    if (cl->enableLastRectEncoding && w * h >= MIN_SPLIT_RECT_SIZE)
      return 0;

    maxRectSize = tightConf[0].maxRectSize;
    maxRectWidth = tightConf[0].maxRectWidth;

    if (w > maxRectWidth || w * h > maxRectSize) {
        subrectMaxWidth = (w > maxRectWidth) ? maxRectWidth : w;
        subrectMaxHeight = maxRectSize / subrectMaxWidth;
        return (((w - 1) / maxRectWidth + 1) *
                ((h - 1) / subrectMaxHeight + 1));
    } else {
        return 1;
    }
}

Bool
rfbSendRectEncodingTight(cl, x, y, w, h)
    rfbClientPtr cl;
    int x, y, w, h;
{
    int nMaxRows;
    CARD32 colorValue;
    int dx, dy, dw, dh;
    int x_best, y_best, w_best, h_best;
    char *fbptr;

    compressLevel = cl->tightCompressLevel;
    qualityLevel = cl->tightQualityLevel;

    if ( cl->format.depth == 24 && cl->format.redMax == 0xFF &&
         cl->format.greenMax == 0xFF && cl->format.blueMax == 0xFF ) {
        usePixelFormat24 = TRUE;
    } else {
        usePixelFormat24 = FALSE;
    }

    if (!cl->enableLastRectEncoding || w * h < MIN_SPLIT_RECT_SIZE)
        return SendRectSimple(cl, x, y, w, h);

    /* Make sure we can write at least one pixel into tightBeforeBuf. */

    if (tightBeforeBufSize < 4) {
        tightBeforeBufSize = 4;
        if (tightBeforeBuf == NULL)
            tightBeforeBuf = (char *)xalloc(tightBeforeBufSize);
        else
            tightBeforeBuf = (char *)xrealloc(tightBeforeBuf,
                                              tightBeforeBufSize);
    }

    /* Calculate maximum number of rows in one non-solid rectangle. */

    {
        int maxRectSize, maxRectWidth, nMaxWidth;

        maxRectSize = tightConf[0].maxRectSize;
        maxRectWidth = tightConf[0].maxRectWidth;
        nMaxWidth = (w > maxRectWidth) ? maxRectWidth : w;
        nMaxRows = maxRectSize / nMaxWidth;
    }

    /* Try to find large solid-color areas and send them separately. */

    for (dy = y; dy < y + h; dy += MAX_SPLIT_TILE_SIZE) {

        /* If a rectangle becomes too large, send its upper part now. */

        if (dy - y >= nMaxRows) {
            if (!SendRectSimple(cl, x, y, w, nMaxRows))
                return 0;
            y += nMaxRows;
            h -= nMaxRows;
        }

        dh = (dy + MAX_SPLIT_TILE_SIZE <= y + h) ?
            MAX_SPLIT_TILE_SIZE : (y + h - dy);

        for (dx = x; dx < x + w; dx += MAX_SPLIT_TILE_SIZE) {

            dw = (dx + MAX_SPLIT_TILE_SIZE <= x + w) ?
                MAX_SPLIT_TILE_SIZE : (x + w - dx);

            if (CheckSolidTile(dx, dy, dw, dh, &colorValue, FALSE)) {

                /* Get dimensions of solid-color area. */

                FindBestSolidArea(dx, dy, w - (dx - x), h - (dy - y),
				  colorValue, &w_best, &h_best);

                /* Make sure a solid rectangle is large enough
                   (or the whole rectangle is of the same color). */

                if ( w_best * h_best != w * h &&
                     w_best * h_best < MIN_SOLID_SUBRECT_SIZE )
                    continue;

                /* Try to extend solid rectangle to maximum size. */

                x_best = dx; y_best = dy;
                ExtendSolidArea(x, y, w, h, colorValue,
                                &x_best, &y_best, &w_best, &h_best);

                /* Send rectangles at top and left to solid-color area. */

                if ( y_best != y &&
                     !SendRectSimple(cl, x, y, w, y_best-y) )
                    return FALSE;
                if ( x_best != x &&
                     !rfbSendRectEncodingTight(cl, x, y_best,
                                               x_best-x, h_best) )
                    return FALSE;

                /* Send solid-color rectangle. */

                if (!SendTightHeader(cl, x_best, y_best, w_best, h_best))
                    return FALSE;

                fbptr = (rfbScreen.pfbMemory +
                         (rfbScreen.paddedWidthInBytes * y_best) +
                         (x_best * (rfbScreen.bitsPerPixel / 8)));

                (*cl->translateFn)(cl->translateLookupTable, &rfbServerFormat,
                                   &cl->format, fbptr, tightBeforeBuf,
                                   rfbScreen.paddedWidthInBytes, 1, 1);

                if (!SendSolidRect(cl))
                    return FALSE;

                /* Send remaining rectangles (at right and bottom). */

                if ( x_best + w_best != x + w &&
                     !rfbSendRectEncodingTight(cl, x_best+w_best, y_best,
                                               w-(x_best-x)-w_best, h_best) )
                    return FALSE;
                if ( y_best + h_best != y + h &&
                     !rfbSendRectEncodingTight(cl, x, y_best+h_best,
                                               w, h-(y_best-y)-h_best) )
                    return FALSE;

                /* Return after all recursive calls are done. */

                return TRUE;
            }

        }

    }

    /* No suitable solid-color rectangles found. */

    return SendRectSimple(cl, x, y, w, h);
}

static void
FindBestSolidArea(x, y, w, h, colorValue, w_ptr, h_ptr)
    int x, y, w, h;
    CARD32 colorValue;
    int *w_ptr, *h_ptr;
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

        if (!CheckSolidTile(x, dy, dw, dh, &colorValue, TRUE))
            break;

        for (dx = x + dw; dx < x + w_prev;) {
            dw = (dx + MAX_SPLIT_TILE_SIZE <= x + w_prev) ?
                MAX_SPLIT_TILE_SIZE : (x + w_prev - dx);
            if (!CheckSolidTile(dx, dy, dw, dh, &colorValue, TRUE))
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

static void
ExtendSolidArea(x, y, w, h, colorValue, x_ptr, y_ptr, w_ptr, h_ptr)
    int x, y, w, h;
    CARD32 colorValue;
    int *x_ptr, *y_ptr, *w_ptr, *h_ptr;
{
    int cx, cy;

    /* Try to extend the area upwards. */
    for ( cy = *y_ptr - 1;
          cy >= y && CheckSolidTile(*x_ptr, cy, *w_ptr, 1, &colorValue, TRUE);
          cy-- );
    *h_ptr += *y_ptr - (cy + 1);
    *y_ptr = cy + 1;

    /* ... downwards. */
    for ( cy = *y_ptr + *h_ptr;
          cy < y + h &&
              CheckSolidTile(*x_ptr, cy, *w_ptr, 1, &colorValue, TRUE);
          cy++ );
    *h_ptr += cy - (*y_ptr + *h_ptr);

    /* ... to the left. */
    for ( cx = *x_ptr - 1;
          cx >= x && CheckSolidTile(cx, *y_ptr, 1, *h_ptr, &colorValue, TRUE);
          cx-- );
    *w_ptr += *x_ptr - (cx + 1);
    *x_ptr = cx + 1;

    /* ... to the right. */
    for ( cx = *x_ptr + *w_ptr;
          cx < x + w &&
              CheckSolidTile(cx, *y_ptr, 1, *h_ptr, &colorValue, TRUE);
          cx++ );
    *w_ptr += cx - (*x_ptr + *w_ptr);
}

/*
 * Check if a rectangle is all of the same color. If needSameColor is
 * set to non-zero, then also check that its color equals to the
 * *colorPtr value. The result is 1 if the test is successfull, and in
 * that case new color will be stored in *colorPtr.
 */

static Bool
CheckSolidTile(x, y, w, h, colorPtr, needSameColor)
    int x, y, w, h;
    CARD32 *colorPtr;
    Bool needSameColor;
{
    switch(rfbServerFormat.bitsPerPixel) {
    case 32:
        return CheckSolidTile32(x, y, w, h, colorPtr, needSameColor);
    case 16:
        return CheckSolidTile16(x, y, w, h, colorPtr, needSameColor);
    default:
        return CheckSolidTile8(x, y, w, h, colorPtr, needSameColor);
    }
}

#define DEFINE_CHECK_SOLID_FUNCTION(bpp)                                      \
                                                                              \
static Bool                                                                   \
CheckSolidTile##bpp(x, y, w, h, colorPtr, needSameColor)                      \
    int x, y;                                                                 \
    CARD32 *colorPtr;                                                         \
    Bool needSameColor;                                                       \
{                                                                             \
    CARD##bpp *fbptr;                                                         \
    CARD##bpp colorValue;                                                     \
    int dx, dy;                                                               \
                                                                              \
    fbptr = (CARD##bpp *)                                                     \
        &rfbScreen.pfbMemory[y * rfbScreen.paddedWidthInBytes + x * (bpp/8)]; \
                                                                              \
    colorValue = *fbptr;                                                      \
    if (needSameColor && (CARD32)colorValue != *colorPtr)                     \
        return FALSE;                                                         \
                                                                              \
    for (dy = 0; dy < h; dy++) {                                              \
        for (dx = 0; dx < w; dx++) {                                          \
            if (colorValue != fbptr[dx])                                      \
                return FALSE;                                                 \
        }                                                                     \
        fbptr = (CARD##bpp *)((CARD8 *)fbptr + rfbScreen.paddedWidthInBytes); \
    }                                                                         \
                                                                              \
    *colorPtr = (CARD32)colorValue;                                           \
    return TRUE;                                                              \
}

DEFINE_CHECK_SOLID_FUNCTION(8)
DEFINE_CHECK_SOLID_FUNCTION(16)
DEFINE_CHECK_SOLID_FUNCTION(32)

static Bool
SendRectSimple(cl, x, y, w, h)
    rfbClientPtr cl;
    int x, y, w, h;
{
    int maxBeforeSize, maxAfterSize;
    int maxRectSize, maxRectWidth;
    int subrectMaxWidth, subrectMaxHeight;
    int dx, dy;
    int rw, rh;

    maxRectSize = tightConf[0].maxRectSize;
    maxRectWidth = tightConf[0].maxRectWidth;

    maxBeforeSize = maxRectSize * (cl->format.bitsPerPixel / 8);
    maxAfterSize = maxBeforeSize + (maxBeforeSize + 99) / 100 + 12;

    if (tightBeforeBufSize < maxBeforeSize) {
        tightBeforeBufSize = maxBeforeSize;
        if (tightBeforeBuf == NULL)
            tightBeforeBuf = (char *)xalloc(tightBeforeBufSize);
        else
            tightBeforeBuf = (char *)xrealloc(tightBeforeBuf,
                                              tightBeforeBufSize);
    }

    if (tightAfterBufSize < maxAfterSize) {
        tightAfterBufSize = maxAfterSize;
        if (tightAfterBuf == NULL)
            tightAfterBuf = (char *)xalloc(tightAfterBufSize);
        else
            tightAfterBuf = (char *)xrealloc(tightAfterBuf,
                                             tightAfterBufSize);
    }

    if (w > maxRectWidth || w * h > maxRectSize) {
        subrectMaxWidth = (w > maxRectWidth) ? maxRectWidth : w;
        subrectMaxHeight = maxRectSize / subrectMaxWidth;

        for (dy = 0; dy < h; dy += subrectMaxHeight) {
            for (dx = 0; dx < w; dx += maxRectWidth) {
                rw = (dx + maxRectWidth < w) ? maxRectWidth : w - dx;
                rh = (dy + subrectMaxHeight < h) ? subrectMaxHeight : h - dy;
                if (!SendSubrect(cl, x+dx, y+dy, rw, rh))
                    return FALSE;
            }
        }
    } else {
        if (!SendSubrect(cl, x, y, w, h))
            return FALSE;
    }

    return TRUE;
}

static Bool
SendSubrect(cl, x, y, w, h)
    rfbClientPtr cl;
    int x, y, w, h;
{
    char *fbptr;
    Bool success = FALSE;

    /* Send pending data if there is more than 128 bytes. */
    if (ublen > 128) {
        if (!rfbSendUpdateBuf(cl))
            return FALSE;
    }

    if (!SendTightHeader(cl, x, y, w, h))
        return FALSE;

    fbptr = (rfbScreen.pfbMemory + (rfbScreen.paddedWidthInBytes * y)
             + (x * (rfbScreen.bitsPerPixel / 8)));

    (*cl->translateFn)(cl->translateLookupTable, &rfbServerFormat,
                       &cl->format, fbptr, tightBeforeBuf,
                       rfbScreen.paddedWidthInBytes, w, h);

    success = SendJpegRect(cl, x, y, w, h, qualityLevel);

    return success;
}

static Bool
SendTightHeader(cl, x, y, w, h)
    rfbClientPtr cl;
    int x, y, w, h;
{
    rfbFramebufferUpdateRectHeader rect;

    if (ublen + sz_rfbFramebufferUpdateRectHeader > UPDATE_BUF_SIZE) {
        if (!rfbSendUpdateBuf(cl))
            return FALSE;
    }

    rect.r.x = Swap16IfLE(x);
    rect.r.y = Swap16IfLE(y);
    rect.r.w = Swap16IfLE(w);
    rect.r.h = Swap16IfLE(h);
    rect.encoding = Swap32IfLE(rfbEncodingTight);

    memcpy(&updateBuf[ublen], (char *)&rect,
           sz_rfbFramebufferUpdateRectHeader);
    ublen += sz_rfbFramebufferUpdateRectHeader;

    cl->rfbRectanglesSent[rfbEncodingTight]++;
    cl->rfbBytesSent[rfbEncodingTight] += sz_rfbFramebufferUpdateRectHeader;

    return TRUE;
}

/*
 * Subencoding implementations.
 */

static Bool
SendSolidRect(cl)
    rfbClientPtr cl;
{
    int len;

    if (usePixelFormat24) {
        Pack24(tightBeforeBuf, &cl->format, 1);
        len = 3;
    } else
        len = cl->format.bitsPerPixel / 8;

    if (ublen + 1 + len > UPDATE_BUF_SIZE) {
        if (!rfbSendUpdateBuf(cl))
            return FALSE;
    }

    updateBuf[ublen++] = (char)(rfbTightFill << 4);
    memcpy (&updateBuf[ublen], tightBeforeBuf, len);
    ublen += len;

    cl->rfbBytesSent[rfbEncodingTight] += len + 1;

    return TRUE;
}

static Bool SendCompressedData(cl, compressedLen)
    rfbClientPtr cl;
    int compressedLen;
{
    int i, portionLen;

    updateBuf[ublen++] = compressedLen & 0x7F;
    cl->rfbBytesSent[rfbEncodingTight]++;
    if (compressedLen > 0x7F) {
        updateBuf[ublen-1] |= 0x80;
        updateBuf[ublen++] = compressedLen >> 7 & 0x7F;
        cl->rfbBytesSent[rfbEncodingTight]++;
        if (compressedLen > 0x3FFF) {
            updateBuf[ublen-1] |= 0x80;
            updateBuf[ublen++] = compressedLen >> 14 & 0xFF;
            cl->rfbBytesSent[rfbEncodingTight]++;
        }
    }

    portionLen = UPDATE_BUF_SIZE;
    for (i = 0; i < compressedLen; i += portionLen) {
        if (i + portionLen > compressedLen) {
            portionLen = compressedLen - i;
        }
        if (ublen + portionLen > UPDATE_BUF_SIZE) {
            if (!rfbSendUpdateBuf(cl))
                return FALSE;
        }
        memcpy(&updateBuf[ublen], &tightAfterBuf[i], portionLen);
        ublen += portionLen;
    }
    cl->rfbBytesSent[rfbEncodingTight] += compressedLen;
    return TRUE;
}

/*
 * Converting 32-bit color samples into 24-bit colors.
 * Should be called only when redMax, greenMax and blueMax are 255.
 * Color components assumed to be byte-aligned.
 */

static void Pack24(buf, fmt, count)
    char *buf;
    rfbPixelFormat *fmt;
    int count;
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
 * JPEG compression stuff.
 */

static unsigned long jpegDstDataLen;
static hpjhandle j=NULL;

static Bool
SendJpegRect(cl, x, y, w, h, quality)
    rfbClientPtr cl;
    int x, y, w, h;
    int quality;
{
    int dy;
    char *srcbuf;
    int ps=rfbServerFormat.bitsPerPixel/8;
    int subsamp=compressLevel==1? HPJ_411: (compressLevel==2? HPJ_422:HPJ_444);
    unsigned long size;

    if (ps < 3) {
      rfbLog("Error: Server must be run with 24-bit or 32-bit depth\n");  return 0;
    }

    if(!j) {
      if((j=hpjInitCompress())==NULL) {
        rfbLog("JPEG Error: %s\n", hpjGetErrorStr());  return 0;
      }
    }

    if (tightAfterBufSize < HPJBUFSIZE(w,h)) {
        if (tightAfterBuf == NULL)
            tightAfterBuf = (char *)xalloc(HPJBUFSIZE(w,h));
        else
            tightAfterBuf = (char *)xrealloc(tightAfterBuf,
                                             HPJBUFSIZE(w,h));
        if(!tightAfterBuf) {
            rfbLog("Memory allocation failure!\n");
            return 0;
        }
        tightAfterBufSize = HPJBUFSIZE(w,h);
    }

    srcbuf=&rfbScreen.pfbMemory[y * rfbScreen.paddedWidthInBytes + x * ps];
    if(hpjCompress(j, (unsigned char *)srcbuf, w, rfbScreen.paddedWidthInBytes,
      h, ps, (unsigned char *)tightAfterBuf, &jpegDstDataLen, subsamp, quality,
      rfbServerFormat.bigEndian?0:HPJ_BGR)==-1) {
      rfbLog("JPEG Error: %s\n", hpjGetErrorStr());
      return 0;
    }

    if (ublen + TIGHT_MIN_TO_COMPRESS + 1 > UPDATE_BUF_SIZE) {
        if (!rfbSendUpdateBuf(cl))
            return FALSE;
    }

    updateBuf[ublen++] = (char)(rfbTightJpeg << 4);
    cl->rfbBytesSent[rfbEncodingTight]++;

    return SendCompressedData(cl, jpegDstDataLen);
}
