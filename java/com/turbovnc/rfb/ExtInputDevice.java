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

import java.util.ArrayList;
import java.util.Iterator;

public class ExtInputDevice {

  // CHECKSTYLE VisibilityModifier:OFF
  public String name;
  public long id;
  public int remoteID;
  public long vendorID;
  public long productID;
  public long canGenerate;
  public int numRegisters;
  public int numButtons;
  public boolean absolute;

  public class Valuator {

    public int index;
    public String longName;
    public String shortName;
    public int rangeMin;
    public int rangeCenter;
    public int rangeMax;
    public int siUnit;
    public int siAdd;
    public int siMul = 1;
    public int siDiv = 1;
    public int siShift;

  };

  public void addValuator(Valuator valuator) {
    if (valuators == null)
      valuators = new ArrayList<Valuator>();
    valuators.add(valuator);
  }

  public void print() {
    vlog.eidebug("DEVICE:");
    vlog.eidebug("  name = " + name);
    vlog.eidebug("  id = " + id);
    vlog.eidebug("  vendorID = " + vendorID);
    vlog.eidebug("  productID = " + productID);
    vlog.eidebug("  canGenerate = " + String.format("0x%08X", canGenerate));
    vlog.eidebug("  numRegisters = " + numRegisters);
    vlog.eidebug("  numValuators = " + valuators.size());
    vlog.eidebug("  numButtons = " + numButtons);
    vlog.eidebug("  absolute = " + absolute);
    if (valuators != null) {
      for (Iterator<Valuator> i = valuators.iterator(); i.hasNext();) {
        Valuator v = (Valuator)i.next();
        vlog.eidebug("  VALUATOR:");
        vlog.eidebug("    index = " + v.index);
        vlog.eidebug("    longName = " + v.longName);
        vlog.eidebug("    shortName = " + v.shortName);
        vlog.eidebug("    rangeMin = " + v.rangeMin);
        vlog.eidebug("    rangeCenter = " + v.rangeCenter);
        vlog.eidebug("    rangeMax = " + v.rangeMax);
        vlog.eidebug("    siUnit = " + v.siUnit);
        vlog.eidebug("    siAdd = " + v.siAdd);
        vlog.eidebug("    siMul = " + v.siMul);
        vlog.eidebug("    siDiv = " + v.siDiv);
        vlog.eidebug("    siShift = " + v.siShift);
      }
    }
  }

  public ArrayList<Valuator> valuators;
  // CHECKSTYLE VisibilityModifier:ON

  static LogWriter vlog = new LogWriter("ExtInputDevice");
};
