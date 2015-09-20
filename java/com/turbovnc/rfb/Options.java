/* Copyright (C) 2012-2013, 2015 D. R. Commander.  All Rights Reserved.
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
    grabKeyboard = old.grabKeyboard;
    fullScreen = old.fullScreen;
    span = old.span;
    scalingFactor = old.scalingFactor;
    desktopSize = new DesktopSize(old.desktopSize.mode, old.desktopSize.width,
                                  old.desktopSize.height);
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
      } catch (NumberFormatException e) {};
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
      String array[] = sizeString.replaceAll("[^\\dx]", "").split("x");
      if (array.length <= 1)
        return null;
      int width = Integer.parseInt(array[0]);
      int height = Integer.parseInt(array[1]);
      if (width < 1 || height < 1)
        return null;
      return new DesktopSize(SIZE_MANUAL, width, height);
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
    printOpt("grabKeyboard", grabKeyboard);
    printOpt("fullScreen", fullScreen);
    printOpt("span", span);
    printOpt("scalingFactor", scalingFactor);
    if (desktopSize.mode == SIZE_MANUAL)
      printOpt("desktopSize", desktopSize.width + "x" + desktopSize.height);
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
    public DesktopSize() {};

    public DesktopSize(int mode, int width, int height) {
      this.mode = mode;
      this.width = width;
      this.height = height;
    }

    public boolean isEqual(DesktopSize size) {
      return size.mode == mode && size.width == width && size.height == height;
    }

    public int mode;
    public int width;
    public int height;
  }

  public String serverName;
  public int port;
  public boolean shared;
  public boolean viewOnly;
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
