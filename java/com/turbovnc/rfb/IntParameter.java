/* Copyright (C) 2012, 2017-2018, 2023 D. R. Commander.  All Rights Reserved.
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

  public IntParameter(String name_, String desc_, int v) {
    super(name_, desc_);
    value = v;
    defValue = v;
    minValue = Integer.MIN_VALUE;
    maxValue = Integer.MAX_VALUE;
    useMin = useMax = false;
  }

  public IntParameter(String name_, String desc_, int v, int minValue_) {
    super(name_, desc_);
    value = v;
    defValue = v;
    minValue = minValue_;
    maxValue = Integer.MAX_VALUE;
    useMin = true;
    useMax = false;
  }

  public IntParameter(String name_, String desc_, int v, int minValue_,
                      int maxValue_) {
    super(name_, desc_);
    value = v;
    defValue = v;
    minValue = minValue_;
    maxValue = maxValue_;
    useMin = useMax = true;
  }

  public boolean setParam(String v) {
    int i;
    try {
      i = Integer.parseInt(v);
    } catch (NumberFormatException e) {
      return false;
    }
    return setValue(i);
  }

  public synchronized boolean setValue(int v) {
    if ((useMin && v < minValue) || (useMax && v > maxValue))
      return false;
    value = v;
    return true;
  }

  public synchronized void reset() { value = defValue; }

  public String getDefaultStr() {
    if (defValue >= 0)
      return Integer.toString(defValue);
    return null;
  }
  public String getValues() {
    if (useMin || useMax) {
      return (useMin ? minValue : "") + "-" + (useMax ? maxValue : "");
    }
    return null;
  }

  public synchronized int getValue() { return value; }

  protected int value;
  protected final int defValue;
  protected final int minValue;
  protected final int maxValue;
  final boolean useMin, useMax;
}
