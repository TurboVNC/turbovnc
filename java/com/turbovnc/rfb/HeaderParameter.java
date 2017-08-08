/* Copyright (C) 2017 D. R. Commander.  All Rights Reserved.
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

public class HeaderParameter extends VoidParameter {

  public HeaderParameter(String name_, String desc_) {
    super(name_, desc_);
  }

  public final boolean setParam(String v) {
    throw new ErrorException("Cannot set header parameter");
  }

  public final String getDefaultStr() {
    throw new ErrorException("Cannot get default string for header parameter");
  }

  public final String getValueStr() {
    throw new ErrorException("Cannot get value string for header parameter");
  }

  public final String getValues() {
    throw new ErrorException("Cannot get values for header parameter");
  }
}
