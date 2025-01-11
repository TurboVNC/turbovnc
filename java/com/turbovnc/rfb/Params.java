/* Copyright (C) 2012-2018, 2020-2025 D. R. Commander.  All Rights Reserved.
 * Copyright (C) 2021 Steffen Kieß
 * Copyright (C) 2011-2012, 2016 Brian P. Hinz
 * Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright 2004-2005 Cendio AB.
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

//
// Params - class for dealing with configuration parameters.
//

package com.turbovnc.rfb;

import java.io.*;
import java.util.*;

import com.turbovnc.network.Socket;
import com.turbovnc.rdr.*;

public final class Params {

  public static final int DEFQUAL = 95;

  public Params() {}

  // Deep copy
  public Params(Params oldParams) {
    VoidParameter current = head;

    while (current != null) {
      if (!(current instanceof HeaderParameter) &&
          !(current instanceof AliasParameter) &&
          !current.getName().equalsIgnoreCase("server") &&
          !current.getName().equalsIgnoreCase("port")) {
        VoidParameter oldCurrent = oldParams.get(current.getName());
        current.set(oldCurrent.getStr(), oldCurrent.isCommandLine());
      }
      current = current.next();
    }
  }

  // Set named parameter to value
  public boolean set(String name, String value, boolean commandLine) {
    VoidParameter param = get(name);
    if (param == null) return false;
    if (param instanceof BoolParameter && ((BoolParameter)param).reverse) {
      ((BoolParameter)param).reverse = false;
      ((BoolParameter)param).set(value, true, commandLine);
      return true;
    }
    return param.set(value, commandLine);
  }

  // Set parameter to value (separated by "=")
  public boolean set(String arg) {
    boolean hyphen = false;
    if (arg.charAt(0) == '-' && arg.length() > 1) {
      hyphen = true;
      if (arg.charAt(1) == '-')
        arg = arg.substring(2);  // allow gnu-style --<option>
      else
        arg = arg.substring(1);
    }
    int equal = arg.indexOf('=');
    if (equal != -1) {
      return set(arg.substring(0, equal), arg.substring(equal + 1), true);
    } else if (hyphen) {
      VoidParameter param = get(arg);
      if (param == null) return false;
      if (param instanceof BoolParameter) {
        if (((BoolParameter)param).reverse) {
          ((BoolParameter)param).reverse = false;
          ((BoolParameter)param).set(false);
        } else
          ((BoolParameter)param).set(true);
        param.setCommandLine(true);
        return true;
      }
    }
    return false;
  }

  // Set named parameter to value, but only if it is configurable in the
  // Options dialog and has not already been set on the command line or in a
  // connection info file.  This allows us to merge per-host options saved by
  // the Options dialog with parameters set on the command line or in a
  // connection info file without overwriting the parameters.  (The TurboVNC
  // Viewer always gives precedence to command-line/connection info file
  // parameters.)
  public boolean setGUI(String name, String value) {
    VoidParameter param = get(name);
    if (param == null) return false;
    if (param.isCommandLine() || !param.isGUI()) return false;
    return set(name, value, false);
  }

  // Get named parameter
  public VoidParameter get(String name) {
    VoidParameter current = head;
    while (current != null) {
      if (name.equalsIgnoreCase(current.getName()))
        return current;
      if (current instanceof BoolParameter) {
        if (name.length() > 2 && name.substring(0, 2).equalsIgnoreCase("no")) {
          String name2 = name.substring(2);
          if (name2.equalsIgnoreCase(current.getName())) {
            ((BoolParameter)current).reverse = true;
            return current;
          }
        }
      }
      current = current.next();
    }
    return null;
  }

  public void list(int width) {
    VoidParameter current = head;

    while (current != null) {
      String desc = current.getDescription();
      if (desc == null) {
        current = current.next();
        continue;
      }
      desc = desc.trim();
      if (current instanceof HeaderParameter) {
        System.out.println(desc);
        for (int i = 0; i < desc.length(); i++)
          System.out.print("-");
        current = current.next();
        System.out.print("\n\n");
        continue;
      }

      System.out.print("--> " + current.getName() + "\n    ");
      if (current.getValues() != null)
        System.out.print("Values: " + current.getValues() + " ");
      if (current.getDefaultStr() != null)
        System.out.print("(default = " + current.getDefaultStr() + ")\n");
      System.out.print("\n   ");

      int column = 4;
      while (true) {
        int s = desc.indexOf(' ');
        while (desc.charAt(s + 1) == ' ') s++;
        int wordLen;
        if (s > -1) wordLen = s;
        else wordLen = desc.length();

        if (column + wordLen + 1 > width) {
          System.out.print("\n   ");
          column = 4;
        }
        if (wordLen >= 1 && desc.charAt(wordLen - 1) == '\r') {
          System.out.format("%" + wordLen + "s",
                            desc.substring(0, wordLen - 1));
          column += wordLen;
        } else {
          System.out.format(" %" + wordLen + "s", desc.substring(0, wordLen));
          column += wordLen + 1;
        }
        if (wordLen >= 1 && (desc.charAt(wordLen - 1) == '\n' ||
                             desc.charAt(wordLen - 1) == '\r')) {
          System.out.print("\n   ");
          column = 4;
        }

        if (s == -1) break;
        desc = desc.substring(wordLen + 1);
      }
      current = current.next();
      System.out.print("\n\n");
    }
  }

  public void loadLegacy(String filename) {
    if (filename == null)
      return;

    /* Read parameters from file */
    Properties props = new Properties();
    try {
      props.load(new FileInputStream(filename));
    } catch (Exception e) {
      throw new WarningException("Cannot open connection info file:\n" +
                                 e.getMessage());
    }

    int scaleNum = -1, scaleDenom = -1, fitWindow = -1;
    int resizeMode = -1, desktopWidth = -1, desktopHeight = -1;
    String desktopSizeStr = null;

    for (Enumeration<?> i = props.propertyNames();  i.hasMoreElements();) {
      String name = (String)i.nextElement();

      if (name.startsWith("[")) {
        // skip the section delimiters
        continue;
      } else if (name.equalsIgnoreCase("host")) {
        set("Server", props.getProperty(name), true);
      } else if (name.equalsIgnoreCase("port") ||
                 name.equalsIgnoreCase("user") ||
                 name.equalsIgnoreCase("restricted") ||
                 name.equalsIgnoreCase("viewonly") ||
                 name.equalsIgnoreCase("reversescroll") ||
                 name.equalsIgnoreCase("fullscreen") ||
                 name.equalsIgnoreCase("fsaltenter") ||
                 name.equalsIgnoreCase("shared") ||
                 name.equalsIgnoreCase("cursorshape") ||
                 name.equalsIgnoreCase("compresslevel") ||
                 name.equalsIgnoreCase("nounixlogin")) {
        set(name, props.getProperty(name), true);
      } else if (name.equalsIgnoreCase("password")) {
        set("EncPassword", props.getProperty(name), true);
      } else if (name.equalsIgnoreCase("preferred_encoding")) {
        int encodingNum = -1;
        try {
          encodingNum = Integer.parseInt(props.getProperty(name));
        } catch (NumberFormatException e) {}
        if (encodingNum >= 0 && encodingNum <= RFB.ENCODING_LAST)
          set("Encoding", RFB.encodingName(encodingNum), true);
      } else if (name.equalsIgnoreCase("grabkeyboard")) {
        int grabKeyboardValue = -1;
        try {
          grabKeyboardValue = Integer.parseInt(props.getProperty(name));
        } catch (NumberFormatException e) {}
        switch (grabKeyboardValue) {
          case GrabParameter.FS:
            set("GrabKeyboard", "FS", true);  break;
          case GrabParameter.ALWAYS:
            set("GrabKeyboard", "Always", true);  break;
          case GrabParameter.MANUAL:
            set("GrabKeyboard", "Manual", true);  break;
        }
      } else if (name.equalsIgnoreCase("span")) {
        int spanValue = -1;
        try {
          spanValue = Integer.parseInt(props.getProperty(name));
        } catch (NumberFormatException e) {}
        if (spanValue == 0) set("Span", "Primary", true);
        else if (spanValue == 1) set("Span", "All", true);
        else if (spanValue == 2) set("Span", "Auto", true);
      } else if (name.equalsIgnoreCase("8bit")) {
        int _8bit = -1;
        try {
          _8bit = Integer.parseInt(props.getProperty(name));
        } catch (NumberFormatException e) {}
        if (_8bit >= 1)
          set("Colors", "256", true);
        else if (_8bit == 0)
          set("Colors", "-1", true);
      } else if (name.equalsIgnoreCase("disableclipboard")) {
        int disableclipboard = -1;
        try {
          disableclipboard = Integer.parseInt(props.getProperty(name));
        } catch (NumberFormatException e) {}
        if (disableclipboard >= 1) {
          set("RecvClipboard", "0", true);
          set("SendClipboard", "0", true);
        } else if (disableclipboard == 0) {
          set("RecvClipboard", "1", true);
          set("SendClipboard", "1", true);
        }
      } else if (name.equalsIgnoreCase("fitwindow")) {
        try {
          fitWindow = Integer.parseInt(props.getProperty(name));
        } catch (NumberFormatException e) {}
      } else if (name.equalsIgnoreCase("scale_num")) {
        int temp = -1;
        try {
          temp = Integer.parseInt(props.getProperty(name));
        } catch (NumberFormatException e) {}
        if (temp >= 1) scaleNum = temp;
      } else if (name.equalsIgnoreCase("scale_den")) {
        int temp = -1;
        try {
          temp = Integer.parseInt(props.getProperty(name));
        } catch (NumberFormatException e) {}
        if (temp >= 1) scaleDenom = temp;
      } else if (name.equalsIgnoreCase("resizemode")) {
        int temp = -1;
        try {
          temp = Integer.parseInt(props.getProperty(name));
        } catch (NumberFormatException e) {}
        if (temp >= 0) resizeMode = temp;
      } else if (name.equalsIgnoreCase("desktopwidth")) {
        int temp = -1;
        try {
          temp = Integer.parseInt(props.getProperty(name));
        } catch (NumberFormatException e) {}
        if (temp >= 1) desktopWidth = temp;
      } else if (name.equalsIgnoreCase("desktopheight")) {
        int temp = -1;
        try {
          temp = Integer.parseInt(props.getProperty(name));
        } catch (NumberFormatException e) {}
        if (temp >= 1) desktopHeight = temp;
      } else if (name.equalsIgnoreCase("desktopsize")) {
        desktopSizeStr = props.getProperty(name);
      } else if (name.equalsIgnoreCase("noremotecursor")) {
        set("LocalCursor", props.getProperty(name), true);
      } else if (name.equalsIgnoreCase("subsampling")) {
        int subsamplingValue = -1;
        try {
          subsamplingValue = Integer.parseInt(props.getProperty(name));
        } catch (NumberFormatException e) {}
        switch (subsamplingValue) {
          case SubsampParameter.NONE:
            set("Subsampling", "1X", true);  break;
          case SubsampParameter.FOURX:
            set("Subsampling", "4X", true);  break;
          case SubsampParameter.TWOX:
            set("Subsampling", "2X", true);  break;
          case SubsampParameter.GRAY:
            set("Subsampling", "Gray", true);  break;
        }
      } else if (name.equalsIgnoreCase("quality")) {
        int qualityValue = -2;
        try {
          qualityValue = Integer.parseInt(props.getProperty(name));
        } catch (NumberFormatException e) {}
        if (qualityValue == -1) set("JPEG", "0", true);
        else if (qualityValue >= 1 && qualityValue <= 100) {
          set("Quality", props.getProperty(name), true);
        }
      } else if (name.equalsIgnoreCase("continuousupdates")) {
        set("CU", props.getProperty(name), true);
      }
    }

    if ((scaleNum >= 1 || scaleDenom >= 1) && fitWindow < 1) {
      if (scaleNum < 1) scaleNum = 1;
      if (scaleDenom < 1) scaleDenom = 1;
      int scalingFactorValue = scaleNum * 100 / scaleDenom;
      set("Scale", Integer.toString(scalingFactorValue), true);
    } else if (fitWindow >= 1) {
      set("Scale", "FixedRatio", true);
    }

    if (desktopSizeStr != null)
      set("DesktopSize", desktopSizeStr, true);
    else {
      switch (resizeMode) {
        case DesktopSize.SERVER:
          set("DesktopSize", "Server", true);  break;
        case DesktopSize.MANUAL:
          if (desktopWidth > 0 && desktopHeight > 0)
            set("DesktopSize", desktopWidth + "x" + desktopHeight, true);
          break;
        case DesktopSize.AUTO:
          set("DesktopSize", "Auto", true);  break;
      }
    }

    reconcile();
  }

  public void load(String filename) {
    if (filename == null)
      return;

    /* Read parameters from file */
    Properties props = new Properties();
    try {
      props.load(new FileInputStream(filename));
    } catch (Exception e) {
      throw new WarningException("Cannot open connection info file:\n" +
                                 e.getMessage());
    }

    for (Enumeration<?> i = props.propertyNames();  i.hasMoreElements();) {
      String name = (String)i.nextElement();

      if (name.startsWith("[")) {
        // skip the section delimiters
        continue;
      } else {
        set(name, props.getProperty(name), true);
      }
    }

    reconcile();
  }

  public void loadDefaults() {
    String filename = Utils.getHomeDir() + ".vnc" + Utils.getFileSeparator() +
                      "default.turbovnc";
    File file = new File(filename);
    if (!file.exists())
      return;

    vlog.info("Loading parameter defaults from " + filename);

    /* Read parameters from file */
    Properties props = new Properties();
    try {
      props.load(new FileInputStream(file));
    } catch (Exception e) {
      vlog.info("Could not load parameter defaults:");
      vlog.info("  " + e.getMessage());
    }

    for (Enumeration<?> i = props.propertyNames();  i.hasMoreElements();) {
      String name = (String)i.nextElement();

      VoidParameter param = get(name);
      if (param == null) {
        vlog.info("Invalid parameter name " + name);
        continue;
      }
      if (!param.setDefault(props.getProperty(name)))
        vlog.info("Invalid default value for " + param.getName() +
                  " parameter");
    }

    reconcile();
  }

  public void print(String message) {
    VoidParameter current = head;

    System.out.println("\nParameters (" + message + "):");

    while (current != null) {
      if (!(current instanceof HeaderParameter)) {
        String str = current.getStr();
        System.out.println(current.getName() + " = " +
                           (str == null ? "" : str));
      }
      current = current.next();
    }
  }

  public void reconcile() {
    if (scale.get() != 100 && desktopSize.getMode() == DesktopSize.AUTO) {
      vlog.info("Desktop scaling enabled.  Disabling automatic desktop resizing.");
      desktopSize.setMode(DesktopSize.SERVER);
    }
  }

  // Reset any parameter that is configurable in the Options dialog to its
  // default value, if is has not already been set on the command line or in a
  // connection info file.
  public void resetGUI() {
    VoidParameter current = head;

    while (current != null) {
      if (current.isGUI() && !current.isCommandLine())
        current.reset();
      current = current.next();
    }

    reconcile();
  }

  public void save(String node) {
    VoidParameter current = head;

    if (node == null) return;

    while (current != null) {
      if (current.isGUI()) {
        String name = current.getName();
        String value = current.getStr();

        UserPreferences.set(node, name, value);
      }
      current = current.next();
    }

    UserPreferences.save(node);
  }

  // CHECKSTYLE Indentation:OFF
  // CHECKSTYLE VisibilityModifier:OFF

  // CONNECTION PARAMETERS

  private HeaderParameter connHeader =
  new HeaderParameter("ConnHeader", this, "CONNECTION PARAMETERS");

  public BoolParameter alwaysShowConnectionDialog =
  new BoolParameter("AlwaysShowConnectionDialog", this, false,
  "Always show the \"New TurboVNC Connection\" dialog even if the server " +
  "has been specified on the command line.", false);

  public BoolParameter confirmClose =
  new BoolParameter("ConfirmClose", this, false,
  "Prompt for confirmation before closing a connection.", false);

  public BoolParameter copyRect =
  new BoolParameter("CopyRect", this, false,
  null, true);

  public BoolParameter continuousUpdates =
  new BoolParameter("CU", this, false,
  null, true);

  public BoolParameter listenMode =
  new BoolParameter("Listen", this, false,
  "Start the viewer in \"listen mode.\"  The viewer will listen on port " +
  "5500 (or on the port specified by the Port parameter) for reverse " +
  "connections from a VNC server.  To connect a TurboVNC session to a " +
  "listening viewer, use the vncconnect program on the TurboVNC host.", false);

  public IntParameter maxClipboard =
  new IntParameter("MaxClipboard", this, false,
  "Maximum permitted length of an incoming or outgoing clipboard update (in " +
  "bytes)", 1048576, 0);

  public BoolParameter noNewConn =
  new BoolParameter("NoNewConn", this, false,
  "Always exit after the first connection closes, and do not allow new " +
  "connections to be made without restarting the viewer.  This is useful in " +
  "portal environments that need to control when and how the viewer is " +
  "launched.  Setting this parameter also disables the \"Close Connection\" " +
  "option in the F8 menu and the \"Disconnect\" button in the toolbar.",
  false);

  public BoolParameter noReconnect =
  new BoolParameter("NoReconnect", this, false,
  "If the viewer is disconnected from the server unexpectedly, exit rather " +
  "than ask whether you want to reconnect.", false);

  public IntParameter port =
  new IntParameter("Port", this, false,
  "The TCP port number on which the VNC server is listening.  For Un*x VNC " +
  "servers, this is typically 5900 + the X display number of the VNC " +
  "session (example: 5901 if connecting to display :1.)  For Windows and " +
  "Mac VNC servers, this is typically 5900." +
  (Utils.getBooleanProperty("turbovnc.sessmgr", true) ? "" :
   "  (default = 5900)") + "\n " +

  "If listen mode is enabled, this parameter specifies the TCP port on " +
  "which the viewer will listen for reverse connections from a VNC server.  " +
  "(default = 5500)", -1, 0, 65535);

  public IntParameter profileInt =
  new IntParameter("ProfileInterval", this, false,
  "How often (in seconds) that performance statistics are updated in the " +
  "profiling dialog or on the console when profiling is enabled.  The " +
  "statistics are averaged over this interval.  Setting the TVNC_PROFILE " +
  "environment variable to 1 causes the viewer to display performance " +
  "statistics about the connection, such as how many updates per second are " +
  "being received and how much network bandwidth is being used, to the " +
  "console.  Profiling can also be activated by selecting \"Performance " +
  "Info...\" in the F8 menu, which pops up a dialog that displays the same " +
  "statistics.", 5);

  public BoolParameter recvClipboard =
  new BoolParameter("RecvClipboard", this, true,
  "Synchronize the local clipboard with the TurboVNC session's clipboard " +
  "when the latter changes.", true);

  public BoolParameter sendClipboard =
  new BoolParameter("SendClipboard", this, true,
  "Synchronize the TurboVNC session's clipboard with the local clipboard " +
  "when the latter changes.", true);

  public ServerNameParameter server =
  new ServerNameParameter("Server", this, false,
  "The VNC server to which to connect.  This can be specified in the " +
  "format {host}[:{display_number}], {host}::{port}, or {host}::{uds_path}, " +
  "where {host} is the host name or IP address of the machine on which the " +
  "VNC server is running (the \"VNC host\"), {display_number} is an " +
  "optional X display " +
  (Utils.getBooleanProperty("turbovnc.sessmgr", true) ?
   "number, " : "number (default: 0), ") +
  "{port} is a TCP port, and {uds_path} is the path (which must begin with " +
  "/ or ~) to a Unix domain socket on the VNC host." +
  (Utils.getBooleanProperty("turbovnc.sessmgr", true) ?
   "  If no port, Unix domain socket path, or display number is specified, " +
   "then the viewer will enable the TurboVNC Session Manager, which allows " +
   "you to remotely start a new TurboVNC session or to choose an existing " +
   "session to which to connect.\n " : "\n ") +

  "In Unix domain socket paths, ~ is expanded to the user's home directory " +
  "on the VNC host, %h is expanded to the VNC host name (from the point of " +
  "view of the VNC host), %i is expanded to the numeric user ID on the VNC " +
  "host, and %u is expanded to the username on the VNC host.  When " +
  "listening on a Unix domain socket, the TurboVNC Server chooses a Unix " +
  "domain socket path of ~/.vnc/%h_{display_number}.uds by default.", null);

  public BoolParameter shared =
  new BoolParameter("Shared", this, true,
  "Request a shared VNC session.  When the session is shared, other users " +
  "can connect to the session (assuming they have the correct " +
  "authentication credentials) and collaborate with the user who started " +
  "the session.  If this parameter is disabled and the TurboVNC session is " +
  "using default settings, then you will only be able to connect to the " +
  "session if no one else is already connected.", true);

  // INPUT PARAMETERS

  private HeaderParameter inputHeader =
  new HeaderParameter("InputHeader", this, "INPUT PARAMETERS");

  public BoolParameter fsAltEnter =
  new BoolParameter("FSAltEnter", this, true,
  "Toggle full-screen mode when Alt-Enter is pressed.", false);

  public GrabParameter grabKeyboard =
  new GrabParameter("GrabKeyboard", this, true,
  "Intercept special key sequences, such as Alt-Tab, that are used to " +
  "switch windows and perform other window management functions, and pass " +
  "those key sequences to the VNC server.\n " +

  "Values:\r " +
  "\"FS\" = Automatically grab the keyboard in full-screen mode and ungrab " +
  "it in windowed mode.\r " +
  "\"Always\" = Automatically grab the keyboard in both full-screen mode " +
  "and windowed mode.\r " +
  "\"Manual\" = Only grab/ungrab the keyboard when the \"Grab Keyboard\" F8 " +
  "menu option is selected or the Ctrl-Alt-Shift-G hotkey is pressed.\n " +

  "Regardless of the grabbing mode, the F8 menu option and hotkey can " +
  "always be used to grab or ungrab the keyboard.", GrabParameter.FS);

  public BoolParameter grabPointer =
  new BoolParameter("GrabPointer", this, false,
  Utils.isX11() ? "Grab the pointer whenever the keyboard is grabbed.  This " +
  "allows certain keyboard + pointer sequences, such as Alt-{drag}, to be " +
  "passed to the server.  The downside, however, is that grabbing the " +
  "pointer prevents any interaction with the local window manager.  (For " +
  "instance, the window can no longer be maximized or closed, and you " +
  "cannot switch to other running applications.)  Thus, this parameter is " +
  "primarily useful with GrabKeyboard=FS." : null, true);

  public MenuKeyParameter menuKey =
  new MenuKeyParameter("MenuKey", this, true,
  "The key used to display the popup menu", "F8");

  public BoolParameter noMacHotkeys =
  new BoolParameter("NoMacHotkeys", this, false,
  Utils.isMac() ? "On macOS, the TurboVNC Viewer normally assigns " +
  "equivalent Command hotkeys for most of its Ctrl-Alt-Shift hotkeys.  " +
  "However, since the Command key maps to the Super/Meta key on Un*x " +
  "systems, those Command hotkeys may interfere with hotkeys used by " +
  "certain applications (such as Emacs) on the remote system.  Setting this " +
  "parameter allows as many Command key combinations as possible to be " +
  "transmitted to the VNC server as Super/Meta key combinations, although " +
  "some Command key combinations (notably Command-F5, Command-Tab, " +
  "Command-H, Command-Q, Command-Comma, and Command-Space) will still be " +
  "intercepted by macOS." : null, false);

  // Prevent the viewer from sending Ctrl-Alt-Del and Ctrl-Esc to the server
  public BoolParameter restricted =
  new BoolParameter("Restricted", this, false,
  null, false);

  public BoolParameter reverseScroll =
  new BoolParameter("ReverseScroll", this, true,
  "Reverse the direction of mouse scroll wheel events that are sent to the " +
  "VNC server.  This is useful when connecting from clients that have " +
  "\"natural scrolling\" enabled.", false);

  public BoolParameter serverKeyMap =
  new BoolParameter("ServerKeyMap", this, false,
  null, true);

  public BoolParameter viewOnly =
  new BoolParameter("ViewOnly", this, true,
  "Ignore all keyboard and mouse events in the viewer window and do not " +
  "pass those events to the VNC server.", false);

  // Set to 0 to disable the view-only checkbox in the Options dialog
  public BoolParameter viewOnlyControl =
  new BoolParameter("ViewOnlyControl", this, false,
  null, true);

  // DISPLAY PARAMETERS

  private HeaderParameter displayHeader =
  new HeaderParameter("DisplayHeader", this, "DISPLAY PARAMETERS");

  public BoolParameter acceptBell =
  new BoolParameter("AcceptBell", this, true,
  "Produce a system beep when a \"bell\" event is received from the VNC " +
  "server.", true);

  public IntParameter colors =
  new IntParameter("Colors", this, false,
  "Color depth to use for the viewer's window\n " +

  "Values:\r " +
  "8 = BGR111 pixel format (1 bit for each red, green, and blue " +
  "component)\r " +
  "64 = BGR222 pixel format\r " +
  "256 = BGR233 pixel format\r " +
  "32768 = BGR555 pixel format\r " +
  "65536 = BGR565 pixel format\n " +

  "Lowering the color depth can significantly reduce network usage when " +
  "using encoding types other than Tight or when using Tight encoding " +
  "without JPEG.  However, colors will not be represented accurately, and " +
  "CPU usage will increase substantially (causing a corresponding decrease " +
  "in performance on fast networks.)  The default is to use the native " +
  "color depth of the display on which the viewer is running, which is " +
  "usually true color (8 bits per component.)", -1);

  public BoolParameter compatibleGUI =
  new BoolParameter("CompatibleGUI", this, false,
  "Expose all compression levels (0-9) in the TurboVNC Viewer Options " +
  "dialog (useful when connecting to non-TurboVNC servers.)  This parameter " +
  "is effectively set when using any encoding type other than Tight or when " +
  "selecting a compression level that is not useful for TurboVNC servers.",
  false);

  public BoolParameter currentMonitorIsPrimary =
  new BoolParameter("CurrentMonitorIsPrimary", this, false,
  "For the purposes of multi-screen spanning, treat the monitor that " +
  "contains the largest number of pixels from the viewer window as the " +
  "primary monitor.  If this parameter is disabled, then the left-most and " +
  "top-most monitor will always be the primary monitor (as was the case in " +
  "TurboVNC 2.0 and prior.)", true);

  public BoolParameter cursorShape =
  new BoolParameter("CursorShape", this, true,
  "Normally, the TurboVNC Server and compatible VNC servers send only " +
  "changes to the remote mouse cursor's shape and position.  This results " +
  "in the best mouse responsiveness.  Disabling this parameter causes the " +
  "server to instead render the mouse cursor and send it to the viewer as " +
  "an image every time the cursor moves or changes shape.  Using a remotely " +
  "rendered cursor can increase network \"chatter\" between host and client " +
  "significantly, which may cause performance problems on slow networks.",
  true);

  public DesktopSizeParameter desktopSize =
  new DesktopSizeParameter("DesktopSize", this, true,
  "If the VNC server supports remote desktop resizing, then attempt to " +
  "resize the remote desktop to the specified size (example: 1920x1200) or " +
  "reconfigure the server's virtual screens with a specified layout " +
  "(example: 1920x1200+0+0,1920x1200+1920+0).  Setting this parameter to " +
  "\"Auto\" causes the remote desktop to be resized to fit in the viewer " +
  "window without using scrollbars, and it causes the server's virtual " +
  "screens to be reconfigured such that their screen boundaries align with " +
  "the client's screen boundaries when the viewer window is in its default " +
  "position.  Setting this parameter to \"Server\" or \"0\" disables remote " +
  "desktop resizing and uses the desktop size and screen configuration set " +
  "by the server.", "Auto");

  public BoolParameter fullScreen =
  new BoolParameter("FullScreen", this, true,
  "Start the viewer in full-screen mode.", false);

  public BoolParameter localCursor =
  new BoolParameter("LocalCursor", this, false,
  "The default behavior of the TurboVNC Viewer is to hide the local cursor " +
  "and show only the remote cursor, which can be rendered either by the " +
  "VNC server or on the client, depending on the value of the CursorShape " +
  "parameter.  However, certain (broken) VNC server implementations do not " +
  "support either method of remote cursor rendering, so this parameter is " +
  "provided as a workaround for connecting to such servers.  If this " +
  "parameter is set, then any cursor shape updates from the server are " +
  "ignored, and the local cursor is always displayed.", false);

  public ScaleParameter scale =
  new ScaleParameter("Scale", this, true,
  "Reduce or enlarge the remote desktop image.  The value is interpreted as " +
  "a scaling factor in percent.  The default value of 100% corresponds to " +
  "the original remote desktop size.  Values below 100 reduce the image " +
  "size, whereas values above 100 enlarge the image proportionally.  If " +
  "this parameter is set to \"Auto\", then automatic scaling is performed.  " +
  "Automatic scaling reduces or enlarges the remote desktop image such that " +
  "the entire image will fit in the viewer window without using " +
  "scrollbars.  If this parameter is set to \"FixedRatio\", then automatic " +
  "scaling is performed, but the original aspect ratio is preserved.  " +
  "Enabling scaling disables automatic desktop resizing.", 100);

  public SpanParameter span =
  new SpanParameter("Span", this, true,
  "Multi-screen spanning mode\n " +

  "Values:\r " +
  "\"Primary\" = The viewer window should span only the primary monitor.\r " +
  "\"All\" = The viewer window should span all monitors.\r " +
  "\"Auto\" = The viewer window should span all monitors only if the window " +
  "cannot fit on the primary monitor.\n " +

  "When using automatic desktop resizing, \"Auto\" has the same effect as " +
  "\"Primary\" when in windowed mode and the same effect as \"All\" when in " +
  "full-screen mode." +
  (Utils.isX11() ? "  Due to general issues with spanning windows across " +
   "multiple monitors in X11, this parameter has no effect on Un*x/X11 " +
   "platforms except in full-screen mode." : "") +
  (Utils.isMac() ? "  This parameter has no effect on macOS unless " +
   "\"Displays have separate Spaces\" is disabled in the system settings." :
   ""), SpanParameter.AUTO);

  public BoolParameter toolbar =
  new BoolParameter("Toolbar", this, true,
  "Show the toolbar by default.", true);

  // ENCODING PARAMETERS

  private HeaderParameter encHeader =
  new HeaderParameter("EncHeader", this, "ENCODING PARAMETERS");

  public IntParameter compressLevel =
  new IntParameter("CompressLevel", this, true,
  "When Tight encoding is used, the compression level specifies the amount " +
  "of zlib compression to apply to subrectangles encoded using the indexed " +
  "color, mono, and raw subencoding types.  If the JPEG subencoding type is " +
  "enabled, then the compression level also defines the \"palette " +
  "threshold\", or the minimum number of unique colors that a subrectangle " +
  "must have before it is encoded using JPEG.  Higher compression levels " +
  "have higher palette thresholds and thus favor the use of indexed color " +
  "subencoding, whereas lower compression levels favor the use of JPEG.\n " +

  "Compression Level 1 is usually the default when JPEG is enabled, because " +
  "extensive experimentation has revealed little or no benefit to using " +
  "higher compression levels with most 3D and video workloads.  However, " +
  "v1.1 and later of the TurboVNC Server also supports Compression Level 2 " +
  "when JPEG is enabled.  Compression Level 2 can reduce the network usage " +
  "of certain types of low-color workloads by about 20-40% (with a " +
  "commensurate increase in CPU usage.)\n " +

  "In v1.2 or later of the TurboVNC Server, compression levels 5-7 map to " +
  "compression levels 0-2, but they also enable the interframe comparison " +
  "engine in the server.  Interframe comparison maintains a copy of the " +
  "remote framebuffer for each connected viewer and compares each " +
  "framebuffer update with the copy to ensure that redundant updates are " +
  "not sent to the viewer.  This prevents unnecessary network traffic if an " +
  "ill-behaved application draws the same thing over and over again, but " +
  "interframe comparison also causes the TurboVNC Server to use more CPU " +
  "time and much more memory.", 1, 0, 9);

  public EncodingParameter encoding =
  new EncodingParameter("Encoding", this, false,
  "Preferred RFB encoding type to use.  If the server does not support the " +
  "preferred encoding type, then the next best one will be chosen.  There " +
  "should be no reason to use an encoding type other than Tight when " +
  "connecting to a TurboVNC session, but this parameter can be useful when " +
  "connecting to other types of VNC servers, such as RealVNC.",
  RFB.ENCODING_TIGHT);

  public BoolParameter jpeg =
  new BoolParameter("JPEG", this, true,
  "Enable the JPEG subencoding type when using Tight encoding.  This causes " +
  "the Tight encoder to use JPEG compression for subrectangles that have a " +
  "high number of unique colors and indexed color subencoding for " +
  "subrectangles that have a low number of unique colors.  If this " +
  "parameter is disabled, then the Tight encoder will select between " +
  "indexed color or raw subencoding, depending on the size of the " +
  "subrectangle and its color count.", true);

  public IntParameter quality =
  new IntParameter("Quality", this, true,
  "JPEG quality to use when compressing JPEG images with the Tight+JPEG " +
  "encoding methods.  Lower quality values produce grainier JPEG images " +
  "with more noticeable compression artifacts, but lower quality values " +
  "also use less network bandwidth and CPU time.  The default value of " +
  DEFQUAL + " should be perceptually lossless (that is, any image " +
  "compression artifacts it produces should be imperceptible to the human " +
  "eye under most viewing conditions.)", DEFQUAL, 1, 100);

  public SubsampParameter subsampling =
  new SubsampParameter("Subsampling", this, true,
  "Chrominance subsampling level to use when compressing JPEG images with " +
  "the Tight+JPEG encoding methods\n " +

  "When compressing an image using JPEG, the RGB pixels are first converted " +
  "to the YCbCr colorspace, a colorspace in which each pixel is represented " +
  "as a brightness (Y, or \"luminance\") value and a pair of color (Cb and " +
  "Cr, or \"chrominance\") values.  After this colorspace conversion, " +
  "chrominance subsampling can be used to discard some of the chrominance " +
  "components in order to save bandwidth.\n " +

  "Values:\r " +
  "\"1X\" = Retain the chrominance components for all pixels (best image " +
  "quality but highest network and CPU usage.)\r " +
  "\"2X\" = Retain the chrominance components for every other pixel.\r " +
  "\"4X\" = Retain the chrominance components for every fourth pixel " +
  "(typically implemented as 2X subsampling in both X and Y directions.)\r " +
  "\"Grayscale\" = Discard all chrominance components, leaving only " +
  "luminance.\n " +

  "2X and 4X subsampling typically produce noticeable aliasing of lines and " +
  "other sharp features, but with photographic or other \"smooth\" image " +
  "content, it may be difficult to detect any difference between 1X, 2X, " +
  "and 4X.", SubsampParameter.NONE);

  private AliasParameter samp =
  new AliasParameter("Samp", this,
  "Alias for Subsampling", subsampling);

  // SECURITY AND AUTHENTICATION PARAMETERS

  private HeaderParameter secHeader =
  new HeaderParameter("SecHeader", this,
  "SECURITY AND AUTHENTICATION PARAMETERS");

  public BoolParameter autoPass =
  new BoolParameter("AutoPass", this, false,
  "Read a plain-text password from stdin and use this password when " +
  "authenticating with the VNC server.  It is strongly recommended that " +
  "this parameter be used only with a one-time password or other disposable " +
  "token.", false);

  public StringParameter encPassword =
  new StringParameter("EncPassword", this, false,
  "Encrypted password, in ASCII hex format, to use when authenticating with " +
  "the VNC server.  You can generate an ASCII hex encrypted password on a " +
  "TurboVNC host by executing\n " +

  "'echo {unencrypted_password} | /opt/TurboVNC/bin/vncpasswd -f | xxd -c 256 -ps'\n " +

  "This parameter allows a password to be supplied to the TurboVNC Viewer " +
  "without exposing the password as plain text.  However, the encryption " +
  "scheme (DES) used for VNC passwords is not particularly strong, so " +
  "encrypting the password guards against only the most casual of attacks.  " +
  "Thus, it is recommended that this parameter be used only with a one-time " +
  "password or other disposable token.", null);

  public BoolParameter extSSH =
  new BoolParameter("ExtSSH", this, false,
  "When using the Via or Tunnel parameters, use an external SSH client " +
  "instead of the built-in SSH client.  The external SSH client command can " +
  "be specified using the ExtSSHCommand parameter, and you can also use the " +
  "VNC_VIA_CMD and VNC_TUNNEL_CMD environment variables or the turbovnc.via " +
  "and turbovnc.tunnel system properties to specify the SSH command-line " +
  "templates for creating the tunnel.  If one of those environment " +
  "variables or system properties is set, then an external SSH client is " +
  "automatically used.  See the TurboVNC User's Guide for more details.\n " +

  (Utils.getBooleanProperty("turbovnc.sessmgr", true) ?
   "When using the TurboVNC Session Manager, this parameter is effectively " +
   "disabled.  (The built-in SSH client is always used with the TurboVNC " +
   "Session Manager.)\n " : "") +

  "This parameter is effectively set if the Server parameter specifies a " +
  "Unix domain socket connection to a remote host.", false);

  public StringParameter extSSHCommand =
  new StringParameter("ExtSSHCommand", this, false,
  "The command that should be used to start the external SSH client",
  (Utils.isWindows() ? "ssh.exe -ax" : "/usr/bin/env ssh -ax"));

  public BoolParameter localUsernameLC =
  new BoolParameter("LocalUsernameLC", this, false,
  "When the SendLocalUsername parameter is set, or when using SSH " +
  "tunneling without a specified SSH username, convert the local username " +
  "to lowercase before using it for authentication.  This may be useful " +
  "with Windows clients, since Windows allows mixed-case usernames but Un*x " +
  "and Mac systems generally don't.", false);

  public BoolParameter noUnixLogin =
  new BoolParameter("NoUnixLogin", this, false,
  "Disable Unix Login authentication when connecting to TightVNC-compatible " +
  "servers and Plain authentication when connecting to VeNCrypt-compatible " +
  "servers.  Setting this parameter effectively removes \"Plain\" (and its " +
  "encrypted derivatives) and \"UnixLogin\" from the value of the " +
  "SecurityTypes parameter.  This is useful if the server is configured to " +
  "prefer a security type that supports Unix Login/Plain authentication and " +
  "you want to override that preference for a particular connection (for " +
  "instance, to use a one-time password.)", false);

  public StringParameter password =
  new StringParameter("Password", this, false,
  "Plain-text password to use when authenticating with the VNC server.  It " +
  "is strongly recommended that this parameter be used only with a one-time " +
  "password or other disposable token.", null);

  public StringParameter passwordFile =
  new StringParameter("PasswordFile", this, false,
  "Password file from which to read the password for Standard VNC " +
  "authentication.  This is useful if your home directory is shared between " +
  "the client machine and VNC host.", null);

  private AliasParameter passwd =
  new AliasParameter("passwd", this,
  "Alias for PasswordFile", passwordFile);

  public SecTypesParameter secTypes =
  new SecTypesParameter("SecurityTypes", this, true,
  "A comma-separated list of the security types that can be used if the " +
  "server supports them.  \"VNC\" and \"None\" are the standard VNC " +
  "password and no-password authentication schemes supported by all VNC " +
  "servers.  The seven supported VeNCrypt security types (\"Plain\", " +
  "\"TLSNone\", \"TLSVnc\", \"TLSPlain\", \"X509None\", \"X509Vnc\", and " +
  "\"X509Plain\") are combinations of three encryption methods (None, " +
  "Anonymous TLS, and TLS with X.509 certificates) and three authentication " +
  "schemes (None, Standard VNC, and Plain.)  The \"UnixLogin\" security " +
  "type enables user/password authentication using the TightVNC security " +
  "extensions rather than VeNCrypt.  \"Plain\" and \"UnixLogin\" " +
  "authenticate using a plain-text username and password, so it is strongly " +
  "recommended that those types be used only with TLS encryption or SSH " +
  "tunneling.  The order of this list does not matter, since the server's " +
  "preferred order is always used.\n " +

  (Utils.getBooleanProperty("turbovnc.sessmgr", true) ?
   "When using the TurboVNC Session Manager, this parameter is effectively " +
   "set to \"VNC\" unless the SessMgrAuto parameter is disabled." : ""),
  "X509Plain,X509Vnc,X509None,TLSPlain,TLSVnc,TLSNone,VNC,Plain,UnixLogin,None");

  public BoolParameter sendLocalUsername =
  new BoolParameter("SendLocalUsername", this, true,
  "When using user/password authentication schemes (Unix Login, Plain), " +
  "authenticate using the local username rather than prompt for a " +
  "username.  As with the User parameter, setting this parameter " +
  "effectively disables any authentication schemes that don't require a " +
  "username.", false);

  public BoolParameter sessMgrAuto =
  new BoolParameter("SessMgrAuto", this, false,
  Utils.getBooleanProperty("turbovnc.sessmgr", true) ?
  "When using the TurboVNC Session Manager, automatically enable OTP " +
  "authentication and SSH tunneling.  Disabling this parameter allows any " +
  "security configuration to be used with the TurboVNC Session Manager." :
  null, true);

  public StringParameter sshConfig =
  new StringParameter("SSHConfig", this, false,
  "Path to an OpenSSH configuration file to use with the built-in SSH " +
  "client.  If specified on the command line or in a connection info file, " +
  "TurboVNC Viewer parameters take precedence over the OpenSSH " +
  "configuration file.", Utils.getHomeDir() + ".ssh/config");

  public StringParameter sshKey =
  new StringParameter("SSHKey", this, false,
  "When using the built-in SSH client with the publickey SSH authentication " +
  "method, this parameter specifies the text of an SSH private key to use " +
  "when authenticating with the SSH server.  You can use \\n within the " +
  "string to specify a new line.", null);

  public StringParameter sshKeyFile =
  new StringParameter("SSHKeyFile", this, false,
  "When using the built-in SSH client with the publickey SSH authentication " +
  "method, this parameter specifies a file that contains an SSH private key " +
  "(or keys) to use when authenticating with the SSH server.  This " +
  "parameter and the SSHKey parameter behave like the OpenSSH -i option and " +
  "IdentityFile configuration keyword.  The SSH client will attempt to use " +
  "the following private keys, in order, when authenticating with the SSH " +
  "server:\n " +

  "1. Any key specified with this parameter or the SSHKey parameter, if the " +
  "key is already provided by the SSH agent (ssh-agent or Pageant) or a " +
  "valid passphrase is supplied for the key using the SSHKeyPass " +
  "parameter\r " +
  "2. Any key specified with the IdentityFile OpenSSH configuration " +
  "keyword, if the key is already provided by the SSH agent\r " +
  "3. Any other keys provided by the SSH agent, in the order provided\r " +
  "4. Any key specified with this parameter, the SSHKey parameter, or the " +
  "IdentityFile OpenSSH configuration keyword, if a valid passphrase is not " +
  "supplied for the key\r " +
  "5. " + Utils.getHomeDir() + ".ssh/id_rsa, " + Utils.getHomeDir() +
  ".ssh/id_dsa, and " + Utils.getHomeDir() + ".ssh/id_ecdsa (in that " +
  "order), if this parameter, the SSHKey parameter, and the IdentityFile " +
  "OpenSSH configuration keyword are not specified", null);

  public StringParameter sshKeyPass =
  new StringParameter("SSHKeyPass", this, false,
  "When using the built-in SSH client with the publickey SSH authentication " +
  "method, this parameter specifies the passphrase for the SSH key(s) " +
  "specified with the SSHKey or SSHKeyFile parameter.", null);

  public IntParameter sshPort =
  new IntParameter("SSHPort", this, false,
  "When using the built-in SSH client, this parameter specifies the TCP " +
  "port on which the SSH server is listening.", 22, 0, 65535);

  public StringParameter sshUser =
  new StringParameter("SSHUser", this, true,
  "The username (default = local username) that should be used when " +
  "authenticating with the SSH server.  When using the Tunnel parameter" +
  (Utils.getBooleanProperty("turbovnc.sessmgr", true) ?
   " or the TurboVNC Session Manager, " : ", ") +
  "the SSH username can also be specified by prefixing the VNC host with " +
  "the username followed by @.  When using the Via parameter with an SSH " +
  "server, the SSH username can also be specified by prefixing the gateway " +
  "host with the username followed by @.", null);

  public BoolParameter tunnel =
  new BoolParameter("Tunnel", this, true,
  "Setting this parameter is equivalent to using the Via parameter with an " +
  "SSH gateway, except that the gateway host is assumed to be the same as " +
  "the VNC host, so you do not need to specify it separately.\n " +

  (Utils.getBooleanProperty("turbovnc.sessmgr", true) ?
   "When using the TurboVNC Session Manager, this parameter is effectively " +
   "set unless the SessMgrAuto parameter is disabled.\n " : "") +

  "This parameter is effectively set if the Server parameter specifies a " +
  "Unix domain socket connection to a remote host and the Via parameter is " +
  "not specified.", false);

  public StringParameter user =
  new StringParameter("User", this, true,
  "The username to use for Unix Login authentication (TightVNC-compatible " +
  "servers) or Plain authentication (VeNCrypt-compatible servers.)  " +
  "Specifying this parameter effectively removes any types from the value " +
  "of the SecurityTypes parameter except for \"Plain\" (and its encrypted " +
  "derivatives) and \"UnixLogin\", thus allowing only authentication " +
  "schemes that require a username.", null);

  public ServerNameParameter via =
  new ServerNameParameter("Via", this, true,
  "SSH server or UltraVNC repeater (\"gateway\") through which the VNC " +
  "connection should be tunneled.  Note that when using the Via parameter, " +
  "the VNC host should be specified from the point of view of the gateway.  " +
  "For example, specifying Via={gateway_host} Server=localhost:1 will cause " +
  "the viewer to connect to display :1 on {gateway_host} through the SSH " +
  "server running on the same host.  Similarly, specifying " +
  "Via={gateway_host}:0 Server=localhost:1 will cause the viewer to connect " +
  "to display :1 on {gateway_host} through the UltraVNC repeater running on " +
  "the same host and listening on port 5900 (VNC display :0.)  If using the " +
  "UltraVNC Repeater in \"Mode II\", specify ID:xxxx as the VNC server " +
  "name, where xxxx is the ID number of the VNC server to which you want to " +
  "connect.", null);

  public StringParameter x509ca =
  new StringParameter("X509CA", this, true,
  "X.509 Certificate Authority certificate to use with the X509* security " +
  "types.  This is used to check the validity of the server's X.509 " +
  "certificate.", Utils.getVncHomeDir() + "x509_ca.pem");

  public StringParameter x509crl =
  new StringParameter("X509CRL", this, true,
  "X.509 Certificate Revocation List to use with the X509* security types. " +
  "This is used to check the validity of the server's X.509 " +
  "certificate.", Utils.getVncHomeDir() + "x509_crl.pem");

  // CHECKSTYLE Indentation:ON

  public boolean sessMgrActive, sshTunnelActive;
  public com.jcraft.jsch.Session sshSession;
  public Socket stdioSocket;
  public String udsPath;

  // CHECKSTYLE VisibilityModifier:ON

  VoidParameter head;
  VoidParameter tail;
  LogWriter vlog = new LogWriter("Params");
}
