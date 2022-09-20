/* Copyright (C) 2015-2019, 2021-2022 D. R. Commander.  All Rights Reserved.
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

#ifdef __SUNPRO_C
/* Oracle Developer Studio sometimes erroneously detects the THROW() macro
   followed by a semicolon as an unreachable statement. */
#pragma error_messages(off, E_STATEMENT_NOT_REACHED)
#endif

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <dlfcn.h>
#include <unistd.h>
#include "jawt_md.h"
#include <X11/extensions/XInput2.h>
#include <X11/Xatom.h>
#include "com_turbovnc_vncviewer_Viewport.h"
#include <X11/Xmd.h>
#include <X11/Xatom.h>
#include "rfbproto.h"
#include "turbovnc_gii.h"


#define THROW(msg) {  \
  jclass _exccls = (*env)->FindClass(env, "java/lang/Exception");  \
  if (!_exccls) goto bailout;  \
  (*env)->ThrowNew(env, _exccls, msg);  \
  goto bailout;  \
}

#define BAILIF0(f) {  \
  if (!(f) || (*env)->ExceptionCheck(env)) {  \
    goto bailout;  \
  }  \
}

#define SET_STRING(cls, obj, fieldName, string) {  \
  jstring str;  \
  BAILIF0(fid = (*env)->GetFieldID(env, cls, #fieldName,  \
                                   "Ljava/lang/String;"));  \
  BAILIF0(str = (*env)->NewStringUTF(env, string));  \
  (*env)->SetObjectField(env, obj, fid, (jobject)str);  \
}

#define SET_LONG(cls, obj, fieldName, val) {  \
  BAILIF0(fid = (*env)->GetFieldID(env, cls, #fieldName, "J"));  \
  (*env)->SetLongField(env, obj, fid, val);  \
}

#define SET_INT(cls, obj, fieldName, val) {  \
  BAILIF0(fid = (*env)->GetFieldID(env, cls, #fieldName, "I"));  \
  (*env)->SetIntField(env, obj, fid, val);  \
}

#define SET_BOOL(cls, obj, fieldName, val) {  \
  BAILIF0(fid = (*env)->GetFieldID(env, cls, #fieldName, "Z"));  \
  (*env)->SetBooleanField(env, obj, fid, val);  \
}


struct isXIEventParams {
  int xiType, xiOpcode;
};


typedef jboolean JNICALL (*__JAWT_GetAWT_type) (JNIEnv *env, JAWT *awt);
static __JAWT_GetAWT_type __JAWT_GetAWT = NULL;

static void *handle = NULL;


/*
 * These functions are borrowed from OpenSUSE.  They give the window manager a
 * hint that the window is full-screen so the taskbar won't appear on top
 * of it.
 */

#define _NET_WM_STATE_REMOVE        0    /* remove/unset property */
#define _NET_WM_STATE_ADD           1    /* add/set property */

static void netwm_set_state(Display *dpy, Window win, int operation,
                            Atom state)
{
  XEvent e;
  Atom _NET_WM_STATE = XInternAtom(dpy, "_NET_WM_STATE", False);

  memset(&e, 0, sizeof(e));
  e.xclient.type = ClientMessage;
  e.xclient.message_type = _NET_WM_STATE;
  e.xclient.display = dpy;
  e.xclient.window = win;
  e.xclient.format = 32;
  e.xclient.data.l[0] = operation;
  e.xclient.data.l[1] = state;

  XSendEvent(dpy, DefaultRootWindow(dpy), False, SubstructureRedirectMask, &e);
}

static void netwm_fullscreen(Display *dpy, Window win, int state)
{
  Atom _NET_WM_STATE_FULLSCREEN =
    XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
  int op = state ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE;
  netwm_set_state(dpy, win, op, _NET_WM_STATE_FULLSCREEN);
}


JNIEXPORT void JNICALL Java_com_turbovnc_vncviewer_Viewport_x11FullScreen
  (JNIEnv *env, jobject obj, jboolean on)
{
  JAWT awt;
  JAWT_DrawingSurface *ds = NULL;
  JAWT_DrawingSurfaceInfo *dsi = NULL;
  JAWT_X11DrawingSurfaceInfo *x11dsi = NULL;
  jfieldID fid;
  jclass cls;
  Atom rulesAtom = None, actualType = None;
  int xkbRules = -1, actualFormat = 0;
  unsigned long nItems = 0, bytesAfter;
  unsigned char *data = NULL;

  awt.version = JAWT_VERSION_1_3;
  if (!handle) {
    if ((handle = dlopen("libjawt.so", RTLD_LAZY)) == NULL)
      THROW(dlerror());
    if ((__JAWT_GetAWT =
         (__JAWT_GetAWT_type)dlsym(handle, "JAWT_GetAWT")) == NULL)
      THROW(dlerror());
  }

  if (__JAWT_GetAWT(env, &awt) == JNI_FALSE)
    THROW("Could not initialize AWT native interface");

  if ((ds = awt.GetDrawingSurface(env, obj)) == NULL)
    THROW("Could not get drawing surface");

  if ((ds->Lock(ds) & JAWT_LOCK_ERROR) != 0)
    THROW("Could not lock surface");

  if ((dsi = ds->GetDrawingSurfaceInfo(ds)) == NULL)
    THROW("Could not get drawing surface info");

  if ((x11dsi = (JAWT_X11DrawingSurfaceInfo *)dsi->platformInfo) == NULL)
    THROW("Could not get X11 drawing surface info");

  BAILIF0(cls = (*env)->GetObjectClass(env, obj));

  netwm_fullscreen(x11dsi->display, x11dsi->drawable, on);
  if (on) {
    XEvent e;
    jint leftMon, rightMon, topMon, bottomMon;

    BAILIF0(fid = (*env)->GetFieldID(env, cls, "leftMon", "I"));
    leftMon = (*env)->GetIntField(env, obj, fid);
    BAILIF0(fid = (*env)->GetFieldID(env, cls, "rightMon", "I"));
    rightMon = (*env)->GetIntField(env, obj, fid);
    BAILIF0(fid = (*env)->GetFieldID(env, cls, "topMon", "I"));
    topMon = (*env)->GetIntField(env, obj, fid);
    BAILIF0(fid = (*env)->GetFieldID(env, cls, "bottomMon", "I"));
    bottomMon = (*env)->GetIntField(env, obj, fid);

    memset(&e, 0, sizeof(e));
    e.xclient.type = ClientMessage;
    e.xclient.message_type = XInternAtom(x11dsi->display,
                                         "_NET_WM_FULLSCREEN_MONITORS", False);
    e.xclient.display = x11dsi->display;
    e.xclient.window = x11dsi->drawable;
    e.xclient.format = 32;
    e.xclient.data.l[0] = topMon;
    e.xclient.data.l[1] = bottomMon;
    e.xclient.data.l[2] = leftMon;
    e.xclient.data.l[3] = rightMon;
    e.xclient.data.l[4] = 1;

    XSendEvent(x11dsi->display, DefaultRootWindow(x11dsi->display), False,
               SubstructureRedirectMask | SubstructureNotifyMask, &e);
  }
  XSync(x11dsi->display, False);

  if ((fid = (*env)->GetFieldID(env, cls, "x11win", "J")) == 0)
    THROW("Could not store X window handle");
  (*env)->SetLongField(env, obj, fid, x11dsi->drawable);

  fprintf(stderr, "TurboVNC Helper: %s X11 full-screen mode for window 0x%.8lx\n",
          on ? "Enabling" : "Disabling", x11dsi->drawable);

  if (!(rulesAtom = XInternAtom(x11dsi->display, "_XKB_RULES_NAMES", True)))
    THROW("Could not get Xkb rules atom");
  if (XGetWindowProperty(x11dsi->display, DefaultRootWindow(x11dsi->display),
                         rulesAtom, 0, 1024, False, XA_STRING, &actualType,
                         &actualFormat, &nItems, &bytesAfter,
                         &data) != Success ||
      actualType != XA_STRING || actualFormat != 8 || nItems < 1 || !data)
    THROW("Could not get Xkb rules");
  if (!strcmp((char *)data, "base") || !strcmp((char *)data, "xorg"))
    xkbRules = 0;
  else if (!strcmp((char *)data, "evdev"))
    xkbRules = 1;

  if ((fid = (*env)->GetFieldID(env, cls, "xkbRules", "I")) == 0)
    THROW("Could not store Xkb rules");
  (*env)->SetIntField(env, obj, fid, xkbRules);

  bailout:
  if (data) XFree(data);
  if (ds) {
    if (dsi) ds->FreeDrawingSurfaceInfo(dsi);
    ds->Unlock(ds);
    awt.FreeDrawingSurface(ds);
  }
}


JNIEXPORT void JNICALL Java_com_turbovnc_vncviewer_Viewport_grabKeyboard
  (JNIEnv *env, jobject obj, jboolean on, jboolean pointer)
{
  JAWT awt;
  JAWT_DrawingSurface *ds = NULL;
  JAWT_DrawingSurfaceInfo *dsi = NULL;
  JAWT_X11DrawingSurfaceInfo *x11dsi = NULL;
  int ret;

  awt.version = JAWT_VERSION_1_3;
  if (!handle) {
    if ((handle = dlopen("libjawt.so", RTLD_LAZY)) == NULL)
      THROW(dlerror());
    if ((__JAWT_GetAWT =
         (__JAWT_GetAWT_type)dlsym(handle, "JAWT_GetAWT")) == NULL)
      THROW(dlerror());
  }

  if (__JAWT_GetAWT(env, &awt) == JNI_FALSE)
    THROW("Could not initialize AWT native interface");

  if ((ds = awt.GetDrawingSurface(env, obj)) == NULL)
    THROW("Could not get drawing surface");

  if ((ds->Lock(ds) & JAWT_LOCK_ERROR) != 0)
    THROW("Could not lock surface");

  if ((dsi = ds->GetDrawingSurfaceInfo(ds)) == NULL)
    THROW("Could not get drawing surface info");

  if ((x11dsi = (JAWT_X11DrawingSurfaceInfo *)dsi->platformInfo) == NULL)
    THROW("Could not get X11 drawing surface info");

  XSync(x11dsi->display, False);
  if (on) {
    int count = 5;

    while ((ret = XGrabKeyboard(x11dsi->display, x11dsi->drawable, True,
                                GrabModeAsync, GrabModeAsync,
                                CurrentTime)) != GrabSuccess) {
      switch (ret) {
        case AlreadyGrabbed:
          THROW("Could not grab keyboard: already grabbed by another application");
        case GrabInvalidTime:
          THROW("Could not grab keyboard: invalid time");
        case GrabNotViewable:
          /* The window should theoretically be viewable by now, but in
             practice, sometimes a race condition occurs with Swing.  It is
             unclear why, since everything should be happening in the EDT. */
          if (count == 0)
            THROW("Could not grab keyboard: window not viewable");
          usleep(100000);
          count--;
          continue;
        case GrabFrozen:
          THROW("Could not grab keyboard: keyboard frozen by another application");
      }
    }

    if (pointer) {
      ret = XGrabPointer(x11dsi->display, x11dsi->drawable, True,
                         ButtonPressMask | ButtonReleaseMask |
                           ButtonMotionMask | PointerMotionMask, GrabModeAsync,
                         GrabModeAsync, None, None, CurrentTime);
      switch (ret) {
        case AlreadyGrabbed:
          THROW("Could not grab pointer: already grabbed by another application");
        case GrabInvalidTime:
          THROW("Could not grab pointer: invalid time");
        case GrabNotViewable:
          THROW("Could not grab pointer: window not viewable");
        case GrabFrozen:
          THROW("Could not grab pointer: pointer frozen by another application");
      }
    }

    fprintf(stderr, "TurboVNC Helper: Grabbed keyboard%s for window 0x%.8lx\n",
            pointer ? " & pointer" : "", x11dsi->drawable);
  } else {
    XUngrabKeyboard(x11dsi->display, CurrentTime);
    if (pointer)
      XUngrabPointer(x11dsi->display, CurrentTime);
    fprintf(stderr, "TurboVNC Helper: Ungrabbed keyboard%s\n",
            pointer ? " & pointer" : "");
  }
  XSync(x11dsi->display, False);

  bailout:
  if (ds) {
    if (dsi) ds->FreeDrawingSurfaceInfo(dsi);
    ds->Unlock(ds);
    awt.FreeDrawingSurface(ds);
  }
}


JNIEXPORT void JNICALL Java_com_turbovnc_vncviewer_Viewport_setupExtInput
  (JNIEnv *env, jobject obj, jboolean unused)
{
  jclass cls, eidcls;
  jfieldID fid;
  jmethodID mid;
  Display *dpy = NULL;
  Window win = 0;
  XIDeviceInfo *devInfo = NULL;
  int nDevices = 0, i, ci, nMasks = 0, xiOpcode = 0, dummy1, dummy2;
  XIEventMask masks[100];
  jobject extInputDevice;
  Bool multitouch = False;

  memset(masks, 0, sizeof(XIEventMask) * 100);

  if ((dpy = XOpenDisplay(NULL)) == NULL)
    THROW("Could not open X display");
  if (!XQueryExtension(dpy, "XInputExtension", &xiOpcode, &dummy1, &dummy2))
    THROW("X Input extension not available");

  BAILIF0(cls = (*env)->GetObjectClass(env, obj));
  BAILIF0(fid = (*env)->GetFieldID(env, cls, "x11win", "J"));
  if ((win = (Window)(*env)->GetLongField(env, obj, fid)) == 0)
    THROW("X window handle has not been initialized");

  if ((devInfo = XIQueryDevice(dpy, XIAllDevices, &nDevices)) == NULL)
    THROW("Could not list XI devices");

  for (i = 0; i < nDevices; i++) {
    Atom *props = NULL, propType = 0, atom;
    int pi, nProps = 0, propFormat = 0, maxTouches = 0, maxValuatorNum = 0;
    unsigned long propNItems = 0, propBytesAfter = 0;
    unsigned char *propData = NULL;
    char *name;
    CARD32 canGenerate = 0, productID = 0;

    if (devInfo[i].use != XISlavePointer || !devInfo[i].enabled ||
        devInfo[i].num_classes < 1)
      continue;
    if ((props = XIListProperties(dpy, devInfo[i].deviceid,
                                  &nProps)) == NULL || nProps < 1)
      continue;

    /* We only handle Wacom devices at the moment. */
    for (pi = 0; pi < nProps; pi++) {
      if ((name = XGetAtomName(dpy, props[pi])) == NULL)
        continue;
      if (!strcmp(name, "Wacom Tool Type")) {
        XFree(name);
        break;
      }
      XFree(name);
    }
    if (pi < nProps) {
      /* Determine Wacom device type. */
      if (XIGetProperty(dpy, devInfo[i].deviceid, props[pi], 0, 1000, False,
                        AnyPropertyType, &propType, &propFormat, &propNItems,
                        &propBytesAfter, &propData) != Success ||
          propType != XA_ATOM || propNItems < 1 || !propData) {
        XFree(props);
        continue;
      }
      XFree(props);
      atom = *(Atom *)propData;
      if ((name = XGetAtomName(dpy, atom)) == NULL)
        continue;
      /* TurboVNC-specific:  we use productID to represent the device type, so
         we can recreate it on the server */
      if (!strcmp(name, "CURSOR"))
        productID = rfbGIIDevTypeCursor;
      else if (!strcmp(name, "STYLUS"))
        productID = rfbGIIDevTypeStylus;
      else if (!strcmp(name, "ERASER"))
        productID = rfbGIIDevTypeEraser;
      else if (!strcmp(name, "TOUCH"))
        productID = rfbGIIDevTypeTouch;
      else if (!strcmp(name, "PAD"))
        productID = rfbGIIDevTypePad;
      else {
        XFree(name);
        continue;
      }
      XFree(name);
    } else {
      /* Not a Wacom device.  We can only handle it if it's a touchscreen. */
      XFree(props);
      for (ci = 0; ci < devInfo[i].num_classes; ci++) {
        if (devInfo[i].classes[ci]->type == XITouchClass &&
            (((XITouchClassInfo *)devInfo[i].classes[ci])->mode ==
             XIDirectTouch)) {
          productID = rfbGIIDevTypeTouch;
          multitouch = True;
          break;
        }
      }
      if (ci >= devInfo[i].num_classes)
        continue;
    }

    BAILIF0(eidcls =
            (*env)->FindClass(env, "com/turbovnc/rfb/ExtInputDevice"));
    BAILIF0(extInputDevice = (*env)->AllocObject(env, eidcls));

    SET_STRING(eidcls, extInputDevice, name, devInfo[i].name);
    SET_LONG(eidcls, extInputDevice, vendorID, 4242);
    SET_LONG(eidcls, extInputDevice, productID, productID);
    SET_LONG(eidcls, extInputDevice, id, devInfo[i].deviceid);

    for (ci = 0; ci < devInfo[i].num_classes; ci++) {

      switch (devInfo[i].classes[ci]->type) {

        case XIButtonClass:
        {
          XIButtonClassInfo *bi = (XIButtonClassInfo *)devInfo[i].classes[ci];

          SET_INT(eidcls, extInputDevice, numButtons, bi->num_buttons);
          canGenerate |= rfbGIIButtonPressMask | rfbGIIButtonReleaseMask;
          break;
        }

        case XIValuatorClass:
        {
          XIValuatorClassInfo *vi =
            (XIValuatorClassInfo *)devInfo[i].classes[ci];
          jclass valcls;
          jobject valuator;
          char longName[75], shortName[5];

          BAILIF0(valcls = (*env)->FindClass(env,
                  "com/turbovnc/rfb/ExtInputDevice$Valuator"));

          if (vi->mode == XIModeAbsolute)
            canGenerate |= rfbGIIValuatorAbsoluteMask;
          /* FIXME: Relative valuators aren't supported (yet) */
          else if (vi->mode == XIModeRelative)
            continue;

          BAILIF0(valuator = (*env)->AllocObject(env, valcls));
          SET_INT(valcls, valuator, index, vi->number);
          if (vi->number > maxValuatorNum)
            maxValuatorNum = vi->number;
          name = XGetAtomName(dpy, vi->label);
          if (name) {
            snprintf(longName, 75, "%s", name);
            XFree(name);
          } else
            snprintf(longName, 75, "Valuator %d", vi->number);
          SET_STRING(valcls, valuator, longName, longName);
          snprintf(shortName, 5, "%d", vi->number);
          SET_STRING(valcls, valuator, shortName, shortName);
          SET_INT(valcls, valuator, rangeMin, (int)vi->min);
          SET_INT(valcls, valuator, rangeCenter,
                  ((int)vi->min + (int)vi->max) / 2);
          SET_INT(valcls, valuator, rangeMax, (int)vi->max);
          SET_INT(valcls, valuator, siUnit, rfbGIIUnitLength);
          SET_INT(valcls, valuator, siDiv, vi->resolution);

          BAILIF0(mid = (*env)->GetMethodID(env, eidcls, "addValuator",
                  "(Lcom/turbovnc/rfb/ExtInputDevice$Valuator;)V"));
          (*env)->CallVoidMethod(env, extInputDevice, mid, valuator);
          break;
        }
      }
    }

    for (ci = 0; ci < devInfo[i].num_classes; ci++) {

      switch (devInfo[i].classes[ci]->type) {

        case XITouchClass:
        {
          /* TurboVNC-specific:  we use fake valuators to transmit XI touch
             events */
          jclass valcls;
          jobject valuator;
          char longName[75], shortName[5];
          int numTouches =
            ((XITouchClassInfo *)devInfo[i].classes[ci])->num_touches;

          if (!multitouch) continue;

          if (numTouches > maxTouches)
            maxTouches = numTouches;

          BAILIF0(valcls = (*env)->FindClass(env,
                  "com/turbovnc/rfb/ExtInputDevice$Valuator"));

          canGenerate |= rfbGIIValuatorAbsoluteMask;

          if (maxValuatorNum > 3)
            THROW("Multitouch device has too many valuators")

          BAILIF0(valuator = (*env)->AllocObject(env, valcls));
          SET_INT(valcls, valuator, index, maxValuatorNum + 1);
          snprintf(longName, 75, "__TURBOVNC FAKE TOUCH ID__");
          SET_STRING(valcls, valuator, longName, longName);
          snprintf(shortName, 5, "TFTI");
          SET_STRING(valcls, valuator, shortName, shortName);
          SET_INT(valcls, valuator, rangeMin, 0);
          SET_INT(valcls, valuator, rangeCenter, INT_MAX / 2);
          SET_INT(valcls, valuator, rangeMax, INT_MAX);
          SET_INT(valcls, valuator, siUnit, rfbGIIUnitUnknown);
          SET_INT(valcls, valuator, siDiv, 0);

          BAILIF0(mid = (*env)->GetMethodID(env, eidcls, "addValuator",
                  "(Lcom/turbovnc/rfb/ExtInputDevice$Valuator;)V"));
          (*env)->CallVoidMethod(env, extInputDevice, mid, valuator);

          BAILIF0(valuator = (*env)->AllocObject(env, valcls));
          SET_INT(valcls, valuator, index, maxValuatorNum + 2);
          snprintf(longName, 75, "__TURBOVNC FAKE TOUCH TYPE__");
          SET_STRING(valcls, valuator, longName, longName);
          snprintf(shortName, 5, "TFTT");
          SET_STRING(valcls, valuator, shortName, shortName);
          SET_INT(valcls, valuator, rangeMin, 0);
          SET_INT(valcls, valuator, rangeCenter, 2);
          SET_INT(valcls, valuator, rangeMax, 5);
          SET_INT(valcls, valuator, siUnit, rfbGIIUnitUnknown);
          SET_INT(valcls, valuator, siDiv, 0);

          BAILIF0(mid = (*env)->GetMethodID(env, eidcls, "addValuator",
                  "(Lcom/turbovnc/rfb/ExtInputDevice$Valuator;)V"));
          (*env)->CallVoidMethod(env, extInputDevice, mid, valuator);

          break;
        }
      }
    }

    if (canGenerate && nMasks < 100) {
      masks[nMasks].deviceid = devInfo[i].deviceid;
      masks[nMasks].mask_len = XIMaskLen(XI_LASTEVENT);
      masks[nMasks].mask =
        (unsigned char *)calloc(masks[nMasks].mask_len, sizeof(unsigned char));
      if (canGenerate & rfbGIIButtonPressMask)
        XISetMask(masks[nMasks].mask, XI_ButtonPress);
      if (canGenerate & rfbGIIButtonReleaseMask)
        XISetMask(masks[nMasks].mask, XI_ButtonRelease);
      if (canGenerate & rfbGIIValuatorAbsoluteMask ||
          canGenerate & rfbGIIValuatorRelativeMask) {
        XISetMask(masks[nMasks].mask, XI_Motion);
        if (multitouch) {
          XISetMask(masks[nMasks].mask, XI_TouchBegin);
          XISetMask(masks[nMasks].mask, XI_TouchUpdate);
          XISetMask(masks[nMasks].mask, XI_TouchEnd);
        }
      }
      nMasks++;
    }

    SET_LONG(eidcls, extInputDevice, canGenerate, canGenerate);
    if (canGenerate & rfbGIIValuatorAbsoluteMask)
      SET_BOOL(eidcls, extInputDevice, absolute, 1);
    SET_INT(eidcls, extInputDevice, numRegisters, maxTouches);

    BAILIF0(mid = (*env)->GetMethodID(env, cls, "addInputDevice",
            "(Lcom/turbovnc/rfb/ExtInputDevice;)V"));
    (*env)->CallVoidMethod(env, obj, mid, extInputDevice);
  }

  XIFreeDeviceInfo(devInfo);  devInfo = NULL;
  if (nMasks == 0) {
    fprintf(stderr, "No extended input devices.\n");
    goto bailout;
  }

  if (XISelectEvents(dpy, win, masks, nMasks))
    THROW("Could not select XI events");
  XSync(dpy, False);

  SET_INT(cls, obj, buttonPressType, XI_ButtonPress);
  SET_INT(cls, obj, buttonReleaseType, XI_ButtonRelease);
  SET_INT(cls, obj, motionType, XI_Motion);
  SET_LONG(cls, obj, x11dpy, (jlong)(intptr_t)dpy);
  if (multitouch)
    SET_BOOL(cls, obj, multitouch, 1);

  fprintf(stderr, "TurboVNC Helper: Listening for XInput events on %s (window 0x%.8x)\n",
          DisplayString(dpy), (unsigned int)win);

  bailout:
  if (nMasks) {
    for (i = 0; i < nMasks; i++)
      free(masks[i].mask);
  }
  if (devInfo) XIFreeDeviceInfo(devInfo);
}


static Bool IsXIEvent(Display *dpy, XEvent *xe, XPointer arg)
{
  struct isXIEventParams *params = (struct isXIEventParams *)arg;
  Bool freeXEventData = False, touchEmulatingPointer = False;
  XIDeviceEvent *xide;

  if (xe->type != GenericEvent || xe->xgeneric.extension != params->xiOpcode ||
      xe->xcookie.type != GenericEvent ||
      xe->xcookie.extension != params->xiOpcode)
    return False;

  /* The TurboVNC Viewer calls the native Viewport.processExtInputEvent()
     method with the 'type' argument (--> params->xiType) set to
     XI_ButtonPress, XI_ButtonRelease, or XI_Motion, corresponding to the type
     of mouse event that the Swing mouse listener received.  For non-multitouch
     events, this predicate function selects events from the queue only if they
     match the specified event type exactly. */
  if (xe->xcookie.evtype < XI_TouchBegin || xe->xcookie.evtype > XI_TouchEnd)
    return (xe->xcookie.evtype == params->xiType);

  if (!xe->xcookie.data) {
    if (!XGetEventData(dpy, &xe->xcookie))
      return False;
    freeXEventData = True;
  }
  xide = (XIDeviceEvent *)xe->xcookie.data;
  touchEmulatingPointer = !!(xide->flags & XITouchEmulatingPointer);
  if (freeXEventData) XFreeEventData(dpy, &xe->xcookie);

  if (params->xiType == -1) {
    /* This predicate function is being called by the multitouch listener
       thread, which only handles multitouch events for the second and
       subsequent touches.  (Only the events for the first touch have pointer
       emulation enabled.) */
    return !touchEmulatingPointer;
  } else {
    /* Multitouch events for the first touch have corresponding Swing mouse
       events, so we handle them similarly to non-multitouch events. */
    if (!touchEmulatingPointer)
      return False;
    if (params->xiType == XI_ButtonPress &&
        xe->xcookie.evtype != XI_TouchBegin)
      return False;
    if (params->xiType == XI_ButtonRelease &&
        xe->xcookie.evtype != XI_TouchEnd)
      return False;
    if (params->xiType == XI_Motion &&
        xe->xcookie.evtype != XI_TouchUpdate)
      return False;
  }

  return True;
}

JNIEXPORT jboolean JNICALL Java_com_turbovnc_vncviewer_Viewport_processExtInputEvent
  (JNIEnv *env, jobject obj, jint type)
{
  jclass cls;
  jfieldID fid;
  Display *dpy;
  jboolean retval = JNI_FALSE;
  int dummy1, dummy2, i;
  XEvent xe;
  Bool freeXEventData = False, monitorEntered = False;
  struct isXIEventParams params;

  if ((*env)->MonitorEnter(env, obj))
    THROW("Could not enter monitor");
  monitorEntered = True;

  BAILIF0(cls = (*env)->GetObjectClass(env, obj));
  BAILIF0(fid = (*env)->GetFieldID(env, cls, "x11dpy", "J"));
  BAILIF0(dpy = (Display *)(intptr_t)(*env)->GetLongField(env, obj, fid));

  if (!XQueryExtension(dpy, "XInputExtension", &params.xiOpcode, &dummy1,
                       &dummy2))
    THROW("X Input extension not available");

  params.xiType = type;
  while (XCheckIfEvent(dpy, &xe, IsXIEvent, (XPointer)&params)) {
    jclass eventcls;  jobject event, jvaluators;
    jint valuators[6];
    XIDeviceEvent *xide;
    long buttonMask = 0;
    int giiEventType = 0, touchType = -1, numValuators = 0, firstValuator = -1;

    if (!XGetEventData(dpy, &xe.xcookie))
      continue;
    freeXEventData = True;
    xide = (XIDeviceEvent *)xe.xcookie.data;

    BAILIF0(eventcls =
            (*env)->FindClass(env, "com/turbovnc/rfb/ExtInputEvent"));
    BAILIF0(fid = (*env)->GetFieldID(env, cls, "lastEvent",
                                     "Lcom/turbovnc/rfb/ExtInputEvent;"));
    BAILIF0(event = (*env)->GetObjectField(env, obj, fid));
    type = xe.xcookie.evtype;
    switch (type) {
      case XI_ButtonPress:  giiEventType = rfbGIIButtonPress;  break;
      case XI_ButtonRelease:  giiEventType = rfbGIIButtonRelease;  break;
      /* NOTE: If the event type is rfbGIIValuatorRelative, then the Java code
         should change it to rfbGIIValuatorAbsolute.  We set the event type to
         rbGIIValuatorRelative here in order to verify that. */
      case XI_Motion:  giiEventType = rfbGIIValuatorRelative;  break;
      case XI_TouchBegin:
      case XI_TouchEnd:
      case XI_TouchUpdate:
        giiEventType = rfbGIIValuatorRelative;
        touchType = rfbGIITouchBegin + (type - XI_TouchBegin);
        if (xide->flags & XITouchEmulatingPointer)
          touchType += 3;
        break;
    }
    SET_INT(eventcls, event, type, giiEventType);
    SET_LONG(eventcls, event, deviceID, xide->deviceid);
    for (i = 0; i < xide->buttons.mask_len * 8; i++)
      if (XIMaskIsSet(xide->buttons.mask, i))
        buttonMask |= (1 << (i + 7));
    SET_LONG(eventcls, event, buttonMask, buttonMask);
    for (i = 0; i < xide->valuators.mask_len * 8; i++) {
      if (XIMaskIsSet(xide->valuators.mask, i) && numValuators < 6) {
        if (firstValuator < 0) firstValuator = i;
        valuators[numValuators] = (jint)xide->valuators.values[i];
        numValuators++;
      }
    }
    if (touchType >= 0) {
      valuators[numValuators++] = (jint)xide->detail;
      valuators[numValuators++] = (jint)touchType;
      SET_INT(eventcls, event, buttonNumber, 1);
    }
    SET_INT(eventcls, event, numValuators, numValuators);
    SET_INT(eventcls, event, firstValuator, firstValuator);
    if (xe.xcookie.evtype == XI_ButtonPress ||
        xe.xcookie.evtype == XI_ButtonRelease)
      SET_INT(eventcls, event, buttonNumber, xide->detail);

    BAILIF0(fid = (*env)->GetFieldID(env, eventcls, "valuators", "[I"));
    BAILIF0(jvaluators = (jintArray)(*env)->GetObjectField(env, event, fid));
    (*env)->SetIntArrayRegion(env, jvaluators, 0, numValuators, valuators);
    if (freeXEventData) {
      XFreeEventData(dpy, &xe.xcookie);
      freeXEventData = False;
    }
    if (xe.xcookie.evtype == XI_ButtonPress ||
        xe.xcookie.evtype == XI_ButtonRelease ||
        xe.xcookie.evtype == XI_TouchBegin ||
        xe.xcookie.evtype == XI_TouchEnd) {
      (*env)->MonitorExit(env, obj);
      return JNI_TRUE;
    } else
      retval = JNI_TRUE;
  }

  bailout:
  if (freeXEventData) XFreeEventData(dpy, &xe.xcookie);
  if (monitorEntered) (*env)->MonitorExit(env, obj);
  return retval;
}


JNIEXPORT void JNICALL Java_com_turbovnc_vncviewer_Viewport_cleanupExtInput
  (JNIEnv *env, jobject obj)
{
  jclass cls;
  jfieldID fid;
  Display *dpy;

  BAILIF0(cls = (*env)->GetObjectClass(env, obj));
  BAILIF0(fid = (*env)->GetFieldID(env, cls, "x11dpy", "J"));
  dpy = (Display *)(intptr_t)(*env)->GetLongField(env, obj, fid);
  if (dpy) {
    fprintf(stderr, "TurboVNC Helper: Shutting down XInput listener on display %s\n",
            DisplayString(dpy));
    XCloseDisplay(dpy);
    SET_LONG(cls, obj, x11dpy, 0);
  }

  bailout:
  return;
}
