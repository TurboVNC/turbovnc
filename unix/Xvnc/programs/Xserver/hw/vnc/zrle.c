/* Copyright (C) 2012, 2014, 2017 D. R. Commander.  All Rights Reserved.
 * Copyright (C) 2003 Sun Microsystems, Inc.
 * Copyright (C) 2002 RealVNC Ltd.  All Rights Reserved.
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
 * zrle.c
 *
 * Routines to implement Zlib Run-length Encoding (ZRLE).
 */

#include "rfb.h"
#include "zrleoutstream.h"


#define GET_IMAGE_INTO_BUF(tx, ty, tw, th, buf) {                  \
  char *fbptr = (cl->fb + (rfbFB.paddedWidthInBytes * ty) +        \
                 (tx * (rfbFB.bitsPerPixel / 8)));                 \
                                                                   \
  (*cl->translateFn) (cl->translateLookupTable, &rfbServerFormat,  \
                      &cl->format, fbptr, (char *)buf,             \
                      rfbFB.paddedWidthInBytes, tw, th);           \
}

#define EXTRA_ARGS , rfbClientPtr cl

#define ENDIAN_LITTLE 0
#define ENDIAN_BIG 1
#define ENDIAN_NO 2
#define BPP 8
#define ZYWRLE_ENDIAN ENDIAN_NO
#include <zrleencodetemplate.c>
#undef BPP
#define BPP 15
#undef ZYWRLE_ENDIAN
#define ZYWRLE_ENDIAN ENDIAN_LITTLE
#include <zrleencodetemplate.c>
#undef ZYWRLE_ENDIAN
#define ZYWRLE_ENDIAN ENDIAN_BIG
#include <zrleencodetemplate.c>
#undef BPP
#define BPP 16
#undef ZYWRLE_ENDIAN
#define ZYWRLE_ENDIAN ENDIAN_LITTLE
#include <zrleencodetemplate.c>
#undef ZYWRLE_ENDIAN
#define ZYWRLE_ENDIAN ENDIAN_BIG
#include <zrleencodetemplate.c>
#undef BPP
#define BPP 32
#undef ZYWRLE_ENDIAN
#define ZYWRLE_ENDIAN ENDIAN_LITTLE
#include <zrleencodetemplate.c>
#undef ZYWRLE_ENDIAN
#define ZYWRLE_ENDIAN ENDIAN_BIG
#include <zrleencodetemplate.c>
#define CPIXEL 24A
#undef ZYWRLE_ENDIAN
#define ZYWRLE_ENDIAN ENDIAN_LITTLE
#include <zrleencodetemplate.c>
#undef ZYWRLE_ENDIAN
#define ZYWRLE_ENDIAN ENDIAN_BIG
#include <zrleencodetemplate.c>
#undef CPIXEL
#define CPIXEL 24B
#undef ZYWRLE_ENDIAN
#define ZYWRLE_ENDIAN ENDIAN_LITTLE
#include <zrleencodetemplate.c>
#undef ZYWRLE_ENDIAN
#define ZYWRLE_ENDIAN ENDIAN_BIG
#include <zrleencodetemplate.c>
#undef CPIXEL
#undef BPP


/*
 * zrleBeforeBuf contains pixel data in the client's format.  It must be at
 * least one pixel bigger than the largest tile of pixel data, since the
 * ZRLE encoding algorithm writes to the position one past the end of the pixel
 * data.
 */


/*
 * rfbSendRectEncodingZRLE - send a given rectangle using ZRLE encoding.
 */

Bool rfbSendRectEncodingZRLE(rfbClientPtr cl, int x, int y, int w, int h)
{
  zrleOutStream *zos;
  rfbFramebufferUpdateRectHeader rect;
  rfbZRLEHeader hdr;
  int i;
  char *zrleBeforeBuf;

  if (cl->zrleBeforeBuf == NULL) {
    cl->zrleBeforeBuf = (char *)rfbAlloc(rfbZRLETileWidth *
                                         rfbZRLETileHeight * 4 + 4);
  }
  zrleBeforeBuf = cl->zrleBeforeBuf;

  if (cl->preferredEncoding == rfbEncodingZYWRLE) {
    if (cl->imageQualityLevel < 0) {
      cl->zywrleLevel = 1;
    } else if (cl->imageQualityLevel < 3) {
      cl->zywrleLevel = 3;
    } else if (cl->imageQualityLevel < 6) {
      cl->zywrleLevel = 2;
    } else {
      cl->zywrleLevel = 1;
    }
  } else
    cl->zywrleLevel = 0;

  if (!cl->zrleData)
    cl->zrleData = zrleOutStreamNew();
  zos = cl->zrleData;
  zos->in.ptr = zos->in.start;
  zos->out.ptr = zos->out.start;

  switch (cl->format.bitsPerPixel) {

    case 8:
      zrleEncode8NE(x, y, w, h, zos, zrleBeforeBuf, cl);
      break;

    case 16:
      if (cl->format.greenMax > 0x1F) {
        if (cl->format.bigEndian)
          zrleEncode16BE(x, y, w, h, zos, zrleBeforeBuf, cl);
        else
          zrleEncode16LE(x, y, w, h, zos, zrleBeforeBuf, cl);
      } else {
        if (cl->format.bigEndian)
          zrleEncode15BE(x, y, w, h, zos, zrleBeforeBuf, cl);
        else
          zrleEncode15LE(x, y, w, h, zos, zrleBeforeBuf, cl);
      }
      break;

    case 32: {
      Bool fitsInLS3Bytes =
        ((cl->format.redMax   << cl->format.redShift)   < (1 << 24) &&
         (cl->format.greenMax << cl->format.greenShift) < (1 << 24) &&
         (cl->format.blueMax  << cl->format.blueShift)  < (1 << 24));

      Bool fitsInMS3Bytes = (cl->format.redShift   > 7  &&
                             cl->format.greenShift > 7  &&
                             cl->format.blueShift  > 7);

      if ((fitsInLS3Bytes && !cl->format.bigEndian) ||
          (fitsInMS3Bytes && cl->format.bigEndian)) {
        if (cl->format.bigEndian)
          zrleEncode24ABE(x, y, w, h, zos, zrleBeforeBuf, cl);
        else
          zrleEncode24ALE(x, y, w, h, zos, zrleBeforeBuf, cl);

      } else if ((fitsInLS3Bytes && cl->format.bigEndian) ||
                 (fitsInMS3Bytes && !cl->format.bigEndian)) {
        if (cl->format.bigEndian)
          zrleEncode24BBE(x, y, w, h, zos, zrleBeforeBuf, cl);
        else
          zrleEncode24BLE(x, y, w, h, zos, zrleBeforeBuf, cl);

      } else {
        if (cl->format.bigEndian)
          zrleEncode32BE(x, y, w, h, zos, zrleBeforeBuf, cl);
        else
          zrleEncode32LE(x, y, w, h, zos, zrleBeforeBuf, cl);
      }
      break;
    }
  }

  cl->rfbBytesSent[rfbEncodingZRLE] += sz_rfbFramebufferUpdateRectHeader +
      sz_rfbZRLEHeader + ZRLE_BUFFER_LENGTH(&zos->out);
  cl->rfbRectanglesSent[rfbEncodingZRLE]++;

  if (ublen + sz_rfbFramebufferUpdateRectHeader + sz_rfbZRLEHeader >
      UPDATE_BUF_SIZE) {
    if (!rfbSendUpdateBuf(cl))
      return FALSE;
  }

  rect.r.x = Swap16IfLE(x);
  rect.r.y = Swap16IfLE(y);
  rect.r.w = Swap16IfLE(w);
  rect.r.h = Swap16IfLE(h);
  rect.encoding = Swap32IfLE(cl->preferredEncoding);

  memcpy(updateBuf + ublen, (char *)&rect,
         sz_rfbFramebufferUpdateRectHeader);
  ublen += sz_rfbFramebufferUpdateRectHeader;

  hdr.length = Swap32IfLE(ZRLE_BUFFER_LENGTH(&zos->out));

  memcpy(updateBuf + ublen, (char *)&hdr, sz_rfbZRLEHeader);
  ublen += sz_rfbZRLEHeader;

  /* copy into updateBuf and send from there.  Maybe should send directly? */

  for (i = 0; i < ZRLE_BUFFER_LENGTH(&zos->out);) {

    int bytesToCopy = UPDATE_BUF_SIZE - ublen;

    if (i + bytesToCopy > ZRLE_BUFFER_LENGTH(&zos->out)) {
      bytesToCopy = ZRLE_BUFFER_LENGTH(&zos->out) - i;
    }

    memcpy(updateBuf + ublen, (CARD8 *)zos->out.start + i, bytesToCopy);

    ublen += bytesToCopy;
    i += bytesToCopy;

    if (ublen == UPDATE_BUF_SIZE) {
      if (!rfbSendUpdateBuf(cl))
        return FALSE;
    }
  }

  return TRUE;
}


void rfbFreeZrleData(rfbClientPtr cl)
{
  if (cl->zrleData) {
    zrleOutStreamFree(cl->zrleData);
  }
  cl->zrleData = NULL;

  free(cl->zrleBeforeBuf);
  cl->zrleBeforeBuf = NULL;

  free(cl->paletteHelper);
  cl->paletteHelper = NULL;
}
