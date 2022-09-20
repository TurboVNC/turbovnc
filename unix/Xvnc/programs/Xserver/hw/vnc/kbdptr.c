/*
 * kbdptr.c - deal with keyboard and pointer device over TCP.
 *
 *
 */

/* Copyright (C) 2014-2016, 2019, 2021-2022 D. R. Commander.
 *                                          All Rights Reserved.
 * Copyright (C) 2013, 2017-2018 Pierre Ossman for Cendio AB.
 *                               All Rights Reserved.
 * Copyright (C) 2009 Red Hat, Inc.  All Rights Reserved.
 * Copyright (C) 2009 TightVNC Team.  All Rights Reserved.
 * Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
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

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include <X11/X.h>
#include <X11/keysym.h>
#include "inputstr.h"
#include "inpututils.h"
#include "xkbsrv.h"
#include "mi.h"
#include "rfb.h"
#include "input-xkb.h"
#include "qnum_to_xorgevdev.h"
#include "qnum_to_xorgkbd.h"

DeviceIntPtr kbdDevice = NULL;
static DeviceIntPtr ptrDevice = NULL;

/* If TRUE, then keys META == ALT as in the original AT&T version. */
Bool compatibleKbd = FALSE;

/* Avoid fake Shift presses for keys affected by NumLock */
Bool avoidShiftNumLock = TRUE;
Bool ignoreLockModifiers = TRUE;
Bool fakeShift = TRUE;
Bool enableQEMUExtKeyEvent = TRUE;

unsigned char ptrAcceleration = 50;

#define NoSymbol4 NoSymbol, NoSymbol, NoSymbol, NoSymbol,
#define NoSymbol16 NoSymbol4 NoSymbol4 NoSymbol4 NoSymbol4
#define NoSymbol64 NoSymbol16 NoSymbol16 NoSymbol16 NoSymbol16
KeySym pressedKeys[256] = { NoSymbol64 NoSymbol64 NoSymbol64 NoSymbol64 };

static const unsigned short *codeMap;
static unsigned int codeMapLen;


void KbdDeviceInit(DeviceIntPtr pDevice)
{
  char *env;

  kbdDevice = pDevice;
  if ((env = getenv("TVNC_XKBFAKESHIFT")) != NULL && !strcmp(env, "0")) {
    rfbLog("Disabling fake shift key event generation in XKEYBOARD handler\n");
    fakeShift = FALSE;
  }
  if ((env = getenv("TVNC_XKBIGNORELOCK")) != NULL && !strcmp(env, "0")) {
    rfbLog("Allowing Caps Lock and other lock modifiers in XKEYBOARD handler\n");
    ignoreLockModifiers = FALSE;
  }
}


void QEMUExtKeyboardEventInit(void)
{
  XkbRMLVOSet rmlvo;

  XkbGetRulesDflts(&rmlvo);
  if (!strcmp(rmlvo.rules, "evdev")) {
    codeMap = code_map_qnum_to_xorgevdev;
    codeMapLen = code_map_qnum_to_xorgevdev_len;
  } else if (!strcmp(rmlvo.rules, "base") || !strcmp(rmlvo.rules, "xorg")) {
    codeMap = code_map_qnum_to_xorgkbd;
    codeMapLen = code_map_qnum_to_xorgkbd_len;
  } else {
    rfbLog("WARNING: QEMU Extended Key Event protocol extension\n");
    rfbLog("  requires evdev or xorg XKB rules.  Disabling extension\n");
    enableQEMUExtKeyEvent = FALSE;
  }
  XkbFreeRMLVOSet(&rmlvo, FALSE);
}


void PtrDeviceOn(DeviceIntPtr pDev)
{
  ptrAcceleration = (char)pDev->ptrfeed->ctrl.num;
  ptrDevice = pDev;
}


void PtrDeviceControl(DevicePtr dev, PtrCtrl *ctrl)
{
  ptrAcceleration = (char)ctrl->num;
}


static inline void PressKey(DeviceIntPtr dev, int kc, Bool down,
                            const char *msg)
{
  int action;

  if (msg != NULL)
    LogMessage(X_DEBUG, "PressKey: %s %d %s\n", msg, kc, down ? "down" : "up");

  action = down ? KeyPress : KeyRelease;
  QueueKeyboardEvents(dev, action, kc);
}


/* altKeysym is a table of alternative keysyms which have the same meaning. */

static struct altKeysym_t {
  KeySym a, b;
} altKeysym[] = {
  { XK_Shift_L, XK_Shift_R },
  { XK_Control_L, XK_Control_R },
  { XK_Meta_L, XK_Meta_R },
  { XK_Alt_L, XK_Alt_R },
  { XK_Super_L, XK_Super_R },
  { XK_Hyper_L, XK_Hyper_R },
  { XK_KP_Space, XK_space },
  { XK_KP_Tab, XK_Tab },
  { XK_KP_Enter, XK_Return },
  { XK_KP_F1, XK_F1 },
  { XK_KP_F2, XK_F2 },
  { XK_KP_F3, XK_F3 },
  { XK_KP_F4, XK_F4 },
  { XK_KP_Home, XK_Home },
  { XK_KP_Left, XK_Left },
  { XK_KP_Up, XK_Up },
  { XK_KP_Right, XK_Right },
  { XK_KP_Down, XK_Down },
  { XK_KP_Page_Up, XK_Page_Up },
  { XK_KP_Page_Down, XK_Page_Down },
  { XK_KP_End, XK_End },
  { XK_KP_Begin, XK_Begin },
  { XK_KP_Insert, XK_Insert },
  { XK_KP_Delete, XK_Delete },
  { XK_KP_Equal, XK_equal },
  { XK_KP_Multiply, XK_asterisk },
  { XK_KP_Add, XK_plus },
  { XK_KP_Separator, XK_comma },
  { XK_KP_Subtract, XK_minus },
  { XK_KP_Decimal, XK_period },
  { XK_KP_Divide, XK_slash },
  { XK_KP_0, XK_0 },
  { XK_KP_1, XK_1 },
  { XK_KP_2, XK_2 },
  { XK_KP_3, XK_3 },
  { XK_KP_4, XK_4 },
  { XK_KP_5, XK_5 },
  { XK_KP_6, XK_6 },
  { XK_KP_7, XK_7 },
  { XK_KP_8, XK_8 },
  { XK_KP_9, XK_9 },
  { XK_ISO_Level3_Shift, XK_Mode_switch },
};


/*
 * ExtKeyEvent() - handle raw keycode from QEMU Extended Key Event extension
 */

void ExtKeyEvent(KeySym keysym, unsigned keycode, BOOL down)
{
  /* Simple case: the client has specified the keycode */
  if (keycode && keycode < codeMapLen) {
    keycode = codeMap[keycode];
    if (!keycode) {
      /* No code map entry found for the keycode.  Try to use the keysym
         instead. */
      if (keysym) KeyEvent(keysym, down);
      return;
    }

    /* Update the pressed keys map in case we get a mix of events with and
       without keycodes. */
    if (down && keysym) pressedKeys[keycode] = keysym;
    else pressedKeys[keycode] = NoSymbol;

    PressKey(kbdDevice, keycode, down, "raw keycode");
    mieqProcessInputEvents();
    return;
  }

  /* The keycode is 0 (should never happen) or exceeds the code map length.
     Try to use the keysym instead. */
  if (keysym) KeyEvent(keysym, down);
}


/*
 * KeyEvent() - work out the best keycode corresponding to the keysym sent by
 * the viewer. This is basically impossible in the general case, but we make
 * a best effort by assuming that all useful keysyms can be reached using
 * just the Shift and Level 3 (AltGr) modifiers. For core keyboards this is
 * basically always TRUE, and should be TRUE for most sane, western XKB
 * layouts.
 *
 * This code was borrowed from TigerVNC.
 */

void KeyEvent(CARD32 keysym, Bool down)
{
  int i;
  unsigned state, new_state;
  KeyCode keycode;

  unsigned level_three_mask;
  KeyCode shift_press = 0, level_three_press = 0;
  KeyCode *shift_release = NULL, *level_three_release = NULL;

  /*
   * Release events must match the press event, so look up what
   * keycode we sent for the press.
   */
  if (!down) {
    for (i = 0; i < 256; i++) {
      if (pressedKeys[i] == keysym) {
        pressedKeys[i] = NoSymbol;
        PressKey(kbdDevice, i, FALSE, "keycode");
        mieqProcessInputEvents();
        return;
      }
    }

    /*
     * This can happen quite often as we ignore some
     * key presses.
     */
    LogMessage(X_DEBUG, "Unexpected release of keysym 0x%x\n", keysym);

    return;
  }

  /*
   * Since we are checking the current state to determine if we need
   * to fake modifiers, we must make sure that everything put on the
   * input queue is processed before we start. Otherwise, shift may be
   * stuck down.
   */
  mieqProcessInputEvents();

  state = GetKeyboardState();

  keycode = KeysymToKeycode(keysym, state, &new_state);

  /*
   * Shift+Alt is often mapped to Meta, so try that rather than
   * allocating a new entry, faking shift, or using the dummy
   * key entries that many layouts have.
   */
  if ((state & ShiftMask) &&
      ((keysym == XK_Alt_L) || (keysym == XK_Alt_R))) {
    KeyCode alt, meta;

    if (keysym == XK_Alt_L) {
      alt = KeysymToKeycode(XK_Alt_L, state & ~ShiftMask, NULL);
      meta = KeysymToKeycode(XK_Meta_L, state, NULL);
    } else {
      alt = KeysymToKeycode(XK_Alt_R, state & ~ShiftMask, NULL);
      meta = KeysymToKeycode(XK_Meta_R, state, NULL);
    }

    if ((meta != 0) && (alt == meta)) {
      LogMessage(X_DEBUG, "Replacing Shift+Alt with Shift+Meta\n");
      keycode = meta;
      new_state = state;
    }
  }

  /* Try some equivalent keysyms if we couldn't find a perfect match */
  if (keycode == 0) {
    for (i = 0; i < sizeof(altKeysym) / sizeof(altKeysym[0]); i++) {
      KeySym altsym;

      if (altKeysym[i].a == keysym)
        altsym = altKeysym[i].b;
      else if (altKeysym[i].b == keysym)
        altsym = altKeysym[i].a;
      else
        continue;

      keycode = KeysymToKeycode(altsym, state, &new_state);
      if (keycode != 0)
        break;
    }
  }

  /* We don't have lock synchronisation... */
  if (IsLockModifier(keycode, new_state) && ignoreLockModifiers) {
    LogMessage(X_DEBUG, "Ignoring lock key (e.g. caps lock)\n");
    return;
  }

  /* No matches. Will have to add a new entry... */
  if (keycode == 0) {
    keycode = AddKeysym(keysym, state);
    if (keycode == 0) {
      rfbLog("ERROR: Could not add new keysym 0x%x\n", keysym);
      return;
    }

    rfbLog("Mapped unknown keysym 0x%x to keycode %d\n", keysym, keycode);

    /*
     * The state given to addKeysym() is just a hint and
     * the actual result might still require some state
     * changes.
     */
    keycode = KeysymToKeycode(keysym, state, &new_state);
    if (keycode == 0) {
      rfbLog("ERROR: Cannot generate keycode for newly-added keysym 0x%x\n",
             keysym);
      return;
    }
  }

  /*
   * X11 generally lets shift toggle the keys on the numeric pad
   * the same way NumLock does. This is however not the case on
   * other systems like Windows. As a result, some applications
   * get confused when we do a fake shift to get the same effect
   * that having NumLock active would produce.
   *
   * Until we have proper NumLock synchronisation (so we can
   * avoid faking shift), we try to avoid the fake shifts if we
   * can use an alternative keysym.
   */
  if (((state & ShiftMask) != (new_state & ShiftMask)) && avoidShiftNumLock &&
      IsAffectedByNumLock(keycode)) {
    KeyCode keycode2 = 0;
    unsigned new_state2;

    LogMessage(X_DEBUG, "Finding alternative to keysym 0x%x to avoid fake shift for numpad\n",
               keysym);

    for (i = 0; i < sizeof(altKeysym) / sizeof(altKeysym[0]); i++) {
      KeySym altsym;

      if (altKeysym[i].a == keysym)
        altsym = altKeysym[i].b;
      else if (altKeysym[i].b == keysym)
        altsym = altKeysym[i].a;
      else
        continue;

      keycode2 = KeysymToKeycode(altsym, state, &new_state2);
      if (keycode2 == 0)
        continue;

      if (((state & ShiftMask) != (new_state2 & ShiftMask)) &&
          IsAffectedByNumLock(keycode2))
        continue;

      break;
    }

    if (i == sizeof(altKeysym) / sizeof(altKeysym[0])) {
      LogMessage(X_DEBUG, "No alternative keysym found\n");
    } else {
      keycode = keycode2;
      new_state = new_state2;
    }
  }

  /*
   * "Shifted Tab" is a bit of a mess. Some systems have varying,
   * special keysyms for this symbol. VNC mandates that clients
   * should always send the plain XK_Tab keysym and the server
   * should deduce the meaning based on current Shift state.
   * To comply with this, we will find the keycode that sends
   * XK_Tab, and make sure that Shift isn't cleared. This can
   * possibly result in a different keysym than XK_Tab, but that
   * is the desired behaviour.
   *
   * Note: We never get ISO_Left_Tab here because it's already
   *       been translated in VNCSConnectionST.
   */
  if (keysym == XK_Tab && (state & ShiftMask))
    new_state |= ShiftMask;

  /*
   * We need a bigger state change than just shift,
   * so we need to know what the mask is for level 3 shifts.
   */
  if ((new_state & ~ShiftMask) != (state & ~ShiftMask))
    level_three_mask = GetLevelThreeMask();
  else
    level_three_mask = 0;

  shift_press = level_three_press = 0;

  /* Need a fake press or release of shift? */
  if (!(state & ShiftMask) && (new_state & ShiftMask) && fakeShift) {
    shift_press = PressShift();
    if (shift_press == 0) {
      rfbLog("ERROR: Unable to find modifier key for Shift key press\n");
      return;
    }
    PressKey(kbdDevice, shift_press, TRUE, "temp shift");
  } else if ((state & ShiftMask) && !(new_state & ShiftMask) && fakeShift) {
    int index = 0;
    shift_release = ReleaseShift();
    if (!shift_release) {
      rfbLog("ERROR: Unable to find modifier key(s) for Shift key release\n");
      return;
    }
    while (shift_release[index])
      PressKey(kbdDevice, shift_release[index++], FALSE, "temp shift");
  }

  /* Need a fake press or release of level three shift? */
  if (!(state & level_three_mask) && (new_state & level_three_mask) &&
      fakeShift) {
    level_three_press = PressLevelThree();
    if (level_three_press == 0) {
      rfbLog("ERROR: Unable to find modifier key for ISO_Level3_Shift/Mode_Switch key press\n");
      return;
    }
    PressKey(kbdDevice, level_three_press, TRUE, "temp level 3 shift");
  } else if ((state & level_three_mask) && !(new_state & level_three_mask) &&
             fakeShift) {
    int index = 0;
    level_three_release = ReleaseLevelThree();
    if (!level_three_release) {
      rfbLog("ERROR: Unable to find modifier key(s) for ISO_Level3_Shift/Mode_Switch key release\n");
      return;
    }
    while (level_three_release[index])
      PressKey(kbdDevice, level_three_release[index++], FALSE,
               "temp level 3 shift");
  }

  /* Now press the actual key */
  PressKey(kbdDevice, keycode, TRUE, "keycode");

  /* And store the mapping so that we can do a proper release later */
  for (i = 0; i < 256; i++) {
    if (i == keycode)
      continue;
    if (pressedKeys[i] == keysym) {
      rfbLog("ERROR: Keysym 0x%x generated by both keys %d and %d\n", keysym,
             i, keycode);
      pressedKeys[i] = NoSymbol;
    }
  }

  pressedKeys[keycode] = keysym;

  /* Undo any fake level three shift */
  if (level_three_press != 0)
    PressKey(kbdDevice, level_three_press, FALSE, "temp level 3 shift");
  else if (level_three_release) {
    int index = 0;
    while (level_three_release[index])
      PressKey(kbdDevice, level_three_release[index++], TRUE,
               "temp level 3 shift");
    free(level_three_release);
  }

  /* Undo any fake shift */
  if (shift_press != 0)
    PressKey(kbdDevice, shift_press, FALSE, "temp shift");
  else if (shift_release) {
    int index = 0;
    while (shift_release[index])
      PressKey(kbdDevice, shift_release[index++], TRUE, "temp shift");
    free(shift_release);
  }

  /*
   * When faking a modifier we are putting a keycode (which can
   * currently activate the desired modifier) on the input
   * queue. A future modmap change can change the mapping so
   * that this keycode means something else entirely. Guard
   * against this by processing the queue now.
   */
  mieqProcessInputEvents();
}


static int cursorPosX = -1, cursorPosY = -1;


void PtrAddEvent(int buttonMask, int x, int y, rfbClientPtr cl)
{
  int i;
  int valuators[2];
  ValuatorMask mask;
  static int oldButtonMask = 0;

  if (!ptrDevice)
    FatalError("Pointer device not initialized");

  if (cursorPosX != x || cursorPosY != y) {
    valuators[0] = x;
    valuators[1] = y;
    valuator_mask_set_range(&mask, 0, 2, valuators);
    QueuePointerEvents(ptrDevice, MotionNotify, 0, POINTER_ABSOLUTE, &mask);

    cursorPosX = x;
    cursorPosY = y;
  }

  for (i = 0; i < 5; i++) {
    if ((buttonMask ^ oldButtonMask) & (1 << i)) {
      if (buttonMask & (1 << i)) {
        valuator_mask_set_range(&mask, 0, 0, NULL);
        QueuePointerEvents(ptrDevice, ButtonPress, i + 1, POINTER_RELATIVE,
                           &mask);
      } else {
        valuator_mask_set_range(&mask, 0, 0, NULL);
        QueuePointerEvents(ptrDevice, ButtonRelease, i + 1, POINTER_RELATIVE,
                           &mask);
      }
    }
  }

  oldButtonMask = buttonMask;
  mieqProcessInputEvents();
}


char *stristr(const char *s1, const char *s2)
{
  char *str1, *str2, *ret;
  int i;

  if (!s1 || !s2 || strlen(s1) < 1 || strlen(s2) < 1)
    return NULL;

  str1 = strdup(s1);
  for (i = 0; i < strlen(str1); i++)
    str1[i] = tolower(str1[i]);
  str2 = strdup(s2);
  for (i = 0; i < strlen(str2); i++)
    str2[i] = tolower(str2[i]);

  ret = strstr(str1, str2);
  free(str1);  free(str2);
  return ret;
}


void ExtInputAddEvent(rfbDevInfoPtr dev, int type, int buttons)
{
  ValuatorMask mask;

  if (!dev)
    FatalError("ExtInputDeviceAddEvent(): Invalid argument");

  if (rfbVirtualTablet) {
    int i;
    rfbDevInfoPtr vtDev;

    switch (dev->productID) {
      case rfbGIIDevTypeStylus:
        vtDev = &virtualTabletStylus;
        break;
      case rfbGIIDevTypeEraser:
        vtDev = &virtualTabletEraser;
        break;
      case rfbGIIDevTypeTouch:
        vtDev = &virtualTabletTouch;
        break;
      case rfbGIIDevTypePad:
        vtDev = &virtualTabletPad;
        break;
      default:
        if (stristr(dev->name, "stylus"))
          vtDev = &virtualTabletStylus;
        else if (stristr(dev->name, "eraser"))
          vtDev = &virtualTabletEraser;
        else if (stristr(dev->name, "touch"))
          vtDev = &virtualTabletTouch;
        else if (stristr(dev->name, "pad"))
          vtDev = &virtualTabletPad;
        else
          return;
    }

    if (dev->valFirst >= vtDev->numValuators || buttons > vtDev->numButtons)
      return;

    vtDev->valFirst = dev->valFirst;
    vtDev->valCount =
      min(dev->valCount, vtDev->numValuators - vtDev->valFirst);

    for (i = vtDev->valFirst; i < vtDev->valFirst + vtDev->valCount; i++) {
      vtDev->values[i] =
        (int)round((double)(dev->values[i] - dev->valuators[i].rangeMin) /
                   (double)(dev->valuators[i].rangeMax -
                            dev->valuators[i].rangeMin) *
                   (double)(vtDev->valuators[i].rangeMax -
                            vtDev->valuators[i].rangeMin) +
                   (double)vtDev->valuators[i].rangeMin);
    }
    dev = vtDev;
  }

  if (dev->valCount > 0) {
    if (type == MotionNotify && dev->multitouch &&
        dev->valFirst + dev->valCount <= dev->numValuators) {
      int touchID = dev->values[dev->numValuators - 2];
      int touchType = dev->values[dev->numValuators - 1];
      Bool touchEmulatingPointer = FALSE;

      valuator_mask_set_range(&mask, 0, dev->numValuators - 2, dev->values);
      if (touchType >= rfbGIITouchBeginEP) {
        touchEmulatingPointer = TRUE;
        touchType -= 3;
      }
      QueueTouchEvents(dev->pDev, XI_TouchBegin + touchType, touchID,
                       touchEmulatingPointer ? TOUCH_POINTER_EMULATED : 0,
                       &mask);
    } else {
      valuator_mask_set_range(&mask, 0, dev->numValuators, dev->values);
      QueuePointerEvents(dev->pDev, type, buttons,
                         dev->mode == Absolute ? POINTER_ABSOLUTE :
                                                 POINTER_RELATIVE,
                         &mask);
    }
  } else {
    valuator_mask_set_range(&mask, 0, 0, NULL);
    QueuePointerEvents(dev->pDev, type, buttons, POINTER_RELATIVE, &mask);
  }
  mieqProcessInputEvents();
}


void KbdReleaseAllKeys(void)
{
  int i, j;

  if (!kbdDevice)
    FatalError("Keyboard device not initialized");

  for (i = 0; i < DOWN_LENGTH; i++) {
    if (kbdDevice->key->down[i] != 0) {
      for (j = 0; j < 8; j++) {
        if (kbdDevice->key->down[i] & (1 << j))
          QueueKeyboardEvents(kbdDevice, KeyRelease, (i << 3) | j);
      }
    }
  }
}
