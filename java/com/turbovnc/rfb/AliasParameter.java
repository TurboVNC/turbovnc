/* Copyright (C) 2012, 2018, 2022-2025 D. R. Commander.  All Rights Reserved.
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

public final class AliasParameter extends VoidParameter {

  public AliasParameter(String name, Params params, String desc,
                        VoidParameter param_) {
    super(name, params, false, param_.isAdvanced(), desc);
    param = param_;
  }

  public boolean set(String str) {
    boolean retval = param.set(str);
    param.setCommandLine(isCommandLine());
    return retval;
  }

  public void reset() { param.reset(); }

  public boolean setDefault(String str) { return param.setDefault(str); }

  public String getDefaultStr() { return null; }
  public String getStr() { return null; }
  public String getValues() { return null; }
  public boolean isBool() { return param.isBool(); }

  private final VoidParameter param;
}
