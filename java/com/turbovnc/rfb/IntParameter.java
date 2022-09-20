/* Copyright (C) 2012, 2017-2018, 2022 D. R. Commander.  All Rights Reserved.
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

public class IntParameter extends VoidParameter {

  public IntParameter(String name, Params params, boolean isGUI, String desc,
                      int defValue_) {
    super(name, params, isGUI, desc);
    value = defValue = defValue_;
    minValue = Integer.MIN_VALUE;
    maxValue = Integer.MAX_VALUE;
    useMinMax = false;
  }

  public IntParameter(String name, Params params, boolean isGUI, String desc,
                      int defValue_, int minValue_, int maxValue_) {
    super(name, params, isGUI, desc);
    value = defValue = defValue_;
    maxValue = maxValue_;
    minValue = minValue_;
    useMinMax = true;
  }

  public boolean set(String str) {
    int i;
    try {
      i = Integer.parseInt(str);
    } catch (NumberFormatException e) {
      return false;
    }
    return set(i);
  }

  public final synchronized boolean set(int value_) {
    if (useMinMax && (value_ < minValue || value_ > maxValue))
      return false;
    value = value_;
    setCommandLine(false);
    return true;
  }

  public final synchronized void reset() { set(defValue); }

  public synchronized int get() { return value; }

  public String getDefaultStr() {
    if (defValue >= 0)
      return Integer.toString(defValue);
    return null;
  }

  public synchronized String getStr() { return Integer.toString(value); }

  public String getValues() {
    if (useMinMax) {
      return minValue + "-" + maxValue;
    }
    return null;
  }

  int value;
  final int defValue, minValue, maxValue;
  private final boolean useMinMax;
}
