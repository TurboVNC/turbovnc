/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright 2004-2005 Cendio AB.
 * Copyright (C) 2012-2013, 2015, 2017-2018 D. R. Commander.
 *                                          All Rights Reserved.
 * Copyright 2012 Brian P. Hinz
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
// Configuration - class for dealing with configuration parameters.
//

package com.turbovnc.rfb;

import com.turbovnc.rdr.*;
import java.io.FileInputStream;
import java.util.*;

public final class Configuration {

  // - Set named parameter to value
  public static boolean setParam(String name, String value) {
    VoidParameter param = getParam(name);
    if (param == null) return false;
    if (param instanceof BoolParameter && ((BoolParameter)param).reverse) {
      ((BoolParameter)param).reverse = false;
      ((BoolParameter)param).setParam(value, true);
      return true;
    }
    return param.setParam(value);
  }

  // - Set parameter to value (separated by "=")
  public static boolean setParam(String config) {
    boolean hyphen = false;
    if (config.charAt(0) == '-' && config.length() > 1) {
      hyphen = true;
      if (config.charAt(1) == '-')
        config = config.substring(2);  // allow gnu-style --<option>
      else
        config = config.substring(1);
    }
    int equal = config.indexOf('=');
    if (equal != -1) {
      return setParam(config.substring(0, equal), config.substring(equal + 1));
    } else if (hyphen) {
      VoidParameter param = getParam(config);
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
  public static VoidParameter getParam(String name) {
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

  public static void listParams(int width) {
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
    } catch (java.lang.Exception e) {
      throw new WarningException("Cannot open connection info file:\n" +
                                 e.getMessage());
    }

    int scaleNum = -1, scaleDenom = -1, fitWindow = -1;
    int resizeMode = -1, desktopWidth = -1, desktopHeight = -1;
    String desktopSize = null;

    for (Enumeration<?> i = props.propertyNames();  i.hasMoreElements();) {
      String name = (String)i.nextElement();

      if (name.startsWith("[")) {
        // skip the section delimiters
        continue;
      } else if (name.equalsIgnoreCase("host")) {
        setParam("Server", props.getProperty(name));
      } else if (name.equalsIgnoreCase("port")) {
        setParam("Port", props.getProperty(name));
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
        setParam("Password", VncAuth.unobfuscatePasswd(encryptedPassword));
      } else if (name.equalsIgnoreCase("user")) {
        setParam("User", props.getProperty(name));
      } else if (name.equalsIgnoreCase("preferred_encoding")) {
        int encoding = -1;
        try {
          encoding = Integer.parseInt(props.getProperty(name));
        } catch (NumberFormatException e) {}
        if (encoding >= 0 && encoding <= RFB.ENCODING_LAST)
          setParam("Encoding", RFB.encodingName(encoding));
      } else if (name.equalsIgnoreCase("restricted")) {
        setParam("Restricted", props.getProperty(name));
      } else if (name.equalsIgnoreCase("viewonly")) {
        setParam("ViewOnly", props.getProperty(name));
      } else if (name.equalsIgnoreCase("reversescroll")) {
        setParam("ReverseScroll", props.getProperty(name));
      } else if (name.equalsIgnoreCase("fullscreen")) {
        setParam("FullScreen", props.getProperty(name));
      } else if (name.equalsIgnoreCase("fsaltenter")) {
        setParam("FSAltEnter", props.getProperty(name));
      } else if (name.equalsIgnoreCase("grabkeyboard")) {
        int grabKeyboard = -1;
        try {
          grabKeyboard = Integer.parseInt(props.getProperty(name));
        } catch (NumberFormatException e) {}
        switch (grabKeyboard) {
          case Options.GRAB_FS:
            setParam("GrabKeyboard", "FS");  break;
          case Options.GRAB_ALWAYS:
            setParam("GrabKeyboard", "Always");  break;
          case Options.GRAB_MANUAL:
            setParam("GrabKeyboard", "Manual");  break;
        }
      } else if (name.equalsIgnoreCase("span")) {
        int span = -1;
        try {
          span = Integer.parseInt(props.getProperty(name));
        } catch (NumberFormatException e) {}
        if (span == 0) setParam("Span", "Primary");
        else if (span == 1) setParam("Span", "All");
        else if (span == 2) setParam("Span", "Auto");
      } else if (name.equalsIgnoreCase("8bit")) {
        int _8bit = -1;
        try {
          _8bit = Integer.parseInt(props.getProperty(name));
        } catch (NumberFormatException e) {}
        if (_8bit >= 1)
          setParam("Colors", "256");
        else if (_8bit == 0)
          setParam("Colors", "-1");
      } else if (name.equalsIgnoreCase("shared")) {
        setParam("Shared", props.getProperty(name));
      } else if (name.equalsIgnoreCase("disableclipboard")) {
        int disableclipboard = -1;
        try {
          disableclipboard = Integer.parseInt(props.getProperty(name));
        } catch (NumberFormatException e) {}
        if (disableclipboard >= 1) {
          setParam("RecvClipboard", "0");
          setParam("SendClipboard", "0");
        } else if (disableclipboard == 0) {
          setParam("RecvClipboard", "1");
          setParam("SendClipboard", "1");
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
        desktopSize = props.getProperty(name);
      } else if (name.equalsIgnoreCase("cursorshape")) {
        setParam("CursorShape", props.getProperty(name));
      } else if (name.equalsIgnoreCase("noremotecursor")) {
        setParam("LocalCursor", props.getProperty(name));
      } else if (name.equalsIgnoreCase("compresslevel")) {
        setParam("CompressLevel", props.getProperty(name));
      } else if (name.equalsIgnoreCase("subsampling")) {
        int subsampling = -1;
        try {
          subsampling = Integer.parseInt(props.getProperty(name));
        } catch (NumberFormatException e) {}
        switch (subsampling) {
          case Options.SUBSAMP_NONE:  setParam("Subsampling", "1X");  break;
          case Options.SUBSAMP_4X:    setParam("Subsampling", "4X");  break;
          case Options.SUBSAMP_2X:    setParam("Subsampling", "2X");  break;
          case Options.SUBSAMP_GRAY:  setParam("Subsampling", "Gray");  break;
        }
      } else if (name.equalsIgnoreCase("quality")) {
        int quality = -2;
        try {
          quality = Integer.parseInt(props.getProperty(name));
        } catch (NumberFormatException e) {}
        if (quality == -1) setParam("JPEG", "0");
        else if (quality >= 1 && quality <= 100) {
          setParam("Quality", props.getProperty(name));
        }
      } else if (name.equalsIgnoreCase("continuousupdates")) {
        setParam("CU", props.getProperty(name));
      } else if (name.equalsIgnoreCase("nounixlogin")) {
        setParam("NoUnixLogin", props.getProperty(name));
      }
    }

    if ((scaleNum >= 1 || scaleDenom >= 1) && fitWindow < 1) {
      if (scaleNum < 1) scaleNum = 1;
      if (scaleDenom < 1) scaleDenom = 1;
      int scalingFactor = scaleNum * 100 / scaleDenom;
      setParam("Scale", Integer.toString(scalingFactor));
    } else if (fitWindow >= 1) {
      setParam("Scale", "FixedRatio");
    }

    if (desktopSize != null)
      setParam("DesktopSize", desktopSize);
    else {
      switch (resizeMode) {
        case Options.SIZE_SERVER:
          setParam("DesktopSize", "Server");  break;
        case Options.SIZE_MANUAL:
          if (desktopWidth > 0 && desktopHeight > 0)
            setParam("DesktopSize", desktopWidth + "x" + desktopHeight);
          break;
        case Options.SIZE_AUTO:
          setParam("DesktopSize", "Auto");  break;
      }
    }
  }

  private Configuration() {}
  static VoidParameter head;
  static VoidParameter tail;
  static LogWriter vlog = new LogWriter("Configuration");
}
