/* Copyright (C) 2012, 2022-2023 D. R. Commander.  All Rights Reserved.
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

public final class SpanParameter extends IntParameter {

  public static final int PRIMARY = 0;
  public static final int ALL = 1;
  public static final int AUTO = 2;

  public SpanParameter(String name, Params params, boolean isGUI, String desc,
                       int defValue) {
    super(name, params, isGUI, desc, defValue, PRIMARY, AUTO);
  }

  public synchronized boolean set(String spanString) {
    if (spanString.toLowerCase().startsWith("p"))
      return set(PRIMARY);
    else if (spanString.toLowerCase().startsWith("al"))
      return set(ALL);
    else if (spanString.toLowerCase().startsWith("au"))
      return set(AUTO);
    else
      throw new ErrorException(getName() + " parameter is incorrect");
  }

  public boolean setDefault(String spanString) {
    if (spanString.toLowerCase().startsWith("p"))
      return super.setDefault(PRIMARY);
    else if (spanString.toLowerCase().startsWith("al"))
      return super.setDefault(ALL);
    else if (spanString.toLowerCase().startsWith("au"))
      return super.setDefault(AUTO);
    else
      return false;
  }

  private String getStr(int value) {
    if (value == PRIMARY)
      return "Primary";
    else if (value == ALL)
      return "All";
    else if (value == AUTO)
      return "Auto";
    else
      return null;
  }

  public synchronized String getDefaultStr() { return getStr(defValue); }
  public synchronized String getStr() { return getStr(value); }

  public String getValues() { return "Primary, All, Auto"; }
};
