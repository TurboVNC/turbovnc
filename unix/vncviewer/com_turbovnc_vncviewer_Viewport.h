#include <jni.h>
/* Header for class com_turbovnc_vncviewer_Viewport */

#ifndef _Included_com_turbovnc_vncviewer_Viewport
#define _Included_com_turbovnc_vncviewer_Viewport
#ifdef __cplusplus
extern "C" {
#endif

/*
 * Class:     com_turbovnc_vncviewer_Viewport
 * Method:    x11FullScreen
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_com_turbovnc_vncviewer_Viewport_x11FullScreen
  (JNIEnv *, jobject, jboolean);

/*
 * Class:     com_turbovnc_vncviewer_Viewport
 * Method:    grabKeyboard
 * Signature: (ZZ)V
 */
JNIEXPORT void JNICALL Java_com_turbovnc_vncviewer_Viewport_grabKeyboard
  (JNIEnv *, jobject, jboolean, jboolean);

/*
 * Class:     com_turbovnc_vncviewer_Viewport
 * Method:    extInputEventLoop
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_turbovnc_vncviewer_Viewport_extInputEventLoop
  (JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif
