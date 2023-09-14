/* Copyright (C) 2012, 2016, 2018, 2020, 2022-2023 D. R. Commander.
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

  private static int getColonPos(String vncServerName) {
    int colonPos = vncServerName.lastIndexOf(':');
    int bracketPos = vncServerName.lastIndexOf(']');
    boolean doubleColon = false;

    if (bracketPos != -1 && colonPos < bracketPos)
      colonPos = -1;
    while (colonPos > 0 && vncServerName.charAt(colonPos - 1) == ':') {
      colonPos--;
      doubleColon = true;
    }
    if (doubleColon) {
      // Check for preceding single colon, indicating an IPv6 address
      for (int p = colonPos - 1; p >= 0; p--) {
        if (vncServerName.charAt(p) == ':') {
          if (p == 0 || vncServerName.charAt(p - 1) != ':')
            colonPos = -1;
          break;
        }
      }
    }

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
