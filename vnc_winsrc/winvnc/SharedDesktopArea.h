// Copyright (C) 2004 TightVNC Development Team. All Rights Reserved.
//
//  TightVNC is free software; you can redistribute it and/or modify
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
// TightVNC homepage on the Web: http://www.tightvnc.com/

// SharedDesktopArea.h: interface for the SharedDesktopArea class.

#ifndef SHAREDDESKTOPAREA_H__
#define SHAREDDESKTOPAREA_H__

class SharedDesktopArea;

#include "resource.h"

#include "WinVNC.h"
#include "vncServer.h"
#include "vncProperties.h"
#include "MatchWindow.h"

class SharedDesktopArea  
{
public:
	SharedDesktopArea(HWND hwnd, CMatchWindow *matchwindow,
					  vncProperties *vncprop, vncServer *server);
	bool ApplySharedControls();
	void FullScreen();
	void SharedWindow();
	void SharedScreen();
	virtual ~SharedDesktopArea();

protected:
	void SetWindowCaption(HWND hWnd);
	HWND m_hWindowName;

private:
	void Init();
	void SetupMatchWindow();
	static void DrawFrameAroundWindow(HWND hWnd);
	static LRESULT CALLBACK BmpWndProc(HWND, UINT, WPARAM, LPARAM);

	HWND m_hwnd;
	LONG m_OldBmpWndProc;
	BOOL m_bCaptured;
	HWND m_KeepHandle;
	vncServer *m_server;
	vncProperties *m_vncprop;

	CMatchWindow *m_pMatchWindow;
	bool m_deleteMatchWindow;
};

#endif
