/* Copyright (C) 2012, 2017-2018, 2020-2023 D. R. Commander.
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

public abstract class VoidParameter {

  public VoidParameter(String name_, Params params_, boolean isGUI_,
                       String desc_) {
    name = name_;
    params = params_;
    desc = desc_;
    isGUI = isGUI_;
    if (params.head == null)
      params.head = this;
    if (params.tail != null)
      params.tail.next = this;
    params.tail = this;
  }

  public final String getName() { return name; }
  public final String getDescription() { return desc; }

  public abstract boolean set(String str);

  public final boolean set(String str, boolean commandLine_) {
    setCommandLine(commandLine_);
    boolean retval = set(str);
    setCommandLine(commandLine_);
    return retval;
  }

  public final void setCommandLine(boolean commandLine_) {
    commandLine = commandLine_;
  }

  public abstract void reset();

  public abstract boolean setDefault(String str);

  public abstract String getStr();
  public abstract String getDefaultStr();
  public abstract String getValues();
  public boolean isBool() { return false; }
  public boolean isCommandLine() { return commandLine; }
  public final boolean isGUI() { return isGUI; }
  public final VoidParameter next() { return next; }

  private VoidParameter next;
  private final String name, desc;
  final Params params;
  private final boolean isGUI;
  private boolean commandLine;
}
