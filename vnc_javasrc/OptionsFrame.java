//
//  Copyright (C) 2006 Sun Microsystems, Inc.  All Rights Reserved.
//  Copyright (C) 2001 HorizonLive.com, Inc.  All Rights Reserved.
//  Copyright (C) 2001 Constantin Kaplinsky.  All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation.  All Rights Reserved.
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
// Options frame.
//
// This deals with all the options the user can play with.
// It sets the encodings array and some booleans.
//

import java.awt.*;
import java.awt.event.*;

class OptionsFrame extends Frame
  implements WindowListener, ActionListener, ItemListener {

  static String[] names = {
    "Connection profile",
    "Image compression type",
    "JPEG chrominance subsampling",
    "JPEG image quality",
    "Cursor shape updates",
    "Use CopyRect",
    "High-latency network",
    "Mouse buttons 2 and 3",
    "View only",
    "Scale remote cursor",
    "Share desktop",
  };

  static String[][] values = {
    { "Broadband (favor performance)", "Broadband (favor image quality)", "High-Speed Network", "Custom" },
    { "None (RGB)", "JPEG" },
    { "Grayscale", "4X", "2X", "None" },
    { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
      "11", "12", "13", "14", "15", "16", "17", "18", "19", "20",
      "21", "22", "23", "24", "25", "26", "27", "28", "29", "30",
      "31", "32", "33", "34", "35", "36", "37", "38", "39", "40",
      "41", "42", "43", "44", "45", "46", "47", "48", "49", "50",
      "51", "52", "53", "54", "55", "56", "57", "58", "59", "60",
      "61", "62", "63", "64", "65", "66", "67", "68", "69", "70",
      "71", "72", "73", "74", "75", "76", "77", "78", "79", "80",
      "81", "82", "83", "84", "85", "86", "87", "88", "89", "90",
      "91", "92", "93", "94", "95", "96", "97", "98", "99", "100" },
    { "Enable", "Ignore", "Disable" },
    { "Yes", "No" },
    { "Yes", "No" },
    { "Normal", "Reversed" },
    { "Yes", "No" },
    { "No", "50%", "75%", "125%", "150%" },
    { "Yes", "No" },
  };

  final int
    presetIndex          = 0,
    compressTypeIndex    = 1,
    compressLevelIndex   = 2,
    jpegQualityIndex     = 3,
    cursorUpdatesIndex   = 4,
    useCopyRectIndex     = 5,
    wanIndex             = 6,
    mouseButtonIndex     = 7,
    viewOnlyIndex        = 8,
    scaleCursorIndex     = 9,
    shareDesktopIndex    = 10;

  Label[] labels = new Label[names.length];
  Choice[] choices = new Choice[names.length];
  Button closeButton;
  VncViewer viewer;


  //
  // The actual data which other classes look at:
  //

  int preferredEncoding;
  int compressLevel;
  int jpegQuality;
  boolean useCopyRect;
  boolean requestCursorUpdates;
  boolean ignoreCursorUpdates;

  boolean wan;

  boolean reverseMouseButtons2And3;
  boolean shareDesktop;
  boolean viewOnly;
  int scaleCursor;

  boolean autoScale;
  int scalingFactor;

  //
  // Constructor.  Set up the labels and choices from the names and values
  // arrays.
  //

  OptionsFrame(VncViewer v) {
    super("TurboVNC Options");

    viewer = v;

    GridBagLayout gridbag = new GridBagLayout();
    setLayout(gridbag);

    GridBagConstraints gbc = new GridBagConstraints();
    gbc.fill = GridBagConstraints.BOTH;

    for (int i = 0; i < names.length; i++) {
      labels[i] = new Label(names[i]);
      gbc.gridwidth = 1;
      gridbag.setConstraints(labels[i],gbc);
      add(labels[i]);

      choices[i] = new Choice();
      gbc.gridwidth = GridBagConstraints.REMAINDER;
      gridbag.setConstraints(choices[i],gbc);
      add(choices[i]);
      choices[i].addItemListener(this);

      for (int j = 0; j < values[i].length; j++) {
	choices[i].addItem(values[i][j]);
      }
    }

    closeButton = new Button("Close");
    gbc.gridwidth = GridBagConstraints.REMAINDER;
    gridbag.setConstraints(closeButton, gbc);
    add(closeButton);
    closeButton.addActionListener(this);

    pack();

    addWindowListener(this);

    // Set up defaults

    choices[compressTypeIndex].select("JPEG");
    choices[compressLevelIndex].select("None");
    choices[jpegQualityIndex].select("95");
    choices[cursorUpdatesIndex].select("Enable");
    choices[useCopyRectIndex].select("Yes");
    choices[wanIndex].select("No");
    choices[mouseButtonIndex].select("Normal");
    choices[viewOnlyIndex].select("No");
    choices[scaleCursorIndex].select("No");
    choices[shareDesktopIndex].select("Yes");

    // But let them be overridden by parameters

    for (int i = 0; i < names.length; i++) {
      String s = viewer.readParameter(names[i], false);
      if (s != null) {
	for (int j = 0; j < values[i].length; j++) {
	  if (s.equalsIgnoreCase(values[i][j])) {
	    choices[i].select(j);
	  }
	}
      }
    }

    // FIXME: Provide some sort of GUI for "Scaling Factor".

    autoScale = false;
    scalingFactor = 100;
    String s = viewer.readParameter("Scaling Factor", false);
    if (s != null) {
      if (s.equalsIgnoreCase("Auto")) {
	autoScale = true;
      } else {
	// Remove the '%' char at the end of string if present.
	if (s.charAt(s.length() - 1) == '%') {
	  s = s.substring(0, s.length() - 1);
	}
	// Convert to an integer.
	try {
	  scalingFactor = Integer.parseInt(s);
	}
	catch (NumberFormatException e) {
	  scalingFactor = 100;
	}
	// Make sure scalingFactor is in the range of [1..1000].
	if (scalingFactor < 1) {
	  scalingFactor = 1;
	} else if (scalingFactor > 1000) {
	  scalingFactor = 1000;
	}
      }
    }

    // Make the booleans and encodings array correspond to the state of the GUI

    setEncodings();
    setOtherOptions();
  }


  //
  // Disable the shareDesktop option
  //

  void disableShareDesktop() {
    labels[shareDesktopIndex].setEnabled(false);
    choices[shareDesktopIndex].setEnabled(false);
  }


  //
  // setEncodings looks at the encoding, compression level, JPEG
  // quality level, cursor shape updates and copyRect choices and sets
  // corresponding variables properly. Then it calls the VncViewer's
  // setEncodings method to send a SetEncodings message to the RFB
  // server.
  //

  void setEncodings() {
    useCopyRect = choices[useCopyRectIndex].getSelectedItem().equals("Yes");

    preferredEncoding = RfbProto.EncodingTight;
    if (choices[compressTypeIndex].getSelectedItem().equals("None (RGB)")) {
      preferredEncoding = RfbProto.EncodingRaw;
    }

    // Handle compression level setting.

    compressLevel = 0;
    if (choices[compressLevelIndex].getSelectedItem().equals("4X")) {
      compressLevel = 1;
    } else if (choices[compressLevelIndex].getSelectedItem().equals("2X")) {
      compressLevel = 2;
    } else if (choices[compressLevelIndex].getSelectedItem().equals("Grayscale")) {
      compressLevel = 3;
    }

    // Handle JPEG quality setting.

    try {
      jpegQuality =
        Integer.parseInt(choices[jpegQualityIndex].getSelectedItem());
    }
    catch (NumberFormatException e) {
      jpegQuality = -1;
    }
    if (jpegQuality < 1 || jpegQuality > 100) {
      jpegQuality = -1;
    }

    // Request cursor shape updates if necessary.

    requestCursorUpdates =
      !choices[cursorUpdatesIndex].getSelectedItem().equals("Disable");

    if (requestCursorUpdates) {
      ignoreCursorUpdates =
	choices[cursorUpdatesIndex].getSelectedItem().equals("Ignore");
    }

    wan =
      choices[wanIndex].getSelectedItem().equals("Yes");

    viewer.setEncodings();
    setPreset();
  }

  //
  // setOtherOptions looks at the "other" choices (ones which don't set the
  // encoding or the color format) and sets the boolean flags appropriately.
  //

  void setOtherOptions() {

    reverseMouseButtons2And3
      = choices[mouseButtonIndex].getSelectedItem().equals("Reversed");

    viewOnly 
      = choices[viewOnlyIndex].getSelectedItem().equals("Yes");
    if (viewer.vc != null)
      viewer.vc.enableInput(!viewOnly);

    shareDesktop
      = choices[shareDesktopIndex].getSelectedItem().equals("Yes");

    String scaleString = choices[scaleCursorIndex].getSelectedItem();
    if (scaleString.endsWith("%"))
      scaleString = scaleString.substring(0, scaleString.length() - 1);
    try {
      scaleCursor = Integer.parseInt(scaleString);
    }
    catch (NumberFormatException e) {
      scaleCursor = 0;
    }
    if (scaleCursor < 10 || scaleCursor > 500) {
      scaleCursor = 0;
    }
    if (requestCursorUpdates && !ignoreCursorUpdates && !viewOnly) {
      labels[scaleCursorIndex].setEnabled(true);
      choices[scaleCursorIndex].setEnabled(true);
    } else {
      labels[scaleCursorIndex].setEnabled(false);
      choices[scaleCursorIndex].setEnabled(false);
    }
    if (viewer.vc != null)
      viewer.vc.createSoftCursor(); // update cursor scaling
  }

  void getPreset() {

    boolean updateOptions = false;

    if (choices[presetIndex].getSelectedItem().equals("Broadband (favor performance)")) {
      preferredEncoding = RfbProto.EncodingTight;
      compressLevel = 1;
      jpegQuality = 30;
      wan = true;
      updateOptions = true;
    } else if (choices[presetIndex].getSelectedItem().equals("Broadband (favor image quality)")) {
      preferredEncoding = RfbProto.EncodingTight;
      compressLevel = 0;
      jpegQuality = 95;
      wan = true;
      updateOptions = true;
    } else if (choices[presetIndex].getSelectedItem().equals("High-Speed Network")) {
      preferredEncoding = RfbProto.EncodingTight;
      compressLevel = 0;
      jpegQuality = 95;
      wan = false;
      updateOptions = true;
    }

    if (updateOptions) {
      choices[jpegQualityIndex].select(String.valueOf(jpegQuality));
      choices[wanIndex].select(wan? "Yes":"No");
      if (compressLevel==1) {
        choices[compressLevelIndex].select("4X");
      } else if (compressLevel==2) {
        choices[compressLevelIndex].select("2X");
      } else if (compressLevel==3) {
        choices[compressLevelIndex].select("Grayscale");
      } else {
        choices[compressLevelIndex].select("None");
      }
      viewer.setEncodings();
    }
  }

  void setPreset() {

    if (preferredEncoding == RfbProto.EncodingTight
      && compressLevel == 0 && jpegQuality == 95 && wan == false) {
      choices[presetIndex].select("High-Speed Network");
    } else if (preferredEncoding == RfbProto.EncodingTight
      && compressLevel == 1 && jpegQuality == 30 && wan == true) {
      choices[presetIndex].select("Broadband (favor performance)");
    } else if (preferredEncoding == RfbProto.EncodingTight
      && compressLevel == 0 && jpegQuality == 95 && wan == true) {
      choices[presetIndex].select("Broadband (favor image quality)");
    } else {
      choices[presetIndex].select("Custom");
    }
  }

  //
  // Respond to actions on Choice controls
  //

  public void itemStateChanged(ItemEvent evt) {
    Object source = evt.getSource();

    if (source == choices[wanIndex] ||
        source == choices[compressTypeIndex] ||
        source == choices[compressLevelIndex] ||
        source == choices[jpegQualityIndex] ||
        source == choices[cursorUpdatesIndex] ||
        source == choices[useCopyRectIndex]) {

      setEncodings();

      if (source == choices[cursorUpdatesIndex]) {
        setOtherOptions();      // update scaleCursor state
      }

    } else if (source == choices[mouseButtonIndex] ||
	       source == choices[shareDesktopIndex] ||
	       source == choices[viewOnlyIndex] ||
	       source == choices[scaleCursorIndex]) {

      setOtherOptions();

    } else if (source == choices[presetIndex]) {

      getPreset();
      viewer.setEncodings();
    }
  }

  //
  // Respond to button press
  //

  public void actionPerformed(ActionEvent evt) {
    if (evt.getSource() == closeButton)
      setVisible(false);
  }

  //
  // Respond to window events
  //

  public void windowClosing(WindowEvent evt) {
    setVisible(false);
  }

  public void windowActivated(WindowEvent evt) {}
  public void windowDeactivated(WindowEvent evt) {}
  public void windowOpened(WindowEvent evt) {}
  public void windowClosed(WindowEvent evt) {}
  public void windowIconified(WindowEvent evt) {}
  public void windowDeiconified(WindowEvent evt) {}
}
