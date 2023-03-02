/* Copyright (C) 2017-2018, 2022-2023 D. R. Commander.  All Rights Reserved.
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

import com.turbovnc.rdr.ErrorException;

public final class HeaderParameter extends VoidParameter {

  public HeaderParameter(String name, Params params, String desc) {
    super(name, params, false, desc);
  }

  public boolean set(String str) {
    throw new ErrorException("Cannot set header parameter");
  }

  public boolean setDefault(String str) {
    throw new ErrorException("Cannot set default value for header parameter");
  }

  public void reset() {
    throw new ErrorException("Cannot reset header parameter");
  }

  public String getDefaultStr() {
    throw new ErrorException("Cannot get default string for header parameter");
  }

  public String getStr() {
    throw new ErrorException("Cannot get string for header parameter");
  }

  public String getValues() {
    throw new ErrorException("Cannot get values for header parameter");
  }
}
