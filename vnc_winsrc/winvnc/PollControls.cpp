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

// PollControls.cpp: implementation of the PollControls class.


#include "PollControls.h"

PollControls::PollControls(HWND hwnd, vncServer *server)
{	
	m_hwnd = hwnd;
	m_server = server;

	SetChecked(IDC_POLL_FULLSCREEN, m_server->PollFullScreen());
	SetChecked(IDC_POLL_FOREGROUND, m_server->PollForeground());
	SetChecked(IDC_POLL_UNDER_CURSOR, m_server->PollUnderCursor());
	SetChecked(IDC_CONSOLE_ONLY, m_server->PollConsoleOnly());
	SetChecked(IDC_ONEVENT_ONLY, m_server->PollOnEventOnly());
	SetChecked(IDC_DONT_SET_HOOKS, m_server->DontSetHooks());
	SetChecked(IDC_DONT_USE_DRIVER, m_server->DontUseDriver());
	SetChecked(IDC_DRIVER_DIRECT_ACCESS, m_server->DriverDirectAccess());
	SetDlgItemInt(m_hwnd, IDC_POLLING_CYCLE, m_server->GetPollingCycle(), FALSE);

	if (m_server->DesktopActive()) {
		if (m_server->DriverActive()) {
			SetDlgItemText(hwnd, IDC_STATIC_DRVINFO, "Driver is in use");
		} else {
			SetDlgItemText(hwnd, IDC_STATIC_DRVINFO, "Driver is not used");
		}
	}

	Validate();
}

void PollControls::Validate()
{
	BOOL full_polling =
		IsChecked(IDC_POLL_FULLSCREEN);
	BOOL window_polling =
		IsChecked(IDC_POLL_FOREGROUND) || IsChecked(IDC_POLL_UNDER_CURSOR);	

	Enable(IDC_POLL_FOREGROUND,   !full_polling);
	Enable(IDC_POLL_UNDER_CURSOR, !full_polling);

	Enable(IDC_CONSOLE_ONLY,      !full_polling && window_polling);
	Enable(IDC_ONEVENT_ONLY,      !full_polling && window_polling);

	Enable(IDC_POLLING_CYCLE,     full_polling || window_polling);
	Enable(IDC_STATIC_PCYCLE,     full_polling || window_polling);
	Enable(IDC_STATIC_MS,         full_polling || window_polling);

	Enable(IDC_DONT_SET_HOOKS,    full_polling);
}

void PollControls::Apply()
{
	m_server->PollFullScreen(IsChecked(IDC_POLL_FULLSCREEN));
	m_server->PollForeground(IsChecked(IDC_POLL_FOREGROUND));
	m_server->PollUnderCursor(IsChecked(IDC_POLL_UNDER_CURSOR));
	m_server->PollConsoleOnly(IsChecked(IDC_CONSOLE_ONLY));
	m_server->PollOnEventOnly(IsChecked(IDC_ONEVENT_ONLY));

	// This should appear AFTER calling m_server->PollFullScreen(...)
	m_server->DontSetHooks(IsChecked(IDC_DONT_SET_HOOKS));
	m_server->DontUseDriver(IsChecked(IDC_DONT_USE_DRIVER));
	m_server->DriverDirectAccess(IsChecked(IDC_DRIVER_DIRECT_ACCESS));

	BOOL success;
	UINT pollingCycle = GetDlgItemInt(m_hwnd, IDC_POLLING_CYCLE, &success, TRUE);
	if (success)
		m_server->SetPollingCycle(pollingCycle);
}

