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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//

/**
 * An interface used to instantiate a plugin that provides an alternative
 * means of obtaining credentials for Unix login authentication
 */
public interface AuthFactory {

  /**
   * AuthPlugin factory constructor
   * 
   * @param viewer     The vncviewer instance
   * @param defUser    Default user name supplied with the USER parameter, or
   *                   <em>null</em> if unset
   * @param defPasswd  Default password supplied with the PASSWORD/ENCPASSWORD
   *                   parameter, or <em>null</em> if unset
   * 
   */
  public AuthPlugin createAuthPlugin(final VncViewer viewer,
                                     final String defUser,
                                     final String defPasswd);
}
