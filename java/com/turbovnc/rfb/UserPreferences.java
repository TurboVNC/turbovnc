/* Copyright (C) 2012 Brian P. Hinz
 * Copyright (C) 2012, 2015, 2018, 2020-2022 D. R. Commander.
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

import java.util.prefs.Preferences;
import java.util.prefs.BackingStoreException;

public final class UserPreferences {

  private static Preferences root = Preferences.userRoot().node("TurboVNC");

  public static void set(String nName, String key, String val) {
    Preferences node = root.node(nName);
    if (val == null) val = "";
    node.put(key, val);
  }

  public static String get(String nName, String key) {
    Preferences node = root.node(nName);
    return node.get(key, null);
  }

  public static void save() {
    try {
      root.sync();
      String[] keys = root.keys();
      for (int i = 0; i < keys.length; i++)
        vlog.debug(keys[i] + " = " + root.get(keys[i], null));
    } catch (BackingStoreException e) {
      vlog.error("Could not save preferences:");
      vlog.error("  " + e.getMessage());
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
      vlog.error("Could not save preferences:");
      vlog.error("  " + e.getMessage());
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
      vlog.error("Could not clear preferences:");
      vlog.error("  " + e.getMessage());
    }
  }

  public static void load(String nName, Params params) {
    // Sets the value of any corresponding Configuration parameters
    try {
      Preferences node = root.node(nName);
      String[] keys = node.keys();
      for (int i = 0; i < keys.length; i++) {
        String key = keys[i], paramName = key;

        if (key.equalsIgnoreCase("SecTypes"))
          paramName = "SecurityTypes";
        else if (key.equalsIgnoreCase("Username"))
          paramName = "User";
        VoidParameter p = params.get(paramName);
        if (p == null ||
            key.equalsIgnoreCase("AlwaysShowConnectionDialog") ||
            key.equalsIgnoreCase("Colors") ||
            key.equalsIgnoreCase("Encoding"))
          continue;

        String valueStr = node.get(key, null);
        if (valueStr != null)
          params.set(paramName, valueStr);
      }
    } catch (BackingStoreException e) {
      vlog.error("Could not get preferences:");
      vlog.error("  " + e.getMessage());
    }
    params.reconcile();
  }

  private UserPreferences() {}
  static LogWriter vlog = new LogWriter("UserPreferences");
}
