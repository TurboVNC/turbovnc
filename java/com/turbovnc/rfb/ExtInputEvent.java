/* Copyright (C) 2015 D. R. Commander.  All Rights Reserved.
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

public class ExtInputEvent {
  public int type;
  public long deviceID;
  public long buttonMask;
  public int buttonNumber;
  public int numValuators;
  public int firstValuator;
  public int[] valuators = new int[6];
  public int x, y;

  public void print() {
    vlog.eidebug("EVENT:");
    vlog.eidebug("  type = " + type);
    vlog.eidebug("  deviceID = " + deviceID);
    vlog.eidebug("  buttonMask = " + buttonMask);
    if (type == giiTypes.giiButtonPress || type == giiTypes.giiButtonRelease)
      vlog.eidebug("  buttonNumber = " + buttonNumber);
    else
      vlog.eidebug("  x, y = " + x + "," + y);
    vlog.eidebug("  firstValuator = " + firstValuator);
    for (int i = 0; i < numValuators; i++)
      vlog.eidebug("    Valuator " + (i + firstValuator) + " = " +
                   valuators[i]);
  }

  static LogWriter vlog = new LogWriter("ExtInputEvent");
};
