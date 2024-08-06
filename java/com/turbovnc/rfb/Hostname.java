/* Copyright (C) 2012, 2016, 2018, 2020, 2023-2024 D. R. Commander.
 *                                                 All Rights Reserved.
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

package com.turbovnc.rfb;

import com.turbovnc.rdr.*;

public final class Hostname {

  // Find the position of the first colon in the display number/port portion of
  // the VNC server name.

  public static int getColonPos(String vncServerName) {
    int colonPos = vncServerName.lastIndexOf(':');
    int bracketPos = vncServerName.lastIndexOf(']');

    // No colon = hostname only
    if (colonPos == -1)
      return -1;
    // Last colon is inside square brackets = IPv6 address only
    if (bracketPos != -1 && colonPos < bracketPos)
      return -1;
    // If the last colon is part of a series, then find the first colon in the
    // series.
    while (colonPos > 0 && vncServerName.charAt(colonPos - 1) == ':')
      colonPos--;
    if (colonPos == 0) {
      // IPv6 loopback address only (special case)
      if (vncServerName.equals("::1"))
        return -1;
      // Display number/port specified without host
      return colonPos;
    }
    // Display number/port specified with bracketed IPv6 address
    if (bracketPos != -1 && colonPos - 1 == bracketPos)
      return colonPos;

    // Check for preceding colons, indicating an IPv6 address.
    int colonCount = 0, p = colonPos;
    while ((p = vncServerName.lastIndexOf(':', p - 1)) > 0) {
      // Double colon = abbreviated IPv6 address
      if (vncServerName.charAt(p - 1) == ':')
        return colonPos;
      colonCount++;
      // 7 colons = full IPv6 address
      if (colonCount >= 7)
        return colonPos;
    }
    // Invalid format (IPv6 address is incomplete or hostname contains colons)
    if (colonCount > 0)
      return -1;

    // Display number/port specified with hostname
    return colonPos;
  }

  public static String getHost(String vncServerName) {
    int colonPos = getColonPos(vncServerName);

    if (colonPos == 0)
      return "localhost";
    if (colonPos == -1)
      colonPos = vncServerName.length();
    return vncServerName.substring(0, colonPos).replaceAll("\\s", "");
  }

  public static int getPort(String vncServerName) {
    int colonPos = getColonPos(vncServerName);

    if (colonPos == -1 || colonPos == vncServerName.length() - 1)
      return Utils.getBooleanProperty("turbovnc.sessmgr", true) ? 0 : 5900;
    if (vncServerName.charAt(colonPos + 1) == ':') {
      try {
        return Integer.parseInt(vncServerName.substring(colonPos + 2));
      } catch (NumberFormatException e) {
        throw new ErrorException("Invalid VNC server specified.");
      }
    }
    try {
      int port = Integer.parseInt(vncServerName.substring(colonPos + 1));
      if (port < 100)
        port += 5900;
      return port;
    } catch (NumberFormatException e) {
      throw new ErrorException("Invalid VNC server specified.");
    }
  }

  private Hostname() {}
}
