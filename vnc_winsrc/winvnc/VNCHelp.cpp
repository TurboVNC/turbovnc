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
#include "HtmlHelp.h"
#include "WinVNC.h"
#include "VNCHelp.h"

void VNCHelp::Popup(LPARAM lParam) 
{
	LPHELPINFO hlp = (LPHELPINFO) lParam;
	HH_POPUP popup;

	if (hlp->iCtrlId != 0) {
		
		popup.cbStruct = sizeof(popup);
		popup.hinst = hAppInstance;
		popup.idString = (UINT)hlp->iCtrlId;
		SetRect(&popup.rcMargins, -1, -1, -1, -1);
		popup.pszFont = "MS Sans Serif, 8, , ";
		popup.clrForeground = -1;
		popup.clrBackground = -1;
		popup.pt.x = -1;
		popup.pt.y = -1;

		switch  (hlp->iCtrlId) {
		case IDC_MAIN_LABEL:
			popup.idString = IDC_PORTRFB;
			break;
		case IDC_HTTP_LABEL:
			popup.idString = IDC_PORTHTTP;
			break;
		case IDC_STATIC_AND:
			popup.idString = IDC_SPECPORT;
			break;
		case IDC_PASSWORD_LABEL:
			popup.idString = IDC_PASSWORD;
			break;
		case IDC_PASSWORD_VIEWONLY_LABEL:
			popup.idString = IDC_PASSWORD_VIEWONLY;
			break;
		case IDC_TIMEOUT_LABEL:
		case IDC_SECONDS_LABEL:
			popup.idString = IDC_DISABLE_TIME;
			break;
		case IDC_STATIC_TIMEOUT:
		case IDC_STATIC_SECONDS:
			popup.idString = IDQUERYTIMEOUT;
			break;
		case IDC_STATIC_PCYCLE:
		case IDC_STATIC_MS:
		case IDC_POLCYCLMS_LABEL:
			popup.idString = IDC_POLLING_CYCLE;
			break;
		case IDC_STATIC_DRVSTATUS:
			popup.idString = IDC_STATIC_DRVINFO;
			break;
		case IDC_LIVESHARE_LABEL:
			popup.idString = IDC_LIVESHARE;
		}

		HtmlHelp((HWND)hlp->hItemHandle,
				 NULL,
				 HH_DISPLAY_TEXT_POPUP,
				 (DWORD)&popup);
	}
}

