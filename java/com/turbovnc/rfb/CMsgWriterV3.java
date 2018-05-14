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

import com.turbovnc.rdr.*;
import java.util.*;

public class CMsgWriterV3 extends CMsgWriter {

  public CMsgWriterV3(ConnParams cp_, OutStream os_) { super(cp_, os_); }

  public synchronized void writeClientInit(boolean shared) {
    os.writeU8(shared ? 1 : 0);
    endMsg();
  }

  public synchronized void startMsg(int type) {
    os.writeU8(type);
  }

  public synchronized void endMsg() {
    os.flush();
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

  public synchronized void writeGIIVersion() {
    if (!cp.supportsGII)
      throw new ErrorException("Server does not support GII");

    startMsg(RFB.GII);

    os.writeU8(RFB.GII_VERSION | RFB.GII_BE);
    os.writeU16(2);
    os.writeU16(1);

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
}
