/* Copyright (C) 2012, 2017-2018, 2021-2023 D. R. Commander.
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

public class StringParameter extends VoidParameter {

  public StringParameter(String name, Params params, boolean isGUI,
                         String desc, String defValue_) {
    this(name, params, isGUI, desc, defValue_, null);
  }

  public StringParameter(String name, Params params, boolean isGUI,
                         String desc, String defValue_, String values_) {
    super(name, params, isGUI, desc);
    value = defValue = defValue_;
    values = values_;
  }

  public synchronized boolean set(String str) {
    if (str != null && str.isEmpty())
      str = null;
    value = str;
    isDefault = false;
    setCommandLine(false);
    return value != null;
  }

  public final synchronized void reset() {
    value = defValue;
    isDefault = true;
    setCommandLine(false);
  }

  public synchronized boolean setDefault(String str) {
    if (str != null && str.isEmpty())
      str = null;
    value = defValue = str;
    isDefault = true;
    return true;
  }

  public final synchronized String get() { return value; }
  public final synchronized String getDefaultStr() { return defValue; }
  public final synchronized String getStr() { return value; }
  public final String getValues() { return values; }
  public final synchronized boolean isDefault() { return isDefault; }

  private boolean isDefault = true;

  String value;
  private String defValue;
  private final String values;
}
