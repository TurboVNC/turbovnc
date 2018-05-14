/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright (C) 2011-2012 Brian P. Hinz
 * Copyright (C) 2012, 2014 D. R. Commander.  All Rights Reserved.
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
import com.jcraft.jsch.*;

class PasswdDialog extends Dialog implements KeyListener, UserInfo,
  UIKeyboardInteractive {

  PasswdDialog(String title_, boolean userDisabled, String userName_,
               boolean passwdDisabled) {
    super(true);
    title = title_;
    userName = userName_;

    p1 = new JPanel();
    userLabel = new JLabel("User name:");
    p1.add(userLabel);
    userEntry = new JTextField(30);
    userEntry.setEnabled(!userDisabled);
    userLabel.setEnabled(!userDisabled);
    if (userName != null)
      userEntry.setText(userName);
    p1.add(userEntry);
    userEntry.addKeyListener(this);

    p2 = new JPanel();
    passwdLabel = new JLabel("Password:");
    passwdLabel.setPreferredSize(userLabel.getPreferredSize());
    p2.add(passwdLabel);
    passwdEntry = new JPasswordField(30);
    passwdEntry.setEnabled(!passwdDisabled);
    passwdLabel.setEnabled(!passwdDisabled);
    p2.add(passwdEntry);
    passwdEntry.addKeyListener(this);
  }

  protected void populateDialog(JDialog dlg) {
    dlg.setResizable(false);
    dlg.setTitle(title);

    dlg.addWindowListener(new WindowAdapter() {
      public void windowClosing(WindowEvent e) {
        endDialog();
      }
    });

    dlg.getContentPane().setLayout(new BoxLayout(dlg.getContentPane(),
                                                 BoxLayout.Y_AXIS));
    dlg.getContentPane().add(p1);
    dlg.getContentPane().add(p2);
    dlg.pack();

    if (userEntry.isEnabled() && userName == null) {
      userEntry.requestFocus();
    } else {
      passwdEntry.requestFocus();
    }
  }

  /** Handle the key-typed event. */
  public void keyTyped(KeyEvent event) {}

  /** Handle the key-released event. */
  public void keyReleased(KeyEvent event) {}

  /** Handle the key-pressed event. */
  public void keyPressed(KeyEvent event) {
    Object s = event.getSource();
    if (s instanceof JTextField && (JTextField)s == userEntry) {
      if (event.getKeyCode() == KeyEvent.VK_ENTER) {
        endDialog();
      }
    } else if (s instanceof JPasswordField &&
               (JPasswordField)s == passwdEntry) {
      if (event.getKeyCode() == KeyEvent.VK_ENTER) {
        endDialog();
      }
    }
  }

  public String getPassword() {
    return new String(passwdEntry.getPassword());
  }

  public String getPassphrase() { return null; }

  public boolean promptPassphrase(String message) { return false; }

  public boolean promptPassword(String message) {
    showDialog(message);
    if (passwdEntry != null)
      return true;
    return false;
  }

  public void showMessage(String message) {
    JOptionPane pane = new JOptionPane(message);
    JDialog dlg = pane.createDialog(null, "SSH Message");
    dlg.setAlwaysOnTop(true);
    dlg.setVisible(true);
  }

  public boolean promptYesNo(String str) {
    Object[] options = { "Yes", "No" };
    JOptionPane pane = new JOptionPane(str, JOptionPane.WARNING_MESSAGE,
                                       JOptionPane.DEFAULT_OPTION, null,
                                       options, options[0]);
    JDialog dlg = pane.createDialog(null, "SSH Message");
    dlg.setAlwaysOnTop(true);
    dlg.setVisible(true);
    return (pane.getValue() == options[0]);
  }

  public String[] promptKeyboardInteractive(String destination,
                                            String name,
                                            String instruction,
                                            String[] prompt,
                                            boolean[] echo) {
    Container panel = new JPanel();
    panel.setLayout(new GridBagLayout());

    GridBagConstraints gbc =
      new GridBagConstraints(0, 0, 1, 1, 1, 1,
                             GridBagConstraints.NORTHWEST,
                             GridBagConstraints.NONE,
                             new Insets(0, 0, 0, 0), 0, 0);
    gbc.weightx = 1.0;
    gbc.gridwidth = GridBagConstraints.REMAINDER;
    gbc.gridx = 0;
    panel.add(new JLabel(instruction), gbc);
    gbc.gridy++;

    gbc.gridwidth = GridBagConstraints.RELATIVE;

    JTextField[] texts = new JTextField[prompt.length];
    for (int i = 0; i < prompt.length; i++) {
      gbc.fill = GridBagConstraints.NONE;
      gbc.gridx = 0;
      gbc.weightx = 1;
      panel.add(new JLabel(prompt[i]), gbc);

      gbc.gridx = 1;
      gbc.fill = GridBagConstraints.HORIZONTAL;
      gbc.weighty = 1;
      if (echo[i]) {
        texts[i] = new JTextField(20);
      } else {
        texts[i] = new JPasswordField(20);
      }
      panel.add(texts[i], gbc);
      gbc.gridy++;
    }

    if (JOptionPane.showConfirmDialog(null, panel, destination + ": " + name,
                                      JOptionPane.OK_CANCEL_OPTION,
                                      JOptionPane.QUESTION_MESSAGE) ==
        JOptionPane.OK_OPTION) {
      String[] response = new String[prompt.length];
      for (int i = 0; i < prompt.length; i++) {
        response[i] = texts[i].getText();
      }
      return response;
    } else {
      return null;  // cancel
    }
  }

  JPanel p1, p2;
  JLabel userLabel;
  JTextField userEntry;
  JLabel passwdLabel;
  JPasswordField passwdEntry;
  String title, userName;
}
