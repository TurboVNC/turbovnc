/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright 2009-2011 Pierre Ossman for Cendio AB
 * Copyright (C) 2011, 2015 Brian P. Hinz
 * Copyright (C) 2012, 2015, 2017-2018 D. R. Commander.  All Rights Reserved.
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

import java.nio.ByteBuffer;
import java.nio.charset.Charset;
import com.turbovnc.rdr.*;
import com.turbovnc.vncviewer.*;

public abstract class CMsgWriter {

  public abstract void writeClientInit(boolean shared);

  public synchronized void writeSetPixelFormat(PixelFormat pf)
  {
    startMsg(RFB.SET_PIXEL_FORMAT);
    os.pad(3);
    pf.write(os);
    endMsg();
  }

  public synchronized void writeSetEncodings(int nEncodings, int[] encodings)
  {
    startMsg(RFB.SET_ENCODINGS);
    os.skip(1);
    os.writeU16(nEncodings);
    for (int i = 0; i < nEncodings; i++)
      os.writeU32(encodings[i]);
    endMsg();
  }

  // Ask for encodings based on which decoders are supported.  Assumes higher
  // encoding numbers are more desirable.

  public synchronized void writeSetEncodings(int preferredEncoding,
                                             int lastEncoding, Options opts)
  {
    int nEncodings = 0;
    int[] encodings = new int[RFB.ENCODING_MAX + 3];
    if (opts.cursorShape) {
      if (!VncViewer.getBooleanProperty("turbovnc.forcexcursor", false))
        encodings[nEncodings++] = RFB.ENCODING_RICH_CURSOR;
      encodings[nEncodings++] = RFB.ENCODING_X_CURSOR;
    }
    if (cp.supportsDesktopResize)
      encodings[nEncodings++] = RFB.ENCODING_NEW_FB_SIZE;
    if (cp.supportsExtendedDesktopSize)
      encodings[nEncodings++] = RFB.ENCODING_EXTENDED_DESKTOP_SIZE;
    if (cp.supportsDesktopRename)
      encodings[nEncodings++] = RFB.ENCODING_DESKTOP_NAME;
    if (cp.supportsClientRedirect)
      encodings[nEncodings++] = RFB.ENCODING_CLIENT_REDIRECT;

    encodings[nEncodings++] = RFB.ENCODING_LAST_RECT;
    if (opts.continuousUpdates) {
      encodings[nEncodings++] = RFB.ENCODING_CONTINUOUS_UPDATES;
      encodings[nEncodings++] = RFB.ENCODING_FENCE;
    }
    encodings[nEncodings++] = RFB.ENCODING_GII;

    if (Decoder.supported(preferredEncoding)) {
      encodings[nEncodings++] = preferredEncoding;
    }

    if (opts.copyRect) {
      encodings[nEncodings++] = RFB.ENCODING_COPYRECT;
    }

    /*
     * Prefer encodings in this order:
     *
     *   Tight, ZRLE, Hextile, *
     */

    if ((preferredEncoding != RFB.ENCODING_TIGHT) &&
        Decoder.supported(RFB.ENCODING_TIGHT))
      encodings[nEncodings++] = RFB.ENCODING_TIGHT;

    if ((preferredEncoding != RFB.ENCODING_ZRLE) &&
        Decoder.supported(RFB.ENCODING_ZRLE))
      encodings[nEncodings++] = RFB.ENCODING_ZRLE;

    if ((preferredEncoding != RFB.ENCODING_HEXTILE) &&
        Decoder.supported(RFB.ENCODING_HEXTILE))
      encodings[nEncodings++] = RFB.ENCODING_HEXTILE;

    // Remaining encodings
    for (int i = RFB.ENCODING_MAX; i >= 0; i--) {
      switch (i) {
        case RFB.ENCODING_TIGHT:
        case RFB.ENCODING_ZRLE:
        case RFB.ENCODING_HEXTILE:
          break;
        default:
          if ((i != preferredEncoding) && Decoder.supported(i))
            encodings[nEncodings++] = i;
      }
    }

    encodings[nEncodings++] = RFB.ENCODING_LAST_RECT;
    if (opts.compressLevel >= 0 && opts.compressLevel <= 9)
      encodings[nEncodings++] = RFB.ENCODING_COMPRESS_LEVEL_0 +
        opts.compressLevel;
    if (opts.allowJpeg && opts.preferredEncoding == RFB.ENCODING_TIGHT) {
      int qualityLevel = opts.quality / 10;
      if (qualityLevel > 9) qualityLevel = 9;
      encodings[nEncodings++] = RFB.ENCODING_QUALITY_LEVEL_0 + qualityLevel;
      encodings[nEncodings++] = RFB.ENCODING_FINE_QUALITY_LEVEL_0 +
        opts.quality;
      encodings[nEncodings++] = RFB.ENCODING_SUBSAMP_1X + opts.subsampling;
    } else if (opts.preferredEncoding != RFB.ENCODING_TIGHT ||
               (lastEncoding >= 0 && lastEncoding != RFB.ENCODING_TIGHT)) {
      int qualityLevel = opts.quality;
      if (qualityLevel > 9) qualityLevel = 9;
      encodings[nEncodings++] = RFB.ENCODING_QUALITY_LEVEL_0 + qualityLevel;
    }

    writeSetEncodings(nEncodings, encodings);
  }

  public synchronized void writeFramebufferUpdateRequest(Rect r,
                                                         boolean incremental)
  {
    startMsg(RFB.FRAMEBUFFER_UPDATE_REQUEST);
    os.writeU8(incremental ? 1 : 0);
    os.writeU16(r.tl.x);
    os.writeU16(r.tl.y);
    os.writeU16(r.width());
    os.writeU16(r.height());
    endMsg();
  }

  public synchronized void writeKeyEvent(int key, boolean down)
  {
    startMsg(RFB.KEY_EVENT);
    os.writeU8(down ? 1 : 0);
    os.pad(2);
    os.writeU32(key);
    endMsg();
  }

  public synchronized void writePointerEvent(Point pos, int buttonMask)
  {
    Point p = new Point(pos.x, pos.y);
    if (p.x < 0) p.x = 0;
    if (p.y < 0) p.y = 0;
    if (p.x >= cp.width) p.x = cp.width - 1;
    if (p.y >= cp.height) p.y = cp.height - 1;

    startMsg(RFB.POINTER_EVENT);
    os.writeU8(buttonMask);
    os.writeU16(p.x);
    os.writeU16(p.y);
    endMsg();
  }

  public synchronized void writeClientCutText(String str, int len)
  {
    startMsg(RFB.CLIENT_CUT_TEXT);
    os.pad(3);
    os.writeU32(len);
    Charset latin1 = Charset.forName("ISO-8859-1");
    ByteBuffer bytes = latin1.encode(str);
    os.writeBytes(bytes.array(), 0, len);
    endMsg();
  }

  public abstract void startMsg(int type);
  public abstract void endMsg();

  public synchronized void setOutStream(OutStream os_) { os = os_; }

  ConnParams getConnParams() { return cp; }
  OutStream getOutStream() { return os; }

  protected CMsgWriter(ConnParams cp_, OutStream os_) {
    cp = cp_;  os = os_;
  }

  ConnParams cp;
  OutStream os;

  static LogWriter vlog = new LogWriter("CMsgWriter");
}
