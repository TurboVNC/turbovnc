/* Copyright (C) 2012 Brian P. Hinz
 * Copyright (C) 2018 D. R. Commander.  All Rights Reserved.
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

package com.turbovnc.vncviewer;

import javax.swing.filechooser.FileSystemView;

import com.turbovnc.rfb.LogWriter;

public final class FileUtils {

  public static String getHomeDir() {
    String homeDir = null;
    try {
      if (VncViewer.OS.startsWith("windows")) {
        // JRE prior to 1.5 cannot reliably determine USERPROFILE.
        // Return user.home and hope it's right.
        if (VncViewer.JAVA_VERSION < 5) {
          try {
            homeDir = System.getProperty("user.home");
          } catch (java.security.AccessControlException e) {
            vlog.error("Cannot access user.home system property:");
            vlog.error("  " + e.getMessage());
          }
        } else {
          homeDir = System.getenv("USERPROFILE");
        }
      } else {
        try {
          homeDir = FileSystemView.getFileSystemView().
                    getDefaultDirectory().getCanonicalPath();
        } catch (java.security.AccessControlException e) {
          vlog.error("Cannot access system property:");
          vlog.error("  " + e.getMessage());
        }
      }
    } catch (java.lang.Exception e) {
      e.printStackTrace();
    }

    return homeDir + getFileSeparator();
  }

  public static String getVncHomeDir() {
    return getHomeDir() + ".vnc" + getFileSeparator();
  }

  public static String getFileSeparator() {
    String separator = null;
    try {
      separator = Character.toString(java.io.File.separatorChar);
    } catch (java.security.AccessControlException e) {
      vlog.error("Cannot access file.separator system property:");
      vlog.error("  " + e.getMessage());
    }
    return separator;
  }

  private FileUtils() {}
  static LogWriter vlog = new LogWriter("FileUtils");
}
