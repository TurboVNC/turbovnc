/*  Copyright (C)2015-2017 D. R. Commander.  All Rights Reserved.
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

#include <string.h>
#include "LowLevelHook.h"
#include "jawt_md.h"
#include "com_turbovnc_vncviewer_Viewport.h"


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
  switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
      LowLevelHook::Initialize(hinstDLL);
      break;
    case DLL_PROCESS_DETACH:
      LowLevelHook::Release();
      break;
  }
  return TRUE;
}


#define _throw(msg) {  \
  jclass _exccls=env->FindClass("java/lang/Exception");  \
  if (!_exccls) goto bailout;  \
  env->ThrowNew(_exccls, msg);  \
  goto bailout;  \
}

#define _throww32() {  \
  char message[256];  \
  if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),  \
                     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message,  \
                     256, NULL))  \
    strncpy_s(message, _countof(message), "Error in FormatMessage()",  \
              _TRUNCATE);  \
  _throw(message);  \
}


typedef jboolean (JNICALL *__JAWT_GetAWT_type)(JNIEnv *env, JAWT *awt);
static __JAWT_GetAWT_type __JAWT_GetAWT = NULL;

static HMODULE handle = NULL;


JNIEXPORT void JNICALL Java_com_turbovnc_vncviewer_Viewport_grabKeyboard
  (JNIEnv *env, jobject obj, jboolean on, jboolean pointer)
{
  JAWT awt;
  JAWT_DrawingSurface *ds = NULL;
  JAWT_DrawingSurfaceInfo *dsi = NULL;
  JAWT_Win32DrawingSurfaceInfo *w32dsi = NULL;

  if (on) {
    awt.version = JAWT_VERSION_1_3;
    if (!handle) {
      if ((handle = LoadLibrary("jawt")) == NULL)
        _throww32();
      if ((__JAWT_GetAWT =
#ifdef _WIN64
           (__JAWT_GetAWT_type)GetProcAddress(handle, "JAWT_GetAWT")) == NULL)
#else
           (__JAWT_GetAWT_type)GetProcAddress(handle, "_JAWT_GetAWT@8")) == NULL)
#endif
        _throww32();
    }

    if (__JAWT_GetAWT(env, &awt) == JNI_FALSE)
      _throw("Could not initialize AWT native interface");

    if ((ds = awt.GetDrawingSurface(env, obj)) == NULL)
      _throw("Could not get drawing surface");

    if ((ds->Lock(ds) & JAWT_LOCK_ERROR) != 0)
      _throw("Could not lock surface");

    if ((dsi = ds->GetDrawingSurfaceInfo(ds)) == NULL)
      _throw("Could not get drawing surface info");

    if ((w32dsi = (JAWT_Win32DrawingSurfaceInfo *)dsi->platformInfo) == NULL)
      _throw("Could not get Win32 drawing surface info");

    LowLevelHook::Activate(w32dsi->hwnd);
    printf("TurboVNC Helper: Grabbed keyboard for window 0x%.8llx\n",
           (unsigned long long)w32dsi->hwnd);
  } else {
    LowLevelHook::Deactivate();
    printf("TurboVNC Helper: Ungrabbed keyboard\n");
  }

  bailout:
  if (ds) {
    if (dsi) ds->FreeDrawingSurfaceInfo(dsi);
    ds->Unlock(ds);
    awt.FreeDrawingSurface(ds);
  }
}
