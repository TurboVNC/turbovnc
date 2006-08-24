// Copyright (C) 2003 TightVNC Development Team. All Rights Reserved.
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

// VNCHelp.cpp: implementation of the VNCHelp class.

#include "stdhdrs.h"
#include "Htmlhelp.h"
#include "vncviewer.h"
#include "VNCHelp.h"

VNCHelp::VNCHelp()
{
	m_dwCookie = NULL;
	HtmlHelp(NULL, NULL, HH_INITIALIZE, (DWORD)&m_dwCookie);
}

void VNCHelp::Popup(LPARAM lParam) 
{
	LPHELPINFO hlp = (LPHELPINFO) lParam;
	HH_POPUP popup;

	if (hlp->iCtrlId != 0) {
		
		popup.cbStruct = sizeof(popup);
		popup.hinst = pApp->m_instance;
		popup.idString = (UINT)hlp->iCtrlId;
		SetRect(&popup.rcMargins, -1, -1, -1, -1);
		popup.pszFont = "MS Sans Serif, 8, , ";
		popup.clrForeground = -1;
		popup.clrBackground = -1;
		popup.pt.x = -1;
		popup.pt.y = -1;

		switch  (hlp->iCtrlId) {
		case IDC_STATIC_LEVEL:
		case IDC_STATIC_TEXT_LEVEL:
		case IDC_STATIC_FAST:
		case IDC_STATIC_BEST:
			popup.idString = IDC_COMPRESSLEVEL;
			break;
		case IDC_STATIC_QUALITY:
		case IDC_STATIC_TEXT_QUALITY:
		case IDC_STATIC_POOR:
		case IDC_STATIC_QBEST:
			popup.idString = IDC_QUALITYLEVEL;
			break;
		case IDC_STATIC_ENCODING:
			popup.idString = IDC_ENCODING;
			break;
		case IDC_STATIC_SCALE:
		case IDC_STATIC_P:
			popup.idString = IDC_SCALE_EDIT;
			break;
		case IDC_STATIC_SERVER:
			popup.idString = IDC_HOSTNAME_EDIT;
			break;
		case IDC_STATIC_LIST:
		case IDC_SPIN1:
			popup.idString = IDC_EDIT_AMOUNT_LIST;
			break;
		case IDC_STATIC_LOG_LEVEL:
		case IDC_SPIN2:
			popup.idString = IDC_EDIT_LOG_LEVEL;
			break;
		case IDC_SPIN3:
		case IDC_STATIC_PORT:
			popup.idString = IDC_LISTEN_PORT;
			break;
		}

		HtmlHelp((HWND)hlp->hItemHandle,
				 NULL,
				 HH_DISPLAY_TEXT_POPUP,
				 (DWORD)&popup);
	}
}

BOOL VNCHelp::TranslateMsg(MSG *pmsg)
{
	return (HtmlHelp(NULL, NULL, HH_PRETRANSLATEMESSAGE, (DWORD)pmsg) != 0);
}

VNCHelp::~VNCHelp()
{
	HtmlHelp(NULL, NULL, HH_UNINITIALIZE, (DWORD)m_dwCookie);
}

