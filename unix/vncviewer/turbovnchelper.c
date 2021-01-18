/*  Copyright (C)2015-2019, 2021 D. R. Commander.  All Rights Reserved.
 *
 *  This is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this software; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 *  USA.
 */

#ifdef __SUNPRO_C
/* Oracle Developer Studio sometimes erroneously detects the _throw() macro
   followed by a semicolon as an unreachable statement. */
#pragma error_messages(off, E_STATEMENT_NOT_REACHED)
#endif

#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include "jawt_md.h"
#include <X11/extensions/XInput2.h>
#include <X11/Xatom.h>
#include "com_turbovnc_vncviewer_Viewport.h"
#include <X11/Xmd.h>
#include "rfbproto.h"
#include "turbovnc_devtypes.h"


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


#define _throw(msg) {  \
  jclass _exccls = (*env)->FindClass(env, "java/lang/Exception");  \
  if (!_exccls) goto bailout;  \
  (*env)->ThrowNew(env, _exccls, msg);  \
  goto bailout;  \
}

#define bailif0(f) {  \
  if (!(f) || (*env)->ExceptionCheck(env)) {  \
    goto bailout;  \
  }  \
}

typedef jboolean JNICALL (*__JAWT_GetAWT_type) (JNIEnv *env, JAWT *awt);
static __JAWT_GetAWT_type __JAWT_GetAWT = NULL;

static void *handle = NULL;


JNIEXPORT void JNICALL Java_com_turbovnc_vncviewer_Viewport_x11FullScreen
  (JNIEnv *env, jobject obj, jboolean on)
{
  JAWT awt;
  JAWT_DrawingSurface *ds = NULL;
  JAWT_DrawingSurfaceInfo *dsi = NULL;
  JAWT_X11DrawingSurfaceInfo *x11dsi = NULL;
  jfieldID fid;
  jclass cls;

  awt.version = JAWT_VERSION_1_3;
  if (!handle) {
    if ((handle = dlopen("libjawt.so", RTLD_LAZY)) == NULL)
      _throw(dlerror());
    if ((__JAWT_GetAWT =
         (__JAWT_GetAWT_type)dlsym(handle, "JAWT_GetAWT")) == NULL)
      _throw(dlerror());
  }

  if (__JAWT_GetAWT(env, &awt) == JNI_FALSE)
    _throw("Could not initialize AWT native interface");

  if ((ds = awt.GetDrawingSurface(env, obj)) == NULL)
    _throw("Could not get drawing surface");

  if ((ds->Lock(ds) & JAWT_LOCK_ERROR) != 0)
    _throw("Could not lock surface");

  if ((dsi = ds->GetDrawingSurfaceInfo(ds)) == NULL)
    _throw("Could not get drawing surface info");

  if ((x11dsi = (JAWT_X11DrawingSurfaceInfo *)dsi->platformInfo) == NULL)
    _throw("Could not get X11 drawing surface info");

  bailif0(cls = (*env)->GetObjectClass(env, obj));

  netwm_fullscreen(x11dsi->display, x11dsi->drawable, on);
  if (on) {
    XEvent e;
    jint leftMon, rightMon, topMon, bottomMon;

    bailif0(fid = (*env)->GetFieldID(env, cls, "leftMon", "I"));
    leftMon = (*env)->GetIntField(env, obj, fid);
    bailif0(fid = (*env)->GetFieldID(env, cls, "rightMon", "I"));
    rightMon = (*env)->GetIntField(env, obj, fid);
    bailif0(fid = (*env)->GetFieldID(env, cls, "topMon", "I"));
    topMon = (*env)->GetIntField(env, obj, fid);
    bailif0(fid = (*env)->GetFieldID(env, cls, "bottomMon", "I"));
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
    _throw("Could not store X window handle");
  (*env)->SetLongField(env, obj, fid, x11dsi->drawable);

  printf("TurboVNC Helper: %s X11 full-screen mode for window 0x%.8lx\n",
         on ? "Enabling" : "Disabling", x11dsi->drawable);

  bailout:
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
      _throw(dlerror());
    if ((__JAWT_GetAWT =
         (__JAWT_GetAWT_type)dlsym(handle, "JAWT_GetAWT")) == NULL)
      _throw(dlerror());
  }

  if (__JAWT_GetAWT(env, &awt) == JNI_FALSE)
    _throw("Could not initialize AWT native interface");

  if ((ds = awt.GetDrawingSurface(env, obj)) == NULL)
    _throw("Could not get drawing surface");

  if ((ds->Lock(ds) & JAWT_LOCK_ERROR) != 0)
    _throw("Could not lock surface");

  if ((dsi = ds->GetDrawingSurfaceInfo(ds)) == NULL)
    _throw("Could not get drawing surface info");

  if ((x11dsi = (JAWT_X11DrawingSurfaceInfo *)dsi->platformInfo) == NULL)
    _throw("Could not get X11 drawing surface info");

  XSync(x11dsi->display, False);
  if (on) {
    int count = 5;

    while ((ret = XGrabKeyboard(x11dsi->display, x11dsi->drawable, True,
                                GrabModeAsync, GrabModeAsync,
                                CurrentTime)) != GrabSuccess) {
      switch (ret) {
        case AlreadyGrabbed:
          _throw("Could not grab keyboard: already grabbed by another application");
        case GrabInvalidTime:
          _throw("Could not grab keyboard: invalid time");
        case GrabNotViewable:
          /* The window should theoretically be viewable by now, but in
             practice, sometimes a race condition occurs with Swing.  It is
             unclear why, since everything should be happening in the EDT. */
          if (count == 0)
            _throw("Could not grab keyboard: window not viewable");
          usleep(100000);
          count--;
          continue;
        case GrabFrozen:
          _throw("Could not grab keyboard: keyboard frozen by another application");
      }
    }

    if (pointer) {
      ret = XGrabPointer(x11dsi->display, x11dsi->drawable, True,
                         ButtonPressMask | ButtonReleaseMask |
                           ButtonMotionMask | PointerMotionMask, GrabModeAsync,
                         GrabModeAsync, None, None, CurrentTime);
      switch (ret) {
        case AlreadyGrabbed:
          _throw("Could not grab pointer: already grabbed by another application");
        case GrabInvalidTime:
          _throw("Could not grab pointer: invalid time");
        case GrabNotViewable:
          _throw("Could not grab pointer: window not viewable");
        case GrabFrozen:
          _throw("Could not grab pointer: pointer frozen by another application");
      }
    }

    printf("TurboVNC Helper: Grabbed keyboard%s for window 0x%.8lx\n",
           pointer ? " & pointer" : "", x11dsi->drawable);
  } else {
    XUngrabKeyboard(x11dsi->display, CurrentTime);
    if (pointer)
      XUngrabPointer(x11dsi->display, CurrentTime);
    printf("TurboVNC Helper: Ungrabbed keyboard%s\n",
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


#define SET_STRING(cls, obj, fieldName, string) {  \
  jstring str;  \
  bailif0(fid = (*env)->GetFieldID(env, cls, #fieldName,  \
                                   "Ljava/lang/String;"));  \
  bailif0(str = (*env)->NewStringUTF(env, string));  \
  (*env)->SetObjectField(env, obj, fid, (jobject)str);  \
}

#define SET_LONG(cls, obj, fieldName, val) {  \
  bailif0(fid = (*env)->GetFieldID(env, cls, #fieldName, "J"));  \
  (*env)->SetLongField(env, obj, fid, val);  \
}

#define SET_INT(cls, obj, fieldName, val) {  \
  bailif0(fid = (*env)->GetFieldID(env, cls, #fieldName, "I"));  \
  (*env)->SetIntField(env, obj, fid, val);  \
}

#define SET_BOOL(cls, obj, fieldName, val) {  \
  bailif0(fid = (*env)->GetFieldID(env, cls, #fieldName, "Z"));  \
  (*env)->SetBooleanField(env, obj, fid, val);  \
}


JNIEXPORT void JNICALL Java_com_turbovnc_vncviewer_Viewport_setupExtInput
  (JNIEnv *env, jobject obj)
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

  memset(masks, 0, sizeof(XIEventMask) * 100);

  if ((dpy = XOpenDisplay(NULL)) == NULL)
    _throw("Could not open X display");
  if (!XQueryExtension(dpy, "XInputExtension", &xiOpcode, &dummy1, &dummy2))
    _throw("X Input extension not available");

  bailif0(cls = (*env)->GetObjectClass(env, obj));
  bailif0(fid = (*env)->GetFieldID(env, cls, "x11win", "J"));
  if ((win = (Window)(*env)->GetLongField(env, obj, fid)) == 0)
    _throw("X window handle has not been initialized");

  if ((devInfo = XIQueryDevice(dpy, XIAllDevices, &nDevices)) == NULL)
    _throw("Could not list XI devices");

  for (i = 0; i < nDevices; i++) {
    Atom *props = NULL, propType = 0, atom;
    int pi, nProps = 0, propFormat = 0;
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
    if (pi >= nProps) {
      XFree(props);
      continue;
    }

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

    /* FIXME: Relative valuators aren't supported (yet) */
    for (ci = 0; ci < devInfo[i].num_classes; ci++) {
      if (devInfo[i].classes[ci]->type == XIValuatorClass &&
          (((XIValuatorClassInfo *)devInfo[i].classes[ci])->mode ==
           XIModeRelative))
        break;
    }
    if (ci < devInfo[i].num_classes)
      continue;

    bailif0(eidcls =
            (*env)->FindClass(env, "com/turbovnc/rfb/ExtInputDevice"));
    bailif0(extInputDevice = (*env)->AllocObject(env, eidcls));

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

          bailif0(valcls = (*env)->FindClass(env,
                  "com/turbovnc/rfb/ExtInputDevice$Valuator"));

          if (vi->mode == XIModeAbsolute)
            canGenerate |= rfbGIIValuatorAbsoluteMask;
          else if (vi->mode == XIModeRelative)
            canGenerate |= rfbGIIValuatorRelativeMask;

          bailif0(valuator = (*env)->AllocObject(env, valcls));
          SET_INT(valcls, valuator, index, vi->number);
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

          bailif0(mid = (*env)->GetMethodID(env, eidcls, "addValuator",
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
          canGenerate & rfbGIIValuatorRelativeMask)
        XISetMask(masks[nMasks].mask, XI_Motion);
      nMasks++;
    }

    SET_LONG(eidcls, extInputDevice, canGenerate, canGenerate);
    if (canGenerate & rfbGIIValuatorAbsoluteMask)
      SET_BOOL(eidcls, extInputDevice, absolute, 1);

    bailif0(mid = (*env)->GetMethodID(env, cls, "addInputDevice",
            "(Lcom/turbovnc/rfb/ExtInputDevice;)V"));
    (*env)->CallVoidMethod(env, obj, mid, extInputDevice);
  }

  XIFreeDeviceInfo(devInfo);  devInfo = NULL;
  if (nMasks == 0) {
    printf("No extended input devices.\n");
    goto bailout;
  }

  if (XISelectEvents(dpy, win, masks, nMasks))
    _throw("Could not select XI events");
  XSync(dpy, False);

  SET_INT(cls, obj, buttonPressType, XI_ButtonPress);
  SET_INT(cls, obj, buttonReleaseType, XI_ButtonRelease);
  SET_INT(cls, obj, motionType, XI_Motion);
  SET_LONG(cls, obj, x11dpy, (jlong)(intptr_t)dpy);

  printf("TurboVNC Helper: Listening for XInput events on %s (window 0x%.8x)\n",
         DisplayString(dpy), (unsigned int)win);

  bailout:
  if (nMasks) {
    for (i = 0; i < nMasks; i++)
      free(masks[i].mask);
  }
  if (devInfo) XIFreeDeviceInfo(devInfo);
}


struct isXIEventParams {
  int xiType, xiOpcode;
};

static Bool IsXIEvent(Display *dpy, XEvent *xe, XPointer arg)
{
  struct isXIEventParams *params = (struct isXIEventParams *)arg;

  if (xe->type == GenericEvent && xe->xgeneric.extension == params->xiOpcode &&
      xe->xcookie.type == GenericEvent &&
      xe->xcookie.extension == params->xiOpcode &&
      xe->xcookie.evtype == params->xiType)
    return True;
  return False;
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
  Bool freeXEventData = False;
  struct isXIEventParams params;

  bailif0(cls = (*env)->GetObjectClass(env, obj));
  bailif0(fid = (*env)->GetFieldID(env, cls, "x11dpy", "J"));
  bailif0(dpy = (Display *)(intptr_t)(*env)->GetLongField(env, obj, fid));

  if (!XQueryExtension(dpy, "XInputExtension", &params.xiOpcode, &dummy1,
                       &dummy2))
    _throw("X Input extension not available");

  params.xiType = type;
  while (XCheckIfEvent(dpy, &xe, IsXIEvent, (XPointer)&params)) {
    jclass eventcls;  jobject event, jvaluators;
    jint valuators[6];
    XIDeviceEvent *xide;
    long buttonMask = 0;
    int giiEventType = 0, numValuators = 0, firstValuator = -1;

    if (!XGetEventData(dpy, &xe.xcookie))
      continue;
    freeXEventData = True;
    xide = (XIDeviceEvent *)xe.xcookie.data;

    bailif0(eventcls =
    (*env)->FindClass(env, "com/turbovnc/rfb/ExtInputEvent"));
    bailif0(fid = (*env)->GetFieldID(env, cls, "lastEvent",
                                     "Lcom/turbovnc/rfb/ExtInputEvent;"));
    bailif0(event = (*env)->GetObjectField(env, obj, fid));
    switch (type) {
      case XI_ButtonPress:  giiEventType = rfbGIIButtonPress;  break;
      case XI_ButtonRelease:  giiEventType = rfbGIIButtonRelease;  break;
      case XI_Motion:  giiEventType = rfbGIIValuatorRelative;  break;
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
    SET_INT(eventcls, event, numValuators, numValuators);
    SET_INT(eventcls, event, firstValuator, firstValuator);
    if (xe.xcookie.evtype == XI_ButtonPress ||
        xe.xcookie.evtype == XI_ButtonRelease)
      SET_INT(eventcls, event, buttonNumber, xide->detail);

    bailif0(fid = (*env)->GetFieldID(env, eventcls, "valuators", "[I"));
    bailif0(jvaluators = (jintArray)(*env)->GetObjectField(env, event, fid));
    (*env)->SetIntArrayRegion(env, jvaluators, 0, numValuators, valuators);
    if (freeXEventData) {
      XFreeEventData(dpy, &xe.xcookie);
      freeXEventData = False;
    }
    if (xe.xcookie.evtype == XI_ButtonPress ||
        xe.xcookie.evtype == XI_ButtonRelease)
      return JNI_TRUE;
    else retval = JNI_TRUE;
  }

  bailout:
  if (freeXEventData) XFreeEventData(dpy, &xe.xcookie);
  return retval;
}


JNIEXPORT void JNICALL Java_com_turbovnc_vncviewer_Viewport_cleanupExtInput
  (JNIEnv *env, jobject obj)
{
  jclass cls;
  jfieldID fid;
  Display *dpy;

  bailif0(cls = (*env)->GetObjectClass(env, obj));
  bailif0(fid = (*env)->GetFieldID(env, cls, "x11dpy", "J"));
  bailif0(dpy = (Display *)(intptr_t)(*env)->GetLongField(env, obj, fid));
  printf("TurboVNC Helper: Shutting down XInput listener on display %s\n",
         DisplayString(dpy));
  XCloseDisplay(dpy);

  bailout:
  return;
}
