/* Copyright (C) 2012-2013, 2015, 2017 D. R. Commander.  All Rights Reserved.
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

public class Options {

  public static final int SCALE_AUTO = -1;
  public static final int SCALE_FIXEDRATIO = -2;

  public static final int NUMSPANOPT = 3;
  public static final int SPAN_PRIMARY = 0;
  public static final int SPAN_ALL = 1;
  public static final int SPAN_AUTO = 2;

  public static final int NUMSIZEOPT = 3;
  public static final int SIZE_SERVER = 0;
  public static final int SIZE_MANUAL = 1;
  public static final int SIZE_AUTO = 2;

  public static final int NUMGRABOPT = 3;
  public static final int GRAB_FS = 0;
  public static final int GRAB_ALWAYS = 1;
  public static final int GRAB_MANUAL = 2;

  public static final int NUMSUBSAMPOPT = 4;
  public static final int SUBSAMP_NONE = 0;
  public static final int SUBSAMP_4X = 1;
  public static final int SUBSAMP_2X = 2;
  public static final int SUBSAMP_GRAY = 3;

  public static final int DEFQUAL = 95;

  public Options() {}

  public Options(Options old) {
    if (old.serverName != null)
      serverName = new String(old.serverName);
    port = old.port;
    shared = old.shared;
    viewOnly = old.viewOnly;
    reverseScroll = old.reverseScroll;
    grabKeyboard = old.grabKeyboard;
    fullScreen = old.fullScreen;
    span = old.span;
    scalingFactor = old.scalingFactor;
    desktopSize = new DesktopSize(old.desktopSize);
    acceptClipboard = old.acceptClipboard;
    sendClipboard = old.sendClipboard;
    acceptBell = old.acceptBell;
    preferredEncoding = old.preferredEncoding;
    allowJpeg = old.allowJpeg;
    quality = old.quality;
    subsampling = old.subsampling;
    compressLevel = old.compressLevel;
    colors = old.colors;
    cursorShape = old.cursorShape;
    continuousUpdates = old.continuousUpdates;
    if (old.user != null) user = new String(old.user);
    sendLocalUsername = old.sendLocalUsername;
    if (old.via != null) via = new String(old.via);
    tunnel = old.tunnel;
    extSSH = old.extSSH;
    if (old.sshUser != null) sshUser = new String(old.sshUser);
  }

  public static int parseScalingFactor(String scaleString) {
    if (scaleString.toLowerCase().startsWith("a"))
      return SCALE_AUTO;
    else if (scaleString.toLowerCase().startsWith("f"))
      return SCALE_FIXEDRATIO;
    else {
      scaleString = scaleString.replaceAll("[^\\d]", "");
      int sf = -1;
      try {
        sf = Integer.parseInt(scaleString);
      } catch (NumberFormatException e) {}
      if (sf >= 1 && sf <= 1000) {
        return sf;
      }
    }
    return 0;
  }

  public void setScalingFactor(String scaleString) {
    int sf = parseScalingFactor(scaleString);
    if (sf != 0)
      scalingFactor = sf;
  }

  public static DesktopSize parseDesktopSize(String sizeString) {
    if (sizeString.toLowerCase().startsWith("a"))
      return new DesktopSize(SIZE_AUTO, 0, 0);
    else if (sizeString.toLowerCase().startsWith("s") ||
             sizeString.equals("0"))
      return new DesktopSize(SIZE_SERVER, 0, 0);
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

      return new DesktopSize(SIZE_MANUAL, fbWidth, fbHeight, layout);
    }
  }

  public void setDesktopSize(String sizeString) {
    DesktopSize size = parseDesktopSize(sizeString);
    if (size != null)
      desktopSize = size;
  }

  public int getSubsamplingOrdinal() {
    switch (subsampling) {
      case SUBSAMP_2X:
        return 1;
      case SUBSAMP_4X:
        return 2;
      case SUBSAMP_GRAY:
        return 3;
    }
    return 0;
  }

  void printOpt(String name, boolean val) {
    System.out.println(name + " = " + (val ? "true" : "false"));
  }

  void printOpt(String name, int val) {
    System.out.println(name + " = " + val);
  }

  void printOpt(String name, String val) {
    System.out.println(name + " = " + val);
  }

  public void print() {
    printOpt("serverName", serverName);
    printOpt("port", port);
    printOpt("shared", shared);
    printOpt("viewOnly", viewOnly);
    printOpt("reverseScroll", reverseScroll);
    printOpt("grabKeyboard", grabKeyboard);
    printOpt("fullScreen", fullScreen);
    printOpt("span", span);
    printOpt("scalingFactor", scalingFactor);
    if (desktopSize.mode == SIZE_MANUAL)
      printOpt("desktopSize", desktopSize.getString());
    else
      printOpt("desktopSize", desktopSize.mode);
    printOpt("acceptClipboard", acceptClipboard);
    printOpt("sendClipboard", sendClipboard);
    printOpt("acceptBell", acceptBell);
    printOpt("preferredEncoding", preferredEncoding);
    printOpt("copyRect", copyRect);
    printOpt("allowJpeg", allowJpeg);
    printOpt("quality", quality);
    printOpt("subsampling", subsampling);
    printOpt("compressLevel", compressLevel);
    printOpt("colors", colors);
    printOpt("cursorShape", cursorShape);
    printOpt("continuousUpdates", continuousUpdates);
    printOpt("user", user);
    printOpt("sendLocalUsername", sendLocalUsername);
    printOpt("via", via);
    printOpt("tunnel", tunnel);
    printOpt("extSSH", extSSH);
    printOpt("sshUser", sshUser);
  }

  public static class DesktopSize {
    public DesktopSize() {}

    // Deep copy
    public DesktopSize(DesktopSize old) {
      this(old.mode, old.width, old.height, new ScreenSet(old.layout));
    }

    public DesktopSize(int mode_, int width_, int height_, ScreenSet layout_) {
      mode = mode_;
      width = width_;
      height = height_;
      layout = layout_;
    }

    public DesktopSize(int mode_, int width_, int height_) {
      this(mode_, width_, height_, new ScreenSet());
    }

    public boolean equals(DesktopSize size) {
      return size.mode == mode && size.width == width &&
             size.height == height && size.layout.equals(layout);
    }

    public String getString() {
      if (mode == Options.SIZE_AUTO)
        return "Auto";
      else if (mode == Options.SIZE_SERVER)
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

    // CHECKSTYLE VisibilityModifier:OFF
    public int mode;
    public int width;
    public int height;
    public ScreenSet layout;
  }

  public String serverName;
  public int port;
  public boolean shared;
  public boolean viewOnly;
  public boolean reverseScroll;
  public int grabKeyboard;
  public boolean fullScreen;
  public int span;
  public int scalingFactor;
  public DesktopSize desktopSize = new DesktopSize();
  public boolean acceptClipboard;
  public boolean sendClipboard;
  public boolean acceptBell;
  public int preferredEncoding;
  public boolean copyRect = true;
  public boolean allowJpeg;
  public int quality;
  public int subsampling;
  public int compressLevel;
  public int colors;
  public boolean cursorShape;
  public boolean continuousUpdates;
  public String user;
  public boolean sendLocalUsername;
  public String via;
  public boolean tunnel;
  public boolean extSSH;
  public String sshUser;
}
