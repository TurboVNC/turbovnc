/* Copyright (C) 2012, 2017-2018, 2022-2023 D. R. Commander.
 *                                          All Rights Reserved.
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

public final class BoolParameter extends VoidParameter {

  public BoolParameter(String name, Params params, boolean isGUI,
                       String desc, boolean defValue_) {
    super(name, params, isGUI, desc);
    value = defValue = defValue_;
  }

  public boolean set(String str) {
    return set(str, false, false);
  }

  public synchronized boolean set(String str, boolean reverse_,
                                  boolean commandLine_) {
    if (str.equals("1") || str.equalsIgnoreCase("on") ||
        str.equalsIgnoreCase("true") || str.equalsIgnoreCase("yes"))
      set(reverse_ ? false : true);
    else if (str.equals("0") || str.equalsIgnoreCase("off") ||
             str.equalsIgnoreCase("false") || str.equalsIgnoreCase("no"))
      set(reverse_ ? true : false);
    else
      return false;
    setCommandLine(commandLine_);
    return true;
  }

  public synchronized void set(boolean value_) {
    value = value_;
    setCommandLine(false);
  }

  public synchronized void reset() {
    set(defValue);
    reverse = false;
  }

  public synchronized boolean setDefault(String str) {
    if (str.equals("1") || str.equalsIgnoreCase("on") ||
        str.equalsIgnoreCase("true") || str.equalsIgnoreCase("yes"))
      value = defValue = true;
    else if (str.equals("0") || str.equalsIgnoreCase("off") ||
             str.equalsIgnoreCase("false") || str.equalsIgnoreCase("no"))
      value = defValue = false;
    else
      return false;
    return true;
  }

  public synchronized boolean get() { return value; }
  public synchronized String getDefaultStr() { return defValue ? "1" : "0"; }
  public synchronized String getStr() { return Boolean.toString(value); }
  public String getValues() { return "0, 1"; }
  public boolean isBool() { return true; }

  @SuppressWarnings("checkstyle:VisibilityModifier")
  public boolean reverse;

  private boolean value, defValue;
}
