/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright (C) 2011-2012 Brian P. Hinz
 * Copyright (C) 2012 D. R. Commander.  All Rights Reserved.
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
import java.util.Hashtable;
import javax.swing.*;
import javax.swing.event.ChangeListener;
import javax.swing.event.ChangeEvent;
import javax.swing.border.*;

import com.turbovnc.rfb.*;

class OptionsDialog extends Dialog implements ActionListener, ChangeListener,
  ItemListener {

  // Constants
  // Static variables
  static LogWriter vlog = new LogWriter("OptionsDialog");

  OptionsDialogCallback cb;
  JPanel encodingPanel, connPanel, globalPanel, secPanel;
  JCheckBox allowJpeg;
  JComboBox menuKey, compressLevel, scalingFactor, encMethodComboBox, span;
  JSlider jpegQualityLevel, subsamplingLevel, compressionLevel;
  JCheckBox viewOnly, acceptClipboard, sendClipboard, acceptBell;
  JCheckBox fullScreen, shared, cursorShape, showToolbar;
  JCheckBox secVeNCrypt, encNone, encTLS, encX509;
  JCheckBox secNone, secVnc, secUnixLogin, secPlain, secIdent,
    sendLocalUsername;
  JButton okButton, cancelButton;
  JButton ca, crl;
  JButton defSaveButton, defClearButton;
  JLabel encMethodLabel;
  JLabel jpegQualityLabel, jpegQualityLabelLo, jpegQualityLabelHi;
  JLabel subsamplingLabel, subsamplingLabelLo, subsamplingLabelHi;
  JLabel compressionLabel, compressionLabelLo, compressionLabelHi;
  String jpegQualityLabelString, subsamplingLabelString;
  String compressionLabelString;
  Hashtable<Integer, String> subsamplingLabelTable;

  public OptionsDialog(OptionsDialogCallback cb_) {
    super(true);
    cb = cb_;
    setResizable(false);
    setTitle("TurboVNC Viewer Options");

    getContentPane().setLayout(
      new BoxLayout(getContentPane(), BoxLayout.PAGE_AXIS));

    JTabbedPane tabPane = new JTabbedPane();

    // Encoding tab
    encodingPanel = new JPanel(new GridBagLayout());
    JPanel imagePanel = new JPanel(new GridBagLayout());

    encMethodLabel = new JLabel("Encoding method:");
    Object[] encMethod = {
      "Tight + Perceptually Lossless JPEG (LAN)",
      "Tight + Medium-Quality JPEG",
      "Tight + Low-Quality JPEG (WAN)",
      "Lossless Tight (Gigabit)",
      "Lossless Tight + Zlib (WAN)"};
    encMethodComboBox = new JComboBox(encMethod);
    encMethodComboBox.addActionListener(this);
    allowJpeg = new JCheckBox("Allow JPEG Compression");
    allowJpeg.addItemListener(this);

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

    compressionLabelString =
      new String("Zlib compression level: ");
    compressionLabel = new JLabel();
    compressionLevel =
      new JSlider(JSlider.HORIZONTAL, 0, 1, 1);
    compressionLevel.addChangeListener(this);
    compressionLevel.setMajorTickSpacing(1);
    compressionLevel.setSnapToTicks(true);
    compressionLevel.setPaintTicks(true);
    compressionLevel.setPaintLabels(false);
    compressionLabelLo = new JLabel("fast");
    compressionLabelHi = new JLabel("best");
    compressionLabel.setText(compressionLabelString +
      (compressionLevel.getValue() == 0 ? "None" :
        compressionLevel.getValue()));

    addGBComponent(encMethodLabel, imagePanel,
                   0, 0, 3, 1, 2, 2, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 2, 2, 2));
    addGBComponent(encMethodComboBox, imagePanel,
                   0, 1, 3, 1, 2, 2, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 20, 2, 2));
    addGBComponent(allowJpeg, imagePanel,
                   0, 2, 3, 1, 0, 2, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 0, 2, 2));
    addGBComponent(subsamplingLabel, imagePanel,
                   0, 3, 3, 1, 2, 2, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 2, 2, 2));
    addGBComponent(subsamplingLabelLo, imagePanel,
                   0, 4, 1, 1, 2, 2, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 12, 2, 2));
    addGBComponent(subsamplingLevel, imagePanel,
                   1, 4, 1, 1, 2, 2, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 2, 2, 2));
    addGBComponent(subsamplingLabelHi, imagePanel,
                   2, 4, 1, 1, 2, 2, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 2, 2, 2));
    addGBComponent(jpegQualityLabel, imagePanel,
                   0, 5, 3, 1, 2, 2, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 2, 2, 2));
    addGBComponent(jpegQualityLabelLo, imagePanel,
                   0, 6, 1, 1, 2, 2, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 12, 2, 2));
    addGBComponent(jpegQualityLevel, imagePanel,
                   1, 6, 1, 1, 2, 2, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 2, 2, 2));
    addGBComponent(jpegQualityLabelHi, imagePanel,
                   2, 6, 1, 1, 2, 2, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 2, 2, 2));
    addGBComponent(compressionLabel, imagePanel,
                   0, 7, 3, 1, 2, 2, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 2, 2, 2));
    addGBComponent(compressionLabelLo, imagePanel,
                   0, 8, 1, 1, 2, 2, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 12, 2, 2));
    addGBComponent(compressionLevel, imagePanel,
                   1, 8, 1, 1, 2, 2, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 2, 2, 2));
    addGBComponent(compressionLabelHi, imagePanel,
                   2, 8, 1, 1, 2, 2, 1, 1,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 2, 2, 2));

    addGBComponent(imagePanel, encodingPanel,
                   0, 0, 1, GridBagConstraints.REMAINDER, 0, 0, 1, 1,
                   GridBagConstraints.HORIZONTAL,
                   GridBagConstraints.PAGE_START,
                   new Insets(4, 4, 4, 4));

    // Connection tab
    connPanel = new JPanel(new GridBagLayout());

    JLabel scalingFactorLabel = new JLabel("Scaling Factor:");
    Object[] scalingFactors = {
      "Auto", "Fixed Aspect Ratio", "50%", "75%", "95%", "100%", "105%",
      "125%", "150%", "175%", "200%", "250%", "300%", "350%", "400%" };
    scalingFactor = new JComboBox(scalingFactors);
    scalingFactor.setEditable(true);
    scalingFactor.addItemListener(this);

    fullScreen = new JCheckBox("Full-screen mode");
    fullScreen.addItemListener(this);

    Object[] spanOptions = {
      "Primary monitor only", "All monitors", "Automatic" };
    JLabel spanLabel = new JLabel("Span mode:");
    span = new JComboBox(spanOptions);
    span.addItemListener(this);

    shared = new JCheckBox("Request shared session");
    shared.addItemListener(this);
    viewOnly = new JCheckBox("View Only (ignore mouse & keyboard)");
    viewOnly.addItemListener(this);
    cursorShape = new JCheckBox("Render cursor locally (enable remote cursor shape updates)");
    cursorShape.addItemListener(this);
    acceptClipboard = new JCheckBox("Accept clipboard from server");
    acceptClipboard.addItemListener(this);
    sendClipboard = new JCheckBox("Send clipboard to server");
    sendClipboard.addItemListener(this);
    acceptBell = new JCheckBox("Beep when requested by the server");
    acceptBell.addItemListener(this);

    addGBComponent(scalingFactorLabel, connPanel,
                   0, 0, 1, 1, 2, 2, 1, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.FIRST_LINE_START,
                   new Insets(8, 8, 0, 5));
    addGBComponent(scalingFactor, connPanel,
                   1, 0, 1, 1, 2, 2, 25, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.FIRST_LINE_START,
                   new Insets(4, 5, 0, 5));
    addGBComponent(fullScreen, connPanel,
                   0, 1, 2, 1, 2, 2, 1, 0,
                   GridBagConstraints.HORIZONTAL,
                   GridBagConstraints.FIRST_LINE_START,
                   new Insets(4, 5, 0, 5));
    addGBComponent(spanLabel, connPanel,
                   0, 2, 1, 1, 2, 2, 1, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.FIRST_LINE_START,
                   new Insets(8, 8, 0, 5));
    addGBComponent(span, connPanel,
                   1, 2, 1, 1, 2, 2, 25, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.FIRST_LINE_START,
                   new Insets(4, 5, 0, 5));
    addGBComponent(shared, connPanel,
                   0, 3, 2, 1, 2, 2, 1, 0,
                   GridBagConstraints.HORIZONTAL,
                   GridBagConstraints.FIRST_LINE_START,
                   new Insets(4, 5, 0, 5));
    addGBComponent(viewOnly, connPanel,
                   0, 4, 2, 1, 2, 2, 1, 0,
                   GridBagConstraints.HORIZONTAL,
                   GridBagConstraints.LINE_START,
                   new Insets(4, 5, 0, 5));
    addGBComponent(cursorShape, connPanel,
                   0, 5, 2, 1, 2, 2, 1, 0,
                   GridBagConstraints.HORIZONTAL,
                   GridBagConstraints.FIRST_LINE_START,
                   new Insets(4, 5, 0, 5));
    addGBComponent(acceptClipboard, connPanel,
                   0, 6, 2, 1, 2, 2, 1, 0,
                   GridBagConstraints.HORIZONTAL,
                   GridBagConstraints.LINE_START,
                   new Insets(4, 5, 0, 5));
    addGBComponent(sendClipboard, connPanel,
                   0, 7, 2, 1, 2, 2, 1, 0,
                   GridBagConstraints.HORIZONTAL,
                   GridBagConstraints.LINE_START,
                   new Insets(4, 5, 0, 5));
    addGBComponent(acceptBell, connPanel,
                   0, 8, 2, 1, 2, 2, 1, 1,
                   GridBagConstraints.HORIZONTAL,
                   GridBagConstraints.FIRST_LINE_START,
                   new Insets(4, 5, 0, 5));

    // Global tab
    globalPanel = new JPanel(new GridBagLayout());

    showToolbar = new JCheckBox("Show toolbar by default");
    showToolbar.addItemListener(this);

    JLabel menuKeyLabel = new JLabel("Menu Key:");
    String[] menuKeys = new String[MenuKey.getMenuKeySymbolCount()];
    for (int i = 0; i < MenuKey.getMenuKeySymbolCount(); i++)
      menuKeys[i] = MenuKey.getMenuKeySymbols()[i].name;
    menuKey  = new JComboBox(menuKeys);
    menuKey.addItemListener(this);

    addGBComponent(showToolbar, globalPanel,
                   0, 1, 2, 1, 2, 2, 1, 0,
                   GridBagConstraints.HORIZONTAL,
                   GridBagConstraints.FIRST_LINE_START,
                   new Insets(4, 5, 0, 5));
    addGBComponent(menuKeyLabel, globalPanel,
                   0, 2, 1, 1, 2, 2, 1, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.FIRST_LINE_START,
                   new Insets(8, 8, 0, 5));
    addGBComponent(menuKey, globalPanel,
                   1, 2, 1, 1, 2, 2, 25, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.FIRST_LINE_START,
                   new Insets(4, 5, 0, 5));

    JPanel defaultsPanel = new JPanel(new GridBagLayout());
    defaultsPanel.setBorder(BorderFactory.createTitledBorder("Defaults"));
    defSaveButton = new JButton("Save");
    defSaveButton.addActionListener(this);
    addGBComponent(defSaveButton, defaultsPanel,
                   0, 0, 1, 1, 2, 2, 1, 0,
                   GridBagConstraints.HORIZONTAL,
                   GridBagConstraints.FIRST_LINE_START,
                   new Insets(2, 2, 2, 2));
    defClearButton = new JButton("Clear");
    defClearButton.addActionListener(this);
    addGBComponent(defClearButton, defaultsPanel,
                   1, 0, 1, 1, 2, 2, 1, 0,
                   GridBagConstraints.HORIZONTAL,
                   GridBagConstraints.FIRST_LINE_START,
                   new Insets(2, 2, 2, 2));

    addGBComponent(defaultsPanel, globalPanel,
                   0, 3, 2, GridBagConstraints.REMAINDER, 2, 2, 1, 1,
                   GridBagConstraints.NONE,
                   GridBagConstraints.FIRST_LINE_START,
                   new Insets(25, 5, 4, 5));

    // security tab
    secPanel = new JPanel(new GridBagLayout());

    JPanel encryptionPanel = new JPanel(new GridBagLayout());
    encryptionPanel.setBorder(BorderFactory.createTitledBorder("Session Encryption"));
    encNone = addCheckbox("None", null, encryptionPanel);
    encTLS = addCheckbox("Anonymous TLS", null, encryptionPanel);
    encX509 = addJCheckBox("TLS with X.509 certificates", null,
                           encryptionPanel,
                           new GridBagConstraints(0, 2, 1, 1, 1, 1,
                             GridBagConstraints.LINE_START,
                             GridBagConstraints.REMAINDER,
                             new Insets(0, 0, 0, 60), 0, 0));

    JPanel x509Panel = new JPanel(new GridBagLayout());
    x509Panel.setBorder(BorderFactory.createTitledBorder("X.509 certificates"));
    ca = new JButton("Load CA certificate");
    ca.addActionListener(this);
    crl = new JButton("Load CRL certificate");
    crl.addActionListener(this);
    addGBComponent(ca, x509Panel,
                   0, 0, 1, 1, 2, 2, 1, 0,
                   GridBagConstraints.HORIZONTAL,
                   GridBagConstraints.FIRST_LINE_START,
                   new Insets(2, 2, 2, 2));
    addGBComponent(crl, x509Panel,
                   1, 0, 1, 1, 2, 2, 1, 0,
                   GridBagConstraints.HORIZONTAL,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 2, 2, 2));

    JPanel authPanel = new JPanel(new GridBagLayout());
    authPanel.setBorder(BorderFactory.createTitledBorder("Authentication Schemes"));
    secNone = addCheckbox("None", null, authPanel);
    secVnc = addCheckbox("Standard VNC", null, authPanel);
    secUnixLogin = addJCheckBox("Unix login (TightVNC/TurboVNC)", null,
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
    sendLocalUsername = new JCheckBox("Send Local Username");
    sendLocalUsername.addItemListener(this);
    addGBComponent(sendLocalUsername, authPanel,
                   1, 3, 1, 2, 0, 0, 2, 1,
                   GridBagConstraints.HORIZONTAL,
                   GridBagConstraints.LINE_START,
                   new Insets(0, 20, 0, 0));

    secVeNCrypt = new JCheckBox("Extended encryption and authentication (VeNCrypt)");
    secVeNCrypt.addItemListener(this);
    addGBComponent(secVeNCrypt, secPanel,
                   0, 0, 1, 1, 2, 2, 1, 0,
                   GridBagConstraints.HORIZONTAL,
                   GridBagConstraints.FIRST_LINE_START,
                   new Insets(4, 5, 0, 30));
    addGBComponent(encryptionPanel, secPanel,
                   0, 1, 1, 1, 2, 2, 1, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(0, 10, 2, 5));
    addGBComponent(x509Panel, secPanel,
                   0, 2, 1, 1, 2, 2, 1, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 10, 2, 5));
    addGBComponent(authPanel, secPanel,
                   0, 3, 1, 1, 2, 2, 1, 1,
                   GridBagConstraints.NONE,
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

    JPanel buttonPane = new JPanel();
    buttonPane.setLayout(new BoxLayout(buttonPane, BoxLayout.LINE_AXIS));
    buttonPane.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
    buttonPane.add(Box.createHorizontalGlue());
    buttonPane.add(okButton);
    buttonPane.add(Box.createRigidArea(new Dimension(5, 0)));
    buttonPane.add(cancelButton);
    buttonPane.add(Box.createRigidArea(new Dimension(5, 0)));

    getContentPane().add(tabPane);
    getContentPane().add(buttonPane);
    encMethodComboBox.setSelectedItem("Tight + Perceptually Lossless JPEG (LAN)");

    pack();

    addWindowListener(new WindowAdapter() {
      public void windowActivated(WindowEvent e) {
        if (cb != null) cb.setTightOptions();
      }
    });
  }

  public void initDialog() {
    if (cb != null) cb.setOptions();
  }

  private void updatePreferences() {
    UserPreferences.set("global", "JPEG", allowJpeg.isSelected());
    String subsamplingStr =
      subsamplingLabelTable.get(subsamplingLevel.getValue());
    UserPreferences.set("global", "Subsampling", subsamplingStr);
    UserPreferences.set("global", "Quality", jpegQualityLevel.getValue());
    UserPreferences.set("global", "CompressLevel",
                        compressionLevel.getValue());
    UserPreferences.set("global", "ViewOnly", viewOnly.isSelected());
    UserPreferences.set("global", "RecvClipboard",
                        acceptClipboard.isSelected());
    UserPreferences.set("global", "SendClipboard", sendClipboard.isSelected());
    String menuKeyStr =
      MenuKey.getMenuKeySymbols()[menuKey.getSelectedIndex()].name;
    UserPreferences.set("global", "MenuKey", menuKeyStr);
    UserPreferences.set("global", "FullScreen", fullScreen.isSelected());
    UserPreferences.set("global", "Shared", shared.isSelected());
    UserPreferences.set("global", "CursorShape", cursorShape.isSelected());
    UserPreferences.set("global", "AcceptBell", acceptBell.isSelected());
    UserPreferences.set("global", "Toolbar", showToolbar.isSelected());
    String scaleString = scalingFactor.getSelectedItem().toString();
    if (scaleString.equalsIgnoreCase("Auto")) {
      UserPreferences.set("global", "Scale", "Auto");
    } else if (scaleString.equalsIgnoreCase("Fixed Aspect Ratio")) {
      UserPreferences.set("global", "Scale", "FixedRatio");
    } else {
      scaleString = scaleString.replaceAll("[^\\d]", "");
      int sf = -1;
      try {
        sf = Integer.parseInt(scaleString);
      } catch(NumberFormatException e) {};
      if (sf >= 1 && sf <= 1000)
        UserPreferences.set("global", "Scale", Integer.toString(sf));
    }
    UserPreferences.set("global", "secVeNCrypt", secVeNCrypt.isSelected());
    UserPreferences.set("global", "encNone", encNone.isSelected());
    UserPreferences.set("global", "encTLS", encTLS.isSelected());
    UserPreferences.set("global", "encX509", encX509.isSelected());
    UserPreferences.set("global", "secNone", secNone.isSelected());
    UserPreferences.set("global", "secVnc", secVnc.isSelected());
    UserPreferences.set("global", "NoUnixLogin", !secUnixLogin.isSelected());
    UserPreferences.set("global", "secPlain", secPlain.isSelected());
    UserPreferences.set("global", "secIdent", secIdent.isSelected());
    UserPreferences.set("global", "SendLocalUsername",
                        sendLocalUsername.isSelected());
    if (!CSecurityTLS.x509ca.isDefault)
      UserPreferences.set("global", "x509ca",
                          CSecurityTLS.x509ca.getValue());
    if (!CSecurityTLS.x509crl.isDefault)
      UserPreferences.set("global", "x509crl",
                          CSecurityTLS.x509crl.getValue());

    // Options with no GUI equivalent
    UserPreferences.set("global", "AlwaysShowConnectionDialog",
                        VncViewer.alwaysShowConnectionDialog.getValue());
    UserPreferences.set("global", "FSAltEnter",
                        VncViewer.fsAltEnter.getValue());
    if (VncViewer.preferredEncoding.getValue() != null)
      UserPreferences.set("global", "Encoding",
                          VncViewer.preferredEncoding.getValue());
    if (VncViewer.colors.getValue() != -1)
      UserPreferences.set("global", "Colors", VncViewer.colors.getValue());
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
      if (cb != null) cb.getOptions();
      endDialog();
    } else if (s instanceof JButton && (JButton)s == cancelButton) {
      endDialog();
    } else if (s instanceof JButton && (JButton)s == defSaveButton) {
      updatePreferences();
      UserPreferences.save();
    } else if (s instanceof JButton && (JButton)s == defClearButton) {
      UserPreferences.clear();
    } else if (s instanceof JButton && (JButton)s == ca) {
      File file = new File(CSecurityTLS.x509ca.getValue());
      JFileChooser fc = new JFileChooser(file.getParent());
      fc.setSelectedFile(file);
      fc.setDialogTitle("Path to X509 CA certificate");
      fc.setApproveButtonText("OK");
      fc.setFileHidingEnabled(false);
      int ret = fc.showOpenDialog(this);
      if (ret == JFileChooser.APPROVE_OPTION)
        CSecurityTLS.x509ca.setParam(fc.getSelectedFile().toString());
    } else if (s instanceof JButton && (JButton)s == crl) {
      File file = new File(CSecurityTLS.x509crl.getValue());
      JFileChooser fc = new JFileChooser(file.getParent());
      fc.setSelectedFile(file);
      fc.setDialogTitle("Path to X509 CRL file");
      fc.setApproveButtonText("OK");
      fc.setFileHidingEnabled(false);
      int ret = fc.showOpenDialog(this);
      if (ret == JFileChooser.APPROVE_OPTION)
        CSecurityTLS.x509crl.setParam(fc.getSelectedFile().toString());
    } else if (s instanceof JComboBox && (JComboBox)s == encMethodComboBox) {
      JComboBox cb = (JComboBox)e.getSource();
      String encMethod = (String)cb.getSelectedItem();
      if (!encMethodComboBox.isEnabled()) return;
      if (encMethod.equals("Tight + Perceptually Lossless JPEG (LAN)")) {
        allowJpeg.setSelected(true);
        subsamplingLevel.setValue(0);
        jpegQualityLevel.setValue(95);
        compressionLevel.setValue(1);
      } else if (encMethod.equals("Tight + Medium-Quality JPEG")) {
        allowJpeg.setSelected(true);
        subsamplingLevel.setValue(1);
        jpegQualityLevel.setValue(80);
        compressionLevel.setValue(1);
      } else if (encMethod.equals("Tight + Low-Quality JPEG (WAN)")) {
        allowJpeg.setSelected(true);
        subsamplingLevel.setValue(2);
        jpegQualityLevel.setValue(30);
        compressionLevel.setValue(1);
      } else if (encMethod.equals("Lossless Tight (Gigabit)")) {
        allowJpeg.setSelected(false);
        compressionLevel.setValue(0);
      } else if (encMethod.equals("Lossless Tight + Zlib (WAN)")) {
        allowJpeg.setSelected(false);
        compressionLevel.setValue(1);
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
        if (compressionLevel.getMaximum() == 1) {
          compressionLevel.setEnabled(false);
          compressionLabel.setEnabled(false);
          compressionLabelLo.setEnabled(false);
          compressionLabelHi.setEnabled(false);
          compressionLevel.setValue(1);
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
        compressionLevel.setEnabled(true);
        compressionLabel.setEnabled(true);
        compressionLabelLo.setEnabled(true);
        compressionLabelHi.setEnabled(true);
      }
      setEncMethodComboBox();
    }
    if (s instanceof JCheckBox && (JCheckBox)s == secVeNCrypt) {
      encNone.setEnabled(secVeNCrypt.isSelected());
      encTLS.setEnabled(secVeNCrypt.isSelected());
      encX509.setEnabled(secVeNCrypt.isSelected());
      ca.setEnabled(secVeNCrypt.isSelected());
      crl.setEnabled(secVeNCrypt.isSelected());
      secIdent.setEnabled(secVeNCrypt.isSelected());
      secPlain.setEnabled(secVeNCrypt.isSelected());
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
  }

  private void setEncMethodComboBox() {
    if (!encMethodComboBox.isEnabled()) return;
    if (subsamplingLevel.getValue() == 0 &&
        compressionLevel.getValue() == 1 &&
        jpegQualityLevel.getValue() == 95 && allowJpeg.isSelected()) {
        encMethodComboBox.setSelectedItem("Tight + Perceptually Lossless JPEG (LAN)");
        if (encMethodComboBox.getItemCount() > 5)
          encMethodComboBox.removeItem("Custom");
    } else if (subsamplingLevel.getValue() == 1 &&
        compressionLevel.getValue() == 1 &&
        jpegQualityLevel.getValue() == 80 && allowJpeg.isSelected()) {
        encMethodComboBox.setSelectedItem("Tight + Medium-Quality JPEG");
        if (encMethodComboBox.getItemCount() > 5)
          encMethodComboBox.removeItem("Custom");
    } else if (subsamplingLevel.getValue() == 2 &&
        compressionLevel.getValue() == 1 &&
        jpegQualityLevel.getValue() == 30 && allowJpeg.isSelected()) {
        encMethodComboBox.setSelectedItem("Tight + Low-Quality JPEG (WAN)");
        if (encMethodComboBox.getItemCount() > 5)
          encMethodComboBox.removeItem("Custom");
    } else if (!allowJpeg.isSelected()) {
      switch (compressionLevel.getValue()) {
      case 0:
        encMethodComboBox.setSelectedItem("Lossless Tight (Gigabit)");
        if (encMethodComboBox.getItemCount() > 5)
          encMethodComboBox.removeItem("Custom");
        break;
      case 1:
        encMethodComboBox.setSelectedItem("Lossless Tight + Zlib (WAN)");
        if (encMethodComboBox.getItemCount() > 5)
          encMethodComboBox.removeItem("Custom");
        break;
      default:
        if (encMethodComboBox.getItemCount() <= 5)
          encMethodComboBox.addItem("Custom");
        encMethodComboBox.setSelectedItem("Custom");
      }
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
      int value = compressionLevel.getValue();
      if (compressionLevel.getMaximum() == 1 && value == 0)
        compressionLabel.setText(compressionLabelString + "None");
      else
        compressionLabel.setText(compressionLabelString + value);
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

}
