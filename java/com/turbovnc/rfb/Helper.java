/* Copyright (C) 2015, 2017-2018, 2020-2022 D. R. Commander.
 *                                          All Rights Reserved.
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

/*
 * Helper.java - TurboVNC Helper dispatcher
 */

package com.turbovnc.rfb;

public final class Helper {

  public static synchronized boolean isAvailable() {
    if (!triedInit) {
      try {
        System.loadLibrary("turbovnchelper");
        available = true;
      } catch (UnsatisfiedLinkError e) {
        vlog.info("WARNING: Could not find TurboVNC Helper JNI library.  If it is in a");
        vlog.info("  non-standard location, then add -Djava.library.path=<dir>");
        vlog.info("  to the Java command line to specify its location.");
        printMissingFeatures();
      } catch (Exception e) {
        vlog.info("WARNING: Could not initialize TurboVNC Helper JNI library:");
        vlog.info("  " + e.toString());
        printMissingFeatures();
      }
    }
    triedInit = true;
    return available;
  }

  private static synchronized void printMissingFeatures() {
    vlog.info("  The following features will be disabled:");
    vlog.info("  - Accelerated JPEG decompression");
    if (Utils.osGrab())
      vlog.info("  - Keyboard grabbing");
    if (Utils.osEID())
      vlog.info("  - Extended input device support");
    if (Utils.isX11()) {
      vlog.info("  - Multi-screen spanning");
    }
    if (Utils.isWindows())
      vlog.info("  - Pageant support");
    else
      vlog.info("  - ssh-agent support");
    if (Utils.isX11() || Utils.isMac())
      vlog.info("  - Server-side keyboard mapping");
  }

  public static synchronized void setAvailable(boolean avail) {
    available = avail;
  }

  static boolean triedInit, available;

  private Helper() {}
  static LogWriter vlog = new LogWriter("Helper");
}
