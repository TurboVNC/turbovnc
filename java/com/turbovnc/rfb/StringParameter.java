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

public class StringParameter extends VoidParameter {

  public StringParameter(String name_, String desc_, String v) {
    this(name_, desc_, v, null);
  }

  public StringParameter(String name_, String desc_, String v, String values_) {
    super(name_, desc_);
    value = v;
    defValue = v;
    values = values_;
  }

  public boolean setParam(String v) {
    value = v;
    isDefault = false;
    return value != null;
  }

  public boolean setDefaultStr(String v) {
    value = defValue = v;
    return defValue != null;
  }

  public String getDefaultStr() { return defValue; }
  public String getValueStr() { return value; }
  public String getValues() { return values; }

  public String getValue() { return value; }
  public String getData() { return value; }

  public boolean isDefault = true;

  protected String value;
  protected String defValue;
  protected String values;
}
