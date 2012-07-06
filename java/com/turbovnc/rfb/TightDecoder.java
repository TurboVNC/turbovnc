/* Copyright (C) 2000-2003 Constantin Kaplinsky.  All Rights Reserved.
 * Copyright 2004-2005 Cendio AB.
 * Copyright (C) 2011-2012 D. R. Commander.  All Rights Reserved.
 * Copyright (C) 2011-2012 Brian P. Hinz
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

import com.turbovnc.rdr.InStream;
import com.turbovnc.rdr.ZlibInStream;
import java.util.ArrayList;
import java.io.InputStream;
import java.awt.image.*;
import java.util.Arrays;
import java.awt.*;
import org.libjpegturbo.turbojpeg.*;

public class TightDecoder extends Decoder {

  final static int TIGHT_MAX_WIDTH = 2048;

  // Compression control
  final static int rfbTightExplicitFilter = 0x04;
  final static int rfbTightFill = 0x08;
  final static int rfbTightJpeg = 0x09;
  final static int rfbTightMaxSubencoding = 0x09;

  // Filters to improve compression efficiency
  final static int rfbTightFilterCopy = 0x00;
  final static int rfbTightFilterPalette = 0x01;
  final static int rfbTightFilterGradient = 0x02;
  final static int rfbTightMinToCompress = 12;

  final static Toolkit tk = Toolkit.getDefaultToolkit();

  public TightDecoder(CMsgReader reader_) { 
    reader = reader_; 
    zis = new ZlibInStream[4];
    for (int i = 0; i < 4; i++)
      zis[i] = new ZlibInStream();
    try {
      tjd = new TJDecompressor();
    } catch (java.lang.NoClassDefFoundError e) {
      vlog.info("WARNING: Could not initialize libjpeg-turbo:");
      vlog.info("  Class not found: "+e.getMessage());
      vlog.info("  Using unaccelerated JPEG decompressor.");
    } catch (java.lang.UnsatisfiedLinkError e) {
      vlog.info("WARNING: Could not find TurboJPEG JNI library.  If it is in a");
      vlog.info("  non-standard location, then add -Djava.library.path=<dir>");
      vlog.info("  to the Java command line to specify its location.");
      vlog.info("  Using unaccelerated JPEG decompressor.");
    } catch (java.lang.Exception e) {
      vlog.info("WARNING: Could not initialize libjpeg-turbo:");
      vlog.info("  "+e.getMessage());
      vlog.info("  Using unaccelerated JPEG decompressor.");
    }
  }

  public void readRect(Rect r, CMsgHandler handler) 
  {
    InStream is = reader.getInStream();
    boolean cutZeros = false;
    clientpf = handler.getPreferredPF();
    serverpf = handler.cp.pf();
    int bpp = serverpf.bpp;
    cutZeros = false;
    if (bpp == 32) {
      if (serverpf.is888()) {
        cutZeros = true;
      }
    }

    int comp_ctl = is.readU8();

    boolean bigEndian = handler.cp.pf().bigEndian;

    // Flush zlib streams if we are told by the server to do so.
    for (int i = 0; i < 4; i++) {
      if ((comp_ctl & 1) != 0) {
        zis[i].reset();
      }
      comp_ctl >>= 1;
    }

    // "Fill" compression type.
    if (comp_ctl == rfbTightFill) {
      int[] pix = new int[1];
      if (cutZeros) {
        byte[] bytebuf = new byte[3];
        is.readBytes(bytebuf, 0, 3);
        serverpf.bufferFromRGB(pix, 0, bytebuf, 0, 1);
      } else {
        pix[0] = is.readPixel(serverpf.bpp/8, serverpf.bigEndian);
      }
      handler.fillRect(r, pix[0]);
      return;
    }

    // "JPEG" compression type.
    if (comp_ctl == rfbTightJpeg) {
      decompressJpegRect(r, is, handler);
      return;
    }

    // Quit on unsupported compression type.
    if (comp_ctl > rfbTightMaxSubencoding) {
      throw new Exception("TightDecoder: bad subencoding value received");
    }

    // "Basic" compression type.
    int palSize = 0;
    int[] palette = new int[256];
    boolean useGradient = false;

    if ((comp_ctl & rfbTightExplicitFilter) != 0) {
      int filterId = is.readU8();

      switch (filterId) {
      case rfbTightFilterPalette:
        palSize = is.readU8() + 1;
        byte[] tightPalette;
        if (cutZeros) {
          tightPalette = new byte[256 * 3];
          is.readBytes(tightPalette, 0, palSize * 3);
          serverpf.bufferFromRGB(palette, 0, tightPalette, 0, palSize);
        } else {
          is.readPixels(palette, palSize, serverpf.bpp/8, serverpf.bigEndian);
        }
        break;
      case rfbTightFilterGradient:
        useGradient = true;
        break;
      case rfbTightFilterCopy:
        break;
      default:
        throw new Exception("TightDecoder: unknown filter code recieved");
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
    InStream input;
    if (dataSize < rfbTightMinToCompress) {
      input = is;
    } else {
      int length = is.readCompactLength();
      streamId = comp_ctl & 0x03;
      zis[streamId].setUnderlying(is, length);
      input = (ZlibInStream)zis[streamId];
    }

    // Allocate netbuf and read in data
    byte[] netbuf = new byte[dataSize];
    input.readBytes(netbuf, 0, dataSize);

    int stride = r.width();
    int[] buf = reader.getImageBuf(r.area());

    if (palSize == 0) {
      // Truecolor data.
      if (useGradient) {
        if (bpp == 32 && cutZeros) {
          filterGradient24(netbuf, buf, stride, r);
        } else {
          filterGradient(netbuf, buf, stride, r);
        }
      } else {
        // Copy
        int h = r.height();
        int ptr = 0;
        int srcPtr = 0;
        int w = r.width();
        if (cutZeros) {
          serverpf.bufferFromRGB(buf, ptr, netbuf, srcPtr, w*h);
        } else {
          int pixelSize = (bpp >= 24) ? 3 : bpp/8;
          while (h > 0) {
            for (int i = 0; i < w; i++) {
              if (bpp == 8) {
                buf[ptr+i] = netbuf[srcPtr+i] & 0xff;
              } else {
                for (int j = pixelSize-1; j >= 0; j--)
                  buf[ptr+i] |= ((netbuf[srcPtr+i+j] & 0xff) << j*8);
              }
            }
            ptr += stride;
            srcPtr += w * pixelSize;
            h--;
          }
        }
      }
    } else {
      // Indexed color
      int x, h = r.height(), w = r.width(), b, pad = stride - w;
      int ptr = 0; 
      int srcPtr = 0, bits;
      if (palSize <= 2) {
        // 2-color palette
        while (h > 0) {
          for (x = 0; x < w / 8; x++) {
            bits = netbuf[srcPtr++];
            for(b = 7; b >= 0; b--) {
              buf[ptr++] = palette[bits >> b & 1];
            }
          }
          if (w % 8 != 0) {
            bits = netbuf[srcPtr++];
            for (b = 7; b >= 8 - w % 8; b--) {
              buf[ptr++] = palette[bits >> b & 1];
            }
          }
          ptr += pad;
          h--;
        }
      } else {
        // 256-color palette
        while (h > 0) {
          int endOfRow = ptr + w;
          while (ptr < endOfRow) {
            buf[ptr++] = palette[netbuf[srcPtr++] & 0xff];
          }
          ptr += pad;
          h--;
        }
      }
    } 

    handler.imageRect(r, buf);

    if (streamId != -1) {
      zis[streamId].reset();
    }
  }

  final private void decompressJpegRect(Rect r, InStream is,
                                        CMsgHandler handler)
  {
    // Read length
    int compressedLen = is.readCompactLength();
    if (compressedLen <= 0)
      vlog.info("Incorrect data received from the server.");

    // Allocate netbuf and read in data
    byte[] netbuf = new byte[compressedLen];
    is.readBytes(netbuf, 0, compressedLen);

    if (tjd != null) {

      int[] stride = new int[1];
      int[] data = handler.getRawPixelsRW(stride);
      int tjpf = TJ.PF_RGB;
      PixelFormat pf = handler.cp.pf();

      try {
        tjd.setJPEGImage(netbuf, compressedLen);

        if (pf.is888()) {
          int redShift, greenShift, blueShift;

          if(pf.bigEndian) {
            redShift = 24 - pf.redShift;
            greenShift = 24 - pf.greenShift;
            blueShift = 24 - pf.blueShift;
          } else {
            redShift = pf.redShift;
            greenShift = pf.greenShift;
            blueShift = pf.blueShift;
          }

          if(redShift == 0 && greenShift == 8 && blueShift == 16)
            tjpf = TJ.PF_RGBX;
          if(redShift == 16 && greenShift == 8 && blueShift == 0)
            tjpf = TJ.PF_BGRX;
          if(redShift == 24 && greenShift == 16 && blueShift == 8)
            tjpf = TJ.PF_XBGR;
          if(redShift == 8 && greenShift == 16 && blueShift == 24)
            tjpf = TJ.PF_XRGB;

          tjd.decompress(data, r.tl.x, r.tl.y, r.width(), stride[0], r.height(),
                         tjpf, 0);
        } else {
          byte[] rgbBuf = new byte[r.width() * r.height() * 3];
          tjd.decompress(rgbBuf, r.width(), 0, r.height(), TJ.PF_RGB, 0);
          pf.bufferFromRGB(data, r.tl.x, r.tl.y, rgbBuf, r.width(), stride[0],
                           r.height(), null);
        }
        handler.releaseRawPixels(r);
        return;
      } catch (java.lang.Exception e) {
        throw new Exception(e.getMessage());
      } catch (java.lang.UnsatisfiedLinkError e) {
        vlog.info("WARNING: TurboJPEG JNI library is not new enough.");
        vlog.info("  Using unaccelerated JPEG decompressor.");
        tjd = null;
      }
    }

    // Create an Image object from the JPEG data.
    Image jpeg = tk.createImage(netbuf);
    jpeg.setAccelerationPriority(1);
    handler.imageRect(r, jpeg);
    jpeg.flush();
  }

  final private void filterGradient24(byte[] netbuf, int[] buf, int stride, 
                                      Rect r)
  {

    int x, y, c;
    byte[] prevRow = new byte[TIGHT_MAX_WIDTH*3];
    byte[] thisRow = new byte[TIGHT_MAX_WIDTH*3];
    byte[] pix = new byte[3];
    int[] est = new int[3];

    // Set up shortcut variables
    int rectHeight = r.height();
    int rectWidth = r.width();

    for (y = 0; y < rectHeight; y++) {
      /* First pixel in a row */
      for (c = 0; c < 3; c++) {
        pix[c] = (byte)(netbuf[y*rectWidth*3+c] + prevRow[c]);
        thisRow[c] = pix[c];
      }
      serverpf.bufferFromRGB(buf, y*stride, pix, 0, 1);

      /* Remaining pixels of a row */
      for (x = 1; x < rectWidth; x++) {
        for (c = 0; c < 3; c++) {
          est[c] = (int)(prevRow[x*3+c] + pix[c] - prevRow[(x-1)*3+c]);
          if (est[c] > 0xFF) {
            est[c] = 0xFF;
          } else if (est[c] < 0) {
            est[c] = 0;
          }
          pix[c] = (byte)(netbuf[(y*rectWidth+x)*3+c] + est[c]);
          thisRow[x*3+c] = pix[c];
        }
        serverpf.bufferFromRGB(buf, y*stride+x, pix, 0, 1);
      }

      System.arraycopy(thisRow, 0, prevRow, 0, prevRow.length);
    }
  }

  final private void filterGradient(byte[] netbuf, int[] buf, int stride, 
                                    Rect r)
  {

    int x, y, c;
    byte[] prevRow = new byte[TIGHT_MAX_WIDTH];
    byte[] thisRow = new byte[TIGHT_MAX_WIDTH];
    byte[] pix = new byte[3];
    int[] est = new int[3];

    // Set up shortcut variables
    int rectHeight = r.height();
    int rectWidth = r.width();

    for (y = 0; y < rectHeight; y++) {
      /* First pixel in a row */
      // FIXME
      //serverpf.rgbFromBuffer(pix, 0, netbuf, y*rectWidth, 1, cm);
      for (c = 0; c < 3; c++)
        pix[c] += prevRow[c];

      System.arraycopy(pix, 0, thisRow, 0, pix.length);

      serverpf.bufferFromRGB(buf, y*stride, pix, 0, 1);
      
      /* Remaining pixels of a row */
      for (x = 1; x < rectWidth; x++) {
        for (c = 0; c < 3; c++) {
          est[c] = (int)(prevRow[x*3+c] + pix[c] - prevRow[(x-1)*3+c]);
          if (est[c] > 0xff) {
            est[c] = 0xff;
          } else if (est[c] < 0) {
            est[c] = 0;
          }
        }

        // FIXME
        //serverpf.rgbFromBuffer(pix, 0, netbuf, y*rectWidth+x, 1, cm);
        for (c = 0; c < 3; c++)
          pix[c] += est[c];

        System.arraycopy(pix, 0, thisRow, x*3, pix.length);

        serverpf.bufferFromRGB(buf, y*stride+x, pix, 0, 1);
      }

      System.arraycopy(thisRow, 0, prevRow, 0, prevRow.length);
    }
  }

  private CMsgReader reader;
  private ZlibInStream[] zis;
  private PixelFormat serverpf;
  private PixelFormat clientpf;
  static LogWriter vlog = new LogWriter("TightDecoder");
  private TJDecompressor tjd;

}
