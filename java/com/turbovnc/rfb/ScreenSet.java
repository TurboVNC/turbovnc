/* Copyright (C) 2017-2018, 2021 D. R. Commander.  All Rights Reserved.
 * Copyright 2011 Brian P. Hinz
 * Copyright 2009 Pierre Ossman for Cendio AB
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

// Management class for the RFB virtual screens

package com.turbovnc.rfb;

import java.util.*;

public class ScreenSet {

  // Represents a complete screen configuration, excluding framebuffer
  // dimensions.

  public ScreenSet() {
    screens = new ArrayList<Screen>();
  }

  // Deep copy
  public ScreenSet(ScreenSet old) {
    screens = new ArrayList<Screen>();
    for (Screen screen : old.screens)
      screens.add(new Screen(screen));
  }

  public final boolean equals(ScreenSet ref) {
    if (this == ref)
      return true;
    if (numScreens() != ref.numScreens())
      return false;

    Iterator<Screen> iter = screens.iterator();
    Iterator<Screen> riter = ref.screens.iterator();
    while (iter.hasNext() && riter.hasNext()) {
      Screen screen = (Screen)iter.next();
      Screen rscreen = (Screen)riter.next();

      if (!screen.equals(rscreen))
        return false;
    }

    return true;
  }

  public final boolean equalsIgnoreID(ScreenSet ref) {
    if (this == ref)
      return true;
    if (numScreens() != ref.numScreens())
      return false;

    Iterator<Screen> iter = screens.iterator();
    Iterator<Screen> riter = ref.screens.iterator();
    while (iter.hasNext() && riter.hasNext()) {
      Screen screen = (Screen)iter.next();
      Screen rscreen = (Screen)riter.next();

      if (!screen.equalsIgnoreID(rscreen))
        return false;
    }

    return true;
  }

  public final int numScreens() { return screens.size(); }

  public final void addScreen(Screen screen) {
    if (!screens.isEmpty()) {
      for (int i = 0; i < screens.size(); i++)
        if (screens.get(i).equals(screen))
          return;
    }
    screens.add(screen);
  }

  public final void addScreen0(Screen screen) {
    if (!screens.isEmpty()) {
      for (int i = 0; i < screens.size(); i++)
        if (screens.get(i).equals(screen))
          return;
    }
    screens.add(0, screen);
  }

  public final void removeScreen(int id) {
    for (Iterator<Screen> iter = screens.iterator(); iter.hasNext();) {
      Screen refScreen = (Screen)iter.next();
      if (refScreen.id == id)
        iter.remove();
    }
  }

  public final boolean validate(int fbWidth, int fbHeight,
                                boolean checkIds) {
    List<Integer> seenIds = new ArrayList<Integer>();
    Rect fbRect = new Rect();

    if (screens.isEmpty())
      return false;
    if (numScreens() > 255)
      return false;

    fbRect.setXYWH(0, 0, fbWidth, fbHeight);

    for (Iterator<Screen> iter = screens.iterator(); iter.hasNext();) {
      Screen refScreen = (Screen)iter.next();
      if (refScreen.dimensions.isEmpty())
        return false;
      if (!refScreen.dimensions.enclosedBy(fbRect))
        return false;
      if (checkIds) {
        for (Iterator<Integer> i = seenIds.iterator(); i.hasNext();) {
          if (refScreen.id == i.next())
            return false;
        }
        seenIds.add(refScreen.id);
      }
    }

    return true;
  }

  public final void assignIDs(ScreenSet ref) {
    for (int i = 0; i < numScreens(); i++) {
      if (ref != null && ref.numScreens() > i) {
        screens.get(i).id = ref.screens.get(i).id;
        screens.get(i).flags = ref.screens.get(i).flags;
      } else
        screens.get(i).generateID(ref);
    }
  }

  public final void debugPrint(String msg) {
    vlog.debug(msg + ":");
    for (Iterator<Screen> iter = screens.iterator(); iter.hasNext();) {
      Screen refScreen = (Screen)iter.next();
      vlog.debug("    " + refScreen.id + " (0x" +
                 Integer.toHexString(refScreen.id) + "): " +
                 refScreen.dimensions.width() + "x" +
                 refScreen.dimensions.height() + "+" +
                 refScreen.dimensions.tl.x + "+" +
                 refScreen.dimensions.tl.y + " (flags 0x" +
                 Integer.toHexString(refScreen.flags) + ")");
    }
  }

  @SuppressWarnings("checkstyle:VisibilityModifier")
  public ArrayList<Screen> screens;

  static LogWriter vlog = new LogWriter("ScreenSet");
}
