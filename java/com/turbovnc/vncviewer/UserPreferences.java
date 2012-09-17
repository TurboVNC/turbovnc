/* Copyright (C) 2012 Brian P. Hinz
 * Copyright (C) 2012 D. R. Commander.  All Rights Reserved.
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
// For storing user preferences not specified as configuration parameters.
//

package com.turbovnc.vncviewer;

import java.util.prefs.Preferences;
import java.util.prefs.BackingStoreException;

import com.turbovnc.rfb.*;

public class UserPreferences {

  private static Preferences root = Preferences.userRoot().node("TurboVNC");

  public static void set(String nName, String key, String val) {
    Preferences node = root.node(nName);
    node.put(key, val);
  }

  public static void set(String nName, String key, int val) {
    Preferences node = root.node(nName);
    node.putInt(key, val);
  }

  public static void set(String nName, String key, boolean val) {
    Preferences node = root.node(nName);
    node.putBoolean(key, val);
  }

  public static String get(String nName, String key) {
    Preferences node = root.node(nName);
    return node.get(key, null);
  }

  public static boolean getBool(String nName, String key, boolean defval) {
    Preferences node = root.node(nName);
    return node.getBoolean(key, defval);
  }

  public static int getInt(String nName, String key, int defval) {
    Preferences node = root.node(nName);
    return node.getInt(key, defval);
  }

  public static void save() {
    try {
      root.sync();
      String[] keys = root.keys();
      for (int i = 0; i < keys.length; i++)
        vlog.debug(keys[i] + " = " + root.get(keys[i], null));
    } catch (BackingStoreException e) {
      vlog.error(e.getMessage());
    }
  }

  public static void save(String nName) {
    try {
      Preferences node = root.node(nName);
      node.sync();
      String[] keys = root.keys();
      for (int i = 0; i < keys.length; i++)
        vlog.debug(keys[i] + " = " + node.get(keys[i], null));
    } catch (BackingStoreException e) {
      vlog.error(e.getMessage());
    }
  }

  public static void clear() {
    try {
      root.clear();
      String[] children = root.childrenNames();
      for (int i = 0; i < children.length; i++) {
        Preferences node = root.node(children[i]);
        node.removeNode();
      }
      root.sync();
    } catch (BackingStoreException e) {
      vlog.error(e.getMessage());
    }
  }

  public static void load(String nName) {
    // Sets the value of any corresponding Configuration parameters
    boolean secVeNCrypt = true, setSecVeNCrypt = false;
    boolean encNone = true, setEncNone = false;
    boolean encTLS = true, setEncTLS = false;
    boolean encX509 = true, setEncX509 = false;
    boolean secPlain = true, setSecPlain = false;
    boolean secIdent = true, setSecIdent = false;
    boolean secNone = true, setSecNone = false;
    boolean secVnc = true, setSecVnc = false;
    try {
      Preferences node = root.node(nName);
      String[] keys = node.keys();
      for (int i = 0; i < keys.length; i++) {
        String key = keys[i];
        VoidParameter p = Configuration.getParam(key);
        if (p == null) {
          if (key.equalsIgnoreCase("secVeNCrypt")) {
            setSecVeNCrypt = true;
            secVeNCrypt = node.getBoolean(key, true);
          } else if (key.equalsIgnoreCase("encNone")) {
            setEncNone = true;
            encNone = node.getBoolean(key, true);
          } else if (key.equalsIgnoreCase("encTLS")) {
            setEncTLS = true;
            encTLS = node.getBoolean(key, true);
          } else if (key.equalsIgnoreCase("encX509")) {
            setEncX509 = true;
            encX509 = node.getBoolean(key, true);
          } else if (key.equalsIgnoreCase("secPlain")) {
            setSecPlain = true;
            secPlain = node.getBoolean(key, true);
          } else if (key.equalsIgnoreCase("secIdent")) {
            setSecIdent = true;
            secIdent = node.getBoolean(key, true);
          } else if (key.equalsIgnoreCase("secNone")) {
            setSecNone = true;
            secNone = node.getBoolean(key, true);
          } else if (key.equalsIgnoreCase("secVnc")) {
            setSecVnc = true;
            secVnc = node.getBoolean(key, true);
          } else if (!key.equalsIgnoreCase("x509ca")) {
            String valueStr = node.get(key, null);
            if (valueStr != null)
              CSecurityTLS.x509ca.setParam(valueStr);
          } else if (!key.equalsIgnoreCase("x509crl")) {
            String valueStr = node.get(key, null);
            if (valueStr != null)
              CSecurityTLS.x509crl.setParam(valueStr);
          } else
            continue;
        }

        String valueStr = node.get(key, null);
        if (valueStr != null)
          Configuration.setParam(key, valueStr);
      }
    } catch (BackingStoreException e) {
      vlog.error(e.getMessage());
    }
    if ((setEncX509 || setEncTLS || setEncNone) &&
        (setSecPlain || setSecIdent || setSecVnc || setSecNone)) {
      Security.DisableSecType(Security.secTypeNone);
      Security.DisableSecType(Security.secTypeVncAuth);
      Security.DisableSecType(Security.secTypePlain);
      Security.DisableSecType(Security.secTypeIdent);
      Security.DisableSecType(Security.secTypeTLSNone);
      Security.DisableSecType(Security.secTypeTLSVnc);
      Security.DisableSecType(Security.secTypeTLSPlain);
      Security.DisableSecType(Security.secTypeTLSIdent);
      Security.DisableSecType(Security.secTypeX509None);
      Security.DisableSecType(Security.secTypeX509Vnc);
      Security.DisableSecType(Security.secTypeX509Plain);
      Security.DisableSecType(Security.secTypeX509Ident);
    }

    if (setSecVeNCrypt) {
      if (secVeNCrypt) Security.EnableSecType(Security.secTypeVeNCrypt);
      else {
        Security.DisableSecType(Security.secTypeVeNCrypt);
        encX509 = encTLS = secPlain = secIdent = false;
      }
      Security.setInUserPrefs = true;
    }
    if (setEncX509 && setSecPlain) {
      if (encX509 && secPlain) Security.EnableSecType(Security.secTypeX509Plain);
      else Security.DisableSecType(Security.secTypeX509Plain);
      Security.setInUserPrefs = true;
    }
    if (setEncX509 && setSecIdent) {
      if (encX509 && secIdent) Security.EnableSecType(Security.secTypeX509Ident);
      else Security.DisableSecType(Security.secTypeX509Ident);
      Security.setInUserPrefs = true;
    }
    if (setEncX509 && setSecVnc) {
      if (encX509 && secVnc) Security.EnableSecType(Security.secTypeX509Vnc);
      else Security.DisableSecType(Security.secTypeX509Vnc);
      Security.setInUserPrefs = true;
    }
    if (setEncX509 && setSecNone) {
      if (encX509 && secNone) Security.EnableSecType(Security.secTypeX509None);
      else Security.DisableSecType(Security.secTypeX509None);
      Security.setInUserPrefs = true;
    }
    if (setEncTLS && setSecPlain) {
      if (encTLS && secPlain) Security.EnableSecType(Security.secTypeTLSPlain);
      else Security.DisableSecType(Security.secTypeTLSPlain);
      Security.setInUserPrefs = true;
    }
    if (setEncTLS && setSecIdent) {
      if (encTLS && secIdent) Security.EnableSecType(Security.secTypeTLSIdent);
      else Security.DisableSecType(Security.secTypeTLSIdent);
      Security.setInUserPrefs = true;
    }
    if (setEncTLS && setSecVnc) {
      if (encTLS && secVnc) Security.EnableSecType(Security.secTypeTLSVnc);
      else Security.DisableSecType(Security.secTypeTLSVnc);
      Security.setInUserPrefs = true;
    }
    if (setEncTLS && setSecNone) {
      if (encTLS && secNone) Security.EnableSecType(Security.secTypeTLSNone);
      else Security.DisableSecType(Security.secTypeTLSNone);
      Security.setInUserPrefs = true;
    }
    if (setEncNone && setSecPlain) {
      if (encNone && secPlain) Security.EnableSecType(Security.secTypePlain);
      else Security.DisableSecType(Security.secTypePlain);
      Security.setInUserPrefs = true;
    }
    if (setEncNone && setSecIdent) {
      if (encNone && secIdent) Security.EnableSecType(Security.secTypeIdent);
      else Security.DisableSecType(Security.secTypeIdent);
      Security.setInUserPrefs = true;
    }
    if (setSecVnc) {
      if (secVnc) Security.EnableSecType(Security.secTypeVncAuth);
      else Security.DisableSecType(Security.secTypeVncAuth);
      Security.setInUserPrefs = true;
    }
    if (setSecNone) {
      if (secNone) Security.EnableSecType(Security.secTypeNone);
      else Security.DisableSecType(Security.secTypeNone);
      Security.setInUserPrefs = true;
    }
  }

  static LogWriter vlog = new LogWriter("UserPreferences");
}

