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
}
