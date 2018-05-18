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

#ifndef EXTINPUTEVENT_H__
#define EXTINPUTEVENT_H__

#include "Log.h"


class ExtInputEvent
{
  public:

    ExtInputEvent(void)
    {
      type = 0;
      deviceID = 0;
      buttonMask = 0;
      buttonNumber = 0;
      numValuators = 0;
      firstValuator = 0;
      memset(valuators, 0, sizeof(int) * 6);
    }

    int type;
    CARD32 deviceID;
    CARD32 buttonMask;
    CARD32 buttonNumber;
    int numValuators;
    int firstValuator;
    int valuators[6];

    void print(void)
    {
      extern Log vnclog;

      vnclog.Print(100, "EVENT:\n");
      vnclog.Print(100, "  type = %d\n", type);
      vnclog.Print(100, "  deviceID = %d\n", deviceID);
      vnclog.Print(100, "  buttonMask = %d\n", buttonMask);
      if (type == rfbGIIButtonPress || type == rfbGIIButtonRelease)
        vnclog.Print(100, "  buttonNumber = %d\n", buttonNumber);
      vnclog.Print(100, "  firstValuator = %d\n", firstValuator);
      for (int i = 0; i < numValuators; i++)
        vnclog.Print(100, "    Valuator %d = %d\n", i + firstValuator,
                     valuators[i]);
    }
};

#endif  // EXTINPUTEVENT_H__
