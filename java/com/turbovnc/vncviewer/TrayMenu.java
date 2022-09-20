/* Copyright (C) 2014, 2018, 2020, 2022 D. R. Commander.  All Rights Reserved.
 * Copyright (C) 2011 Brian P. Hinz
 * Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
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

package com.turbovnc.vncviewer;

import java.awt.*;
import java.awt.event.*;
import java.lang.reflect.*;

import com.turbovnc.rdr.*;
import com.turbovnc.rfb.*;

public class TrayMenu extends PopupMenu implements ActionListener {

  public TrayMenu(VncViewer viewer_) {
    super("TurboVNC Viewer");
    viewer = viewer_;

    if (!viewer.getParams().noNewConn.get()) {
      newConn = addMenuItem("New connection...");
    }
    options = addMenuItem("Default options...");
    addSeparator();
    about = addMenuItem("About TurboVNC Viewer...");
    if (!viewer.getParams().noNewConn.get()) {
      addSeparator();
      exit = addMenuItem("Close listener");
    }

    if (Utils.isMac()) {
      setDockMenu(this);
    } else {
      trayIcon = new TrayIcon(VncViewer.FRAME_IMAGE);
      trayIcon.setPopupMenu(this);
      tray = SystemTray.getSystemTray();
      try {
        tray.add(trayIcon);
      } catch (AWTException e) {
        vlog.error(e.toString());
      }
    }
  }

  void setDockMenu(PopupMenu menu) {
    try {
      Class appClass;
      Object obj;

      if (Utils.JAVA_VERSION >= 9) {
        appClass = Class.forName("java.awt.Taskbar");
        Method getTaskbar =
          appClass.getMethod("getTaskbar", (Class[])null);
        obj = getTaskbar.invoke(appClass);
      } else {
        appClass = Class.forName("com.apple.eawt.Application");
        Method getApplication =
          appClass.getMethod("getApplication", (Class[])null);
        obj = getApplication.invoke(appClass);
      }
      Class[] paramTypes = new Class[1];
      paramTypes[0] = PopupMenu.class;
      Method setDockMenu = Utils.JAVA_VERSION >= 9 ?
        appClass.getMethod("setMenu", paramTypes) :
        appClass.getMethod("setDockMenu", paramTypes);
      setDockMenu.invoke(obj, menu);
    } catch (Exception e) {
      vlog.error("Could not modify dock menu:");
      vlog.error("  " + e.toString());
    }
  }

  static boolean isSupported() {
    return (Utils.isMac() || SystemTray.isSupported());
  }

  MenuItem addMenuItem(String str) {
    MenuItem item = new MenuItem(str);
    item.addActionListener(this);
    add(item);
    return item;
  }

  boolean actionMatch(ActionEvent ev, MenuItem item) {
    if (item == null)
      return false;
    return ev.getActionCommand().equals(item.getActionCommand());
  }

  public void actionPerformed(ActionEvent ev) {
    if (!viewer.getParams().noNewConn.get() && actionMatch(ev, newConn)) {
      VncViewer.newViewer(viewer);
    } else if (actionMatch(ev, options)) {
      viewer.showOptions();
    } else if (actionMatch(ev, about)) {
      VncViewer.showAbout(null);
    } else if (actionMatch(ev, exit)) {
      if (Utils.isMac())
        setDockMenu(null);
      else if (tray != null && trayIcon != null)
        tray.remove(trayIcon);
      viewer.killListener();
      viewer.exit(0);
    }
  }

  MenuItem newConn, options, about, exit;
  TrayIcon trayIcon;
  SystemTray tray;
  VncViewer viewer;

  static LogWriter vlog = new LogWriter("Tray menu");
}
