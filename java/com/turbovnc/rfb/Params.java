/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright 2004-2005 Cendio AB.
 * Copyright (C) 2012-2018, 2020 D. R. Commander.  All Rights Reserved.
 * Copyright (C) 2011-2012, 2016 Brian P. Hinz
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

//
// Params - class for dealing with configuration parameters.
//

package com.turbovnc.rfb;

import java.io.FileInputStream;
import java.util.*;

import com.turbovnc.rdr.*;

public final class Params {

  // - Set named parameter to value
  public static boolean set(String name, String value) {
    VoidParameter param = get(name);
    if (param == null) return false;
    if (param instanceof BoolParameter && ((BoolParameter)param).reverse) {
      ((BoolParameter)param).reverse = false;
      ((BoolParameter)param).setParam(value, true);
      return true;
    }
    return param.setParam(value);
  }

  // - Set parameter to value (separated by "=")
  public static boolean set(String arg) {
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
      return set(arg.substring(0, equal), arg.substring(equal + 1));
    } else if (hyphen) {
      VoidParameter param = get(arg);
      if (param == null) return false;
      if (param instanceof BoolParameter && ((BoolParameter)param).reverse) {
        ((BoolParameter)param).reverse = false;
        ((BoolParameter)param).setParam(false);
        return true;
      }
      return param.setParam();
    }
    return false;
  }

  // - Get named parameter
  public static VoidParameter get(String name) {
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
      current = current.next;
    }
    return null;
  }

  public static void list(int width) {
    VoidParameter current = head;

    while (current != null) {
      String desc = current.getDescription();
      if (desc == null) {
        current = current.next;
        continue;
      }
      desc = desc.trim();
      if (current instanceof HeaderParameter) {
        System.out.println(desc);
        for (int i = 0; i < desc.length(); i++)
          System.out.print("-");
        current = current.next;
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
        System.out.format(" %" + wordLen + "s", desc.substring(0, wordLen));
        column += wordLen + 1;
        if (wordLen >= 1 && desc.charAt(wordLen - 1) == '\n') {
          System.out.print("\n   ");
          column = 4;
        }

        if (s == -1) break;
        desc = desc.substring(wordLen + 1);
      }
      current = current.next;
      System.out.print("\n\n");
    }
  }

  public static void load(String filename) {
    if (filename == null)
      return;

    /* Read parameters from file */
    Properties props = new Properties();
    try {
      props.load(new FileInputStream(filename));
    } catch (java.security.AccessControlException e) {
      throw new WarningException("Cannot access connection info file:\n" +
                                 e.getMessage());
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
        set("Server", props.getProperty(name));
      } else if (name.equalsIgnoreCase("port")) {
        set("Port", props.getProperty(name));
      } else if (name.equalsIgnoreCase("password")) {
        byte[] encryptedPassword = new byte[8];
        String passwordString = props.getProperty(name);
        if (passwordString.length() != 16)
          throw new ErrorException("Password stored in connection info file is invalid.");
        for (int c = 0; c < 16; c += 2) {
          int temp = -1;
          try {
            temp = Integer.parseInt(passwordString.substring(c, c + 2), 16);
          } catch (NumberFormatException e) {}
          if (temp >= 0)
            encryptedPassword[c / 2] = (byte)temp;
          else break;
        }
        set("Password", VncAuth.unobfuscatePasswd(encryptedPassword));
      } else if (name.equalsIgnoreCase("user")) {
        set("User", props.getProperty(name));
      } else if (name.equalsIgnoreCase("preferred_encoding")) {
        int encoding = -1;
        try {
          encoding = Integer.parseInt(props.getProperty(name));
        } catch (NumberFormatException e) {}
        if (encoding >= 0 && encoding <= RFB.ENCODING_LAST)
          set("Encoding", RFB.encodingName(encoding));
      } else if (name.equalsIgnoreCase("restricted")) {
        set("Restricted", props.getProperty(name));
      } else if (name.equalsIgnoreCase("viewonly")) {
        set("ViewOnly", props.getProperty(name));
      } else if (name.equalsIgnoreCase("reversescroll")) {
        set("ReverseScroll", props.getProperty(name));
      } else if (name.equalsIgnoreCase("fullscreen")) {
        set("FullScreen", props.getProperty(name));
      } else if (name.equalsIgnoreCase("fsaltenter")) {
        set("FSAltEnter", props.getProperty(name));
      } else if (name.equalsIgnoreCase("grabkeyboard")) {
        int grabKeyboardValue = -1;
        try {
          grabKeyboardValue = Integer.parseInt(props.getProperty(name));
        } catch (NumberFormatException e) {}
        switch (grabKeyboardValue) {
          case Options.GRAB_FS:
            set("GrabKeyboard", "FS");  break;
          case Options.GRAB_ALWAYS:
            set("GrabKeyboard", "Always");  break;
          case Options.GRAB_MANUAL:
            set("GrabKeyboard", "Manual");  break;
        }
      } else if (name.equalsIgnoreCase("span")) {
        int spanValue = -1;
        try {
          spanValue = Integer.parseInt(props.getProperty(name));
        } catch (NumberFormatException e) {}
        if (spanValue == 0) set("Span", "Primary");
        else if (spanValue == 1) set("Span", "All");
        else if (spanValue == 2) set("Span", "Auto");
      } else if (name.equalsIgnoreCase("8bit")) {
        int _8bit = -1;
        try {
          _8bit = Integer.parseInt(props.getProperty(name));
        } catch (NumberFormatException e) {}
        if (_8bit >= 1)
          set("Colors", "256");
        else if (_8bit == 0)
          set("Colors", "-1");
      } else if (name.equalsIgnoreCase("shared")) {
        set("Shared", props.getProperty(name));
      } else if (name.equalsIgnoreCase("disableclipboard")) {
        int disableclipboard = -1;
        try {
          disableclipboard = Integer.parseInt(props.getProperty(name));
        } catch (NumberFormatException e) {}
        if (disableclipboard >= 1) {
          set("RecvClipboard", "0");
          set("SendClipboard", "0");
        } else if (disableclipboard == 0) {
          set("RecvClipboard", "1");
          set("SendClipboard", "1");
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
      } else if (name.equalsIgnoreCase("cursorshape")) {
        set("CursorShape", props.getProperty(name));
      } else if (name.equalsIgnoreCase("noremotecursor")) {
        set("LocalCursor", props.getProperty(name));
      } else if (name.equalsIgnoreCase("compresslevel")) {
        set("CompressLevel", props.getProperty(name));
      } else if (name.equalsIgnoreCase("subsampling")) {
        int subsamplingValue = -1;
        try {
          subsamplingValue = Integer.parseInt(props.getProperty(name));
        } catch (NumberFormatException e) {}
        switch (subsamplingValue) {
          case Options.SUBSAMP_NONE:  set("Subsampling", "1X");  break;
          case Options.SUBSAMP_4X:    set("Subsampling", "4X");  break;
          case Options.SUBSAMP_2X:    set("Subsampling", "2X");  break;
          case Options.SUBSAMP_GRAY:  set("Subsampling", "Gray");  break;
        }
      } else if (name.equalsIgnoreCase("quality")) {
        int qualityValue = -2;
        try {
          qualityValue = Integer.parseInt(props.getProperty(name));
        } catch (NumberFormatException e) {}
        if (qualityValue == -1) set("JPEG", "0");
        else if (qualityValue >= 1 && qualityValue <= 100) {
          set("Quality", props.getProperty(name));
        }
      } else if (name.equalsIgnoreCase("continuousupdates")) {
        set("CU", props.getProperty(name));
      } else if (name.equalsIgnoreCase("nounixlogin")) {
        set("NoUnixLogin", props.getProperty(name));
      }
    }

    if ((scaleNum >= 1 || scaleDenom >= 1) && fitWindow < 1) {
      if (scaleNum < 1) scaleNum = 1;
      if (scaleDenom < 1) scaleDenom = 1;
      int scalingFactorValue = scaleNum * 100 / scaleDenom;
      set("Scale", Integer.toString(scalingFactorValue));
    } else if (fitWindow >= 1) {
      set("Scale", "FixedRatio");
    }

    if (desktopSizeStr != null)
      set("DesktopSize", desktopSizeStr);
    else {
      switch (resizeMode) {
        case Options.SIZE_SERVER:
          set("DesktopSize", "Server");  break;
        case Options.SIZE_MANUAL:
          if (desktopWidth > 0 && desktopHeight > 0)
            set("DesktopSize", desktopWidth + "x" + desktopHeight);
          break;
        case Options.SIZE_AUTO:
          set("DesktopSize", "Auto");  break;
      }
    }
  }

  public static void reset() {
    VoidParameter current = head;

    while (current != null) {
      if (!(current instanceof HeaderParameter))
        current.reset();
      current = current.next;
    }
  }

  // CHECKSTYLE Indentation:OFF
  // CHECKSTYLE VisibilityModifier:OFF

  // CONNECTION PARAMETERS

  private static HeaderParameter connHeader =
  new HeaderParameter("ConnHeader", "CONNECTION PARAMETERS");

  public static BoolParameter alwaysShowConnectionDialog =
  new BoolParameter("AlwaysShowConnectionDialog",
  "Always show the \"New TurboVNC Connection\" dialog even if the server " +
  "has been specified on the command line.", false);

  public static BoolParameter clientRedirect =
  new BoolParameter("ClientRedirect", null, false);

  public static StringParameter config =
  new StringParameter("Config",
  "File from which to read connection information.  This file can be " +
  "generated by selecting \"Save connection info as...\" in the system menu " +
  "of the Windows TurboVNC Viewer.  Connection info files can also be " +
  "dragged & dropped onto the TurboVNC Viewer icon in order to initiate a " +
  "new connection.", null);

  public static BoolParameter copyRect =
  new BoolParameter("CopyRect", null, true);

  public static BoolParameter continuousUpdates =
  new BoolParameter("CU", null, true);

  public static BoolParameter listenMode =
  new BoolParameter("Listen",
  "Start the viewer in \"listen mode.\"  The viewer will listen on port " +
  "5500 (or on the port specified by the Port parameter) for reverse " +
  "connections from a VNC server.  To connect a TurboVNC session to a " +
  "listening viewer, use the vncconnect program on the TurboVNC host.", false);

  public static IntParameter maxClipboard =
  new IntParameter("MaxClipboard",
  "Maximum permitted length of an outgoing clipboard update (in bytes)",
  1048576);

  public static BoolParameter noNewConn =
  new BoolParameter("NoNewConn",
  "Always exit after the first connection closes, and do not allow new " +
  "connections to be made without restarting the viewer.  This is useful in " +
  "portal environments that need to control when and how the viewer is " +
  "launched.  Setting this parameter also disables the \"Close Connection\" " +
  "option in the F8 menu and the \"Disconnect\" button in the toolbar.",
  false);

  public static BoolParameter noReconnect =
  new BoolParameter("NoReconnect",
  "Normally, if the viewer is disconnected from the server unexpectedly, " +
  "the viewer will ask whether you want to reconnect.  Setting this " +
  "parameter disables that behavior.", false);

  public static IntParameter vncServerPort =
  new IntParameter("Port",
  "The TCP port number on which the VNC server is listening.  For Un*x VNC " +
  "servers, this is typically 5900 + the X display number of the VNC " +
  "session (example: 5901 if connecting to display :1.)  For Windows and " +
  "Mac VNC servers, this is typically 5900." +
  (Utils.getBooleanProperty("turbovnc.sessmgr", true) ? "" :
   "  (default = 5900)") + "\n " +
  "If listen mode is enabled, this parameter specifies the TCP port on " +
  "which the viewer will listen for reverse connections from a VNC server.  " +
  "(default = 5500)", -1);

  public static IntParameter profileInt =
  new IntParameter("ProfileInterval",
  "TurboVNC includes an internal profiling system that can be used to " +
  "display performance statistics about the connection, such as how many " +
  "updates per second are being received and how much network bandwidth is " +
  "being used.  Profiling is activated by selecting \"Performance Info...\" " +
  "in the F8 menu, which pops up a dialog that displays the statistics.  " +
  "Profiling can also be enabled on the console only by setting the " +
  "environment variable TVNC_PROFILE to 1.  The ProfileInterval parameter " +
  "specifies how often (in seconds) that the performance statistics are " +
  "updated in the dialog or on the console.  The statistics are averaged " +
  "over this interval.", 5);

  public static BoolParameter recvClipboard =
  new BoolParameter("RecvClipboard",
  "Synchronize the local clipboard with the clipboard of the TurboVNC " +
  "session whenever the latter changes.", true);

  public static BoolParameter sendClipboard =
  new BoolParameter("SendClipboard",
  "Synchronize the TurboVNC session clipboard with the local clipboard " +
  "whenever the latter changes.", true);

  public static StringParameter vncServerName =
  new StringParameter("Server",
  "The VNC server to which to connect.  This can be specified in the " +
  "format {host}[:{display_number}], {host}::{port} or" +
  " {host}:/{unix_socket_path}, where {host} is the " +
  "host name or IP address of the machine on which the VNC server is " +
  "running (the \"VNC host\"), {display_number} is an optional X display " +
  (Utils.getBooleanProperty("turbovnc.sessmgr", true) ?
   "number, and {port} is a TCP port.  If no port or display number is " +
   "specified, then the viewer will enable the TurboVNC Session Manager, " +
   "which allows you to remotely start a new TurboVNC session or to choose " +
   "an existing TurboVNC session to which to connect." :
   "number (default: 0), and {port} is a TCP port."), null);

  public static BoolParameter shared =
  new BoolParameter("Shared",
  "Request a shared VNC session.  When the session is shared, other users " +
  "can connect to the session (assuming they have the correct " +
  "authentication credentials) and collaborate with the user who started " +
  "the session.  If this parameter is disabled and the TurboVNC session is " +
  "using default settings, then you will only be able to connect to the " +
  "session if no one else is already connected.", true);

  // INPUT PARAMETERS

  private static HeaderParameter inputHeader =
  new HeaderParameter("InputHeader", "INPUT PARAMETERS");

  public static BoolParameter fsAltEnter =
  new BoolParameter("FSAltEnter",
  "Normally, the viewer will switch into and out of full-screen mode when " +
  "Ctrl-Alt-Shift-F is pressed or \"Full screen\" is selected from the " +
  "popup menu.  Setting this parameter will additionally cause the viewer " +
  "to switch into and out of full-screen mode when Alt-Enter is pressed.",
  false);

  public static StringParameter grabKeyboard =
  new StringParameter("GrabKeyboard",
  Utils.osGrab() ? "When the keyboard is grabbed, special key sequences " +
  "(such as Alt-Tab) that are used to switch windows and perform other " +
  "window management functions are passed to the VNC server instead of " +
  "being handled by the local window manager.  The default behavior " +
  "(\"FS\") is to automatically grab the keyboard in full-screen mode and " +
  "ungrab it in windowed mode.  When this parameter is set to \"Always\", " +
  "the keyboard is automatically grabbed in both full-screen mode and " +
  "windowed mode.  When this parameter is set to \"Manual\", the keyboard " +
  "is only grabbed or ungrabbed when the \"Grab Keyboard\" option is " +
  "selected in the F8 menu, or when the Ctrl-Alt-Shift-G hotkey is " +
  "pressed.  Regardless of the grabbing mode, the F8 menu option and hotkey " +
  "can always be used to grab or ungrab the keyboard." : null, "FS",
  "Always, FS, Manual");

  public static BoolParameter grabPointer =
  new BoolParameter("GrabPointer",
  Utils.isX11() ? "If this parameter is set, then the pointer will be " +
  "grabbed whenever the keyboard is grabbed.  This allows certain keyboard " +
  "+ pointer sequences, such as Alt-{drag}, to be passed to the server.  " +
  "The downside, however, is that grabbing the pointer prevents any " +
  "interaction with the local window manager whatsoever (for instance, the " +
  "window can no longer be maximized or closed, and you cannot switch to " +
  "other running applications.)  Thus, this parameter is primarily useful " +
  "in conjunction with GrabKeyboard=FS." : null, true);

  public static StringParameter menuKey =
  new StringParameter("MenuKey",
  "The key used to display the popup menu", "F8",
  MenuKey.getMenuKeyValueStr());

  // Prevent the viewer from sending Ctrl-Alt-Del and Ctrl-Esc to the server
  public static BoolParameter restricted =
  new BoolParameter("Restricted", null, false);

  public static BoolParameter reverseScroll =
  new BoolParameter("ReverseScroll",
  "Reverse the direction of mouse scroll wheel events that are sent to the " +
  "VNC server.  This is useful when connecting from clients that have " +
  "\"natural scrolling\" enabled.", false);

  public static BoolParameter viewOnly =
  new BoolParameter("ViewOnly",
  "Ignore all keyboard and mouse events in the viewer window and do not " +
  "pass those events to the VNC server.", false);

  // Set to 0 to disable the view-only checkbox in the Options dialog
  public static BoolParameter viewOnlyControl =
  new BoolParameter("ViewOnlyControl", null, true);

  // DISPLAY PARAMETERS

  private static HeaderParameter displayHeader =
  new HeaderParameter("DisplayHeader", "DISPLAY PARAMETERS");

  public static BoolParameter acceptBell =
  new BoolParameter("AcceptBell",
  "Produce a system beep when a \"bell\" event is received from the VNC " +
  "server.", true);

  public static IntParameter colors =
  new IntParameter("Colors",
  "The color depth to use for the viewer's window.  Setting this parameter " +
  "to 8 specifies a BGR111 pixel format (1 bit for each red, green, and " +
  "blue component), 64 specifies a BGR222 pixel format, 256 specifies a " +
  "BGR233 pixel format, 32768 specifies a BGR555 pixel format, and 65536 " +
  "specifies a BGR565 pixel format.  Lowering the color depth can " +
  "significantly reduce bandwidth when using encoding types other than " +
  "Tight or when using Tight encoding without JPEG.  However, colors will " +
  "not be represented accurately, and CPU usage will increase substantially " +
  "(causing a corresponding decrease in performance on fast networks.)  The " +
  "default is to use the native color depth of the display on which the " +
  "viewer is running, which is usually true color (8 bits per component.)",
  -1);

  public static BoolParameter compatibleGUI =
  new BoolParameter("CompatibleGUI",
  "Normally, the TurboVNC Viewer GUI exposes only the settings that are " +
  "useful for TurboVNC servers.  Setting this parameter changes the " +
  "compression level slider such that it can be used to select any " +
  "compression level from 0-9, which is useful when connecting to other " +
  "types of VNC servers.  This parameter is effectively set when using any " +
  "encoding type other than Tight or when selecting a compression level " +
  "that the GUI normally does not expose.", false);

  public static BoolParameter currentMonitorIsPrimary =
  new BoolParameter("CurrentMonitorIsPrimary",
  "If this parameter is set, then the monitor that contains the largest " +
  "number of pixels from the viewer window will be treated as the primary " +
  "monitor for the purposes of spanning.  Otherwise, the left-most and " +
  "top-most monitor will always be the primary monitor (as was the case in " +
  "TurboVNC 2.0 and prior versions.)", true);

  public static BoolParameter cursorShape =
  new BoolParameter("CursorShape",
  "Normally, the TurboVNC Server and compatible VNC servers will send only " +
  "changes to the remote mouse cursor's shape and position.  This results " +
  "in the best mouse responsiveness.  Disabling this parameter causes the " +
  "server to instead render the mouse cursor and send it to the viewer as " +
  "an image every time the cursor moves or changes shape.  Thus, using a " +
  "remotely rendered cursor can increase network \"chatter\" between host " +
  "and client significantly, which may cause performance problems on slow " +
  "networks.", true);

  public static StringParameter desktopSize =
  new StringParameter("DesktopSize",
  "If the VNC server supports remote desktop resizing, then attempt to " +
  "resize the remote desktop to the specified size (example: 1920x1200) or " +
  "reconfigure the server's virtual screens with a specified layout " +
  "(example: 1920x1200+0+0,1920x1200+1920+0).  Setting this parameter to " +
  "\"Auto\" causes the remote desktop to be resized to fit in the viewer " +
  "window without using scrollbars, and it causes the server's virtual " +
  "screens to be reconfigured such that their screen boundaries align with " +
  "the client's screen boundaries when the viewer window is in its default " +
  "position (this is the default behavior.)  Setting this parameter to " +
  "\"Server\" or \"0\" disables remote desktop resizing and uses the " +
  "desktop size and screen configuration set by the server.", "Auto",
  "WxH, W0xH0+X0+Y0[,W1xH1+X1+Y1,...], Auto, or Server");

  public static BoolParameter fullScreen =
  new BoolParameter("FullScreen",
  "Start the viewer in full-screen mode.", false);

  public static BoolParameter localCursor =
  new BoolParameter("LocalCursor",
  "The default behavior of the TurboVNC Viewer is to hide the local cursor " +
  "and show only the remote cursor, which can be rendered either by the " +
  "VNC server or on the client, depending on the value of the CursorShape " +
  "parameter.  However, certain (broken) VNC server implementations do not " +
  "support either method of remote cursor rendering, so this parameter is " +
  "provided as a workaround for connecting to such servers.  If this " +
  "parameter is set, then any cursor shape updates from the server are " +
  "ignored, and the local cursor is always displayed.", false);

  public static StringParameter scalingFactor =
  new StringParameter("Scale",
  "Reduce or enlarge the remote desktop image.  The value is interpreted as " +
  "a scaling factor in percent.  The default value of 100% corresponds to " +
  "the original remote desktop size.  Values below 100 reduce the image " +
  "size, whereas values above 100 enlarge the image proportionally.  If " +
  "this parameter is set to \"Auto\", then automatic scaling is performed.  " +
  "Automatic scaling reduces or enlarges the remote desktop image such that " +
  "the entire image will fit in the viewer window without using " +
  "scrollbars.  If this parameter is set to \"FixedRatio\", then automatic " +
  "scaling is performed, but the original aspect ratio is preserved.  " +
  "Enabling scaling disables automatic desktop resizing.", "100",
  "1-1000, Auto, or FixedRatio");

  public static StringParameter span =
  new StringParameter("Span",
  "This parameter specifies whether the viewer window should span only the " +
  "primary monitor (\"Primary\"), all monitors (\"All\"), or all monitors " +
  "only if the window cannot fit on the primary monitor (\"Auto\".)  When " +
  "using automatic desktop resizing, \"Auto\" has the same effect as " +
  "\"Primary\" when in windowed mode and the same effect as \"All\" when in " +
  "full-screen mode.  Due to general issues with spanning windows across " +
  "multiple monitors in X11, this parameter does not work on Un*x/X11 " +
  "platforms except in full-screen mode, and it requires the TurboVNC " +
  "Helper library.", "Auto", "Primary, All, Auto");

  public static BoolParameter showToolbar =
  new BoolParameter("Toolbar",
  "Show the toolbar by default.", true);

  // ENCODING PARAMETERS

  private static HeaderParameter encHeader =
  new HeaderParameter("EncHeader", "ENCODING PARAMETERS");

  public static IntParameter compressLevel =
  new IntParameter("CompressLevel",
  "When Tight encoding is used, the compression level specifies the amount " +
  "of zlib compression to apply to subrectangles encoded using the indexed " +
  "color, mono, and raw subencoding types.  If the JPEG subencoding type is " +
  "enabled, then the compression level also defines the \"palette " +
  "threshold\", or the minimum number of colors that a subrectangle must " +
  "have before it is encoded using JPEG.  Higher compression levels have " +
  "higher palette thresholds and thus favor the use of indexed color " +
  "subencoding, whereas lower compression levels favor the use of JPEG.\n " +

  "Compression Level 1 is usually the default whenever JPEG is enabled, " +
  "because extensive experimentation has revealed little or no benefit to " +
  "using higher compression levels with most 3D and video workloads.  " +
  "However, v1.1 and later of the TurboVNC Server also supports Compression " +
  "Level 2 when JPEG is enabled.  Compression Level 2 can be shown to " +
  "reduce the bandwidth of certain types of low-color workloads by " +
  "typically 20-40% (with a commensurate increase in CPU usage.)\n " +

  "In v1.2 or later of the TurboVNC Server, compression levels 5-7 map to " +
  "compression levels 0-2, but they also enable the interframe comparison " +
  "engine in the server.  Interframe comparison maintains a copy of the " +
  "remote framebuffer for each connected viewer and compares each " +
  "framebuffer update with the copy to ensure that redundant updates are " +
  "not sent to the viewer.  This prevents unnecessary network traffic if an " +
  "ill-behaved application draws the same thing over and over again, but " +
  "interframe comparison also causes the TurboVNC Server to use more CPU " +
  "time and much more memory.", 1, 0, 9);

  public static StringParameter preferredEncoding =
  new StringParameter("Encoding",
  "Preferred encoding type to use.  If the server does not support the " +
  "preferred encoding type, then the next best one will be chosen.  There " +
  "should be no reason to use an encoding type other than Tight when " +
  "connecting to a TurboVNC session, but this parameter can be useful when " +
  "connecting to other types of VNC servers, such as RealVNC.",
  "Tight", "Tight, ZRLE, Hextile, Raw");

  public static BoolParameter allowJpeg =
  new BoolParameter("JPEG",
  "Enable the JPEG subencoding type when using Tight encoding.  This causes " +
  "the Tight encoder to use JPEG compression for subrectangles that have a " +
  "high number of unique colors and indexed color subencoding for " +
  "subrectangles that have a low number of unique colors.  If this " +
  "parameter is disabled, then the Tight encoder will select between " +
  "indexed color or raw subencoding, depending on the size of the " +
  "subrectangle and its color count.", true);

  public static IntParameter quality =
  new IntParameter("Quality",
  "Specifies the JPEG quality to use when compressing JPEG images with the " +
  "Tight+JPEG encoding methods.  Lower quality values produce grainier JPEG " +
  "images with more noticeable compression artifacts, but lower quality " +
  "values also use less network bandwidth and CPU time.  The default value " +
  "of " + Options.DEFQUAL + " should be perceptually lossless (that is, any " +
  "image compression artifacts it produces should be imperceptible to the " +
  "human eye under most viewing conditions.)", Options.DEFQUAL, 1, 100);

  public static StringParameter subsampling =
  new StringParameter("Subsampling",
  "When compressing an image using JPEG, the RGB pixels are first converted " +
  "to the YCbCr colorspace, a colorspace in which each pixel is represented " +
  "as a brightness (Y, or \"luminance\") value and a pair of color (Cb and " +
  "Cr, or \"chrominance\") values.  After this colorspace conversion, " +
  "chrominance subsampling can be used to discard some of the chrominance " +
  "components in order to save bandwidth.  1X subsampling retains the " +
  "chrominance components for all pixels, and thus it provides the best " +
  "image quality but also uses the most network bandwidth and CPU time.  " +
  "2X subsampling retains the chrominance components for every other pixel, " +
  "and 4X subsampling retains the chrominance components for every fourth " +
  "pixel (this is typically implemented as 2X subsampling in both X and Y " +
  "directions.)  Grayscale throws out all of the chrominance components, " +
  "leaving only luminance.  2X and 4X subsampling will typically produce " +
  "noticeable aliasing of lines and other sharp features, but with " +
  "photographic or other \"smooth\" image content, it may be difficult to " +
  "detect any difference between 1X, 2X, and 4X.", "1X", "1X, 2X, 4X, Gray");

  private static AliasParameter samp =
  new AliasParameter("Samp",
  "Alias for Subsampling", subsampling);

  // SECURITY AND AUTHENTICATION PARAMETERS

  private static HeaderParameter secHeader =
  new HeaderParameter("SecHeader", "SECURITY AND AUTHENTICATION PARAMETERS");

  public static BoolParameter autoPass =
  new BoolParameter("AutoPass",
  "Read a plain-text password from stdin and use this password when " +
  "authenticating with the VNC server.  It is strongly recommended that " +
  "this parameter be used only in conjunction with a one-time password or " +
  "other disposable token.", false);

  public static StringParameter encPassword =
  new StringParameter("EncPassword",
  "Encrypted password to use when authenticating with the VNC server.  The " +
  "encrypted password should be in the same ASCII hex format used by " +
  "TurboVNC connection info (.vnc) files.  For instance, you can generate " +
  "an ASCII hex VNC password on the TurboVNC host by executing\n " +
  "'cat {VNC_password_file} | xxd -c 256 -ps' or\n " +
  "'echo {unencrypted_password} | /opt/TurboVNC/bin/vncpasswd -f | xxd -c 256 -ps'\n " +
  "This parameter allows a password to be supplied to the TurboVNC Viewer " +
  "without exposing the password as plain text.  However, the encryption " +
  "scheme (DES) used for VNC passwords is not particularly strong, so " +
  "encrypting the password guards against only the most casual of attacks.  " +
  "It is thus recommended that this parameter be used only in conjunction " +
  "with a one-time password or other disposable token.", null);

  public static BoolParameter extSSH =
  new BoolParameter("ExtSSH",
  "Use an external SSH client instead of the built-in SSH client.  The " +
  "external client defaults to /usr/bin/ssh on Un*x and Mac systems and " +
  "ssh.exe on Windows systems, but you can use the VNC_VIA_CMD and " +
  "VNC_TUNNEL_CMD environment variables or the turbovnc.via and " +
  "turbovnc.tunnel system properties to specify the exact command line to " +
  "use when creating the tunnel.  If one of those environment variables or " +
  "system properties is set, then an external SSH client is automatically " +
  "used.  See the TurboVNC User's Guide for more details.", false);

  public static BoolParameter localUsernameLC =
  new BoolParameter("LocalUsernameLC",
  "When the SendLocalUsername parameter is set, setting this parameter will " +
  "cause the local user name to be sent in lowercase, which may be useful " +
  "when using the viewer on Windows machines (Windows allows mixed-case " +
  "user names, whereas Un*x and Mac platforms generally don't.)", false);

  public static BoolParameter noUnixLogin =
  new BoolParameter("NoUnixLogin",
  "This disables the use of Unix Login authentication when connecting to " +
  "TightVNC-compatible servers and Plain authentication when connecting to " +
  "VeNCrypt-compatible servers.  Setting this parameter has the effect of " +
  "removing \"Plain\" (and its encrypted derivatives) and \"UnixLogin\" " +
  "from the SecurityTypes parameter.  This is useful if the server is " +
  "configured to prefer a security type that supports Unix Login/Plain " +
  "authentication and you want to override that preference for a particular " +
  "connection (for instance, to use a one-time password.)",
  false);

  public static StringParameter password =
  new StringParameter("Password",
  "Plain-text password to use when authenticating with the VNC server.  It " +
  "is strongly recommended that this parameter be used only in conjunction " +
  "with a one-time password or other disposable token.", null);

  public static StringParameter passwordFile =
  new StringParameter("PasswordFile",
  "Password file from which to read the password for Standard VNC " +
  "authentication.  This is useful if your home directory is shared between " +
  "the client machine and VNC host.", null);

  private static AliasParameter passwd =
  new AliasParameter("passwd",
  "Alias for PasswordFile", passwordFile);

  public static StringParameter secTypes =
  new StringParameter("SecurityTypes",
  "A comma-separated list of the security types that can be used, if the " +
  "server supports them.  \"VNC\" and \"None\" are the standard VNC " +
  "password and no-password authentication schemes supported by all VNC " +
  "servers.  The ten supported VeNCrypt security types (\"Plain\", " +
  "\"Ident\", \"TLSNone\", \"TLSVnc\", \"TLSPlain\", \"TLSIdent\", " +
  "\"X509None\", \"X509Vnc\", \"X509Plain\", and \"X509Ident\") are " +
  "combinations of three encryption methods (None, Anonymous TLS, and TLS " +
  "with X.509 certificates) and four authentication schemes (None, Standard " +
  "VNC, Plain, and Ident.)  The \"UnixLogin\" security type enables " +
  "user/password authentication using the TightVNC security extensions " +
  "rather than VeNCrypt.  \"Plain\" and \"UnixLogin\" authenticate using a " +
  "plain-text user name and password, so it is strongly recommended that " +
  "those types only be used with either TLS encryption or SSH tunneling.  " +
  "\"Ident\", which is designed for use by VNC proxies, authenticates using " +
  "only a user name.  The order of this list does not matter, since the " +
  "server's preferred order is always used.\n " +

  "When using the TurboVNC Session Manager, this parameter is effectively " +
  "set to \"VNC\" unless the SessMgrAuto parameter is disabled.",
  "X509Plain,X509Ident,X509Vnc,X509None,TLSPlain,TLSIdent,TLSVnc,TLSNone,VNC,Ident,Plain,UnixLogin,None");

  public static BoolParameter sendLocalUsername =
  new BoolParameter("SendLocalUsername",
  "Send the local user name when using user/password authentication schemes " +
  "(Unix Login, Plain, Ident) rather than prompting for it.  As with the " +
  "User parameter, setting this parameter has the effect of disabling any " +
  "authentication schemes that don't require a user name.", false);

  public static BoolParameter sessMgrAuto =
  new BoolParameter("SessMgrAuto",
  "When using the TurboVNC Session Manager, the default behavior is to " +
  "automatically enable OTP authentication and SSH tunneling.  Disabling " +
  "this parameter overrides that behavior and allows any security " +
  "configuration to be used.", true);

  public static StringParameter sshConfig =
  new StringParameter("SSHConfig",
  "When using the Via or Tunnel parameters with the built-in SSH client, " +
  "this parameter specifies the path to an OpenSSH configuration file to " +
  "use when authenticating with the SSH server.  The OpenSSH configuration " +
  "file takes precedence over any TurboVNC Viewer parameters.",
  Utils.getHomeDir() + ".ssh/config");

  public static StringParameter sshKey =
  new StringParameter("SSHKey",
  "When using the Via or Tunnel parameters with the built-in SSH client, or " +
  "when using the TurboVNC Session Manager, this parameter specifies the " +
  "text of the SSH private key to use when authenticating with the SSH " +
  "server.  You can use \\n within the string to specify a new line.", null);

  public static StringParameter sshKeyFile =
  new StringParameter("SSHKeyFile",
  "When using the Via or Tunnel parameters with the built-in SSH client, or " +
  "when using the TurboVNC Session Manager, this parameter specifies a file " +
  "that contains an SSH private key (or keys) to use when authenticating " +
  "with the SSH server.  If not specified, then the built-in SSH client " +
  "will attempt to read private keys from ~/.ssh/id_dsa and ~/.ssh/id_rsa.  " +
  "It will fall back to asking for an SSH password if private key " +
  "authentication fails.", null);

  public static StringParameter sshKeyPass =
  new StringParameter("SSHKeyPass",
  "When using the Via or Tunnel parameters with the built-in SSH client, or " +
  "when using the TurboVNC Session Manager, this parameter specifies the " +
  "passphrase for the SSH key.", null);

  public static IntParameter sshPort =
  new IntParameter("SSHPort",
  "When using the Via or Tunnel parameters with the built-in SSH client, or " +
  "when using the TurboVNC Session Manager, this parameter specifies the " +
  "TCP port on which the SSH server is listening.", 22);

  public static BoolParameter tunnel =
  new BoolParameter("Tunnel",
  "Setting this parameter is equivalent to using the Via parameter with an " +
  "SSH gateway, except that the gateway host is assumed to be the same as " +
  "the VNC host, so you do not need to specify it separately.  When using " +
  "the Tunnel parameter, the VNC host can be prefixed with {user}@ to " +
  "indicate that user name {user} (default = local user name) should be " +
  "used when authenticating with the SSH server. This parameter is implied " +
  "if a connection to a unix socket path with a non-localhost server name is " +
  "used and the Via parameter is not set.\n " +

  "When using the TurboVNC Session Manager, this parameter is effectively " +
  "set unless the SessMgrAuto parameter is disabled.", false);

  public static StringParameter user =
  new StringParameter("User",
  "The user name to use for Unix Login authentication (TightVNC-compatible " +
  "servers) or for Plain and Ident authentication (VeNCrypt-compatible " +
  "servers.)  Specifying this parameter has the effect of removing any " +
  "types from the SecurityTypes parameter except for \"Plain\" and " +
  "\"Ident\" (and their encrypted derivatives) and \"UnixLogin\", thus " +
  "allowing only authentication schemes that require a user name.", null);

  public static StringParameter via =
  new StringParameter("Via",
  "This parameter specifies an SSH server or UltraVNC repeater " +
  "(\"gateway\") through which the VNC connection should be tunneled.  Note " +
  "that when using the Via parameter, the VNC host should be specified from " +
  "the point of view of the gateway.  For example, specifying " +
  "Via={gateway_host} Server=localhost:1 will cause the viewer to connect " +
  "to display :1 on {gateway_host} through the SSH server running on that " +
  "same host.  Similarly, specifying Via={gateway_host}:0 " +
  "Server=localhost:1 will cause the viewer to connect to display :1 on " +
  "{gateway_host} through the UltraVNC repeater running on that same host " +
  "and listening on port 5900 (VNC display :0.)  If using the UltraVNC " +
  "Repeater in \"Mode II\", then specify ID:xxxx as the VNC server name, " +
  "where xxxx is the ID number of the VNC server to which you want to " +
  "connect.  If using an SSH server, then the gateway host can be prefixed " +
  "with {user}@ to indicate that user name {user} (default = local user " +
  "name) should be used when authenticating with the SSH server.", null);

  public static StringParameter x509ca =
  new StringParameter("X509CA",
  "X.509 Certificate Authority certificate to use with the X509* security " +
  "types.  This is used to check the validity of the server's X.509 " +
  "certificate.", Utils.getVncHomeDir() + "x509_ca.pem");

  public static StringParameter x509crl =
  new StringParameter("X509CRL",
  "X.509 Certificate Revocation List to use with the X509* security types. " +
  "This is used to check the validity of the server's X.509 " +
  "certificate.", Utils.getVncHomeDir() + "x509_crl.pem");

  // CHECKSTYLE Indentation:ON
  // CHECKSTYLE VisibilityModifier:ON

  private Params() {}
  static VoidParameter head;
  static VoidParameter tail;
  static LogWriter vlog = new LogWriter("Params");
}
