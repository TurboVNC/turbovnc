/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright (C) 2010 TigerVNC Team
 * Copyright (C) 2011 Brian P. Hinz
 * Copyright (C) 2012-2013, 2015-2018, 2020-2021 D. R. Commander.
 *                                               All Rights Reserved.
 * Copyright (C) 2021 Steffen Kieß
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

import java.util.*;

import com.turbovnc.rdr.*;

import com.jcraft.jsch.Session;

import com.turbovnc.network.Socket;

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
    port = old.port;
    recvClipboard = old.recvClipboard;
    sendClipboard = old.sendClipboard;
    if (old.serverName != null) serverName = new String(old.serverName);
    shared = old.shared;
    if (old.unixDomainPath != null)
      unixDomainPath = new String(old.unixDomainPath);

    fsAltEnter = old.fsAltEnter;
    grabKeyboard = old.grabKeyboard;
    menuKeyCode = old.menuKeyCode;
    menuKeySym = old.menuKeySym;
    reverseScroll = old.reverseScroll;
    viewOnly = old.viewOnly;

    acceptBell = old.acceptBell;
    cursorShape = old.cursorShape;
    desktopSize = new DesktopSize(old.desktopSize);
    fullScreen = old.fullScreen;
    scalingFactor = old.scalingFactor;
    span = old.span;
    showToolbar = old.showToolbar;

    compressLevel = old.compressLevel;
    preferredEncoding = old.preferredEncoding;
    allowJpeg = old.allowJpeg;
    quality = old.quality;
    subsampling = old.subsampling;

    enabledSecTypes = new ArrayList<Integer>(old.enabledSecTypes);
    sendLocalUsername = old.sendLocalUsername;
    stdioSocket = old.stdioSocket;
    sshSession = old.sshSession;
    sshTunnelActive = old.sshTunnelActive;
    if (old.sshUser != null) sshUser = new String(old.sshUser);
    tunnel = old.tunnel;
    if (old.user != null) user = new String(old.user);
    if (old.via != null) via = new String(old.via);
    if (old.x509ca != null) x509ca = new String(old.x509ca);
    if (old.x509crl != null) x509crl = new String(old.x509crl);
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

  public void save() {
    // CONNECTION OPTIONS
    UserPreferences.set("global", "RecvClipboard", recvClipboard);
    UserPreferences.set("global", "SendClipboard", sendClipboard);
    UserPreferences.set("global", "Shared", shared);

    // INPUT OPTIONS
    UserPreferences.set("global", "FSAltEnter", fsAltEnter);
    if (Utils.osGrab()) {
      if (grabKeyboard == GRAB_ALWAYS)
        UserPreferences.set("global", "GrabKeyboard", "Always");
      else if (grabKeyboard == GRAB_MANUAL)
        UserPreferences.set("global", "GrabKeyboard", "Manual");
      else
        UserPreferences.set("global", "GrabKeyboard", "FS");
    }
    for (MenuKey.MenuKeySymbol mks : MenuKey.getMenuKeySymbols()) {
      if (mks.keycode == menuKeyCode && mks.keysym == menuKeySym)
        UserPreferences.set("global", "MenuKey", mks.name);
    }
    UserPreferences.set("global", "ReverseScroll", reverseScroll);
    UserPreferences.set("global", "ViewOnly", viewOnly);

    // DISPLAY OPTIONS
    UserPreferences.set("global", "AcceptBell", acceptBell);
    UserPreferences.set("global", "CursorShape", cursorShape);
    UserPreferences.set("global", "DesktopSize", desktopSize.getString());
    UserPreferences.set("global", "FullScreen", fullScreen);

    if (scalingFactor == SCALE_AUTO)
      UserPreferences.set("global", "Scale", "Auto");
    else if (scalingFactor == SCALE_FIXEDRATIO)
      UserPreferences.set("global", "Scale", "FixedRatio");
    else
      UserPreferences.set("global", "Scale", scalingFactor);

    if (span == SPAN_PRIMARY)
      UserPreferences.set("global", "Span", "Primary");
    else if (span == SPAN_ALL)
      UserPreferences.set("global", "Span", "All");
    else
      UserPreferences.set("global", "Span", "Auto");

    UserPreferences.set("global", "Toolbar", showToolbar);

    // ENCODING OPTIONS
    UserPreferences.set("global", "CompressLevel", compressLevel);
    UserPreferences.set("global", "JPEG", allowJpeg);
    UserPreferences.set("global", "Quality", quality);

    if (subsampling == SUBSAMP_2X)
      UserPreferences.set("global", "Subsampling", "2X");
    else if (subsampling == SUBSAMP_4X)
      UserPreferences.set("global", "Subsampling", "4X");
    else if (subsampling == SUBSAMP_GRAY)
      UserPreferences.set("global", "Subsampling", "Gray");
    else
      UserPreferences.set("global", "Subsampling", "1X");

    // SECURITY AND AUTHENTICATION OPTIONS
    //
    // NOTE: We use key names of "SecTypes" rather than "SecurityTypes" and
    // "Username" rather than "User".  This is to prevent older versions of the
    // TurboVNC Viewer from automatically loading these values into parameters,
    // since older versions of the TurboVNC Viewer did not provide a way to
    // configure the values of those parameters using the GUI.
    UserPreferences.set("global", "SecTypes", getSecTypesString());
    UserPreferences.set("global", "SendLocalUsername", sendLocalUsername);
    UserPreferences.set("global", "tunnel", tunnel);
    UserPreferences.set("global", "Username", user);
    if (via != null && sshUser != null)
      UserPreferences.set("global", "via", sshUser + "@" + via);
    else
      UserPreferences.set("global", "via", via);
    UserPreferences.set("global", "x509ca", x509ca);
    UserPreferences.set("global", "x509crl", x509crl);

    UserPreferences.save();
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
    printOpt("port", port);
    printOpt("recvClipboard", recvClipboard);
    printOpt("sendClipboard", sendClipboard);
    printOpt("serverName", serverName);
    printOpt("shared", shared);
    printOpt("unixDomainPath", unixDomainPath);

    printOpt("fsAltEnter", fsAltEnter);
    printOpt("grabKeyboard", grabKeyboard);
    printOpt("menuKeyCode", menuKeyCode);
    printOpt("menuKeySym", menuKeySym);
    printOpt("reverseScroll", reverseScroll);
    printOpt("viewOnly", viewOnly);

    printOpt("acceptBell", acceptBell);
    printOpt("cursorShape", cursorShape);
    if (desktopSize.mode == SIZE_MANUAL)
      printOpt("desktopSize", desktopSize.getString());
    else
      printOpt("desktopSize", desktopSize.mode);
    printOpt("fullScreen", fullScreen);
    printOpt("scalingFactor", scalingFactor);
    printOpt("span", span);
    printOpt("showToolbar", showToolbar);

    printOpt("compressLevel", compressLevel);
    printOpt("preferredEncoding", preferredEncoding);
    printOpt("allowJpeg", allowJpeg);
    printOpt("quality", quality);
    printOpt("subsampling", subsampling);

    printOpt("secTypes", getSecTypesString());
    printOpt("sendLocalUsername", sendLocalUsername);
    printOpt("sshUser", sshUser);
    printOpt("tunnel", tunnel);
    printOpt("user", user);
    printOpt("via", via);
    printOpt("x509ca", x509ca);
    printOpt("x509crl", x509crl);
  }

  public void setSecurityTypes(String secTypesString) {
    if (secTypesString == null)
      enabledSecTypes = new ArrayList<Integer>();
    else
      enabledSecTypes = parseSecTypes(secTypesString);
  }

  public List<Integer> getEnabledSecTypes() {
    List<Integer> result = new ArrayList<Integer>();

    for (Iterator<Integer> i = enabledSecTypes.iterator(); i.hasNext();) {
      int refType = (Integer)i.next();
      if (refType >= 0x100) {
        result.add(RFB.SECTYPE_VENCRYPT);
        break;
      }
    }
    result.add(RFB.SECTYPE_TIGHT);
    result.add(RFB.SECTYPE_TLS);
    for (Iterator<Integer> i = enabledSecTypes.iterator(); i.hasNext();) {
      int refType = (Integer)i.next();
      if (refType < 0x100 && refType != RFB.SECTYPE_TIGHT &&
          refType != RFB.SECTYPE_TLS)
        result.add(refType);
    }

    return (result);
  }

  public List<Integer> getEnabledExtSecTypes() {
    List<Integer> result = new ArrayList<Integer>();

    for (Iterator<Integer> i = enabledSecTypes.iterator(); i.hasNext();) {
      int refType = (Integer)i.next();
      if (refType != RFB.SECTYPE_VENCRYPT && refType != RFB.SECTYPE_UNIX_LOGIN)
        /* ^^ Do not include VeNCrypt to avoid loops */
        result.add(refType);
    }

    return (result);
  }

  public List<Integer> getEnabledTightSecTypes() {
    List<Integer> result = new ArrayList<Integer>();

    for (Iterator<Integer> i = enabledSecTypes.iterator(); i.hasNext();) {
      int refType = (Integer)i.next();
      if (refType < 0x100 && refType != RFB.SECTYPE_VENCRYPT &&
          refType != RFB.SECTYPE_TIGHT && refType != RFB.SECTYPE_TLS)
        result.add(refType);
    }

    return (result);
  }

  private String getSecTypesString() {
    StringBuilder sb = new StringBuilder();
    for (Iterator<Integer> i = enabledSecTypes.iterator(); i.hasNext();) {
      int refType = ((Integer)i.next());
      if (refType != RFB.SECTYPE_VENCRYPT && refType != RFB.SECTYPE_TIGHT &&
          refType != RFB.SECTYPE_TLS) {
        sb.append(RFB.secTypeName(refType));
        if (i.hasNext()) sb.append(',');
      }
    }
    return sb.toString();
  }

  public void enableSecType(int secType) {
    for (Iterator<Integer> i = enabledSecTypes.iterator(); i.hasNext();)
      if ((Integer)i.next() == secType)
        return;

    if (isSecTypeAllowed(secType))
      enabledSecTypes.add(secType);
  }

  public boolean isSecTypeSupported(int secType) {
    Iterator<Integer> i;

    for (i = enabledSecTypes.iterator(); i.hasNext();)
      if ((Integer)i.next() == secType)
        return true;
    if (secType == RFB.SECTYPE_VENCRYPT)
      return true;
    if (secType == RFB.SECTYPE_TIGHT)
      return true;
    if (secType == RFB.SECTYPE_TLS)
      return true;

    return false;
  }

  public void disableSecType(int secType) {
    enabledSecTypes.remove((Object)secType);
  }

  private List<Integer> parseSecTypes(String types_) {
    List<Integer> result = new ArrayList<Integer>();
    String[] types = types_.split(",");
    for (int i = 0; i < types.length; i++) {
      if (types[i].length() < 1) continue;
      int typeNum = RFB.secTypeNum(types[i]);
      if (typeNum != RFB.SECTYPE_INVALID) {
        if (isSecTypeAllowed(typeNum))
          result.add(typeNum);
      } else
        throw new WarningException("Security type \'" + types[i] +
                                   "\' is not valid");
    }
    return result;
  }

  private boolean isSecTypeAllowed(int secType) {
    if (Params.noUnixLogin.getValue() &&
        (secType == RFB.SECTYPE_PLAIN || secType == RFB.SECTYPE_TLS_PLAIN ||
         secType == RFB.SECTYPE_X509_PLAIN ||
         secType == RFB.SECTYPE_UNIX_LOGIN))
      return false;
    if ((user != null || sendLocalUsername) &&
        (secType == RFB.SECTYPE_VNCAUTH || secType == RFB.SECTYPE_TLS_VNC ||
         secType == RFB.SECTYPE_X509_VNC || secType == RFB.SECTYPE_NONE ||
         secType == RFB.SECTYPE_TLS_NONE || secType == RFB.SECTYPE_X509_NONE))
      return false;
    return true;
  }

  public static class DesktopSize {
    public DesktopSize() {}

    // Deep copy
    public DesktopSize(DesktopSize old) {
      this(old.mode, old.width, old.height,
           (old.layout != null ? new ScreenSet(old.layout) : null));
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

    public boolean equalsIgnoreID(DesktopSize size) {
      return size.mode == mode && size.width == width &&
             size.height == height && size.layout.equalsIgnoreID(layout);
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

  // CONNECTION OPTIONS
  public int port;
  public boolean recvClipboard;
  public boolean sendClipboard;
  public String serverName;
  public boolean shared;
  public String unixDomainPath;
  // INPUT OPTIONS
  public boolean fsAltEnter;
  public int grabKeyboard;
  public int menuKeyCode, menuKeySym;
  public boolean reverseScroll;
  public boolean viewOnly;
  // DISPLAY OPTIONS
  public boolean acceptBell;
  public boolean cursorShape;
  public DesktopSize desktopSize = new DesktopSize();
  public boolean fullScreen;
  public int scalingFactor;
  public int span;
  public boolean showToolbar;
  // ENCODING OPTIONS
  public int compressLevel;
  public int preferredEncoding;
  public boolean allowJpeg;
  public int quality;
  public int subsampling;
  // SECURITY AND AUTHENTICATION OPTIONS
  private List<Integer> enabledSecTypes;
  public boolean sendLocalUsername;
  public Socket stdioSocket;
  public Session sshSession;
  public boolean sshTunnelActive;
  public boolean sessMgrActive;
  public String sshUser;
  public boolean tunnel;
  public String user;
  public String via;
  public String x509ca;
  public String x509crl;
}
