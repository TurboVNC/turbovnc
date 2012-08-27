/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright (C) 2011 Brian P. Hinz
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

// On Mac, we can add items to the actual menu bar, so this provides a more
// Mac-like L&F than using the F8 menu.

package com.turbovnc.vncviewer;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

import com.turbovnc.rfb.*;

public class MacMenuBar extends JMenuBar implements ActionListener
{
  public MacMenuBar(CConn cc) {
    int acceleratorMask = Toolkit.getDefaultToolkit().getMenuShortcutKeyMask();

    // Theoretically, we should be able to add this to the actual Mac app menu,
    // but doing so requires using com.apple.eawt, which is not cross-platform.
    // The Macify wrapper enables the use of com.apple.eawt in a cross-platform
    // manner (by dynamically loading it with Reflection), but unfortunately
    // Macify is not GPL-compatible.  :(
    JMenu mainMenu = new JMenu("TurboVNC Viewer");
    about = addMenuItem(mainMenu, "About TurboVNC Viewer");
    about.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_A,
                                                acceleratorMask));
    mainMenu.addSeparator();
    options = addMenuItem(mainMenu, "Preferences");
    options.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_COMMA,
                                                  acceleratorMask));

    JMenu connMenu = new JMenu("Connection");
    newConn = addMenuItem(connMenu, "New Connection...");
    newConn.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_N,
                                                  acceleratorMask));
    connMenu.addSeparator();
    info = addMenuItem(connMenu, "Connection Info...");
    info.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_I,
                                               acceleratorMask));
    connMenu.addSeparator();
    refresh = addMenuItem(connMenu, "Request Screen Refresh");
    refresh.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_R,
                                                  acceleratorMask));
    connMenu.addSeparator();
    fullScreen = new JCheckBoxMenuItem("Full Screen");
    fullScreen.setSelected(cc.fullScreen);
    fullScreen.addActionListener(this);
    connMenu.add(fullScreen);
    fullScreen.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_F,
                                                     acceleratorMask));
    connMenu.addSeparator();
    ctrlAltDel = addMenuItem(connMenu, "Send Ctrl-Alt-Del");
    ctrlEsc = addMenuItem(connMenu, "Send Ctrl-Esc");
    connMenu.addSeparator();
    clipboard = addMenuItem(connMenu, "Clipboard ...");

    add(mainMenu);
    add(connMenu);

    this.cc = cc;
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
    System.out.println(ev.getActionCommand());
    if (actionMatch(ev, fullScreen)) {
      cc.toggleFullScreen();
    } else if (actionMatch(ev, clipboard)) {
      cc.clipboardDialog.showDialog(cc.viewport);
    } else if (actionMatch(ev, ctrlAltDel)) {
      cc.writeKeyEvent(Keysyms.Control_L, true);
      cc.writeKeyEvent(Keysyms.Alt_L, true);
      cc.writeKeyEvent(Keysyms.Delete, true);
      cc.writeKeyEvent(Keysyms.Delete, false);
      cc.writeKeyEvent(Keysyms.Alt_L, false);
      cc.writeKeyEvent(Keysyms.Control_L, false);
    } else if (actionMatch(ev, ctrlEsc)) {
      cc.writeKeyEvent(Keysyms.Control_L, true);
      cc.writeKeyEvent(Keysyms.Escape, true);
      cc.writeKeyEvent(Keysyms.Escape, false);
      cc.writeKeyEvent(Keysyms.Control_L, false);
    } else if (actionMatch(ev, refresh)) {
      cc.refresh();
    } else if (actionMatch(ev, newConn)) {
      VncViewer.newViewer(cc.viewer);
    } else if (actionMatch(ev, options)) {
      cc.options.showDialog(cc.viewport);
    } else if (actionMatch(ev, info)) {
      cc.showInfo();
    } else if (actionMatch(ev, about)) {
      cc.showAbout();
    }
  }

  CConn cc;
  JMenuItem clipboard, ctrlAltDel, ctrlEsc, refresh;
  JMenuItem newConn, options, info, about;
  JCheckBoxMenuItem fullScreen;
}
