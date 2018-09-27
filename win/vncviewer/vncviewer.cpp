//  Copyright (C) 2010, 2012, 2015-2017 D. R. Commander. All Rights Reserved.
//  Copyright 2005 Alexandre Julliard
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


#include "stdhdrs.h"
#include "vncviewer.h"
#include "Exception.h"
#include "omnithread.h"
#include "VNCviewerApp32.h"


// All logging is done via the log object
Log vnclog;
VNCHelp help;
HotKeys hotkeys;
bool console = false;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   PSTR szCmdLine, int iCmdShow)
{
  int retval = 0;

  srand((unsigned int)time(NULL));

  // The state of the application as a whole is contained in the one app object
  VNCviewerApp32 app(hInstance, szCmdLine);

  // Start a new connection if one if specified on the command line or if
  // listening mode is not specified

  if (app.m_options.m_connectionSpecified) {
    retval = app.NewConnection(app.m_options.m_host, app.m_options.m_port);
  } else if (!app.m_options.m_listening) {
    // This one will also read from a config file, if specified
    retval = app.NewConnection();
  }

  if (app.m_options.m_benchFile) {
    fclose(app.m_options.m_benchFile);
    return retval;
  }

  MSG msg;
  std::list<HWND>::iterator iter;

  try {
    while (GetMessage(&msg, NULL, 0, 0)) {
      if (!hotkeys.TranslateAccel(&msg) && !help.TranslateMsg(&msg) &&
          !app.ProcessDialogMessage(&msg)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
      if (msg.message == WM_CLOSE) retval = (int)msg.wParam;
    }
  } catch (WarningException &e) {
    e.Report();
    retval = 1;
  } catch (QuietException &e) {
    e.Report();
    retval = 1;
  }

  // Clean up winsock
  WSACleanup();

  vnclog.Print(3, "Exiting\n");

  return retval;
}


// Borrowed from WINE
int main(int argc, char **argv)
{
  console = true;
  STARTUPINFOA info;
  char *cmdline = GetCommandLineA();
  int bcount = 0;
  BOOL in_quotes = FALSE;

  while (*cmdline) {
    if ((*cmdline == '\t' || *cmdline == ' ') && !in_quotes) break;
    else if (*cmdline == '\\') bcount++;
    else if (*cmdline == '\"') {
      if (!(bcount & 1)) in_quotes = !in_quotes;
      bcount = 0;
    } else
      bcount = 0;
    cmdline++;
  }
  while (*cmdline == '\t' || *cmdline == ' ') cmdline++;

  GetStartupInfoA(&info);
  if (!(info.dwFlags & STARTF_USESHOWWINDOW)) info.wShowWindow = SW_SHOWNORMAL;
  return WinMain(GetModuleHandleA(0), 0, cmdline, info.wShowWindow);
}


// Move the given window to the center of the screen and bring it to the top
void CenterWindow(HWND hwnd)
{
  RECT winrect, workrect;

  // Find how large the desktop work area is
  SystemParametersInfo(SPI_GETWORKAREA, 0, &workrect, 0);
  int workwidth = workrect.right -  workrect.left;
  int workheight = workrect.bottom - workrect.top;

  // And how big the window is
  GetWindowRect(hwnd, &winrect);
  int winwidth = winrect.right - winrect.left;
  int winheight = winrect.bottom - winrect.top;
  // Make sure it's not bigger than the work area
  winwidth = min(winwidth, workwidth);
  winheight = min(winheight, workheight);

  // Now center it
  SetWindowPos(hwnd, HWND_TOP, workrect.left + (workwidth - winwidth) / 2,
               workrect.top + (workheight - winheight) / 2, winwidth,
               winheight, SWP_SHOWWINDOW);
  SetForegroundWindow(hwnd);
}


// Convert "host:display" or "host::port" into host and port.
// Returns true if format is valid, false if not.  Takes initial string,
// addresses of results, and size of host buffer as wchars.  If the display
// info passed in is longer than the size of the host buffer, it is assumed to
// be invalid, so false is returned.  If the function returns true, then it
// also replaces the display[] string with its canonical representation.

bool ParseDisplay(LPTSTR display, int displaylen, LPTSTR phost, int hostlen,
                  int *pport)
{
  if (hostlen - 1 < (int)strlen(display))
    return false;

  int tmp_port;
  bool doubleColon = false;
  char *colonpos = strrchr(display, ':');
  char *bracketpos = strrchr(display, ']');

  if (bracketpos && colonpos < bracketpos)
    colonpos = NULL;

  while (colonpos > display && *(colonpos - 1) == ':') {
    colonpos--;
    doubleColon = true;
  }
  if (doubleColon) {
    // Check for preceding single colon, indicating an IPv6 address
    for (char *p = colonpos - 1; p >= display; p--) {
      if (*p == ':') {
        if (p == display || *(p - 1) != ':')
          colonpos = NULL;
        break;
      }
    }
  }

  if (colonpos == NULL) {
    // No colon -- use default port number
    tmp_port = RFB_PORT_OFFSET;
    STRNCPY(phost, display, hostlen);
    phost[MAX_HOST_NAME_LEN] = '\0';
  } else {
    STRNCPY(phost, display, hostlen);
    phost[colonpos - display] = '\0';
    if (colonpos[1] == ':') {
      // Two colons -- interpret as a port number
      if (sscanf_s(colonpos + 2, "%d", &tmp_port) != 1)
        return false;
    } else {
      // One colon -- interpret as a display or port number
      if (sscanf_s(colonpos + 1, "%d", &tmp_port) != 1)
        return false;
      if (tmp_port < 100)
        tmp_port += RFB_PORT_OFFSET;
    }
  }
  *pport = tmp_port;

  if (strlen(phost) == 0)
    STRNCPY(phost, "localhost", hostlen);

  FormatDisplay(tmp_port, display, displaylen, phost);
  return true;
}


void FormatDisplay(int port, LPTSTR display, int displaylen, LPTSTR host)
{
  if (port == 5900) {
    STRNCPY(display, host, displaylen);
  } else if (port > 5900 && port <= 5999) {
    snprintf(display, displaylen, "%s:%d", host, port - 5900);
  } else {
    snprintf(display, displaylen, "%s::%d", host, port);
  }
}
