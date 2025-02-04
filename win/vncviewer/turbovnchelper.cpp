/* Copyright (C) 2015-2017, 2022, 2025 D. R. Commander.  All Rights Reserved.
 * Copyright (C) 2011 ymnk, JCraft, Inc.  All Rights Reserved.
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

#include <string.h>
#include "LowLevelHook.h"
#include "jawt_md.h"
#include "com_jcraft_jsch_PageantConnector.h"
#include "com_turbovnc_vncviewer_Viewport.h"
#include "safestr.h"


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


#define _throwe(msg, exceptionClass) {  \
  jclass _exccls = env->FindClass(exceptionClass);  \
  if (!_exccls) goto bailout;  \
  env->ThrowNew(_exccls, msg);  \
  goto bailout;  \
}

#define _throw(msg) _throwe(msg, "java/lang/Exception");

#define _throww32e(exceptionClass) {  \
  char message[256];  \
  if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),  \
                     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message,  \
                     256, NULL))  \
    strncpy_s(message, _countof(message), "Error in FormatMessage()",  \
              _TRUNCATE);  \
  _throwe(message, exceptionClass);  \
}

#define _throww32() _throww32e("java/lang/Exception");

#define BAILIF0(f) {  \
  if (!(f) || env->ExceptionCheck()) {  \
    goto bailout;  \
  }  \
}


typedef jboolean (JNICALL *__JAWT_GetAWT_type)(JNIEnv *env, JAWT *awt);
static __JAWT_GetAWT_type __JAWT_GetAWT = NULL;

static HMODULE handle = NULL;


extern "C" {

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


#define AGENT_MAX_MSGLEN  262144
#define AGENT_COPYDATA_ID  0x804e50baL


JNIEXPORT void JNICALL Java_com_jcraft_jsch_PageantConnector_queryPageant
  (JNIEnv *env, jobject obj, jobject buffer)
{
  HANDLE sharedFile = 0;
  char *buf = NULL, *sharedMemory = NULL;
  COPYDATASTRUCT cds;
  jclass bufferClass;
  jfieldID fid;
  jmethodID mid;
  jbyteArray jbuf;
  DWORD length;
  jint len;

  memset(&cds, 0, sizeof(cds));

  BAILIF0(bufferClass = env->GetObjectClass(buffer));
  BAILIF0(fid = env->GetFieldID(bufferClass, "buffer", "[B"));
  BAILIF0(jbuf = (jbyteArray)env->GetObjectField(buffer, fid));
  BAILIF0(mid = env->GetMethodID(bufferClass, "getLength", "()I"));
  len = env->CallIntMethod(buffer, mid);

  if (len > AGENT_MAX_MSGLEN)
    _throwe("Query too large.", "com/jcraft/jsch/AgentProxyException");

  HWND hwnd = FindWindow("Pageant", "Pageant");
  if (!hwnd)
    _throwe("Pageant is not runnning.", "com/jcraft/jsch/AgentProxyException");

  char mapName[30];
  snprintf(mapName, 30, "PageantRequest%08x", GetCurrentThreadId());
  sharedFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
                                 AGENT_MAX_MSGLEN, mapName);
  if (!sharedFile)
    _throww32e("com/jcraft/jsch/AgentProxyException");
  sharedMemory = (char *)MapViewOfFile(sharedFile, SECTION_MAP_WRITE, 0, 0, 0);
  if (!sharedMemory)
    _throww32e("com/jcraft/jsch/AgentProxyException");

  BAILIF0(buf = (char *)env->GetPrimitiveArrayCritical(jbuf, 0));
  memcpy(sharedMemory, buf, len);
  env->ReleasePrimitiveArrayCritical(jbuf, buf, 0);

  cds.dwData = AGENT_COPYDATA_ID;
  cds.cbData = (DWORD)(strlen(mapName) + 1);
  cds.lpData = _strdup(mapName);
  if (!SendMessage(hwnd, WM_COPYDATA, 0, (LPARAM)(LPVOID)&cds))
    _throww32e("com/jcraft/jsch/AgentProxyException");

  length = *(DWORD *)sharedMemory;
  length = (length >> 24) | ((length & 0x00FF0000) >> 8) |
           ((length & 0x0000FF00) << 8) | (length << 24);
  if (length <= 0 || length > AGENT_MAX_MSGLEN - 4)
    _throwe("Illegal length", "com/jcraft/jsch/AgentProxyException");

  BAILIF0(mid = env->GetMethodID(bufferClass, "rewind", "()V"));
  env->CallVoidMethod(buffer, mid);
  BAILIF0(mid = env->GetMethodID(bufferClass, "checkFreeSize", "(I)V"));
  env->CallVoidMethod(buffer, mid, (jint)length);

  BAILIF0(jbuf = (jbyteArray)env->GetObjectField(buffer, fid));
  BAILIF0(buf = (char *)env->GetPrimitiveArrayCritical(jbuf, 0));
  memcpy(buf, &sharedMemory[4], length);
  env->ReleasePrimitiveArrayCritical(jbuf, buf, 0);

  bailout:
  free(cds.lpData);
  if (sharedMemory) UnmapViewOfFile(sharedMemory);
  if (sharedFile) CloseHandle(sharedFile);
}

}  // extern "C"
