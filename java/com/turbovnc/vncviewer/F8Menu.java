/* Copyright (C) 2012-2015, 2017-2018, 2020-2022 D. R. Commander.
 *                                               All Rights Reserved.
 * Copyright (C) 2011, 2013 Brian P. Hinz
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

import java.awt.Cursor;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;

import com.turbovnc.rfb.*;

public class F8Menu extends JPopupMenu implements ActionListener {
  public F8Menu(CConn cc_) {
    super("VNC Menu");
    setLightWeightPopupEnabled(false);
    cc = cc_;

    if (!cc.params.noNewConn.get()) {
      exit = addMenuItem("Close Connection", KeyEvent.VK_C);
      addSeparator();
    }
    options = addMenuItem("Options...   (Ctrl-Alt-Shift-O)", KeyEvent.VK_O);
    info = addMenuItem("Connection Info...  (Ctrl-Alt-Shift-I)",
                       KeyEvent.VK_I);
    info.setDisplayedMnemonicIndex(11);
    profile = new JCheckBoxMenuItem("Performance Info...  (Ctrl-Alt-Shift-P)");
    profile.setMnemonic(KeyEvent.VK_P);
    profile.setSelected(cc.profileDialog.isVisible());
    profile.addActionListener(this);
    add(profile);
    addSeparator();
    refresh = addMenuItem("Request Screen Refresh   (Ctrl-Alt-Shift-R)",
                          KeyEvent.VK_R);
    refresh.setDisplayedMnemonicIndex(15);
    losslessRefresh =
      addMenuItem("Request Lossless Refresh   (Ctrl-Alt-Shift-L)",
                  KeyEvent.VK_L);
    losslessRefresh.setDisplayedMnemonicIndex(8);
    screenshot =
      addMenuItem("Save Remote Desktop Image...   (Ctrl-Alt-Shift-M)",
                  KeyEvent.VK_M);
    screenshot.setDisplayedMnemonicIndex(21);
    addSeparator();
    fullScreen = new JCheckBoxMenuItem("Full Screen   (Ctrl-Alt-Shift-F)");
    fullScreen.setMnemonic(KeyEvent.VK_F);
    fullScreen.setSelected(cc.params.fullScreen.get());
    fullScreen.addActionListener(this);
    add(fullScreen);
    defaultSize =
      addMenuItem("Default Window Size/Position   (Ctrl-Alt-Shift-Z)",
                  KeyEvent.VK_Z);
    zoomIn = addMenuItem("Zoom In   (Ctrl-Alt-Shift-9)", KeyEvent.VK_9);
    zoomOut = addMenuItem("Zoom Out   (Ctrl-Alt-Shift-8)", KeyEvent.VK_8);
    zoom100 = addMenuItem("Zoom 100%   (Ctrl-Alt-Shift-0)", KeyEvent.VK_0);
    showToolbar = new JCheckBoxMenuItem("Show Toolbar   (Ctrl-Alt-Shift-T)");
    showToolbar.setMnemonic(KeyEvent.VK_T);
    showToolbar.setSelected(cc.params.toolbar.get());
    showToolbar.addActionListener(this);
    add(showToolbar);
    tileWindows = addMenuItem("Tile All Viewer Windows   (Ctrl-Alt-Shift-X)",
                              KeyEvent.VK_X);
    addSeparator();
    viewOnly = new JCheckBoxMenuItem("View Only   (Ctrl-Alt-Shift-V)");
    viewOnly.setMnemonic(KeyEvent.VK_V);
    viewOnly.setSelected(cc.params.viewOnly.get());
    viewOnly.addActionListener(this);
    add(viewOnly);
    if (Utils.osGrab() && Helper.isAvailable()) {
      grabKeyboard =
        new JCheckBoxMenuItem("Grab Keyboard   (Ctrl-Alt-Shift-G)");
      grabKeyboard.setMnemonic(KeyEvent.VK_G);
      grabKeyboard.setSelected(VncViewer.isKeyboardGrabbed());
      grabKeyboard.addActionListener(this);
      add(grabKeyboard);
    }
    f8 = addMenuItem("Send " + cc.params.menuKey.getStr());
    KeyStroke ks = KeyStroke.getKeyStroke(cc.params.menuKey.getVKeyCode(), 0);
    f8.setAccelerator(ks);
    if (!cc.params.restricted.get()) {
      ctrlAltDel = addMenuItem("Send Ctrl-Alt-Del");
      ctrlEsc = addMenuItem("Send Ctrl-Esc");
    }
    addSeparator();
    clipboard = addMenuItem("Clipboard...");
    addSeparator();
    if (!cc.params.noNewConn.get()) {
      newConn = addMenuItem("New Connection...   (Ctrl-Alt-Shift-N)",
                            KeyEvent.VK_N);
      addSeparator();
    }
    about = addMenuItem("About TurboVNC Viewer...", KeyEvent.VK_A);
    addSeparator();
    dismiss = addMenuItem("Dismiss Menu");
    setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));

    if (Utils.osGrab())
      addPopupMenuListener(new PopupMenuListener() {
        public void popupMenuCanceled(PopupMenuEvent e) {
          if (cc.isGrabSelected())
            cc.viewport.grabKeyboardHelper(true, true);
        }
        public void popupMenuWillBecomeInvisible(PopupMenuEvent e) {}
        public void popupMenuWillBecomeVisible(PopupMenuEvent e) {}
      });

    updateZoom();
  }

  JMenuItem addMenuItem(String str, int mnemonic) {
    JMenuItem item = new JMenuItem(str, mnemonic);
    item.addActionListener(this);
    add(item);
    return item;
  }

  JMenuItem addMenuItem(String str) {
    JMenuItem item = new JMenuItem(str);
    item.addActionListener(this);
    add(item);
    return item;
  }

  void updateMenuKey() {
    f8.setText("Send " + cc.params.menuKey.getStr());
    KeyStroke ks = KeyStroke.getKeyStroke(cc.params.menuKey.getVKeyCode(), 0);
    f8.setAccelerator(ks);
  }

  boolean actionMatch(ActionEvent ev, JMenuItem item) {
    if (item == null)
      return false;
    return ev.getActionCommand().equals(item.getActionCommand());
  }

  public void actionPerformed(ActionEvent ev) {
    if (!cc.params.noNewConn.get() && actionMatch(ev, exit)) {
      cc.close();
    } else if (actionMatch(ev, fullScreen)) {
      cc.toggleFullScreen();
    } else if (actionMatch(ev, showToolbar)) {
      cc.toggleToolbar();
      showToolbar.setSelected(cc.params.toolbar.get());
    } else if (actionMatch(ev, defaultSize)) {
      cc.sizeWindow();
      firePopupMenuCanceled();
    } else if (actionMatch(ev, zoomIn)) {
      cc.zoomIn();
      firePopupMenuCanceled();
    } else if (actionMatch(ev, zoomOut)) {
      cc.zoomOut();
      firePopupMenuCanceled();
    } else if (actionMatch(ev, zoom100)) {
      cc.zoom100();
      firePopupMenuCanceled();
    } else if (actionMatch(ev, tileWindows)) {
      VncViewer.tileWindows();
    } else if (actionMatch(ev, clipboard)) {
      cc.clipboardDialog.showDialog(cc.viewport);
    } else if (actionMatch(ev, viewOnly)) {
      cc.toggleViewOnly();
    } else if (actionMatch(ev, grabKeyboard)) {
      if (cc.viewport != null)
        cc.viewport.grabKeyboardHelper(grabKeyboard.isSelected());
    } else if (actionMatch(ev, f8)) {
      cc.writeKeyEvent(cc.params.menuKey.getKeySym(), true);
      cc.writeKeyEvent(cc.params.menuKey.getKeySym(), false);
      firePopupMenuCanceled();
    } else if (actionMatch(ev, ctrlAltDel)) {
      cc.writeKeyEvent(Keysyms.CONTROL_L, true);
      cc.writeKeyEvent(Keysyms.ALT_L, true);
      cc.writeKeyEvent(Keysyms.DELETE, true);
      cc.writeKeyEvent(Keysyms.DELETE, false);
      cc.writeKeyEvent(Keysyms.ALT_L, false);
      cc.writeKeyEvent(Keysyms.CONTROL_L, false);
      firePopupMenuCanceled();
    } else if (actionMatch(ev, ctrlEsc)) {
      cc.writeKeyEvent(Keysyms.CONTROL_L, true);
      cc.writeKeyEvent(Keysyms.ESCAPE, true);
      cc.writeKeyEvent(Keysyms.ESCAPE, false);
      cc.writeKeyEvent(Keysyms.CONTROL_L, false);
      firePopupMenuCanceled();
    } else if (actionMatch(ev, refresh)) {
      cc.refresh();
      firePopupMenuCanceled();
    } else if (actionMatch(ev, losslessRefresh)) {
      cc.losslessRefresh();
      firePopupMenuCanceled();
    } else if (actionMatch(ev, screenshot)) {
      cc.screenshot();
    } else if (!cc.params.noNewConn.get() && actionMatch(ev, newConn)) {
      VncViewer.newViewer(cc.viewer);
    } else if (actionMatch(ev, options)) {
      cc.options.showDialog(cc.viewport);
    } else if (actionMatch(ev, info)) {
      cc.showInfo();
    } else if (actionMatch(ev, profile)) {
      if (cc.profileDialog.isVisible())
        cc.profileDialog.endDialog();
      else
        cc.profileDialog.showDialog(cc.viewport);
      cc.toggleProfile();
    } else if (actionMatch(ev, about)) {
      cc.showAbout();
    } else if (actionMatch(ev, dismiss)) {
      firePopupMenuCanceled();
    }
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
  JMenuItem exit, clipboard, ctrlAltDel, ctrlEsc, refresh, losslessRefresh;
  JMenuItem newConn, options, info, profile, screenshot, about, dismiss;
  static JMenuItem f8;
  JCheckBoxMenuItem fullScreen, showToolbar, viewOnly, grabKeyboard;

  static LogWriter vlog = new LogWriter("F8Menu");
}
