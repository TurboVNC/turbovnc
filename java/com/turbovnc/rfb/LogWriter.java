/* Copyright (C) 2015, 2018 D. R. Commander.  All Rights Reserved.
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

public class LogWriter {

  public LogWriter(String name_) {
    name = name_;
    level = globalLogLevel;
    next = logWriters;
    logWriters = this;
  }

  public void setLevel(int level_) { level = level_; }

  public void write(int level_, String str) {
    if (level_ <= level) {
      System.err.println(name + ": " + str);
    }
  }

  public void error(String str) { write(0, str); }
  public void status(String str) { write(10, str); }
  public void info(String str) { write(30, str); }
  public void debug(String str) { write(100, str); }
  public void sshdebug(String str) { write(110, str); }
  public void eidebug(String str) { write(150, str); }

  public static boolean setLogParams(String params) {
    globalLogLevel = Integer.parseInt(params);
    LogWriter current = logWriters;
    while (current != null) {
      current.setLevel(globalLogLevel);
      current = current.next;
    }
    return true;
  }

  static LogWriter getLogWriter(String name) {
    LogWriter current = logWriters;
    while (current != null) {
      if (name.equalsIgnoreCase(current.name)) return current;
      current = current.next;
    }
    return null;
  }

  String name;
  int level;
  LogWriter next;
  static LogWriter logWriters;
  static int globalLogLevel = 30;
}
