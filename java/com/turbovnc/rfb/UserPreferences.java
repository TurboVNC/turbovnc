/* Copyright (C) 2012, 2015, 2018, 2020-2022, 2024-2025 D. R. Commander.
 *                                                      All Rights Reserved.
 * Copyright (C) 2012 Brian P. Hinz
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

import javax.swing.*;
import java.util.*;
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

  private static void deleteNode(String nName, boolean caseSensitive) {
    try {
      String[] children = root.childrenNames();
      for (int i = 0; i < children.length; i++) {
        if (nName != null &&
            (caseSensitive ? children[i].equals(nName) :
                             children[i].equalsIgnoreCase(nName))) {
          Preferences node = root.node(children[i]);
          node.removeNode();
        }
      }
      root.sync();
    } catch (BackingStoreException e) {
      vlog.error("Could not delete preferences:");
      vlog.error("  " + e.getMessage());
    }
  }

  public static void deleteHostOptions(String host) {
    Object[] dlgOptions = {
      UIManager.getString("OptionPane.yesButtonText"),
      UIManager.getString("OptionPane.noButtonText")
    };
    JOptionPane pane = new JOptionPane("Do you want to delete the saved\n" +
                                       "options for host " + host + "?",
                                       JOptionPane.WARNING_MESSAGE,
                                       JOptionPane.YES_NO_OPTION, null,
                                       dlgOptions, dlgOptions[1]);
    JDialog dlg = pane.createDialog(null, "TurboVNC Viewer");
    dlg.setAlwaysOnTop(true);
    dlg.setVisible(true);
    if (pane.getValue() == dlgOptions[1])
      return;
    deleteNode(host, true);
  }

  public static void deleteAllOptions() {
    try {
      Object[] dlgOptions = {
        UIManager.getString("OptionPane.yesButtonText"),
        UIManager.getString("OptionPane.noButtonText")
      };
      JOptionPane pane = new JOptionPane("Are you sure you want to delete\n" +
                                         "the saved options for all hosts?",
                                         JOptionPane.WARNING_MESSAGE,
                                         JOptionPane.YES_NO_OPTION, null,
                                         dlgOptions, dlgOptions[1]);
      JDialog dlg = pane.createDialog(null, "TurboVNC Viewer");
      dlg.setAlwaysOnTop(true);
      dlg.setVisible(true);
      if (pane.getValue() == dlgOptions[1])
        return;
      root.clear();
      String[] children = root.childrenNames();
      for (int i = 0; i < children.length; i++) {
        Preferences node = root.node(children[i]);
        node.removeNode();
      }
      root.sync();
    } catch (BackingStoreException e) {
      vlog.error("Could not delete preferences:");
      vlog.error("  " + e.getMessage());
    }
  }

  public static void updateHistory(String serverStr, boolean clear) {
    // Update the history list
    String valueStr = get("ServerDialog", "history");
    String t = (valueStr == null) ? "" : valueStr;
    StringTokenizer st = new StringTokenizer(t, ",");
    StringBuffer sb = new StringBuffer();
    if (!clear)
      sb.append(serverStr);
    while (st.hasMoreTokens()) {
      String str = st.nextToken();
      if (!str.equals(serverStr) && !str.equals("")) {
        if (sb.length() > 0)
          sb.append(',');
        sb.append(str);
      }
    }
    String history = sb.toString();
    if (history.length() > Preferences.MAX_VALUE_LENGTH) {
      int lastComma = history.lastIndexOf(',', Preferences.MAX_VALUE_LENGTH);
      if (lastComma <= 0)
        history = "";
      else
        history = history.substring(0, lastComma);
    }
    set("ServerDialog", "history", history);
    save("ServerDialog");
  }

  public static void clearHistory() {
    deleteNode("ServerDialog", false);
  }

  public static void load(String nName, Params params) {
    // Load the value of any parameters that have not already been set on the
    // command line or in a connection info file
    try {
      Preferences node = root.node(nName.replaceAll("[\\[\\]]", ""));
      String[] keys = node.keys();
      params.resetGUI();
      for (int i = 0; i < keys.length; i++) {
        String key = keys[i], paramName = key;
        String valueStr = node.get(key, null);
        if (valueStr != null)
          params.setGUI(paramName, valueStr);
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
