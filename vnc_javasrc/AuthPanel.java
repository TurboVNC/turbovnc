//
//  Copyright (C) 2010 D. R. Commander.  All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
//  Copyright (C) 2002-2006 Constantin Kaplinsky.  All Rights Reserved.
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

  TextField passwordField;
  TextField userField;
  Button okButton;

  //
  // Constructor.
  //

  public AuthPanel(VncViewer viewer, boolean unixLogin, String user)
  {
    Label titleLabel, userLabel = null;
    if (unixLogin)
      titleLabel  = new Label("Unix Login Authentication", Label.CENTER);
    else
      titleLabel  = new Label("Standard VNC Authentication", Label.CENTER);
    titleLabel.setFont(new Font("Helvetica", Font.BOLD, 18));

    if (unixLogin) {
      userLabel = new Label("User name:", Label.CENTER);

      if (user != null)
        userField = new TextField(user, 20);
      else
        userField = new TextField(20);
      userField.setForeground(Color.black);
      userField.setBackground(Color.white);
    }

    Label passwordLabel = new Label("Password:", Label.CENTER);

    passwordField = new TextField(20);
    passwordField.setForeground(Color.black);
    passwordField.setBackground(Color.white);
    passwordField.setEchoChar('*');

    okButton = new Button("OK");

    GridBagLayout gridbag = new GridBagLayout();
    GridBagConstraints gbc = new GridBagConstraints();

    setLayout(gridbag);

    gbc.gridwidth = GridBagConstraints.REMAINDER;
    gbc.insets = new Insets(0,0,20,0);
    gridbag.setConstraints(titleLabel,gbc);
    add(titleLabel);

    if (unixLogin) {
      gbc.fill = GridBagConstraints.NONE;
      gbc.gridwidth = 1;
      gbc.insets = new Insets(0,0,0,0);
      gridbag.setConstraints(userLabel,gbc);
      add(userLabel);

      gridbag.setConstraints(userField,gbc);
      add(userField);
    }

    gbc.fill = GridBagConstraints.NONE;
    gbc.gridwidth = 1;
    gbc.insets = new Insets(0,0,0,0);
    gridbag.setConstraints(passwordLabel,gbc);
    add(passwordLabel);

    gridbag.setConstraints(passwordField,gbc);
    add(passwordField);
    passwordField.addActionListener(this);

    // gbc.ipady = 10;
    gbc.gridwidth = GridBagConstraints.REMAINDER;
    gbc.fill = GridBagConstraints.BOTH;
    gbc.insets = new Insets(0,20,0,0);
    gbc.ipadx = 30;
    gridbag.setConstraints(okButton,gbc);
    add(okButton);
    okButton.addActionListener(this);
  }

  //
  // Move keyboard focus to the default object, that is, the password
  // text field.
  //

  public void moveFocusToDefaultField()
  {
    if (userField != null && (userField.getText() == null
      || userField.getText().length() < 1))
      userField.requestFocus();
    else
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
  // Wait for user entering a password, and return it as String.
  //

  public synchronized String getPassword() throws Exception
  {
    try {
      wait();
    } catch (InterruptedException e) { }
    return passwordField.getText();
  }

  public synchronized String getUsername() throws Exception
  {
    return userField.getText();
  }

}
