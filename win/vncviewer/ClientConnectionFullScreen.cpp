//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
//  Copyright (C) 2010-2012 D. R. Commander. All Rights Reserved.
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

// Many thanks to Greg Hewgill <greg@hewgill.com> for providing the basis for
// the full-screen mode.

#include "stdhdrs.h"
#include "vncviewer.h"
#include "ClientConnection.h"
#include "LowLevelHook.h"

// Parameters for scrolling in full screen mode
#define BUMPSCROLLBORDER 4
#define BUMPSCROLLAMOUNTX 8
#define BUMPSCROLLAMOUNTY 6


bool ClientConnection::InFullScreenMode()
{
  return m_opts.m_FullScreen;
};


void ClientConnection::SetFullScreenMode(bool enable, bool suppressPrompt)
{
  m_opts.m_FullScreen = enable;
  RealiseFullScreenMode(suppressPrompt);
}


// If the full-screen option has been changed other than by calling
// SetFullScreenMode(), then this function must be called to actually change
// the mode.

void ClientConnection::RealiseFullScreenMode(bool suppressPrompt)
{
  LONG style = GetWindowLong(m_hwnd1, GWL_STYLE);
  static RECT savedRect = {-1, -1, -1, -1};
  if (m_opts.m_FullScreen) {
    if (!suppressPrompt && !pApp->m_options.m_skipprompt) {
      MessageBox(m_hwnd1,
        "To exit from full-screen mode, press Ctrl-Alt-Shift-F.\r\n"
        "Alternatively, press Ctrl-Esc Esc and then right-click\r\n"
        "on the vncviewer taskbar icon to see the menu.",
        "VNCviewer full-screen mode",
        MB_OK | MB_ICONINFORMATION | MB_TOPMOST | MB_SETFOREGROUND);
    }
    ShowWindow(m_hToolbar, SW_HIDE);
    EnableMenuItem(GetSystemMenu(m_hwnd1, FALSE), ID_TOOLBAR,
                   MF_BYCOMMAND | MF_GRAYED);
    style = GetWindowLong(m_hwnd1, GWL_STYLE);
    style &= ~(WS_DLGFRAME | WS_THICKFRAME | WS_BORDER);

    SetWindowLong(m_hwnd1, GWL_STYLE, style);
    RECT screenArea, workArea;
    GetFullScreenMetrics(screenArea, workArea);
    GetWindowRect(m_hwnd1, &savedRect);
    SetWindowPos(m_hwnd1, HWND_TOPMOST, screenArea.left, screenArea.top,
      screenArea.right - screenArea.left,
      screenArea.bottom - screenArea.top, SWP_FRAMECHANGED);
    CheckMenuItem(GetSystemMenu(m_hwnd1, FALSE), ID_FULLSCREEN,
                  MF_BYCOMMAND | MF_CHECKED);
    if (m_opts.m_GrabKeyboard == TVNC_FS) {
      LowLevelHook::Activate(m_hwnd1);
      CheckMenuItem(GetSystemMenu(m_hwnd1, FALSE), ID_TOGGLE_GRAB,
                    MF_BYCOMMAND | MF_CHECKED);
    }
  } else {
    ShowWindow(m_hToolbar, SW_SHOW);
    EnableMenuItem(GetSystemMenu(m_hwnd1, FALSE), ID_TOOLBAR,
                   MF_BYCOMMAND | MF_ENABLED);
    style |= (WS_DLGFRAME | WS_THICKFRAME | WS_BORDER);

    SetWindowLong(m_hwnd1, GWL_STYLE, style);
    if (savedRect.bottom - savedRect.top > 0 &&
      savedRect.right - savedRect.left > 0)
      SetWindowPos(m_hwnd1, HWND_NOTOPMOST, savedRect.left,
                   savedRect.top, savedRect.right - savedRect.left,
                   savedRect.bottom - savedRect.top, 0);
    else
      SetWindowPos(m_hwnd1, HWND_NOTOPMOST, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE);
    CheckMenuItem(GetSystemMenu(m_hwnd1, FALSE), ID_FULLSCREEN,
                  MF_BYCOMMAND | MF_UNCHECKED);
    if (m_opts.m_GrabKeyboard == TVNC_FS) {
      LowLevelHook::Deactivate();
      CheckMenuItem(GetSystemMenu(m_hwnd1, FALSE), ID_TOGGLE_GRAB,
                    MF_BYCOMMAND | MF_UNCHECKED);
    }
  }
}


#define WidthOf(rect) ((rect).right - (rect).left)
#define HeightOf(rect) ((rect).bottom - (rect).top)

typedef struct _FSMetrics {
  RECT screenArea, workArea, screenArea0, workArea0;
  bool equal;
} FSMetrics;


static BOOL CALLBACK MonitorEnumProc(HMONITOR hmon, HDC hdc, LPRECT rect,
                                     LPARAM data)
{
  FSMetrics *fsm = (FSMetrics *)data;
  MONITORINFO mi;

  memset(&mi, 0, sizeof(MONITORINFO));
  mi.cbSize = sizeof(MONITORINFO);
  GetMonitorInfo(hmon, &mi);

  // If any monitors aren't equal in resolution to and evenly offset from the
  // primary, then we can't use the simple path.
  if (WidthOf(mi.rcMonitor) != WidthOf(fsm->screenArea0) ||
      HeightOf(mi.rcMonitor) != HeightOf(fsm->screenArea0) ||
      (abs(mi.rcMonitor.top - fsm->screenArea0.top) %
        HeightOf(fsm->screenArea0)) != 0 ||
      (abs(mi.rcMonitor.left - fsm->screenArea0.left) %
        WidthOf(fsm->screenArea0)) != 0)
    fsm->equal = 0;

  // If the screen areas of the primary monitor and this monitor overlap
  // vertically, then allow the full-screen window to extend horizontally to
  // this monitor, and constrain it vertically, if necessary, to fit within
  // this monitor's dimensions.
  if (min(mi.rcMonitor.bottom, fsm->screenArea0.bottom) -
      max(mi.rcMonitor.top, fsm->screenArea0.top) > 0) {
    fsm->screenArea.top = max(mi.rcMonitor.top, fsm->screenArea.top);
    fsm->screenArea.left = min(mi.rcMonitor.left, fsm->screenArea.left);
    fsm->screenArea.right = max(mi.rcMonitor.right, fsm->screenArea.right);
    fsm->screenArea.bottom = min(mi.rcMonitor.bottom, fsm->screenArea.bottom);
  }

  // If the work areas of the primary monitor and this monitor overlap
  // vertically, and if the top portion of the primary monitor intersects with
  // this monitor's work area, then allow the non-full-screen window to extend
  // horizontally to this monitor, and constrain it vertically, if necessary,
  // to fit within this monitor's work area dimensions.
  if (mi.rcWork.top <= 0 && mi.rcWork.left >= 0 &&
      min(mi.rcWork.bottom, fsm->workArea0.bottom) -
      max(mi.rcWork.top, fsm->workArea0.top) > 0) {
    fsm->workArea.top = max(mi.rcWork.top, fsm->workArea.top);
    fsm->workArea.left = min(mi.rcWork.left, fsm->workArea.left);
    fsm->workArea.right = max(mi.rcWork.right, fsm->workArea.right);
    fsm->workArea.bottom = min(mi.rcWork.bottom, fsm->workArea.bottom);
  }

  return TRUE;
}


void ClientConnection::GetFullScreenMetrics(RECT &screenArea, RECT &workArea)
{
  FSMetrics fsm;
  int primaryWidth = GetSystemMetrics(SM_CXSCREEN);
  int primaryHeight = GetSystemMetrics(SM_CYSCREEN);
  int scaledWidth = m_si.framebufferWidth * m_opts.m_scale_num /
                    m_opts.m_scale_den;
  int scaledHeight = m_si.framebufferHeight * m_opts.m_scale_num /
                     m_opts.m_scale_den;

  if (m_opts.m_FitWindow) {
    scaledWidth = m_si.framebufferWidth;
    scaledHeight = m_si.framebufferHeight;
  }
  fsm.equal = 1;
  fsm.screenArea.top = fsm.screenArea.left = 0;
  fsm.screenArea.right = primaryWidth;
  fsm.screenArea.bottom = primaryHeight;
  fsm.screenArea0 = fsm.screenArea;
  SystemParametersInfo(SPI_GETWORKAREA, 0, &fsm.workArea, 0);
  fsm.workArea0 = fsm.workArea;

  if (m_opts.m_Span == SPAN_PRIMARY ||
      (m_opts.m_Span == SPAN_AUTO &&
       (scaledWidth <= primaryWidth ||
        WidthOf(fsm.screenArea) <= primaryWidth) &&
       (scaledHeight <= primaryHeight ||
        HeightOf(fsm.screenArea) <= primaryHeight)) ||
      !EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&fsm)) {
    workArea = fsm.workArea0;
    screenArea = fsm.screenArea0;
  } else {
    if (fsm.equal) {
      // All monitors are equal in resolution and aligned in a perfect grid.
      // Thus, we can extend the viewer window to all of them, both
      // horizontally and vertically (otherwise, the viewer window can only
      // be extended horizontally.)
      screenArea.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
      screenArea.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
      screenArea.right = GetSystemMetrics(SM_CXVIRTUALSCREEN);
      screenArea.bottom = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    }
    else screenArea = fsm.screenArea;
    workArea = fsm.workArea;
  }
}


bool ClientConnection::BumpScroll(int x, int y)
{
  int dx = 0;
  int dy = 0;
  RECT screenArea, workArea;
  GetFullScreenMetrics(screenArea, workArea);
  int rightborder = screenArea.right - screenArea.left - BUMPSCROLLBORDER;
  int bottomborder = screenArea.bottom - screenArea.top - BUMPSCROLLBORDER;
  if (x < BUMPSCROLLBORDER)
    dx = -BUMPSCROLLAMOUNTX * m_opts.m_scale_num / m_opts.m_scale_den;
  if (x >= rightborder)
    dx = +BUMPSCROLLAMOUNTX * m_opts.m_scale_num / m_opts.m_scale_den;;
  if (y < BUMPSCROLLBORDER)
    dy = -BUMPSCROLLAMOUNTY * m_opts.m_scale_num / m_opts.m_scale_den;;
  if (y >= bottomborder)
    dy = +BUMPSCROLLAMOUNTY * m_opts.m_scale_num / m_opts.m_scale_den;;
  if (dx || dy) {
    if (ScrollScreen(dx, dy)) {
      // If we haven't physically moved the cursor, then artificially
      // generate another mouse event so we keep scrolling.
      POINT p;
      GetCursorPos(&p);
      if (p.x == x && p.y == y)
        SetCursorPos(x, y);
      return true;
    }
  }
  return false;
}
