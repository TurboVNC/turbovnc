/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright (C) 2012, 2017 D. R. Commander.  All Rights Reserved.
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

public abstract class VoidParameter {
  public VoidParameter(String name_, String desc_) {
    name = name_;
    description = desc_;
    if (Configuration.head == null)
      Configuration.head = this;
    if (Configuration.tail != null)
      Configuration.tail.next = this;
    Configuration.tail = this;
  }

  public final String getName() { return name; }
  public final String getDescription() { return description; }

  public abstract boolean setParam(String value);
  public boolean setParam() { return false; }
  public abstract String getDefaultStr();
  public abstract String getValueStr();
  public abstract String getValues();
  public boolean isBool() { return false; }

  VoidParameter next;
  protected final String name;
  protected final String description;
}
