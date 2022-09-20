/* Copyright (C) 2012-2014, 2016, 2018 D. R. Commander.  All Rights Reserved.
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

// This Dialog class implements a pop-up dialog.  The dialog can be made
// visible by calling its showDialog() method.  Dialogs can be modal or
// non-modal.  For a modal dialog box, the showDialog() method must be called
// from a thread other than the GUI thread, and it only returns when the dialog
// box has been dismissed.  For a non-modal dialog box, the showDialog() method
// returns immediately.

package com.turbovnc.vncviewer;

import java.awt.*;
import java.awt.Dialog.*;
import javax.swing.*;
import javax.swing.text.*;

import com.turbovnc.rdr.*;
import com.turbovnc.rfb.LogWriter;

class Dialog {

  Dialog(boolean modal_) {
    modal = modal_;
  }

  public boolean showDialog(Window w, String title) {
    initDialog();

    if (w == null) {
      // Try to assign the dialog a null owner, so it will always appear in the
      // taskbar and task switcher.
      try {
        dlg = new JDialog((java.awt.Dialog)null, modal);
      } catch (Exception e) {
      }
      // That didn't work, perhaps because we're running under Java 1.5, so
      // fall back to assigning the shared frame as the owner.  This is OK
      // under Java 1.5, because Java 1.5 always registers ownerless dialogs
      // with the task switcher.
      if (dlg == null)
        dlg = new JDialog((Frame)null, modal);
    } else if (w instanceof Frame) {
      dlg = new JDialog((Frame)w, modal);
    } else if (w instanceof java.awt.Dialog) {
      dlg = new JDialog((java.awt.Dialog)w, modal);
    } else {
      throw new ErrorException("Unknown window type");
    }

    // Under Java 6 and later, ownerless JDialogs in the TurboVNC Viewer are
    // created with a null owner to ensure that they appear in the task
    // switcher on Linux.  However, this necessitates using
    // JDialog.setIconImage() to set the icon image.
    dlg.setIconImage(VncViewer.FRAME_IMAGE);

    populateDialog(dlg);
    if (title != null)
      dlg.setTitle(title);

    if (w != null) {
      dlg.setLocationRelativeTo(w);
    } else {
      Dimension dpySize = dlg.getToolkit().getScreenSize();
      Dimension mySize = dlg.getSize();
      int x = (dpySize.width - mySize.width) / 2;
      int y = (dpySize.height - mySize.height) / 2;
      dlg.setLocation(x, y);
    }

    Window owner = dlg.getOwner();
    if (owner instanceof java.awt.Dialog) {
      if (owner.isAlwaysOnTop())
        dlg.setAlwaysOnTop(true);
    }
    if (!modal)
      dlg.setAlwaysOnTop(true);
    dlg.setVisible(true);
    return ret;
  }

  public boolean showDialog() {
    return showDialog(null, null);
  }

  public boolean showDialog(Window w) {
    return showDialog(w, null);
  }

  public boolean showDialog(String title) {
    return showDialog(null, title);
  }

  public boolean isShown() {
    return (dlg != null && dlg.isVisible());
  }

  public void endDialog() {
    if (dlg != null) {
      dlg.dispose();
      dlg = null;
    }
  }

  JDialog getJDialog() {
    return dlg;
  }

  // initDialog() can be overridden in a derived class.  Typically it is used
  // to make sure that checkboxes have the right state, etc.
  public void initDialog() {
  }

  // populateDialog() can be overridden in a derived class.  Typically it is
  // used to add pre-initialized components to the dialog instance.
  protected void populateDialog(JDialog dialog) {
  }

  public static void addGBComponent(JComponent c, JComponent cp,
                                    int gx, int gy,
                                    int gw, int gh,
                                    int gipx, int gipy,
                                    double gwx, double gwy,
                                    int fill, int anchor,
                                    Insets insets) {
    GridBagConstraints gbc = new GridBagConstraints();
    gbc.anchor = anchor;
    gbc.fill = fill;
    gbc.gridx = gx;
    gbc.gridy = gy;
    gbc.gridwidth = gw;
    gbc.gridheight = gh;
    gbc.insets = insets;
    gbc.ipadx = gipx;
    gbc.ipady = gipy;
    gbc.weightx = gwx;
    gbc.weighty = gwy;
    cp.add(c, gbc);
  }

  class WhitespaceDocumentFilter extends DocumentFilter {
    public void insertString(DocumentFilter.FilterBypass fb, int offset,
                             String string, AttributeSet attr)
                             throws BadLocationException {
      if (string != null)
        string = string.replaceAll("\\s", "");
      super.insertString(fb, offset, string, attr);
    }

    public void replace(DocumentFilter.FilterBypass fb, int offset, int length,
                        String text, AttributeSet attrs)
                        throws BadLocationException {
      if (text != null)
        text = text.replaceAll("\\s", "");
      super.replace(fb, offset, length, text, attrs);
    }
  };

  public void filterWhitespace(JTextField textField) {
    DocumentFilter filter = new WhitespaceDocumentFilter();
    ((AbstractDocument)textField.getDocument()).setDocumentFilter(filter);
  }

  private JDialog dlg;

  protected boolean ret = true;
  boolean modal;

  static LogWriter vlog = new LogWriter("Dialog");
}
