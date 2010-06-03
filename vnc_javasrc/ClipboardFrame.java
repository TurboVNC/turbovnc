//
//  Copyright (C) 2010 D. R. Commander.  All Rights Reserved.
//  Copyright (C) 2001 HorizonLive.com, Inc.  All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
//
//  This is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this software; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//

//
// Clipboard frame.
//

import java.awt.*;
import java.awt.event.*;
import java.awt.datatransfer.*;

class ClipboardFrame extends Frame
  implements WindowListener, ActionListener {

  TextArea textArea;
  Button clearButton, closeButton;
  String selection;
  VncViewer viewer;

  static Clipboard systemClipboard = null;
  static String systemClipboardText = null;
  static Clipboard systemSelection = null;
  static String systemSelectionText = null;
  static {
    try {
      // Used for Windows and explicit Copy/Paste in Linux ("CLIPBOARD Selection")
      systemClipboard = Toolkit.getDefaultToolkit().getSystemClipboard();
      if (systemClipboard != null) {
        Transferable contents = systemClipboard.getContents(null);
        if (contents != null && contents.isDataFlavorSupported(DataFlavor.stringFlavor)) {
          systemClipboardText = (String)(contents.getTransferData(DataFlavor.stringFlavor));
        }
      }
    } catch (Exception e) {
        System.out.println("WARNING: Could not get system clipboard, and thus automatic clipboard transfer");
        System.out.println("   will not work.  This may be due to the applet being unsigned.");
    }
    try {
      // Used for Select/Middle-Click in Linux ("PRIMARY Selection")
      systemSelection = Toolkit.getDefaultToolkit().getSystemSelection();
      if (systemSelection != null) {
        Transferable contents = systemSelection.getContents(null);
        if (contents != null && contents.isDataFlavorSupported(DataFlavor.stringFlavor)) {
          systemSelectionText = (String)(contents.getTransferData(DataFlavor.stringFlavor));
        }
      }
    } catch (Exception e) {
        System.out.println("WARNING: Could not get system clipboard, and thus automatic clipboard transfer");
        System.out.println("   will not work.  This may be due to the applet being unsigned.");
    }
  }

  //
  // Constructor.
  //

  ClipboardFrame(VncViewer v) {
    super("TurboVNC Clipboard");

    viewer = v;

    GridBagLayout gridbag = new GridBagLayout();
    setLayout(gridbag);

    GridBagConstraints gbc = new GridBagConstraints();
    gbc.gridwidth = GridBagConstraints.REMAINDER;
    gbc.fill = GridBagConstraints.BOTH;
    gbc.weighty = 1.0;

    textArea = new TextArea("", 5, 40, TextArea.SCROLLBARS_NONE);
    gridbag.setConstraints(textArea, gbc);
    add(textArea);

    gbc.fill = GridBagConstraints.HORIZONTAL;
    gbc.weightx = 1.0;
    gbc.weighty = 0.0;
    gbc.gridwidth = 1;

    clearButton = new Button("Clear");
    gridbag.setConstraints(clearButton, gbc);
    add(clearButton);
    clearButton.addActionListener(this);

    closeButton = new Button("Close");
    gridbag.setConstraints(closeButton, gbc);
    add(closeButton);
    closeButton.addActionListener(this);

    pack();

    addWindowListener(this);
  }


  // Send the given cut text to the RFB server.
  void sendCutText(String text) {
    try {
      if (viewer.rfb != null && viewer.rfb.inNormalProtocol) {
	viewer.rfb.writeClientCutText(text);
      }
    } catch (Exception e) {
      e.printStackTrace();
    }
  }

  // Called when cut text is received from the RFB server.
  void recvCutText(String text) {
    setContents(text);
    setLocalClipboard(text);
  }


  // Set the window contents.
  void setContents(String text) {
    selection = text;
    textArea.setText(text);
    if (isVisible()) {
      textArea.selectAll();
    }
  }

  // Set the local clipboard's contents.
  // (In Linux, set both clipboards)
  void setLocalClipboard(String text) {
    if (viewer.syncClipboards) {
      StringSelection ss = new StringSelection(text);
      if (systemClipboard != null) {
        systemClipboard.setContents(ss, ss);
      }
      if (systemSelection != null) {
        systemSelection.setContents(ss, ss);
      }
    }
  }


  // Check for updates to the local clipboard.
  // (In Linux, if both clipboards changed, prefer the explicit Copy/Paste clipboard)
  void checkLocalClipboard() {
    if (viewer.syncClipboards) {
      boolean clipboardChanged = false;
      boolean selectionChanged = false;
      if (systemClipboard != null) {
        try {
          Transferable contents = systemClipboard.getContents(this);
          if (contents != null && contents.isDataFlavorSupported(DataFlavor.stringFlavor)) {
            String text = (String)(contents.getTransferData(DataFlavor.stringFlavor));
            if (text != null && (systemClipboardText == null || !text.equals(systemClipboardText))) {
              systemClipboardText = text;
              clipboardChanged = true;
            }
          }
        } catch (Exception e) {}
      }
      if (systemSelection != null) {
        try {
          Transferable contents = systemSelection.getContents(this);
          if (contents != null && contents.isDataFlavorSupported(DataFlavor.stringFlavor)) {
            String text = (String)(contents.getTransferData(DataFlavor.stringFlavor));
            if (text != null && (systemSelectionText == null || !text.equals(systemSelectionText))) {
              systemSelectionText = text;
              selectionChanged = true;
            }
          }
        } catch (Exception e) {}
      }
      if (clipboardChanged) {
        sendCutText(systemClipboardText);
        setContents(systemClipboardText);
      } else if (selectionChanged) {
        sendCutText(systemSelectionText);
        setContents(systemSelectionText);
      }
    }
  }


  //
  // When the focus enters the window, check for updates to the local clipboard.
  //

  public void windowActivated(WindowEvent evt) {
    checkLocalClipboard();
  }

  //
  // When the focus leaves the window, see if we have new cut text and
  // if so send it to the RFB server.
  //

  public void windowDeactivated (WindowEvent evt) {
    if (selection != null && !selection.equals(textArea.getText())) {
      selection = textArea.getText();
      sendCutText(selection);
      setLocalClipboard(selection);
    }
  }

  //
  // Close our window properly.
  //

  public void windowClosing(WindowEvent evt) {
    setVisible(false);
  }

  //
  // Ignore window events we're not interested in.
  //

  public void windowOpened(WindowEvent evt) {}
  public void windowClosed(WindowEvent evt) {}
  public void windowIconified(WindowEvent evt) {}
  public void windowDeiconified(WindowEvent evt) {}


  //
  // Respond to button presses
  //

  public void actionPerformed(ActionEvent evt) {
    if (evt.getSource() == clearButton) {
      sendCutText("");
      setContents("");
      setLocalClipboard("");
    } else if (evt.getSource() == closeButton) {
      setVisible(false);
    }
  }
}
