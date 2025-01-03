/* Copyright (C) 2012-2018, 2020-2022, 2025 D. R. Commander.
 *                                          All Rights Reserved.
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
import java.io.File;
import java.util.*;
import javax.swing.*;
import javax.swing.border.*;
import javax.swing.event.*;

import com.turbovnc.rdr.*;
import com.turbovnc.rfb.*;

class OptionsDialog extends Dialog implements ActionListener, ChangeListener,
  ItemListener, DocumentListener {

  // Constants
  // Static variables
  static LogWriter vlog = new LogWriter("OptionsDialog");

  private OptionsDialogCallback callback;
  private JTabbedPane tabPane;
  private JPanel buttonPane, encodingPanel, connPanel, globalPanel, secPanel;
  private JCheckBox allowJpeg, interframe;
  private JComboBox menuKey, scalingFactor, encMethodComboBox, span,
    desktopSize, grabKeyboard;
  private JSlider jpegQualityLevel, subsamplingLevel, compressionLevel;
  private JCheckBox viewOnly, recvClipboard, sendClipboard, acceptBell,
    reverseScroll, fsAltEnter;
  private JCheckBox fullScreen, shared, cursorShape, showToolbar;
  private JCheckBox secNone, secVnc, secPlain, secIdent, secTLSNone, secTLSVnc,
    secTLSPlain, secTLSIdent, secX509None, secX509Vnc, secX509Plain,
    secX509Ident, secUnixLogin;
  private JPanel encNonePanel, encTLSPanel, encX509Panel;
  private JCheckBox sendLocalUsername, tunnel;
  private JTextField username, sshUser, gateway;
  private JLabel usernameLabel, sshUserLabel, gatewayLabel;
  private JButton okButton, cancelButton;
  private JButton x509caButton, x509crlButton;
  private JLabel x509caLabel, x509crlLabel;
  private JTextField x509ca, x509crl;
  private JButton defClearButton;
  private JLabel encMethodLabel;
  private JLabel jpegQualityLabel, jpegQualityLabelLo, jpegQualityLabelHi;
  private JLabel subsamplingLabel, subsamplingLabelLo, subsamplingLabelHi;
  private JLabel compressionLabel, compressionLabelLo, compressionLabelHi;
  private String jpegQualityLabelString, subsamplingLabelString;
  private String compressionLabelString;
  private Hashtable<Integer, String> subsamplingLabelTable;
  private String oldScalingFactor, oldDesktopSize;
  private boolean enableX509 = true;

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
      "3200x1800", "3200x1800+0+0,3200x1800+3200+0",
      "3840x2160", "3840x2160+0+0,3840x2160+3840+0"
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
    if (Utils.isX11() && Helper.isAvailable())
      spanLabel = new JLabel("Full-screen span mode:");
    else
      spanLabel = new JLabel("Span mode:");
    span = new JComboBox(spanOptions);
    span.addItemListener(this);
    if (Utils.isX11() && !Helper.isAvailable()) {
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

    showToolbar = new JCheckBox("Show toolbar");
    showToolbar.addItemListener(this);

    Dialog.addGBComponent(showToolbar, displayPanel,
                          0, 6, 2, 1, 2, 2, 1, 0,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(4, 5, 0, 5));

    JPanel inputPanel = new JPanel(new GridBagLayout());
    inputPanel.setBorder(BorderFactory.createTitledBorder("Input"));

    viewOnly = new JCheckBox("View only (ignore mouse & keyboard)");
    viewOnly.addItemListener(this);
    viewOnly.setEnabled(Params.viewOnlyControl.getValue());

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

    fsAltEnter = new JCheckBox("Toggle full-screen mode with Alt-Enter");
    fsAltEnter.addItemListener(this);

    Dialog.addGBComponent(fsAltEnter, inputPanel,
                          0, 2, 2, 1, 2, 2, 1, 0,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.LINE_START,
                          new Insets(4, 5, 0, 5));

    JLabel menuKeyLabel = new JLabel("Menu key:");
    String[] menuKeys = new String[MenuKey.getMenuKeySymbolCount()];
    for (int i = 0; i < MenuKey.getMenuKeySymbolCount(); i++)
      menuKeys[i] = MenuKey.getMenuKeySymbols()[i].name;
    menuKey  = new JComboBox(menuKeys);
    menuKey.addItemListener(this);

    Dialog.addGBComponent(menuKeyLabel, inputPanel,
                          0, 3, 1, 1, 2, 2, 1, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(8, 8, 0, 5));
    Dialog.addGBComponent(menuKey, inputPanel,
                          1, 3, 1, 1, 2, 2, 25, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(4, 5, 0, 5));

    boolean enableGrab = Utils.osGrab() && Helper.isAvailable();

    if (enableGrab) {
      JLabel grabLabel;
      if (Utils.isX11() && Params.grabPointer.getValue())
        grabLabel = new JLabel("Keyboard/pointer grab mode:");
      else
        grabLabel = new JLabel("Keyboard grab mode:");
      Object[] grabModes = { "Full-screen only", "Always", "Manual" };
      grabKeyboard = new JComboBox(grabModes);
      grabKeyboard.addItemListener(this);

      Dialog.addGBComponent(grabLabel, inputPanel,
                            0, 4, 1, 1, 2, 2, 1, 0,
                            GridBagConstraints.NONE,
                            GridBagConstraints.FIRST_LINE_START,
                            new Insets(8, 8, 0, 5));
      Dialog.addGBComponent(grabKeyboard, inputPanel,
                            1, 4, 1, 1, 2, 2, 25, 0,
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

    recvClipboard = new JCheckBox("Accept clipboard from server");
    recvClipboard.addItemListener(this);

    Dialog.addGBComponent(recvClipboard, restrictionsPanel,
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

    defClearButton = new JButton("Clear the list of saved connections");
    defClearButton.addActionListener(this);

    Dialog.addGBComponent(defClearButton, globalPanel,
                          0, 0, 2, GridBagConstraints.REMAINDER, 2, 2, 1, 1,
                          GridBagConstraints.NONE,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(8, 5, 0, 5));

    // Security tab
    secPanel = new JPanel(new GridBagLayout());

    JPanel secTypesPanel = new JPanel(new GridBagLayout());
    secTypesPanel.setBorder(
      BorderFactory.createTitledBorder("Security types"));

    encNonePanel = new JPanel(new GridBagLayout());
    Border border = BorderFactory.createLineBorder(Color.LIGHT_GRAY);
    TitledBorder tb = BorderFactory.createTitledBorder(border, "No encryption",
      TitledBorder.CENTER, TitledBorder.TOP);
    encNonePanel.setBorder(tb);

    secNone = addJCheckBox("No authentication", null, encNonePanel,
                           new GridBagConstraints(0, 0, 1, 1, 1, 1,
                             GridBagConstraints.LINE_START,
                             GridBagConstraints.NONE,
                             new Insets(0, 0, 0, 5), 0, 0));
    secVnc = addJCheckBox("Standard VNC", null, encNonePanel,
                          new GridBagConstraints(0, 1, 1, 1, 1, 1,
                            GridBagConstraints.LINE_START,
                            GridBagConstraints.NONE,
                            new Insets(0, 0, 0, 5), 0, 0));
    secPlain = addJCheckBox("Plain (VeNCrypt)", null, encNonePanel,
                            new GridBagConstraints(1, 0, 1, 1, 1, 1,
                              GridBagConstraints.LINE_START,
                              GridBagConstraints.NONE,
                              new Insets(0, 0, 0, 5), 0, 0));
    secIdent = addJCheckBox("Ident (VeNCrypt)", null, encNonePanel,
                            new GridBagConstraints(1, 1, 1, 1, 1, 1,
                              GridBagConstraints.LINE_START,
                              GridBagConstraints.NONE,
                              new Insets(0, 0, 0, 5), 0, 0));
    secUnixLogin = addJCheckBox("Unix Login (TightVNC/TurboVNC)", null,
                                encNonePanel,
                                new GridBagConstraints(1, 2, 1, 1, 1, 1,
                                  GridBagConstraints.LINE_START,
                                  GridBagConstraints.NONE,
                                  new Insets(0, 0, 0, 5), 0, 0));

    encTLSPanel = new JPanel(new GridBagLayout());
    border = BorderFactory.createLineBorder(Color.LIGHT_GRAY);
    tb = BorderFactory.createTitledBorder(border,
      "Anonymous TLS encryption (VeNCrypt)", TitledBorder.CENTER,
      TitledBorder.TOP);
    encTLSPanel.setBorder(tb);

    secTLSNone = addJCheckBox("No authentication", null, encTLSPanel,
                              new GridBagConstraints(0, 0, 1, 1, 1, 1,
                                GridBagConstraints.LINE_START,
                                GridBagConstraints.NONE,
                                new Insets(0, 0, 0, 5), 0, 0));
    secTLSVnc = addJCheckBox("Standard VNC", null, encTLSPanel,
                             new GridBagConstraints(0, 1, 1, 1, 1, 1,
                               GridBagConstraints.LINE_START,
                               GridBagConstraints.NONE,
                               new Insets(0, 0, 0, 5), 0, 0));
    secTLSPlain = addJCheckBox("Plain", null, encTLSPanel,
                               new GridBagConstraints(1, 0, 1, 1, 1, 1,
                                 GridBagConstraints.LINE_START,
                                 GridBagConstraints.NONE,
                                 new Insets(0, 0, 0, 5), 0, 0));
    secTLSIdent = addJCheckBox("Ident", null, encTLSPanel,
                               new GridBagConstraints(1, 1, 1, 1, 1, 1,
                                 GridBagConstraints.LINE_START,
                                 GridBagConstraints.NONE,
                                 new Insets(0, 0, 0, 5), 0, 0));

    encX509Panel = new JPanel(new GridBagLayout());
    border = BorderFactory.createLineBorder(Color.LIGHT_GRAY);
    tb = BorderFactory.createTitledBorder(border,
      "TLS encryption with X.509 certificates (VeNCrypt)",
      TitledBorder.CENTER, TitledBorder.TOP);
    encX509Panel.setBorder(tb);

    secX509None = addJCheckBox("No authentication", null, encX509Panel,
                               new GridBagConstraints(0, 0, 1, 1, 1, 1,
                                 GridBagConstraints.LINE_START,
                                 GridBagConstraints.NONE,
                                 new Insets(0, 0, 0, 5), 0, 0));
    secX509Vnc = addJCheckBox("Standard VNC", null, encX509Panel,
                              new GridBagConstraints(0, 1, 1, 1, 1, 1,
                                GridBagConstraints.LINE_START,
                                GridBagConstraints.NONE,
                                new Insets(0, 0, 0, 5), 0, 0));
    secX509Plain = addJCheckBox("Plain", null, encX509Panel,
                                new GridBagConstraints(1, 0, 1, 1, 1, 1,
                                  GridBagConstraints.LINE_START,
                                  GridBagConstraints.NONE,
                                  new Insets(0, 0, 0, 5), 0, 0));
    secX509Ident = addJCheckBox("Ident", null, encX509Panel,
                                new GridBagConstraints(1, 1, 1, 1, 1, 1,
                                  GridBagConstraints.LINE_START,
                                  GridBagConstraints.NONE,
                                  new Insets(0, 0, 0, 5), 0, 0));

    Dialog.addGBComponent(encNonePanel, secTypesPanel,
                          0, 0, 1, 1, 2, 2, 1, 0,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 10, 2, 5));
    Dialog.addGBComponent(encTLSPanel, secTypesPanel,
                          0, 1, 1, 1, 2, 2, 1, 1,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(2, 10, 2, 5));
    Dialog.addGBComponent(encX509Panel, secTypesPanel,
                          0, 2, 1, 1, 2, 2, 1, 1,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(2, 10, 2, 5));

    JPanel x509Panel = new JPanel(new GridBagLayout());
    x509Panel.setBorder(
      BorderFactory.createTitledBorder("X.509 certificate validation"));
    x509ca = new JTextField("", 1);
    x509caLabel = new JLabel("CA cert:");
    x509caButton = new JButton("Load");
    x509caButton.addActionListener(this);
    x509crl = new JTextField("", 1);
    x509crlLabel = new JLabel("CRL:");
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

    username = new JTextField("", 1);
    username.getDocument().addDocumentListener(this);
    usernameLabel = new JLabel("Username:");
    sendLocalUsername = new JCheckBox("Send local username");
    sendLocalUsername.addItemListener(this);

    JPanel gatewayPanel = new JPanel(new GridBagLayout());
    gatewayPanel.setBorder(
      BorderFactory.createTitledBorder("Gateway (SSH server or UltraVNC repeater)"));
    gateway = new JTextField("", 1);
    filterWhitespace(gateway);
    gatewayLabel = new JLabel("Host:");
    sshUser = new JTextField("", 1);
    sshUserLabel = new JLabel("SSH user:");
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

    Dialog.addGBComponent(secTypesPanel, secPanel,
                          0, 0, 3, 1, 2, 2, 1, 0,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(8, 10, 2, 5));
    Dialog.addGBComponent(usernameLabel, secPanel,
                          0, 1, 1, 1, 0, 0, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(8, 10, 0, 2));
    Dialog.addGBComponent(username, secPanel,
                          1, 1, 1, 1, 0, 0, 1, 0,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(4, 2, 0, 2));
    Dialog.addGBComponent(sendLocalUsername, secPanel,
                          2, 1, 1, 1, 0, 0, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.FIRST_LINE_END,
                          new Insets(4, 2, 0, 2));
    Dialog.addGBComponent(x509Panel, secPanel,
                          0, 2, 3, 1, 2, 2, 1, 1,
                          GridBagConstraints.HORIZONTAL,
                          GridBagConstraints.FIRST_LINE_START,
                          new Insets(2, 10, 2, 5));
    Dialog.addGBComponent(gatewayPanel, secPanel,
                          0, 3, 3, 1, 2, 2, 1, 1,
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
    if (s instanceof JCheckBox && (JCheckBox)s == secNone ||
        s instanceof JCheckBox && (JCheckBox)s == secVnc ||
        s instanceof JCheckBox && (JCheckBox)s == secPlain ||
        s instanceof JCheckBox && (JCheckBox)s == secIdent ||
        s instanceof JCheckBox && (JCheckBox)s == secUnixLogin ||
        s instanceof JCheckBox && (JCheckBox)s == secTLSNone ||
        s instanceof JCheckBox && (JCheckBox)s == secTLSVnc ||
        s instanceof JCheckBox && (JCheckBox)s == secTLSPlain ||
        s instanceof JCheckBox && (JCheckBox)s == secTLSIdent ||
        s instanceof JCheckBox && (JCheckBox)s == secX509None ||
        s instanceof JCheckBox && (JCheckBox)s == secX509Vnc ||
        s instanceof JCheckBox && (JCheckBox)s == secX509Plain ||
        s instanceof JCheckBox && (JCheckBox)s == secX509Ident ||
        s instanceof JCheckBox && (JCheckBox)s == sendLocalUsername) {
      updateSecurityPanel();
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
    if (s instanceof JCheckBox && (JCheckBox)s == tunnel) {
      gateway.setEnabled(!tunnel.isSelected());
      gatewayLabel.setEnabled(!tunnel.isSelected());
    }
  }

  public void changedUpdate(DocumentEvent e) {
    if (e.getDocument() == username.getDocument())
      updateSecurityPanel();
  }

  public void insertUpdate(DocumentEvent e) {
    if (e.getDocument() == username.getDocument())
      updateSecurityPanel();
  }

  public void removeUpdate(DocumentEvent e) {
    if (e.getDocument() == username.getDocument())
      updateSecurityPanel();
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
    if (encoding != RFB.ENCODING_TIGHT || Params.compatibleGUI.getValue()) {
      compressionLevel.setMinimum(0);
      compressionLevel.setMaximum(9);
      compressionLabelString = new String("Compression level: ");
      compressionLabel.setText(compressionLabelString + getCompressionLevel());
      interframe.setEnabled(false);
    }
  }

  private void updateSecurityPanel() {
    if (Params.noUnixLogin.getValue()) {
      secIdent.setEnabled(false);
      secPlain.setEnabled(false);
      secTLSIdent.setEnabled(false);
      secTLSPlain.setEnabled(false);
      secX509Ident.setEnabled(false);
      secX509Plain.setEnabled(false);
      secUnixLogin.setEnabled(false);
      sendLocalUsername.setEnabled(false);
      usernameLabel.setEnabled(false);
      username.setEnabled(false);
    }

    boolean unixLogin =
      ((secIdent.isEnabled() && secIdent.isSelected()) ||
       (secPlain.isEnabled() && secPlain.isSelected()) ||
       (secTLSIdent.isEnabled() && secTLSIdent.isSelected()) ||
       (secTLSPlain.isEnabled() && secTLSPlain.isSelected()) ||
       (secX509Ident.isEnabled() && secX509Ident.isSelected()) ||
       (secX509Plain.isEnabled() && secX509Plain.isSelected()) ||
       (secUnixLogin.isEnabled() && secUnixLogin.isSelected()));
    sendLocalUsername.setEnabled(unixLogin);
    username.setEnabled(unixLogin && !sendLocalUsername.isSelected());
    usernameLabel.setEnabled(unixLogin && !sendLocalUsername.isSelected());

    boolean unixLoginForced =
      (sendLocalUsername.isEnabled() && sendLocalUsername.isSelected()) ||
      (username.isEnabled() && !username.getText().isEmpty());
    secNone.setEnabled(!unixLoginForced);
    secVnc.setEnabled(!unixLoginForced);
    secTLSNone.setEnabled(!unixLoginForced);
    secTLSVnc.setEnabled(!unixLoginForced);
    secX509None.setEnabled(!unixLoginForced && enableX509);
    secX509Vnc.setEnabled(!unixLoginForced && enableX509);

    boolean x509 = (secX509None.isEnabled() && secX509None.isSelected()) ||
      (secX509Vnc.isEnabled() && secX509Vnc.isSelected()) ||
      (secX509Plain.isEnabled() && secX509Plain.isSelected()) ||
      (secX509Ident.isEnabled() && secX509Ident.isSelected());
    x509ca.setEnabled(x509 && enableX509);
    x509caButton.setEnabled(x509 && enableX509);
    x509caLabel.setEnabled(x509 && enableX509);
    x509crl.setEnabled(x509 && enableX509);
    x509crlButton.setEnabled(x509 && enableX509);
    x509crlLabel.setEnabled(x509 && enableX509);
  }

  public void setX509Enabled(boolean enabled) {
    enableX509 = enabled;
    if (!enabled) {
      secX509None.setEnabled(false);
      secX509Vnc.setEnabled(false);
      secX509Plain.setEnabled(false);
      secX509Ident.setEnabled(false);
      x509ca.setEnabled(false);
      x509caButton.setEnabled(false);
      x509caLabel.setEnabled(false);
      x509crl.setEnabled(false);
      x509crlButton.setEnabled(false);
      x509crlLabel.setEnabled(false);
    }
  }

  public void setOptions(Options opts, boolean enableDesktopSize,
                         boolean disableShared, boolean disableSecurity,
                         boolean disableSSH) {
    // Encoding
    allowJpeg.setSelected(opts.allowJpeg);
    subsamplingLevel.setValue(opts.getSubsamplingOrdinal());
    jpegQualityLevel.setValue(opts.quality);
    setCompressionLevel(opts.compressLevel);

    // Connection: Display
    if (opts.scalingFactor == Options.SCALE_AUTO) {
      scalingFactor.setSelectedItem("Auto");
    } else if (opts.scalingFactor == Options.SCALE_FIXEDRATIO) {
      scalingFactor.setSelectedItem("Fixed Aspect Ratio");
    } else {
      scalingFactor.setSelectedItem(opts.scalingFactor + "%");
    }

    desktopSize.setSelectedItem(opts.desktopSize.getString());
    fullScreen.setSelected(opts.fullScreen);
    span.setSelectedIndex(opts.span);
    acceptBell.setSelected(opts.acceptBell);
    cursorShape.setSelected(opts.cursorShape);
    showToolbar.setSelected(opts.showToolbar);

    // Connection: Input
    viewOnly.setSelected(opts.viewOnly);
    reverseScroll.setSelected(opts.reverseScroll);
    fsAltEnter.setSelected(opts.fsAltEnter);
    String menuKeyStr =
      MenuKey.getMenuKeyName(opts.menuKeyCode, opts.menuKeySym);
    if (menuKeyStr != null)
      menuKey.setSelectedItem(menuKeyStr);
    if (Utils.osGrab() && Helper.isAvailable())
      grabKeyboard.setSelectedIndex(opts.grabKeyboard);

    // Connection: Restrictions
    shared.setSelected(opts.shared);
    recvClipboard.setSelected(opts.recvClipboard);
    sendClipboard.setSelected(opts.sendClipboard);

    // Security: Security types
    secNone.setSelected(opts.isSecTypeSupported(RFB.SECTYPE_NONE));
    secVnc.setSelected(opts.isSecTypeSupported(RFB.SECTYPE_VNCAUTH));
    secPlain.setSelected(opts.isSecTypeSupported(RFB.SECTYPE_PLAIN));
    secIdent.setSelected(opts.isSecTypeSupported(RFB.SECTYPE_IDENT));
    secTLSNone.setSelected(opts.isSecTypeSupported(RFB.SECTYPE_TLS_NONE));
    secTLSVnc.setSelected(opts.isSecTypeSupported(RFB.SECTYPE_TLS_VNC));
    secTLSPlain.setSelected(opts.isSecTypeSupported(RFB.SECTYPE_TLS_PLAIN));
    secTLSIdent.setSelected(opts.isSecTypeSupported(RFB.SECTYPE_TLS_IDENT));
    secX509None.setSelected(opts.isSecTypeSupported(RFB.SECTYPE_X509_NONE));
    secX509Vnc.setSelected(opts.isSecTypeSupported(RFB.SECTYPE_X509_VNC));
    secX509Plain.setSelected(opts.isSecTypeSupported(RFB.SECTYPE_X509_PLAIN));
    secX509Ident.setSelected(opts.isSecTypeSupported(RFB.SECTYPE_X509_IDENT));
    secUnixLogin.setSelected(opts.isSecTypeSupported(RFB.SECTYPE_UNIX_LOGIN));

    // Security
    if (opts.user != null)
      username.setText(opts.user);
    sendLocalUsername.setSelected(opts.sendLocalUsername);

    // Security: X.509 certificate validation
    if (opts.x509ca != null)
      x509ca.setText(opts.x509ca);
    if (opts.x509crl != null)
      x509crl.setText(opts.x509crl);

    // Security: Gateway
    if (opts.sshUser != null)
      sshUser.setText(opts.sshUser);
    if (opts.via != null)
      gateway.setText(opts.via);
    tunnel.setSelected(opts.tunnel);

    desktopSize.setEnabled(enableDesktopSize);
    if (opts.desktopSize.mode == Options.SIZE_AUTO)
      scalingFactor.setEnabled(false);
    else
      scalingFactor.setEnabled(true);
    if (disableShared) shared.setEnabled(false);
    updateSecurityPanel();
    if (disableSecurity) {
      secNone.setEnabled(false);
      secVnc.setEnabled(false);
      secPlain.setEnabled(false);
      secIdent.setEnabled(false);
      secUnixLogin.setEnabled(false);
      secTLSNone.setEnabled(false);
      secTLSVnc.setEnabled(false);
      secTLSPlain.setEnabled(false);
      secTLSIdent.setEnabled(false);
      secX509None.setEnabled(false);
      secX509Vnc.setEnabled(false);
      secX509Plain.setEnabled(false);
      secX509Ident.setEnabled(false);
      username.setEnabled(false);
      usernameLabel.setEnabled(false);
      sendLocalUsername.setEnabled(false);
      x509ca.setEnabled(false);
      x509caButton.setEnabled(false);
      x509caLabel.setEnabled(false);
      x509crl.setEnabled(false);
      x509crlButton.setEnabled(false);
      x509crlLabel.setEnabled(false);
    }
    if (disableSSH) {
      sshUser.setEnabled(false);
      sshUserLabel.setEnabled(false);
      gateway.setEnabled(false);
      gatewayLabel.setEnabled(false);
      tunnel.setEnabled(false);
    }
  }

  public void getOptions(Options opts) {
    // Encoding
    opts.allowJpeg = allowJpeg.isSelected();
    opts.subsampling = getSubsamplingLevel();
    opts.quality = jpegQualityLevel.getValue();
    opts.compressLevel = getCompressionLevel();

    // Connection: Display
    opts.setScalingFactor(scalingFactor.getSelectedItem().toString());
    opts.setDesktopSize(desktopSize.getSelectedItem().toString());
    opts.fullScreen = fullScreen.isSelected();
    int index = span.getSelectedIndex();
    if (index >= 0 && index < Options.NUMSPANOPT)
      opts.span = index;
    opts.acceptBell = acceptBell.isSelected();
    opts.cursorShape = cursorShape.isSelected();
    opts.showToolbar = showToolbar.isSelected();

    // Connection: Input
    opts.viewOnly = viewOnly.isSelected();
    opts.reverseScroll = reverseScroll.isSelected();
    opts.fsAltEnter = fsAltEnter.isSelected();
    opts.menuKeyCode =
      MenuKey.getMenuKeySymbols()[menuKey.getSelectedIndex()].keycode;
    opts.menuKeySym =
      MenuKey.getMenuKeySymbols()[menuKey.getSelectedIndex()].keysym;
    if (Utils.osGrab() && Helper.isAvailable())
      opts.grabKeyboard = grabKeyboard.getSelectedIndex();

    // Connection: Restrictions
    opts.shared = shared.isSelected();
    opts.recvClipboard = recvClipboard.isSelected();
    opts.sendClipboard = sendClipboard.isSelected();

    // Security
    opts.user = (username.getText().isEmpty() ? null : username.getText());
    opts.sendLocalUsername = sendLocalUsername.isSelected();

    // Security: Security types
    opts.disableSecType(RFB.SECTYPE_NONE);
    opts.disableSecType(RFB.SECTYPE_VNCAUTH);
    opts.disableSecType(RFB.SECTYPE_PLAIN);
    opts.disableSecType(RFB.SECTYPE_IDENT);
    opts.disableSecType(RFB.SECTYPE_TLS_NONE);
    opts.disableSecType(RFB.SECTYPE_TLS_VNC);
    opts.disableSecType(RFB.SECTYPE_TLS_PLAIN);
    opts.disableSecType(RFB.SECTYPE_TLS_IDENT);
    opts.disableSecType(RFB.SECTYPE_X509_NONE);
    opts.disableSecType(RFB.SECTYPE_X509_VNC);
    opts.disableSecType(RFB.SECTYPE_X509_PLAIN);
    opts.disableSecType(RFB.SECTYPE_X509_IDENT);
    opts.disableSecType(RFB.SECTYPE_UNIX_LOGIN);

    if (secNone.isSelected())
      opts.enableSecType(RFB.SECTYPE_NONE);
    if (secVnc.isSelected())
      opts.enableSecType(RFB.SECTYPE_VNCAUTH);
    if (secPlain.isSelected())
      opts.enableSecType(RFB.SECTYPE_PLAIN);
    if (secIdent.isSelected())
      opts.enableSecType(RFB.SECTYPE_IDENT);
    if (secTLSNone.isSelected())
      opts.enableSecType(RFB.SECTYPE_TLS_NONE);
    if (secTLSVnc.isSelected())
      opts.enableSecType(RFB.SECTYPE_TLS_VNC);
    if (secTLSPlain.isSelected())
      opts.enableSecType(RFB.SECTYPE_TLS_PLAIN);
    if (secTLSIdent.isSelected())
      opts.enableSecType(RFB.SECTYPE_TLS_IDENT);
    if (secX509None.isSelected())
      opts.enableSecType(RFB.SECTYPE_X509_NONE);
    if (secX509Vnc.isSelected())
      opts.enableSecType(RFB.SECTYPE_X509_VNC);
    if (secX509Plain.isSelected())
      opts.enableSecType(RFB.SECTYPE_X509_PLAIN);
    if (secX509Ident.isSelected())
      opts.enableSecType(RFB.SECTYPE_X509_IDENT);
    if (secUnixLogin.isSelected())
      opts.enableSecType(RFB.SECTYPE_UNIX_LOGIN);

    // Security: X.509 certificate validation
    opts.x509ca = (x509ca.getText().isEmpty() ? null : x509ca.getText());
    opts.x509crl = (x509crl.getText().isEmpty() ? null : x509crl.getText());

    // Security: Gateway
    opts.sshUser = (sshUser.getText().isEmpty() ? null : sshUser.getText());
    opts.via = (gateway.getText().isEmpty() ? null : gateway.getText());
    opts.tunnel = tunnel.isSelected();
  }

}
