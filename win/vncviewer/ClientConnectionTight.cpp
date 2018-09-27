//  Copyright (C) 2010, 2015 D. R. Commander. All Rights Reserved.
//  Copyright (C) 2005 Sun Microsystems, Inc. All Rights Reserved.
//  Copyright (C) 2004 Landmark Graphics Corporation. All Rights Reserved.
//  Copyright (C) 2000, 2001 Constantin Kaplinsky. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
//
//  This file is part of the VNC system.
//
//  The VNC system is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
//  USA.

#include "stdhdrs.h"
#include "vncviewer.h"
#include "ClientConnection.h"
#include "Exception.h"

#define TIGHT_MIN_TO_COMPRESS 12
#define TIGHT_BUFFER_SIZE (2048 * 200)


void ClientConnection::ReadTightRect(rfbFramebufferUpdateRectHeader *pfburh)
{
  if (m_myFormat.bitsPerPixel != 8 &&
      m_myFormat.bitsPerPixel != 16 &&
      m_myFormat.bitsPerPixel != 32) {
    vnclog.Print(0, "Invalid number of bits per pixel: %d\n",
                 m_myFormat.bitsPerPixel);
    return;
  }

  CARD8 comp_ctl;
  ReadExact((char *)&comp_ctl, 1);

  // Flush Zlib streams if we are told by the server to do so
  for (int i = 0; i < 4; i++) {
    if ((comp_ctl & 1) && m_tightZlibStreamActive[i]) {
      int err = inflateEnd(&m_tightZlibStream[i]);
      if (err != Z_OK) {
        if (m_tightZlibStream[i].msg != NULL)
          vnclog.Print(0, "zlib inflateEnd() error: %s\n",
                       m_tightZlibStream[i].msg);
        else
          vnclog.Print(0, "zlib inflateEnd() error: %d\n", err);
        return;
      }
      m_tightZlibStreamActive[i] = FALSE;
    }
    comp_ctl >>= 1;
  }

  bool readUncompressed = false;
  if ((comp_ctl & rfbTightNoZlib) == rfbTightNoZlib) {
    comp_ctl &= ~(rfbTightNoZlib);
    readUncompressed = true;
  }

  // Handle solid subrectangles
  if (comp_ctl == rfbTightFill) {
    COLORREF fillColor;
    if (m_myFormat.depth == 24 && m_myFormat.redMax == 0xFF &&
        m_myFormat.greenMax == 0xFF && m_myFormat.blueMax == 0xFF) {
      CARD8 fillColorBuf[3];
      ReadExact((char *)&fillColorBuf, 3);
      fillColor = COLOR_FROM_PIXEL24_ADDRESS(fillColorBuf);
    } else {
      CARD32 fillColorBuf;
      ReadExact((char *)&fillColorBuf, m_myFormat.bitsPerPixel / 8);

      SETUP_COLOR_SHORTCUTS;

      switch (m_myFormat.bitsPerPixel) {
        case 8:
          fillColor = COLOR_FROM_PIXEL8_ADDRESS(&fillColorBuf);
          break;
        case 16:
          fillColor = COLOR_FROM_PIXEL16_ADDRESS(&fillColorBuf);
          break;
        default:
          fillColor = COLOR_FROM_PIXEL32_ADDRESS(&fillColorBuf);
      }
    }

    if (m_opts.m_DoubleBuffer) {
      node->isFill = 1;
      node->fillColor = fillColor;
    } else {
      double tBlitStart;
      if (m_opts.m_benchFile) tBlitStart = getTime();
      omni_mutex_lock l(m_bitmapdcMutex);
      ObjectSelector b(m_hBitmapDC, m_hBitmap);
      PaletteSelector p(m_hBitmapDC, m_hPalette);
      FillSolidRect(pfburh->r.x, pfburh->r.y, pfburh->r.w, pfburh->r.h,
                    fillColor);
      if (m_opts.m_benchFile) tBlit += getTime() - tBlitStart;
    }
    return;
  }

  if (comp_ctl == rfbTightJpeg) {
    DecompressJpegRect(pfburh->r.x, pfburh->r.y, pfburh->r.w, pfburh->r.h);
    return;
  }

  // Quit on unsupported subencoding value
  if (comp_ctl >= rfbTightMaxSubencoding) {
    vnclog.Print(0, "Tight encoding: bad subencoding value received.\n");
    return;
  }

  // If we get here, then the subrectangle was encoded using indexed color,
  // raw, or mono subencoding with optional Zlib compression.

  // First, we should identify a filter to use
  int bitsPixel;
  if ((comp_ctl & rfbTightExplicitFilter) != 0) {
    CARD8 filter_id;
    ReadExact((char *)&filter_id, 1);

    switch (filter_id) {
      case rfbTightFilterCopy:
        bitsPixel = InitFilterCopy(pfburh->r.w, pfburh->r.h);
        break;
      case rfbTightFilterPalette:
        bitsPixel = InitFilterPalette(pfburh->r.w, pfburh->r.h);
        break;
      case rfbTightFilterGradient:
        bitsPixel = InitFilterGradient(pfburh->r.w, pfburh->r.h);
        break;
      default:
        vnclog.Print(0, "Tight encoding: unknown filter code received.\n");
        return;
    }
  } else
    bitsPixel = InitFilterCopy(pfburh->r.w, pfburh->r.h);
  if (bitsPixel == 0) {
    vnclog.Print(0, "Tight encoding: error receiving palette.\n");
    return;
  }

  // Determine if the data should be decompressed or just copied
  int rowSize = (pfburh->r.w * bitsPixel + 7) / 8;
  int bufsize = -1;

  if (pfburh->r.h * rowSize < TIGHT_MIN_TO_COMPRESS)
    bufsize = pfburh->r.h * rowSize;
  else if (readUncompressed)
    bufsize = ReadCompactLen();

  if (bufsize != -1) {
    CheckBufferSize(bufsize);
    ReadExact(m_netbuf, bufsize);
    (this->*m_tightCurrentFilter)(pfburh->r.x, pfburh->r.y, pfburh->r.h);
    return;
  }

  // Read the length of the compressed data that follows
  int compressedLen = ReadCompactLen();
  if (compressedLen <= 0) {
    vnclog.Print(0, "Tight encoding: bad data received from server.\n");
    return;
  }

  // Initialize compression stream, if needed
  int stream_id = comp_ctl & 0x03;
  z_streamp zs = &m_tightZlibStream[stream_id];
  if (!m_tightZlibStreamActive[stream_id]) {
    zs->zalloc = Z_NULL;
    zs->zfree = Z_NULL;
    zs->opaque = Z_NULL;
    int err = inflateInit(zs);
    if (err != Z_OK) {
      if (zs->msg != NULL)
        vnclog.Print(0, "zlib inflateInit() error: %s.\n", zs->msg);
      else
        vnclog.Print(0, "zlib inflateInit() error: %d.\n", err);
      return;
    }
    m_tightZlibStreamActive[stream_id] = TRUE;
  }

  // Read, decode, and draw actual pixel data in a loop
  CheckZlibBufferSize(compressedLen);
  CheckBufferSize(pfburh->r.h * rowSize);

  ReadExact((char *)m_zlibbuf, compressedLen);
  zs->next_in = (Bytef *)m_zlibbuf;
  zs->avail_in = compressedLen;
  zs->next_out = (Bytef *)m_netbuf;
  zs->avail_out = pfburh->r.h * rowSize;

  int err = inflate(zs, Z_SYNC_FLUSH);
  if (err != Z_OK && err != Z_STREAM_END) {
    if (zs->msg != NULL)
      vnclog.Print(0, "zlib inflate() error: %s.\n", zs->msg);
    else
      vnclog.Print(0, "zlib inflate() error: %d.\n", err);
    return;
  }

  (this->*m_tightCurrentFilter)(pfburh->r.x, pfburh->r.y, pfburh->r.h);
}


// The length of compressed data is transmitted as 1-3 bytes, in which each
// byte contains 7 bits of actual data, and the 8th bit is set to indicate
// that an additional byte should be read.

int ClientConnection::ReadCompactLen()
{
  CARD8 len_byte;
  ReadExact((char *)&len_byte, 1);
  int compressedLen = (int)len_byte & 0x7F;
  if (len_byte & 0x80) {
    ReadExact((char *)&len_byte, 1);
    compressedLen |= ((int)len_byte & 0x7F) << 7;
    if (len_byte & 0x80) {
      ReadExact((char *)&len_byte, 1);
      compressedLen |= ((int)len_byte & 0xFF) << 14;
    }
  }
  return compressedLen;
}


//----------------------------------------------------------------------------
//
// Filter stuff.
//

// The following variables are defined in the class declaration:
//   tightFilterFunc m_tightCurrentFilter;
//   Bool m_tightCutZeros;
//   int m_tightRectWidth, m_tightRectColors;
//   COLORREF m_tightPalette[256];
//   CARD8 m_tightPrevRow[2048 * 3 * sizeof(CARD16)];

int ClientConnection::InitFilterCopy(int rw, int rh)
{
  tightFilterFunc funcArray[3] = { &ClientConnection::FilterCopy8,
                                   &ClientConnection::FilterCopy16,
                                   &ClientConnection::FilterCopy32 };

  m_tightCurrentFilter = funcArray[m_myFormat.bitsPerPixel / 16];
  m_tightRectWidth = rw;

  if (m_myFormat.depth == 24 && m_myFormat.redMax == 0xFF &&
      m_myFormat.greenMax == 0xFF && m_myFormat.blueMax == 0xFF) {
    m_tightCutZeros = TRUE;
    m_tightCurrentFilter = &ClientConnection::FilterCopy24;
    return 24;
  }

  m_tightCutZeros = FALSE;
  return m_myFormat.bitsPerPixel;
}


int ClientConnection::InitFilterGradient(int rw, int rh)
{
  int bits = InitFilterCopy(rw, rh);

  tightFilterFunc funcArray[3] = { &ClientConnection::FilterGradient8,
                                   &ClientConnection::FilterGradient16,
                                   &ClientConnection::FilterGradient32 };

  m_tightCurrentFilter = funcArray[m_myFormat.bitsPerPixel / 16];

  if (m_tightCutZeros) {
    m_tightCurrentFilter = &ClientConnection::FilterGradient24;
    memset(m_tightPrevRow, 0, rw * 3);
  } else
    memset(m_tightPrevRow, 0, rw * 3 * sizeof(CARD16));

  return bits;
}


int ClientConnection::InitFilterPalette(int rw, int rh)
{
  m_tightCurrentFilter = &ClientConnection::FilterPalette;
  m_tightRectWidth = rw;

  CARD8 numColors;
  ReadExact((char *)&numColors, 1);

  m_tightRectColors = (int)numColors;
  if (++m_tightRectColors < 2)
    return 0;

  CARD32 drs = fbx_roffset[fb.format] * 8;
  CARD32 dgs = fbx_goffset[fb.format] * 8;
  CARD32 dbs = fbx_boffset[fb.format] * 8;

  if (m_myFormat.depth == 24 && m_myFormat.redMax == 0xFF &&
      m_myFormat.greenMax == 0xFF && m_myFormat.blueMax == 0xFF) {

    CheckBufferSize(m_tightRectColors * 3);
    ReadExact(m_netbuf, m_tightRectColors * 3);

    CARD8 *p = (CARD8 *)m_netbuf;
    for (int i = 0; i < m_tightRectColors; i++, p += 3)
      m_tightPalette[i] = ((CARD32)p[0] << drs) |
                          ((CARD32)p[1] << dgs) |
                          ((CARD32)p[2] << dbs);
  } else {
    CheckBufferSize(m_tightRectColors * (m_myFormat.bitsPerPixel / 8));
    ReadExact(m_netbuf, m_tightRectColors * (m_myFormat.bitsPerPixel / 8));

    SETUP_COLOR_SHORTCUTS;

    int i;
    switch (m_myFormat.bitsPerPixel) {
      case 8:
      {
        CARD8 *p = (CARD8 *)m_netbuf;
        for (i = 0; i < m_tightRectColors; i++, p++)
          m_tightPalette[i] =
            (((((CARD32)(*p) >> rs) & rm) * 255 / rm) << drs) |
            (((((CARD32)(*p) >> gs) & gm) * 255 / gm) << dgs) |
            (((((CARD32)(*p) >> bs) & bm) * 255 / bm) << dbs);
        break;
      }
      case 16:
      {
        CARD16 *p = (CARD16 *)m_netbuf;
        for (i = 0; i < m_tightRectColors; i++, p++)
          m_tightPalette[i] =
            (((((CARD32)(*p) >> rs) & rm) * 255 / rm) << drs) |
            (((((CARD32)(*p) >> gs) & gm) * 255 / gm) << dgs) |
            (((((CARD32)(*p) >> bs) & bm) * 255 / bm) << dbs);
        break;
      }
      default:
      {
        CARD32 *p = (CARD32 *)m_netbuf;
        for (i = 0; i < m_tightRectColors; i++, p++)
          m_tightPalette[i] =
            (((((CARD32)(*p) >> rs) & rm) * 255 / rm) << drs) |
            (((((CARD32)(*p) >> gs) & gm) * 255 / gm) << dgs) |
            (((((CARD32)(*p) >> bs) & bm) * 255 / bm) << dbs);
      }
    }
  }

  return (m_tightRectColors == 2) ? 1 : 8;
}


// Actual filtering code follows

#define DEFINE_TIGHT_FILTER_COPY(bpp)                                         \
                                                                              \
void ClientConnection::FilterCopy##bpp(int srcx, int srcy, int numRows)       \
{                                                                             \
  CARD32 *dstPtr =                                                            \
    (CARD32 *)&fb.bits[srcy * fb.pitch + srcx * fbx_ps[fb.format]];           \
  CARD##bpp *srcPtr = (CARD##bpp *)m_netbuf;                                  \
  int stride = fb.pitch / fbx_ps[fb.format];                                  \
  int dstPad = stride - m_tightRectWidth;                                     \
                                                                              \
  SETUP_COLOR_SHORTCUTS;                                                      \
  CARD32 drs = fbx_roffset[fb.format] * 8;                                    \
  CARD32 dgs = fbx_goffset[fb.format] * 8;                                    \
  CARD32 dbs = fbx_boffset[fb.format] * 8;                                    \
                                                                              \
  if (rm == 0xFF && gm == 0xFF && bm == 0xFF && bpp == 32) {                  \
    if (rs == drs && gs == dgs && bs == dbs) {                                \
      while (numRows-- > 0) {                                                 \
        memcpy(dstPtr, srcPtr, m_tightRectWidth * bpp / 8);                   \
        dstPtr += stride;                                                     \
        srcPtr += m_tightRectWidth;                                           \
      }                                                                       \
    } else {                                                                  \
      while (numRows-- > 0) {                                                 \
        CARD32 *dstEndOfRow = dstPtr + m_tightRectWidth;                      \
        while (dstPtr < dstEndOfRow) {                                        \
          *dstPtr++ = ((((CARD32)(*srcPtr) >> rs) & rm) << drs) |             \
                      ((((CARD32)(*srcPtr) >> gs) & gm) << dgs) |             \
                      ((((CARD32)(*srcPtr) >> bs) & bm) << dbs);              \
          srcPtr++;                                                           \
        }                                                                     \
        dstPtr += dstPad;                                                     \
      }                                                                       \
    }                                                                         \
  } else {                                                                    \
    while (numRows-- > 0) {                                                   \
      CARD32 *dstEndOfRow = dstPtr + m_tightRectWidth;                        \
      while (dstPtr < dstEndOfRow) {                                          \
        *dstPtr++ = (((((CARD32)(*srcPtr) >> rs) & rm) * 255 / rm) << drs) |  \
                    (((((CARD32)(*srcPtr) >> gs) & gm) * 255 / gm) << dgs) |  \
                    (((((CARD32)(*srcPtr) >> bs) & bm) * 255 / bm) << dbs);   \
        srcPtr++;                                                             \
      }                                                                       \
      dstPtr += dstPad;                                                       \
    }                                                                         \
  }                                                                           \
}

DEFINE_TIGHT_FILTER_COPY(8)
DEFINE_TIGHT_FILTER_COPY(16)
DEFINE_TIGHT_FILTER_COPY(32)


void ClientConnection::FilterCopy24(int srcx, int srcy, int numRows)
{
  CARD32 *dstPtr =
    (CARD32 *)&fb.bits[srcy * fb.pitch + srcx * fbx_ps[fb.format]];
  CARD8 *srcPtr = (CARD8 *)m_netbuf;
  int dstPad = fb.pitch / fbx_ps[fb.format] - m_tightRectWidth;

  CARD32 drs = fbx_roffset[fb.format] * 8;
  CARD32 dgs = fbx_goffset[fb.format] * 8;
  CARD32 dbs = fbx_boffset[fb.format] * 8;

  while (numRows-- > 0) {
    CARD32 *dstEndOfRow = dstPtr + m_tightRectWidth;
    while (dstPtr < dstEndOfRow) {
      *dstPtr++ = (srcPtr[0] << drs) | (srcPtr[1] << dgs) | (srcPtr[2] << dbs);
      srcPtr += 3;
    }
    dstPtr += dstPad;
  }
}


#define DEFINE_TIGHT_FILTER_GRADIENT(bpp)                                     \
                                                                              \
void ClientConnection::FilterGradient##bpp(int srcx, int srcy, int numRows)   \
{                                                                             \
  int x, y, c;                                                                \
  CARD##bpp *src = (CARD##bpp *)m_netbuf;                                     \
  CARD32 *dst =                                                               \
    (CARD32 *)&fb.bits[srcy * fb.pitch + srcx * fbx_ps[fb.format]];           \
  int dstw = fb.pitch / fbx_ps[fb.format];                                    \
  CARD16 *thatRow = (CARD16 *)m_tightPrevRow;                                 \
  CARD16 thisRow[2048 * 3];                                                   \
  CARD16 pix[3];                                                              \
  CARD16 max[3];                                                              \
  int shift[3];                                                               \
  int est[3];                                                                 \
                                                                              \
  CARD32 drs = fbx_roffset[fb.format] * 8;                                    \
  CARD32 dgs = fbx_goffset[fb.format] * 8;                                    \
  CARD32 dbs = fbx_boffset[fb.format] * 8;                                    \
                                                                              \
  max[0] = m_myFormat.redMax;                                                 \
  max[1] = m_myFormat.greenMax;                                               \
  max[2] = m_myFormat.blueMax;                                                \
                                                                              \
  shift[0] = m_myFormat.redShift;                                             \
  shift[1] = m_myFormat.greenShift;                                           \
  shift[2] = m_myFormat.blueShift;                                            \
                                                                              \
  for (y = 0; y < numRows; y++) {                                             \
                                                                              \
    /* First pixel in a row */                                                \
    for (c = 0; c < 3; c++) {                                                 \
      pix[c] = (CARD16)((src[y * m_tightRectWidth] >> shift[c]) +             \
                        thatRow[c] & max[c]);                                 \
      thisRow[c] = pix[c];                                                    \
    }                                                                         \
    dst[y * dstw] = (((CARD32)pix[0] * 255 / max[0]) << drs) |                \
                    (((CARD32)pix[1] * 255 / max[1]) << dgs) |                \
                    (((CARD32)pix[2] * 255 / max[2]) << dbs);                 \
                                                                              \
    /* Remaining pixels in the row */                                         \
    for (x = 1; x < m_tightRectWidth; x++) {                                  \
      for (c = 0; c < 3; c++) {                                               \
        est[c] = (int)thatRow[x * 3 + c] + (int)pix[c] -                      \
                 (int)thatRow[(x - 1) * 3 + c];                               \
        if (est[c] > (int)max[c])                                             \
          est[c] = (int)max[c];                                               \
        else if (est[c] < 0)                                                  \
          est[c] = 0;                                                         \
        pix[c] = (CARD16)((src[y * m_tightRectWidth + x] >> shift[c]) +       \
                          est[c] & max[c]);                                   \
        thisRow[x * 3 + c] = pix[c];                                          \
      }                                                                       \
      dst[y * dstw + x] = (((CARD32)pix[0] * 255 / max[0]) << drs) |          \
                          (((CARD32)pix[1] * 255 / max[1]) << dgs) |          \
                          (((CARD32)pix[2] * 255 / max[2]) << dbs);           \
    }                                                                         \
    memcpy(thatRow, thisRow, m_tightRectWidth * 3 * sizeof(CARD16));          \
  }                                                                           \
}

DEFINE_TIGHT_FILTER_GRADIENT(8)
DEFINE_TIGHT_FILTER_GRADIENT(16)
DEFINE_TIGHT_FILTER_GRADIENT(32)


void ClientConnection::FilterGradient24(int srcx, int srcy, int numRows)
{
  CARD8 thisRow[2048 * 3];
  CARD8 pix[3];
  int est[3];

  int ps = fbx_ps[fb.format];
  CARD8 *dst = (CARD8 *)&fb.bits[srcy * fb.pitch + srcx * ps];

  int rindex = fbx_roffset[fb.format];
  int gindex = fbx_goffset[fb.format];
  int bindex = fbx_boffset[fb.format];

  for (int y = 0; y < numRows; y++) {

    // First pixel in a row
    for (int c = 0; c < 3; c++) {
      pix[c] = m_tightPrevRow[c] + m_netbuf[y * m_tightRectWidth * 3 + c];
      thisRow[c] = pix[c];
    }
    dst[y * fb.pitch + rindex] = pix[0];
    dst[y * fb.pitch + gindex] = pix[1];
    dst[y * fb.pitch + bindex] = pix[2];

    // Remaining pixels in the row
    for (int x = 1; x < m_tightRectWidth; x++) {
      for (int c = 0; c < 3; c++) {
        est[c] = (int)m_tightPrevRow[x * 3 + c] + (int)pix[c] -
                 (int)m_tightPrevRow[(x - 1) * 3 + c];
        if (est[c] > 0xFF)
          est[c] = 0xFF;
        else if (est[c] < 0x00)
          est[c] = 0x00;
        pix[c] = (CARD8)est[c] + m_netbuf[(y * m_tightRectWidth + x) * 3 + c];
        thisRow[x * 3 + c] = pix[c];
      }
      dst[y * fb.pitch + x * ps + rindex] = pix[0];
      dst[y * fb.pitch + x * ps + gindex] = pix[1];
      dst[y * fb.pitch + x * ps + bindex] = pix[2];
    }

    memcpy(m_tightPrevRow, thisRow, m_tightRectWidth * 3);
  }
}


void ClientConnection::FilterPalette(int srcx, int srcy, int numRows)
{
  CARD8 *srcPtr = (CARD8 *)m_netbuf;
  CARD32 *dstPtr =
    (CARD32 *)&fb.bits[srcy * fb.pitch + srcx * fbx_ps[fb.format]];
  int dstPad = fb.pitch / fbx_ps[fb.format] - m_tightRectWidth;

  if (m_tightRectColors == 2) {
    int remainder = m_tightRectWidth % 8;
    int w8 = m_tightRectWidth - remainder;
    while (numRows-- > 0) {
      CARD32 *dstEndOfRow = dstPtr + w8;
      while (dstPtr < dstEndOfRow) {
        CARD8 bits = *srcPtr++;
        *dstPtr++ = m_tightPalette[bits >> 7 & 1];
        *dstPtr++ = m_tightPalette[bits >> 6 & 1];
        *dstPtr++ = m_tightPalette[bits >> 5 & 1];
        *dstPtr++ = m_tightPalette[bits >> 4 & 1];
        *dstPtr++ = m_tightPalette[bits >> 3 & 1];
        *dstPtr++ = m_tightPalette[bits >> 2 & 1];
        *dstPtr++ = m_tightPalette[bits >> 1 & 1];
        *dstPtr++ = m_tightPalette[bits & 1];
      }
      if (remainder != 0) {
        CARD8 bits = *srcPtr++;
        for (int b = 7; b >= 8 - remainder; b--)
          *dstPtr++ = m_tightPalette[bits >> b & 1];
      }
      dstPtr += dstPad;
    }
  } else {
    while (numRows-- > 0) {
      CARD32 *dstEndOfRow = dstPtr + m_tightRectWidth;
      while (dstPtr < dstEndOfRow)
        *dstPtr++ = m_tightPalette[*srcPtr++];
      dstPtr += dstPad;
    }
  }
}


void ClientConnection::DecompressJpegRect(int x, int y, int w, int h)
{
  int compressedLen = (int)ReadCompactLen();
  if (compressedLen <= 0) {
    vnclog.Print(0, "Incorrect data received from the server.\n");
    return;
  }

  CheckBufferSize(compressedLen);
  ReadExact(m_netbuf, compressedLen);

  omni_mutex_lock l(m_bitmapdcMutex);
  ObjectSelector b(m_hBitmapDC, m_hBitmap);

  if ((tjDecompress(j, (unsigned char *)m_netbuf, (unsigned long)compressedLen,
     (unsigned char *)&fb.bits[y * fb.pitch + x * fbx_ps[fb.format]], w,
     fb.pitch, h, fbx_ps[fb.format], fbx_bgr[fb.format] ? TJ_BGR : 0)) == -1)
     throw(ErrorException(tjGetErrorStr()));
}
