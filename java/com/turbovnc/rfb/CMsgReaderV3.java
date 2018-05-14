/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright 2009-2011 Pierre Ossman for Cendio AB
 * Copyright (C) 2011 Brian P. Hinz
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

import java.util.*;
import com.turbovnc.rdr.*;

public class CMsgReaderV3 extends CMsgReader {

  public CMsgReaderV3(CMsgHandler handler_, InStream is_) {
    super(handler_, is_);
    nUpdateRectsLeft = 0;
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

  public void readMsg() {
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
          readServerCutText();  break;
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
        case RFB.ENCODING_CLIENT_REDIRECT:
          readClientRedirect(x, y, w, h);
          break;
        default:
          readRect(new Rect(x, y, x + w, y + h), encoding);
          break;
      }

      nUpdateRectsLeft--;
      if (nUpdateRectsLeft == 0) handler.framebufferUpdateEnd();
    }
  }

  void readFramebufferUpdate() {
    is.skip(1);
    nUpdateRectsLeft = is.readU16();
    handler.framebufferUpdateStart();
  }

  void readSetDesktopName(int x, int y, int w, int h) {
    String name = is.readString();

    if (x != 0 || y != 0 || w != 0 || h != 0) {
      vlog.error("Ignoring DesktopName rect with non-zero position/size");
    } else {
      handler.setName(name);
    }

  }

  void readExtendedDesktopSize(int x, int y, int w, int h) {
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

  void readFence() {
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

  void readEndOfContinuousUpdates() {
    handler.endOfContinuousUpdates();
  }

  void readGII() {
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

  void readClientRedirect(int x, int y, int w, int h) {
    int port = is.readU16();
    String host = is.readString();
    String x509subject = is.readString();

    if (x != 0 || y != 0 || w != 0 || h != 0) {
      vlog.error("Ignoring ClientRedirect rect with non-zero position/size");
    } else {
      handler.clientRedirect(port, host, x509subject);
    }
  }

  int nUpdateRectsLeft;

  static LogWriter vlog = new LogWriter("CMsgReaderV3");
}
