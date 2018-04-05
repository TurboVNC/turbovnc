/* Copyright (C) 2015, 2017 D. R. Commander.  All Rights Reserved.
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

public class giiTypes {
  // Message sub-types
  public static final int giiEvent = 0;
  public static final int giiVersion = 1;
  public static final int giiDeviceCreate = 2;
  public static final int giiDeviceDestroy = 3;

  public static final int giiBE = 128;

  // Event types
  public static final int giiButtonPress = 10;
  public static final int giiButtonRelease = 11;
  public static final int giiValuatorRelative = 12;
  public static final int giiValuatorAbsolute = 13;

  // Event masks
  public static final int giiButtonPressMask = 0x00000400;
  public static final int giiButtonReleaseMask = 0x00000800;
  public static final int giiValuatorRelativeMask = 0x00001000;
  public static final int giiValuatorAbsoluteMask = 0x00002000;

  // Unit types
  public static final int giiUnitUnknown = 0;
  public static final int giiUnitTime = 1;
  public static final int giiUnitFreq = 2;
  public static final int giiUnitLength = 3;
  public static final int giiUnitVelocity = 4;
  public static final int giiUnitAccel = 5;
  public static final int giiUnitAngle = 6;
  public static final int giiUnitAngularVelocity = 7;
  public static final int giiUnitAngularAccel = 8;
  public static final int giiUnitArea = 9;
  public static final int giiUnitVolume = 10;
  public static final int giiUnitMass = 11;
  public static final int giiUnitForce = 12;
  public static final int giiUnitPressure = 13;
  public static final int giiUnitTorque = 14;
  public static final int giiUnitEnergy = 15;
  public static final int giiUnitPower = 16;
  public static final int giiUnitTemp = 17;
  public static final int giiUnitCurrent = 18;
  public static final int giiUnitVoltage = 19;
  public static final int giiUnitResistance = 20;
  public static final int giiUnitCapacity = 21;
  public static final int giiUnitInductivity = 22;

  // Device types
  public static final int giiDevTypeNone = 0;
  public static final int giiDevTypeCursor = 1;
  public static final int giiDevTypeStylus = 2;
  public static final int giiDevTypeEraser = 3;
  public static final int giiDevTypeTouch = 4;
  public static final int giiDevTypePad = 5;
};
