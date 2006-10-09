//
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

import java.awt.*;
import java.awt.event.*;

//
// The panel which implements the user authentication scheme
//

class AuthPanel extends Panel implements ActionListener {

  String passwordParam;

  Label titleLabel, retryLabel, promptLabel;
  TextField passwordField;
  Button okButton;

  //
  // Constructor.
  //

  public AuthPanel(VncViewer viewer)
  {
    readParameters(viewer);
    if (!isInteractionNecessary())
      return;

    titleLabel = new Label("VNC Authentication",Label.CENTER);
    titleLabel.setFont(new Font("Helvetica", Font.BOLD, 18));

    promptLabel = new Label("Password:",Label.CENTER);

    passwordField = new TextField(10);
    passwordField.setForeground(Color.black);
    passwordField.setBackground(Color.white);
    passwordField.setEchoChar('*');

    okButton = new Button("OK");

    retryLabel = new Label("",Label.CENTER);
    retryLabel.setFont(new Font("Courier", Font.BOLD, 16));

    GridBagLayout gridbag = new GridBagLayout();
    GridBagConstraints gbc = new GridBagConstraints();

    setLayout(gridbag);

    gbc.gridwidth = GridBagConstraints.REMAINDER;
    gridbag.setConstraints(titleLabel,gbc);
    add(titleLabel);

    gbc.fill = GridBagConstraints.HORIZONTAL;
    gridbag.setConstraints(retryLabel,gbc);
    add(retryLabel);

    gbc.fill = GridBagConstraints.NONE;
    gbc.gridwidth = 1;
    gridbag.setConstraints(promptLabel,gbc);
    add(promptLabel);

    gridbag.setConstraints(passwordField,gbc);
    add(passwordField);
    passwordField.addActionListener(this);

    gbc.ipady = 10;
    gbc.gridwidth = GridBagConstraints.REMAINDER;
    gbc.fill = GridBagConstraints.BOTH;
    gbc.insets = new Insets(0,20,0,0);
    gbc.ipadx = 40;
    gridbag.setConstraints(okButton,gbc);
    add(okButton);
    okButton.addActionListener(this);
  }

  //
  // Read applet or command-line parameters. If an "ENCPASSWORD"
  // parameter is set, then decrypt the password into the
  // passwordParam string. Otherwise, try to read the "PASSWORD"
  // parameter directly to passwordParam.
  //

  private void readParameters(VncViewer viewer)
  {
    String encPasswordParam = viewer.readParameter("ENCPASSWORD", false);
    if (encPasswordParam == null) {
      passwordParam = viewer.readParameter("PASSWORD", false);
    } else {
      // ENCPASSWORD is hexascii-encoded. Decode.
      byte[] pw = {0, 0, 0, 0, 0, 0, 0, 0};
      int len = encPasswordParam.length() / 2;
      if (len > 8)
	len = 8;
      for (int i = 0; i < len; i++) {
	String hex = encPasswordParam.substring(i*2, i*2+2);
	Integer x = new Integer(Integer.parseInt(hex, 16));
	pw[i] = x.byteValue();
      }
      // Decrypt the password.
      byte[] key = {23, 82, 107, 6, 35, 78, 88, 7};
      DesCipher des = new DesCipher(key);
      des.decrypt(pw, 0, pw, 0);
      passwordParam = new String(pw);
    }
  }

  //
  // Check if we should show the GUI and ask user for authentication
  // data (password). If we already have a password, we don't need to
  // ask user. In that case, authentication failures would be fatal.
  //

  public boolean isInteractionNecessary()
  {
    return (passwordParam == null);
  }

  //
  // Move keyboard focus to the default object, that is, the password
  // text field.
  //
  // FIXME: here moveFocusToDefaultField() does not always work
  // under Netscape 4.7x/Java 1.1.5/Linux. It seems like this call
  // is being executed before the password field of the
  // authenticator is fully drawn and activated, therefore
  // requestFocus() does not work. Currently, I don't know how to
  // solve this problem.
  //   -- const
  //

  public void moveFocusToDefaultField()
  {
    passwordField.requestFocus();
  }

  //
  // This method is called when a button is pressed or return is
  // pressed in the password text field.
  //

  public synchronized void actionPerformed(ActionEvent evt)
  {
    if (evt.getSource() == passwordField || evt.getSource() == okButton) {
      passwordField.setEnabled(false);
      notify();
    }
  }

  //
  // Try to authenticate, either with a password read from parameters,
  // or with a password entered by the user.
  //

  public synchronized boolean tryAuthenticate(RfbProto rfb) throws Exception
  {
    String pw = passwordParam;
    if (pw == null) {
      try {
	// Wait for user entering a password.
	wait();
      } catch (InterruptedException e) { }
      pw = passwordField.getText();
    }

    byte[] challenge = new byte[16];
    rfb.readFully(challenge);

    if (pw.length() > 8)
      pw = pw.substring(0, 8);	// Truncate to 8 chars

    // vncEncryptBytes in the UNIX libvncauth truncates password
    // after the first zero byte. We do to.
    int firstZero = pw.indexOf(0);
    if (firstZero != -1)
      pw = pw.substring(0, firstZero);

    byte[] key = {0, 0, 0, 0, 0, 0, 0, 0};
    System.arraycopy(pw.getBytes(), 0, key, 0, pw.length());

    DesCipher des = new DesCipher(key);

    des.encrypt(challenge, 0, challenge, 0);
    des.encrypt(challenge, 8, challenge, 8);

    rfb.os.write(challenge);

    int authResult = rfb.is.readInt();

    switch (authResult) {
    case RfbProto.VncAuthOK:
      System.out.println("VNC authentication succeeded");
      return true;
    case RfbProto.VncAuthFailed:
      System.out.println("VNC authentication failed");
      break;
    case RfbProto.VncAuthTooMany:
      throw new Exception("VNC authentication failed - too many tries");
    default:
      throw new Exception("Unknown VNC authentication result " + authResult);
    }

    return false;
  }

  //
  // retry().
  //

  public void retry()
  {
    retryLabel.setText("Sorry. Try again.");
    passwordField.setEnabled(true);
    passwordField.setText("");
    moveFocusToDefaultField();
  }

}
