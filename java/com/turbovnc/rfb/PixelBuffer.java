/* Copyright (C) 2012, 2015, 2018 D. R. Commander.  All Rights Reserved.
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
// PixelBuffer - note that this code is only written for the 8, 16, and 32 bpp
// cases at the moment.
//

package com.turbovnc.rfb;

import java.awt.image.*;
import java.awt.color.*;

import com.turbovnc.rdr.ErrorException;

public class PixelBuffer {

  public PixelBuffer() {
    setPF(new PixelFormat());
  }

  public void setPF(PixelFormat pf) {
    if (!(pf.bpp == 32) && !(pf.bpp == 16) && !(pf.bpp == 15) &&
        !(pf.bpp == 8))
      throw new ErrorException("setPF() called with incorrect pixel format (bpp = " +
                               pf.bpp + ")");
    format = pf;
    switch (pf.depth) {
      case  3:
        // Fall-through to depth 8
      case  6:
        // Fall-through to depth 8
      case  8:
        int rmask = pf.redMax << pf.redShift;
        int gmask = pf.greenMax << pf.greenShift;
        int bmask = pf.blueMax << pf.blueShift;
        if (pf.trueColour)
          cm = new DirectColorModel(8, rmask, gmask, bmask);
        else
          cm = new IndexColorModel(8, 256, new byte[256], new byte[256],
                                   new byte[256]);
        break;
      case 15:
        cm = new DirectColorModel(15, 0x7C00, 0x03E0, 0x001F);
        break;
      case 16:
        cm = new DirectColorModel(16, 0xF800, 0x07E0, 0x001F);
        break;
      case 24:
        if (pf.alpha)
          cm = new DirectColorModel(ColorSpace.getInstance(ColorSpace.CS_sRGB),
                                    32, (0xff << 16), (0xff << 8), 0xff,
                                    (0xff << 24), pf.alphaPreMultiplied,
                                    DataBuffer.TYPE_INT);
        else
          cm = new DirectColorModel(32, (0xff << 16), (0xff << 8), 0xff);
        break;
      case 32:
        cm = new DirectColorModel(32, (0xff << pf.redShift),
          (0xff << pf.greenShift), (0xff << pf.blueShift));
        break;
      default:
        throw new ErrorException("Unsupported color depth (" + pf.depth + ")");
    }
  }
  public PixelFormat getPF() { return format; }

  public final int width() { return width; }
  public final int height() { return height; }
  public final int area() { return width * height; }

  public void fillRect(int x, int y, int w, int h, int pix) {
    assert data instanceof int[];
    for (int ry = y; ry < y + h; ry++)
      for (int rx = x; rx < x + w; rx++)
        ((int[])data)[ry * width + rx] = pix;
  }

  public void imageRect(int x, int y, int w, int h, int[] pix) {
    assert data instanceof int[];
    for (int j = 0; j < h; j++)
      System.arraycopy(pix, (w * j), data, width * (y + j) + x, w);
  }

  public void copyRect(int x, int y, int w, int h, int srcX, int srcY) {
    int dest = (width * y) + x;
    int src = (width * srcY) + srcX;
    int inc = width;

    if (y > srcY) {
      src += (h - 1) * inc;
      dest += (h - 1) * inc;
      inc = -inc;
    }
    int destEnd = dest + h * inc;

    while (dest != destEnd) {
      System.arraycopy(data, src, data, dest, w);
      src += inc;
      dest += inc;
    }
  }

  public Object getRawPixelsRW(int[] stride_) {
    if (stride > 0)
      stride_[0] = stride;
    else
      stride_[0] = width;
    return data;
  }

  public void maskRect(int x, int y, int w, int h, int[] pix, byte[] mask) {
    assert data instanceof int[];
    int maskBytesPerRow = (w + 7) / 8;

    for (int j = 0; j < h; j++) {
      int cy = y + j;

      if (cy < 0 || cy >= height)
        continue;

      for (int i = 0; i < w; i++) {
        int cx = x + i;

        if (cx < 0 || cx >= width)
          continue;

        int _byte = j * maskBytesPerRow + i / 8;
        int bit = 7 - i % 8;

        if ((mask[_byte] & (1 << bit)) != 0)
         ((int[])data)[cy * width + cx] = pix[j * w + i];
      }
    }
  }

  // CHECKSTYLE VisibilityModifier:OFF
  public Object data;
  public ColorModel cm;
  // CHECKSTYLE VisibilityModifier:ON

  protected PixelFormat format;
  protected int width, height, stride;
}
