/* Copyright (C) 2018 D. R. Commander.  All Rights Reserved.
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

class SessionManagerDialog extends Dialog implements ActionListener {

  SessionManagerDialog(String[] sessions_, String host_) {
    super(true);
    sessions = sessions_;
    host = host_;

    sessionManagerPanel = new JPanel(new GridBagLayout());
    sessionManagerPanel.setBorder(BorderFactory.createEmptyBorder(0, 0, 0, 0));

    if (sessions != null) {
      sessionsPanel = new JPanel(new GridBagLayout());
      sessionsPanel.setBorder(BorderFactory.createEmptyBorder(0, 0, 0, 0));

      for (int i = 0;
           i < Math.min(sessions.length, SessionManager.MAX_SESSIONS); i++) {
        JLabel displayLabel = new JLabel(host + sessions[i]);
        connectButton[i] = new JButton("Connect");
        connectButton[i].addActionListener(this);
        newOTPButton[i] = new JButton("New OTP");
        newOTPButton[i].addActionListener(this);
        viewOnlyBox[i] = new JCheckBox("View-only");
        killButton[i] = new JButton("Kill");
        killButton[i].addActionListener(this);

        Dialog.addGBComponent(displayLabel, sessionsPanel,
                              0, i, 1, 1, 0, 0, 1, 1,
                              GridBagConstraints.HORIZONTAL,
                              GridBagConstraints.CENTER,
                              new Insets(0, 2, 0, 5));
        Dialog.addGBComponent(connectButton[i], sessionsPanel,
                              1, i, 1, 1, 0, 0, 1, 1,
                              GridBagConstraints.HORIZONTAL,
                              GridBagConstraints.CENTER,
                              new Insets(0, 2, 0, 5));
        Dialog.addGBComponent(newOTPButton[i], sessionsPanel,
                              2, i, 1, 1, 0, 0, 1, 1,
                              GridBagConstraints.HORIZONTAL,
                              GridBagConstraints.CENTER,
                              new Insets(0, 2, 0, 5));
        Dialog.addGBComponent(viewOnlyBox[i], sessionsPanel,
                              3, i, 1, 1, 0, 0, 1, 1,
                              GridBagConstraints.HORIZONTAL,
                              GridBagConstraints.CENTER,
                              new Insets(0, 2, 0, 5));
        Dialog.addGBComponent(killButton[i], sessionsPanel,
                              4, i, 1, 1, 0, 0, 1, 1,
                              GridBagConstraints.HORIZONTAL,
                              GridBagConstraints.CENTER,
                              new Insets(0, 2, 0, 5));
      }

      scrollPane = new JScrollPane(sessionsPanel);
      scrollPane.setHorizontalScrollBarPolicy(
        ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
      scrollPane.setBorder(BorderFactory.createTitledBorder("Sessions"));
      scrollPane.setBackground(sessionsPanel.getBackground());
      Dimension spSize = scrollPane.getPreferredSize();
      spSize.width += ((Integer)UIManager.get("ScrollBar.width")).intValue();
      scrollPane.setPreferredSize(spSize);
    }

    newSessionButton = new JButton("New session");
    newSessionButton.addActionListener(this);
    Dialog.addGBComponent(newSessionButton, sessionManagerPanel,
                          0, 1, 1, 1, 0, 0, 1, 1,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.CENTER,
                          new Insets(0, 2, 0, 5));

    refreshButton = new JButton("Refresh");
    refreshButton.addActionListener(this);
    Dialog.addGBComponent(refreshButton, sessionManagerPanel,
                          1, 1, 1, 1, 0, 0, 1, 1,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.CENTER,
                          new Insets(0, 2, 0, 5));

    cancelButton = new JButton("Cancel");
    cancelButton.addActionListener(this);
    Dialog.addGBComponent(cancelButton, sessionManagerPanel,
                          2, 1, 1, 1, 0, 0, 1, 1,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.CENTER,
                          new Insets(0, 2, 0, 5));
  }

  protected void populateDialog(JDialog dlg) {
    dlg.setResizable(true);
    dlg.setTitle("TurboVNC Session Manager");

    dlg.addWindowListener(new WindowAdapter() {
      public void windowClosing(WindowEvent e) {
        ret = false;
      }
    });

    dlg.getContentPane().setLayout(
      new BoxLayout(dlg.getContentPane(), BoxLayout.PAGE_AXIS));
    ((JPanel)dlg.getContentPane()).setBorder(
      BorderFactory.createEmptyBorder(5, 5, 5, 5));
    if (scrollPane != null)
      dlg.getContentPane().add(scrollPane);
    dlg.getContentPane().add(sessionManagerPanel);
    dlg.pack();
  }

  public void actionPerformed(ActionEvent e) {
    Object s = e.getSource();
    if (s instanceof JButton && (JButton)s == newSessionButton) {
      connectSession = "NEW";
      endDialog();
    } else if (s instanceof JButton && (JButton)s == refreshButton) {
      endDialog();
    } else if (s instanceof JButton && (JButton)s == cancelButton) {
      ret = false;
      endDialog();
    }

    if (sessions != null) {
      for (int i = 0;
           i < Math.min(sessions.length, SessionManager.MAX_SESSIONS); i++) {
        if (s instanceof JButton && (JButton)s == connectButton[i]) {
          connectSession = sessions[i];
          viewOnly = viewOnlyBox[i].isSelected();
          endDialog();
        } else if (s instanceof JButton && (JButton)s == killButton[i]) {
          String message = "Are you sure you want to kill session\n" +
                           host + sessions[i] + " ?";
          String[] options = { "Yes", "No" };
          if (JOptionPane.showOptionDialog(getJDialog(), message,
                                           "Are you sure?",
                                           JOptionPane.YES_NO_OPTION,
                                           JOptionPane.WARNING_MESSAGE, null,
                                           options, options[1]) ==
              JOptionPane.YES_OPTION) {
            killSession = sessions[i];
            endDialog();
          }
        } else if (s instanceof JButton && (JButton)s == newOTPButton[i]) {
          newOTPSession = sessions[i];
          viewOnly = viewOnlyBox[i].isSelected();
          endDialog();
        }
      }
    }
  }

  String getConnectSession() { return connectSession; }

  String getKillSession() { return killSession; }

  String getNewOTPSession() { return newOTPSession; }

  boolean getViewOnly() { return viewOnly; }

  JPanel sessionManagerPanel, sessionsPanel, scrollPanel;
  JScrollPane scrollPane;
  JButton[] connectButton = new JButton[SessionManager.MAX_SESSIONS];
  JButton[] killButton = new JButton[SessionManager.MAX_SESSIONS];
  JButton[] newOTPButton = new JButton[SessionManager.MAX_SESSIONS];
  JCheckBox[] viewOnlyBox = new JCheckBox[SessionManager.MAX_SESSIONS];
  JButton newSessionButton, refreshButton, cancelButton;

  String[] sessions;
  String connectSession, killSession, newOTPSession, host;
  boolean viewOnly;

  static LogWriter vlog = new LogWriter("SessionManagerDialog");
}
