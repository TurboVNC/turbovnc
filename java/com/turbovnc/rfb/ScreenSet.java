/* Copyright 2009 Pierre Ossman for Cendio AB
 * Copyright 2011 Brian P. Hinz
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

  public final int numScreens() { return screens.size(); }

  public final void addScreen(Screen screen) { screens.add(screen); }

  public final void removeScreen(int id) {
    for (Iterator<Screen> iter = screens.iterator(); iter.hasNext();) {
      Screen refScreen = (Screen)iter.next();
      if (refScreen.id == id)
        iter.remove();
    }
  }

  public final boolean validate(int fbWidth, int fbHeight) {
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
      //if (seenIds.lastIndexOf(refScreen.id) != seenIds.get(-1))
      //  return false;
      seenIds.add(refScreen.id);
    }

    return true;
  }

  public final void debugPrint() {
    for (Iterator<Screen> iter = screens.iterator(); iter.hasNext();) {
      Screen refScreen = (Screen)iter.next();
      vlog.error("    " + refScreen.id + " (0x" + refScreen.id + "): " +
                 refScreen.dimensions.width() + "x" +
                 refScreen.dimensions.height() + "+" +
                 refScreen.dimensions.tl.x + "+" +
                 refScreen.dimensions.tl.y + " (flags 0x" + refScreen.flags +
                 ")");
    }
  }

  // FIXME: List order shouldn't matter
  //inline bool operator(const ScreenSet& r) const { return screens == r.screens; }
  //inline bool operator(const ScreenSet& r) const { return screens != r.screens; }

  public ArrayList<Screen> screens;

  static LogWriter vlog = new LogWriter("ScreenSet");

}

