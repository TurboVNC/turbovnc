//
//  Copyright (C) 2012 Secure Mission Solutions, Inc.  All Rights Reserved.
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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
//  USA.
//

import java.awt.Toolkit;
import java.awt.datatransfer.Clipboard;
import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.StringSelection;
import java.awt.datatransfer.Transferable;
import java.awt.datatransfer.UnsupportedFlavorException;
import java.io.IOException;

/**
 * Example authentication plugin that obtains the password from the clipboard
 */
public class AuthClipboard implements AuthPlugin {
  String title  = "Clipboard";
  String user;
  String passwd;
  
  /**
   *  Constructor.
   * 
   *  Squirrel away the default user name and password.  For this contrived
   *  example, there is no reason to remember the VncViewer object.
   * 
   */
  public AuthClipboard(final VncViewer viewer, String defUser,
                       String defPasswd) {
    this.user   = defUser;
    this.passwd = (defPasswd == null ? "" : defPasswd);
  }
 
  public void clear() {
    Clipboard cb =  Toolkit.getDefaultToolkit().getSystemClipboard();
    StringSelection ss = new StringSelection("");
    try {
      cb.setContents(ss, null);
    }
    catch (IllegalStateException e)  { /* ignore */ }
  }

  public void dispose() {
    clear();
  }

  /**
   * Return the password contained in the clipboard, or the
   * application-supplied default if the clipboard is empty or unavailable. 
   */
  public char[] getPassword() {
    String s = passwd;
    char[] apasswd;

    try {
      Clipboard cb =  Toolkit.getDefaultToolkit().getSystemClipboard();
      Transferable tr = cb.getContents(null);
      s = (String) tr.getTransferData(DataFlavor.stringFlavor);
    } 
    catch (NullPointerException e)       { /* ignore */ }
    catch (UnsupportedFlavorException e) { /* ignore */ }
    catch (IllegalStateException e)      { /* ignore */ }
    catch (IOException e)                { /* ignore */ }

    apasswd = s.toCharArray();
    clear();
    return(apasswd);
  }

  public String getTitle() { 
    return(title); 
  }

  /* Return the application-supplied default user name. */
  public String getUser() { 
    return(user); 
  }
}
