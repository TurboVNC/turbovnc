/* Copyright (C) 2015, 2022-2023 D. R. Commander.  All Rights Reserved.
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

public final class DesktopSizeParameter extends VoidParameter {

  public DesktopSizeParameter(String name, Params params, boolean isGUI,
                              String desc, String defValue_) {
    super(name, params, isGUI, desc);
    defValue = defValue_;
    set(defValue);
  }

  public synchronized boolean set(String sizeString) {
    DesktopSize size = DesktopSize.parse(sizeString);
    if (size == null)
      throw new ErrorException(getName() + " parameter is incorrect");
    desktopSize = size;
    setCommandLine(false);
    return true;
  }

  public synchronized void set(int width, int height) {
    desktopSize.width = width;
    desktopSize.height = height;
    setCommandLine(false);
  }

  public synchronized void reset() { set(defValue); }

  public synchronized void setMode(int mode) {
    desktopSize.mode = mode;
    setCommandLine(false);
  }

  public synchronized boolean setDefault(String sizeString) {
    DesktopSize size = DesktopSize.parse(sizeString);
    if (size == null)
      return false;
    defValue = size.getString();
    set(defValue);
    return true;
  }

  public synchronized boolean equalsIgnoreID(DesktopSizeParameter sizeParam) {
    return sizeParam.desktopSize.equalsIgnoreID(desktopSize);
  }

  public synchronized String getDefaultStr() { return defValue; }
  public synchronized int getHeight() { return desktopSize.height; }
  public synchronized ScreenSet getLayout() { return desktopSize.layout; }
  public synchronized int getMode() { return desktopSize.mode; }
  public synchronized String getStr() { return desktopSize.getString(); }
  public synchronized int getWidth() { return desktopSize.width; }

  public String getValues() {
    return "WxH, W0xH0+X0+Y0[,W1xH1+X1+Y1,...], Auto, or Server";
  }

  private DesktopSize desktopSize;
  private String defValue;
};
