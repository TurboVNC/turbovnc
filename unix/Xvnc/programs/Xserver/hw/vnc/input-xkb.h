/* Copyright (C) 2014 D. R. Commander.  All Rights Reserved.
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

#include <X11/X.h>
#include "eventstr.h"
#include "input.h"

unsigned GetKeyboardState(void);
unsigned GetLevelThreeMask(void);
KeyCode PressShift(void);
KeyCode *ReleaseShift(void);
KeyCode PressLevelThree(void);
KeyCode *ReleaseLevelThree(void);
KeyCode KeysymToKeycode(KeySym keysym, unsigned state, unsigned *new_state);
Bool IsLockModifier(KeyCode keycode, unsigned state);
Bool IsAffectedByNumLock(KeyCode keycode);
KeyCode AddKeysym(KeySym keysym, unsigned state);
void vncXkbProcessDeviceEvent(int screenNum, InternalEvent *event,
                              DeviceIntPtr dev);
