/* Copyright (C) 2015, 2017, 2018, 2021-2022 D. R. Commander.
 *                                           All Rights Reserved.
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

public class DesktopSize {

  public static final int SERVER = 0;
  public static final int MANUAL = 1;
  public static final int AUTO = 2;

  public DesktopSize(int mode_, int width_, int height_, ScreenSet layout_) {
    mode = mode_;
    width = width_;
    height = height_;
    layout = layout_;
  }

  public DesktopSize(int mode_, int width_, int height_) {
    this(mode_, width_, height_, new ScreenSet());
  }

  public static DesktopSize parse(String sizeString) {
    if (sizeString.toLowerCase().startsWith("a"))
      return new DesktopSize(AUTO, 0, 0);
    else if (sizeString.toLowerCase().startsWith("s") ||
             sizeString.equals("0"))
      return new DesktopSize(SERVER, 0, 0);
    else {
      ScreenSet layout = new ScreenSet();
      String[] screenSpecs = sizeString.replaceAll("[^\\dx,+]", "").split(",");
      int fbWidth = 0, fbHeight = 0;
      int r = Integer.MIN_VALUE, b = Integer.MIN_VALUE;

      if (screenSpecs.length < 1)
        return null;

      for (int i = 0; i < screenSpecs.length; i++) {
        String[] array = screenSpecs[i].split("[x\\+]");

        if (array.length < 2)
          return null;

        int w = Integer.parseInt(array[0]);
        int h = Integer.parseInt(array[1]);
        if (w < 1 || h < 1)
          return null;

        int x = 0, y = 0;
        if (array.length > 2) x = Integer.parseInt(array[2]);
        if (array.length > 3) y = Integer.parseInt(array[3]);
        if (x < 0 || y < 0)
          return null;

        if (x >= 65535 || y >= 65535) continue;
        if (x + w > 65535) w = 65535 - x;
        if (y + h > 65535) h = 65535 - y;

        layout.addScreen(new Screen(0, x, y, w, h, 0));
        if (x + w > r) r = x + w;
        if (y + h > b) b = y + h;
      }

      fbWidth = r;
      fbHeight = b;

      if (!layout.validate(fbWidth, fbHeight, false))
        return null;

      return new DesktopSize(MANUAL, fbWidth, fbHeight, layout);
    }
  }

  public boolean equals(DesktopSize size) {
    return size.mode == mode && size.width == width &&
           size.height == height && size.layout.equals(layout);
  }

  public boolean equalsIgnoreID(DesktopSize size) {
    return size.mode == mode && size.width == width &&
           size.height == height && size.layout.equalsIgnoreID(layout);
  }

  public synchronized int getMode() { return mode; }

  public String getString() {
    if (mode == AUTO)
      return "Auto";
    else if (mode == SERVER)
      return "Server";
    else {
      if (layout.numScreens() < 2)
        return width + "x" + height;
      else {
        StringBuffer s = new StringBuffer();

        for (int i = 0; i < layout.numScreens(); i++) {
          Screen screen = layout.screens.get(i);

          s.append(screen.dimensions.width() + "x" +
                   screen.dimensions.height() + "+" +
                   screen.dimensions.tl.x + "+" + screen.dimensions.tl.y +
                   (i < layout.numScreens() - 1 ? "," : ""));
        }

        return s.toString();
      }
    }
  }

  int mode;
  int width;
  int height;
  ScreenSet layout;
}
