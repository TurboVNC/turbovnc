/* Copyright (C) 2012, 2018, 2020, 2022 D. R. Commander.  All Rights Reserved.
 * Copyright 2012 Brian P. Hinz
 * Copyright 2011 Martin Koegler <mkoegler@auto.tuwien.ac.at>
 * Copyright 2011 Pierre Ossman <ossman@cendio.se> for Cendio AB
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

import java.awt.event.KeyEvent;

public final class MenuKey {

  public static class Symbol {
    Symbol(String name_, int vKeyCode_, int keysym_, int rfbKeyCode_) {
      name = name_;
      vKeyCode = vKeyCode_;
      keysym = keysym_;
      rfbKeyCode = rfbKeyCode_;
    }
    // CHECKSTYLE VisibilityModifier:OFF
    public String name;
    public int vKeyCode;
    public int keysym;
    public int rfbKeyCode;
    // CHECKSTYLE VisibilityModifier:ON
  }

  private static final Symbol[] SYMBOLS = {
    new Symbol("F1", KeyEvent.VK_F1, Keysyms.F1, 0x3b),
    new Symbol("F2", KeyEvent.VK_F2, Keysyms.F2, 0x3c),
    new Symbol("F3", KeyEvent.VK_F3, Keysyms.F3, 0x3d),
    new Symbol("F4", KeyEvent.VK_F4, Keysyms.F4, 0x3e),
    new Symbol("F5", KeyEvent.VK_F5, Keysyms.F5, 0x3f),
    new Symbol("F6", KeyEvent.VK_F6, Keysyms.F6, 0x40),
    new Symbol("F7", KeyEvent.VK_F7, Keysyms.F7, 0x41),
    new Symbol("F8", KeyEvent.VK_F8, Keysyms.F8, 0x42),
    new Symbol("F9", KeyEvent.VK_F9, Keysyms.F9, 0x43),
    new Symbol("F10", KeyEvent.VK_F10, Keysyms.F10, 0x44),
    new Symbol("F11", KeyEvent.VK_F11, Keysyms.F11, 0x57),
    new Symbol("F12", KeyEvent.VK_F12, Keysyms.F12, 0x58),
    new Symbol("Pause", KeyEvent.VK_PAUSE, Keysyms.PAUSE, 0xc6),
    new Symbol("Print", KeyEvent.VK_PRINTSCREEN, Keysyms.PRINT, 0xb9),
    new Symbol("ScrollLock", KeyEvent.VK_SCROLL_LOCK, Keysyms.SCROLL_LOCK,
               0x46),
    new Symbol("Escape", KeyEvent.VK_ESCAPE, Keysyms.ESCAPE, 0x1),
    new Symbol("Insert", KeyEvent.VK_INSERT, Keysyms.INSERT, 0xd2),
    new Symbol("Delete", KeyEvent.VK_DELETE, Keysyms.DELETE, 0xd3),
    new Symbol("Home", KeyEvent.VK_HOME, Keysyms.HOME, 0xc7),
    new Symbol("PageUp", KeyEvent.VK_PAGE_UP, Keysyms.PRIOR, 0xc9),
    new Symbol("PageDown", KeyEvent.VK_PAGE_DOWN, Keysyms.NEXT, 0xd1)
  };

  public static int getSymbolCount() {
    return SYMBOLS.length;
  }

  public static Symbol[] getSymbols() {
    return SYMBOLS;
  }

  public static String getValueStr() {
    String s = "";
    for (int i = 0; i < getSymbolCount(); i++) {
      s += SYMBOLS[i].name;
      if (i < getSymbolCount() - 1)
        s += ", ";
    }
    return s;
  }

  public static Symbol getSymbol(String menuKeyStr) {
    for (int i = 0; i < getSymbolCount(); i++)
      if (SYMBOLS[i].name.equalsIgnoreCase(menuKeyStr))
        return SYMBOLS[i];

    return null;
  }

  private MenuKey() {}
}
