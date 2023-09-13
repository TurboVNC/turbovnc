/* Copyright (C) 2013, 2015, 2018, 2020-2023 D. R. Commander.
 *                                           All Rights Reserved.
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

public final class ServerNameParameter extends StringParameter {

  public ServerNameParameter(String name, Params params, boolean isGUI,
                             String desc, String defValue) {
    super(name, params, isGUI, desc, defValue);
    setDefault(defValue);
  }

  public synchronized boolean set(String str) {
    if (str != null && !str.isEmpty()) {
      str = str.replaceAll("\\s", "");
      int atIndex = str.lastIndexOf('@');
      if (atIndex >= 0) {
        params.sshUser.set(str.substring(0, atIndex));
        if (isCommandLine() || getName().equalsIgnoreCase("Server"))
          params.sshUser.setCommandLine(true);
        return super.set(str.substring(atIndex + 1));
      }
    }
    return super.set(str);
  }

  public synchronized boolean setDefault(String str) {
    if (str != null && !str.isEmpty()) {
      str = str.replaceAll("\\s", "");
      int atIndex = str.lastIndexOf('@');
      if (atIndex >= 0) {
        params.sshUser.setDefault(str.substring(0, atIndex));
        return super.setDefault(str.substring(atIndex + 1));
      }
    }
    return super.setDefault(str);
  }
}
