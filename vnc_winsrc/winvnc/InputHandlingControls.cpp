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

// InputHandlingControls.cpp: implementation of the InputHandlingControls class.


#include "InputHandlingControls.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

InputHandlingControls::InputHandlingControls(HWND hwnd, vncServer *server)
{
	m_server = server;

	// Remote input settings
	HWND hEnableRemoteInputs = GetDlgItem(hwnd, IDC_DISABLE_INPUTS);
	SendMessage(hEnableRemoteInputs,
				BM_SETCHECK,
				!(m_server->RemoteInputsEnabled()),
				0);

	// Local input settings
	HWND hDisableLocalInputs = GetDlgItem(hwnd, IDC_DISABLE_LOCAL_INPUTS);
	SendMessage(hDisableLocalInputs,
				BM_SETCHECK,
				m_server->LocalInputsDisabled(),
				0);

	// Local input prioity settings
	HWND hRemoteDisable = GetDlgItem(hwnd, IDC_REMOTE_DISABLE);
	SendMessage(hRemoteDisable,
				BM_SETCHECK,
				m_server->LocalInputPriority(),
				0);

	HWND hDisableTime = GetDlgItem(hwnd, IDC_DISABLE_TIME);
	SetDlgItemInt(hwnd, IDC_DISABLE_TIME, m_server->DisableTime(), FALSE);
			
	EnableRemote(hwnd, false);
	EnableInputs(hwnd);
}
void InputHandlingControls::ApplyInputsControlsContents(HWND hwnd)
{
	// Remote input stuff
	HWND hEnableRemoteInputs = GetDlgItem(hwnd, IDC_DISABLE_INPUTS);
	m_server->EnableRemoteInputs(SendMessage(hEnableRemoteInputs,
								BM_GETCHECK, 0, 0) != BST_CHECKED);
	// Local input stuff
	HWND hDisableLocalInputs = GetDlgItem(hwnd, IDC_DISABLE_LOCAL_INPUTS);
	m_server->DisableLocalInputs(SendMessage(hDisableLocalInputs,
								BM_GETCHECK, 0, 0) == BST_CHECKED);
	HWND hRemoteDisable = GetDlgItem(hwnd, IDC_REMOTE_DISABLE);
	m_server->LocalInputPriority(SendMessage(hRemoteDisable,
								BM_GETCHECK, 0, 0) == BST_CHECKED);

	BOOL success;
	UINT disabletime = GetDlgItemInt(hwnd, IDC_DISABLE_TIME, &success, TRUE);
	if (success)
		m_server->SetDisableTime(disabletime);
}

void InputHandlingControls::EnableInputs(HWND hwnd)
{
	HWND hDisableInputs = GetDlgItem(hwnd, IDC_DISABLE_INPUTS);
	HWND hDisableLocalInputs = GetDlgItem(hwnd, IDC_DISABLE_LOCAL_INPUTS);
	
	// Determine whether to enable the modifier options
	BOOL enabled = (SendMessage(hDisableInputs, BM_GETCHECK, 0, 0) != BST_CHECKED) &&
	(SendMessage(hDisableLocalInputs, BM_GETCHECK, 0, 0) != BST_CHECKED);

	HWND hRemoteDisable = GetDlgItem(hwnd, IDC_REMOTE_DISABLE);
	HWND hDisableTime = GetDlgItem(hwnd, IDC_DISABLE_TIME);
				
	BOOL enabl = (SendMessage(hRemoteDisable, BM_GETCHECK, 0, 0) != BST_CHECKED);
								
	EnableWindow(hRemoteDisable, enabled);
	EnableTime(hwnd, enabled & !enabl);
				
	if (!enabled)
		SendMessage(hRemoteDisable, BM_SETCHECK, enabled, 0);
}

void InputHandlingControls::EnableRemote(HWND hwnd, bool mayMoveFocus)
{
	HWND hDisableTime = GetDlgItem(hwnd, IDC_DISABLE_TIME);
	HWND hRemoteDisable = GetDlgItem(hwnd, IDC_REMOTE_DISABLE);
	bool enabled = (SendMessage(hRemoteDisable, BM_GETCHECK, 0, 0) == BST_CHECKED);
	EnableTime(hwnd, enabled);
	if (enabled && mayMoveFocus) {
		SetFocus(hDisableTime);
		SendMessage(hDisableTime, EM_SETSEL, 0, (LPARAM)-1);
	}
}

void InputHandlingControls::EnableTime(HWND hwnd, bool enable)
{
	HWND hDisableTime = GetDlgItem(hwnd, IDC_DISABLE_TIME);
	HWND hTimeLebel = GetDlgItem(hwnd, IDC_TIMEOUT_LABEL);
	HWND hSecondLebel = GetDlgItem(hwnd, IDC_SECONDS_LABEL);
	EnableWindow(hDisableTime, enable);
	EnableWindow(hTimeLebel, enable);
	EnableWindow(hSecondLebel, enable);
}
InputHandlingControls::~InputHandlingControls()
{

}
