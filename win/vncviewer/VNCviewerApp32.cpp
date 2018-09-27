//  Copyright (C) 2012, 2015-2016 D. R. Commander. All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
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

#include "VNCviewerApp32.h"
#include "vncviewer.h"
#include "Exception.h"
#include "LowLevelHook.h"
#include "wintab/Utils.h"


char *gpszProgramName = "vncviewer";


VNCviewerApp32::VNCviewerApp32(HINSTANCE hInstance, PSTR szCmdLine) :
  VNCviewerApp(hInstance, szCmdLine)
{
  m_pdaemon = NULL;
  m_wacom = false;

  // Load a requested keyboard layout
  if (m_options.m_kbdSpecified) {
    HKL hkl = LoadKeyboardLayout(m_options.m_kbdname,
                                 KLF_ACTIVATE | KLF_REPLACELANG | KLF_REORDER);
    if (hkl == NULL) {
      MessageBox(NULL, "Error loading specified keyboard layout", "VNC info",
                 MB_OK | MB_ICONSTOP);
      exit(1);
    }
  }

  // Start listening daemons, if requested
  if (m_options.m_listening && FindWindow("VNCviewer Daemon", 0) == NULL) {
    vnclog.Print(3, "In listen mode - starting daemons\n");
    ListenMode();
  } else
    m_options.m_listening = false;

  RegisterSounds();
  LowLevelHook::Initialize(hInstance);

  if (!LoadWintab()) {
    vnclog.Print(-1, "WinTab library not available\n");
    return;
  }
  if (!gpWTInfoA(0, 0, NULL)) {
    vnclog.Print(-1, "WinTab services not available\n");
    return;
  }
  char name[256];
  if (!gpWTInfoA(WTI_DEVICES, DVC_NAME, name) || strncmp(name, "WACOM", 5)) {
    vnclog.Print(-1, "Wacom tablet not installed\n");
    return;
  }
  m_wacom = true;
}


// These should maintain a list of connections
// FIXME: Eliminate duplicated code (see the following three functions)

int VNCviewerApp32::NewConnection()
{
  int retries = 0, retval = 0;
  ClientConnection *pcc;
  ClientConnection *old_pcc;

  if (m_options.m_benchFile != NULL) {
    double tAvg = 0.0, tAvgDecode = 0.0, tAvgBlit = 0.0;

    for (int i = 0; i < m_options.m_benchIter + m_options.m_benchWarmup; i++) {
      double tStart = 0.0, tTotal;

      try {
        pcc = new ClientConnection(this);
        if (i < m_options.m_benchWarmup)
          vnclog.Print(-1, "Benchmark warmup run %d\n", i + 1);
        else
          vnclog.Print(-1, "Benchmark run %d:\n",
                       i + 1 - m_options.m_benchWarmup);
        tStart = getTime();
        pcc->Run();
        MSG msg;  BOOL ret;
        while ((ret = GetMessage(&msg, NULL, 0, 0)) != 0) {
          if (ret == -1) {
            vnclog.Print(-1, "GetMessage() failed: %d\n", GetLastError());
            break;
          }
          TranslateMessage(&msg);
          DispatchMessage(&msg);
          if (msg.message == WM_CLOSE) retval = (int)msg.wParam;
        }
        tTotal = getTime() - tStart - tRead;
        if (i >= m_options.m_benchWarmup) {
          vnclog.Print(-1, "%f s (Decode = %f, Blit = %f)\n",
                       tTotal, tDecode, tBlit);
          vnclog.Print(-1, "     Decode statistics:\n");
          vnclog.Print(-1, "     %.3f Mpixels, %.3f Mpixels/sec, %d rect, %.0f pixels/rect,\n",
                       (double)decodePixels / 1000000.,
                       (double)decodePixels / 1000000. / tDecode, decodeRect,
                       (double)decodePixels / (double)decodeRect);
          vnclog.Print(-1, "       %.0f rects/update\n",
                       (double)decodeRect / (double)updates);
          vnclog.Print(-1, "     Blit statistics:\n");
          vnclog.Print(-1, "     %.3f Mpixels, %.3f Mpixels/sec, %d rect, %.0f pixels/rect\n",
                       (double)blitPixels / 1000000.,
                       (double)blitPixels / 1000000. / tBlit, blits,
                       (double)blitPixels / (double)blits);
          tAvg += tTotal;
          tAvgDecode += tDecode;
          tAvgBlit += tBlit;
        }
        fseek(m_options.m_benchFile, 0, SEEK_SET);
        vnclog.Print(-1, "\n");
      } catch (Exception &e) {
        vnclog.Print(-1, "Exception: %s\n", e.m_info);
        retval = 1;
        break;
      }
    }

    if (m_options.m_benchFile && m_options.m_benchIter > 1)
      vnclog.Print(-1, "Average          :  %f s (Decode = %f, Blit = %f)\n",
                   tAvg / (double)m_options.m_benchIter,
                   tAvgDecode / (double)m_options.m_benchIter,
                   tAvgBlit / (double)m_options.m_benchIter);

    return retval;
  }

  pcc = new ClientConnection(this);
  while (retries < MAX_AUTH_RETRIES) {
    try {
      pcc->Run();
      return retval;
    } catch (AuthException &e) {
      e.Report();
      pcc->UnloadConnection();
      // If the connection count drops to zero, the app exits
      old_pcc = pcc;
      pcc = new ClientConnection(this);
      // Get the previous options for the next try
      pcc->CopyOptions(old_pcc);
      delete old_pcc;
      retval = 1;
    } catch (Exception &e) {
      e.Report();
      retval = 1;
      break;
    }
    retries++;
  }
  delete pcc;
  return retval;
}


int VNCviewerApp32::NewConnection(char *host, int port)
{
  int retries = 0, retval = 0;
  ClientConnection *pcc;
  ClientConnection *old_pcc;

  pcc = new ClientConnection(this, host, port);
  while (retries < MAX_AUTH_RETRIES) {
    try {
      pcc->Run();
      return retval;
    } catch (AuthException &e) {
      e.Report();
      // If the connection count drops to zero, the app exits
      old_pcc = pcc;
      pcc = new ClientConnection(this, host, port);
      // Get the previous options for the next try
      pcc->CopyOptions(old_pcc);
      delete old_pcc;
      retval = 1;
    } catch (Exception &e) {
      e.Report();
      retval = 1;
      break;
    }
    retries++;
  }
  delete pcc;
  return retval;
}


int VNCviewerApp32::NewConnection(SOCKET sock)
{
  int retries = 0, retval = 0;
  ClientConnection *pcc;
  ClientConnection *old_pcc;

  pcc = new ClientConnection(this, sock);
  while (retries < MAX_AUTH_RETRIES) {
    try {
      pcc->Run();
      return retval;
    } catch (AuthException &e) {
      e.Report();
      // If the connection count drops to zero, the app exits
      old_pcc = pcc;
      pcc = new ClientConnection(this, sock);
      // Get the previous options for the next try
      pcc->CopyOptions(old_pcc);
      delete old_pcc;
      retval = 1;
    } catch (Exception &e) {
      e.Report();
      retval = 1;
      break;
    }
    retries++;
  }
  delete pcc;
  return retval;
}


void VNCviewerApp32::ListenMode()
{
  try {
    m_pdaemon = new Daemon(m_options.m_listenPort, m_options.m_ipv6);
  } catch (WarningException &e) {
    char msg[1024];
    SPRINTF(msg, "Error creating listening daemon:\n\r(%s)\n\r%s", e.m_info,
            "Perhaps another VNCviewer is already running?");
    MessageBox(NULL, msg, "VNCviewer error", MB_OK | MB_ICONSTOP);
    exit(1);
  }
}


// Register the Bell sound event

const char *BELL_APPL_KEY_NAME = "AppEvents\\Schemes\\Apps\\VNCviewer";
const char *BELL_LABEL = "VNCviewerBell";

void VNCviewerApp32::RegisterSounds()
{
  HKEY hBellKey;
  char keybuf[256];

  SPRINTF(keybuf, "AppEvents\\EventLabels\\%s", BELL_LABEL);
  // First, create a label for it
  if (RegCreateKey(HKEY_CURRENT_USER, keybuf, &hBellKey) == ERROR_SUCCESS) {
    RegSetValue(hBellKey, NULL, REG_SZ, "Bell", 0);
    RegCloseKey(hBellKey);

    // Then put the detail in the app-specific area
    if (RegCreateKey(HKEY_CURRENT_USER, BELL_APPL_KEY_NAME,
                     &hBellKey) == ERROR_SUCCESS) {
      SPRINTF(keybuf, "%s\\%s", BELL_APPL_KEY_NAME, BELL_LABEL);
      RegCreateKey(HKEY_CURRENT_USER, keybuf, &hBellKey);
      RegSetValue(hBellKey, NULL, REG_SZ, "Bell", 0);
      RegCloseKey(hBellKey);

      SPRINTF(keybuf, "%s\\%s\\.current", BELL_APPL_KEY_NAME, BELL_LABEL);
      if (RegOpenKey(HKEY_CURRENT_USER, keybuf, &hBellKey) != ERROR_SUCCESS) {
        RegCreateKey(HKEY_CURRENT_USER, keybuf, &hBellKey);
        RegSetValue(hBellKey, NULL, REG_SZ, "ding.wav", 0);
      }
      RegCloseKey(hBellKey);

      SPRINTF(keybuf, "%s\\%s\\.default", BELL_APPL_KEY_NAME, BELL_LABEL);
      if (RegOpenKey(HKEY_CURRENT_USER, keybuf, &hBellKey) != ERROR_SUCCESS) {
        RegCreateKey(HKEY_CURRENT_USER, keybuf, &hBellKey);
        RegSetValue(hBellKey, NULL, REG_SZ, "ding.wav", 0);
      }
      RegCloseKey(hBellKey);
    }
  }
}


bool VNCviewerApp32::ProcessDialogMessage(MSG *pmsg)
{
  if (!m_dialogs.empty()) {
    omni_mutex_lock l(m_dialogsMutex);
    std::list<HWND>::iterator iter;
    for (iter = m_dialogs.begin(); iter != m_dialogs.end(); iter++) {
      if (IsDialogMessage(*iter, pmsg))
        return true;
    }
  }
  return false;
}


VNCviewerApp32::~VNCviewerApp32()
{
  // We don't need to clean up pcc if the thread has been joined.
  if (m_pdaemon != NULL) delete m_pdaemon;
  LowLevelHook::Release();
  UnloadWintab();
}
