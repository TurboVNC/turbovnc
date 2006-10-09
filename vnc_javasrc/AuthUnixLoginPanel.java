//
//  Copyright (C) 2004 Constantin Kaplinsky.  All Rights Reserved.
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
// The panel which implements Unix login-stype authentication
//

class AuthUnixLoginPanel extends Panel implements ActionListener {

  Label titleLabel, retryLabel;
  Label passwordPromptLabel, usernamePromptLabel;
  TextField usernameField;
  TextField passwordField;
  Button okButton;

  //
  // Constructor.
  //

  public AuthUnixLoginPanel()
  {
    titleLabel = new Label("Unix Login Authentication", Label.CENTER);
    titleLabel.setFont(new Font("Helvetica", Font.BOLD, 18));

    usernamePromptLabel = new Label("Login:", Label.CENTER);
    passwordPromptLabel = new Label("Password:", Label.CENTER);

    usernameField = new TextField(10);
    usernameField.setForeground(Color.black);
    usernameField.setBackground(Color.white);

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
    gridbag.setConstraints(usernamePromptLabel, gbc);
    add(usernamePromptLabel);

    gbc.ipadx = 10;
    gridbag.setConstraints(usernameField, gbc);
    add(usernameField);
    usernameField.addActionListener(this);

    gbc.ipadx = 0;
    gridbag.setConstraints(passwordPromptLabel, gbc);
    add(passwordPromptLabel);

    gbc.ipadx = 10;
    gridbag.setConstraints(passwordField, gbc);
    add(passwordField);
    passwordField.addActionListener(this);

    gbc.ipady = 10;
    gbc.ipadx = 40;
    gbc.gridwidth = GridBagConstraints.REMAINDER;
    gbc.fill = GridBagConstraints.BOTH;
    gbc.insets = new Insets(0,20,0,0);
    gridbag.setConstraints(okButton,gbc);
    add(okButton);
    okButton.addActionListener(this);
  }

  //
  // Move keyboard focus to the default object. It is either the
  // username text field or the password text field.
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
    if (usernameField.getText().length() == 0) {
      usernameField.requestFocus();
    } else {
      passwordField.requestFocus();
    }
  }

  //
  // This method is called when a button is pressed or return is
  // pressed in a text field.
  //

  public synchronized void actionPerformed(ActionEvent evt)
  {
    if (evt.getSource() == usernameField) {
      passwordField.requestFocus();
    } else if (evt.getSource() == passwordField ||
	       evt.getSource() == okButton) {
      usernameField.setEnabled(false);
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
    try {
      // Wait for user entering a password.
      wait();
    } catch (InterruptedException e) { }

    String login = usernameField.getText();
    String passwd = passwordField.getText();

    rfb.writeInt(login.length());
    rfb.writeInt(passwd.length());
    rfb.os.write(login.getBytes());
    rfb.os.write(passwd.getBytes());

    int authResult = rfb.is.readInt();

    switch (authResult) {
    case RfbProto.VncAuthOK:
      System.out.println("Authentication succeeded");
      return true;
    case RfbProto.VncAuthFailed:
      System.out.println("Authentication failed");
      break;
    case RfbProto.VncAuthTooMany:
      throw new Exception("Authentication failed - too many tries");
    default:
      throw new Exception("Unknown authentication result " + authResult);
    }

    return false;
  }

  //
  // retry().
  //

  public void retry()
  {
    retryLabel.setText("Sorry. Try again.");
    usernameField.setEnabled(true);
    passwordField.setEnabled(true);
    passwordField.setText("");
    moveFocusToDefaultField();
  }

}
