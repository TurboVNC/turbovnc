/* Copyright (C) 2011-2013, 2015, 2017-2018, 2020-2021 D. R. Commander.
 *                                                     All Rights Reserved.
 * Copyright (C) 2011-2012 Brian P. Hinz
 * Copyright 2004-2005 Cendio AB.
 * Copyright (C) 2000-2003 Constantin Kaplinsky.  All Rights Reserved.
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

package com.turbovnc.rfb;

import com.turbovnc.rdr.*;
import java.awt.image.*;
import java.util.Arrays;
import java.awt.*;
import java.util.zip.*;

public class TightDecoder extends Decoder {

  static final int TIGHT_MAX_WIDTH = 2048;
  static final int TIGHT_MIN_TO_COMPRESS = 12;

  static final Toolkit TK = Toolkit.getDefaultToolkit();

  static final int TJPF_RGB = 0;
  static final int TJPF_RGBX = 2;
  static final int TJPF_BGRX = 3;
  static final int TJPF_XBGR = 4;
  static final int TJPF_XRGB = 5;

  public TightDecoder(CMsgReader reader_) {
    reader = reader_;
    inflater = new Inflater[4];
    for (int i = 0; i < 4; i++)
      inflater[i] = new Inflater();
    if (Helper.isAvailable() &&
        Utils.getBooleanProperty("turbovnc.turbojpeg", true)) {
      try {
        tjhandle = tjInitDecompress();
      } catch (Exception e) {
        throw new SystemException(e);
      }
    }
    tightPalette = new byte[256 * 3];
  }

  public void reset() {
    for (int i = 0; i < 4; i++) {
      if (inflater[i] != null)
        inflater[i].reset();
    }
  }

  // NOTE: must be idempotent
  public void close() {
    for (int i = 0; i < 4; i++) {
      if (inflater[i] != null)
        inflater[i].end();
    }
    if (tjhandle != 0) {
      try {
        tjDestroy(tjhandle);
      } catch (Exception e) {}
      tjhandle = 0;
    }
  }

  public boolean isTurboJPEG() {
    return tjhandle != 0;
  }

  short getShort(byte[] src, int srcPtr) {
    return (short)((src[srcPtr++] & 0xff) |
                   (src[srcPtr] & 0xff) << 8);
  }

  void checkPalette(int bpp, boolean cutZeros) {
    if (cutZeros || bpp > 16) {
      if (palette != null && palette instanceof int[])
        return;
      palette = new int[256];
    } else if (bpp == 8) {
      if (palette != null && palette instanceof byte[])
        return;
      palette = new byte[256];
    } else if (bpp == 16) {
      if (palette != null && palette instanceof short[])
        return;
      palette = new short[256];
    } else {
      // We should never get here
      throw new ErrorException("Unsupported pixel format");
    }
  }

  void checkNetbuf(int size) {
    if (netbufSize < size || netbuf == null) {
      if (netbuf != null)
        netbuf = null;
      netbuf = new byte[size];
      netbufSize = size;
    }
  }

  void checkDecodebuf(int size) {
    if (decodebufSize < size || decodebuf == null) {
      if (decodebuf != null)
        decodebuf = null;
      decodebuf = new byte[size];
      decodebufSize = size;
    }
  }

  @SuppressWarnings("fallthrough")
  public void readRect(Rect r, CMsgHandler handler) {
    InStream is = reader.getInStream();
    boolean cutZeros = false;
    serverpf = handler.cp.pf();
    int bpp = serverpf.bpp;
    cutZeros = false;
    if (bpp == 32 && serverpf.is888())
      cutZeros = true;

    int compCtl = is.readU8();

    boolean bigEndian = handler.cp.pf().bigEndian;

    // Flush zlib streams if we are told by the server to do so.
    for (int i = 0; i < 4; i++) {
      if ((compCtl & 1) != 0)
        inflater[i].end();
      compCtl >>= 1;
    }

    boolean readUncompressed = false;
    if ((compCtl & RFB.TIGHT_NO_ZLIB) == RFB.TIGHT_NO_ZLIB) {
      compCtl &= ~(RFB.TIGHT_NO_ZLIB);
      readUncompressed = true;
    }

    // "JPEG" compression type.
    if (compCtl == RFB.TIGHT_JPEG) {
      decompressJpegRect(r, is, handler);
      return;
    }

    // Quit on unsupported compression type.
    if (compCtl > RFB.TIGHT_MAX_SUBENCODING) {
      throw new ErrorException("TightDecoder: bad subencoding value received");
    }

    int w = r.width(), h = r.height();
    int[] stride = { w };
    Object buf = handler.getRawPixelsRW(stride);
    int pad = stride[0] - w;
    int ptr = r.tl.y * stride[0] + r.tl.x;

    // "Fill" compression type.
    if (compCtl == RFB.TIGHT_FILL) {
      if (cutZeros) {
        byte[] bytebuf = new byte[3];
        is.readBytes(bytebuf, 0, 3);
        int pix = (bytebuf[0] & 0xff) << serverpf.redShift |
                  (bytebuf[1] & 0xff) << serverpf.greenShift |
                  (bytebuf[2] & 0xff) << serverpf.blueShift | (0xff << 24);
        while (h > 0) {
          Arrays.fill((int[])buf, ptr, ptr + w, pix);
          ptr += stride[0];
          h--;
        }
      } else if (buf instanceof byte[]) {
        int pix = is.readU8();
        while (h > 0) {
          Arrays.fill((byte[])buf, ptr, ptr + w, (byte)pix);
          ptr += stride[0];
          h--;
        }
      } else if (buf instanceof short[]) {
        int pix = is.readPixel(bpp / 8, serverpf.bigEndian);
        while (h > 0) {
          Arrays.fill((short[])buf, ptr, ptr + w, (short)pix);
          ptr += stride[0];
          h--;
        }
      } else {
        // We should never get here
        throw new ErrorException("Unsupported pixel type");
      }
      handler.releaseRawPixels(r);
      return;
    }

    // "Basic" compression type.
    int palSize = 0;
    boolean useGradient = false;

    if ((compCtl & RFB.TIGHT_EXPLICIT_FILTER) != 0) {
      int filterId = is.readU8();

      switch (filterId) {
        case RFB.TIGHT_FILTER_PALETTE:
          palSize = is.readU8() + 1;
          checkPalette(bpp, cutZeros);
          if (cutZeros) {
            is.readBytes(tightPalette, 0, palSize * 3);
            serverpf.bufferFromRGB((int[])palette, 0, tightPalette, 0,
                                   palSize);
          } else
            is.readPixels(palette, palSize, serverpf.bpp / 8,
                          serverpf.bigEndian);
          break;
        case RFB.TIGHT_FILTER_GRADIENT:
          useGradient = true;
          break;
        case RFB.TIGHT_FILTER_COPY:
          break;
        default:
          throw new ErrorException("TightDecoder: unknown filter code recieved");
      }
    }

    int bppp = bpp;
    if (palSize != 0) {
      bppp = (palSize <= 2) ? 1 : 8;
    } else if (cutZeros) {
      bppp = 24;
    }

    // Determine if the data should be decompressed or just copied.
    int rowSize = (r.width() * bppp + 7) / 8;
    int dataSize = r.height() * rowSize;
    int streamId = -1;

    // Allocate netbuf and read in data
    if (dataSize < TIGHT_MIN_TO_COMPRESS || readUncompressed) {
      if (dataSize >= TIGHT_MIN_TO_COMPRESS)
        dataSize = is.readCompactLength();
      checkDecodebuf(dataSize);
      is.readBytes(decodebuf, 0, dataSize);
    } else {
      int length = is.readCompactLength();
      checkNetbuf(length);
      is.readBytes(netbuf, 0, length);
      streamId = compCtl & 0x03;
      checkDecodebuf(dataSize);
      inflater[streamId].setInput(netbuf, 0, length);
      try {
        inflater[streamId].inflate(decodebuf, 0, dataSize);
      } catch (DataFormatException e) {
        throw new ErrorException(e.getMessage());
      }
    }

    int srcPtr = 0;

    if (palSize == 0) {
      // Truecolor data.
      if (useGradient) {
        if (cutZeros) {
          filterGradient24((int[])buf, stride[0], r);
        } else if (bpp == 16) {
          filterGradient16((short[])buf, stride[0], r);
        } else {
          // We should never get here
          throw new ErrorException("Unsupported pixel type");
        }
      } else {
        // Copy
        if (cutZeros) {
          serverpf.bufferFromRGB((int[])buf, r.tl.x, r.tl.y, stride[0],
                                 decodebuf, w, h);
        } else if (buf instanceof byte[]) {
          while (h > 0) {
            System.arraycopy(decodebuf, srcPtr, (byte[])buf, ptr, w);
            ptr += stride[0];
            srcPtr += w;
            h--;
          }
        } else if (buf instanceof short[]) {
          while (h > 0) {
            int endOfRow = ptr + w;
            while (ptr < endOfRow) {
              ((short[])buf)[ptr++] = getShort(decodebuf, srcPtr);
              srcPtr += 2;
            }
            ptr += pad;
            h--;
          }
        } else {
          // We should never get here
          throw new ErrorException("Unsupported pixel type");
        }
      }
    } else {
      // Indexed color
      int x, bits;
      if (palSize <= 2) {
        // 2-color palette
        int remainder = w % 8;
        int w8 = w - remainder;
        if (buf instanceof byte[]) {
          while (h > 0) {
            int endOfRow = ptr + w8;
            while (ptr < endOfRow) {
              bits = decodebuf[srcPtr++];
              ((byte[])buf)[ptr++] = ((byte[])palette)[bits >> 7 & 1];
              ((byte[])buf)[ptr++] = ((byte[])palette)[bits >> 6 & 1];
              ((byte[])buf)[ptr++] = ((byte[])palette)[bits >> 5 & 1];
              ((byte[])buf)[ptr++] = ((byte[])palette)[bits >> 4 & 1];
              ((byte[])buf)[ptr++] = ((byte[])palette)[bits >> 3 & 1];
              ((byte[])buf)[ptr++] = ((byte[])palette)[bits >> 2 & 1];
              ((byte[])buf)[ptr++] = ((byte[])palette)[bits >> 1 & 1];
              ((byte[])buf)[ptr++] = ((byte[])palette)[bits & 1];
            }
            if (remainder != 0) {
              bits = decodebuf[srcPtr++];
              for (int b = 7; b >= 8 - remainder; b--) {
                ((byte[])buf)[ptr++] = ((byte[])palette)[bits >> b & 1];
              }
            }
            ptr += pad;
            h--;
          }
        } else if (buf instanceof short[]) {
          while (h > 0) {
            int endOfRow = ptr + w8;
            while (ptr < endOfRow) {
              bits = decodebuf[srcPtr++];
              ((short[])buf)[ptr++] = ((short[])palette)[bits >> 7 & 1];
              ((short[])buf)[ptr++] = ((short[])palette)[bits >> 6 & 1];
              ((short[])buf)[ptr++] = ((short[])palette)[bits >> 5 & 1];
              ((short[])buf)[ptr++] = ((short[])palette)[bits >> 4 & 1];
              ((short[])buf)[ptr++] = ((short[])palette)[bits >> 3 & 1];
              ((short[])buf)[ptr++] = ((short[])palette)[bits >> 2 & 1];
              ((short[])buf)[ptr++] = ((short[])palette)[bits >> 1 & 1];
              ((short[])buf)[ptr++] = ((short[])palette)[bits & 1];
            }
            if (remainder != 0) {
              bits = decodebuf[srcPtr++];
              for (int b = 7; b >= 8 - remainder; b--) {
                ((short[])buf)[ptr++] = ((short[])palette)[bits >> b & 1];
              }
            }
            ptr += pad;
            h--;
          }
        } else {
          while (h > 0) {
            int endOfRow = ptr + w8;
            while (ptr < endOfRow) {
              bits = decodebuf[srcPtr++];
              ((int[])buf)[ptr++] = ((int[])palette)[bits >> 7 & 1];
              ((int[])buf)[ptr++] = ((int[])palette)[bits >> 6 & 1];
              ((int[])buf)[ptr++] = ((int[])palette)[bits >> 5 & 1];
              ((int[])buf)[ptr++] = ((int[])palette)[bits >> 4 & 1];
              ((int[])buf)[ptr++] = ((int[])palette)[bits >> 3 & 1];
              ((int[])buf)[ptr++] = ((int[])palette)[bits >> 2 & 1];
              ((int[])buf)[ptr++] = ((int[])palette)[bits >> 1 & 1];
              ((int[])buf)[ptr++] = ((int[])palette)[bits & 1];
            }
            if (remainder != 0) {
              bits = decodebuf[srcPtr++];
              for (int b = 7; b >= 8 - remainder; b--) {
                ((int[])buf)[ptr++] = ((int[])palette)[bits >> b & 1];
              }
            }
            ptr += pad;
            h--;
          }
        }
      } else {
        // 256-color palette
        if (buf instanceof byte[]) {
          while (h > 0) {
            int endOfRow = ptr + w;
            while (ptr < endOfRow) {
              ((byte[])buf)[ptr++] =
                ((byte[])palette)[decodebuf[srcPtr++] & 0xff];
            }
            ptr += pad;
            h--;
          }
        } else if (buf instanceof short[]) {
          while (h > 0) {
            int endOfRow = ptr + w;
            while (ptr < endOfRow) {
              ((short[])buf)[ptr++] =
                ((short[])palette)[decodebuf[srcPtr++] & 0xff];
            }
            ptr += pad;
            h--;
          }
        } else {
          while (h > 0) {
            int endOfRow = ptr + w;
            while (ptr < endOfRow) {
              ((int[])buf)[ptr++] =
                ((int[])palette)[decodebuf[srcPtr++] & 0xff];
            }
            ptr += pad;
            h--;
          }
        }
      }
    }

    handler.releaseRawPixels(r);
  }

  private void decompressJpegRect(Rect r, InStream is,
                                  CMsgHandler handler) {
    // Read length
    int compressedLen = is.readCompactLength();
    if (compressedLen <= 0)
      vlog.info("Incorrect data received from the server.");

    // Allocate netbuf and read in data
    checkNetbuf(compressedLen);
    is.readBytes(netbuf, 0, compressedLen);

    if (tjhandle != 0) {

      int[] stride = new int[1];
      Object data = handler.getRawPixelsRW(stride);
      int tjpf = TJPF_RGB;
      PixelFormat pf = handler.cp.pf();

      if (pf.is888()) {
        int redShift, greenShift, blueShift;

        if (pf.bigEndian) {
          redShift = 24 - pf.redShift;
          greenShift = 24 - pf.greenShift;
          blueShift = 24 - pf.blueShift;
        } else {
          redShift = pf.redShift;
          greenShift = pf.greenShift;
          blueShift = pf.blueShift;
        }

        if (redShift == 0 && greenShift == 8 && blueShift == 16)
          tjpf = TJPF_RGBX;
        if (redShift == 16 && greenShift == 8 && blueShift == 0)
          tjpf = TJPF_BGRX;
        if (redShift == 24 && greenShift == 16 && blueShift == 8)
          tjpf = TJPF_XBGR;
        if (redShift == 8 && greenShift == 16 && blueShift == 24)
          tjpf = TJPF_XRGB;

        try {
          tjDecompress(tjhandle, netbuf, compressedLen, (int[])data, r.tl.x,
                       r.tl.y, r.width(), stride[0], r.height(), tjpf, 0);
        } catch (Exception e) {
          throw new SystemException(e);
        }
      } else {
        byte[] rgbBuf = new byte[r.width() * r.height() * 3];
        try {
          tjDecompress(tjhandle, netbuf, compressedLen, rgbBuf, 0, 0,
                       r.width(), 0, r.height(), TJPF_RGB, 0);
        } catch (Exception e) {
          throw new SystemException(e);
        }
        pf.bufferFromRGB(data, r.tl.x, r.tl.y, stride[0], rgbBuf,
                         r.width(), r.height());
      }
      handler.releaseRawPixels(r);
      return;
    }

    // Create an Image object from the JPEG data.
    Image jpeg = TK.createImage(netbuf);
    jpeg.setAccelerationPriority(1);
    handler.imageRect(r, jpeg);
    jpeg.flush();
  }

  /* NOTE: we support gradient encoding only for backward compatibility with
     TightVNC 1.3.x.  It is decidedly non-optimal. */

  private void filterGradient24(int[] buf, int stride, Rect r) {

    int x, y, c;
    int ptr = r.tl.y * stride + r.tl.x;
    int[] prevRow = new int[TIGHT_MAX_WIDTH * 3];
    int[] thisRow = new int[TIGHT_MAX_WIDTH * 3];
    int[] pix = new int[3];
    int[] est = new int[3];

    // Set up shortcut variables
    int rectHeight = r.height();
    int rectWidth = r.width();

    for (y = 0; y < rectHeight; y++) {
      /* First pixel in a row */
      for (c = 0; c < 3; c++) {
        pix[c] = (decodebuf[y * rectWidth * 3 + c] + prevRow[c]) & 0xff;
        thisRow[c] = pix[c];
      }
      buf[ptr + y * stride] = serverpf.pixelFromRGB(pix[0], pix[1], pix[2],
                                                    null);

      /* Remaining pixels of a row */
      for (x = 1; x < rectWidth; x++) {
        for (c = 0; c < 3; c++) {
          est[c] = prevRow[x * 3 + c] + pix[c] - prevRow[(x - 1) * 3 + c];
          if (est[c] > 0xFF) {
            est[c] = 0xFF;
          } else if (est[c] < 0) {
            est[c] = 0;
          }
          pix[c] = (decodebuf[(y * rectWidth + x) * 3 + c] + est[c]) & 0xff;
          thisRow[x * 3 + c] = pix[c];
        }
        buf[ptr + y * stride + x] = serverpf.pixelFromRGB(pix[0], pix[1],
                                                          pix[2], null);
      }

      System.arraycopy(thisRow, 0, prevRow, 0, prevRow.length);
    }
  }

  private void filterGradient16(short[] buf, int stride, Rect r) {

    int x, y, c, p;
    int ptr = r.tl.y * stride + r.tl.x;
    int[] prevRow = new int[TIGHT_MAX_WIDTH * 3];
    int[] thisRow = new int[TIGHT_MAX_WIDTH * 3];
    int[] pix = new int[3];
    int[] est = new int[3];
    int[] max = new int[] { serverpf.redMax, serverpf.greenMax,
                            serverpf.blueMax };
    int[] shift = new int[] { serverpf.redShift, serverpf.greenShift,
                              serverpf.blueShift };

    // Set up shortcut variables
    int rectHeight = r.height();
    int rectWidth = r.width();

    for (y = 0; y < rectHeight; y++) {
      /* First pixel in a row */
      p = getShort(decodebuf, y * rectWidth * 2);
      for (c = 0; c < 3; c++) {
        pix[c] = ((p >> shift[c]) + prevRow[c]) & max[c];
        thisRow[c] = pix[c];
      }
      buf[ptr + y * stride] = (short)((pix[0] << shift[0]) |
                                      (pix[1] << shift[1]) |
                                      (pix[2] << shift[2]));

      /* Remaining pixels of a row */
      for (x = 1; x < rectWidth; x++) {
        p = getShort(decodebuf, (y * rectWidth + x) * 2);
        for (c = 0; c < 3; c++) {
          est[c] = prevRow[x * 3 + c] + pix[c] - prevRow[(x - 1) * 3 + c];
          if (est[c] > max[c]) {
            est[c] = max[c];
          } else if (est[c] < 0) {
            est[c] = 0;
          }
          pix[c] = ((p >> shift[c]) + est[c]) & max[c];
          thisRow[x * 3 + c] = pix[c];
        }
        buf[ptr + y * stride + x] = (short)((pix[0] << shift[0]) |
                                            (pix[1] << shift[1]) |
                                            (pix[2] << shift[2]));
      }

      System.arraycopy(thisRow, 0, prevRow, 0, prevRow.length);
    }
  }

  private CMsgReader reader;
  private Inflater[] inflater;
  private PixelFormat serverpf;
  private long tjhandle;
  private Object palette;
  private byte[] tightPalette;
  private byte[] netbuf;
  private int netbufSize;
  private byte[] decodebuf;
  private int decodebufSize;

  private native long tjInitDecompress() throws Exception;
  private native void tjDecompress(long handle, byte[] srcBuf, int size,
    byte[] dstBuf, int x, int y, int desiredWidth, int pitch,
    int desiredHeight, int pixelFormat, int flags) throws Exception;
  private native void tjDecompress(long handle, byte[] srcBuf, int size,
    int[] dstBuf, int x, int y, int desiredWidth, int pitch,
    int desiredHeight, int pixelFormat, int flags) throws Exception;
  private native void tjDestroy(long handle) throws Exception;

  static LogWriter vlog = new LogWriter("TightDecoder");
}
