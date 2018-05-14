/* Copyright 2011 Martin Koegler <mkoegler@auto.tuwien.ac.at>
 * Copyright 2011 Pierre Ossman <ossman@cendio.se> for Cendio AB
 * Copyright 2012 Brian P. Hinz
 * Copyright (C) 2012, 2018 D. R. Commander.  All Rights Reserved.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 * USA.
 */

package com.turbovnc.vncviewer;

import java.awt.event.KeyEvent;

import com.turbovnc.rfb.*;

public final class MenuKey {

  static class MenuKeySymbol {
    MenuKeySymbol(String name_, int keycode_, int keysym_) {
      name = name_;
      keycode = keycode_;
      keysym = keysym_;
    }
    String name;
    int keycode;
    int keysym;
  }

  private static final MenuKeySymbol[] MENU_SYMBOLS = {
    new MenuKeySymbol("F1", KeyEvent.VK_F1, Keysyms.F1),
    new MenuKeySymbol("F2", KeyEvent.VK_F2, Keysyms.F2),
    new MenuKeySymbol("F3", KeyEvent.VK_F3, Keysyms.F3),
    new MenuKeySymbol("F4", KeyEvent.VK_F4, Keysyms.F4),
    new MenuKeySymbol("F5", KeyEvent.VK_F5, Keysyms.F5),
    new MenuKeySymbol("F6", KeyEvent.VK_F6, Keysyms.F6),
    new MenuKeySymbol("F7", KeyEvent.VK_F7, Keysyms.F7),
    new MenuKeySymbol("F8", KeyEvent.VK_F8, Keysyms.F8),
    new MenuKeySymbol("F9", KeyEvent.VK_F9, Keysyms.F9),
    new MenuKeySymbol("F10", KeyEvent.VK_F10, Keysyms.F10),
    new MenuKeySymbol("F11", KeyEvent.VK_F11, Keysyms.F11),
    new MenuKeySymbol("F12", KeyEvent.VK_F12, Keysyms.F12),
    new MenuKeySymbol("Pause", KeyEvent.VK_PAUSE, Keysyms.PAUSE),
    new MenuKeySymbol("Print", KeyEvent.VK_PRINTSCREEN, Keysyms.PRINT),
    new MenuKeySymbol("ScrollLock", KeyEvent.VK_SCROLL_LOCK,
                      Keysyms.SCROLL_LOCK),
    new MenuKeySymbol("Escape", KeyEvent.VK_ESCAPE, Keysyms.ESCAPE),
    new MenuKeySymbol("Insert", KeyEvent.VK_INSERT, Keysyms.INSERT),
    new MenuKeySymbol("Delete", KeyEvent.VK_DELETE, Keysyms.DELETE),
    new MenuKeySymbol("Home", KeyEvent.VK_HOME, Keysyms.HOME),
    new MenuKeySymbol("PageUp", KeyEvent.VK_PAGE_UP, Keysyms.PRIOR),
    new MenuKeySymbol("PageDown", KeyEvent.VK_PAGE_DOWN, Keysyms.NEXT)
  };

  static int getMenuKeySymbolCount() {
    return MENU_SYMBOLS.length;
  }

  public static MenuKeySymbol[] getMenuKeySymbols() {
    return MENU_SYMBOLS;
  }

  public static String getMenuKeyValueStr() {
    String s = "";
    for (int i = 0; i < getMenuKeySymbolCount(); i++) {
      s += MENU_SYMBOLS[i].name;
      if (i < getMenuKeySymbolCount() - 1)
        s += ", ";
    }
    return s;
  }

  static int getMenuKeyCode() {
    String menuKeyStr;
    int menuKeyCode = KeyEvent.VK_F8;

    menuKeyStr =
      Configuration.getParam("menuKey").getValueStr();
    for (int i = 0; i < getMenuKeySymbolCount(); i++)
      if (MENU_SYMBOLS[i].name.equals(menuKeyStr))
        menuKeyCode = MENU_SYMBOLS[i].keycode;

    return menuKeyCode;
  }

  static int getMenuKeySym() {
    String menuKeyStr;
    int menuKeySym = Keysyms.F8;

    menuKeyStr =
      Configuration.getParam("menuKey").getValueStr();
    for (int i = 0; i < getMenuKeySymbolCount(); i++)
      if (MENU_SYMBOLS[i].name.equals(menuKeyStr))
        menuKeySym = MENU_SYMBOLS[i].keysym;

    return menuKeySym;
  }

  private MenuKey() {}
}
