/* Copyright (C) 2012, 2015, 2017-2018, 2020-2023 D. R. Commander.
 *                                                All Rights Reserved.
 * Copyright 2009-2011, 2017-2019 Pierre Ossman for Cendio AB
 * Copyright (C) 2011, 2015 Brian P. Hinz
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

package com.turbovnc.rfb;

import java.nio.ByteBuffer;
import java.nio.charset.*;
import java.util.*;

import com.turbovnc.rdr.*;

public class CMsgWriter {

  public CMsgWriter(ConnParams cp_, OutStream os_) {
    cp = cp_;  os = os_;
  }

  private synchronized void endMsg() {
    os.flush();
  }

  private synchronized void startMsg(int type) {
    os.writeU8(type);
  }

  public synchronized void writeClientCutText(String str)
  {
    int len = str.length();

    startMsg(RFB.CLIENT_CUT_TEXT);
    os.pad(3);
    os.writeU32(len);
    ByteBuffer bytes = StandardCharsets.ISO_8859_1.encode(str);
    os.writeBytes(bytes.array(), 0, len);
    endMsg();
  }

  public synchronized void writeClientInit(boolean shared) {
    os.writeU8(shared ? 1 : 0);
    endMsg();
  }

  public synchronized void writeClipboardCaps(int caps, int[] lengths)
  {
    int i, count;

    if ((cp.clipboardFlags() & RFB.EXTCLIP_ACTION_CAPS) == 0)
      throw new ErrorException("Server does not support Extended Clipboard caps message");

    count = 0;
    for (i = 0; i < 16; i++) {
      if ((caps & (1 << i)) != 0)
        count++;
    }

    startMsg(RFB.CLIENT_CUT_TEXT);
    os.pad(3);
    os.writeS32(-(4 + 4 * count));

    os.writeU32(caps | RFB.EXTCLIP_ACTION_CAPS);

    count = 0;
    for (i = 0; i < 16; i++) {
      if ((caps & (1 << i)) != 0)
        os.writeU32(lengths[count++]);
    }

    endMsg();
  }

  public synchronized void writeClipboardNotify(int flags)
  {
    if ((cp.clipboardFlags() & RFB.EXTCLIP_ACTION_NOTIFY) == 0)
      throw new ErrorException("Server does not support Extended Clipboard notify message");

    startMsg(RFB.CLIENT_CUT_TEXT);
    os.pad(3);
    os.writeS32(-4);
    os.writeU32(flags | RFB.EXTCLIP_ACTION_NOTIFY);
    endMsg();
  }

  public synchronized void writeClipboardPeek(int flags)
  {
    if ((cp.clipboardFlags() & RFB.EXTCLIP_ACTION_PEEK) == 0)
      throw new ErrorException("Server does not support Extended Clipboard peek message");

    startMsg(RFB.CLIENT_CUT_TEXT);
    os.pad(3);
    os.writeS32(-4);
    os.writeU32(flags | RFB.EXTCLIP_ACTION_PEEK);
    endMsg();
  }

  public synchronized void writeClipboardProvide(int flags, int[] lengths,
                                                 byte[][] data)
  {
    MemOutStream mos = new MemOutStream();
    ZlibOutStream zos = new ZlibOutStream();

    int i, count;

    if ((cp.clipboardFlags() & RFB.EXTCLIP_ACTION_PROVIDE) == 0)
      throw new ErrorException("Server does not support Extended Clipboard provide message");

    zos.setUnderlying(mos);

    count = 0;
    for (i = 0; i < 16; i++) {
      if ((flags & (1 << i)) == 0)
        continue;
      zos.writeU32(lengths[count]);
      zos.writeBytes(data[count], 0, lengths[count]);
      count++;
    }

    zos.flush();
    zos.close();

    startMsg(RFB.CLIENT_CUT_TEXT);
    os.pad(3);
    os.writeS32(-(4 + mos.length()));
    os.writeU32(flags | RFB.EXTCLIP_ACTION_PROVIDE);
    os.writeBytes(mos.data(), 0, mos.length());
    endMsg();
  }

  public synchronized void writeClipboardRequest(int flags)
  {
    if ((cp.clipboardFlags() & RFB.EXTCLIP_ACTION_REQUEST) == 0)
      throw new ErrorException("Server does not support Extended Clipboard request message");

    startMsg(RFB.CLIENT_CUT_TEXT);
    os.pad(3);
    os.writeS32(-4);
    os.writeU32(flags | RFB.EXTCLIP_ACTION_REQUEST);
    endMsg();
  }

  public synchronized void writeEnableContinuousUpdates(boolean enable,
                                                        int x, int y,
                                                        int w, int h) {
    if (!cp.supportsContinuousUpdates)
      throw new ErrorException("Server does not support continuous updates");

    startMsg(RFB.ENABLE_CONTINUOUS_UPDATES);

    os.writeU8((enable ? 1 : 0));

    os.writeU16(x);
    os.writeU16(y);
    os.writeU16(w);
    os.writeU16(h);

    endMsg();
  }

  public synchronized void writeFence(int flags, int len, byte[] data) {
    if (!cp.supportsFence)
      throw new ErrorException("Server does not support fences");
    if (len > 64)
      throw new ErrorException("Fence payload too large");
    if ((flags & ~RFB.FENCE_FLAGS_SUPPORTED) != 0)
      throw new ErrorException("Unknown fence flags");

    startMsg(RFB.FENCE);
    os.pad(3);

    os.writeU32(flags);

    os.writeU8(len);
    os.writeBytes(data, 0, len);

    endMsg();
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

  public synchronized void writeGIIDeviceCreate(ExtInputDevice dev) {
    if (!cp.supportsGII)
      throw new ErrorException("Server does not support GII");

    startMsg(RFB.GII);

    os.writeU8(RFB.GII_DEVICE_CREATE | RFB.GII_BE);
    os.writeU16(56 + dev.valuators.size() * 116);
    os.writePaddedString(dev.name, 32);
    os.writeU32((int)dev.vendorID);
    os.writeU32((int)dev.productID);
    os.writeU32((int)dev.canGenerate);
    os.writeU32(dev.numRegisters);
    os.writeU32(dev.valuators.size());
    os.writeU32(dev.numButtons);

    for (Iterator<ExtInputDevice.Valuator> i = dev.valuators.iterator();
         i.hasNext();) {
      ExtInputDevice.Valuator v = (ExtInputDevice.Valuator)i.next();

      os.writeU32(v.index);
      os.writePaddedString(v.longName, 75);
      os.writePaddedString(v.shortName, 5);
      os.writeU32(v.rangeMin);
      os.writeU32(v.rangeCenter);
      os.writeU32(v.rangeMax);
      os.writeU32(v.siUnit);
      os.writeU32(v.siAdd);
      os.writeU32(v.siMul);
      os.writeU32(v.siDiv);
      os.writeU32(v.siShift);
    }

    endMsg();
  }

  public synchronized void writeGIIEvent(ExtInputDevice dev, ExtInputEvent e) {
    if (!cp.supportsGII)
      throw new ErrorException("Server does not support GII");

    if ((e.type == RFB.GII_BUTTON_PRESS || e.type == RFB.GII_BUTTON_RELEASE) &&
        (e.buttonNumber > dev.numButtons || e.buttonNumber < 1)) {
      vlog.error("Button " + e.buttonNumber + " event ignored.");
      vlog.error("  Device " + dev.name + " has buttons 1-" + dev.numButtons +
                 ".");
      return;
    }

    if ((e.type == RFB.GII_VALUATOR_RELATIVE ||
         e.type == RFB.GII_VALUATOR_ABSOLUTE) &&
        (e.firstValuator + e.numValuators > dev.valuators.size())) {
      vlog.error("Valuator " + e.firstValuator + "-" +
                 (e.firstValuator + e.numValuators - 1) + " event ignored.");
      vlog.error("  Device " + dev.name + " has valuators 0-" +
                 (dev.valuators.size() - 1) + ".");
      return;
    }

    startMsg(RFB.GII);

    os.writeU8(RFB.GII_EVENT | RFB.GII_BE);

    switch (e.type) {

      case RFB.GII_BUTTON_PRESS:
      case RFB.GII_BUTTON_RELEASE:

        os.writeU16(12);
        os.writeU8(12);
        os.writeU8(e.type);
        os.writeU16(0);
        os.writeU32(dev.remoteID);
        os.writeU32(e.buttonNumber);
        break;

      case RFB.GII_VALUATOR_RELATIVE:
      case RFB.GII_VALUATOR_ABSOLUTE:

        os.writeU16(16 + e.numValuators * 4);
        os.writeU8(16 + e.numValuators * 4);
        os.writeU8(e.type);
        os.writeU16(0);
        os.writeU32(dev.remoteID);
        os.writeU32(e.firstValuator);
        os.writeU32(e.numValuators);
        for (int i = 0; i < e.numValuators; i++)
          os.writeU32(e.valuators[i]);
        break;

    }

    endMsg();
  }

  public synchronized void writeGIIVersion() {
    if (!cp.supportsGII)
      throw new ErrorException("Server does not support GII");

    startMsg(RFB.GII);

    os.writeU8(RFB.GII_VERSION | RFB.GII_BE);
    os.writeU16(2);
    os.writeU16(1);

    endMsg();
  }

  public synchronized void writeKeyEvent(int keysym, int rfbKeyCode,
                                         boolean down)
  {
    if (!cp.supportsQEMUExtKeyEvent || rfbKeyCode == 0) {
      startMsg(RFB.KEY_EVENT);
      os.writeU8(down ? 1 : 0);
      os.pad(2);
      os.writeU32(keysym);
      endMsg();
    } else {
      startMsg(RFB.QEMU);
      os.writeU8(RFB.QEMU_EXTENDED_KEY_EVENT);
      os.writeU16(down ? 1 : 0);
      os.writeU32(keysym);
      os.writeU32(rfbKeyCode);
      endMsg();
    }
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

  public synchronized void writeSetDesktopSize(int width, int height,
                                               ScreenSet layout) {
    if (!cp.supportsSetDesktopSize)
      throw new ErrorException("Server does not support SetDesktopSize");

    startMsg(RFB.SET_DESKTOP_SIZE);
    os.pad(1);

    os.writeU16(width);
    os.writeU16(height);

    os.writeU8(layout.numScreens());
    os.pad(1);

    for (Iterator<Screen> iter = layout.screens.iterator(); iter.hasNext();) {
      Screen refScreen = (Screen)iter.next();
      os.writeU32(refScreen.id);
      os.writeU16(refScreen.dimensions.tl.x);
      os.writeU16(refScreen.dimensions.tl.y);
      os.writeU16(refScreen.dimensions.width());
      os.writeU16(refScreen.dimensions.height());
      os.writeU32(refScreen.flags);
    }

    endMsg();

    layout.debugPrint("LAYOUT SENT");
  }

  private synchronized void writeSetEncodings(int nEncodings, int[] encodings)
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
                                             int lastEncoding, Params params)
  {
    int nEncodings = 0;
    int[] encodings = new int[RFB.ENCODING_MAX + 3];
    if (params.cursorShape.get()) {
      if (!Utils.getBooleanProperty("turbovnc.forcexcursor", false))
        encodings[nEncodings++] = RFB.ENCODING_RICH_CURSOR;
      encodings[nEncodings++] = RFB.ENCODING_X_CURSOR;
    }
    if (cp.supportsDesktopResize)
      encodings[nEncodings++] = RFB.ENCODING_NEW_FB_SIZE;
    if (cp.supportsExtendedDesktopSize)
      encodings[nEncodings++] = RFB.ENCODING_EXTENDED_DESKTOP_SIZE;
    if (cp.supportsDesktopRename)
      encodings[nEncodings++] = RFB.ENCODING_DESKTOP_NAME;

    encodings[nEncodings++] = RFB.ENCODING_LAST_RECT;
    if (params.continuousUpdates.get()) {
      encodings[nEncodings++] = RFB.ENCODING_CONTINUOUS_UPDATES;
      encodings[nEncodings++] = RFB.ENCODING_FENCE;
    }
    encodings[nEncodings++] = RFB.ENCODING_EXTENDED_CLIPBOARD;
    if (Utils.getBooleanProperty("turbovnc.gii", true))
      encodings[nEncodings++] = RFB.ENCODING_GII;
    if (params.serverKeyMap.get()) {
      encodings[nEncodings++] = RFB.ENCODING_QEMU_EXTENDED_KEY_EVENT;
      encodings[nEncodings++] = RFB.ENCODING_QEMU_LED_STATE;
      encodings[nEncodings++] = RFB.ENCODING_VMWARE_LED_STATE;
    }

    if (Decoder.supported(preferredEncoding)) {
      encodings[nEncodings++] = preferredEncoding;
    }

    if (params.copyRect.get()) {
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
    if (params.compressLevel.get() >= 0 && params.compressLevel.get() <= 9)
      encodings[nEncodings++] = RFB.ENCODING_COMPRESS_LEVEL_0 +
        params.compressLevel.get();
    if (params.jpeg.get() &&
        params.encoding.get() == RFB.ENCODING_TIGHT) {
      int qualityLevel = params.quality.get() / 10;
      if (qualityLevel > 9) qualityLevel = 9;
      encodings[nEncodings++] = RFB.ENCODING_QUALITY_LEVEL_0 + qualityLevel;
      encodings[nEncodings++] = RFB.ENCODING_FINE_QUALITY_LEVEL_0 +
        params.quality.get();
      encodings[nEncodings++] = RFB.ENCODING_SUBSAMP_1X +
        params.subsampling.get();
    } else if (params.encoding.get() != RFB.ENCODING_TIGHT ||
               (lastEncoding >= 0 && lastEncoding != RFB.ENCODING_TIGHT)) {
      int qualityLevel = params.quality.get();
      if (qualityLevel > 9) qualityLevel = 9;
      encodings[nEncodings++] = RFB.ENCODING_QUALITY_LEVEL_0 + qualityLevel;
    }

    writeSetEncodings(nEncodings, encodings);
  }

  public synchronized void writeSetPixelFormat(PixelFormat pf)
  {
    startMsg(RFB.SET_PIXEL_FORMAT);
    os.pad(3);
    pf.write(os);
    endMsg();
  }

  private ConnParams cp;
  private OutStream os;

  static LogWriter vlog = new LogWriter("CMsgWriter");
}
