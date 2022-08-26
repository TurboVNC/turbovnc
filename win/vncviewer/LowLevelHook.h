//  Based on LowLevelHook.h from Ultr@VNC, written by Assaf Gordon
//  (Assaf@mazleg.com), 10/9/2003 (original source lacks copyright attribution)
//  Modifications:
//  Copyright (C) 2012, 2015, 2020 D. R. Commander.  All Rights Reserved.
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

// This is the source for the low-level keyboard hook, which allows
// intercepting and sending special keys (such as ALT+TAB, etc.) to the VNC
// server.


#pragma once

#include <windows.h>
#include "omnithread.h"
#include <map>


class LowLevelHook
{
  public:
    static void Initialize(HINSTANCE);
    static void Activate(HWND);
    static bool isActive(HWND);
    static void Deactivate();
    static void Release();

    static DWORD WINAPI HookThreadProc(LPVOID);

  private:
    static LRESULT CALLBACK VncLowLevelKbHookProc(INT, WPARAM, LPARAM);

    static HWND g_hwndVNCViewer;
    static DWORD g_VncProcessID;
    static HHOOK g_HookID;

    // adzm 2009-09-25 - Hook installed on separate thread
    static HANDLE g_hThread;
    static DWORD g_nThreadID;

    static omni_mutex g_Mutex;

    static std::map<UINT, HWND> g_PressedKeys;
};
