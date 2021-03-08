/* Copyright (C) 2015, 2018, 2021 D. R. Commander.  All Rights Reserved.
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
  // CHECKSTYLE VisibilityModifier:OFF
  public int type;
  public long deviceID;
  public long buttonMask;
  public int buttonNumber;
  public int numValuators;
  public int firstValuator;
  public int[] valuators = new int[6];
  // CHECKSTYLE VisibilityModifier:ON

  public void print() {
    vlog.eidebug("EVENT:");
    vlog.eidebug("  type = " + RFB.giiEventName(type));
    vlog.eidebug("  deviceID = " + deviceID);
    vlog.eidebug("  buttonMask = " + buttonMask);
    if (type == RFB.GII_BUTTON_PRESS || type == RFB.GII_BUTTON_RELEASE)
      vlog.eidebug("  buttonNumber = " + buttonNumber);
    vlog.eidebug("  firstValuator = " + firstValuator);
    for (int i = 0; i < numValuators; i++)
      vlog.eidebug("    Valuator " + (i + firstValuator) + " = " +
                   valuators[i]);
  }

  static LogWriter vlog = new LogWriter("ExtInputEvent");
};
