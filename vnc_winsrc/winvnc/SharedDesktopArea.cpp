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

// SharedDesktopArea.cpp: implementation of the SharedDesktopArea class.


#include "SharedDesktopArea.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SharedDesktopArea::SharedDesktopArea(HWND hwnd,
									 CMatchWindow *matchwindow,
									 vncProperties *vncprop,
									 vncServer *server)
{
	m_hwnd = hwnd;
	m_pMatchWindow = matchwindow;
	m_vncprop = vncprop;
	m_server = server;

	m_bCaptured = FALSE;
	m_KeepHandle = NULL;

	// If a matchwindow wasn't passed in, we're responsible for it
	m_deleteMatchWindow = (matchwindow == NULL);

	Init();
}

SharedDesktopArea::~SharedDesktopArea()
{
	if (m_deleteMatchWindow && m_pMatchWindow != NULL)
		delete m_pMatchWindow;
}

void SharedDesktopArea::Init()
{
	//
	// configure select window picture
	//

	HWND bmp_hWnd = GetDlgItem(m_hwnd, IDC_BMPCURSOR);
	m_OldBmpWndProc = GetWindowLong(bmp_hWnd, GWL_WNDPROC);
	SetWindowLong(bmp_hWnd, GWL_WNDPROC, (LONG)BmpWndProc);
	SetWindowLong(bmp_hWnd, GWL_USERDATA, (LONG)this);

	//
	// setup match window
	//

	if (m_pMatchWindow == NULL)
		SetupMatchWindow();

	m_pMatchWindow->CanModify(TRUE);
	m_pMatchWindow->Hide();

	//
	// initialize controls
	//

	BOOL propFullScreen = m_vncprop->GetPrefFullScreen();
	BOOL propWindowShared = m_vncprop->GetPrefWindowShared();
	BOOL propAreaShared = m_vncprop->GetPrefScreenAreaShared();

	SendDlgItemMessage(m_hwnd, IDC_FULLSCREEN, BM_SETCHECK, propFullScreen, 0);
	SendDlgItemMessage(m_hwnd, IDC_WINDOW, BM_SETCHECK, propWindowShared, 0);
	SendDlgItemMessage(m_hwnd, IDC_SCREEN, BM_SETCHECK, propAreaShared, 0);

	m_hWindowName = GetDlgItem(m_hwnd, IDC_NAME_APPLI);
	EnableWindow(m_hWindowName, propWindowShared);

	//
	// toggle selected option
	//

	if (propFullScreen) {
		FullScreen();
	} else if (propAreaShared) {
		SharedScreen();
	} else { // if (propWindowShared)
		SharedWindow();
	}

	// bring dialog to the front
	SetForegroundWindow(m_hwnd);
}

void SharedDesktopArea::SetupMatchWindow()
{
	// get the desktop's bounds
	RECT desktopRect;
	GetWindowRect(GetDesktopWindow(), &desktopRect);

	//
	// instantiate object with default bounds
	// ( i.e. top-left quadrant of the desktop )
	//

	m_pMatchWindow = new CMatchWindow(m_server, 
									  desktopRect.left+5, desktopRect.top+5, 
									  desktopRect.right/2, desktopRect.bottom/2);

	//
	// if region sharing is the current setting,
	// the server rect is not whole screen, 
	// and the server rect is not empty, 
	// then use the server's bounds
	//

	if (m_vncprop->GetPrefScreenAreaShared())
	{
		// get the server's current rect
		RECT serverRect;
		serverRect = m_server->GetSharedRect();

		if ( EqualRect(&desktopRect, &serverRect) == FALSE &&
			 IsRectEmpty(&serverRect) == FALSE ) {
			// use it for match window
			m_pMatchWindow->ModifyPosition(serverRect.left, serverRect.top,
										   serverRect.right, serverRect.bottom);
		}
	}
}

bool SharedDesktopArea::ApplySharedControls()
{
	if (m_vncprop->GetPrefWindowShared() && m_server->GetWindowShared() == NULL) {	
		MessageBox(NULL,
				"You have not yet selected a window to share.\n"
				"Please first select a window with the 'Window Target'\n"
				"icon, and try again.", "No Window Selected",
				 MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	// Handle the share one window stuff
	HWND hFullScreen = GetDlgItem(m_hwnd, IDC_FULLSCREEN);
	m_server->FullScreen(SendMessage(hFullScreen,
									 BM_GETCHECK, 0, 0) == BST_CHECKED);

	HWND hWindowCapture = GetDlgItem(m_hwnd, IDC_WINDOW);
	m_server->WindowShared(SendMessage(hWindowCapture,
									   BM_GETCHECK, 0, 0) == BST_CHECKED);

	HWND hScreenArea = GetDlgItem(m_hwnd, IDC_SCREEN);
	m_server->ScreenAreaShared(SendMessage(hScreenArea,
										   BM_GETCHECK, 0, 0) == BST_CHECKED);
				
	if (m_vncprop->GetPrefScreenAreaShared()) {
		int left,right,top,bottom;
		m_pMatchWindow->GetPosition(left,top,right,bottom);
		m_server->SetMatchSizeFields(left,top,right,bottom);
	}

	if (m_vncprop->GetPrefFullScreen()) {
		RECT temp;
		GetWindowRect(GetDesktopWindow(), &temp);
		m_server->SetMatchSizeFields(temp.left, temp.top, temp.right, temp.bottom);
	}

	return true;
}

void SharedDesktopArea::FullScreen()
{
	// disable window cursor
	HWND bmp_hWnd = GetDlgItem(m_hwnd, IDC_BMPCURSOR);
	EnableWindow(bmp_hWnd, FALSE);

	// change cursor image
	HBITMAP hNewImage = LoadBitmap(hAppInstance, MAKEINTRESOURCE(IDB_BITMAP3));
	HBITMAP hOldImage = (HBITMAP)::SendMessage(bmp_hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNewImage);
	DeleteObject(hOldImage);

	// set window text name
	EnableWindow(m_hWindowName, FALSE);
	::SetWindowText(m_hWindowName, "* full desktop selected *");

	// hide match window
	m_pMatchWindow->Hide();

	// update properties
	m_vncprop->SetPrefFullScreen(TRUE);
	m_vncprop->SetPrefWindowShared(FALSE);
	m_vncprop->SetPrefScreenAreaShared(FALSE);
}

void SharedDesktopArea::SharedWindow()
{
	// enable window cursor
	HWND bmp_hWnd = GetDlgItem(m_hwnd, IDC_BMPCURSOR);
	EnableWindow(bmp_hWnd, TRUE);

	// change cursor image
	HBITMAP hNewImage = LoadBitmap(hAppInstance, MAKEINTRESOURCE(IDB_BITMAP1));
	HBITMAP hOldImage = (HBITMAP)::SendMessage(bmp_hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNewImage);
	DeleteObject(hOldImage);

	// set window text name
	EnableWindow(m_hWindowName, FALSE);
	SetWindowCaption(m_server->GetWindowShared());
	EnableWindow(m_hWindowName, TRUE);

	// hide match window
	m_pMatchWindow->Hide();

	// update properties
	m_vncprop->SetPrefFullScreen(FALSE);
	m_vncprop->SetPrefWindowShared(TRUE);
	m_vncprop->SetPrefScreenAreaShared(FALSE);
}

void SharedDesktopArea::SharedScreen()
{
	// disable window cursor
	HWND bmp_hWnd = GetDlgItem(m_hwnd, IDC_BMPCURSOR);
	EnableWindow(bmp_hWnd, FALSE);

	// change cursor image
	HBITMAP hNewImage = LoadBitmap(hAppInstance, MAKEINTRESOURCE(IDB_BITMAP3));
	HBITMAP hOldImage = (HBITMAP)::SendMessage(bmp_hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNewImage);
	DeleteObject(hOldImage);

	// set window text name
	EnableWindow(m_hWindowName, FALSE);
	::SetWindowText(m_hWindowName, "* screen area selected *");

	// show match window
	m_pMatchWindow->Show();

	// update properties
	m_vncprop->SetPrefFullScreen(FALSE);
	m_vncprop->SetPrefWindowShared(FALSE);
	m_vncprop->SetPrefScreenAreaShared(TRUE);
}

LRESULT CALLBACK SharedDesktopArea::BmpWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HBITMAP hNewImage, hOldImage;
	HCURSOR hNewCursor, hOldCursor;
	SharedDesktopArea* pDialog = (SharedDesktopArea*) GetWindowLong(hWnd, GWL_USERDATA);

	switch (message) {

	case WM_SETCURSOR :
		if (HIWORD(lParam) == WM_LBUTTONDOWN) {
			SetCapture(hWnd);
			hNewImage = LoadBitmap(hAppInstance, MAKEINTRESOURCE(IDB_BITMAP2));
			hOldImage = (HBITMAP)::SendMessage(hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNewImage);
			DeleteObject(hOldImage);
			hNewCursor = (HCURSOR)LoadImage(hAppInstance, MAKEINTRESOURCE(IDC_CURSOR1),
											IMAGE_CURSOR, 32, 32, LR_DEFAULTCOLOR);
			hOldCursor = SetCursor(hNewCursor);
			DestroyCursor(hOldCursor);
			pDialog->m_bCaptured = TRUE;
		}
		break;

	case WM_LBUTTONUP:
		ReleaseCapture();
		
		if (pDialog->m_KeepHandle != NULL) {
			// We need to remove frame
			DrawFrameAroundWindow(pDialog->m_KeepHandle);
			pDialog->m_server->SetWindowShared(pDialog->m_KeepHandle);
			// No more need
			pDialog->m_KeepHandle = NULL;
		} else {
			pDialog->m_server->SetWindowShared(NULL);
		}

		hNewImage = LoadBitmap(hAppInstance, MAKEINTRESOURCE(IDB_BITMAP1));
		hOldImage = (HBITMAP)::SendMessage(hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNewImage);
		DeleteObject(hOldImage);
		hNewCursor = LoadCursor(hAppInstance, MAKEINTRESOURCE(IDC_ARROW));
		hOldCursor = SetCursor(hNewCursor);
		DestroyCursor(hOldCursor);
		pDialog->m_bCaptured = FALSE;
		break;
	
	case WM_MOUSEMOVE:
		if (pDialog->m_bCaptured) {
			HWND hParent = ::GetParent(hWnd);
			POINTS pnt;
			POINT pnt1;
			pnt = MAKEPOINTS(lParam);
			pnt1.x = pnt.x;
			pnt1.y = pnt.y;
			ClientToScreen(hWnd, &pnt1);
			HWND hMouseWnd=::WindowFromPoint(pnt1);
			if (pDialog->m_KeepHandle != hMouseWnd) {
				// New Windows Handle
				// Was KeepHandle A Real Window ?
				if (pDialog->m_KeepHandle != NULL) {
					// We need to remove frame
					SharedDesktopArea::DrawFrameAroundWindow(pDialog->m_KeepHandle);
				}
				pDialog->m_KeepHandle = hMouseWnd;
				if (!IsChild(hParent, hMouseWnd) && hMouseWnd != hParent) {
					pDialog->SetWindowCaption(hMouseWnd);
					SharedDesktopArea::DrawFrameAroundWindow(hMouseWnd);
				} else {	// My Window
					pDialog->m_KeepHandle = NULL;
					pDialog->SetWindowCaption(NULL);
				}
			}
		}
		break;

	case WM_PAINT:
	case STM_SETIMAGE:
		return CallWindowProc((WNDPROC)pDialog->m_OldBmpWndProc,
							  hWnd, message, wParam, lParam);
	
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


void SharedDesktopArea::DrawFrameAroundWindow(HWND hWnd)
{
	HDC hWindowDc=::GetWindowDC(hWnd);
	HBRUSH hBrush=CreateSolidBrush(RGB(0,0,0));
	HRGN Rgn=CreateRectRgn(0,0,1,1);
	int iRectResult=GetWindowRgn(hWnd,Rgn);
	if (iRectResult==ERROR || iRectResult==NULLREGION || Rgn==NULL)
	{
		RECT rect;
		GetWindowRect(hWnd,&rect);
		OffsetRect(&rect,-rect.left,-rect.top);
		Rgn=CreateRectRgn(rect.left,rect.top,rect.right,rect.bottom);
	}
	
	SetROP2(hWindowDc,R2_MERGEPENNOT);
	FrameRgn(hWindowDc,Rgn,hBrush,3,3);
	::DeleteObject(Rgn);
	::DeleteObject(hBrush);
    ::ReleaseDC(hWnd,hWindowDc);
}

void SharedDesktopArea::SetWindowCaption(HWND hWnd)
{
	char strWindowText[256];
	if (hWnd == NULL) {
		strcpy(strWindowText, "* no window selected *");
	} else {
		GetWindowText(hWnd, strWindowText, sizeof(strWindowText));
		if (!strWindowText[0]) 
		{
			int bytes = sprintf(strWindowText, "0x%x ", hWnd);
			GetClassName(hWnd, strWindowText + bytes,
						 sizeof(strWindowText) - bytes);
		}
	}

	if (m_hWindowName)
		::SetWindowText(m_hWindowName, strWindowText);
}

