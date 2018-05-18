//  Copyright (C) 2012, 2016 D. R. Commander. All Rights Reserved.
//  Copyright (C) 2005 Sun Microsystems, Inc. All Rights Reserved.
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


#ifndef VNCVIEWER_H__
#define VNCVIEWER_H__

#pragma once

#include "res/resource.h"
#include "VNCviewerApp.h"
#include "Log.h"
#include "VNCHelp.h"
#include "HotKeys.h"

#define WM_SOCKEVENT WM_USER + 1
#define WM_TRAYNOTIFY WM_SOCKEVENT + 1
#define WM_REGIONUPDATED WM_TRAYNOTIFY + 1
#define WM_FBUPDATERECVD WM_REGIONUPDATED + 1
#define WM_SHUTDOWNLLKBHOOK WM_FBUPDATERECVD + 1


// The enum is ordered in this way so as to maintain backward compatibility
// with TurboVNC 0.3.x
#define TVNC_SAMPOPT 4
enum { TVNC_1X = 0, TVNC_4X, TVNC_2X, TVNC_GRAY };

#define TVNC_GRABOPT 3
enum { TVNC_FS = 0, TVNC_ALWAYS, TVNC_MANUAL };


// The Application
extern VNCviewerApp *pApp;


// Global logger - may be used by anything
extern Log vnclog;
extern VNCHelp help;
extern HotKeys hotkeys;


// Display given window in center of screen
void CenterWindow(HWND hwnd);


// Convert "host:display" into host and port
// Returns true if valid.
bool ParseDisplay(LPTSTR display, int displaylen, LPTSTR phost, int hostlen,
                  int *port);
void FormatDisplay(int port, LPTSTR display, int displaylen, LPTSTR host);


__inline double getTime(void)
{
  LARGE_INTEGER frequency, time;
  if (QueryPerformanceFrequency(&frequency) != 0) {
    QueryPerformanceCounter(&time);
    return (double)time.QuadPart / (double)frequency.QuadPart;
  } else
    return (double)GetTickCount() * 0.001;
}

#endif  // VNCVIEWER_H__
