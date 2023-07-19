/* Copyright (C) 2012-2015, 2018, 2020-2023 D. R. Commander.
 *                                          All Rights Reserved.
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
        if (!cc.options.isShown())
          cc.options.showDialog(cc.viewport);
      } else if (method.getName().equals("handleQuitRequestWith")) {
        try {
          Class quitResponseClass;

          if (Utils.JAVA_VERSION >= 9)
            quitResponseClass = Class.forName("java.awt.desktop.QuitResponse");
          else
            quitResponseClass = Class.forName("com.apple.eawt.QuitResponse");

          Method cancelQuit =
            quitResponseClass.getMethod("cancelQuit", (Class[])null);
          Method performQuit =
            quitResponseClass.getMethod("performQuit", (Class[])null);

          if (cc.confirmClose())
            performQuit.invoke(args[1]);
          else
            cancelQuit.invoke(args[1]);
        } catch (Exception e) {
          vlog.info("Could not handle quit request:");
          vlog.info("  " + e.toString());
        }
      }
      return null;
    }

    CConn cc;
  }

  void setupAppMenu() {
    try {
      Class appClass, aboutHandlerClass, prefsHandlerClass, quitHandlerClass;
      Object obj;

      if (Utils.JAVA_VERSION >= 9) {
        appClass = Desktop.class;
        obj = Desktop.getDesktop();
        aboutHandlerClass = Class.forName("java.awt.desktop.AboutHandler");
        prefsHandlerClass =
          Class.forName("java.awt.desktop.PreferencesHandler");
        quitHandlerClass = Class.forName("java.awt.desktop.QuitHandler");
      } else {
        appClass = Class.forName("com.apple.eawt.Application");
        Method getApplication = appClass.getMethod("getApplication",
                                                   (Class[])null);
        obj = getApplication.invoke(appClass);
        aboutHandlerClass = Class.forName("com.apple.eawt.AboutHandler");
        prefsHandlerClass = Class.forName("com.apple.eawt.PreferencesHandler");
        quitHandlerClass = Class.forName("com.apple.eawt.QuitHandler");
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

      InvocationHandler quitHandler = new MyInvocationHandler(cc);
      proxy = Proxy.newProxyInstance(quitHandlerClass.getClassLoader(),
                                     new Class[]{ quitHandlerClass },
                                     quitHandler);
      Method setQuitHandler =
        appClass.getMethod("setQuitHandler", quitHandlerClass);
      setQuitHandler.invoke(obj, new Object[]{ proxy });
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
    if (!cc.params.noNewConn.get()) {
      newConn = addMenuItem(connMenu, "New Connection...");
      if (cc.params.macHotkeys.get())
        newConn.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_N,
                                                      acceleratorMask));
      closeConn = addMenuItem(connMenu, "Close Connection");
      if (cc.params.macHotkeys.get())
        closeConn.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_W,
                                                        acceleratorMask));
      connMenu.addSeparator();
    }
    info = addMenuItem(connMenu, "Connection Info...");
    if (cc.params.macHotkeys.get())
      info.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_I,
                                                 acceleratorMask));
    profile = new JCheckBoxMenuItem("Performance Info...");
    profile.setSelected(cc.profileDialog.isVisible());
    profile.addActionListener(this);
    connMenu.add(profile);
    if (cc.params.macHotkeys.get())
      profile.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_P,
                                                    acceleratorMask));

    connMenu.addSeparator();
    refresh = addMenuItem(connMenu, "Request Screen Refresh");
    if (cc.params.macHotkeys.get())
      refresh.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_R,
                                                    acceleratorMask));
    losslessRefresh = addMenuItem(connMenu, "Request Lossless Refresh");
    if (cc.params.macHotkeys.get())
      losslessRefresh.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_L,
                                                            acceleratorMask));
    screenshot = addMenuItem(connMenu, "Save Remote Desktop Image");
    if (cc.params.macHotkeys.get())
      screenshot.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_M,
                                                       acceleratorMask));
    connMenu.addSeparator();
    fullScreen = new JCheckBoxMenuItem("Full Screen");
    fullScreen.setSelected(cc.params.fullScreen.get());
    fullScreen.addActionListener(this);
    connMenu.add(fullScreen);
    if (cc.params.macHotkeys.get())
      fullScreen.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_F,
                                                       acceleratorMask));
    defaultSize = addMenuItem(connMenu, "Default Window Size/Position");
    if (cc.params.macHotkeys.get())
      defaultSize.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_Z,
                                                        acceleratorMask));
    zoomIn = addMenuItem(connMenu, "Zoom In");
    if (cc.params.macHotkeys.get())
      zoomIn.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_9,
                                                   acceleratorMask));
    zoomOut = addMenuItem(connMenu, "Zoom Out");
    if (cc.params.macHotkeys.get())
      zoomOut.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_8,
                                                    acceleratorMask));
    zoom100 = addMenuItem(connMenu, "Zoom 100%", KeyEvent.VK_0);
    if (cc.params.macHotkeys.get())
      zoom100.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_0,
                                                    acceleratorMask));
    tileWindows = addMenuItem(connMenu, "Tile All Viewer Windows");
    if (cc.params.macHotkeys.get())
      tileWindows.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_X,
                                                        acceleratorMask));
    showToolbar = new JCheckBoxMenuItem("Show Toolbar");
    showToolbar.setSelected(cc.params.toolbar.get());
    showToolbar.addActionListener(this);
    connMenu.add(showToolbar);
    if (cc.params.macHotkeys.get())
      showToolbar.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_T,
                                                        acceleratorMask));
    connMenu.addSeparator();
    viewOnly = new JCheckBoxMenuItem("View Only");
    viewOnly.setSelected(cc.params.viewOnly.get());
    viewOnly.addActionListener(this);
    connMenu.add(viewOnly);
    if (!cc.params.restricted.get()) {
      ctrlAltDel = addMenuItem(connMenu, "Send Ctrl-Alt-Del");
      ctrlEsc = addMenuItem(connMenu, "Send Ctrl-Esc");
      connMenu.addSeparator();
    }
    clipboard = addMenuItem(connMenu, "Clipboard ...");

    add(connMenu);

    updateZoom();
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
    } else if (actionMatch(ev, zoomIn)) {
      cc.zoomIn();
    } else if (actionMatch(ev, zoomOut)) {
      cc.zoomOut();
    } else if (actionMatch(ev, zoom100)) {
      cc.zoom100();
    } else if (actionMatch(ev, tileWindows)) {
      VncViewer.tileWindows();
    } else if (actionMatch(ev, showToolbar)) {
      cc.toggleToolbar();
      showToolbar.setSelected(cc.params.toolbar.get());
    } else if (actionMatch(ev, viewOnly)) {
      cc.toggleViewOnly();
      viewOnly.setSelected(cc.params.viewOnly.get());
    } else if (actionMatch(ev, clipboard)) {
      cc.clipboardDialog.showDialog(cc.viewport);
    } else if (actionMatch(ev, ctrlAltDel) && !cc.params.viewOnly.get()) {
      cc.writeKeyEvent(Keysyms.CONTROL_L, true);
      cc.writeKeyEvent(Keysyms.ALT_L, true);
      cc.writeKeyEvent(Keysyms.DELETE, true);
      cc.writeKeyEvent(Keysyms.DELETE, false);
      cc.writeKeyEvent(Keysyms.ALT_L, false);
      cc.writeKeyEvent(Keysyms.CONTROL_L, false);
    } else if (actionMatch(ev, ctrlEsc) && !cc.params.viewOnly.get()) {
      cc.writeKeyEvent(Keysyms.CONTROL_L, true);
      cc.writeKeyEvent(Keysyms.ESCAPE, true);
      cc.writeKeyEvent(Keysyms.ESCAPE, false);
      cc.writeKeyEvent(Keysyms.CONTROL_L, false);
    } else if (actionMatch(ev, refresh)) {
      cc.refresh();
    } else if (actionMatch(ev, losslessRefresh)) {
      cc.losslessRefresh();
    } else if (actionMatch(ev, screenshot)) {
      cc.screenshot();
    } else if (!cc.params.noNewConn.get() && actionMatch(ev, newConn)) {
      VncViewer.newViewer(cc.viewer);
    } else if (!cc.params.noNewConn.get() && actionMatch(ev, closeConn)) {
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
    fullScreen.setSelected(cc.params.fullScreen.get());
  }

  void updateProfile() {
    profile.setSelected(cc.profileDialog.isVisible());
  }

  void updateZoom() {
    if (cc.params.desktopSize.getMode() == DesktopSize.AUTO ||
        cc.params.scale.get() == ScaleParameter.AUTO ||
        cc.params.scale.get() == ScaleParameter.FIXEDRATIO) {
      zoomIn.setEnabled(false);
      zoomOut.setEnabled(false);
      zoom100.setEnabled(false);
    } else {
      zoomIn.setEnabled(true);
      zoomOut.setEnabled(true);
      zoom100.setEnabled(true);
    }
  }

  CConn cc;
  JMenuItem defaultSize, zoomIn, zoomOut, zoom100, tileWindows;
  JMenuItem clipboard, ctrlAltDel, ctrlEsc, refresh, losslessRefresh;
  JMenuItem newConn, closeConn, info, screenshot;
  JCheckBoxMenuItem profile, fullScreen, showToolbar, viewOnly;
  static LogWriter vlog = new LogWriter("MacMenuBar");
}
