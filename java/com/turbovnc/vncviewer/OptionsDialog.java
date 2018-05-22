/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright (C) 2011-2013 Brian P. Hinz
 * Copyright (C) 2012-2018 D. R. Commander.  All Rights Reserved.
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
import java.io.File;
import java.util.*;
import javax.swing.*;
import javax.swing.event.ChangeListener;
import javax.swing.event.ChangeEvent;
import javax.swing.border.*;

import com.turbovnc.rdr.*;
import com.turbovnc.rfb.*;

class OptionsDialog extends Dialog implements ActionListener, ChangeListener,
  ItemListener {

  // Constants
  // Static variables
  static LogWriter vlog = new LogWriter("OptionsDialog");

  OptionsDialogCallback callback;
  JTabbedPane tabPane;
  JPanel buttonPane, encodingPanel, connPanel, globalPanel, secPanel;
  JCheckBox allowJpeg, interframe;
  JComboBox menuKey, scalingFactor, encMethodComboBox, span, desktopSize,
    grabKeyboard;
  JSlider jpegQualityLevel, subsamplingLevel, compressionLevel;
  JCheckBox viewOnly, acceptClipboard, sendClipboard, acceptBell,
    reverseScroll;
  JCheckBox fullScreen, shared, cursorShape, showToolbar;
  JCheckBox secVeNCrypt, encNone, encTLS, encX509;
  JCheckBox secNone, secVnc, secUnixLogin, secPlain, secIdent,
    sendLocalUsername, tunnel;
  JTextField sshUser, gateway;
  JButton okButton, cancelButton;
  JButton x509caButton, x509crlButton;
  JTextField x509ca, x509crl;
  JButton defClearButton;
  JLabel encMethodLabel;
  JLabel jpegQualityLabel, jpegQualityLabelLo, jpegQualityLabelHi;
  JLabel subsamplingLabel, subsamplingLabelLo, subsamplingLabelHi;
  JLabel compressionLabel, compressionLabelLo, compressionLabelHi;
  String jpegQualityLabelString, subsamplingLabelString;
  String compressionLabelString;
  Hashtable<Integer, String> subsamplingLabelTable;
  String oldScalingFactor, oldDesktopSize;

  OptionsDialog(OptionsDialogCallback callback_) {
    super(true);
    callback = callback_;

    tabPane = new JTabbedPane();

    // Encoding tab
    encodingPanel = new JPanel(new GridBagLayout());

    JPanel dummyPanel = new JPanel(new GridBagLayout());

    encMethodLabel = new JLabel("Encoding method:");
    Object[] encMethod = {
      "Tight + Perceptually Lossless JPEG (LAN)",
      "Tight + Medium-Quality JPEG",
      "Tight + Low-Quality JPEG (WAN)",
      "Lossless Tight (Gigabit)",
      "Lossless Tight + Zlib (WAN)"
    };
    encMethodComboBox = new JComboBox(encMethod);
    encMethodComboBox.addActionListener(this);

    Dialog.addGBComponent(encMethodLabel, dummyPanel,
                          0, 0, 3, 1, 2, 2, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 2, 2, 2));
    Dialog.addGBComponent(encMethodComboBox, dummyPanel,
                          0, 1, 3, 1, 2, 2, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 20, 2, 2));

    allowJpeg = new JCheckBox("Allow JPEG compression");
    allowJpeg.addItemListener(this);

    Dialog.addGBComponent(allowJpeg, dummyPanel,
                          0, 2, 3, 1, 0, 2, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 0, 2, 2));

    subsamplingLabelString =
      new String("JPEG chrominance subsampling: ");
    subsamplingLabel = new JLabel();
    subsamplingLevel =
      new JSlider(JSlider.HORIZONTAL, 0, Options.NUMSUBSAMPOPT - 1, 0);
    subsamplingLevel.addChangeListener(this);
    subsamplingLevel.setMajorTickSpacing(1);
    subsamplingLevel.setSnapToTicks(true);
    subsamplingLevel.setPaintTicks(true);
    subsamplingLevel.setInverted(true);
    subsamplingLabelTable = new Hashtable<Integer, String>();
    subsamplingLabelTable.put(0, new String("None"));
    subsamplingLabelTable.put(1, new String("2X"));
    subsamplingLabelTable.put(2, new String("4X"));
    subsamplingLabelTable.put(3, new String("Grayscale"));
    subsamplingLevel.setPaintLabels(false);
    subsamplingLabelLo = new JLabel("fast");
    subsamplingLabelHi = new JLabel("best");
    subsamplingLabel.setText(subsamplingLabelString +
      subsamplingLabelTable.get(subsamplingLevel.getValue()));

    Dialog.addGBComponent(subsamplingLabel, dummyPanel,
                          0, 3, 3, 1, 2, 2, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 2, 2, 2));
    Dialog.addGBComponent(subsamplingLabelLo, dummyPanel,
                          0, 4, 1, 1, 2, 2, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 12, 2, 2));
    Dialog.addGBComponent(subsamplingLevel, dummyPanel,
                          1, 4, 1, 1, 2, 2, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 2, 2, 2));
    Dialog.addGBComponent(subsamplingLabelHi, dummyPanel,
                          2, 4, 1, 1, 2, 2, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 2, 2, 2));

    jpegQualityLabelString =
      new String("JPEG image quality: ");
    jpegQualityLabel = new JLabel();
    jpegQualityLevel =
      new JSlider(JSlider.HORIZONTAL, 1, 100, Options.DEFQUAL);
    jpegQualityLevel.addChangeListener(this);
    jpegQualityLevel.setMajorTickSpacing(10);
    jpegQualityLevel.setMinorTickSpacing(5);
    jpegQualityLevel.setSnapToTicks(false);
    jpegQualityLevel.setPaintTicks(true);
    jpegQualityLevel.setPaintLabels(false);
    jpegQualityLabelLo = new JLabel("poor");
    jpegQualityLabelHi = new JLabel("best");
    jpegQualityLabel.setText(jpegQualityLabelString +
      jpegQualityLevel.getValue());

    Dialog.addGBComponent(jpegQualityLabel, dummyPanel,
                          0, 5, 3, 1, 2, 2, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 2, 2, 2));
    Dialog.addGBComponent(jpegQualityLabelLo, dummyPanel,
                          0, 6, 1, 1, 2, 2, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 12, 2, 2));
    Dialog.addGBComponent(jpegQualityLevel, dummyPanel,
                          1, 6, 1, 1, 2, 2, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 2, 2, 2));
    Dialog.addGBComponent(jpegQualityLabelHi, dummyPanel,
                          2, 6, 1, 1, 2, 2, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 2, 2, 2));

    compressionLabelString =
      new String("Compression level (see docs): ");
    compressionLabel = new JLabel();
    compressionLevel =
      new JSlider(JSlider.HORIZONTAL, 0, 2, 1);
    compressionLevel.addChangeListener(this);
    compressionLevel.setMajorTickSpacing(1);
    compressionLevel.setSnapToTicks(true);
    compressionLevel.setPaintTicks(true);
    compressionLevel.setPaintLabels(false);
    compressionLabelLo = new JLabel("fast");
    compressionLabelHi = new JLabel("best");
    compressionLabel.setText(compressionLabelString +
                             compressionLevel.getValue());

    Dialog.addGBComponent(compressionLabel, dummyPanel,
                          0, 7, 3, 1, 2, 2, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 2, 2, 2));
    Dialog.addGBComponent(compressionLabelLo, dummyPanel,
                          0, 8, 1, 1, 2, 2, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 12, 2, 2));
    Dialog.addGBComponent(compressionLevel, dummyPanel,
                          1, 8, 1, 1, 2, 2, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 2, 2, 2));
    Dialog.addGBComponent(compressionLabelHi, dummyPanel,
                          2, 8, 1, 1, 2, 2, 1, 1,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 2, 2, 2));

    interframe = new JCheckBox("Interframe comparison");
    interframe.addItemListener(this);

    Dialog.addGBComponent(interframe, dummyPanel,
                          0, 9, 3, 1, 0, 2, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 0, 2, 2));

    Dialog.addGBComponent(dummyPanel, encodingPanel,
                          0, 0, 1, GridBagConstraints.REMAINDER, 0, 0, 1, 1,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.PAGE_START,
                          new Insets(4, 4, 4, 4));

    // Connection tab
    connPanel = new JPanel(new GridBagLayout());

    dummyPanel = new JPanel(new GridBagLayout());

    JPanel displayPanel = new JPanel(new GridBagLayout());
    displayPanel.setBorder(BorderFactory.createTitledBorder("Display"));

    JLabel scalingFactorLabel = new JLabel("Scaling factor:");
    Object[] scalingFactors = {
      "Auto", "Fixed Aspect Ratio", "50%", "75%", "95%", "100%", "105%",
      "125%", "150%", "175%", "200%", "250%", "300%", "350%", "400%"
    };
    scalingFactor = new JComboBox(scalingFactors);
    scalingFactor.setEditable(true);
    scalingFactor.addItemListener(this);

    Dialog.addGBComponent(scalingFactorLabel, displayPanel,
                          0, 0, 1, 1, 2, 2, 1, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(8, 8, 0, 5));
    Dialog.addGBComponent(scalingFactor, displayPanel,
                          1, 0, 1, 1, 2, 2, 25, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(4, 5, 0, 5));

    Object[] desktopSizeOptions = {
      "Auto", "Server", "480x320", "640x360", "640x480", "800x480", "800x600",
      "854x480", "960x540", "960x600", "960x640",
      "1024x640", "1024x640+0+0,1024x640+1024+0",
      "1024x768", "1136x640", "1152x864",
      "1280x720", "1280x720+0+0,1280x720+1280+0",
      "1280x800", "1280x800+0+0,1280x800+1280+0",
      "1280x960", "1280x1024",
      "1344x840", "1344x840+0+0,1344x840+1344+0",
      "1344x1008", "1360x768",
      "1366x768", "1366x768+0+0,1366x768+1366+0",
      "1400x1050",
      "1440x900", "1440x900+0+0,1440x900+1440+0",
      "1600x900", "1600x900+0+0,1600x900+1600+0",
      "1600x1000", "1600x1000+0+0,1600x1000+1600+0",
      "1600x1200", "1680x1050",
      "1920x1080", "1920x1080+0+0,1920x1080+1920+0",
      "1920x1200", "1920x1200+0+0,1920x1200+1920+0",
      "2048x1152", "2048x1536",
      "2560x1440", "2560x1440+0+0,2560x1440+2560+0",
      "2560x1600", "2560x1600+0+0,2560x1600+2560+0",
      "2880x1800", "2880x1800+0+0,2880x1800+2880+0",
      "3200x1800", "3200x1800+0+0,3200x1800+3200+0"
    };
    JLabel desktopSizeLabel = new JLabel("Remote desktop size:");
    desktopSize = new JComboBox(desktopSizeOptions);
    desktopSize.setEditable(true);
    desktopSize.addItemListener(this);
    desktopSize.setMaximumSize(desktopSize.getPreferredSize());

    Dialog.addGBComponent(desktopSizeLabel, displayPanel,
                          0, 1, 1, 1, 2, 2, 1, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(8, 8, 0, 5));
    Dialog.addGBComponent(desktopSize, displayPanel,
                          1, 1, 1, 1, 2, 2, 25, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(4, 5, 0, 5));

    fullScreen = new JCheckBox("Full-screen mode");
    fullScreen.addItemListener(this);

    Dialog.addGBComponent(fullScreen, displayPanel,
                          0, 2, 2, 1, 2, 2, 1, 0,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(4, 5, 0, 5));

    Object[] spanOptions = {
      "Primary monitor only", "All monitors", "Automatic"
    };
    JLabel spanLabel;
    if (VncViewer.isX11() && Viewport.isHelperAvailable())
      spanLabel = new JLabel("Full-screen span mode:");
    else
      spanLabel = new JLabel("Span mode:");
    span = new JComboBox(spanOptions);
    span.addItemListener(this);
    if (VncViewer.isX11() && !Viewport.isHelperAvailable()) {
      spanLabel.setEnabled(false);
      span.setEnabled(false);
    }

    Dialog.addGBComponent(spanLabel, displayPanel,
                          0, 3, 1, 1, 2, 2, 1, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(8, 8, 0, 5));
    Dialog.addGBComponent(span, displayPanel,
                          1, 3, 1, 1, 2, 2, 25, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(4, 5, 0, 5));

    acceptBell = new JCheckBox("Beep when requested by the server");
    acceptBell.addItemListener(this);

    Dialog.addGBComponent(acceptBell, displayPanel,
                          0, 4, 2, 1, 2, 2, 1, 1,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(4, 5, 0, 5));

    cursorShape =
      new JCheckBox("Render cursor locally (enable remote cursor shape updates)");
    cursorShape.addItemListener(this);

    Dialog.addGBComponent(cursorShape, displayPanel,
                          0, 5, 2, 1, 2, 2, 1, 0,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(4, 5, 0, 5));

    JPanel inputPanel = new JPanel(new GridBagLayout());
    inputPanel.setBorder(BorderFactory.createTitledBorder("Input"));

    viewOnly = new JCheckBox("View only (ignore mouse & keyboard)");
    viewOnly.addItemListener(this);
    viewOnly.setEnabled(VncViewer.viewOnlyControl.getValue());

    Dialog.addGBComponent(viewOnly, inputPanel,
                          0, 0, 2, 1, 2, 2, 1, 0,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.LINE_START,
                          new Insets(4, 5, 0, 5));

    reverseScroll = new JCheckBox("Reverse scroll wheel direction");
    reverseScroll.addItemListener(this);
    Dialog.addGBComponent(reverseScroll, inputPanel,
                          0, 1, 2, 1, 2, 2, 1, 0,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.LINE_START,
                          new Insets(4, 5, 0, 5));

    boolean enableGrab = VncViewer.osGrab() && Viewport.isHelperAvailable();

    if (enableGrab) {
      JLabel grabLabel;
      if (VncViewer.grabPointer.getValue())
        grabLabel = new JLabel("Keyboard/pointer grab mode:");
      else
        grabLabel = new JLabel("Keyboard grab mode:");
      Object[] grabModes = { "Full-screen only", "Always", "Manual" };
      grabKeyboard = new JComboBox(grabModes);
      grabKeyboard.addItemListener(this);

      Dialog.addGBComponent(grabLabel, inputPanel,
                            0, 2, 1, 1, 2, 2, 1, 0,
                            GridBagConstraints.NONE,
                            GridBagConstraints.FIRST_LINE_START,
                            new Insets(8, 8, 0, 5));
      Dialog.addGBComponent(grabKeyboard, inputPanel,
                            1, 2, 1, 1, 2, 2, 25, 0,
                            GridBagConstraints.NONE,
                            GridBagConstraints.FIRST_LINE_START,
                            new Insets(4, 5, 0, 5));
    }

    JPanel restrictionsPanel = new JPanel(new GridBagLayout());
    restrictionsPanel.setBorder(
      BorderFactory.createTitledBorder("Restrictions"));

    shared = new JCheckBox("Request shared session");
    shared.addItemListener(this);

    Dialog.addGBComponent(shared, restrictionsPanel,
                          0, 0, 2, 1, 2, 2, 1, 0,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(4, 5, 0, 5));

    acceptClipboard = new JCheckBox("Accept clipboard from server");
    acceptClipboard.addItemListener(this);

    Dialog.addGBComponent(acceptClipboard, restrictionsPanel,
                          0, 1, 2, 1, 2, 2, 1, 0,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.LINE_START,
                          new Insets(4, 5, 0, 5));

    sendClipboard = new JCheckBox("Send clipboard to server");
    sendClipboard.addItemListener(this);

    Dialog.addGBComponent(sendClipboard, restrictionsPanel,
                          0, 2, 2, 1, 2, 2, 1, 0,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.LINE_START,
                          new Insets(4, 5, 0, 5));

    Dialog.addGBComponent(displayPanel, dummyPanel,
                          0, 0, 1, 1, 2, 2, 1, 0,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(4, 5, 0, 30));
    Dialog.addGBComponent(inputPanel, dummyPanel,
                          0, 1, 1, 1, 2, 2, 1, 0,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.LINE_START,
                          new Insets(4, 5, 0, 30));
    Dialog.addGBComponent(restrictionsPanel, dummyPanel,
                          0, 2, 1, 1, 2, 2, 1, 0,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.LINE_START,
                          new Insets(4, 5, 0, 30));

    Dialog.addGBComponent(dummyPanel, connPanel,
                          0, 0, 1, GridBagConstraints.REMAINDER, 0, 0, 1, 1,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.PAGE_START,
                          new Insets(4, 4, 4, 4));

    // Global tab
    globalPanel = new JPanel(new GridBagLayout());

    showToolbar = new JCheckBox("Show toolbar by default");
    showToolbar.addItemListener(this);

    JLabel menuKeyLabel = new JLabel("Menu key:");
    String[] menuKeys = new String[MenuKey.getMenuKeySymbolCount()];
    for (int i = 0; i < MenuKey.getMenuKeySymbolCount(); i++)
      menuKeys[i] = MenuKey.getMenuKeySymbols()[i].name;
    menuKey  = new JComboBox(menuKeys);
    menuKey.addItemListener(this);

    Dialog.addGBComponent(showToolbar, globalPanel,
                          0, 1, 2, 1, 2, 2, 1, 0,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(4, 5, 0, 5));
    Dialog.addGBComponent(menuKeyLabel, globalPanel,
                          0, 2, 1, 1, 2, 2, 1, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(8, 8, 0, 5));
    Dialog.addGBComponent(menuKey, globalPanel,
                          1, 2, 1, 1, 2, 2, 25, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(4, 5, 0, 5));

    JPanel defaultsPanel = new JPanel(new GridBagLayout());
    defaultsPanel.setBorder(BorderFactory.createTitledBorder("Defaults"));
    defClearButton = new JButton("Clear");
    defClearButton.addActionListener(this);
    Dialog.addGBComponent(defClearButton, defaultsPanel,
                          0, 0, 1, 1, 2, 2, 1, 0,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(2, 2, 2, 2));

    Dialog.addGBComponent(defaultsPanel, globalPanel,
                          0, 3, 2, GridBagConstraints.REMAINDER, 2, 2, 1, 1,
                          GridBagConstraints.NONE,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(25, 5, 4, 5));

    // security tab
    secPanel = new JPanel(new GridBagLayout());

    JPanel encryptionPanel = new JPanel(new GridBagLayout());
    encryptionPanel.setBorder(
      BorderFactory.createTitledBorder("Session encryption"));
    encNone = addCheckbox("None", null, encryptionPanel);
    encTLS = addCheckbox("Anonymous TLS", null, encryptionPanel);
    encX509 = addJCheckBox("TLS with X.509 certificates", null,
                           encryptionPanel,
                           new GridBagConstraints(0, 2, 1, 1, 1, 1,
                             GridBagConstraints.LINE_START,
                             GridBagConstraints.REMAINDER,
                             new Insets(0, 0, 0, 60), 0, 0));

    JPanel x509Panel = new JPanel(new GridBagLayout());
    x509Panel.setBorder(
      BorderFactory.createTitledBorder("X.509 certificate validation"));
    x509ca = new JTextField("", 1);
    JLabel x509caLabel = new JLabel("CA cert:");
    x509caButton = new JButton("Load");
    x509caButton.addActionListener(this);
    x509crl = new JTextField("", 1);
    JLabel x509crlLabel = new JLabel("CRL:");
    x509crlButton = new JButton("Load");
    x509crlButton.addActionListener(this);
    Dialog.addGBComponent(x509caLabel, x509Panel,
                          0, 0, 1, 1, 2, 2, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(8, 8, 0, 2));
    Dialog.addGBComponent(x509ca, x509Panel,
                          1, 0, 1, 1, 2, 2, 0.7, 0,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(4, 2, 0, 2));
    Dialog.addGBComponent(x509caButton, x509Panel,
                          2, 0, 1, 1, 2, 2, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(4, 2, 0, 2));
    Dialog.addGBComponent(x509crlLabel, x509Panel,
                          0, 1, 1, 1, 2, 2, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(8, 8, 0, 2));
    Dialog.addGBComponent(x509crl, x509Panel,
                          1, 1, 1, 1, 2, 2, 0.7, 0,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(4, 2, 0, 2));
    Dialog.addGBComponent(x509crlButton, x509Panel,
                          2, 1, 1, 1, 2, 2, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(4, 2, 0, 2));

    JPanel authPanel = new JPanel(new GridBagLayout());
    authPanel.setBorder(
      BorderFactory.createTitledBorder("Authentication schemes"));
    secNone = addCheckbox("None", null, authPanel);
    secVnc = addCheckbox("Standard VNC", null, authPanel);
    secUnixLogin = addJCheckBox("Unix Login (TightVNC/TurboVNC)", null,
                                authPanel,
                                new GridBagConstraints(0, 2, 1, 1, 1, 1,
                                  GridBagConstraints.LINE_START,
                                  GridBagConstraints.NONE,
                                  new Insets(0, 0, 0, 5), 0, 0));
    secPlain = addJCheckBox("Plain (VeNCrypt)", null, authPanel,
                            new GridBagConstraints(0, 3, 1, 1, 1, 1,
                              GridBagConstraints.LINE_START,
                              GridBagConstraints.NONE,
                              new Insets(0, 0, 0, 5), 0, 0));
    secIdent = addJCheckBox("Ident (VeNCrypt)", null, authPanel,
                            new GridBagConstraints(0, 4, 1, 1, 1, 1,
                              GridBagConstraints.LINE_START,
                              GridBagConstraints.NONE,
                              new Insets(0, 0, 0, 5), 0, 0));
    sendLocalUsername = new JCheckBox("Send local username");
    sendLocalUsername.addItemListener(this);
    Dialog.addGBComponent(sendLocalUsername, authPanel,
                          1, 3, 1, 2, 0, 0, 2, 1,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.LINE_START,
                          new Insets(0, 20, 0, 0));

    secVeNCrypt =
      new JCheckBox("Extended encryption and authentication (VeNCrypt)");
    secVeNCrypt.addItemListener(this);

    JPanel gatewayPanel = new JPanel(new GridBagLayout());
    gatewayPanel.setBorder(
      BorderFactory.createTitledBorder("Gateway (SSH server or UltraVNC repeater)"));
    gateway = new JTextField("", 1);
    JLabel gatewayLabel = new JLabel("Host:");
    sshUser = new JTextField("", 1);
    JLabel sshUserLabel = new JLabel("SSH user:");
    tunnel = new JCheckBox("Use VNC server as gateway");
    tunnel.addItemListener(this);

    Dialog.addGBComponent(sshUserLabel, gatewayPanel,
                          0, 0, 1, 1, 2, 2, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(8, 8, 0, 2));
    Dialog.addGBComponent(sshUser, gatewayPanel,
                          1, 0, 1, 1, 2, 2, 0.7, 0,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(4, 2, 0, 2));
    Dialog.addGBComponent(gatewayLabel, gatewayPanel,
                          2, 0, 1, 1, 2, 2, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(8, 2, 0, 2));
    Dialog.addGBComponent(gateway, gatewayPanel,
                          3, 0, 1, 1, 2, 2, 1, 0,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(4, 2, 0, 5));
    Dialog.addGBComponent(tunnel, gatewayPanel,
                          0, 1, 4, 1, 2, 2, 1, 1,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(4, 5, 0, 5));

    Dialog.addGBComponent(secVeNCrypt, secPanel,
                          0, 0, 1, 1, 2, 2, 1, 0,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(4, 5, 0, 30));
    Dialog.addGBComponent(encryptionPanel, secPanel,
                          0, 1, 1, 1, 2, 2, 1, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(0, 10, 2, 5));
    Dialog.addGBComponent(x509Panel, secPanel,
                          0, 2, 1, 1, 2, 2, 1, 1,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(2, 10, 2, 5));
    Dialog.addGBComponent(authPanel, secPanel,
                          0, 3, 1, 1, 2, 2, 1, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(2, 10, 2, 5));
    Dialog.addGBComponent(gatewayPanel, secPanel,
                          0, 4, 1, 1, 2, 2, 1, 1,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(2, 10, 2, 5));

    tabPane.add(encodingPanel);
    tabPane.add(connPanel);
    tabPane.add(globalPanel);
    tabPane.add(secPanel);
    tabPane.addTab("Encoding", encodingPanel);
    tabPane.addTab("Connection", connPanel);
    tabPane.addTab("Global", globalPanel);
    tabPane.addTab("Security", secPanel);
    tabPane.setBorder(BorderFactory.createEmptyBorder(0, 0, 0, 0));

    okButton = new JButton("OK");
    okButton.setPreferredSize(new Dimension(90, 30));
    okButton.addActionListener(this);
    cancelButton = new JButton("Cancel");
    cancelButton.setPreferredSize(new Dimension(90, 30));
    cancelButton.addActionListener(this);

    buttonPane = new JPanel();
    buttonPane.setLayout(new BoxLayout(buttonPane, BoxLayout.LINE_AXIS));
    buttonPane.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
    buttonPane.add(Box.createHorizontalGlue());
    buttonPane.add(okButton);
    buttonPane.add(Box.createRigidArea(new Dimension(5, 0)));
    buttonPane.add(cancelButton);
    buttonPane.add(Box.createRigidArea(new Dimension(5, 0)));

    encMethodComboBox.setSelectedItem("Tight + Perceptually Lossless JPEG (LAN)");
  }

  protected void populateDialog(JDialog dlg) {
    dlg.setResizable(false);
    dlg.setTitle("TurboVNC Viewer Options");
    dlg.getContentPane().setLayout(
      new BoxLayout(dlg.getContentPane(), BoxLayout.PAGE_AXIS));
    dlg.getContentPane().add(tabPane);
    dlg.getContentPane().add(buttonPane);
    dlg.pack();

    dlg.addWindowListener(new WindowAdapter() {
      public void windowActivated(WindowEvent e) {
        if (callback != null) callback.setTightOptions();
      }
    });
  }

  public void initDialog() {
    if (callback != null) callback.setOptions();
    oldScalingFactor = scalingFactor.getSelectedItem().toString();
    oldDesktopSize = desktopSize.getSelectedItem().toString();
  }

  private void updatePreferences() {
    // CONNECTION
    //   (no GUI equivalent)
    UserPreferences.set("global", "AlwaysShowConnectionDialog",
                        VncViewer.alwaysShowConnectionDialog.getValue());
    UserPreferences.set("global", "RecvClipboard",
                        acceptClipboard.isSelected());
    UserPreferences.set("global", "SendClipboard", sendClipboard.isSelected());
    UserPreferences.set("global", "Shared", shared.isSelected());

    // INPUT
    if (VncViewer.osGrab() && Viewport.isHelperAvailable()) {
      String grabStr = (String)grabKeyboard.getSelectedItem();
      if (grabStr.equalsIgnoreCase("Full-screen only"))
        grabStr = "FS";
      UserPreferences.set("global", "GrabKeyboard", grabStr);
    }
    String menuKeyStr =
      MenuKey.getMenuKeySymbols()[menuKey.getSelectedIndex()].name;
    UserPreferences.set("global", "MenuKey", menuKeyStr);
    UserPreferences.set("global", "ReverseScroll", reverseScroll.isSelected());
    UserPreferences.set("global", "ViewOnly", viewOnly.isSelected());

    // DISPLAY
    //   (no GUI equivalent)
    UserPreferences.set("global", "AcceptBell", acceptBell.isSelected());
    if (VncViewer.colors.getValue() != -1)
      UserPreferences.set("global", "Colors", VncViewer.colors.getValue());
    UserPreferences.set("global", "CursorShape", cursorShape.isSelected());
    UserPreferences.set("global", "DesktopSize",
                        desktopSize.getSelectedItem().toString());
    //   (no GUI equivalent)
    UserPreferences.set("global", "FSAltEnter",
                        VncViewer.fsAltEnter.getValue());
    UserPreferences.set("global", "FullScreen", fullScreen.isSelected());
    int sf =
      Options.parseScalingFactor(scalingFactor.getSelectedItem().toString());
    if (sf == Options.SCALE_AUTO)
      UserPreferences.set("global", "Scale", "Auto");
    else if (sf == Options.SCALE_FIXEDRATIO)
      UserPreferences.set("global", "Scale", "FixedRatio");
    else
      UserPreferences.set("global", "Scale", sf);
    String spanString = ((String)span.getSelectedItem()).toLowerCase();
    if (spanString.startsWith("p"))
      UserPreferences.set("global", "Span", "Primary");
    else if (spanString.startsWith("al"))
      UserPreferences.set("global", "Span", "All");
    else
      UserPreferences.set("global", "Span", "Auto");
    UserPreferences.set("global", "Toolbar", showToolbar.isSelected());

    // ENCODING
    UserPreferences.set("global", "CompressLevel", getCompressionLevel());
    //   (no GUI equivalent)
    if (VncViewer.preferredEncoding.getValue() != null)
      UserPreferences.set("global", "Encoding",
                          VncViewer.preferredEncoding.getValue());
    UserPreferences.set("global", "JPEG", allowJpeg.isSelected());
    UserPreferences.set("global", "Quality", jpegQualityLevel.getValue());
    String subsamplingStr =
      subsamplingLabelTable.get(subsamplingLevel.getValue());
    UserPreferences.set("global", "Subsampling", subsamplingStr);

    // SECURITY AND AUTHENTICATION
    UserPreferences.set("global", "secVeNCrypt", secVeNCrypt.isSelected());
    UserPreferences.set("global", "encNone", encNone.isSelected());
    UserPreferences.set("global", "encTLS", encTLS.isSelected());
    UserPreferences.set("global", "encX509", encX509.isSelected());
    UserPreferences.set("global", "secNone", secNone.isSelected());
    UserPreferences.set("global", "secVnc", secVnc.isSelected());
    UserPreferences.set("global", "secUnixLogin", secUnixLogin.isSelected());
    UserPreferences.set("global", "secPlain", secPlain.isSelected());
    UserPreferences.set("global", "secIdent", secIdent.isSelected());
    UserPreferences.set("global", "x509ca", x509ca.getText());
    UserPreferences.set("global", "x509crl", x509crl.getText());

    UserPreferences.set("global", "SendLocalUsername",
                        sendLocalUsername.isSelected());
    UserPreferences.set("global", "tunnel", tunnel.isSelected());
    if (!sshUser.getText().isEmpty())
      UserPreferences.set("global", "via", sshUser.getText() + "@" +
                          gateway.getText());
    else
      UserPreferences.set("global", "via", gateway.getText());
  }

  JRadioButton addRadioCheckbox(String str, ButtonGroup group, JPanel panel) {
    JRadioButton c = new JRadioButton(str);
    GridBagConstraints gbc = new GridBagConstraints();
    gbc.anchor = GridBagConstraints.LINE_START;
    gbc.gridwidth = GridBagConstraints.REMAINDER;
    gbc.weightx = 1;
    gbc.weighty = 1;
    panel.add(c, gbc);
    group.add(c);
    c.addItemListener(this);
    return c;
  }

  JCheckBox addCheckbox(String str, ButtonGroup group, JPanel panel) {
    JCheckBox c = new JCheckBox(str);
    GridBagConstraints gbc = new GridBagConstraints();
    gbc.anchor = GridBagConstraints.LINE_START;
    gbc.gridwidth = GridBagConstraints.REMAINDER;
    gbc.weightx = 1;
    gbc.weighty = 1;
    panel.add(c, gbc);
    if (group != null)
      group.add(c);
    c.addItemListener(this);
    return c;
  }

  JCheckBox addJCheckBox(String str, ButtonGroup group, JPanel panel,
      GridBagConstraints gbc) {
    JCheckBox c = new JCheckBox(str);
    panel.add(c, gbc);
    if (group != null)
      group.add(c);
    c.addItemListener(this);

    return c;
  }

  public void actionPerformed(ActionEvent e) {
    Object s = e.getSource();
    if (s instanceof JButton && (JButton)s == okButton) {
      updatePreferences();
      UserPreferences.save();
      if (callback != null) callback.getOptions();
      endDialog();
    } else if (s instanceof JButton && (JButton)s == cancelButton) {
      endDialog();
    } else if (s instanceof JButton && (JButton)s == defClearButton) {
      UserPreferences.clear();
    } else if (s instanceof JButton && (JButton)s == x509caButton) {
      File file = new File(x509ca.getText());
      JFileChooser fc = new JFileChooser(file.getParent());
      fc.setSelectedFile(file);
      fc.setDialogTitle("Path to X.509 CA certificate");
      fc.setApproveButtonText("OK");
      fc.setFileHidingEnabled(false);
      int ret = fc.showOpenDialog(getJDialog());
      if (ret == JFileChooser.APPROVE_OPTION)
        x509ca.setText(fc.getSelectedFile().toString());
    } else if (s instanceof JButton && (JButton)s == x509crlButton) {
      File file = new File(x509crl.getText());
      JFileChooser fc = new JFileChooser(file.getParent());
      fc.setSelectedFile(file);
      fc.setDialogTitle("Path to X.509 Certificate Revocation List");
      fc.setApproveButtonText("OK");
      fc.setFileHidingEnabled(false);
      int ret = fc.showOpenDialog(getJDialog());
      if (ret == JFileChooser.APPROVE_OPTION)
        x509crl.setText(fc.getSelectedFile().toString());
    } else if (s instanceof JComboBox && (JComboBox)s == encMethodComboBox) {
      JComboBox comboBox = (JComboBox)e.getSource();
      String encMethod = (String)comboBox.getSelectedItem();
      if (!encMethodComboBox.isEnabled()) return;
      if (encMethod.equals("Tight + Perceptually Lossless JPEG (LAN)")) {
        allowJpeg.setSelected(true);
        subsamplingLevel.setValue(0);
        jpegQualityLevel.setValue(95);
        setCompressionLevel(1);
      } else if (encMethod.equals("Tight + Medium-Quality JPEG")) {
        allowJpeg.setSelected(true);
        subsamplingLevel.setValue(1);
        jpegQualityLevel.setValue(80);
        setCompressionLevel(6);
      } else if (encMethod.equals("Tight + Low-Quality JPEG (WAN)")) {
        allowJpeg.setSelected(true);
        subsamplingLevel.setValue(2);
        jpegQualityLevel.setValue(30);
        setCompressionLevel(7);
      } else if (encMethod.equals("Lossless Tight (Gigabit)")) {
        allowJpeg.setSelected(false);
        setCompressionLevel(0);
      } else if (encMethod.equals("Lossless Tight + Zlib (WAN)")) {
        allowJpeg.setSelected(false);
        setCompressionLevel(6);
      }
    }
  }

  public void itemStateChanged(ItemEvent e) {
    Object s = e.getSource();
    if (s instanceof JCheckBox && (JCheckBox)s == allowJpeg) {
      if (allowJpeg.isSelected()) {
        jpegQualityLevel.setEnabled(true);
        jpegQualityLabel.setEnabled(true);
        jpegQualityLabelLo.setEnabled(true);
        jpegQualityLabelHi.setEnabled(true);
        subsamplingLevel.setEnabled(true);
        subsamplingLabel.setEnabled(true);
        subsamplingLabelLo.setEnabled(true);
        subsamplingLabelHi.setEnabled(true);
        if (compressionLevel.getMaximum() < 9) {
          compressionLevel.setMinimum(1);
          compressionLevel.setMaximum(2);
        }
      } else {
        jpegQualityLevel.setEnabled(false);
        jpegQualityLabel.setEnabled(false);
        jpegQualityLabelLo.setEnabled(false);
        jpegQualityLabelHi.setEnabled(false);
        subsamplingLevel.setEnabled(false);
        subsamplingLabel.setEnabled(false);
        subsamplingLabelLo.setEnabled(false);
        subsamplingLabelHi.setEnabled(false);
        if (compressionLevel.getMaximum() < 9) {
          compressionLevel.setMinimum(0);
          compressionLevel.setMaximum(1);
        }
      }
      setEncMethodComboBox();
    }
    if (s instanceof JCheckBox && (JCheckBox)s == interframe) {
      setEncMethodComboBox();
      compressionLabel.setText(compressionLabelString + getCompressionLevel());
    }
    if (s instanceof JCheckBox && (JCheckBox)s == secVeNCrypt) {
      encNone.setEnabled(secVeNCrypt.isSelected());
      encTLS.setEnabled(secVeNCrypt.isSelected());
      encX509.setEnabled(secVeNCrypt.isSelected());
      x509ca.setEnabled(encX509.isSelected() && secVeNCrypt.isSelected());
      x509caButton.setEnabled(encX509.isSelected() &&
                              secVeNCrypt.isSelected());
      x509crl.setEnabled(encX509.isSelected() && secVeNCrypt.isSelected());
      x509crlButton.setEnabled(encX509.isSelected() &&
                               secVeNCrypt.isSelected());
      secIdent.setEnabled(secVeNCrypt.isSelected());
      secPlain.setEnabled(secVeNCrypt.isSelected());
    }
    if (s instanceof JCheckBox && (JCheckBox)s == encX509) {
      x509ca.setEnabled(encX509.isSelected() && secVeNCrypt.isSelected());
      x509caButton.setEnabled(encX509.isSelected() &&
                              secVeNCrypt.isSelected());
      x509crl.setEnabled(encX509.isSelected() && secVeNCrypt.isSelected());
      x509crlButton.setEnabled(encX509.isSelected() &&
                               secVeNCrypt.isSelected());
    }
    if (s instanceof JCheckBox && (JCheckBox)s == secIdent ||
        s instanceof JCheckBox && (JCheckBox)s == secPlain ||
        s instanceof JCheckBox && (JCheckBox)s == secUnixLogin ||
        s instanceof JCheckBox && (JCheckBox)s == secVeNCrypt) {
      sendLocalUsername.setEnabled(
        (secIdent.isSelected() && secIdent.isEnabled()) ||
        (secPlain.isSelected() && secPlain.isEnabled()) ||
        (secUnixLogin.isSelected() && secUnixLogin.isEnabled()));
    }
    if (s instanceof JComboBox && (JComboBox)s == scalingFactor) {
      String newScalingFactor = scalingFactor.getSelectedItem().toString();
      int sf = Options.parseScalingFactor(newScalingFactor);
      if (sf == 0) {
        vlog.error("Bogus scaling factor");
        scalingFactor.setSelectedItem(oldScalingFactor);
      } else {
        String newsf;
        if (sf == Options.SCALE_AUTO)
          newsf = "Auto";
        else if (sf == Options.SCALE_FIXEDRATIO)
          newsf = "Fixed Aspect Ratio";
        else
          newsf = sf + "%";
        oldScalingFactor = newsf;
        if (!newsf.equals(newScalingFactor))
          scalingFactor.setSelectedItem(newsf);
        if ((sf == Options.SCALE_AUTO || sf == Options.SCALE_FIXEDRATIO) &&
          desktopSize.getSelectedItem().toString().equals("Auto"))
          desktopSize.setSelectedItem("Server");
        else
          desktopSize.setEnabled(callback.supportsSetDesktopSize());
      }
    }
    if (s instanceof JComboBox && (JComboBox)s == desktopSize) {
      String newDesktopSize = desktopSize.getSelectedItem().toString();
      Options.DesktopSize size = Options.parseDesktopSize(newDesktopSize);
      if (size == null) {
        vlog.error("Bogus desktop size");
        desktopSize.setSelectedItem(oldDesktopSize);
      } else {
        String newsize = size.getString();
        oldDesktopSize = newsize;
        if (!newsize.equals(newDesktopSize))
          desktopSize.setSelectedItem(newsize);
        if (size.mode == Options.SIZE_AUTO) {
          scalingFactor.setEnabled(false);
          scalingFactor.setSelectedItem("100%");
        } else
          scalingFactor.setEnabled(true);
      }
    }
    if (s instanceof JCheckBox && (JCheckBox)s == tunnel)
      gateway.setEnabled(!tunnel.isSelected());
  }

  private void setEncMethodComboBox() {
    if (!encMethodComboBox.isEnabled()) return;
    int level = getCompressionLevel();
    if (subsamplingLevel.getValue() == 0 && level == 1 &&
        jpegQualityLevel.getValue() == 95 && allowJpeg.isSelected()) {
      encMethodComboBox.setSelectedItem("Tight + Perceptually Lossless JPEG (LAN)");
      if (encMethodComboBox.getItemCount() > 5)
        encMethodComboBox.removeItem("Custom");
    } else if (subsamplingLevel.getValue() == 1 && level == 6 &&
               jpegQualityLevel.getValue() == 80 && allowJpeg.isSelected()) {
      encMethodComboBox.setSelectedItem("Tight + Medium-Quality JPEG");
      if (encMethodComboBox.getItemCount() > 5)
        encMethodComboBox.removeItem("Custom");
    } else if (subsamplingLevel.getValue() == 2 && level == 7 &&
               jpegQualityLevel.getValue() == 30 && allowJpeg.isSelected()) {
      encMethodComboBox.setSelectedItem("Tight + Low-Quality JPEG (WAN)");
      if (encMethodComboBox.getItemCount() > 5)
        encMethodComboBox.removeItem("Custom");
    } else if (level == 0 && !allowJpeg.isSelected()) {
      encMethodComboBox.setSelectedItem("Lossless Tight (Gigabit)");
      if (encMethodComboBox.getItemCount() > 5)
        encMethodComboBox.removeItem("Custom");
    } else if (level == 6 && !allowJpeg.isSelected()) {
      encMethodComboBox.setSelectedItem("Lossless Tight + Zlib (WAN)");
      if (encMethodComboBox.getItemCount() > 5)
        encMethodComboBox.removeItem("Custom");
    } else {
      if (encMethodComboBox.getItemCount() <= 5)
        encMethodComboBox.addItem("Custom");
      encMethodComboBox.setSelectedItem("Custom");
    }
  }

  public void stateChanged(ChangeEvent e) {
    Object s = e.getSource();
    if (s instanceof JSlider && (JSlider)s == subsamplingLevel) {
      subsamplingLabel.setText(subsamplingLabelString +
        subsamplingLabelTable.get(subsamplingLevel.getValue()));
      setEncMethodComboBox();
    } else if (s instanceof JSlider && (JSlider)s == jpegQualityLevel) {
      jpegQualityLabel.setText(jpegQualityLabelString +
        jpegQualityLevel.getValue());
      setEncMethodComboBox();
    } else if (s instanceof JSlider && (JSlider)s == compressionLevel) {
      compressionLabel.setText(compressionLabelString + getCompressionLevel());
      setEncMethodComboBox();
    }
  }

  public int getSubsamplingLevel() {
    switch (subsamplingLevel.getValue()) {
      case 1:
        return Options.SUBSAMP_2X;
      case 2:
        return Options.SUBSAMP_4X;
      case 3:
        return Options.SUBSAMP_GRAY;
    }
    return Options.SUBSAMP_NONE;
  }

  boolean isTurboCompressionLevel(int level) {
    if (allowJpeg.isSelected() &&
        ((level >= 1 && level <= 2) || (level >= 6 && level <= 7)))
      return true;
    if (!allowJpeg.isSelected() &&
        ((level >= 0 && level <= 1) || (level >= 5 && level <= 6)))
      return true;
    return false;
  }

  public void setCompressionLevel(int level) {
    boolean selectICE = false;
    if (!isTurboCompressionLevel(level)) {
      compressionLevel.setMinimum(0);
      compressionLevel.setMaximum(9);
      compressionLabelString = new String("Compression level: ");
      interframe.setEnabled(false);
    }
    if (compressionLevel.getMaximum() < 9 &&
        interframe.isEnabled()) {
      if (level >= 5 && level <= 8) {
        level -= 5;
        selectICE = true;
      }
    }
    compressionLevel.setValue(level);
    interframe.setSelected(selectICE);
  }

  public int getCompressionLevel() {
    int level = compressionLevel.getValue();
    if (interframe.isEnabled() && interframe.isSelected() && level <= 3)
      level += 5;
    return level;
  }

  void setTightOptions(int encoding) {
    if (encoding != RFB.ENCODING_TIGHT) {
      allowJpeg.setEnabled(false);
      subsamplingLevel.setEnabled(false);
      subsamplingLabel.setEnabled(false);
      subsamplingLabelLo.setEnabled(false);
      subsamplingLabelHi.setEnabled(false);
      jpegQualityLevel.setMinimum(0);
      jpegQualityLevel.setMaximum(9);
      jpegQualityLevel.setMajorTickSpacing(1);
      jpegQualityLevel.setMinorTickSpacing(0);
      jpegQualityLevel.setSnapToTicks(true);
      jpegQualityLevel.setEnabled(true);
      jpegQualityLabelString = new String("Image quality level: ");
      jpegQualityLabel.setText(jpegQualityLabelString +
        jpegQualityLevel.getValue());
      jpegQualityLabel.setEnabled(true);
      jpegQualityLabelLo.setEnabled(true);
      jpegQualityLabelHi.setEnabled(true);
      encMethodComboBox.setEnabled(false);
      if (encMethodComboBox.getItemCount() > 5)
        encMethodComboBox.removeItemAt(5);
      encMethodComboBox.insertItemAt(RFB.encodingName(encoding), 5);
      encMethodComboBox.setSelectedItem(RFB.encodingName(encoding));
      encMethodLabel.setText("Encoding type:");
      encMethodLabel.setEnabled(false);
    }
    if (encoding != RFB.ENCODING_TIGHT || VncViewer.compatibleGUI.getValue()) {
      compressionLevel.setMinimum(0);
      compressionLevel.setMaximum(9);
      compressionLabelString = new String("Compression level: ");
      compressionLabel.setText(compressionLabelString + getCompressionLevel());
      interframe.setEnabled(false);
    }
  }

  void setSecurityOptions() {
    /* Process non-VeNCrypt sectypes */
    java.util.List<Integer> secTypes = new ArrayList<Integer>();
    secTypes = Security.getEnabledSecTypes();
    boolean enableVeNCrypt = false;
    for (Iterator<Integer> i = secTypes.iterator(); i.hasNext();) {
      switch ((Integer)i.next()) {
        case RFB.SECTYPE_VENCRYPT:
          enableVeNCrypt = true;
          break;
        case RFB.SECTYPE_NONE:
          secNone.setSelected(true);
          break;
        case RFB.SECTYPE_VNCAUTH:
          secVnc.setSelected(true);
          break;
        case RFB.SECTYPE_UNIX_LOGIN:
          secUnixLogin.setSelected(true);
          break;
      }
    }

    /* Process VeNCrypt subtypes */
    if (enableVeNCrypt) {
      java.util.List<Integer> secTypesExt = new ArrayList<Integer>();
      secTypesExt = Security.getEnabledExtSecTypes();
      for (Iterator<Integer> iext = secTypesExt.iterator(); iext.hasNext();) {
        switch ((Integer)iext.next()) {
          case RFB.SECTYPE_PLAIN:
            secVeNCrypt.setSelected(true);
            encNone.setSelected(true);
            secPlain.setSelected(true);
            break;
          case RFB.SECTYPE_IDENT:
            secVeNCrypt.setSelected(true);
            encNone.setSelected(true);
            secIdent.setSelected(true);
            break;
          case RFB.SECTYPE_TLS_NONE:
            secVeNCrypt.setSelected(true);
            encTLS.setSelected(true);
            secNone.setSelected(true);
            break;
          case RFB.SECTYPE_TLS_VNC:
            secVeNCrypt.setSelected(true);
            encTLS.setSelected(true);
            secVnc.setSelected(true);
            break;
          case RFB.SECTYPE_TLS_PLAIN:
            secVeNCrypt.setSelected(true);
            encTLS.setSelected(true);
            secPlain.setSelected(true);
            break;
          case RFB.SECTYPE_TLS_IDENT:
            secVeNCrypt.setSelected(true);
            encTLS.setSelected(true);
            secIdent.setSelected(true);
            break;
          case RFB.SECTYPE_X509_NONE:
            secVeNCrypt.setSelected(true);
            encX509.setSelected(true);
            secNone.setSelected(true);
            break;
          case RFB.SECTYPE_X509_VNC:
            secVeNCrypt.setSelected(true);
            encX509.setSelected(true);
            secVnc.setSelected(true);
            break;
          case RFB.SECTYPE_X509_PLAIN:
            secVeNCrypt.setSelected(true);
            encX509.setSelected(true);
            secPlain.setSelected(true);
            break;
          case RFB.SECTYPE_X509_IDENT:
            secVeNCrypt.setSelected(true);
            encX509.setSelected(true);
            secIdent.setSelected(true);
            break;
        }
      }
    }
    if (!secVeNCrypt.isSelected()) {
      encNone.setEnabled(false);
      encTLS.setEnabled(false);
      encX509.setEnabled(false);
      secIdent.setEnabled(false);
      secPlain.setEnabled(false);
    }
    if (!encX509.isSelected() || !secVeNCrypt.isSelected()) {
      x509ca.setEnabled(false);
      x509caButton.setEnabled(false);
      x509crl.setEnabled(false);
      x509crlButton.setEnabled(false);
    }
    sendLocalUsername.setEnabled(
      (secIdent.isSelected() && secIdent.isEnabled()) ||
      (secPlain.isSelected() && secPlain.isEnabled()) ||
      (secUnixLogin.isSelected() && secUnixLogin.isEnabled()));
  }

  public void getSecurityOptions() {
    Security.disableSecType(RFB.SECTYPE_NONE);
    Security.disableSecType(RFB.SECTYPE_VNCAUTH);
    Security.disableSecType(RFB.SECTYPE_PLAIN);
    Security.disableSecType(RFB.SECTYPE_IDENT);
    Security.disableSecType(RFB.SECTYPE_TLS_NONE);
    Security.disableSecType(RFB.SECTYPE_TLS_VNC);
    Security.disableSecType(RFB.SECTYPE_TLS_PLAIN);
    Security.disableSecType(RFB.SECTYPE_TLS_IDENT);
    Security.disableSecType(RFB.SECTYPE_X509_NONE);
    Security.disableSecType(RFB.SECTYPE_X509_VNC);
    Security.disableSecType(RFB.SECTYPE_X509_PLAIN);
    Security.disableSecType(RFB.SECTYPE_X509_IDENT);
    Security.disableSecType(RFB.SECTYPE_UNIX_LOGIN);

    /* Process security types which don't use encryption */
    if (encNone.isSelected() || !secVeNCrypt.isSelected()) {
      if (secNone.isSelected())
        Security.enableSecType(RFB.SECTYPE_NONE);

      if (secVnc.isSelected())
        Security.enableSecType(RFB.SECTYPE_VNCAUTH);
    }

    if (encNone.isSelected() && secVeNCrypt.isSelected()) {
      if (secPlain.isSelected())
        Security.enableSecType(RFB.SECTYPE_PLAIN);

      if (secIdent.isSelected())
        Security.enableSecType(RFB.SECTYPE_IDENT);
    }

    /* Process security types which use TLS encryption */
    if (encTLS.isSelected() && secVeNCrypt.isSelected()) {
      if (secNone.isSelected())
        Security.enableSecType(RFB.SECTYPE_TLS_NONE);

      if (secVnc.isSelected())
        Security.enableSecType(RFB.SECTYPE_TLS_VNC);

      if (secPlain.isSelected())
        Security.enableSecType(RFB.SECTYPE_TLS_PLAIN);

      if (secIdent.isSelected())
        Security.enableSecType(RFB.SECTYPE_TLS_IDENT);
    }

    /* Process security types which use X509 encryption */
    if (encX509.isSelected() && secVeNCrypt.isSelected()) {
      if (secNone.isSelected())
        Security.enableSecType(RFB.SECTYPE_X509_NONE);

      if (secVnc.isSelected())
        Security.enableSecType(RFB.SECTYPE_X509_VNC);

      if (secPlain.isSelected())
        Security.enableSecType(RFB.SECTYPE_X509_PLAIN);

      if (secIdent.isSelected())
        Security.enableSecType(RFB.SECTYPE_X509_IDENT);
    }

    if (secUnixLogin.isSelected())
      Security.enableSecType(RFB.SECTYPE_UNIX_LOGIN);
  }

}
