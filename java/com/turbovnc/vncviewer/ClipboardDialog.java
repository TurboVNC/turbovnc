/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright (C) 2011-2012 Brian P. Hinz
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
import java.awt.datatransfer.Clipboard;
import java.awt.datatransfer.StringSelection;
import javax.swing.*;
import javax.swing.border.*;

import com.turbovnc.rdr.*;
import com.turbovnc.rfb.LogWriter;

class ClipboardDialog implements ActionListener {

  public ClipboardDialog(CConn cc_) {
    cc = cc_;

    pt = new JPanel();
    pt.setLayout(new BoxLayout(pt, BoxLayout.LINE_AXIS));
    pt.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
    textArea = new JTextArea();
    textArea.setLineWrap(false);
    textArea.setWrapStyleWord(false);
    JScrollPane sp = new JScrollPane(textArea);
    sp.setPreferredSize(new Dimension(300, 250));
    pt.add(sp);

    pb = new JPanel();
    clearButton = new JButton("Clear");
    pb.add(clearButton);
    clearButton.addActionListener(this);
    sendButton = new JButton("Send to VNC server");
    pb.add(sendButton);
    sendButton.addActionListener(this);
    cancelButton = new JButton("Cancel");
    pb.add(cancelButton);
    cancelButton.addActionListener(this);
  }

  public void showDialog(Window w) {
    if (w instanceof Frame)
      dlg = new Dialog((Frame)w, false);
    else
      throw new ErrorException("Unknown window type");

    dlg.setTitle("VNC clipboard");
    dlg.getContentPane().add("Center", pt);
    dlg.getContentPane().add("South", pb);
    dlg.pack();
    dlg.setMinimumSize(dlg.getSize());

    dlg.showDialog(w);
  }

  public void initDialog() {
    textArea.setText(current);
    textArea.selectAll();
  }

  public void setContents(String str) {
    current = str;
    textArea.setText(str);
    textArea.selectAll();
  }

  public void serverCutText(String str, int len) {
    setContents(str);
    SecurityManager sm = System.getSecurityManager();
    try {
      if (sm != null) sm.checkSystemClipboardAccess();
      Clipboard cb = Toolkit.getDefaultToolkit().getSystemClipboard();
      if (cb != null) {
        StringSelection ss = new StringSelection(str);
        try {
          cb.setContents(ss, ss);
        } catch(Exception e) {
          vlog.debug(e.toString());
        }
      }
    } catch(SecurityException e) {
      System.err.println("Cannot access the system clipboard");
    }
  }

  public void setSendingEnabled(boolean b) {
    sendButton.setEnabled(b);
  }

  private void endDialog() {
    if (dlg != null) {
      dlg.endDialog();
      dlg.dispose();
      dlg = null;
    }
  }

  public void actionPerformed(ActionEvent e) {
    Object s = e.getSource();
    if (s instanceof JButton && (JButton)s == clearButton) {
      current = "";
      textArea.setText(current);
    } else if (s instanceof JButton && (JButton)s == sendButton) {
      current = textArea.getText();
      cc.writeClientCutText(current, current.length());
      endDialog();
    } else if (s instanceof JButton && (JButton)s == cancelButton) {
      endDialog();
    }
  }

  Dialog dlg;
  CConn cc;
  String current;
  JPanel pt, pb;
  JTextArea textArea;
  JButton clearButton, sendButton, cancelButton;
  static LogWriter vlog = new LogWriter("ClipboardDialog");
}
