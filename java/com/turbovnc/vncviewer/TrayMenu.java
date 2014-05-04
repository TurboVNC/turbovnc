/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright (C) 2011 Brian P. Hinz
 * Copyright (C) 2014 D. R. Commander.  All Rights Reserved.
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

import com.turbovnc.rdr.*;
import com.turbovnc.rfb.*;

public class TrayMenu extends PopupMenu implements ActionListener {

  public TrayMenu(VncViewer viewer_) {
    super("TurboVNC Viewer");
    viewer = viewer_;

    if (!VncViewer.noNewConn.getValue()) {
      newConn = addMenuItem("New connection...");
    }
    options = addMenuItem("Default connection options...");
    addSeparator();
    about = addMenuItem("About TurboVNC Viewer...");
    addSeparator();
    exit = addMenuItem("Close listening daemon");

    trayIcon = new TrayIcon(VncViewer.frameImage);
    trayIcon.setPopupMenu(this);
    tray = SystemTray.getSystemTray();
    try {
      tray.add(trayIcon);
    } catch(java.awt.AWTException e) {
      throw new ErrorException(e.getMessage());
    }
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
    if (!VncViewer.noNewConn.getValue() && actionMatch(ev, newConn)) {
      VncViewer.newViewer(viewer);
    } else if (actionMatch(ev, options)) {
      viewer.showOptions();
    } else if (actionMatch(ev, about)) {
      VncViewer.showAbout(null);
    } else if (actionMatch(ev, exit)) {
      tray.remove(trayIcon);
      viewer.exit(0);
    }
  }

  MenuItem newConn, options, about, exit;
  TrayIcon trayIcon;
  SystemTray tray;
  VncViewer viewer;
}
