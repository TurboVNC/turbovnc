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

// QuerySettingsControls.cpp: implementation of the QuerySettingsControls class.


#include "QuerySettingsControls.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QuerySettingsControls::QuerySettingsControls(HWND hwnd, vncServer * server)
{
	m_server = server;
	m_hwnd = hwnd;
	Init();
}

void QuerySettingsControls::Validate()
{
	BOOL enable = IsChecked(IDQUERY);
	Enable(IDC_STATIC_TIMEOUT, enable);
	Enable(IDC_STATIC_ACTION, enable);
	Enable(IDC_STATIC_SECONDS, enable);
	Enable(IDQUERYTIMEOUT, enable);
	Enable(IDC_ACTION_REFUSE, enable);
	Enable(IDC_ACTION_ACCEPT, enable);
	Enable(IDQUERYALLOWNOPASS, enable);
	SetFocus(GetDlgItem(m_hwnd, IDQUERYTIMEOUT));
	SendMessage(GetDlgItem(m_hwnd, IDQUERYTIMEOUT), EM_SETSEL, 0, (LPARAM)-1);
}
void QuerySettingsControls::Apply()
{
	// Save the timeout
	char timeout[256];
	if (GetDlgItemText(m_hwnd, IDQUERYTIMEOUT, (LPSTR) &timeout, 256) != 0)
		m_server->SetQueryTimeout(atoi(timeout));
					
	// Save the new settings to the server
	m_server->SetQuerySetting((IsChecked(IDQUERY))? 4 : 2);									
	m_server->SetQueryAccept(IsChecked(IDC_ACTION_ACCEPT));
	m_server->SetQueryAllowNoPass(IsChecked(IDQUERYALLOWNOPASS));				
}
void QuerySettingsControls::Init()
{
	SetChecked(IDQUERY, (m_server->QuerySetting() == 4));	
	SetChecked(((m_server->QueryAccept()) ?
				IDC_ACTION_ACCEPT : IDC_ACTION_REFUSE), TRUE);			
	SetChecked(IDQUERYALLOWNOPASS, m_server->QueryAllowNoPass());		
		
	// Get the timeout
	char timeout[128];
	UINT t;
	t = m_server->QueryTimeout();
	sprintf(timeout, "%d", (int)t);
	SetDlgItemText(m_hwnd, IDQUERYTIMEOUT, (const char *) timeout);

	Validate();
}

QuerySettingsControls::~QuerySettingsControls()
{

}
