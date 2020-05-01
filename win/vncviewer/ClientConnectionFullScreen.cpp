//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
//  Copyright (C) 2010-2012, 2015, 2017, 2020 D. R. Commander.
//                                            All Rights Reserved.
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
#include "ScreenSet.h"

// Parameters for scrolling in full screen mode
#define BUMPSCROLLBORDER 4
#define BUMPSCROLLAMOUNTX 8
#define BUMPSCROLLAMOUNTY 6


bool ClientConnection::InFullScreenMode()
{
  return m_opts.m_FullScreen;
}


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
    if (savedRect.left < 0 && savedRect.right < 0 && savedRect.top < 0 &&
        savedRect.bottom < 0)
      GetWindowRect(m_hwnd1, &savedRect);
    SetWindowPos(m_hwnd1, HWND_TOPMOST, screenArea.left, screenArea.top,
                 screenArea.right - screenArea.left,
                 screenArea.bottom - screenArea.top, SWP_FRAMECHANGED);
    CheckMenuItem(GetSystemMenu(m_hwnd1, FALSE), ID_FULLSCREEN,
                  MF_BYCOMMAND | MF_CHECKED);
    if (m_opts.m_GrabKeyboard == TVNC_FS)
      GrabKeyboard(true);
  } else {
    ShowWindow(m_hToolbar, SW_SHOW);
    EnableMenuItem(GetSystemMenu(m_hwnd1, FALSE), ID_TOOLBAR,
                   MF_BYCOMMAND | MF_ENABLED);
    style |= (WS_DLGFRAME | WS_THICKFRAME | WS_BORDER);

    SetWindowLong(m_hwnd1, GWL_STYLE, style);
    if (savedRect.bottom - savedRect.top > 0 &&
        savedRect.right - savedRect.left > 0) {
      SetWindowPos(m_hwnd1, HWND_NOTOPMOST, savedRect.left, savedRect.top,
                   savedRect.right - savedRect.left,
                   savedRect.bottom - savedRect.top, 0);
      SetRect(&savedRect, -1, -1, -1, -1);
    } else
      SetWindowPos(m_hwnd1, HWND_NOTOPMOST, 0, 0, 0, 0,
                   SWP_NOMOVE | SWP_NOSIZE);
    CheckMenuItem(GetSystemMenu(m_hwnd1, FALSE), ID_FULLSCREEN,
                  MF_BYCOMMAND | MF_UNCHECKED);
    if (m_opts.m_GrabKeyboard == TVNC_FS)
      UngrabKeyboard(true);
  }
}


typedef struct _FSMetrics {
  RECT screenArea, workArea, screenArea0, workArea0, winRect, clientRect;
  bool equal, fullScreen, verbose;
  int maxArea, screenNum;
  ScreenSet computedLayout;
} FSMetrics;


static BOOL CALLBACK MonitorEnumProc(HMONITOR hmon, HDC hdc, LPRECT rect,
                                     LPARAM data)
{
  FSMetrics *fsm = (FSMetrics *)data;
  MONITORINFO mi;

  memset(&mi, 0, sizeof(MONITORINFO));
  mi.cbSize = sizeof(MONITORINFO);
  GetMonitorInfo(hmon, &mi);

  if (fsm->verbose) {
    if (fsm->fullScreen)
      vnclog.Print(4, "Screen %d FS area: %d, %d %d x %d\n", fsm->screenNum++,
                   mi.rcMonitor.left, mi.rcMonitor.top, WidthOf(mi.rcMonitor),
                   HeightOf(mi.rcMonitor));
    else
      vnclog.Print(4, "Screen %d work area: %d, %d %d x %d\n",
                   fsm->screenNum++, mi.rcWork.left, mi.rcWork.top,
                   WidthOf(mi.rcWork), HeightOf(mi.rcWork));
  }

  // If any monitors aren't equal in resolution to and evenly offset from the
  // primary, then we can't use the simple path.
  if (WidthOf(mi.rcMonitor) != WidthOf(fsm->screenArea0) ||
      HeightOf(mi.rcMonitor) != HeightOf(fsm->screenArea0) ||
      (abs(mi.rcMonitor.top - fsm->screenArea0.top) %
       HeightOf(fsm->screenArea0)) != 0 ||
      (abs(mi.rcMonitor.left - fsm->screenArea0.left) %
       WidthOf(fsm->screenArea0)) != 0)
    fsm->equal = 0;

  // If the screen area of this monitor overlaps vertically with the
  // multi-screen area, then allow the full-screen window to extend
  // horizontally to this monitor, and constrain it vertically, if necessary,
  // to fit within this monitor's dimensions.
  if (min(mi.rcMonitor.bottom, fsm->screenArea.bottom) -
      max(mi.rcMonitor.top, fsm->screenArea.top) > 0) {
    fsm->screenArea.top = max(mi.rcMonitor.top, fsm->screenArea.top);
    fsm->screenArea.left = min(mi.rcMonitor.left, fsm->screenArea.left);
    fsm->screenArea.right = max(mi.rcMonitor.right, fsm->screenArea.right);
    fsm->screenArea.bottom = min(mi.rcMonitor.bottom, fsm->screenArea.bottom);
  }

  // If the work area of this monitor overlaps vertically with the multi-screen
  // work area, then allow the non-full-screen window to extend horizontally to
  // this monitor, and constrain it vertically, if necessary, to fit within
  // this monitor's work area.
  if (min(mi.rcWork.bottom, fsm->workArea.bottom) -
      max(mi.rcWork.top, fsm->workArea.top) > 0) {
    fsm->workArea.top = max(mi.rcWork.top, fsm->workArea.top);
    fsm->workArea.left = min(mi.rcWork.left, fsm->workArea.left);
    fsm->workArea.right = max(mi.rcWork.right, fsm->workArea.right);
    fsm->workArea.bottom = min(mi.rcWork.bottom, fsm->workArea.bottom);
  }

  if (WidthOf(fsm->winRect) > 0 && HeightOf(fsm->winRect) > 0) {
    RECT vpRect;

    IntersectRect(&vpRect, &fsm->winRect, &mi.rcMonitor);
    int area = IsRectEmpty(&vpRect) ? 0 : WidthOf(vpRect) * HeightOf(vpRect);
    if (area > fsm->maxArea) {
      fsm->maxArea = area;
      fsm->workArea0 = mi.rcWork;
      fsm->screenArea0 = mi.rcMonitor;
    }

    IntersectRect(&vpRect, &fsm->clientRect, &mi.rcMonitor);
    if (!IsRectEmpty(&vpRect)) {
      vpRect.left -= fsm->clientRect.left;
      vpRect.top -= fsm->clientRect.top;
      vpRect.right -= fsm->clientRect.left;
      vpRect.bottom -= fsm->clientRect.top;
      Screen screen(0, vpRect.left, vpRect.top, WidthOf(vpRect),
                    HeightOf(vpRect), 0);

      // We map client screens to server screens in the server's preferred
      // order (which, in the case of the TurboVNC Server, is always the order
      // of RANDR outputs), so we send the "primary" screen (the screen
      // containing 0, 0) first.  This ensures that the window manager taskbar
      // on the server will follow the taskbar on the client.
      POINT origin = { 0, 0 };
      if (PtInRect(&mi.rcMonitor, origin))
        fsm->computedLayout.add_screen0(screen);
      else
        fsm->computedLayout.add_screen(screen);
    }
  }

  return TRUE;
}


ScreenSet ClientConnection::GetFullScreenMetrics(RECT &screenArea,
                                                 RECT &workArea, int spanMode,
                                                 bool verbose)
{
  FSMetrics fsm;
  int primaryWidth = GetSystemMetrics(SM_CXSCREEN);
  int primaryHeight = GetSystemMetrics(SM_CYSCREEN);
  int scaledWidth = m_si.framebufferWidth * m_opts.m_scale_num /
                    m_opts.m_scale_den;
  int scaledHeight = m_si.framebufferHeight * m_opts.m_scale_num /
                     m_opts.m_scale_den;

  fsm.equal = 1;
  fsm.screenArea.top = fsm.screenArea.left = 0;
  fsm.screenArea.right = primaryWidth;
  fsm.screenArea.bottom = primaryHeight;
  fsm.screenArea0 = fsm.screenArea;
  SystemParametersInfo(SPI_GETWORKAREA, 0, &fsm.workArea, 0);
  fsm.workArea0 = fsm.workArea;
  fsm.maxArea = fsm.screenNum = 0;
  fsm.fullScreen = m_opts.m_FullScreen;
  fsm.verbose = verbose;

  GetActualClientRect(&fsm.clientRect);
  POINT ul = { fsm.clientRect.left, fsm.clientRect.top };
  POINT lr = { fsm.clientRect.right, fsm.clientRect.bottom };
  ClientToScreen(m_hwnd1, &ul);
  ClientToScreen(m_hwnd1, &lr);
  SetRect(&fsm.clientRect, ul.x, ul.y, lr.x, lr.y);
  if (m_opts.m_CurrentMonitorIsPrimary) {
    GetWindowRect(m_hwnd1, &fsm.winRect);
    if (m_opts.m_FullScreen && savedRect.bottom - savedRect.top > 0 &&
        savedRect.right - savedRect.left > 0)
      fsm.winRect = savedRect;
  } else {
    fsm.winRect.left = fsm.winRect.right = -1;
    fsm.winRect.top = fsm.winRect.bottom = -1;
  }

  BOOL ret = EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&fsm);

  // Enable Primary spanning if explicitly selected, or enumerating the
  // monitors fails, or ...
  if (spanMode == SPAN_PRIMARY || !ret ||
      // Automatic spanning + Manual or Server resizing is enabled and the
      // server desktop fits on the primary monitor, or ...
      (spanMode == SPAN_AUTO && m_opts.m_desktopSize.mode != SIZE_AUTO &&
       (scaledWidth <= primaryWidth ||
        WidthOf(fsm.screenArea) <= primaryWidth) &&
       (scaledHeight <= primaryHeight ||
        HeightOf(fsm.screenArea) <= primaryHeight)) ||
      // Automatic spanning + Auto resizing is enabled and we're in windowed
      // mode
      (spanMode == SPAN_AUTO && m_opts.m_desktopSize.mode == SIZE_AUTO &&
       !m_opts.m_FullScreen)) {
    workArea = fsm.workArea0;
    screenArea = fsm.screenArea0;
  } else {
    if (fsm.equal ||
        (fsm.fullScreen && m_opts.m_desktopSize.mode == SIZE_AUTO &&
         m_serverXinerama)) {
      // All monitors are equal in resolution and aligned in a perfect grid.
      // Thus, we can extend the viewer window to all of them, both
      // horizontally and vertically (otherwise, the viewer window can only
      // be extended horizontally.)
      screenArea.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
      screenArea.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
      screenArea.right = screenArea.left +
                         GetSystemMetrics(SM_CXVIRTUALSCREEN);
      screenArea.bottom = screenArea.top +
                          GetSystemMetrics(SM_CYVIRTUALSCREEN);
    } else
      screenArea = fsm.screenArea;
    workArea = fsm.workArea;
  }

  return fsm.computedLayout;
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
    dx = BUMPSCROLLAMOUNTX * m_opts.m_scale_num / m_opts.m_scale_den;
  if (y < BUMPSCROLLBORDER)
    dy = -BUMPSCROLLAMOUNTY * m_opts.m_scale_num / m_opts.m_scale_den;
  if (y >= bottomborder)
    dy = BUMPSCROLLAMOUNTY * m_opts.m_scale_num / m_opts.m_scale_den;
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
