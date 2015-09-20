/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
/* Copyright (C) 2011 Brian P. Hinz
 * Copyright (C) 2012, 2015 D. R. Commander.  All Rights Reserved.
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

//
// rdr::InStream marshalls data from a buffer stored in RDR (RFB Data
// Representation).
//

package com.turbovnc.rdr;

import com.turbovnc.network.*;
import com.turbovnc.rfb.*;

public abstract class InStream {

  // check() ensures there is buffer data for at least one item of size
  // itemSize bytes.  Returns the number of items in the buffer (up to a
  // maximum of nItems).

  public int check(int itemSize, int nItems, boolean wait) {
    int available = end - ptr;
    if (itemSize * nItems > available) {
      if (itemSize > available)
        return overrun(itemSize, nItems, wait);

      nItems = available / itemSize;
    }
    return nItems;
  }

  public int check(int itemSize, int nItems) {
    return check(itemSize, nItems, true);
  }

  public int check(int itemSize) { return check(itemSize, 1); }

  // checkNoWait() tries to make sure that the given number of bytes can
  // be read without blocking.  It returns true if this is the case, false
  // otherwise.  The length must be "small" (less than the buffer size).

  public final boolean checkNoWait(int length) {
    return check(length, 1, false) != 0;
  }

  // readU/SN() methods read unsigned and signed N-bit integers.

  public final int readS8()  { check(1, 1, true);  return b[ptr++]; }
  public final int readS16() { check(2, 1, true);  int b0 = b[ptr++];
                               int b1 = b[ptr++] & 0xff;
                               return b0 << 8 | b1; }
  public final int readS32() { check(4, 1, true);  int b0 = b[ptr++];
                               int b1 = b[ptr++] & 0xff;
                               int b2 = b[ptr++] & 0xff;
                               int b3 = b[ptr++] & 0xff;
                               return b0 << 24 | b1 << 16 | b2 << 8 | b3; }

  public final int readU8()  { return readS8()  & 0xff;  }
  public final int readU16() { return readS16() & 0xffff; }
  public final int readU32() { return readS32() & 0xffffffff; }

  // readString() reads a string - a U32 length followed by the data.

  public final String readString() {
    int len = readU32();
    if (len > maxStringLength)
      throw new ErrorException("InStream max string length exceeded");

    byte[] str = new byte[len];
    readBytes(str, 0, len);
    String utf8string = new String();
    try {
      utf8string = new String(str, "UTF8");
    } catch (java.io.UnsupportedEncodingException e) {
      e.printStackTrace();
    }
    return utf8string;
  }

  // maxStringLength protects against allocating a huge buffer.  Set it
  // higher if you need longer strings.

  public static int maxStringLength = 65535;

  public final void skip(int bytes) {
    while (bytes > 0) {
      int n = check(1, bytes, true);
      ptr += n;
      bytes -= n;
    }
  }

  // readBytes() reads an exact number of bytes into an array at an offset.

  public void readBytes(byte[] data, int dataPtr, int length) {
    int dataEnd = dataPtr + length;
    while (dataPtr < dataEnd) {
      int n = check(1, dataEnd - dataPtr, true);
      System.arraycopy(b, ptr, data, dataPtr, n);
      ptr += n;
      dataPtr += n;
    }
  }

  // readOpaqueN() reads a quantity "without byte-swapping".  Because java has
  // no byte-ordering, we just use big-endian.

  public final int readOpaque8()  { return readU8(); }
  public final int readOpaque16() { return readU16(); }
  public final int readOpaque32() { return readU32(); }
  public final int readOpaque24A() { check(3, 1, true);  int b0 = b[ptr++];
                                     int b1 = b[ptr++];  int b2 = b[ptr++];
                                     return b0 << 24 | b1 << 16 | b2 << 8; }
  public final int readOpaque24B() { check(3, 1, true);  int b0 = b[ptr++];
                                     int b1 = b[ptr++];  int b2 = b[ptr++];
                                     return b0 << 16 | b1 << 8 | b2; }

  public final int readPixel(int bytesPerPixel, boolean bigEndian) {
    byte[] pix = new byte[4];
    readBytes(pix, 0, bytesPerPixel);

    if (bigEndian) {
      return 0x000000ff | (pix[0] & 0xff) << 24 |
             (pix[1] & 0xff) << 16 | (pix[2] & 0xff) << 8;
    } else {
      return 0xff000000 | (pix[2] & 0xff) << 16 |
             (pix[1] & 0xff) << 8 | (pix[0] & 0xff);
    }
  }

  public final void readPixels(Object buf, int length, int bytesPerPixel,
                               boolean bigEndian) {
    int nbytes = length * bytesPerPixel;
    int ptr = 0, srcPtr = 0;
    byte[] pixels = new byte[nbytes];

    readBytes(pixels, 0, nbytes);

    if (bytesPerPixel == 1 && buf instanceof byte[]) {
      System.arraycopy(pixels, 0, (byte[])buf, 0, length);
    } else if (bytesPerPixel == 2 && buf instanceof short[]) {
      if (bigEndian) {
        while (length-- > 0) {
          ((short[])buf)[ptr] = (short)((pixels[srcPtr++] & 0xff) << 8);
          ((short[])buf)[ptr++] |= (pixels[srcPtr++] & 0xff);
        }
      } else {
        while (length-- > 0) {
          ((short[])buf)[ptr] = (short)(pixels[srcPtr++] & 0xff);
          ((short[])buf)[ptr++] |= (pixels[srcPtr++] & 0xff) << 8;
        }
      }
    } else if (bytesPerPixel == 3 && buf instanceof int[]) {
      if (bigEndian) {
        while (length-- > 0)
          ((int[])buf)[ptr++] = (pixels[srcPtr++] & 0xff) << 24 |
                                (pixels[srcPtr++] & 0xff) << 16 |
                                (pixels[srcPtr++] & 0xff) << 8 |
                                0x000000ff;
      } else {
        while (length-- > 0)
          ((int[])buf)[ptr++] = (pixels[srcPtr++] & 0xff) |
                                (pixels[srcPtr++] & 0xff) << 8 |
                                (pixels[srcPtr++] & 0xff) << 16 |
                                0xff000000;
      }
    } else if (bytesPerPixel == 4 && buf instanceof int[]) {
      if (bigEndian) {
        while (length-- > 0)
          ((int[])buf)[ptr++] = (pixels[srcPtr++] & 0xff) << 24 |
                                (pixels[srcPtr++] & 0xff) << 16 |
                                (pixels[srcPtr++] & 0xff) << 8 |
                                (pixels[srcPtr++] & 0xff);
      } else {
        while (length-- > 0)
          ((int[])buf)[ptr++] = (pixels[srcPtr++] & 0xff) |
                                (pixels[srcPtr++] & 0xff) << 8 |
                                (pixels[srcPtr++] & 0xff) << 16 |
                                (pixels[srcPtr++] & 0xff) << 24;
      }
    } else {
      assert buf instanceof int[];
      if (bigEndian) {
        while (length-- > 0) {
          byte[] pix = new byte[4];
          System.arraycopy(pixels, srcPtr, pix, 0, bytesPerPixel);
          ((int[])buf)[ptr++] = (pix[0] & 0xff) << 24 |
                                (pix[1] & 0xff) << 16 |
                                (pix[2] & 0xff) << 8 |
                                0x000000ff;
          srcPtr += bytesPerPixel;
        }
      } else {
        while (length-- > 0) {
          byte[] pix = new byte[4];
          System.arraycopy(pixels, srcPtr, pix, 0, bytesPerPixel);
          ((int[])buf)[ptr++] = (pix[0] & 0xff) |
                                (pix[1] & 0xff) << 8 |
                                (pix[2] & 0xff) << 16 |
                                0xff000000;
          srcPtr += bytesPerPixel;
        }
      }
    }
  }


  public final void readPixels(Object buf, int stride, Rect r,
                               int bytesPerPixel, boolean bigEndian) {
    int w = r.width(), h = r.height();
    int nbytes = w * h * bytesPerPixel;
    int ptr = r.tl.y * stride + r.tl.x, srcPtr = 0;
    byte[] pixels = new byte[nbytes];
    int pad = stride - w;

    readBytes(pixels, 0, nbytes);

    if (bytesPerPixel == 1 && buf instanceof byte[]) {
      while (h > 0) {
        System.arraycopy(pixels, srcPtr, (byte[])buf, ptr, w);
        ptr += stride;
        srcPtr += w;
        h--;
      }
    } else if (bytesPerPixel == 2 && buf instanceof short[]) {
      if (bigEndian) {
        while (h > 0) {
          int endOfRow = ptr + w;
          while (ptr < endOfRow) {
            ((short[])buf)[ptr] = (short)((pixels[srcPtr++] & 0xff) << 8);
            ((short[])buf)[ptr++] |= (pixels[srcPtr++] & 0xff);
          }
          ptr += pad;
          h--;
        }
      } else {
        while (h > 0) {
          int endOfRow = ptr + w;
          while (ptr < endOfRow) {
            ((short[])buf)[ptr] = (short)(pixels[srcPtr++] & 0xff);
            ((short[])buf)[ptr++] |= (pixels[srcPtr++] & 0xff) << 8;
          }
          ptr += pad;
          h--;
        }
      }
    } else if (bytesPerPixel == 3 && buf instanceof int[]) {
      if (bigEndian) {
        while (h > 0) {
          int endOfRow = ptr + w;
          while (ptr < endOfRow)
            ((int[])buf)[ptr++] = (pixels[srcPtr++] & 0xff) << 24 |
                                  (pixels[srcPtr++] & 0xff) << 16 |
                                  (pixels[srcPtr++] & 0xff) << 8 |
                                  0x000000ff;
          ptr += pad;
          h--;
        }
      } else {
        while (h > 0) {
          int endOfRow = ptr + w;
          while (ptr < endOfRow)
            ((int[])buf)[ptr++] = (pixels[srcPtr++] & 0xff) |
                                  (pixels[srcPtr++] & 0xff) << 8 |
                                  (pixels[srcPtr++] & 0xff) << 16 |
                                  0xff000000;
          ptr += pad;
          h--;
        }
      }
    } else if (bytesPerPixel == 4 && buf instanceof int[]) {
      if (bigEndian) {
        while (h > 0) {
          int endOfRow = ptr + w;
          while (ptr < endOfRow) {
            ((int[])buf)[ptr++] = (pixels[srcPtr++] & 0xff) << 24 |
                                  (pixels[srcPtr++] & 0xff) << 16 |
                                  (pixels[srcPtr++] & 0xff) << 8 |
                                  0x000000ff;
            srcPtr++;
          }
          ptr += pad;
          h--;
        }
      } else {
        while (h > 0) {
          int endOfRow = ptr + w;
          while (ptr < endOfRow) {
            ((int[])buf)[ptr++] = (pixels[srcPtr++] & 0xff) |
                                  (pixels[srcPtr++] & 0xff) << 8 |
                                  (pixels[srcPtr++] & 0xff) << 16 |
                                  0xff000000;
            srcPtr++;
          }
          ptr += pad;
          h--;
        }
      }
    } else {
      // We should never get here
      throw new ErrorException("Unsupported pixel format");
    }
  }

  public final int readCompactLength() {
    int b = readU8();
    int result = b & 0x7F;
    if ((b & 0x80) != 0) {
      b = readU8();
      result |= (b & 0x7F) << 7;
      if ((b & 0x80) != 0) {
        b = readU8();
        result |= (b & 0xFF) << 14;
      }
    }
    return result;
  }

  // pos() returns the position in the stream.

  public abstract int pos();

  // bytesAvailable() returns true if at least one byte can be read from the
  // stream without blocking.  i.e. if false is returned then readU8() would
  // block.

  public boolean bytesAvailable() { return end != ptr; }

  // getbuf(), getptr(), getend() and setptr() are "dirty" methods which allow
  // you to manipulate the buffer directly.  This is useful for a stream which
  // is a wrapper around an underlying stream.

  public final byte[] getbuf() { return b; }
  public final int getptr() { return ptr; }
  public final int getend() { return end; }
  public final void setptr(int p) { ptr = p; }

  // overrun() is implemented by a derived class to cope with buffer overrun.
  // It ensures there are at least itemSize bytes of buffer data.  Returns
  // the number of items in the buffer (up to a maximum of nItems).  itemSize
  // is supposed to be "small" (a few bytes).

  protected abstract int overrun(int itemSize, int nItems, boolean wait);

  protected InStream() {}
  protected byte[] b;
  protected int ptr;
  protected int end;
}
