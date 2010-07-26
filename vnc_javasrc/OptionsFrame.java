//
//  Copyright (C) 2010 D. R. Commander.  All Rights Reserved.
//  Copyright (C) 2009 Paul Donohue.  All Rights Reserved.
//  Copyright (C) 2006-2008 Sun Microsystems, Inc.  All Rights Reserved.
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
    "Image encoding protocol",
    "Allow JPEG compression",
    "JPEG chrominance subsampling",
    "JPEG image quality",
    "Zlib compression level",
    "Cursor shape updates",
    "Use CopyRect",
    "Mouse buttons 2 and 3",
    "View only",
    "Scaling factor",
    "Scale remote cursor",
    "Share desktop"
  };

  static String[][] values = {
    { "Tight + Perceptually Lossless JPEG (LAN)",
      "Tight + Medium Quality JPEG", "Tight + Low Quality JPEG (WAN)",
      "Lossless Tight (Gigabit)", "Lossless Tight + Zlib (WAN)", "Custom" },
    { "Yes", "No" },
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
    { "None", "1" },
    { "Enable", "Ignore", "Disable" },
    { "Yes", "No" },
    { "Normal", "Reversed" },
    { "Yes", "No" },
    { "Auto", "Fixed Ratio", "50%", "75%", "95%", "100%", "105%", "125%",
      "150%", "175%", "200%", "250%", "300%", "350%", "400%"}, 
    { "No", "50%", "75%", "125%", "150%" },
    { "Yes", "No" }
  };

  final int
    encodingIndex        = 0,
    enableJpegIndex      = 1,
    subsampLevelIndex    = 2,
    jpegQualityIndex     = 3,
    compressLevelIndex   = 4,
    cursorUpdatesIndex   = 5,
    useCopyRectIndex     = 6,
    mouseButtonIndex     = 7,
    viewOnlyIndex        = 8,
    scalingFactorIndex   = 9,
    scaleCursorIndex     = 10,
    shareDesktopIndex    = 11;

  Label[] labels = new Label[names.length];
  Choice[] choices = new Choice[names.length];
  Button closeButton;
  VncViewer viewer;


  //
  // The actual data which other classes look at:
  //

  int preferredEncoding;
  int compressLevel;
  int subsampLevel;
  int jpegQuality;
  boolean enableJpeg;
  boolean useCopyRect;
  boolean requestCursorUpdates;
  boolean ignoreCursorUpdates;

  boolean reverseMouseButtons2And3;
  boolean shareDesktop;
  boolean viewOnly;
  int scaleCursor;

  boolean autoScale;
  boolean fixedRatioScale;
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

    choices[encodingIndex].select("Tight + Perceptually Lossless JPEG (LAN)");
    choices[enableJpegIndex].select("Yes");
    choices[subsampLevelIndex].select("None");
    choices[jpegQualityIndex].select("95");
    choices[compressLevelIndex].select("None");
    choices[cursorUpdatesIndex].select("Enable");
    choices[useCopyRectIndex].select("Yes");
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

    // Get scaling factor from parameters and set it
    // to gui and class member scalingFactor
    
    String s = viewer.readParameter("Scaling Factor", false);
    if (s == null) s = "100%";
    setScalingFactor(s);
    if (autoScale) {
      choices[scalingFactorIndex].select("Auto");
    } else if (fixedRatioScale) {
      choices[scalingFactorIndex].select("Fixed Ratio");
    } else {
      choices[scalingFactorIndex].select(s);
    }

    // Make the booleans and encodings array correspond to the state of the GUI

    setEncodings();
    setOtherOptions();
  }
  
  //
  // Set scaling factor class member value
  //
  
  void setScalingFactor(int sf) {
      setScalingFactor(new Integer(sf).toString());
  }
  
  void setScalingFactor(String s) {
    autoScale = false;
    fixedRatioScale = false;
    scalingFactor = 100;
    if (s != null) {
      if (s.equalsIgnoreCase("Auto")) {
	autoScale = true;
      } else if (s.equalsIgnoreCase("Fixed Ratio")) {
	fixedRatioScale = true;
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
    enableJpeg = choices[enableJpegIndex].getSelectedItem().equals("Yes");

    preferredEncoding = RfbProto.EncodingTight;
    boolean enableCompressLevel = false;
    boolean enableSubsampLevel = false;
    boolean enableQualityLevel = false;

    if (enableJpeg) {
      enableSubsampLevel = true;
      enableQualityLevel = true;
    } else {
      enableCompressLevel = true;
    }

    // Handle compression level setting.

    compressLevel = 0;
    if (choices[compressLevelIndex].getSelectedItem().equals("1")) {
      compressLevel = 1;
    }
    labels[compressLevelIndex].setEnabled(enableCompressLevel);
    choices[compressLevelIndex].setEnabled(enableCompressLevel);

    // Handle subsampling level setting.

    subsampLevel = 0;
    if (choices[subsampLevelIndex].getSelectedItem().equals("4X")) {
      subsampLevel = 1;
    } else if (choices[subsampLevelIndex].getSelectedItem().equals("2X")) {
      subsampLevel = 2;
    } else if (choices[subsampLevelIndex].getSelectedItem().equals("Grayscale")) {
      subsampLevel = 3;
    }
    labels[subsampLevelIndex].setEnabled(enableSubsampLevel);
    choices[subsampLevelIndex].setEnabled(enableSubsampLevel);

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
    labels[jpegQualityIndex].setEnabled(enableQualityLevel);
    choices[jpegQualityIndex].setEnabled(enableQualityLevel);

    // Request cursor shape updates if necessary.

    requestCursorUpdates =
      !choices[cursorUpdatesIndex].getSelectedItem().equals("Disable");

    if (requestCursorUpdates) {
      ignoreCursorUpdates =
	choices[cursorUpdatesIndex].getSelectedItem().equals("Ignore");
    }

    viewer.setEncodings();
    setPreset();
  }

  //
  // setOtherOptions looks at the "other" choices (ones that do not
  // cause sending any protocol messages) and sets the boolean flags
  // appropriately.
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

    if (choices[encodingIndex].getSelectedItem().equals("Tight + Perceptually Lossless JPEG (LAN)")) {
      preferredEncoding = RfbProto.EncodingTight;
      enableJpeg = true;
      subsampLevel = 0;
      jpegQuality = 95;
      useCopyRect = true;
      updateOptions = true;
    } else if (choices[encodingIndex].getSelectedItem().equals("Tight + Medium Quality JPEG")) {
      preferredEncoding = RfbProto.EncodingTight;
      enableJpeg = true;
      subsampLevel = 2;
      jpegQuality = 80;
      useCopyRect = true;
      updateOptions = true;
    } else if (choices[encodingIndex].getSelectedItem().equals("Tight + Low Quality JPEG (WAN)")) {
      preferredEncoding = RfbProto.EncodingTight;
      enableJpeg = true;
      subsampLevel = 1;
      jpegQuality = 30;
      useCopyRect = true;
      updateOptions = true;
    } else if (choices[encodingIndex].getSelectedItem().equals("Lossless Tight (Gigabit)")) {
      preferredEncoding = RfbProto.EncodingTight;
      enableJpeg = false;
      compressLevel = 0;
      useCopyRect = true;
      updateOptions = true;
    } else if (choices[encodingIndex].getSelectedItem().equals("Lossless Tight + Zlib (WAN)")) {
      preferredEncoding = RfbProto.EncodingTight;
      enableJpeg = false;
      compressLevel = 1;
      useCopyRect = true;
      updateOptions = true;
    }

    if (updateOptions) {
      if (enableJpeg) {
        choices[enableJpegIndex].select("Yes");
        labels[subsampLevelIndex].setEnabled(true);
        choices[subsampLevelIndex].setEnabled(true);
        labels[jpegQualityIndex].setEnabled(true);
        choices[jpegQualityIndex].setEnabled(true);
        labels[compressLevelIndex].setEnabled(false);
        choices[compressLevelIndex].setEnabled(false);
      } else {
        choices[enableJpegIndex].select("No");
        labels[subsampLevelIndex].setEnabled(false);
        choices[subsampLevelIndex].setEnabled(false);
        labels[jpegQualityIndex].setEnabled(false);
        choices[jpegQualityIndex].setEnabled(false);
        labels[compressLevelIndex].setEnabled(true);
        choices[compressLevelIndex].setEnabled(true);
      }
      if (useCopyRect) {
        choices[useCopyRectIndex].select("Yes");
      } else {
        choices[useCopyRectIndex].select("No");
      }
      choices[jpegQualityIndex].select(String.valueOf(jpegQuality));
      if (subsampLevel==1) {
        choices[subsampLevelIndex].select("4X");
      } else if (subsampLevel==2) {
        choices[subsampLevelIndex].select("2X");
      } else if (subsampLevel==3) {
        choices[subsampLevelIndex].select("Grayscale");
      } else {
        choices[subsampLevelIndex].select("None");
      }
      if (compressLevel==1) {
        choices[compressLevelIndex].select("1");
      } else {
        choices[compressLevelIndex].select("None");
      }
      viewer.setEncodings();
    }
  }

  void setPreset() {

    if (enableJpeg && useCopyRect
      && preferredEncoding == RfbProto.EncodingTight
      && subsampLevel == 0 && jpegQuality == 95) {
      choices[encodingIndex].select("Tight + Perceptually Lossless JPEG (LAN)");
    } else if (enableJpeg && useCopyRect
      && preferredEncoding == RfbProto.EncodingTight
      && subsampLevel == 2 && jpegQuality == 80) {
      choices[encodingIndex].select("Tight + Medium Quality JPEG");
    } else if (enableJpeg && useCopyRect
      && preferredEncoding == RfbProto.EncodingTight
      && subsampLevel == 1 && jpegQuality == 30) {
      choices[encodingIndex].select("Tight + Low Quality JPEG (WAN)");
    } else if (!enableJpeg && useCopyRect
      && preferredEncoding == RfbProto.EncodingTight
      && compressLevel == 0) {
      choices[encodingIndex].select("Lossless Tight (Gigabit)");
    } else if (!enableJpeg && useCopyRect
      && preferredEncoding == RfbProto.EncodingTight
      && compressLevel == 1) {
      choices[encodingIndex].select("Lossless Tight + Zlib (WAN)");
    } else {
      choices[encodingIndex].select("Custom");
    }
  }

  //
  // Respond to actions on Choice controls
  //

  public void itemStateChanged(ItemEvent evt) {
    Object source = evt.getSource();

    if (source == choices[enableJpegIndex] ||
        source == choices[subsampLevelIndex] ||
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

    } else if (source == choices[encodingIndex]) {

      getPreset();
      viewer.setEncodings();

    } else if (source == choices[scalingFactorIndex]){
        // Tell VNC canvas that scaling factor has changed
        setScalingFactor(choices[scalingFactorIndex].getSelectedItem());
        if (viewer.vc != null)
          viewer.vc.setScaledSize();
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
