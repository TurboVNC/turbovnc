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

public final class GrabParameter extends IntParameter {

  public static final int FS = 0;
  public static final int ALWAYS = 1;
  public static final int MANUAL = 2;

  public GrabParameter(String name, Params params, boolean isGUI, String desc,
                       int defValue) {
    super(name, params, Utils.osGrab(), Utils.osGrab() ? desc : null,
          defValue, FS, MANUAL);
  }

  public boolean set(String grabString) {
    if (!Utils.osGrab()) return false;

    if (grabString.toLowerCase().startsWith("f"))
      return set(FS);
    else if (grabString.toLowerCase().startsWith("a"))
      return set(ALWAYS);
    else if (grabString.toLowerCase().startsWith("m"))
      return set(MANUAL);
    else
      throw new ErrorException(getName() + " parameter is incorrect");
  }

  public boolean setDefault(String grabString) {
    if (grabString.toLowerCase().startsWith("f"))
      return super.setDefault(FS);
    else if (grabString.toLowerCase().startsWith("a"))
      return super.setDefault(ALWAYS);
    else if (grabString.toLowerCase().startsWith("m"))
      return super.setDefault(MANUAL);
    else
      return false;
  }

  private static String getStr(int value) {
    if (value == FS)
      return "FS";
    else if (value == ALWAYS)
      return "Always";
    else if (value == MANUAL)
      return "Manual";
    else
      return null;
  }

  public synchronized String getDefaultStr() { return getStr(defValue); }
  public synchronized String getStr() { return getStr(value); };
  public String getValues() { return "Always, FS, Manual"; }
};
