//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
//  Copyright (C) 2010-2011 D. R. Commander. All Rights Reserved.
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
// TightVNC distribution homepage on the Web: http://www.tightvnc.com/
//
// If the source code for the VNC system is not available from the place 
// whence you received this file, check http://www.uk.research.att.com/vnc or 
// contact the authors on vnc@uk.research.att.com for information on obtaining it.
//
// Many thanks to Greg Hewgill <greg@hewgill.com> for providing the basis for 
// the full-screen mode.

#include "stdhdrs.h"
#include "vncviewer.h"
#include "ClientConnection.h"

// Parameters for scrolling in full screen mode
#define BUMPSCROLLBORDER 4
#define BUMPSCROLLAMOUNTX 8
#define BUMPSCROLLAMOUNTY 6

bool ClientConnection::InFullScreenMode() 
{
	return m_opts.m_FullScreen; 
};

// You can explicitly change mode by calling this
void ClientConnection::SetFullScreenMode(bool enable, bool suppressPrompt)
{	
	m_opts.m_FullScreen = enable;
	RealiseFullScreenMode(suppressPrompt);
}

// If the options have been changed other than by calling 
// SetFullScreenMode, you need to call this to make it happen.
void ClientConnection::RealiseFullScreenMode(bool suppressPrompt)
{
	LONG style = GetWindowLong(m_hwnd1, GWL_STYLE);
	if (m_opts.m_FullScreen) {
		if (!suppressPrompt && !pApp->m_options.m_skipprompt) {
			MessageBox(m_hwnd1, 
				_T("To exit from full-screen mode, press Ctrl-Alt-Shift-F.\r\n"
				"Alternatively, press Ctrl-Esc Esc and then right-click\r\n"
				"on the vncviewer taskbar icon to see the menu."),
				_T("VNCviewer full-screen mode"),
				MB_OK | MB_ICONINFORMATION | MB_TOPMOST | MB_SETFOREGROUND);
		}
		ShowWindow(m_hToolbar, SW_HIDE);
		EnableMenuItem(GetSystemMenu(m_hwnd1, FALSE), ID_TOOLBAR, MF_BYCOMMAND|MF_GRAYED);
		style = GetWindowLong(m_hwnd1, GWL_STYLE);
		style &= ~(WS_DLGFRAME | WS_THICKFRAME | WS_BORDER);
		
		SetWindowLong(m_hwnd1, GWL_STYLE, style);
		RECT screenArea, workArea;
		GetFullScreenMetrics(screenArea, workArea);
		SetWindowPos(m_hwnd1, HWND_TOPMOST, screenArea.left, screenArea.top,
			screenArea.right - screenArea.left,
			screenArea.bottom - screenArea.top, SWP_FRAMECHANGED);
		CheckMenuItem(GetSystemMenu(m_hwnd1, FALSE), ID_FULLSCREEN, MF_BYCOMMAND|MF_CHECKED);
		
	} else {
		ShowWindow(m_hToolbar, SW_SHOW);
		EnableMenuItem(GetSystemMenu(m_hwnd1, FALSE), ID_TOOLBAR, MF_BYCOMMAND|MF_ENABLED);
		style |= (WS_DLGFRAME | WS_THICKFRAME | WS_BORDER);
		
		SetWindowLong(m_hwnd1, GWL_STYLE, style);
		ShowWindow(m_hwnd1, SW_NORMAL);		
		SizeWindow(true);
		CheckMenuItem(GetSystemMenu(m_hwnd1, FALSE), ID_FULLSCREEN, MF_BYCOMMAND|MF_UNCHECKED);

	}
}

typedef struct _FSMetrics {
	RECT screenArea, workArea;
	bool equal;
	int primaryWidth, primaryHeight;
} FSMetrics;

static BOOL MonitorEnumProc(HMONITOR hmon, HDC hdc, LPRECT rect, LPARAM data)
{
	FSMetrics *fsm = (FSMetrics *)data;
	MONITORINFO mi;

	memset(&mi, 0, sizeof(MONITORINFO));
	mi.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(hmon, &mi);

	if ((mi.rcMonitor.right - mi.rcMonitor.left != fsm->primaryWidth) ||
		(mi.rcMonitor.bottom - mi.rcMonitor.top != fsm->primaryHeight))
		fsm->equal = 0;

	if (mi.rcMonitor.top >= 0 && mi.rcMonitor.bottom <= fsm->primaryHeight &&
		mi.rcMonitor.left >= 0) {
		fsm->screenArea.top = max(mi.rcMonitor.top, fsm->screenArea.top);
		fsm->screenArea.left = min(mi.rcMonitor.left, fsm->screenArea.left);
		fsm->screenArea.right = max(mi.rcMonitor.right, fsm->screenArea.right);
		fsm->screenArea.bottom = min(mi.rcMonitor.bottom, fsm->screenArea.bottom);
	}

	if (mi.rcWork.top == 0 && mi.rcWork.left >= 0) {
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
	fsm.primaryWidth = GetSystemMetrics(SM_CXSCREEN);
	fsm.primaryHeight = GetSystemMetrics(SM_CYSCREEN);
	int scaledWidth = m_si.framebufferWidth * m_opts.m_scale_num /
		m_opts.m_scale_den;
	int scaledHeight = m_si.framebufferHeight * m_opts.m_scale_num /
		m_opts.m_scale_den;
	fsm.equal = 1;
	fsm.screenArea.top = fsm.screenArea.left = 0;
	fsm.screenArea.right = fsm.primaryWidth;
	fsm.screenArea.bottom = fsm.primaryHeight;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &fsm.workArea, 0);

	if (m_opts.m_Span == SPAN_PRIMARY ||
	    (m_opts.m_Span == SPAN_AUTO &&
	     scaledWidth <= fsm.primaryWidth && scaledHeight <= fsm.primaryHeight) ||
	    !EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&fsm)) {
		SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
		screenArea.left = screenArea.top = 0;
		screenArea.right = fsm.primaryWidth;
		screenArea.bottom = fsm.primaryHeight;
	} else {
		if (fsm.equal) {
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
		if (ScrollScreen(dx,dy)) {
			// If we haven't physically moved the cursor, artificially
			// generate another mouse event so we keep scrolling.
			POINT p;
			GetCursorPos(&p);
			if (p.x == x && p.y == y)
				SetCursorPos(x,y);
			return true;
		} 
	}
	return false;
}
