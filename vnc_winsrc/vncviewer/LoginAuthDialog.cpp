//  Copyright (C) 2003 Constantin Kaplinsky. All Rights Reserved.
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
// TightVNC distribution homepage on the Web: http://www.tightvnc.com/
//
// If the source code for the VNC system is not available from the place 
// whence you received this file, check http://www.uk.research.att.com/vnc or contact
// the authors on vnc@uk.research.att.com for information on obtaining it.


// LoginAuthDialog.cpp: implementation of the dialog box for authentication
// with a username/password pair.

#include "stdhdrs.h"
#include "vncviewer.h"
#include "LoginAuthDialog.h"
#include "Exception.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LoginAuthDialog::LoginAuthDialog(char *vnchost, char *title, char *username)
{
	if (title != NULL) {
		strncpy(m_title, title, sizeof(m_title)-1);
		m_title[sizeof(m_title)-1] = '\0';
	} else {
		m_title[0] = '\0';
	}

	if (username == NULL || username[0] == '\0') {
		m_username[0] = TEXT('\0');
	} else {
		_tcsncpy(m_username, username, 255);
		m_username[255] = TEXT('\0');
	}
	m_passwd[0] = TEXT('\0');
	m_vnchost = (vnchost != NULL) ? vnchost : "[unknown]";
}

LoginAuthDialog::~LoginAuthDialog()
{
}

int LoginAuthDialog::DoDialog()
{
	return DialogBoxParam(pApp->m_instance,
						  DIALOG_MAKEINTRESOURCE(IDD_LOGIN_AUTH_DIALOG), 
						  NULL, (DLGPROC)DlgProc, (LONG)this);
}

BOOL CALLBACK LoginAuthDialog::DlgProc(HWND hwnd, UINT uMsg,
									   WPARAM wParam, LPARAM lParam) {
	// This is a static method, so we don't know which instantiation we're 
	// dealing with. But we can get a pseudo-this from the parameter to 
	// WM_INITDIALOG, which we therafter store with the window and retrieve
	// as follows:
	LoginAuthDialog *_this =
		(LoginAuthDialog *)GetWindowLong(hwnd, GWL_USERDATA);

	switch (uMsg) {
	case WM_INITDIALOG:
		SetWindowLong(hwnd, GWL_USERDATA, lParam);
		_this = (LoginAuthDialog *)lParam;
		if (_this->m_title[0] != '\0')
			SetWindowText(hwnd, _this->m_title);
		SetDlgItemText(hwnd, IDC_VNCHOST, _this->m_vnchost);
		CentreWindow(hwnd);
		if (_this->m_username[0] != '\0') {
			SetDlgItemText(hwnd, IDC_LOGIN_EDIT, _this->m_username);
			SetFocus(GetDlgItem(hwnd, IDC_PASSWD_EDIT));
			return FALSE;
		}
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			GetDlgItemText(hwnd, IDC_LOGIN_EDIT, _this->m_username, 256);
			GetDlgItemText(hwnd, IDC_PASSWD_EDIT, _this->m_passwd, 256);
			EndDialog(hwnd, TRUE);
			return TRUE;
		case IDCANCEL:
			EndDialog(hwnd, FALSE);
			throw QuietException("User canceled authentication.");
			return TRUE;
		}
		break;
	case WM_DESTROY:
		EndDialog(hwnd, FALSE);
		return TRUE;
	}
	return 0;
}

