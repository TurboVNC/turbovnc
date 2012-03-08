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

/**
 * An interface used to obtain credentials for Unix login authentication.
 *
 * <p>
 * AuthPlugin provides a generic mechanism for obtaining authentication
 * information to be passed back to the VNC server.  A concrete implementation
 * of this interface may obtain those credentials from whatever source is most
 * appropriate to a particular site, such as a password file, LDAP, hardware
 * token, CAC, etc.
 * </p>
 *
 * @see @{link AuthFactory}
 */

public interface AuthPlugin {

  /**
   * Clear temporary, sensitive resources.
   *
   * This method should wipe any sensitive data internal to the class.  It is
   * called immediately after the client has attempted to authenticate.
   *
   */
  public void clear();

  /**
   * Dispose of all sensitive resources.
   * 
   * This method should release any persistent resources required by the
   * authentication plugin and wipe any sensitive data.  It is called before
   * the client closes the connection and exits.
   * 
   */
  public void dispose();

  /**
   * Get the user name for Unix login authentication.
   *
   * @return The user name for Unix login authentication.
   */
  public String getUser();

  /**
   * Get the password for Unix login authentication.
   *
   * An empty password is returned as a zero-length byte array.  For security,
   * the caller should wipe the returned array once authentication is complete.
   *
   * @return Byte array representing the password.
   */
  public char[] getPassword();

  /**
   * The identifier for this authentication plugin.
   *
   * @return Identification string for this plugin.
   */
  public String getTitle();
}
