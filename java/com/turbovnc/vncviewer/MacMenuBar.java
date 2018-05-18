/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright (C) 2011 Brian P. Hinz
 * Copyright (C) 2012-2015, 2018 D. R. Commander.  All Rights Reserved.
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

// On Mac, we can add items to the actual menu bar, so this provides a more
// Mac-like L&F than using the F8 menu.

package com.turbovnc.vncviewer;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.lang.reflect.*;

import com.turbovnc.rfb.*;

public class MacMenuBar extends JMenuBar implements ActionListener {

  // The following code allows us to pop up our own dialogs whenever "About"
  // and "Preferences" are selected from the application menu.

  class MyInvocationHandler implements InvocationHandler {
    MyInvocationHandler(CConn cc_) { cc = cc_; }

    public Object invoke(Object proxy, Method method, Object[] args) {
      if (method.getName().equals("handleAbout")) {
        cc.showAbout();
      } else if (method.getName().equals("handlePreferences")) {
        cc.options.showDialog(cc.viewport);
      }
      return null;
    }

    CConn cc;
  }

  void setupAppMenu() {
    try {
      Class appClass, aboutHandlerClass, prefsHandlerClass;
      Object obj;

      if (VncViewer.JAVA_VERSION >= 9) {
        appClass = Desktop.class;
        obj = Desktop.getDesktop();
        aboutHandlerClass = Class.forName("java.awt.desktop.AboutHandler");
        prefsHandlerClass =
          Class.forName("java.awt.desktop.PreferencesHandler");
      } else {
        appClass = Class.forName("com.apple.eawt.Application");
        Method getApplication = appClass.getMethod("getApplication",
                                                   (Class[])null);
        obj = getApplication.invoke(appClass);
        aboutHandlerClass = Class.forName("com.apple.eawt.AboutHandler");
        prefsHandlerClass = Class.forName("com.apple.eawt.PreferencesHandler");
      }

      InvocationHandler aboutHandler = new MyInvocationHandler(cc);
      Object proxy = Proxy.newProxyInstance(aboutHandlerClass.getClassLoader(),
                                            new Class[]{ aboutHandlerClass },
                                            aboutHandler);
      Method setAboutHandler =
        appClass.getMethod("setAboutHandler", aboutHandlerClass);
      setAboutHandler.invoke(obj, new Object[]{ proxy });

      InvocationHandler prefsHandler = new MyInvocationHandler(cc);
      proxy = Proxy.newProxyInstance(prefsHandlerClass.getClassLoader(),
                                     new Class[]{ prefsHandlerClass },
                                     prefsHandler);
      Method setPreferencesHandler =
        appClass.getMethod("setPreferencesHandler", prefsHandlerClass);
      setPreferencesHandler.invoke(obj, new Object[]{ proxy });
    } catch (Exception e) {
      vlog.info("Could not override About/Preferences menu items:");
      vlog.info("  " + e.toString());
    }
  }

  public MacMenuBar(CConn cc_) {
    cc = cc_;
    int acceleratorMask = VncViewer.getMenuShortcutKeyMask();

    setupAppMenu();

    JMenu connMenu = new JMenu("Connection");
    if (!VncViewer.noNewConn.getValue()) {
      newConn = addMenuItem(connMenu, "New Connection...");
      newConn.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_N,
                                                    acceleratorMask));
      closeConn = addMenuItem(connMenu, "Close Connection");
      closeConn.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_W,
                                                      acceleratorMask));
      connMenu.addSeparator();
    }
    info = addMenuItem(connMenu, "Connection Info...");
    info.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_I,
                                               acceleratorMask));
    profile = new JCheckBoxMenuItem("Performance Info...");
    profile.setSelected(cc.profileDialog.isVisible());
    profile.addActionListener(this);
    connMenu.add(profile);
    profile.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_P,
                                                  acceleratorMask));

    connMenu.addSeparator();
    refresh = addMenuItem(connMenu, "Request Screen Refresh");
    refresh.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_R,
                                                  acceleratorMask));
    losslessRefresh = addMenuItem(connMenu, "Request Lossless Refresh");
    losslessRefresh.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_L,
                                                          acceleratorMask));
    connMenu.addSeparator();
    fullScreen = new JCheckBoxMenuItem("Full Screen");
    fullScreen.setSelected(cc.opts.fullScreen);
    fullScreen.addActionListener(this);
    connMenu.add(fullScreen);
    fullScreen.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_F,
                                                     acceleratorMask));
    defaultSize = addMenuItem(connMenu, "Default Window Size/Position");
    defaultSize.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_Z,
                                                      acceleratorMask));
    showToolbar = new JCheckBoxMenuItem("Show Toolbar");
    showToolbar.setSelected(cc.showToolbar);
    showToolbar.addActionListener(this);
    connMenu.add(showToolbar);
    showToolbar.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_T,
                                                      acceleratorMask));
    connMenu.addSeparator();
    if (!VncViewer.restricted.getValue()) {
      ctrlAltDel = addMenuItem(connMenu, "Send Ctrl-Alt-Del");
      ctrlEsc = addMenuItem(connMenu, "Send Ctrl-Esc");
      connMenu.addSeparator();
    }
    clipboard = addMenuItem(connMenu, "Clipboard ...");

    add(connMenu);
  }

  JMenuItem addMenuItem(JMenu menu, String str, int mnemonic) {
    JMenuItem item = new JMenuItem(str, mnemonic);
    item.addActionListener(this);
    menu.add(item);
    return item;
  }

  JMenuItem addMenuItem(JMenu menu, String str) {
    JMenuItem item = new JMenuItem(str);
    item.addActionListener(this);
    menu.add(item);
    return item;
  }

  boolean actionMatch(ActionEvent ev, JMenuItem item) {
    return ev.getActionCommand().equals(item.getActionCommand());
  }

  public void actionPerformed(ActionEvent ev) {
    if (actionMatch(ev, fullScreen)) {
      cc.toggleFullScreen();
    } else if (actionMatch(ev, defaultSize)) {
      cc.sizeWindow();
    } else if (actionMatch(ev, showToolbar)) {
      cc.toggleToolbar();
      showToolbar.setSelected(cc.showToolbar);
    } else if (actionMatch(ev, clipboard)) {
      cc.clipboardDialog.showDialog(cc.viewport);
    } else if (actionMatch(ev, ctrlAltDel) && !cc.opts.viewOnly) {
      cc.writeKeyEvent(Keysyms.CONTROL_L, true);
      cc.writeKeyEvent(Keysyms.ALT_L, true);
      cc.writeKeyEvent(Keysyms.DELETE, true);
      cc.writeKeyEvent(Keysyms.DELETE, false);
      cc.writeKeyEvent(Keysyms.ALT_L, false);
      cc.writeKeyEvent(Keysyms.CONTROL_L, false);
    } else if (actionMatch(ev, ctrlEsc) && !cc.opts.viewOnly) {
      cc.writeKeyEvent(Keysyms.CONTROL_L, true);
      cc.writeKeyEvent(Keysyms.ESCAPE, true);
      cc.writeKeyEvent(Keysyms.ESCAPE, false);
      cc.writeKeyEvent(Keysyms.CONTROL_L, false);
    } else if (actionMatch(ev, refresh)) {
      cc.refresh();
    } else if (actionMatch(ev, losslessRefresh)) {
      cc.losslessRefresh();
    } else if (!VncViewer.noNewConn.getValue() && actionMatch(ev, newConn)) {
      VncViewer.newViewer(cc.viewer);
    } else if (!VncViewer.noNewConn.getValue() && actionMatch(ev, closeConn)) {
      cc.close();
    } else if (actionMatch(ev, info)) {
      cc.showInfo();
    } else if (actionMatch(ev, profile)) {
      if (cc.profileDialog.isVisible())
        cc.profileDialog.endDialog();
      else
        cc.profileDialog.showDialog(cc.viewport);
      cc.toggleProfile();
    }
  }

  void updateFullScreen() {
    fullScreen.setSelected(cc.opts.fullScreen);
  }

  void updateProfile() {
    profile.setSelected(cc.profileDialog.isVisible());
  }

  CConn cc;
  JMenuItem defaultSize;
  JMenuItem clipboard, ctrlAltDel, ctrlEsc, refresh, losslessRefresh;
  JMenuItem newConn, closeConn, info, profile, showToolbar;
  JCheckBoxMenuItem fullScreen;
  static LogWriter vlog = new LogWriter("MacMenuBar");
}
