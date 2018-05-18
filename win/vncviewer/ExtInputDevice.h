// Copyright (C) 2015 D. R. Commander.  All Rights Reserved.
//
// This is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this software; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
// USA.

#ifndef EXTINPUTDEVICE_H__
#define EXTINPUTDEVICE_H__

#include "Log.h"


class ExtInputDevice
{
  public:

    ExtInputDevice(void)
    {
      name[0] = 0;
      id = 0;
      remoteID = 0;
      vendorID = 0;
      productID = 0;
      canGenerate = 0;
      numRegisters = 0;
      numButtons = 0;
      absolute = false;
    }

    char name[32];
    CARD32 id;
    CARD32 remoteID;
    CARD32 vendorID;
    CARD32 productID;
    CARD32 canGenerate;
    CARD32 numRegisters;
    CARD32 numButtons;
    boolean absolute;

    std::list<rfbGIIValuator> valuators;

    void addValuator(rfbGIIValuator &valuator)
    {
      valuators.push_back(valuator);
    }

    void print()
    {
      extern Log vnclog;

      vnclog.Print(100, "DEVICE:\n");
      vnclog.Print(100, "  name = %s\n", name);
      vnclog.Print(100, "  id = %d\n", id);
      vnclog.Print(100, "  vendorID = %d\n", vendorID);
      vnclog.Print(100, "  productID = %d\n", productID);
      vnclog.Print(100, "  canGenerate = 0x%.8x\n", canGenerate);
      vnclog.Print(100, "  numRegisters = %d\n", numRegisters);
      vnclog.Print(100, "  numValuators = %d\n", valuators.size());
      vnclog.Print(100, "  numButtons = %d\n", numButtons);
      vnclog.Print(100, "  absolute = %d\n", absolute);

      std::list<rfbGIIValuator>::const_iterator v;
      for (v = valuators.begin(); v != valuators.end(); ++v) {
        vnclog.Print(100, "  VALUATOR:\n");
        vnclog.Print(100, "    index = %d\n", v->index);
        vnclog.Print(100, "    longName = %s\n", v->longName);
        vnclog.Print(100, "    shortName = %s\n", v->shortName);
        vnclog.Print(100, "    rangeMin = %d\n", v->rangeMin);
        vnclog.Print(100, "    rangeCenter = %d\n", v->rangeCenter);
        vnclog.Print(100, "    rangeMax = %d\n", v->rangeMax);
        vnclog.Print(100, "    siUnit = %d\n", v->siUnit);
        vnclog.Print(100, "    siAdd = %d\n", v->siAdd);
        vnclog.Print(100, "    siMul = %d\n", v->siMul);
        vnclog.Print(100, "    siDiv = %d\n", v->siDiv);
        vnclog.Print(100, "    siShift = %d\n", v->siShift);
      }
    }
};

#endif  // EXTINPUTDEVICE_H__
