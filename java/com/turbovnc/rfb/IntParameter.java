/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright (C) 2012 D. R. Commander.  All Rights Reserved.
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
    this(name_, desc_, v, 0, -1);
  }

  public IntParameter(String name_, String desc_, int v, int minValue_,
    int maxValue_) {
    super(name_, desc_);
    value = v;
    defValue = v;
    maxValue = maxValue_;
    minValue = minValue_;
  }

  public boolean setParam(String v) {
    try {
      value = Integer.parseInt(v);
    } catch (NumberFormatException e) {
      return false;
    }
    return true;
  }

  public String getDefaultStr() {
    if (defValue >= 0)
      return Integer.toString(defValue);
    return null;
  }
  public String getValueStr() { return Integer.toString(value); }
  public String getValues() {
    if (maxValue >= minValue) {
      return minValue + "-" + maxValue;
    }
    return null;
  }

  public int getValue() { return value; }

  protected int value;
  protected int defValue;
  protected int minValue;
  protected int maxValue;
}
