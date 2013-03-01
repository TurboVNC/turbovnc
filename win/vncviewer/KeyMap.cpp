//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
//  Copyright (C) 2013 D. R. Commander. All Rights Reserved.
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

// Thanks to Martin C. Mueller <mcm@itwm.uni-kl.de> for assistance with the
// international keyboard mapping stuff.

// KeyMap.cpp

#include "stdhdrs.h"
#include "KeyMap.h"
#include "vncviewer.h"


typedef struct vncKeymapping_struct {
  UINT wincode;
  UINT Xcode;
} vncKeymapping;


// Make up a VK for Enter - I think anything outside the range 0-255 will do
static const UINT VK_KEYPAD_ENTER  = 0x1000;
static const UINT VK_KEYPAD_HOME   = 0x1001;
static const UINT VK_KEYPAD_LEFT   = 0x1002;
static const UINT VK_KEYPAD_UP     = 0x1003;
static const UINT VK_KEYPAD_RIGHT  = 0x1004;
static const UINT VK_KEYPAD_DOWN   = 0x1005;
static const UINT VK_KEYPAD_PRIOR  = 0x1006;
static const UINT VK_KEYPAD_NEXT   = 0x1007;
static const UINT VK_KEYPAD_END    = 0x1008;
static const UINT VK_KEYPAD_BEGIN  = 0x1009;
static const UINT VK_KEYPAD_INSERT = 0x100A;
static const UINT VK_KEYPAD_DELETE = 0x100B;

static const vncKeymapping keymap[] = {
  {VK_BACK,         XK_BackSpace},
  {VK_TAB,          XK_Tab},
  {VK_CLEAR,        XK_Clear},
  {VK_RETURN,       XK_Return},
  {VK_LSHIFT,       XK_Shift_L},
  {VK_RSHIFT,       XK_Shift_R},
  {VK_SHIFT,        XK_Shift_L},
  {VK_LCONTROL,     XK_Control_L},
  {VK_RCONTROL,     XK_Control_R},
  {VK_CONTROL,      XK_Control_L},
  {VK_LMENU,        XK_Alt_L},
  {VK_RMENU,        XK_Alt_R},
  {VK_MENU,         XK_Alt_L},
  {VK_PAUSE,        XK_Pause},
  {VK_CAPITAL,      XK_Caps_Lock},
  {VK_ESCAPE,       XK_Escape},
  {VK_SPACE,        XK_space},
  {VK_PRIOR,        XK_Page_Up},
  {VK_NEXT,         XK_Page_Down},
  {VK_END,          XK_End},
  {VK_HOME,         XK_Home},
  {VK_LEFT,         XK_Left},
  {VK_UP,           XK_Up},
  {VK_RIGHT,        XK_Right},
  {VK_DOWN,         XK_Down},
  {VK_SELECT,       XK_Select},
  {VK_EXECUTE,      XK_Execute},
  {VK_SNAPSHOT,     XK_Print},
  {VK_INSERT,       XK_Insert},
  {VK_DELETE,       XK_Delete},
  {VK_HELP,         XK_Help},
  {VK_NUMPAD0,      XK_KP_0},
  {VK_NUMPAD1,      XK_KP_1},
  {VK_NUMPAD2,      XK_KP_2},
  {VK_NUMPAD3,      XK_KP_3},
  {VK_NUMPAD4,      XK_KP_4},
  {VK_NUMPAD5,      XK_KP_5},
  {VK_NUMPAD6,      XK_KP_6},
  {VK_NUMPAD7,      XK_KP_7},
  {VK_NUMPAD8,      XK_KP_8},
  {VK_NUMPAD9,      XK_KP_9},
  {VK_MULTIPLY,     XK_KP_Multiply},
  {VK_ADD,          XK_KP_Add},
  {VK_SEPARATOR,    XK_KP_Separator},   // often comma
  {VK_SUBTRACT,     XK_KP_Subtract},
  {VK_DECIMAL,      XK_KP_Decimal},
  {VK_DIVIDE,       XK_KP_Divide},
  {VK_F1,           XK_F1},
  {VK_F2,           XK_F2},
  {VK_F3,           XK_F3},
  {VK_F4,           XK_F4},
  {VK_F5,           XK_F5},
  {VK_F6,           XK_F6},
  {VK_F7,           XK_F7},
  {VK_F8,           XK_F8},
  {VK_F9,           XK_F9},
  {VK_F10,          XK_F10},
  {VK_F11,          XK_F11},
  {VK_F12,          XK_F12},
  {VK_F13,          XK_F13},
  {VK_F14,          XK_F14},
  {VK_F15,          XK_F15},
  {VK_F16,          XK_F16},
  {VK_F17,          XK_F17},
  {VK_F18,          XK_F18},
  {VK_F19,          XK_F19},
  {VK_F20,          XK_F20},
  {VK_F21,          XK_F21},
  {VK_F22,          XK_F22},
  {VK_F23,          XK_F23},
  {VK_F24,          XK_F24},
  {VK_NUMLOCK,      XK_Num_Lock},
  {VK_SCROLL,       XK_Scroll_Lock},
  {VK_KEYPAD_ENTER, XK_KP_Enter},
  {VK_KEYPAD_HOME,  XK_KP_Home},
  {VK_KEYPAD_LEFT,  XK_KP_Left},
  {VK_KEYPAD_UP,    XK_KP_Up},
  {VK_KEYPAD_RIGHT, XK_KP_Right},
  {VK_KEYPAD_DOWN,  XK_KP_Down},
  {VK_KEYPAD_PRIOR, XK_KP_Prior},
  {VK_KEYPAD_NEXT,  XK_KP_Next},
  {VK_KEYPAD_END,   XK_KP_End},
  {VK_KEYPAD_BEGIN, XK_KP_Begin},
  {VK_KEYPAD_INSERT, XK_KP_Insert},
  {VK_KEYPAD_DELETE, XK_KP_Delete},
  {VK_CANCEL,       XK_Break}
};


KeyMap::KeyMap()
{
};


KeyActionSpec KeyMap::PCtoX(UINT virtkey, DWORD keyData)
{
  UINT numkeys = 0;

  KeyActionSpec kas;
  kas.releaseModifiers = 0;

  bool extended = ((keyData & 0x1000000) != 0);
  vnclog.Print(8, " keyData %04x ", keyData);

  if (extended) {
    vnclog.Print(8, " (extended) ");
    switch (virtkey) {
      case VK_MENU :
        virtkey = VK_RMENU;  break;
      case VK_CONTROL:
        virtkey = VK_RCONTROL;  break;
      case VK_RETURN:
        virtkey = VK_KEYPAD_ENTER;  break;
    }
  } else {
    switch (virtkey) {
      case VK_HOME:
        virtkey = VK_KEYPAD_HOME;  break;
      case VK_LEFT:
        virtkey = VK_KEYPAD_LEFT;  break;
      case VK_UP:
        virtkey = VK_KEYPAD_UP;  break;
      case VK_RIGHT:
        virtkey = VK_KEYPAD_RIGHT;  break;
      case VK_DOWN:
        virtkey = VK_KEYPAD_DOWN;  break;
      case VK_PRIOR:
        virtkey = VK_KEYPAD_PRIOR;  break;
      case VK_NEXT:
        virtkey = VK_KEYPAD_NEXT;  break;
      case VK_END:
        virtkey = VK_KEYPAD_END;  break;
      case VK_CLEAR:
        virtkey = VK_KEYPAD_BEGIN;  break;
      case VK_INSERT:
        virtkey = VK_KEYPAD_INSERT;  break;
      case VK_DELETE:
        virtkey = VK_KEYPAD_DELETE;  break;
    }
  }

  // Try looking it up in our table
  UINT mapsize = sizeof(keymap) / sizeof(vncKeymapping);

  // Look up the desired code in the table
  for (UINT i = 0; i < mapsize; i++) {
    if (keymap[i].wincode == virtkey) {
      kas.keycodes[numkeys++] = keymap[i].Xcode;
      break;
    }
  }

  if (numkeys != 0) {
    // A special case - use Meta instead of Alt if ScrollLock is on
    UINT key = kas.keycodes[numkeys - 1];
    if ((key == XK_Alt_L || key == XK_Alt_R) &&
        GetKeyState(VK_SCROLL)) {
      if (key == XK_Alt_L)
        kas.keycodes[numkeys-1] = XK_Meta_L;
      else
        kas.keycodes[numkeys-1] = XK_Meta_R;
    }
    vnclog.Print(8, "keymap gives %u (%x) ", key, key);

  } else {
    // not found in table
    vnclog.Print(8, "not in special keymap, ");

    // Try a simple conversion to ASCII, using the current keyboard mapping
    GetKeyboardState(keystate);

    int ret = ToAscii(virtkey, 0, keystate, (WORD *) buf, 0);

    // If Left Ctrl & Alt are both pressed and ToAscii() gives a valid keysym.
    // (This is for AltGr on international keyboards (= LCtrl-Alt).
    // e.g. Ctrl-Alt-Q gives @ on German keyboards.)
    if (((keystate[VK_MENU] & 0x80) != 0) &&
        ((keystate[VK_CONTROL] & 0x80) != 0)) {

      // If the key means anything in this keyboard layout
      if ((ret >= 1) && (((*buf >= 32) && (*buf <= 126)) ||
                         ((*buf >= 160) && (*buf <= 255)))) {

        // Release the modifiers, then the keystroke, then press the modifiers.
        // We don't release Right Ctrl; this allows German users to use it for
        // doing Ctrl-@ etc.

        if (GetKeyState(VK_LCONTROL) & 0x8000)
          kas.releaseModifiers |= KEYMAP_LCONTROL;
        if (GetKeyState(VK_LMENU) & 0x8000)
          kas.releaseModifiers |= KEYMAP_LALT;
        if (GetKeyState(VK_RMENU) & 0x8000)
          kas.releaseModifiers |= KEYMAP_RALT;

        // This is for Windows '95, and possibly other systems.  The above
        // GetKeyState() calls don't work in '95 - they always return 0.
        // But if we're at this point in the code, we know that Ctrl and Alt
        // are pressed, so let's release all Ctrl and Alt keys if we haven't
        // registered them as already having been released.
        if (kas.releaseModifiers == 0)
          kas.releaseModifiers = KEYMAP_LCONTROL | KEYMAP_LALT | KEYMAP_RALT;

        vnclog.Print(8, "Ctrl-Alt pressed: ToAscii (without modifiers) returns %d byte(s): ",
                     ret);
        for (int i = 0; i < ret; i++) {
          kas.keycodes[numkeys++] = *(buf + i);
          vnclog.Print(8, "%02x (%c) ", *(buf + i) , *(buf + i));
        }
        vnclog.Print(8, "\n");
      }
    }

    // If not a Ctrl-Alt key
    if (numkeys == 0) {

      // There are no keysyms corresponding to control characters, e.g. Ctrl-F.
      // The server already knows whether the Ctrl key is pressed, so we are
      // interested in the key that would be sent if the Ctrl key were not
      // pressed.
      keystate[VK_CONTROL] = keystate[VK_LCONTROL] = keystate[VK_RCONTROL] = 0;

      int ret = ToAscii(virtkey, 0, keystate, (WORD *) buf, 0);
      if (ret < 0) {
        switch (*buf) {
          case '`' :
            kas.keycodes[numkeys++] = XK_dead_grave;  break;
          case '\'' :
            kas.keycodes[numkeys++] = XK_dead_acute;  break;
          case '~' :
            kas.keycodes[numkeys++] = XK_dead_tilde;  break;
          case '^':
            kas.keycodes[numkeys++] = XK_dead_circumflex;  break;
          case 168:
            // dead_tilde / dead_diaeresis
            if ((GetKeyState(VK_CONTROL) & 0x8000) &&
                (GetKeyState(VK_MENU) & 0x8000)) {
              // AltGr is pressed
              kas.keycodes[numkeys++] = XK_dead_tilde;
            } else {
              kas.keycodes[numkeys++] = XK_dead_diaeresis;
            }
            break;
        }
      }
      // If this works, and it's a regular printable character, we just send
      // that
      if (ret >= 1) {
        vnclog.Print(8, "ToAscii (without ctrl) returns %d byte(s): ",
                     ret);
        for (int i = 0; i < ret; i++) {
          kas.keycodes[numkeys++] = *(buf + i);
          vnclog.Print(8, "%02x (%c) ", *(buf + i) , *(buf + i));
        }
      }
    }
  }

  kas.keycodes[numkeys] = VoidKeyCode;
  return kas;
};
