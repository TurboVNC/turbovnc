/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright 2004-2005 Cendio AB.
 * Copyright (C) 2012 D. R. Commander.  All Rights Reserved.
 * Copyright 2012 Brian P. Hinz
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

//
// Configuration - class for dealing with configuration parameters.
//

package com.turbovnc.rfb;

public class Configuration {

  // - Set named parameter to value
  public static boolean setParam(String name, String value) {
    VoidParameter param = getParam(name);
    if (param == null) return false;
    if (param instanceof BoolParameter && ((BoolParameter)param).reverse) {
      ((BoolParameter)param).reverse = false;
      ((BoolParameter)param).setParam(value, true);
      return true;
    }
    return param.setParam(value);
  }

  // - Set parameter to value (separated by "=")
  public static boolean setParam(String config) {
    boolean hyphen = false;
    if (config.charAt(0) == '-') {
      hyphen = true;
      if (config.charAt(1) == '-')
        config = config.substring(2); // allow gnu-style --<option>
      else
        config = config.substring(1);
    }
    int equal = config.indexOf('=');
    if (equal != -1) {
      return setParam(config.substring(0, equal), config.substring(equal+1));
    } else if (hyphen) {
      VoidParameter param = getParam(config);
      if (param == null) return false;
      if (param instanceof BoolParameter && ((BoolParameter)param).reverse) {  
        ((BoolParameter)param).reverse = false;
        ((BoolParameter)param).setParam(false);
        return true;
      }
      return param.setParam();
    }
    return false;
  }

  // - Get named parameter
  public static VoidParameter getParam(String name) {
    VoidParameter current = head;
    while (current != null) {
      if (name.equalsIgnoreCase(current.getName()))
        return current;
      if (current instanceof BoolParameter) {
        if (name.length() > 2 && name.substring(0, 2).equalsIgnoreCase("no")) {
          String name2 = name.substring(2);
          if (name2.equalsIgnoreCase(current.getName())) {
            ((BoolParameter)current).reverse = true;
            return current;
          }
        }
      }
      current = current.next;
    }
    return null;
  }

  public static void listParams(int width) {
    VoidParameter current = head;

    while (current != null) {
      String desc = current.getDescription().trim();
      System.err.print("--> " + current.getName() + "\n    ");
      if (current.getValues() != null)
        System.err.print("Values: " + current.getValues() + " ");
      if (current.getDefaultStr() != null)
        System.err.print("(default = " + current.getDefaultStr() + ")\n");
      System.err.print("\n   ");

      int column = 4;
      while (true) {
        int s = desc.indexOf(' ');
        while (desc.charAt(s + 1) == ' ') s++;
        int wordLen;
        if (s > -1) wordLen = s;
        else wordLen = desc.length();
  
        if (column + wordLen + 1 > width) {
          System.err.print("\n   ");
          column = 4;
        }
        System.err.format(" %" + wordLen + "s", desc.substring(0, wordLen));
        column += wordLen + 1;
        if (wordLen >= 1 && desc.charAt(wordLen - 1) == '\n') {
          System.err.print("\n   ");
          column = 4;
        }

        if (s == -1) break;
        desc = desc.substring(wordLen+1);
      }
      current = current.next;
      System.err.print("\n\n");
    }
  }

  public static void readAppletParams(java.applet.Applet applet) {
    VoidParameter current = head;
    while (current != null) {
      String str = applet.getParameter(current.getName());
      if (str != null)
        current.setParam(str);
      current = current.next;
    }
  }

  public static VoidParameter head;
  public static VoidParameter tail;
}
