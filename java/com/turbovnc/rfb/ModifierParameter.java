/* Copyright (C) 2025 D. R. Commander.  All Rights Reserved.
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

import java.awt.event.*;

import com.turbovnc.rdr.*;

public final class ModifierParameter extends IntParameter {

  public ModifierParameter(String name, Params params, String desc,
                           int defValue) {
    super(name, params, true, true, desc, defValue);
  }

  public synchronized boolean set(String modifierString) {
    int modifierMask = getMaskFromString(modifierString);

    if (modifierMask == 0 || modifierMask == InputEvent.SHIFT_DOWN_MASK)
      throw new ErrorException(getName() + " parameter is incorrect");

    return set(modifierMask);
  }

  public synchronized boolean setDefault(String modifierString) {
    int modifierMask = getMaskFromString(modifierString);

    if (modifierMask == 0 || modifierMask == InputEvent.SHIFT_DOWN_MASK)
      return false;

    return super.setDefault(modifierMask);
  }

  public synchronized String getDefaultStr() {
    return getStringFromMask(defValue);
  }
  public synchronized String getStr() { return getStringFromMask(value); }

  public String getValues() {
    return "Any case-insensitive combination of " +
           "Cmd/Command/Meta/Super/Win/Windows, Ctrl/Control, " +
           "Alt/Opt/Option, or Shift separated by dashes";
  }

  private static int getMaskFromString(String modifierString) {
    if (modifierString == null || modifierString.length() == 0)
      return 0;

    int modifierMask = 0;

    String[] tokens = modifierString.replaceAll("\\s", "").split("-");
    for (String str : tokens) {
      if (str.equalsIgnoreCase("Cmd") || str.equalsIgnoreCase("Command") ||
          str.equalsIgnoreCase("Meta") || str.equalsIgnoreCase("Super") ||
          str.equalsIgnoreCase("Win") || str.equalsIgnoreCase("Windows"))
        modifierMask |= InputEvent.META_DOWN_MASK;
      else if (str.equalsIgnoreCase("Ctrl") || str.equalsIgnoreCase("Control"))
        modifierMask |= InputEvent.CTRL_DOWN_MASK;
      else if (str.equalsIgnoreCase("Alt") || str.equalsIgnoreCase("Opt") ||
               str.equalsIgnoreCase("Option"))
        modifierMask |= InputEvent.ALT_DOWN_MASK;
      else if (str.equalsIgnoreCase("Shift"))
        modifierMask |= InputEvent.SHIFT_DOWN_MASK;
    }

    return modifierMask;
  }

  private static String getStringFromMask(int modifierMask) {
    StringBuffer sb = new StringBuffer();

    if (modifierMask == 0)
      return null;

    if ((modifierMask & InputEvent.META_DOWN_MASK) != 0) {
      if (Utils.isWindows())
        sb.append("Windows");
      else if (Utils.isMac())
        sb.append("Command");
      else
        sb.append("Super");
      modifierMask &= ~InputEvent.META_DOWN_MASK;
      if (modifierMask != 0)
        sb.append("-");
    }

    if ((modifierMask & InputEvent.CTRL_DOWN_MASK) != 0) {
      sb.append("Control");
      modifierMask &= ~InputEvent.CTRL_DOWN_MASK;
      if (modifierMask != 0)
        sb.append("-");
    }

    if ((modifierMask & InputEvent.ALT_DOWN_MASK) != 0) {
      if (Utils.isMac())
        sb.append("Option");
      else
        sb.append("Alt");
      modifierMask &= ~InputEvent.ALT_DOWN_MASK;
      if (modifierMask != 0)
        sb.append("-");
    }

    if ((modifierMask & InputEvent.SHIFT_DOWN_MASK) != 0) {
      sb.append("Shift");
      modifierMask &= ~InputEvent.SHIFT_DOWN_MASK;
    }

    if (modifierMask != 0)
      return null;

    return sb.toString();
  }
};
