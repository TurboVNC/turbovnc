/* Copyright (C) 2012-2015, 2018, 2020, 2022 D. R. Commander.
 *                                           All Rights Reserved.
 * Copyright (C) 2011-2013 Brian P. Hinz
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
import javax.swing.*;
import javax.swing.border.*;
import javax.swing.WindowConstants.*;
import java.util.*;

import com.turbovnc.rdr.*;
import com.turbovnc.rfb.*;

class ServerDialog extends Dialog implements ActionListener {

  ServerDialog(OptionsDialog options_, Params params_, CConn cc_) {

    super(true);
    cc = cc_;
    params = params_;

    options = options_;

    JLabel serverLabel = new JLabel("VNC server:", JLabel.RIGHT);
    String valueStr = null;
    if ((valueStr = UserPreferences.get("ServerDialog", "history")) != null) {
      server = new JComboBox(valueStr.split(","));
    } else {
      server = new JComboBox();
    }
    JLabel sessionLabel =
      new JLabel(Utils.getBooleanProperty("turbovnc.sessmgr", true) ?
                 "host:displayNum, host::port, host::uds_path = " +
                 "connect to VNC server" : "");
    JLabel sessionLabel2 =
      new JLabel(Utils.getBooleanProperty("turbovnc.sessmgr", true) ?
                 "[user@]host = start TurboVNC Session Manager for host" : "");

    // Hack to set the left inset on editable JComboBox
    if (UIManager.getLookAndFeel().getID().equals("Windows")) {
      server.setBorder(BorderFactory.createCompoundBorder(server.getBorder(),
        BorderFactory.createEmptyBorder(0, 2, 0, 0)));
    } else if (UIManager.getLookAndFeel().getID().equals("Metal")) {
      ComboBoxEditor cbEditor = server.getEditor();
      JTextField jtf = (JTextField)cbEditor.getEditorComponent();
      jtf.setBorder(new CompoundBorder(jtf.getBorder(),
                                       new EmptyBorder(0, 2, 0, 0)));
    }

    server.setEditable(true);
    editor = server.getEditor();
    filterWhitespace((JTextField)editor.getEditorComponent());
    editor.getEditorComponent().addKeyListener(new KeyListener() {
      public void keyTyped(KeyEvent e) { updateButtons(); }
      public void keyReleased(KeyEvent e) { updateButtons(); }
      public void keyPressed(KeyEvent e) {
        updateButtons();
        if (e.getKeyCode() == KeyEvent.VK_ENTER && okButton.isEnabled()) {
          if (commit())
            endDialog();
        }
      }
    });
    if (params.server.get() != null)
      server.setSelectedItem(params.server.get());

    topPanel = new JPanel(new GridBagLayout());

    Dialog.addGBComponent(new JLabel(VncViewer.LOGO_ICON), topPanel,
                          0, 0, 1, 3, 0, 0, 0, 1,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.LINE_START,
                          new Insets(5, 5, 5, 15));
    Dialog.addGBComponent(serverLabel, topPanel,
                          1, 0, 1, 1, 0, 0, 0, 1,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.LINE_END,
                          new Insets(10, 0, 5, 5));
    Dialog.addGBComponent(server, topPanel,
                          2, 0, 1, 1, 0, 0, 1, 1,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.CENTER,
                          new Insets(10, 0, 5, 20));
    Dialog.addGBComponent(sessionLabel, topPanel,
                          1, 1, 2, 1, 0, 0, 1, 1,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.CENTER,
                          new Insets(0, 0, 0, 20));
    Dialog.addGBComponent(sessionLabel2, topPanel,
                          1, 2, 2, 1, 0, 0, 1, 1,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.CENTER,
                          new Insets(0, 0, 0, 20));

    optionsButton = new JButton("Options...");
    aboutButton = new JButton("About...");
    okButton = new JButton("Connect");
    updateButtons();
    cancelButton = new JButton("Cancel");
    buttonPanel = new JPanel(new GridBagLayout());
    buttonPanel.setPreferredSize(new Dimension(350, 40));
    Dialog.addGBComponent(aboutButton, buttonPanel,
                          0, 3, 1, 1, 0, 0, 0.8, 1,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.CENTER,
                          new Insets(0, 2, 0, 5));
    Dialog.addGBComponent(optionsButton, buttonPanel,
                          1, 3, 1, 1, 0, 0, 1, 1,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.CENTER,
                          new Insets(0, 2, 0, 5));
    Dialog.addGBComponent(okButton, buttonPanel,
                          2, 3, 1, 1, 0, 0, 0.7, 1,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.CENTER,
                          new Insets(0, 2, 0, 5));
    Dialog.addGBComponent(cancelButton, buttonPanel,
                          3, 3, 1, 1, 0, 0, 0.6, 1,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.CENTER,
                          new Insets(0, 2, 0, 5));

    server.addActionListener(this);
    optionsButton.addActionListener(this);
    aboutButton.addActionListener(this);
    okButton.addActionListener(this);
    cancelButton.addActionListener(this);
  }

  protected void populateDialog(JDialog dlg) {
    dlg.setDefaultCloseOperation(JDialog.DO_NOTHING_ON_CLOSE);
    dlg.setResizable(false);
    dlg.setSize(new Dimension(350, 135));
    dlg.setTitle("New TurboVNC Connection");

    dlg.addWindowListener(new WindowAdapter() {
      public void windowClosing(WindowEvent e) {
        if (VncViewer.nViewers == 1) {
          cc.viewer.exit(1);
        } else {
          ret = false;
          endDialog();
        }
      }
    });

    dlg.getContentPane().setLayout(new GridBagLayout());
    GridBagConstraints gbc = new GridBagConstraints();
    gbc.anchor = GridBagConstraints.LINE_START;
    gbc.fill = GridBagConstraints.BOTH;
    gbc.gridwidth = GridBagConstraints.REMAINDER;
    gbc.gridheight = 1;
    gbc.insets = new Insets(0, 0, 0, 0);
    gbc.ipadx = 0;
    gbc.ipady = 0;
    gbc.weightx = 1;
    gbc.weighty = 1;

    dlg.getContentPane().add(topPanel, gbc);
    dlg.getContentPane().add(buttonPanel);
    dlg.pack();
  }

  private void updateButtons() {
    okButton.setEnabled(editor.getItem() != null &&
                        ((String)editor.getItem()).length() > 0);
    optionsButton.setEnabled(editor.getItem() != null &&
                             ((String)editor.getItem()).length() > 0);
  }

  public void actionPerformed(ActionEvent e) {
    Object s = e.getSource();
    if (s instanceof JButton && (JButton)s == okButton) {
      if (commit())
        endDialog();
    } else if (s instanceof JButton && (JButton)s == cancelButton) {
      if (VncViewer.nViewers == 1)
        cc.viewer.exit(1);
      ret = false;
      endDialog();
    } else if (s instanceof JButton && (JButton)s == optionsButton) {
      if (editor.getItem() != null &&
          ((String)editor.getItem()).length() > 0) {
        String serverStr = (String)editor.getItem();
        int atIndex = serverStr.lastIndexOf('@');
        if (atIndex >= 0) serverStr = serverStr.substring(atIndex + 1);
        serverStr = Hostname.getHost(serverStr);
        UserPreferences.load(serverStr, params);
        options.setNode(serverStr);
        options.showDialog(getJDialog());
      }
      if (UserPreferences.get("ServerDialog", "history") == null) {
        String serverStr = (String)editor.getItem();
        server.removeAllItems();
        if (serverStr != null && serverStr.length() > 0)
          ((JTextField)editor.getEditorComponent()).setText(serverStr);
        updateButtons();
      }
    } else if (s instanceof JButton && (JButton)s == aboutButton) {
      VncViewer.showAbout(getJDialog());
    } else if (s instanceof JComboBox && (JComboBox)s == server) {
      if (e.getActionCommand().equals("comboBoxEdited") ||
          e.getActionCommand().equals("comboBoxChanged"))
        updateButtons();
    }
  }

  private boolean commit() {
    try {

      String serverName = (String)editor.getItem();
      if (serverName == null || serverName.equals(""))
        throw new WarningException("No server name specified");

      // set params
      if (params.via.get() != null && params.via.get().indexOf(':') >= 0) {
        params.server.set(serverName);
      } else {
        params.server.set(Hostname.getHost(serverName));
        params.port.set(Hostname.getPort(serverName));
        params.udsPath = Hostname.getUDSPath(serverName);
      }

      // Update the history list
      String valueStr = UserPreferences.get("ServerDialog", "history");
      String t = (valueStr == null) ? "" : valueStr;
      StringTokenizer st = new StringTokenizer(t, ",");
      StringBuffer sb = new StringBuffer().append(serverName);
      while (st.hasMoreTokens()) {
        String str = st.nextToken();
        if (!str.equals(serverName) && !str.equals("")) {
          sb.append(',');
          sb.append(str);
        }
      }
      UserPreferences.set("ServerDialog", "history", sb.toString());
      UserPreferences.save("ServerDialog");

    } catch (Exception e) {
      cc.viewer.reportException(e, false);
      return false;
    }

    return true;
  }

  Window win;
  CConn cc;
  Params params;
  JComboBox server;
  ComboBoxEditor editor;
  JPanel topPanel, buttonPanel;
  JButton aboutButton, optionsButton, okButton, cancelButton;
  OptionsDialog options;

  static LogWriter vlog = new LogWriter("ServerDialog");
}
