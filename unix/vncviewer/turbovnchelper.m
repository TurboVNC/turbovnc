/*  Copyright (C)2015-2017, 2019 D. R. Commander.  All Rights Reserved.
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

/* Contains code from JNITablet (http://www.jhlabs.com/java/tablet):
 *
 * Copyright 2006 Jerry Huxtable
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS",
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include "com_turbovnc_vncviewer_Viewport.h"
#include <Cocoa/Cocoa.h>
#include "JRSwizzle.h"
#import <objc/objc-class.h>


#define bailif0(f) {  \
  if (!(f) || (*env)->ExceptionCheck(env)) {  \
    goto bailout;  \
  }  \
}

#define SET_LONG(cls, obj, fieldName, val) {  \
  bailif0(fid = (*env)->GetFieldID(env, cls, #fieldName, "J"));  \
  (*env)->SetLongField(env, obj, fid, val);  \
}


static JavaVM *g_jvm = NULL;
static jobject g_object = NULL;
static jmethodID g_methodID = NULL, g_methodID_prox = NULL;
IMP origSendEvent = NULL;


static jint GetJNIEnv(JNIEnv **env, bool *mustDetach)
{
  jint getEnvErr = JNI_OK;

  *mustDetach = false;
  if (g_jvm) {
    getEnvErr = (*g_jvm)->GetEnv(g_jvm, (void **)env, JNI_VERSION_1_4);
    if (getEnvErr == JNI_EDETACHED) {
      getEnvErr = (*g_jvm)->AttachCurrentThread(g_jvm, (void **)env, NULL);
      if (getEnvErr == JNI_OK)
        *mustDetach = true;
    }
  }

  return getEnvErr;
}


@interface NSApplication(TabletEvents)
@end

@implementation NSApplication(TabletEvents)

- (void)newSendEvent:(NSEvent *)event
{
  JNIEnv *env;
  bool shouldDetach = false, success = true;

  switch ([event type]) {
    case NSTabletProximity:
      if (!g_object || !g_methodID_prox) break;

      if (GetJNIEnv(&env, &shouldDetach) != JNI_OK) {
        NSLog(@"Couldn't attach to JVM");
        return;
      }
      (*env)->CallVoidMethod(env, g_object, g_methodID_prox,
                             [event isEnteringProximity],
                             [event pointingDeviceType], [event window]);
      if (shouldDetach)
        (*g_jvm)->DetachCurrentThread(g_jvm);
      return;

    case NSLeftMouseDown:      // 1
    case NSLeftMouseUp:        // 2
    case NSRightMouseDown:     // 3
    case NSRightMouseUp:       // 4
    case NSMouseMoved:         // 5
    case NSLeftMouseDragged:   // 6
    case NSRightMouseDragged:  // 7
    case NSOtherMouseDown:     // 25
    case NSOtherMouseUp:       // 26
    case NSOtherMouseDragged:  // 27
      if (!g_object || !g_methodID) break;

      if ([event subtype] != NSTabletPointEventSubtype &&
          [event type] <= NSRightMouseDragged)
        break;
      if (GetJNIEnv(&env, &shouldDetach) != JNI_OK) {
        NSLog(@"Couldn't attach to JVM");
        return;
      }
      NSPoint tilt = [event tilt];
      NSPoint location = [event locationInWindow];
      if ([event window] == NULL)
        break;
      success = (*env)->CallBooleanMethod(env, g_object, g_methodID,
                                          [event type], location.x, location.y,
                                          [event pressure], tilt.x, tilt.y,
                                          [event window]);
      if (shouldDetach)
        (*g_jvm)->DetachCurrentThread(g_jvm);
      if (success) return;

    default:
      break;
  }
  [self newSendEvent: event];
}

@end


JNIEXPORT void JNICALL Java_com_turbovnc_vncviewer_Viewport_setupExtInput
  (JNIEnv *env, jobject obj)
{
  jclass cls;
  jfieldID fid;
  NSError *error = NULL;

  if (!g_jvm) {
    origSendEvent =
      class_getMethodImplementation(objc_getClass("NSApplication"),
                                    @selector(sendEvent:));
    if ((*env)->GetJavaVM(env, &g_jvm) || (*env)->ExceptionCheck(env))
      goto bailout;
  }

  if (class_getMethodImplementation(objc_getClass("NSApplication"),
                                    @selector(sendEvent:)) == origSendEvent) {
    [NSApplication jr_swizzleMethod:@selector(sendEvent:)
                   withMethod:@selector(newSendEvent:) error:&error];
    if (error) {
      NSLog(@"%@", [error localizedDescription]);
      return;
    }
  }

  if (g_object) {
    (*env)->DeleteGlobalRef(env, g_object);  g_object = NULL;
  }
  bailif0(g_object = (*env)->NewGlobalRef(env, obj));

  bailif0(cls = (*env)->GetObjectClass(env, obj));

  bailif0(g_methodID = (*env)->GetMethodID(env, cls,
                                           "handleTabletEvent",
                                           "(IDDFFFJ)Z"));
  bailif0(g_methodID_prox = (*env)->GetMethodID(env, cls,
                                                "handleTabletProximityEvent",
                                                "(ZIJ)V"));

  SET_LONG(cls, obj, x11dpy, 1);
  SET_LONG(cls, obj, x11win,
           (jlong)[[NSApplication sharedApplication] keyWindow]);

  printf("TurboVNC Helper: Intercepting tablet events for window 0x%.8lx\n",
         (unsigned long)[[NSApplication sharedApplication] keyWindow]);
  return;

  bailout:
  if (g_object) {
    (*env)->DeleteGlobalRef(env, g_object);  g_object = NULL;
  }
}


JNIEXPORT void JNICALL Java_com_turbovnc_vncviewer_Viewport_cleanupExtInput
  (JNIEnv *env, jobject obj)
{
  jclass cls;
  jfieldID fid;
  NSError *error = NULL;

  if (class_getMethodImplementation(objc_getClass("NSApplication"),
                                    @selector(sendEvent:)) != origSendEvent) {
    [NSApplication jr_swizzleMethod:@selector(sendEvent:)
                   withMethod:@selector(newSendEvent:) error:&error];
    if (error) {
      NSLog(@"%@", [error localizedDescription]);
      return;
    }
  }

  if (g_object) {
    (*env)->DeleteGlobalRef(env, g_object);  g_object = NULL;
  }
  g_methodID = g_methodID_prox = NULL;
  g_jvm = NULL;

  bailif0(cls = (*env)->GetObjectClass(env, obj));
  SET_LONG(cls, obj, x11dpy, 0);

  printf("TurboVNC Helper: Shutting down tablet event interceptor\n");

  bailout:
  return;
}
