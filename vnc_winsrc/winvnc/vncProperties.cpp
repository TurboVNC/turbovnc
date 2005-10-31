//  Copyright (C) 2002-2003 Constantin Kaplinsky. All Rights Reserved.
//  Copyright (C) 2002 RealVNC Ltd. All Rights Reserved.
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


// vncProperties.cpp

// Implementation of the Properties dialog!

#include "stdhdrs.h"
#include "lmcons.h"
#include "vncService.h"

#include "WinVNC.h"
#include "vncProperties.h"
#include "vncServer.h"
#include "vncPasswd.h"
#include "commctrl.h"

const char NO_PASSWORD_WARN [] = "WARNING : Running WinVNC without setting a password is "
								"a dangerous security risk!\n"
								"Until you set a password, WinVNC will not accept incoming connections.";
const char NO_OVERRIDE_ERR [] = "This machine has been preconfigured with WinVNC settings, "
								"which cannot be overridden by individual users.  "
								"The preconfigured settings may be modified only by a System Administrator.";
const char NO_PASSWD_NO_OVERRIDE_ERR [] =
								"No password has been set & this machine has been "
								"preconfigured to prevent users from setting their own.\n"
								"You must contact a System Administrator to configure WinVNC properly.";
const char NO_PASSWD_NO_OVERRIDE_WARN [] =
								"WARNING : This machine has been preconfigured to allow un-authenticated\n"
								"connections to be accepted and to prevent users from enabling authentication.";
const char NO_PASSWD_NO_LOGON_WARN [] =
								"WARNING : This machine has no default password set.  WinVNC will present the "
								"Default Properties dialog now to allow one to be entered.";
const char NO_CURRENT_USER_ERR [] = "The WinVNC settings for the current user are unavailable at present.";
const char CANNOT_EDIT_DEFAULT_PREFS [] = "You do not have sufficient priviliges to edit the default local WinVNC settings.";

// Constructor & Destructor
vncProperties::vncProperties()
{
	m_alloweditclients = TRUE;
	m_allowproperties = TRUE;
	m_allowshutdown = TRUE;
	m_dlgvisible = FALSE;
	m_usersettings = TRUE;
	m_inadvanced = FALSE;	

	m_pMatchWindow = NULL;
}

vncProperties::~vncProperties()
{
}

// Initialisation
BOOL
vncProperties::Init(vncServer *server)
{
	// Save the server pointer
	m_server = server;
	
	m_inadvanced = FALSE;
	
	// Load the settings from the registry
	Load(TRUE);

	if (m_pMatchWindow == NULL) 
	{
		RECT temp;
		GetWindowRect(GetDesktopWindow(), &temp);
		m_pMatchWindow=new CMatchWindow(m_server,temp.left+5,temp.top+5,temp.right/2,temp.bottom/2);
		m_pMatchWindow->CanModify(TRUE);
	}

	// If the password is empty then always show a dialog
	char passwd[MAXPWLEN];
	m_server->GetPassword(passwd);
	{
	    vncPasswd::ToText plain(passwd);
	    if (strlen(plain) == 0)
			if (!m_allowproperties) {
				if(m_server->AuthRequired()) {
					MessageBox(NULL, NO_PASSWD_NO_OVERRIDE_ERR,
								"WinVNC Error",
								MB_OK | MB_ICONSTOP);
					PostQuitMessage(0);
				} else {
					MessageBox(NULL, NO_PASSWD_NO_OVERRIDE_WARN,
								"WinVNC Error",
								MB_OK | MB_ICONEXCLAMATION);
				}
			} else {
				// If null passwords are not allowed, ensure that one is entered!
				if (m_server->AuthRequired()) {
					char username[UNLEN+1];
					if (!vncService::CurrentUser(username, sizeof(username)))
						return FALSE;
					if (strcmp(username, "") == 0) {
						MessageBox(NULL, NO_CURRENT_USER_ERR,
									"WinVNC Error",
									MB_OK | MB_ICONEXCLAMATION);
						Show(TRUE, FALSE);
					} else {
						Show(TRUE, TRUE);
					}
				}
			}
	}

	return TRUE;
}

// Dialog box handling functions
void
vncProperties::Show(BOOL show, BOOL usersettings)
{
	if (show)
	{
		m_inadvanced = FALSE;
		if (!m_allowproperties)
		{
			// If the user isn't allowed to override the settings then tell them
			MessageBox(NULL, NO_OVERRIDE_ERR, "WinVNC Error", MB_OK | MB_ICONEXCLAMATION);
			return;
		}

		// Verify that we know who is logged on
		if (usersettings) {
			char username[UNLEN+1];
			if (!vncService::CurrentUser(username, sizeof(username)))
				return;
		} else {
			// We're trying to edit the default local settings - verify that we can
			HKEY hkLocal, hkDefault;
			BOOL canEditDefaultPrefs = 1;
			DWORD dw;
			if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
				WINVNC_REGISTRY_KEY,
				0, REG_NONE, REG_OPTION_NON_VOLATILE,
				KEY_READ, NULL, &hkLocal, &dw) != ERROR_SUCCESS)
				canEditDefaultPrefs = 0;
			else if (RegCreateKeyEx(hkLocal,
				"Default",
				0, REG_NONE, REG_OPTION_NON_VOLATILE,
				KEY_WRITE | KEY_READ, NULL, &hkDefault, &dw) != ERROR_SUCCESS)
				canEditDefaultPrefs = 0;
			if (hkLocal) RegCloseKey(hkLocal);
			if (hkDefault) RegCloseKey(hkDefault);

			if (!canEditDefaultPrefs) {
				MessageBox(NULL, CANNOT_EDIT_DEFAULT_PREFS, "WinVNC Error", MB_OK | MB_ICONEXCLAMATION);
				return;
			}
		}

		// Now, if the dialog is not already displayed, show it!
		if (!m_dlgvisible)
		{
			if (usersettings)
				vnclog.Print(LL_INTINFO, VNCLOG("show per-user Properties\n"));
			else
				vnclog.Print(LL_INTINFO, VNCLOG("show default system Properties\n"));

			// Load in the settings relevant to the user or system
			Load(usersettings);

			for (;;)
			{
				m_returncode_valid = FALSE;

				// Do the dialog box
				int result = DialogBoxParam(hAppInstance,
				    MAKEINTRESOURCE(IDD_PROPERTIES_PARENT), 
				    NULL,
						(DLGPROC) ParentDlgProc,
				    (LONG) this);

				if (!m_returncode_valid)
				    result = IDCANCEL;

				vnclog.Print(LL_INTINFO, VNCLOG("dialog result = %d\n"), result);

				if (result == -1)
				{
					// Dialog box failed, so quit
					PostQuitMessage(0);
					return;
				}

				// We're allowed to exit if the password is not empty
				char passwd[MAXPWLEN];
				char passwd_viewonly[MAXPWLEN];
				m_server->GetPassword(passwd);
				m_server->GetPasswordViewOnly(passwd_viewonly);
				{
				    vncPasswd::ToText plain(passwd);
					vncPasswd::ToText plain_viewonly(passwd_viewonly);
					if ((strlen(plain) != 0) || !m_server->AuthRequired()) {
						if (strlen(passwd_viewonly) == 0) {
							m_server->GetPassword(passwd);
							m_server->SetPasswordViewOnly(passwd);
						}
						break;
					}
				}

				vnclog.Print(LL_INTERR, VNCLOG("warning - empty password\n"));

				// The password is empty, so if OK was used then redisplay the box,
				// otherwise, if CANCEL was used, close down WinVNC
				if (result == IDCANCEL)
				{
				    vnclog.Print(LL_INTERR, VNCLOG("no password - QUITTING\n"));
				    PostQuitMessage(0);
				    return;
				}

				// If we reached here then OK was used & there is no password!
				int result2 = MessageBox(NULL, NO_PASSWORD_WARN,
				    "WinVNC Warning", MB_OK | MB_ICONEXCLAMATION);

				omni_thread::sleep(4);
			}

			// Load in all the settings
			Load(TRUE);
		}
	}
}

BOOL CALLBACK
vncProperties::ParentDlgProc(HWND hwnd,
						  UINT uMsg,
						  WPARAM wParam,
						  LPARAM lParam )
{
	// We use the dialog-box's USERDATA to store a _this pointer
	// This is set only once WM_INITDIALOG has been recieved, though!
	vncProperties *_this = (vncProperties *) GetWindowLong(hwnd, GWL_USERDATA);

	switch (uMsg)
	{

	case WM_INITDIALOG:
		{
			// Retrieve the Dialog box parameter and use it as a pointer
			// to the calling vncProperties object
			SetWindowLong(hwnd, GWL_USERDATA, lParam);
			vncProperties *_this = (vncProperties *) lParam;
			_this->m_dlgvisible = TRUE;

			InitCommonControls();

			_this->m_hTab = GetDlgItem(hwnd, IDC_TAB);

			TCITEM item;
			item.mask = TCIF_TEXT; 
			item.pszText="Server";
			TabCtrl_InsertItem(_this->m_hTab, 0, &item);
			item.pszText = "Hooks";
			TabCtrl_InsertItem(_this->m_hTab, 1, &item);
			item.pszText = "Sharing";
			TabCtrl_InsertItem(_this->m_hTab, 2, &item);
			item.pszText = "Query";
			TabCtrl_InsertItem(_this->m_hTab, 3, &item);
			item.pszText = "Administration";
			TabCtrl_InsertItem(_this->m_hTab, 4, &item);

			_this->m_hShared = CreateDialogParam(hAppInstance, 
				MAKEINTRESOURCE(IDD_SHARED_DESKTOP_AREA),
				hwnd,
				(DLGPROC)_this->SharedDlgProc,
				(LONG)_this);

			_this->m_hIncoming = CreateDialogParam(hAppInstance,
				MAKEINTRESOURCE(IDD_INCOMING),
				hwnd,
				(DLGPROC)_this->IncomingDlgProc,
				(LONG)_this);

			_this->m_hPoll = CreateDialogParam(hAppInstance, 
				MAKEINTRESOURCE(IDD_UPDATE_HANDLING),
				hwnd,
				(DLGPROC)_this->PollDlgProc,
				(LONG)_this);

			_this->m_hQuerySettings = CreateDialogParam(hAppInstance, 
				MAKEINTRESOURCE(IDD_QUERY_SETTINGS),
				hwnd,
				(DLGPROC)_this->QuerySettingsDlgProc,
				(LONG)_this);

			_this->m_hAdministration = CreateDialogParam(hAppInstance, 
				MAKEINTRESOURCE(IDD_ADMINISTRATION),
				hwnd,
				(DLGPROC)_this->AdministrationDlgProc,
				(LONG)_this);

			// Position child dialogs, to fit the Tab control's display area
			RECT rc;
			GetWindowRect(_this->m_hTab, &rc);
			MapWindowPoints(NULL, hwnd, (POINT *)&rc, 2);
			TabCtrl_AdjustRect(_this->m_hTab, FALSE, &rc);
			SetWindowPos(_this->m_hIncoming, HWND_TOP, rc.left, rc.top,
						 rc.right - rc.left, rc.bottom - rc.top,
						 SWP_SHOWWINDOW);
			SetWindowPos(_this->m_hPoll, HWND_TOP, rc.left, rc.top,
						 rc.right - rc.left, rc.bottom - rc.top,
						 SWP_HIDEWINDOW);
			SetWindowPos(_this->m_hShared, HWND_TOP, rc.left, rc.top,
						 rc.right - rc.left, rc.bottom - rc.top,
						 SWP_HIDEWINDOW);
			SetWindowPos(_this->m_hQuerySettings, HWND_TOP, rc.left, rc.top,
						 rc.right - rc.left, rc.bottom - rc.top,
						 SWP_HIDEWINDOW);
			SetWindowPos(_this->m_hAdministration, HWND_TOP, rc.left, rc.top,
						 rc.right - rc.left, rc.bottom - rc.top,
						 SWP_HIDEWINDOW);

			// Set the dialog box's title to indicate which Properties we're editting
			if (_this->m_usersettings) {
				SetWindowText(hwnd, "TightVNC Server: Current User Properties");
			} else {
				SetWindowText(hwnd, "TightVNC Server: Default Local System Properties");
			}						
				
			// We return FALSE because we set the keyboard focus explicitly.
			return FALSE;
		}
	case WM_HELP:	
		VNCHelp::Popup(lParam);
		return 0;
    case WM_NOTIFY:
		{
			LPNMHDR pn = (LPNMHDR)lParam;			
			switch (pn->idFrom) {
			case IDC_TAB:
				{
				int i = TabCtrl_GetCurFocus(_this->m_hTab);
				DWORD style;
				if (pn->code == TCN_SELCHANGE)
					style = SW_SHOW;
				if (pn->code == TCN_SELCHANGING)
					style = SW_HIDE;
				if ((style != SW_HIDE) && (style != SW_SHOW))
					return 0;
				switch (i) {
				case 0:
					ShowWindow(_this->m_hIncoming, style);
					return 0;
				case 1:
					ShowWindow(_this->m_hPoll, style);						
					return 0;
				case 2:
					ShowWindow(_this->m_hShared, style);						
					return 0;
				case 3:
					ShowWindow(_this->m_hQuerySettings, style);						
					return 0;
				case 4:
					ShowWindow(_this->m_hAdministration, style);						
					return 0;
				}
				return 0;
				}
			}
			return 0;
		}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
		case IDC_APPLY:
			{	
			SendMessage(_this->m_hIncoming, WM_COMMAND, IDC_APPLY,0);
			SendMessage(_this->m_hPoll, WM_COMMAND, IDC_APPLY,0);
			SendMessage(_this->m_hShared, WM_COMMAND, IDC_APPLY,0);
			SendMessage(_this->m_hQuerySettings, WM_COMMAND, IDC_APPLY,0);
			SendMessage(_this->m_hAdministration, WM_COMMAND, IDC_APPLY,0);

			_this->Save();
        
			// Was ok pressed?
			if (LOWORD(wParam) == IDOK) {
        
				// Yes, so close the dialog
				vnclog.Print(LL_INTINFO, VNCLOG("enddialog (OK)\n"));

				_this->m_returncode_valid = TRUE;

				EndDialog(hwnd, IDOK);
				_this->m_dlgvisible = FALSE;
				_this->m_hTab = NULL;
			}
			return TRUE;
			}
		case IDCANCEL:
			vnclog.Print(LL_INTINFO, VNCLOG("enddialog (CANCEL)\n"));
			_this->m_returncode_valid = TRUE;
			EndDialog(hwnd, IDCANCEL);
			_this->m_dlgvisible = FALSE;
			_this->m_hTab = NULL;
			return TRUE;		
		}
		return 0;
	}
	return 0;
}

BOOL CALLBACK vncProperties::IncomingDlgProc(HWND hwnd, UINT uMsg,
                                             WPARAM wParam, LPARAM lParam)
{
	// We use the dialog-box's USERDATA to store a _this pointer
	// This is set only once WM_INITDIALOG has been recieved, though!
	vncProperties *_this = (vncProperties *) GetWindowLong(hwnd, GWL_USERDATA);
	
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			// Retrieve the Dialog box parameter and use it as a pointer
			// to the calling vncProperties object
			SetWindowLong(hwnd, GWL_USERDATA, lParam);
			vncProperties *_this = (vncProperties *) lParam;
			_this->m_incConnCtrl = new IncomingConnectionsControls(hwnd, _this->m_server);
			_this->m_inputhandcontr = new InputHandlingControls(hwnd, _this->m_server);
			HWND hLockSetting;
			switch (_this->m_server->LockSettings()) {
			case 1:
				hLockSetting = GetDlgItem(hwnd, IDC_LOCKSETTING_LOCK);
				break;
			case 2:
				hLockSetting = GetDlgItem(hwnd, IDC_LOCKSETTING_LOGOFF);
				break;
			default:
				hLockSetting = GetDlgItem(hwnd, IDC_LOCKSETTING_NOTHING);
			}
			SendMessage(hLockSetting, BM_SETCHECK, TRUE, 0);
			return 0;
		}
	case WM_HELP:	
		VNCHelp::Popup(lParam);
		return 0;
    case WM_COMMAND:		
		switch (LOWORD(wParam))
		{
		case IDC_CONNECT_SOCK:
		case IDC_PORTNO_AUTO:
		case IDC_SPECDISPLAY:
		case IDC_SPECPORT:
			_this->m_incConnCtrl->Validate(FALSE);
			return TRUE;      
		case IDC_DISABLE_INPUTS:
		case IDC_DISABLE_LOCAL_INPUTS:			
			_this->m_inputhandcontr->EnableInputs(hwnd);				
			return TRUE;	
		case IDC_REMOTE_DISABLE:
			_this->m_inputhandcontr->EnableRemote(hwnd);
			return TRUE;
		case IDC_APPLY:
			_this->m_incConnCtrl->Apply();
			_this->m_inputhandcontr->ApplyInputsControlsContents(hwnd);
			// Lock settings handling
			if (SendMessage(GetDlgItem(hwnd, IDC_LOCKSETTING_LOCK), BM_GETCHECK, 0, 0)
				== BST_CHECKED) {
				_this->m_server->SetLockSettings(1);
			} else if (SendMessage(GetDlgItem(hwnd, IDC_LOCKSETTING_LOGOFF), BM_GETCHECK, 0, 0)
				== BST_CHECKED) {
				_this->m_server->SetLockSettings(2);
			} else {
				_this->m_server->SetLockSettings(0);
			}
			return TRUE;
		}
		return 0;
	case WM_DESTROY:
		delete _this->m_incConnCtrl;      
		_this->m_incConnCtrl = NULL;  
		delete _this->m_inputhandcontr;
		_this->m_inputhandcontr = NULL;
		_this->m_hIncoming = NULL;  
		return 0;
	}
	return 0;
}

BOOL CALLBACK vncProperties::PollDlgProc(HWND hwnd, UINT uMsg,
                                         WPARAM wParam, LPARAM lParam)
{
	// We use the dialog-box's USERDATA to store a _this pointer
	// This is set only once WM_INITDIALOG has been recieved, though!
	vncProperties *_this = (vncProperties *) GetWindowLong(hwnd, GWL_USERDATA);
	
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			// Retrieve the Dialog box parameter and use it as a pointer
			// to the calling vncProperties object
			SetWindowLong(hwnd, GWL_USERDATA, lParam);
			vncProperties *_this = (vncProperties *) lParam;

			_this->m_pollcontrols = new PollControls(hwnd, _this->m_server); 
			return 0;
		}
	case WM_HELP:	
		VNCHelp::Popup(lParam);
		return 0;
	case WM_COMMAND:	
		switch (LOWORD(wParam))
		{
		case IDC_POLL_FOREGROUND:
		case IDC_POLL_UNDER_CURSOR:
		case IDC_POLL_FULLSCREEN:
			_this->m_pollcontrols->Validate();
			return TRUE;

		case IDOK:
		case IDC_APPLY:
			_this->m_pollcontrols->Apply();
			return TRUE;
		}
		return 0;
	case WM_DESTROY:
		delete _this->m_pollcontrols;
		_this->m_pollcontrols = NULL;  
		_this->m_hPoll = NULL;
		return 0;
	}
	return 0;
}

BOOL CALLBACK vncProperties::SharedDlgProc(HWND hwnd, UINT uMsg,
                                           WPARAM wParam, LPARAM lParam)
{
	// We use the dialog-box's USERDATA to store a _this pointer
	// This is set only once WM_INITDIALOG has been recieved, though!
	vncProperties *_this = (vncProperties *) GetWindowLong(hwnd, GWL_USERDATA);
	
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			// Retrieve the Dialog box parameter and use it as a pointer
			// to the calling vncProperties object
			SetWindowLong(hwnd, GWL_USERDATA, lParam);
			vncProperties *_this = (vncProperties *) lParam;
			
			_this->m_shareddtarea = new SharedDesktopArea(hwnd,
				_this->m_pMatchWindow,
				_this,
				_this->m_server);

			return 0;
		}
	case WM_HELP:	
		VNCHelp::Popup(lParam);
		return 0;
	case WM_COMMAND:	
		switch (LOWORD(wParam))
		{
		case IDC_FULLSCREEN:
			_this->m_shareddtarea->FullScreen();
			return TRUE;
			
		case IDC_WINDOW:
			_this->m_shareddtarea->SharedWindow();
			return TRUE;
			
		case IDC_SCREEN:
			_this->m_shareddtarea->SharedScreen();
			return TRUE;

		case IDC_APPLY:
		case IDOK:

			_this->m_shareddtarea->ApplySharedControls();
			return TRUE;
		}
		return 0;

	case WM_DESTROY:
		delete _this->m_shareddtarea;
		_this->m_shareddtarea = NULL;
		_this->m_hShared = NULL;
		return 0;
	}

	return 0;
}

BOOL CALLBACK vncProperties::AdministrationDlgProc(HWND hwnd, UINT uMsg,
												  WPARAM wParam, LPARAM lParam)
{
	// We use the dialog-box's USERDATA to store a _this pointer
	// This is set only once WM_INITDIALOG has been recieved, though!
	vncProperties *_this = (vncProperties *) GetWindowLong(hwnd, GWL_USERDATA);
	
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			// Retrieve the Dialog box parameter and use it as a pointer
			// to the calling vncProperties object
			SetWindowLong(hwnd, GWL_USERDATA, lParam);
			vncProperties *_this = (vncProperties *) lParam;
			_this->m_AdminControls = new AdministrationControls(hwnd, _this->m_server);
			return 0;
		}
	case WM_HELP:	
		VNCHelp::Popup(lParam);
		return 0;
    case WM_COMMAND:		
		switch (LOWORD(wParam))
		{
		case IDALLOWLOOPBACK:
		case IDLOG:
		case IDENABLEHTTPD:
			_this->m_AdminControls->Validate();
			return TRUE;      
		case IDC_APPLY:
			_this->m_AdminControls->Apply();
			return TRUE;
		}
		return 0;
	case WM_DESTROY:
		delete _this->m_AdminControls;      
		_this->m_AdminControls = NULL;  
		_this->m_hAdministration = NULL;  
		return 0;
	}
	return 0;	
}

BOOL CALLBACK vncProperties::QuerySettingsDlgProc(HWND hwnd, UINT uMsg,
												   WPARAM wParam, LPARAM lParam)
{
	// We use the dialog-box's USERDATA to store a _this pointer
	// This is set only once WM_INITDIALOG has been recieved, though!
	vncProperties *_this = (vncProperties *) GetWindowLong(hwnd, GWL_USERDATA);
	
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			// Retrieve the Dialog box parameter and use it as a pointer
			// to the calling vncProperties object
			SetWindowLong(hwnd, GWL_USERDATA, lParam);
			vncProperties *_this = (vncProperties *) lParam;
			_this->m_QSControls = new QuerySettingsControls(hwnd, _this->m_server);
			return 0;
		}
	case WM_HELP:	
		VNCHelp::Popup(lParam);
		return 0;
    case WM_COMMAND:		
		switch (LOWORD(wParam))
		{
		case IDQUERY:
			_this->m_QSControls->Validate();
			return TRUE;      
		case IDC_APPLY:
			_this->m_QSControls->Apply();
			return TRUE;
		}
		return 0;
	case WM_DESTROY:
		delete _this->m_QSControls;      
		_this->m_QSControls = NULL;  
		_this->m_hQuerySettings = NULL;  
		return 0;
	}
	return 0;	
}

// Functions to load & save the settings
LONG
vncProperties::LoadInt(HKEY key, LPCSTR valname, LONG defval)
{
	LONG pref;
	ULONG type = REG_DWORD;
	ULONG prefsize = sizeof(pref);

	if (key == NULL)
		return defval;

	if (RegQueryValueEx(key,
		valname,
		NULL,
		&type,
		(LPBYTE) &pref,
		&prefsize) != ERROR_SUCCESS)
		return defval;

	if (type != REG_DWORD)
		return defval;

	if (prefsize != sizeof(pref))
		return defval;

	return pref;
}

void
vncProperties::LoadPassword(HKEY key, char *buffer, const char *entry_name)
{
	DWORD type = REG_BINARY;
	int slen=MAXPWLEN;
	char inouttext[MAXPWLEN];

	if (key == NULL)
		return;

	// Retrieve the encrypted password
	if (RegQueryValueEx(key,
		(LPCSTR) entry_name,
		NULL,
		&type,
		(LPBYTE) &inouttext,
		(LPDWORD) &slen) != ERROR_SUCCESS)
		return;

	if (slen > MAXPWLEN)
		return;

	memcpy(buffer, inouttext, MAXPWLEN);
}

char *
vncProperties::LoadString(HKEY key, LPCSTR keyname)
{
	DWORD type = REG_SZ;
	DWORD buflen = 0;
	BYTE *buffer = 0;

	if (key == NULL)
		return 0;

	// Get the length of the AuthHosts string
	if (RegQueryValueEx(key,
		keyname,
		NULL,
		&type,
		NULL,
		&buflen) != ERROR_SUCCESS)
		return 0;

	if (type != REG_SZ)
		return 0;
	buffer = new BYTE[buflen];
	if (buffer == 0)
		return 0;

	// Get the AuthHosts string data
	if (RegQueryValueEx(key,
		keyname,
		NULL,
		&type,
		buffer,
		&buflen) != ERROR_SUCCESS) {
		delete [] buffer;
		return 0;
	}

	// Verify the type
	if (type != REG_SZ) {
		delete [] buffer;
		return 0;
	}

	return (char *)buffer;
}

void
vncProperties::Load(BOOL usersettings)
{
	if (m_dlgvisible) {
		vnclog.Print(LL_INTWARN, VNCLOG("service helper invoked while Properties panel displayed\n"));
		return;
	}

	char username[UNLEN+1];
	HKEY hkLocal, hkLocalUser, hkDefault;
	DWORD dw;

	// NEW (R3) PREFERENCES ALGORITHM
	// 1.	Look in HKEY_LOCAL_MACHINE/Software/ORL/WinVNC3/%username%
	//		for sysadmin-defined, user-specific settings.
	// 2.	If not found, fall back to %username%=Default
	// 3.	If AllowOverrides is set then load settings from
	//		HKEY_CURRENT_USER/Software/ORL/WinVNC3

	// GET THE CORRECT KEY TO READ FROM

	// Get the user name / service name
	if (!vncService::CurrentUser((char *)&username, sizeof(username)))
		return;

	// If there is no user logged on them default to SYSTEM
	if (strcmp(username, "") == 0)
		strcpy((char *)&username, "SYSTEM");

	// Try to get the machine registry key for WinVNC
	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
		WINVNC_REGISTRY_KEY,
		0, REG_NONE, REG_OPTION_NON_VOLATILE,
		KEY_READ, NULL, &hkLocal, &dw) != ERROR_SUCCESS)
		hkLocal = NULL;

	// Now try to get the per-user local key
	if ( hkLocal == NULL ||
		 RegOpenKeyEx(hkLocal, username, 0,
					  KEY_READ, &hkLocalUser) != ERROR_SUCCESS )
		hkLocalUser = NULL;

	// Get the default key
	if ( hkLocal == NULL ||
		 RegCreateKeyEx(hkLocal, "Default", 0, REG_NONE, REG_OPTION_NON_VOLATILE,
						KEY_READ, NULL, &hkDefault, &dw) != ERROR_SUCCESS )
		hkDefault = NULL;

	// LOAD THE MACHINE-LEVEL PREFS

	// Logging/debugging prefs
	vnclog.Print(LL_INTINFO, VNCLOG("loading local-only settings\n"));
	vnclog.SetMode(LoadInt(hkLocal, "DebugMode", 0));
	vnclog.SetLevel(LoadInt(hkLocal, "DebugLevel", 0));

	// Disable Tray Icon
	m_server->SetDisableTrayIcon(LoadInt(hkLocal, "DisableTrayIcon", false));

	// Authentication required, loopback allowed, loopbackOnly
	m_server->SetLoopbackOk(LoadInt(hkLocal, "AllowLoopback", false));
	if (!m_server->LoopbackOk())
		m_server->SetLoopbackOnly(false);
	else
		m_server->SetLoopbackOnly(LoadInt(hkLocal, "LoopbackOnly", false));
	m_server->SetHttpdEnabled(LoadInt(hkLocal, "EnableHTTPDaemon", true),
							  LoadInt(hkLocal, "EnableURLParams", false));
	m_server->SetAuthRequired(LoadInt(hkLocal, "AuthRequired", true));
	// NOTE: RealVNC sets ConnectPriority to 0 by default, we set it to 2.
	m_server->SetConnectPriority(LoadInt(hkLocal, "ConnectPriority", 2));
	if (!m_server->LoopbackOnly())
	{
		char *authhosts = LoadString(hkLocal, "AuthHosts");
		if (authhosts != 0) {
			m_server->SetAuthHosts(authhosts);
			delete [] authhosts;
		} else {
			m_server->SetAuthHosts(0);
		}
	} else {
		m_server->SetAuthHosts(0);
	}

	// LOAD THE USER PREFERENCES

	// Set the default user prefs
	vnclog.Print(LL_INTINFO, VNCLOG("clearing user settings\n"));
	m_pref_AutoPortSelect=TRUE;
	m_pref_PortNumber=RFB_PORT_OFFSET;
	m_pref_SockConnect=TRUE;
	m_pref_CORBAConn=FALSE;
	{
		vncPasswd::FromClear crypt;
		memcpy(m_pref_passwd, crypt, MAXPWLEN);
		memcpy(m_pref_passwd_viewonly, crypt, MAXPWLEN);
	}
	m_pref_externalAuth=FALSE;
	m_pref_QuerySetting=2;
	m_pref_QueryTimeout=30;
	m_pref_QueryAccept=FALSE;
	m_pref_QueryAllowNoPass=FALSE;
	m_pref_IdleTimeout=0;
	m_pref_EnableRemoteInputs=TRUE;
	m_pref_DisableLocalInputs=FALSE;
	m_pref_LockSettings=-1;
	m_pref_PollUnderCursor=FALSE;
	m_pref_PollForeground=TRUE;
	m_pref_PollFullScreen=FALSE;
	m_pref_PollConsoleOnly=TRUE;
	m_pref_PollOnEventOnly=FALSE;
	m_pref_DontSetHooks=FALSE;
	m_pref_DontUseDriver=FALSE;
	m_pref_RemoveWallpaper=TRUE;
	m_pref_BlankScreen = FALSE;
	m_pref_EnableFileTransfers = TRUE;
	m_alloweditclients = TRUE;
	m_allowshutdown = TRUE;
	m_allowproperties = TRUE;
	m_pref_FullScreen = TRUE;
	m_pref_WindowShared = FALSE;
	m_pref_ScreenAreaShared = FALSE;
	m_pref_PriorityTime = 3;
	m_pref_LocalInputPriority = FALSE;
	m_pref_PollingCycle = 300;

	// Load the local prefs for this user
	if (hkDefault != NULL)
	{
		vnclog.Print(LL_INTINFO, VNCLOG("loading DEFAULT local settings\n"));
		LoadUserPrefs(hkDefault);
		m_allowshutdown = LoadInt(hkDefault, "AllowShutdown", m_allowshutdown);
		m_allowproperties = LoadInt(hkDefault, "AllowProperties", m_allowproperties);
		m_alloweditclients = LoadInt(hkDefault, "AllowEditClients", m_alloweditclients);
	}

	// Are we being asked to load the user settings, or just the default local system settings?
	if (usersettings) {
		// We want the user settings, so load them!

		if (hkLocalUser != NULL)
		{
			vnclog.Print(LL_INTINFO, VNCLOG("loading \"%s\" local settings\n"), username);
			LoadUserPrefs(hkLocalUser);
			m_allowshutdown = LoadInt(hkLocalUser, "AllowShutdown", m_allowshutdown);
			m_allowproperties = LoadInt(hkLocalUser, "AllowProperties", m_allowproperties);
			m_alloweditclients = LoadInt(hkLocalUser, "AllowEditClients", m_alloweditclients);
		}

		// Now override the system settings with the user's settings
		// If the username is SYSTEM then don't try to load them, because there aren't any...
		if (m_allowproperties && (strcmp(username, "SYSTEM") != 0))
		{
			HKEY hkGlobalUser;
			if (RegCreateKeyEx(HKEY_CURRENT_USER,
				WINVNC_REGISTRY_KEY,
				0, REG_NONE, REG_OPTION_NON_VOLATILE,
				KEY_READ, NULL, &hkGlobalUser, &dw) == ERROR_SUCCESS)
			{
				vnclog.Print(LL_INTINFO, VNCLOG("loading \"%s\" global settings\n"), username);
				LoadUserPrefs(hkGlobalUser);
				RegCloseKey(hkGlobalUser);

				// Close the user registry hive so it can unload if required
				RegCloseKey(HKEY_CURRENT_USER);
			}
		}
	} else {
		vnclog.Print(LL_INTINFO, VNCLOG("bypassing user-specific settings (both local and global)\n"));
	}

	if (hkLocalUser != NULL) RegCloseKey(hkLocalUser);
	if (hkDefault != NULL) RegCloseKey(hkDefault);
	if (hkLocal != NULL) RegCloseKey(hkLocal);

	// Make the loaded settings active..
	ApplyUserPrefs();

	// Note whether we loaded the user settings or just the default system settings
	m_usersettings = usersettings;
}

void
vncProperties::LoadUserPrefs(HKEY appkey)
{
	// LOAD USER PREFS FROM THE SELECTED KEY

	// Connection prefs
	m_pref_SockConnect=LoadInt(appkey, "SocketConnect", m_pref_SockConnect);
	m_pref_AutoPortSelect=LoadInt(appkey, "AutoPortSelect", m_pref_AutoPortSelect);
	m_pref_PortNumber=LoadInt(appkey, "PortNumber", m_pref_PortNumber);
	m_pref_HttpPortNumber=LoadInt(appkey, "HTTPPortNumber",
		DISPLAY_TO_HPORT(PORT_TO_DISPLAY(m_pref_PortNumber)));
	m_pref_BeepConnect=LoadInt(appkey, "BeepConnect", m_pref_BeepConnect);
	m_pref_BeepDisconnect=LoadInt(appkey, "BeepDisconnect", m_pref_BeepDisconnect);
	m_pref_IdleTimeout=LoadInt(appkey, "IdleTimeout", m_pref_IdleTimeout);
	
	m_pref_RemoveWallpaper=LoadInt(appkey, "RemoveWallpaper", m_pref_RemoveWallpaper);
	m_pref_BlankScreen=LoadInt(appkey, "BlankScreen", m_pref_BlankScreen);
	m_pref_EnableFileTransfers=LoadInt(appkey, "EnableFileTransfers", m_pref_EnableFileTransfers);

	m_pref_PriorityTime =LoadInt(appkey, "LocalInputsPriorityTime", m_pref_PriorityTime);

	// Connection querying settings
	m_pref_QuerySetting=LoadInt(appkey, "QuerySetting", m_pref_QuerySetting);
	m_server->SetQuerySetting(m_pref_QuerySetting);
	m_pref_QueryTimeout=LoadInt(appkey, "QueryTimeout", m_pref_QueryTimeout);
	m_server->SetQueryTimeout(m_pref_QueryTimeout);
	m_pref_QueryAccept=LoadInt(appkey, "QueryAccept", m_pref_QueryAccept);
	m_server->SetQueryAccept(m_pref_QueryAccept);
	m_pref_QueryAllowNoPass=LoadInt(appkey, "QueryAllowNoPass", m_pref_QueryAllowNoPass);
	m_server->SetQueryAllowNoPass(m_pref_QueryAllowNoPass);

	// Load the primary password
	LoadPassword(appkey, m_pref_passwd, "Password");
	// Load the view-only password, default to the primary one
	memcpy(m_pref_passwd_viewonly, m_pref_passwd, MAXPWLEN);
	LoadPassword(appkey, m_pref_passwd_viewonly, "PasswordViewOnly");
	// External authentication (disabled in the public version)
	// m_pref_externalAuth=LoadInt(appkey, "ExternalAuth", m_pref_externalAuth);
	// CORBA Settings
	m_pref_CORBAConn=LoadInt(appkey, "CORBAConnect", m_pref_CORBAConn);

	// Remote access prefs
	m_pref_EnableRemoteInputs=LoadInt(appkey, "InputsEnabled", m_pref_EnableRemoteInputs);
	m_pref_LockSettings=LoadInt(appkey, "LockSetting", m_pref_LockSettings);
	m_pref_DisableLocalInputs=LoadInt(appkey, "LocalInputsDisabled", m_pref_DisableLocalInputs);
	m_pref_PollingCycle=LoadInt(appkey, "PollingCycle", m_pref_PollingCycle);

	// Polling prefs
	m_pref_PollUnderCursor=LoadInt(appkey, "PollUnderCursor", m_pref_PollUnderCursor);
	m_pref_PollForeground=LoadInt(appkey, "PollForeground", m_pref_PollForeground);
	m_pref_PollFullScreen=LoadInt(appkey, "PollFullScreen", m_pref_PollFullScreen);
	m_pref_PollConsoleOnly=LoadInt(appkey, "OnlyPollConsole", m_pref_PollConsoleOnly);
	m_pref_PollOnEventOnly=LoadInt(appkey, "OnlyPollOnEvent", m_pref_PollOnEventOnly);
	m_pref_DontSetHooks=LoadInt(appkey, "DontSetHooks", m_pref_DontSetHooks);
	m_pref_DontUseDriver=LoadInt(appkey, "DontUseDriver", m_pref_DontUseDriver);

	// screen area sharing prefs
	m_pref_FullScreen = m_server->FullScreen();
	m_pref_WindowShared = m_server->WindowShared();
	m_pref_ScreenAreaShared = m_server->ScreenAreaShared();

	m_pref_LocalInputPriority=LoadInt(appkey, "LocalInputsPriority", m_pref_LocalInputPriority);
}

void
vncProperties::ApplyUserPrefs()
{
	// APPLY THE CACHED PREFERENCES TO THE SERVER
	// Update the connection querying settings
	m_server->SetQuerySetting(m_pref_QuerySetting);
	m_server->SetQueryTimeout(m_pref_QueryTimeout);
	m_server->SetQueryAccept(m_pref_QueryAccept);
	m_server->SetQueryAllowNoPass(m_pref_QueryAllowNoPass);
	m_server->SetAutoIdleDisconnectTimeout(m_pref_IdleTimeout);
	m_server->EnableRemoveWallpaper(m_pref_RemoveWallpaper);
	m_server->SetBlankScreen(m_pref_BlankScreen);
	m_server->EnableFileTransfers(m_pref_EnableFileTransfers);

	// Update the password
	m_server->SetPassword(m_pref_passwd);
	m_server->SetPasswordViewOnly(m_pref_passwd_viewonly);

	// Now change the listening port settings
	m_server->SetAutoPortSelect(m_pref_AutoPortSelect);
	if (!m_pref_AutoPortSelect)
		m_server->SetPorts(m_pref_PortNumber, m_pref_HttpPortNumber);
	
	// Set the beep options
	m_server->SetBeepConnect(m_pref_BeepConnect);
	m_server->SetBeepDisconnect(m_pref_BeepDisconnect);
	
	// Set the CORBA connection status
	m_server->CORBAConnect(m_pref_CORBAConn);

	// Remote access prefs
	m_server->EnableRemoteInputs(m_pref_EnableRemoteInputs);
	m_server->SetLockSettings(m_pref_LockSettings);
	m_server->DisableLocalInputs(m_pref_DisableLocalInputs);
	
	m_server->SetDisableTime(m_pref_PriorityTime);
	m_server->SetPollingCycle(m_pref_PollingCycle);

	// Enable/disable external authentication
	m_server->EnableExternalAuth(m_pref_externalAuth);
	m_server->SockConnect(m_pref_SockConnect);
	// Polling prefs
	m_server->PollUnderCursor(m_pref_PollUnderCursor);
	m_server->PollForeground(m_pref_PollForeground);
	m_server->PollFullScreen(m_pref_PollFullScreen);
	m_server->PollConsoleOnly(m_pref_PollConsoleOnly);
	m_server->PollOnEventOnly(m_pref_PollOnEventOnly);
	m_server->DontSetHooks(m_pref_DontSetHooks);
	m_server->DontUseDriver(m_pref_DontUseDriver);

	m_server->FullScreen(m_pref_FullScreen);
	m_server->WindowShared(m_pref_WindowShared);
	m_server->ScreenAreaShared(m_pref_ScreenAreaShared);

	m_server->LocalInputPriority(m_pref_LocalInputPriority);
}

void
vncProperties::SaveInt(HKEY key, LPCSTR valname, LONG val)
{
	RegSetValueEx(key, valname, 0, REG_DWORD, (LPBYTE) &val, sizeof(val));
}

void
vncProperties::SavePassword(HKEY key, const char *buffer, const char *entry_name)
{
	RegSetValueEx(key, entry_name, 0, REG_BINARY, (LPBYTE) buffer, MAXPWLEN);
}

void
vncProperties::Save()
{
	HKEY appkey;
	DWORD dw;

	if (!m_allowproperties)
		return;

	// NEW (R3) PREFERENCES ALGORITHM
	// The user's prefs are only saved if the user is allowed to override
	// the machine-local settings specified for them.  Otherwise, the
	// properties entry on the tray icon menu will be greyed out.

	// GET THE CORRECT KEY TO READ FROM

	// Have we loaded user settings, or system settings?
	if (m_usersettings) {
		// Verify that we know who is logged on
		char username[UNLEN+1];
		if (!vncService::CurrentUser((char *)&username, sizeof(username)))
			return;
		if (strcmp(username, "") == 0)
			return;

		// Try to get the per-user, global registry key for WinVNC
		if (RegCreateKeyEx(HKEY_CURRENT_USER,
			WINVNC_REGISTRY_KEY,
			0, REG_NONE, REG_OPTION_NON_VOLATILE,
			KEY_WRITE | KEY_READ, NULL, &appkey, &dw) != ERROR_SUCCESS)
			return;
	} else {
		// Try to get the default local registry key for WinVNC
		HKEY hkLocal;
		if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
			WINVNC_REGISTRY_KEY,
			0, REG_NONE, REG_OPTION_NON_VOLATILE,
			KEY_READ, NULL, &hkLocal, &dw) != ERROR_SUCCESS) {
			MessageBox(NULL, "MB1", "WVNC", MB_OK);
			return;
		}
		if (RegCreateKeyEx(hkLocal,
			"Default",
			0, REG_NONE, REG_OPTION_NON_VOLATILE,
			KEY_WRITE | KEY_READ, NULL, &appkey, &dw) != ERROR_SUCCESS) {
			RegCloseKey(hkLocal);
			return;
		}
		RegCloseKey(hkLocal);
	}

	// SAVE PER-USER PREFS IF ALLOWED
	SaveUserPrefs(appkey);

	RegCloseKey(appkey);

	// Machine Preferences
	// Try to get the machine registry key for WinVNC
	HKEY hkLocal;
	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
		WINVNC_REGISTRY_KEY,
		0, REG_NONE, REG_OPTION_NON_VOLATILE,
		KEY_WRITE | KEY_READ, NULL, &hkLocal, &dw) != ERROR_SUCCESS)
		return;

	SaveInt(hkLocal, "ConnectPriority", m_server->ConnectPriority());
	SaveInt(hkLocal, "LoopbackOnly", m_server->LoopbackOnly());
	SaveInt(hkLocal, "EnableHTTPDaemon", m_server->HttpdEnabled());
	SaveInt(hkLocal, "EnableURLParams", m_server->HttpdParamsEnabled());
	SaveInt(hkLocal, "AllowLoopback", m_server->LoopbackOk());
	SaveInt(hkLocal, "AuthRequired", m_server->AuthRequired());

	SaveInt(hkLocal, "DebugMode", vnclog.GetMode());
	SaveInt(hkLocal, "DebugLevel", vnclog.GetLevel());
	RegCloseKey(hkLocal);


	// Close the user registry hive, to allow it to unload if reqd
	RegCloseKey(HKEY_CURRENT_USER);
}

void
vncProperties::SaveUserPrefs(HKEY appkey)
{
	// SAVE THE PER USER PREFS
	vnclog.Print(LL_INTINFO, VNCLOG("saving current settings to registry\n"));

	// Connection prefs
	SaveInt(appkey, "SocketConnect", m_server->SockConnected());
	SaveInt(appkey, "AutoPortSelect", m_server->AutoPortSelect());
	if (!m_server->AutoPortSelect()) {
		SaveInt(appkey, "PortNumber", m_server->GetPort());
		SaveInt(appkey, "HTTPPortNumber", m_server->GetHttpPort());
	}
	SaveInt(appkey, "InputsEnabled", m_server->RemoteInputsEnabled());
	SaveInt(appkey, "LocalInputsDisabled", m_server->LocalInputsDisabled());
	SaveInt(appkey, "IdleTimeout", m_server->AutoIdleDisconnectTimeout());
	
	SaveInt(appkey, "LocalInputsPriorityTime", m_server->DisableTime());
	
	// Connection querying settings
	SaveInt(appkey, "QuerySetting", m_server->QuerySetting());
	SaveInt(appkey, "QueryTimeout", m_server->QueryTimeout());
	SaveInt(appkey, "QueryAccept", m_server->QueryAccept());
	SaveInt(appkey, "QueryAllowNoPass", m_server->QueryAllowNoPass());

	SaveInt(appkey, "LockSetting", m_server->LockSettings());
	SaveInt(appkey, "RemoveWallpaper", m_server->RemoveWallpaperEnabled());
	SaveInt(appkey, "BlankScreen", m_server->GetBlankScreen());
	SaveInt(appkey, "EnableFileTransfers", m_server->FileTransfersEnabled());

	// Save the password
	char passwd[MAXPWLEN];
	m_server->GetPassword(passwd);
	SavePassword(appkey, passwd, "Password");
	m_server->GetPasswordViewOnly(passwd);
	SavePassword(appkey, passwd, "PasswordViewOnly");

#if(defined(_CORBA))
	// Don't save the CORBA enabled flag if CORBA is not compiled in!
	SaveInt(appkey, "CORBAConnect", m_server->CORBAConnected());
#endif

	// Polling prefs
	SaveInt(appkey, "PollUnderCursor", m_server->PollUnderCursor());
	SaveInt(appkey, "PollForeground", m_server->PollForeground());
	SaveInt(appkey, "PollFullScreen", m_server->PollFullScreen());

	SaveInt(appkey, "OnlyPollConsole", m_server->PollConsoleOnly());
	SaveInt(appkey, "OnlyPollOnEvent", m_server->PollOnEventOnly());
	SaveInt(appkey, "PollingCycle", m_server->GetPollingCycle());

	SaveInt(appkey, "DontSetHooks", m_server->DontSetHooks());
	SaveInt(appkey, "DontUseDriver", m_server->DontUseDriver());

	SaveInt(appkey, "LocalInputsPriority", m_server->LocalInputPriority());
}


