/* Copyright (C) 2012, 2017-2018, 2022 D. R. Commander.  All Rights Reserved.
 * Copyright 2016, 2018-2019 Pierre Ossman for Cendio AB
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
// CMsgReader - class for reading RFB messages on the client side
// (i.e. messages from server to client).
//

package com.turbovnc.rfb;

import java.util.*;

import com.turbovnc.rdr.*;

public class CMsgReader {

  public CMsgReader(CMsgHandler handler_, InStream is_) {
    imageBufIdealSize = 0;
    handler = handler_;
    is = is_;
    imageBuf = null;
    imageBufSize = 0;
    decoders = new Decoder[RFB.ENCODING_MAX + 1];
    nUpdateRectsLeft = 0;
  }

  public final int bpp() {
    return handler.cp.pf().bpp;
  }

  public final void close() {
    for (int i = 0; i < RFB.ENCODING_MAX; i++) {
      if (decoders[i] != null)
        decoders[i].close();
    }
  }

  public int[] getImageBuf(int required) {
    return getImageBuf(required, 0, 0);
  }

  private int[] getImageBuf(int required, int requested, int nPixels) {
    int requiredBytes = required;
    int requestedBytes = requested;
    int size = requestedBytes;
    if (size > imageBufIdealSize) size = imageBufIdealSize;

    if (size < requiredBytes)
      size = requiredBytes;

    if (imageBufSize < size) {
      imageBufSize = size;
      imageBuf = new int[imageBufSize];
    }
    if (nPixels != 0)
      nPixels = imageBufSize / (handler.cp.pf().bpp / 8);
    return imageBuf;
  }

  public InStream getInStream() { return is; }

  public final boolean isTurboJPEG() {
    Decoder d = decoders[RFB.ENCODING_TIGHT];
    if (d instanceof TightDecoder && d != null)
      return ((TightDecoder)d).isTurboJPEG();
    return false;
  }

  private void readBell() {
    handler.bell();
  }

  private void readCopyRect(Rect r) {
    int srcX = is.readU16();
    int srcY = is.readU16();
    handler.copyRect(r, srcX, srcY);
  }

  private void readEndOfContinuousUpdates() {
    handler.endOfContinuousUpdates();
  }

  private void readExtendedClipboard(int len, Params params)
  {
    int action, flags;

    if (len < 4)
      throw new ErrorException("Malformed Extended Clipboard message");
    if (len > params.maxClipboard.get()) {
      vlog.error("Ignoring " + len +
                 "-byte Extended Clipboard message (limit = " +
                 params.maxClipboard.get() + " bytes)");
      is.skip(len);
      return;
    }

    flags = is.readU32();
    action = flags & RFB.EXTCLIP_ACTION_MASK;

    if ((action & RFB.EXTCLIP_ACTION_CAPS) != 0) {
      int i, num;
      int[] lengths = new int[16];

      num = 0;
      for (i = 0; i < 16; i++) {
        if ((flags & (1 << i)) != 0)
          num++;
      }

      if (len < (4 + 4 * num))
        throw new ErrorException("Malformed Extended Clipboard message");

      num = 0;
      for (i = 0; i < 16; i++) {
        if ((flags & (1 << i)) != 0)
          lengths[num++] = is.readU32();
      }

      handler.handleClipboardCaps(flags, lengths);
    } else if (action == RFB.EXTCLIP_ACTION_PROVIDE) {
      ZlibInStream zis = new ZlibInStream();

      int i, num, ignoredBytes = 0;
      int[] lengths = new int[16];
      byte[][] buffers = new byte[16][];

      zis.setUnderlying(is, len - 4);

      num = 0;
      for (i = 0; i < 16; i++) {
        if ((flags & (1 << i)) == 0)
          continue;

        lengths[num] = zis.readU32();

        if (lengths[num] > params.maxClipboard.get()) {
          vlog.error("Truncating " + lengths[num] +
                     "-byte incoming clipboard update to " +
                     params.maxClipboard.get() + " bytes.");
          ignoredBytes = lengths[num] - params.maxClipboard.get();
          lengths[num] = params.maxClipboard.get();
        }

        buffers[num] = new byte[lengths[num]];
        zis.readBytes(buffers[num], 0, lengths[num]);

        if (ignoredBytes != 0)
          zis.skip(ignoredBytes);

        num++;
      }

      zis.reset();

      handler.handleClipboardProvide(flags, lengths, buffers);
    } else {
      switch (action) {
        case RFB.EXTCLIP_ACTION_REQUEST:
          handler.handleClipboardRequest(flags);
          break;
        case RFB.EXTCLIP_ACTION_PEEK:
          handler.handleClipboardPeek(flags);
          break;
        case RFB.EXTCLIP_ACTION_NOTIFY:
          handler.handleClipboardNotify(flags);
          break;
        default:
          throw new ErrorException("Invalid Extended Clipboard action");
      }
    }

    return;
  }

  private void readExtendedDesktopSize(int x, int y, int w, int h) {
    int screens, i;
    int id, flags;
    int sx, sy, sw, sh;
    ScreenSet layout = new ScreenSet();

    screens = is.readU8();
    is.skip(3);

    for (i = 0; i < screens; i++) {
      id = is.readU32();
      sx = is.readU16();
      sy = is.readU16();
      sw = is.readU16();
      sh = is.readU16();
      flags = is.readU32();

      layout.addScreen(new Screen(id, sx, sy, sw, sh, flags));
    }
    layout.debugPrint("LAYOUT RECEIVED");

    handler.setExtendedDesktopSize(x, y, w, h, layout);
  }

  private void readFence() {
    int flags;
    int len;
    byte[] data = new byte[64];

    is.skip(3);

    flags = is.readU32();

    len = is.readU8();
    if (len > data.length) {
      System.err.println("Ignoring fence with too large payload\n");
      is.skip(len);
      return;
    }

    is.readBytes(data, 0, len);

    handler.fence(flags, len, data);
  }

  private void readFramebufferUpdate() {
    is.skip(1);
    nUpdateRectsLeft = is.readU16();
    handler.framebufferUpdateStart();
  }

  private void readGII() {
    int endianAndSubType = is.readU8();
    int endian = endianAndSubType & RFB.GII_BE;
    int subType = endianAndSubType & ~RFB.GII_BE;

    if (endian != RFB.GII_BE) {
      vlog.error("ERROR: don't know how to handle little endian GII messages");
      is.skip(6);
      return;
    }

    int length = is.readU16();
    if (length != 4) {
      vlog.error("ERROR: improperly formatted GII server message");
      is.skip(4);
      return;
    }

    switch (subType) {
      case RFB.GII_VERSION:
        int maximumVersion = is.readU16();
        int minimumVersion = is.readU16();
        if (maximumVersion < 1 || minimumVersion > 1) {
          vlog.error("ERROR: GII version mismatch");
          return;
        }
        if (minimumVersion != maximumVersion)
          vlog.debug("Server supports GII versions " + minimumVersion + " - " +
                     maximumVersion);
        else
          vlog.debug("Server supports GII version " + minimumVersion);
        handler.enableGII();
        break;

      case RFB.GII_DEVICE_CREATE:
        int deviceOrigin = is.readU32();
        if (deviceOrigin == 0) {
          vlog.error("ERROR: Could not create GII device");
          return;
        }
        handler.giiDeviceCreated(deviceOrigin);
        break;
    }
  }

  // readMsg() reads a message, calling the handler as appropriate.
  public void readMsg(Params params) {
    if (nUpdateRectsLeft == 0) {

      int type = is.readU8();
      switch (type) {
        case RFB.FRAMEBUFFER_UPDATE:
          readFramebufferUpdate();  break;
        case RFB.SET_COLOUR_MAP_ENTRIES:
          readSetColourMapEntries();  break;
        case RFB.BELL:
          readBell();  break;
        case RFB.SERVER_CUT_TEXT:
          readServerCutText(params);  break;
        case RFB.FENCE:
          readFence();  break;
        case RFB.END_OF_CONTINUOUS_UPDATES:
          readEndOfContinuousUpdates();  break;
        case RFB.GII:
          readGII();  break;
        default:
          vlog.error("Unknown message type " + type);
          throw new ErrorException("Unknown message type " + type);
      }

    } else {

      int x = is.readU16();
      int y = is.readU16();
      int w = is.readU16();
      int h = is.readU16();
      int encoding = is.readS32();

      switch (encoding) {
        case RFB.ENCODING_NEW_FB_SIZE:
          handler.setDesktopSize(w, h);
          break;
        case RFB.ENCODING_EXTENDED_DESKTOP_SIZE:
          readExtendedDesktopSize(x, y, w, h);
          break;
        case RFB.ENCODING_DESKTOP_NAME:
          readSetDesktopName(x, y, w, h);
          break;
        case RFB.ENCODING_X_CURSOR:
          readSetXCursor(w, h, new Point(x, y));
          break;
        case RFB.ENCODING_RICH_CURSOR:
          readSetCursor(w, h, new Point(x, y));
          break;
        case RFB.ENCODING_LAST_RECT:
          nUpdateRectsLeft = 1;   // this rectangle is the last one
          break;
        case RFB.ENCODING_QEMU_EXTENDED_KEY_EVENT:
          handler.enableQEMUExtKeyEvent();
          break;
        case RFB.ENCODING_QEMU_LED_STATE:
          readQEMULEDState();
          break;
        case RFB.ENCODING_VMWARE_LED_STATE:
          readVMwareLEDState();
          break;
        default:
          readRect(new Rect(x, y, x + w, y + h), encoding);
          break;
      }

      nUpdateRectsLeft--;
      if (nUpdateRectsLeft == 0) handler.framebufferUpdateEnd();
    }
  }

  private void readQEMULEDState()
  {
    int state = is.readU8();

    handler.setLEDState(state);
  }

  private void readRect(Rect r, int encoding) {
    if ((r.br.x > handler.cp.width) || (r.br.y > handler.cp.height)) {
      vlog.error("Rect too big: " + r.width() + "x" + r.height() + " at " +
                  r.tl.x + "," + r.tl.y + " exceeds " + handler.cp.width +
                  "x" + handler.cp.height);
      throw new ErrorException("Rect too big");
    }

    if (r.isEmpty())
      vlog.error("Ignoring zero size rect");

    handler.beginRect(r, encoding);

    if (encoding == RFB.ENCODING_COPYRECT) {
      readCopyRect(r);
    } else {

      if (decoders[encoding] == null) {
        decoders[encoding] = Decoder.createDecoder(encoding, this);
        if (decoders[encoding] == null) {
          vlog.error("Unknown rect encoding " + encoding);
          throw new ErrorException("Unknown rect encoding" + encoding);
        }
      }
      handler.startDecodeTimer();
      decoders[encoding].readRect(r, handler);
      handler.stopDecodeTimer();
    }

    handler.endRect(r, encoding);
  }

  private void readServerCutText(Params params) {
    int ignoredBytes = 0;
    is.skip(3);
    int len = is.readU32();

    if ((len & 0x80000000) != 0) {
      len = -len;
      readExtendedClipboard(len, params);
      return;
    }

    if (len > params.maxClipboard.get()) {
      ignoredBytes = len - params.maxClipboard.get();
      vlog.error("Truncating " + len + "-byte incoming clipboard update to " +
                 params.maxClipboard.get() + " bytes.");
      len = params.maxClipboard.get();
    }
    byte[] buf = new byte[len];
    is.readBytes(buf, 0, len);
    is.skip(ignoredBytes);
    String str = null;
    try {
      str = new String(buf, "UTF8");
    } catch (java.io.UnsupportedEncodingException e) {
      e.printStackTrace();
      return;
    }
    str = Utils.convertLF(str);
    handler.serverCutText(str);
  }

  public void readServerInit(boolean benchmark) {
    int width = is.readU16();
    int height = is.readU16();
    handler.setDesktopSize(width, height);
    PixelFormat pf = new PixelFormat();
    pf.read(is);
    handler.setPixelFormat(pf);
    String name = is.readString();
    handler.setName(name);
    if (!benchmark &&
        handler.getCurrentCSecurity().getType() == RFB.SECTYPE_TIGHT) {
      int nServerMsg = is.readU16();
      int nClientMsg = is.readU16();
      int nEncodings = is.readU16();
      is.skip(2);
      List<byte[]> serverMsgCaps = new ArrayList<byte[]>();
      for (int i = 0; i < nServerMsg; i++) {
        byte[] cap = new byte[16];
        is.readBytes(cap, 0, 16);
        serverMsgCaps.add(cap);
      }
      List<byte[]> clientMsgCaps = new ArrayList<byte[]>();
      for (int i = 0; i < nClientMsg; i++) {
        byte[] cap = new byte[16];
        is.readBytes(cap, 0, 16);
        clientMsgCaps.add(cap);
      }
      List<byte[]> supportedEncodings = new ArrayList<byte[]>();
      for (int i = 0; i < nEncodings; i++) {
        byte[] cap = new byte[16];
        is.readBytes(cap, 0, 16);
        supportedEncodings.add(cap);
      }
    }
    handler.serverInit();
  }

  private void readSetColourMapEntries() {
    is.skip(1);
    int firstColour = is.readU16();
    int nColours = is.readU16();
    int[] rgbs = new int[nColours * 3];
    for (int i = 0; i < nColours * 3; i++)
      rgbs[i] = is.readU16();
    handler.setColourMapEntries(firstColour, nColours, rgbs);
  }

  private void readSetCursor(int width, int height, Point hotspot) {
    int dataLen = width * height;
    int maskLen = ((width + 7) / 8) * height;
    int[] data = new int[dataLen];
    byte[] mask = new byte[maskLen];

    is.readPixels(data, dataLen, (handler.cp.pf().bpp / 8),
                  handler.cp.pf().bigEndian);
    is.readBytes(mask, 0, maskLen);

    handler.setCursor(width, height, hotspot, data, mask);
  }

  private void readSetDesktopName(int x, int y, int w, int h) {
    String name = is.readString();

    if (x != 0 || y != 0 || w != 0 || h != 0) {
      vlog.error("Ignoring DesktopName rect with non-zero position/size");
    } else {
      handler.setName(name);
    }
  }

  private void readSetXCursor(int width, int height, Point hotspot)
  {
    byte r, g, b;
    int x, y, n, bytesPerRow = ((width + 7) / 8), len = bytesPerRow * height;
    byte[] data, mask;
    int[] cursor, colors = new int[2];

    if (width * height == 0)
      return;

    r = (byte)is.readU8();
    g = (byte)is.readU8();
    b = (byte)is.readU8();
    colors[1] = handler.cp.pf().pixelFromRGB(r, g, b, null);

    r = (byte)is.readU8();
    g = (byte)is.readU8();
    b = (byte)is.readU8();
    colors[0] = handler.cp.pf().pixelFromRGB(r, g, b, null);

    data = new byte[len];
    mask = new byte[len];
    cursor = new int[width * height];

    is.readBytes(data, 0, len);
    is.readBytes(mask, 0, len);

    int i = 0;
    for (y = 0; y < height; y++) {
      for (x = 0; x < width / 8; x++) {
        byte dataByte = data[y * bytesPerRow + x];
        for (n = 7; n >= 0; n--)
          cursor[i++] = colors[dataByte >> n & 1];
      }
      for (n = 7; n >= 8 - width % 8; n--) {
        byte dataByte = data[y * bytesPerRow + x];
        cursor[i++] = colors[dataByte >> n & 1];
      }
    }

    handler.setCursor(width, height, hotspot, cursor, mask);
  }

  private void readVMwareLEDState()
  {
    int state = is.readU32();

    // As luck would have it, this extension uses the same bit definitions as
    // the QEMU LED State extension, so no conversion is required.
    handler.setLEDState(state);
  }

  public final void reset() {
    for (int i = 0; i < RFB.ENCODING_MAX; i++) {
      if (decoders[i] != null)
        decoders[i].reset();
    }
  }

  private int imageBufIdealSize;

  private CMsgHandler handler;
  private InStream is;
  private Decoder[] decoders;
  private int[] imageBuf;
  private int imageBufSize;
  private int nUpdateRectsLeft;

  static LogWriter vlog = new LogWriter("CMsgReader");
}
