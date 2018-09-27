//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
//  Copyright (C) 2016 D. R. Commander. All Rights Reserved.
//
//  This file is part of the VNC system.
//
//  The VNC system is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
//  USA.

// mapping of windows virtual key codes to X keysyms.

#ifndef KEYMAP_H__
#define KEYMAP_H__

#pragma once

#include "keysym.h"
#include "rfb.h"


// A single key press on the client may result in more than one key event being
// sent to the server.

const unsigned int MaxKeysPerKey = 4;
const CARD32 VoidKeyCode = XK_VoidSymbol;


// keycodes contains the keysyms terminated by a VoidKeyCode.  releaseModifiers
// is a set of OR'ed flags indicating whether particular modifier release
// events should be sent before the key event and modifier press events should
// be sent after it.

const CARD32 KEYMAP_LCONTROL = 0x0001;
const CARD32 KEYMAP_RCONTROL = 0x0002;
const CARD32 KEYMAP_LALT     = 0x0004;
const CARD32 KEYMAP_RALT     = 0x0008;

typedef struct {
  CARD32 keycodes[MaxKeysPerKey];
  CARD32 releaseModifiers;
} KeyActionSpec;



class KeyMap
{
  public:
    KeyMap();
    KeyActionSpec PCtoX(UINT virtkey, DWORD keyData);
  private:
    // CARD32 keymap[256];
    WCHAR buf[10];  // lots of space for now
    BYTE keystate[256];
};

#endif  // KEYMAP_H__
