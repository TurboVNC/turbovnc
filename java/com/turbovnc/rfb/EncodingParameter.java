/* Copyright (C) 2022-2023 D. R. Commander.  All Rights Reserved.
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

import com.turbovnc.rdr.*;

public final class EncodingParameter extends IntParameter {

  public EncodingParameter(String name, Params params, boolean isGUI,
                           String desc, int defValue) {
    super(name, params, isGUI, desc, defValue, 0, RFB.ENCODING_LAST);
  }

  public boolean set(String encString) {
    int encNum = RFB.encodingNum(encString);
    if (encNum == -1)
      throw new ErrorException(getName() + " parameter is incorrect");
    return set(encNum);
  }

  public boolean setDefault(String encString) {
    int encNum = RFB.encodingNum(encString);
    if (encNum == -1)
      return false;
    return super.setDefault(encNum);
  }

  public synchronized String getDefaultStr() {
    return RFB.encodingName(defValue);
  }

  public synchronized String getStr() {
    return RFB.encodingName(value);
  }

  public String getValues() { return "Tight, ZRLE, Hextile, Raw"; }
}
