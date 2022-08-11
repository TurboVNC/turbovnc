/* Copyright (C) 2022 D. R. Commander.  All Rights Reserved.
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

public final class MenuKeyParameter extends VoidParameter {

  public MenuKeyParameter(String name, Params params, boolean isGUI,
                          String desc, String defValue_) {
    super(name, params, isGUI, desc);
    defValue = defValue_;
    set(defValue);
  }

  public synchronized boolean set(String menuKeyString) {
    code = MenuKey.getMenuKeyCode(menuKeyString);
    sym = MenuKey.getMenuKeySym(menuKeyString);
    if (code < 0 || sym < 0)
      throw new ErrorException(getName() + " parameter is incorrect");
    setCommandLine(false);
    return true;
  }

  public void reset() { set(defValue); }

  public int getCode() { return code; }
  public int getSym() { return sym; }

  public String getDefaultStr() { return defValue; }

  public synchronized String getStr() {
    for (MenuKey.MenuKeySymbol mks : MenuKey.getMenuKeySymbols()) {
      if (mks.keycode == code && mks.keysym == sym)
        return mks.name;
    }
    return null;
  }

  public String getValues() { return MenuKey.getMenuKeyValueStr(); }

  private final String defValue;
  private int code, sym;
};
