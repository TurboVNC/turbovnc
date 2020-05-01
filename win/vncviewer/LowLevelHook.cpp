//  Based on LowLevelHook.cpp from Ultr@VNC, written by Assaf Gordon
//  (Assaf@mazleg.com), 10/9/2003 (original source lacks copyright attribution)
//  Modifications:
//  Copyright (C) 2012, 2016, 2020 D. R. Commander.  All Rights Reserved.
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.

// This is the source for the low-level keyboard hook, which allows
// intercepting and sending special keys (such as ALT+TAB, etc.) to the VNC
// server.

#include "LowLevelHook.h"
#include "XF86keysym.h"


HWND LowLevelHook::g_hwndVNCViewer = NULL;
DWORD LowLevelHook::g_VncProcessID = 0;
HHOOK LowLevelHook::g_HookID = 0;
HANDLE LowLevelHook::g_hThread = NULL;
DWORD LowLevelHook::g_nThreadID = 0;
omni_mutex LowLevelHook::g_Mutex;
std::map<UINT, HWND> LowLevelHook::g_PressedKeys;


void LowLevelHook::Initialize(HINSTANCE hInstance)
{
  // adzm 2009-09-25 - Install the hook in a different thread.  We receive
  // the hook callbacks via the message pump, so by using it on the main
  // connection thread, it could be delayed because of file transfers, etc.
  // Thus, we use a dedicated thread.
  g_hThread =
    CreateThread(NULL, 0, HookThreadProc, hInstance, 0, &g_nThreadID);
  if (!g_hThread)
    vnclog.Print(0, "Error %d from CreateThread()", GetLastError());
}


// adzm 2009-09-25 - Hook events handled on this thread
DWORD WINAPI LowLevelHook::HookThreadProc(LPVOID lpParam)
{
  HINSTANCE hInstance = (HINSTANCE)lpParam;

  MSG msg;
  BOOL bRet;

  // Ensure that the message queue is operational
  PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

  // Try to set the hook procedure
  g_HookID = SetWindowsHookEx(WH_KEYBOARD_LL, VncLowLevelKbHookProc,
                              hInstance, 0);

  if (g_HookID == 0) {
    vnclog.Print(0, "Error %d from SetWindowsHookEx()", GetLastError());
    return 0;
  }

  while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
    if (bRet == -1) {
      vnclog.Print(0, "Error %d from GetMessage()", GetLastError());
      return 0;
    } else if (msg.message == WM_SHUTDOWNLLKBHOOK) {
      PostQuitMessage(0);
    } else {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  return 0;
}


void LowLevelHook::Release()
{
  // adzm 2009-09-25 - Post a message to the thread instructing it to
  // terminate
  if (g_hThread) {
    PostThreadMessage(g_nThreadID, WM_SHUTDOWNLLKBHOOK, 0, 0);
    WaitForSingleObject(g_hThread, INFINITE);
    CloseHandle(g_hThread);
  }

  if (g_HookID) UnhookWindowsHookEx(g_HookID);
}


void LowLevelHook::Activate(HWND win)
{
  omni_mutex_lock l(g_Mutex);

  g_hwndVNCViewer = win;

  // Store the process ID of the VNC window.  This will prevent the keyboard
  // hook procedure from interfering with keypresses in other processes'
  // windows.
  GetWindowThreadProcessId(g_hwndVNCViewer, &g_VncProcessID);
}


void LowLevelHook::Deactivate(void)
{
  omni_mutex_lock l(g_Mutex);

  // If the VNC viewer window previously received a press event for the Windows
  // key but hasn't yet received a release event for same (this can happen, for
  // instance, when using WIN+L to lock the screen), then we need to send a
  // fake key release for the Windows key to the server.
  for (UINT vkCode = VK_LWIN; vkCode <= VK_RWIN; vkCode++) {
    int xkey = (vkCode == VK_LWIN ? XK_Super_L : XK_Super_R);
    std::map<UINT, HWND>::iterator iter = g_PressedKeys.find(vkCode);

    if (iter != g_PressedKeys.end()) {
      if (iter->second == g_hwndVNCViewer)
        PostMessage(g_hwndVNCViewer, WM_SYSCOMMAND, ID_CONN_SENDKEYUP, xkey);
      g_PressedKeys.erase(iter);
    }
  }

  g_hwndVNCViewer = NULL;
  g_VncProcessID = 0;
}


bool LowLevelHook::isActive(HWND win)
{
  return win == g_hwndVNCViewer;
}


bool LowLevelHook::isActive(void)
{
  return g_hwndVNCViewer != 0;
}


LRESULT CALLBACK LowLevelHook::VncLowLevelKbHookProc(INT nCode, WPARAM wParam,
                                                     LPARAM lParam)
{
  omni_mutex_lock l(g_Mutex);

  // If set to TRUE, the keypress message will NOT be passed on to Windows.
  BOOL fHandled = FALSE;

  BOOL fKeyDown = FALSE;

  if (nCode == HC_ACTION) {
    KBDLLHOOKSTRUCT *pkbdllhook = (KBDLLHOOKSTRUCT *)lParam;
    DWORD ProcessID;

    // Get the process ID of the Active Window (the window with the input
    // focus)
    HWND hwndCurrent = GetForegroundWindow();
    GetWindowThreadProcessId(hwndCurrent, &ProcessID);

    fKeyDown = ((wParam == WM_KEYDOWN) || (wParam == WM_SYSKEYDOWN));

    // Intercept keypresses only if this is the vncviewer process and
    // only if the foreground window is the one we want to hook
    if (ProcessID == g_VncProcessID && isActive(hwndCurrent)) {
      int xkey;

      switch (pkbdllhook->vkCode) {
        case VK_LWIN:
          xkey = XK_Super_L;  break;
        case VK_RWIN:
          xkey = XK_Super_R;  break;
        case VK_APPS:
          xkey = XK_Menu;  break;
        case VK_BROWSER_HOME:
          xkey = XF86XK_HomePage;  break;
        case VK_BROWSER_SEARCH:
          xkey = XF86XK_Search;  break;
        case VK_LAUNCH_MAIL:
          xkey = XF86XK_Mail;  break;
        case VK_VOLUME_MUTE:
          xkey = XF86XK_AudioMute;  break;
        case VK_VOLUME_DOWN:
          xkey = XF86XK_AudioLowerVolume;  break;
        case VK_VOLUME_UP:
          xkey = XF86XK_AudioRaiseVolume;  break;
        case VK_MEDIA_PLAY_PAUSE:
          xkey = XF86XK_AudioPlay;  break;
        case VK_LAUNCH_APP2:
          xkey = XF86XK_Calculator;  break;
        case VK_BROWSER_FAVORITES:
          xkey = XF86XK_Favorites;  break;
        case VK_BROWSER_BACK:
          xkey = XF86XK_Back;  break;
        case VK_BROWSER_FORWARD:
          xkey = XF86XK_Forward;  break;
      }

      switch (pkbdllhook->vkCode) {
        case VK_LWIN:
        case VK_RWIN:
        {
          // If the VNC viewer window is not active when switching virtual
          // desktops using WIN+CTRL+LEFT or WIN+CTRL+RIGHT, then the viewer
          // window may receive a release event for the Windows key that was
          // not preceded by a press event.  Thus, we only handle the release
          // event if the press event was also received by the viewer window.
          HWND hwnd = 0;

          if (fKeyDown)
            g_PressedKeys[pkbdllhook->vkCode] = hwndCurrent;
          else {
            std::map<UINT, HWND>::iterator iter =
              g_PressedKeys.find(pkbdllhook->vkCode);

            if (iter != g_PressedKeys.end()) {
              hwnd = iter->second;
              g_PressedKeys.erase(iter);
            }
          }

          if (fKeyDown || hwnd == g_hwndVNCViewer) {
            PostMessage(g_hwndVNCViewer, WM_SYSCOMMAND,
                        fKeyDown ? ID_CONN_SENDKEYDOWN : ID_CONN_SENDKEYUP,
                        xkey);
            fHandled = TRUE;
          }
          break;
        }
        case VK_APPS:
        case VK_BROWSER_HOME:
        case VK_BROWSER_SEARCH:
        case VK_LAUNCH_MAIL:
        case VK_VOLUME_MUTE:
        case VK_VOLUME_DOWN:
        case VK_VOLUME_UP:
        case VK_MEDIA_PLAY_PAUSE:
        case VK_LAUNCH_APP2:
        case VK_BROWSER_FAVORITES:
        case VK_BROWSER_BACK:
        case VK_BROWSER_FORWARD:
          PostMessage(g_hwndVNCViewer, WM_SYSCOMMAND,
                      fKeyDown ? ID_CONN_SENDKEYDOWN : ID_CONN_SENDKEYUP,
                      xkey);
          fHandled = TRUE;
          break;

        // For window switching sequences (ALT+TAB, ALT+ESC, CTRL+ESC),
        // we intercept the primary keypress when it occurs after the
        // modifier keypress, then we look for the corresponding
        // primary key release and intercept it as well.  Both primary
        // keystrokes are sent to the VNC server but not to Windows.
        case VK_TAB:
        {
          static BOOL fAltTab = FALSE;
          if (pkbdllhook->flags & LLKHF_ALTDOWN && fKeyDown) {
            PostMessage(g_hwndVNCViewer, WM_SYSCOMMAND, ID_CONN_SENDKEYDOWN,
                        XK_Tab);
            fHandled = TRUE;
            fAltTab = TRUE;
          } else if (fAltTab) {
            if (!fKeyDown) {
              PostMessage(g_hwndVNCViewer, WM_SYSCOMMAND, ID_CONN_SENDKEYUP,
                          XK_Tab);
              fHandled = TRUE;
            }
            fAltTab = FALSE;
          }
          break;
        }

        case VK_ESCAPE:
        {
          static BOOL fAltEsc = FALSE, fCtrlEsc = FALSE;
          if (pkbdllhook->flags & LLKHF_ALTDOWN && fKeyDown) {
            PostMessage(g_hwndVNCViewer, WM_SYSCOMMAND, ID_CONN_SENDKEYDOWN,
                        XK_Escape);
            fHandled = TRUE;
            fAltEsc = TRUE;
          } else if (fAltEsc) {
            if (!fKeyDown) {
              PostMessage(g_hwndVNCViewer, WM_SYSCOMMAND, ID_CONN_SENDKEYUP,
                          XK_Escape);
              fHandled = TRUE;
            }
            fAltEsc = FALSE;
          }
          if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && fKeyDown) {
            PostMessage(g_hwndVNCViewer, WM_SYSCOMMAND, ID_CONN_SENDKEYDOWN,
                        XK_Escape);
            fHandled = TRUE;
            fCtrlEsc = TRUE;
          } else if (fCtrlEsc) {
            if (!fKeyDown) {
              PostMessage(g_hwndVNCViewer, WM_SYSCOMMAND, ID_CONN_SENDKEYUP,
                          XK_Escape);
              fHandled = TRUE;
            }
            fCtrlEsc = FALSE;
          }
          break;
        }

      }  // switch (pkbdllhook->vkCode)

    }  // if (ProcessID == g_VncProcesID && isActive(hwndCurrent))

  }  // if (nCode == HT_ACTION)

  // Call the next hook, if we didn't handle this message
  return fHandled ? TRUE : CallNextHookEx(g_HookID, nCode, wParam, lParam);
}
