/* Copyright (C) 2012, 2016, 2018, 2020, 2022-2025 D. R. Commander.
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

  // Find the position of the first colon in the display number/port/UDS
  // portion of the VNC server name.

  public static int getColonPos(String vncServerName) {
    if (vncServerName == null)
      return -1;

    int colonPos = vncServerName.lastIndexOf(':');
    int bracketPos = vncServerName.lastIndexOf(']');
    int atPos = vncServerName.lastIndexOf('@');

    // No colon = hostname only
    if (colonPos == -1 || colonPos < atPos)
      return -1;
    // Last colon is inside square brackets = IPv6 address only
    if (bracketPos != -1 && colonPos < bracketPos)
      return -1;
    // If the last colon is part of a series, then find the first colon in the
    // series.
    while (colonPos > 0 && vncServerName.charAt(colonPos - 1) == ':')
      colonPos--;
    if (colonPos == atPos + 1) {
      // IPv6 loopback address only (special case)
      if (vncServerName.regionMatches(atPos + 1, "::1", 0, 3))
        return -1;
      // Display number/port/UDS specified without host
      return colonPos;
    }
    // Display number/port/UDS specified with bracketed IPv6 address
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

    // Display number/port/UDS specified with hostname
    return colonPos;
  }

  public static String getSSHUser(String vncServerName) {
    if (vncServerName == null)
      return null;

    int atPos = vncServerName.lastIndexOf('@');

    if (atPos > 0)
      return vncServerName.substring(0, atPos);
    return null;
  }

  public static String getSSHHost(String vncServerName) {
    if (vncServerName == null)
      return null;

    int colonPos = getColonPos(vncServerName);
    int atPos = vncServerName.lastIndexOf('@');

    if (colonPos == atPos + 1 || atPos == vncServerName.length() - 1)
      return null;
    if (colonPos == -1 || colonPos < atPos)
      colonPos = vncServerName.length();
    return vncServerName.substring(atPos + 1, colonPos);
  }

  public static String getHost(String vncServerName) {
    if (vncServerName == null)
      return null;

    int colonPos = getColonPos(vncServerName);
    int atPos = vncServerName.lastIndexOf('@');

    if (colonPos == atPos + 1)
      return "localhost";
    if (colonPos == -1 || colonPos < atPos)
      colonPos = vncServerName.length();
    return vncServerName.substring(atPos + 1, colonPos);
  }

  public static int getSSHPort(String vncServerName) {
    int colonPos = getColonPos(vncServerName);

    if (colonPos >= 0 && colonPos != vncServerName.length() - 1) {
      try {
        int port = Integer.parseInt(vncServerName.substring(colonPos + 1));
        if (port >= 0 && port <= 65535)
          return port;
      } catch (NumberFormatException e) {
      }
    }

    return -1;
  }

  public static int getPort(String vncServerName) {
    int colonPos = getColonPos(vncServerName);

    if (colonPos == -1 || colonPos == vncServerName.length() - 1)
      return Utils.getBooleanProperty("turbovnc.sessmgr", true) ? 0 : 5900;

    String substring = vncServerName.substring(colonPos);
    if (substring.startsWith("::/") || substring.startsWith("::~/"))
      return -1;

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

  public static String getUDSPath(String vncServerName) {
    int colonPos = getColonPos(vncServerName);

    if (colonPos == -1 || colonPos == vncServerName.length() - 1)
      return null;

    String substring = vncServerName.substring(colonPos);
    if (!substring.startsWith("::/") && !substring.startsWith("::~/"))
      return null;

    return vncServerName.substring(colonPos + 2);
  }

  private Hostname() {}
}
