/* Copyright (C) 2011-2012, 2015, 2018 D. R. Commander.  All Rights Reserved.
 * Copyright (C) 2011 Brian P. Hinz
 * Copyright 2009 Pierre Ossman for Cendio AB
 * Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
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
// PixelFormat
//

package com.turbovnc.rfb;

import com.turbovnc.rdr.*;
import java.awt.image.ColorModel;

public class PixelFormat {

  public PixelFormat(int b, int d, boolean e, boolean t) {
    bpp = b;
    depth = d;
    bigEndian = e;
    trueColour = t;
  }

  public PixelFormat(int b, int d, boolean e, boolean t,
                     int rm, int gm, int bm, int rs, int gs, int bs) {
    this(b, d, e, t);
    redMax = rm;
    greenMax = gm;
    blueMax = bm;
    redShift = rs;
    greenShift = gs;
    blueShift = bs;
  }

  public PixelFormat() { this(8, 8, false, true, 7, 7, 3, 0, 3, 6); }

  public boolean equal(PixelFormat x) {
    return (bpp == x.bpp &&
            depth == x.depth &&
            (bigEndian == x.bigEndian || bpp == 8) &&
            trueColour == x.trueColour &&
            (!trueColour || (redMax == x.redMax &&
                             greenMax == x.greenMax &&
                             blueMax == x.blueMax &&
                             redShift == x.redShift &&
                             greenShift == x.greenShift &&
                             blueShift == x.blueShift)));
  }

  public void read(InStream is) {
    bpp = is.readU8();
    depth = is.readU8();
    bigEndian = is.readU8() != 0;
    trueColour = is.readU8() != 0;
    redMax = is.readU16();
    greenMax = is.readU16();
    blueMax = is.readU16();
    redShift = is.readU8();
    greenShift = is.readU8();
    blueShift = is.readU8();
    is.skip(3);
  }

  public void write(OutStream os) {
    os.writeU8(bpp);
    os.writeU8(depth);
    os.writeU8(bigEndian ? 1 : 0);
    os.writeU8(trueColour ? 1 : 0);
    os.writeU16(redMax);
    os.writeU16(greenMax);
    os.writeU16(blueMax);
    os.writeU8(redShift);
    os.writeU8(greenShift);
    os.writeU8(blueShift);
    os.pad(3);
  }

  public final boolean is888() {
    if (!trueColour)
      return false;
    if (bpp != 32)
      return false;
    if (depth != 24)
      return false;
    if (redMax != 255)
      return false;
    if (greenMax != 255)
      return false;
    if (blueMax != 255)
      return false;

    return true;
  }

  public int pixelFromRGB(int red, int green, int blue, ColorModel cm) {
    if (is888()) {
      return (red << redShift) | (green << greenShift) | (blue << blueShift) |
             (0xff << 24);
    } else if (trueColour) {
      int r = (red   * redMax     + 127) / 255;
      int g = (green * greenMax   + 127) / 255;
      int b = (blue  * blueMax    + 127) / 255;

      return (r << redShift) | (g << greenShift) | (b << blueShift);
    } else if (cm != null) {
      // Try to find the closest pixel by Cartesian distance
      int colours = 1 << depth;
      int diff = 256 * 256 * 4;
      int col = 0;
      for (int i = 0; i < colours; i++) {
        int r, g, b;
        r = cm.getRed(i);
        g = cm.getGreen(i);
        b = cm.getBlue(i);
        int rd = (r - red) >> 8;
        int gd = (g - green) >> 8;
        int bd = (b - blue) >> 8;
        int d = rd * rd + gd * gd + bd * bd;
        if (d < diff) {
          col = i;
          diff = d;
        }
      }
      return col;
    }
    // XXX just return 0 for colour map?
    return 0;
  }

  public void bufferFromRGB(int[] dst, int dstPtr, byte[] src,
                            int srcPtr, int pixels) {
    if (is888()) {
      // Optimised common case
      int rshift = redShift, gshift = greenShift, bshift = blueShift;

      if (bigEndian) {
        rshift = 24 - redShift;
        gshift = 24 - greenShift;
        bshift = 24 - blueShift;
      }

      if (alpha) {
        while (pixels-- != 0)
          dst[dstPtr++] = ((src[srcPtr++] & 0xff) << rshift) |
                          ((src[srcPtr++] & 0xff) << gshift) |
                          ((src[srcPtr++] & 0xff) << bshift) | (0xff << 24);
      } else {
        while (pixels-- != 0)
          dst[dstPtr++] = ((src[srcPtr++] & 0xff) << rshift) |
                          ((src[srcPtr++] & 0xff) << gshift) |
                          ((src[srcPtr++] & 0xff) << bshift);
      }
    } else {
      // Generic code
      int r, g, b;

      while (pixels-- != 0) {
        r = src[srcPtr++] & 0xff;
        g = src[srcPtr++] & 0xff;
        b = src[srcPtr++] & 0xff;

        dst[dstPtr++] = pixelFromRGB(r, g, b, null);
      }
    }
  }

  public void bufferFromRGB(Object dst, int x, int y, int stride,
                            byte[] src, int w, int h) {
    if (dst instanceof byte[])
      bufferFromRGB((byte[])dst, x, y, stride, src, w, h);
    else if (dst instanceof short[])
      bufferFromRGB((short[])dst, x, y, stride, src, w, h);
    else
      bufferFromRGB((int[])dst, x, y, stride, src, w, h);
  }

  public void bufferFromRGB(int[] dst, int x, int y, int stride,
                            byte[] src, int w, int h) {
    int dstPtr = y * stride + x, srcPtr = 0;

    if (is888()) {
      // Optimised common case
      int rshift = redShift, gshift = greenShift, bshift = blueShift;

      if (bigEndian) {
        rshift = 24 - redShift;
        gshift = 24 - greenShift;
        bshift = 24 - blueShift;
      }

      int dstPad = stride - w;
      if (alpha) {
        while (h > 0) {
          int dstEndOfRow = dstPtr + w;
          while (dstPtr < dstEndOfRow)
            dst[dstPtr++] = ((src[srcPtr++] & 0xff) << rshift) |
                            ((src[srcPtr++] & 0xff) << gshift) |
                            ((src[srcPtr++] & 0xff) << bshift) | (0xff << 24);
          dstPtr += dstPad;
          h--;
        }
      } else {
        while (h > 0) {
          int dstEndOfRow = dstPtr + w;
          while (dstPtr < dstEndOfRow)
            dst[dstPtr++] = ((src[srcPtr++] & 0xff) << rshift) |
                            ((src[srcPtr++] & 0xff) << gshift) |
                            ((src[srcPtr++] & 0xff) << bshift);
          dstPtr += dstPad;
          h--;
        }
      }
    } else {
      // Generic code
      int r, g, b;

      int dstPad = stride - w;
      while (h > 0) {
        int dstEndOfRow = dstPtr + w;

        while (dstPtr < dstEndOfRow) {
          r = src[srcPtr++] & 0xff;
          g = src[srcPtr++] & 0xff;
          b = src[srcPtr++] & 0xff;

          dst[dstPtr++] = pixelFromRGB(r, g, b, null);
        }
        dstPtr += dstPad;
        h--;
      }
    }
  }

  public void bufferFromRGB(byte[] dst, int x, int y, int stride,
                            byte[] src, int w, int h) {
    int dstPtr = y * stride + x, srcPtr = 0;

    // Generic code
    int r, g, b;

    int dstPad = stride - w;
    while (h > 0) {
      int dstEndOfRow = dstPtr + w;

      while (dstPtr < dstEndOfRow) {
        r = src[srcPtr++] & 0xff;
        g = src[srcPtr++] & 0xff;
        b = src[srcPtr++] & 0xff;

        dst[dstPtr++] = (byte)pixelFromRGB(r, g, b, null);
      }
      dstPtr += dstPad;
      h--;
    }
  }

  public void bufferFromRGB(short[] dst, int x, int y, int stride,
                            byte[] src, int w, int h) {
    int dstPtr = y * stride + x, srcPtr = 0;

    // Generic code
    int r, g, b;

    int dstPad = stride - w;
    while (h > 0) {
      int dstEndOfRow = dstPtr + w;

      while (dstPtr < dstEndOfRow) {
        r = src[srcPtr++] & 0xff;
        g = src[srcPtr++] & 0xff;
        b = src[srcPtr++] & 0xff;

        dst[dstPtr++] = (short)pixelFromRGB(r, g, b, null);
      }
      dstPtr += dstPad;
      h--;
    }
  }

  public String print() {
    StringBuffer s = new StringBuffer();
    s.append("depth " + depth + " (" + bpp + "bpp)");
    if (bpp != 8) {
      if (bigEndian)
        s.append(" big-endian");
      else
        s.append(" little-endian");
    }

    if (!trueColour) {
      s.append(" colour-map");
      return s.toString();
    }

    if (blueShift == 0 && greenShift > blueShift && redShift > greenShift &&
        blueMax  == (1 << greenShift) - 1 &&
        greenMax == (1 << (redShift - greenShift)) - 1 &&
        redMax   == (1 << (depth - redShift)) - 1) {
      if (alpha)
        s.append(" rgba" + (depth - redShift) + (redShift - greenShift) +
                 greenShift + "8");
      else
        s.append(" rgb" + (depth - redShift) + (redShift - greenShift) +
                 greenShift);
      return s.toString();
    }

    if (redShift == 0 && greenShift > redShift && blueShift > greenShift &&
        redMax   == (1 << greenShift) - 1 &&
        greenMax == (1 << (blueShift - greenShift)) - 1 &&
        blueMax  == (1 << (depth - blueShift)) - 1) {
      if (alpha)
        s.append(" bgra" + (depth - blueShift) + (blueShift - greenShift) +
                 greenShift + "8");
      else
        s.append(" bgr" + (depth - blueShift) + (blueShift - greenShift) +
                 greenShift);

      return s.toString();
    }

    s.append(" rgb max " + redMax + "," + greenMax + "," + blueMax +
             " shift " + redShift + "," + greenShift + "," + blueShift);
    return s.toString();
  }

  // CHECKSTYLE VisibilityModifier:OFF
  public int bpp;
  public int depth;
  public boolean bigEndian;
  public boolean trueColour;
  public int redMax;
  public int greenMax;
  public int blueMax;
  public int redShift;
  public int greenShift;
  public int blueShift;
  public boolean alpha;
  public boolean alphaPreMultiplied = true;
  // CHECKSTYLE VisibilityModifier:ON
}
