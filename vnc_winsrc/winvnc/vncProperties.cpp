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
#include "vncAdvancedProperties.h"
#include "vncServer.h"
#include "vncPasswd.h"

const char WINVNC_REGISTRY_KEY [] = "Software\\ORL\\WinVNC3";
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
						MessageBox(NULL, NO_PASSWD_NO_LOGON_WARN,
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
			if (strcmp(username, "") == 0) {
				MessageBox(NULL, NO_CURRENT_USER_ERR, "WinVNC Error", MB_OK | MB_ICONEXCLAMATION);
				return;
			}
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
				    MAKEINTRESOURCE(IDD_PROPERTIES), 
				    NULL,
				    (DLGPROC) DialogProc,
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
vncProperties::DialogProc(HWND hwnd,
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
			_this = (vncProperties *) lParam;
			_this->m_dlgvisible = TRUE;

			// Set the dialog box's title to indicate which Properties we're editting
			if (_this->m_usersettings) {
				SetWindowText(hwnd, "WinVNC: Current User Properties");
			} else {
				SetWindowText(hwnd, "WinVNC: Default Local System Properties");
			}

			// Initialise the properties controls
			HWND hConnectSock = GetDlgItem(hwnd, IDC_CONNECT_SOCK);
			BOOL bConnectSock = _this->m_server->SockConnected();
			SendMessage(hConnectSock, BM_SETCHECK, bConnectSock, 0);

			// Set the content of password fields to a predefined string.
			SetDlgItemText(hwnd, IDC_PASSWORD, "~~~~~~~~");
			EnableWindow(GetDlgItem(hwnd, IDC_PASSWORD), bConnectSock);
			SetDlgItemText(hwnd, IDC_PASSWORD_VIEWONLY, "~~~~~~~~");
			EnableWindow(GetDlgItem(hwnd, IDC_PASSWORD_VIEWONLY), bConnectSock);

			// Set the initial keyboard focus
			if (bConnectSock) {
				SetFocus(GetDlgItem(hwnd, IDC_PASSWORD));
				SendDlgItemMessage(hwnd, IDC_PASSWORD, EM_SETSEL, 0, (LPARAM)-1);
			} else {
				SetFocus(hConnectSock);
			}

			// Set display/ports settings
			_this->InitPortSettings(hwnd);

			// Remote input settings
			HWND hEnableRemoteInputs = GetDlgItem(hwnd, IDC_DISABLE_INPUTS);
			SendMessage(hEnableRemoteInputs,
				BM_SETCHECK,
				!(_this->m_server->RemoteInputsEnabled()),
				0);

			// Local input settings
			HWND hDisableLocalInputs = GetDlgItem(hwnd, IDC_DISABLE_LOCAL_INPUTS);
			SendMessage(hDisableLocalInputs,
				BM_SETCHECK,
				_this->m_server->LocalInputsDisabled(),
				0);

			// Remove the wallpaper
			HWND hRemoveWallpaper = GetDlgItem(hwnd, IDC_REMOVE_WALLPAPER);
			SendMessage(hRemoveWallpaper,
				BM_SETCHECK,
				_this->m_server->RemoveWallpaperEnabled(),
				0);

			// Lock settings
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
			};
			SendMessage(hLockSetting,
				BM_SETCHECK,
				TRUE,
				0);

			// Set the polling options
			HWND hPollFullScreen = GetDlgItem(hwnd, IDC_POLL_FULLSCREEN);
			SendMessage(hPollFullScreen,
				BM_SETCHECK,
				_this->m_server->PollFullScreen(),
				0);

			HWND hPollForeground = GetDlgItem(hwnd, IDC_POLL_FOREGROUND);
			SendMessage(hPollForeground,
				BM_SETCHECK,
				_this->m_server->PollForeground(),
				0);

			HWND hPollUnderCursor = GetDlgItem(hwnd, IDC_POLL_UNDER_CURSOR);
			SendMessage(hPollUnderCursor,
				BM_SETCHECK,
				_this->m_server->PollUnderCursor(),
				0);

			HWND hPollConsoleOnly = GetDlgItem(hwnd, IDC_CONSOLE_ONLY);
			SendMessage(hPollConsoleOnly,
				BM_SETCHECK,
				_this->m_server->PollConsoleOnly(),
				0);
			EnableWindow(hPollConsoleOnly,
				_this->m_server->PollUnderCursor() || _this->m_server->PollForeground()
				);

			HWND hPollOnEventOnly = GetDlgItem(hwnd, IDC_ONEVENT_ONLY);
			SendMessage(hPollOnEventOnly,
				BM_SETCHECK,
				_this->m_server->PollOnEventOnly(),
				0);
			EnableWindow(hPollOnEventOnly,
				_this->m_server->PollUnderCursor() || _this->m_server->PollForeground()
				);

			SetForegroundWindow(hwnd);

			// We return FALSE because we set the keyboard focus explicitly.
			return FALSE;
		}

	case WM_COMMAND:
		if (_this->m_inadvanced)
			return FALSE;
		switch (LOWORD(wParam))
		{

		case IDOK:
		case IDC_APPLY:
			{
				// Save the password if one was entered
				char passwd[MAXPWLEN+1];
				int len = GetDlgItemText(hwnd, IDC_PASSWORD, (LPSTR)&passwd, MAXPWLEN+1);
				if (strcmp(passwd, "~~~~~~~~") != 0) {
					if (len == 0) {
						vncPasswd::FromClear crypt;
						_this->m_server->SetPassword(crypt);
					} else {
						vncPasswd::FromText crypt(passwd);
						_this->m_server->SetPassword(crypt);
					}
				}

				// Save the password (view only) if one was entered
				// FIXME: Code duplication, see above
				len = GetDlgItemText(hwnd, IDC_PASSWORD_VIEWONLY, (LPSTR)&passwd, MAXPWLEN+1);
				if (strcmp(passwd, "~~~~~~~~") != 0) {
					if (len == 0) {
						vncPasswd::FromClear crypt;
						_this->m_server->SetPasswordViewOnly(crypt);
					} else {
						vncPasswd::FromText crypt(passwd);
						_this->m_server->SetPasswordViewOnly(crypt);
					}
				}

				// Save the new settings to the server
				int state = SendDlgItemMessage(hwnd, IDC_PORTNO_AUTO, BM_GETCHECK, 0, 0);
				_this->m_server->SetAutoPortSelect(state == BST_CHECKED);

				// Save port numbers if we're not auto selecting
				if (!_this->m_server->AutoPortSelect()) {
					if ( SendDlgItemMessage(hwnd, IDC_SPECDISPLAY,
											BM_GETCHECK, 0, 0) == BST_CHECKED ) {
						// Display number was specified
						BOOL ok;
						UINT display = GetDlgItemInt(hwnd, IDC_DISPLAYNO, &ok, TRUE);
						if (ok)
							_this->m_server->SetPorts(DISPLAY_TO_PORT(display),
													  DISPLAY_TO_HPORT(display));
					} else {
						// Assuming that port numbers were specified
						BOOL ok1, ok2;
						UINT port_rfb = GetDlgItemInt(hwnd, IDC_PORTRFB, &ok1, TRUE);
						UINT port_http = GetDlgItemInt(hwnd, IDC_PORTHTTP, &ok2, TRUE);
						if (ok1 && ok2)
							_this->m_server->SetPorts(port_rfb, port_http);
					}
				}

				HWND hConnectSock = GetDlgItem(hwnd, IDC_CONNECT_SOCK);
				_this->m_server->SockConnect(
					SendMessage(hConnectSock, BM_GETCHECK, 0, 0) == BST_CHECKED
					);

				// Update display/port controls on pressing the "Apply" button
				if (LOWORD(wParam) == IDC_APPLY)
					_this->InitPortSettings(hwnd);

				// Remote input stuff
				HWND hEnableRemoteInputs = GetDlgItem(hwnd, IDC_DISABLE_INPUTS);
				_this->m_server->EnableRemoteInputs(
					SendMessage(hEnableRemoteInputs, BM_GETCHECK, 0, 0) != BST_CHECKED
					);

				// Local input stuff
				HWND hDisableLocalInputs = GetDlgItem(hwnd, IDC_DISABLE_LOCAL_INPUTS);
				_this->m_server->DisableLocalInputs(
					SendMessage(hDisableLocalInputs, BM_GETCHECK, 0, 0) == BST_CHECKED
					);

				// Wallpaper handling
				HWND hRemoveWallpaper = GetDlgItem(hwnd, IDC_REMOVE_WALLPAPER);
				_this->m_server->EnableRemoveWallpaper(
					SendMessage(hRemoveWallpaper, BM_GETCHECK, 0, 0) == BST_CHECKED
					);

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

				// Handle the polling stuff
				HWND hPollFullScreen = GetDlgItem(hwnd, IDC_POLL_FULLSCREEN);
				_this->m_server->PollFullScreen(
					SendMessage(hPollFullScreen, BM_GETCHECK, 0, 0) == BST_CHECKED
					);

				HWND hPollForeground = GetDlgItem(hwnd, IDC_POLL_FOREGROUND);
				_this->m_server->PollForeground(
					SendMessage(hPollForeground, BM_GETCHECK, 0, 0) == BST_CHECKED
					);

				HWND hPollUnderCursor = GetDlgItem(hwnd, IDC_POLL_UNDER_CURSOR);
				_this->m_server->PollUnderCursor(
					SendMessage(hPollUnderCursor, BM_GETCHECK, 0, 0) == BST_CHECKED
					);

				HWND hPollConsoleOnly = GetDlgItem(hwnd, IDC_CONSOLE_ONLY);
				_this->m_server->PollConsoleOnly(
					SendMessage(hPollConsoleOnly, BM_GETCHECK, 0, 0) == BST_CHECKED
					);

				HWND hPollOnEventOnly = GetDlgItem(hwnd, IDC_ONEVENT_ONLY);
				_this->m_server->PollOnEventOnly(
					SendMessage(hPollOnEventOnly, BM_GETCHECK, 0, 0) == BST_CHECKED
					);

				// And to the registry
				_this->Save();

				// Was ok pressed?
				if (LOWORD(wParam) == IDOK)
				{
					// Yes, so close the dialog
					vnclog.Print(LL_INTINFO, VNCLOG("enddialog (OK)\n"));

					_this->m_returncode_valid = TRUE;

					EndDialog(hwnd, IDOK);
					_this->m_dlgvisible = FALSE;
				}

				return TRUE;
			}

		case IDCANCEL:
			vnclog.Print(LL_INTINFO, VNCLOG("enddialog (CANCEL)\n"));

			_this->m_returncode_valid = TRUE;

			EndDialog(hwnd, IDCANCEL);
			_this->m_dlgvisible = FALSE;
			return TRUE;

		case IDADVANCED:
			vnclog.Print(LL_INTINFO, VNCLOG("newdialog (ADVANCED)\n"));
			{
				EnableWindow(hwnd, FALSE);
				_this->m_inadvanced = TRUE;
				vncAdvancedProperties *aprop = new vncAdvancedProperties();
				if (aprop->Init(_this->m_server))
				{
					aprop->Show(TRUE, _this->m_usersettings);
				}
				//aprop->DoDialog();
				SetForegroundWindow(hwnd);
				_this->m_inadvanced = FALSE;
				EnableWindow(hwnd, TRUE);
				omni_thread::sleep(0, 200000000);
			}
			return TRUE;

		case IDC_CONNECT_SOCK:
			// The user has clicked on the socket connect tickbox
			{
				BOOL bConnectSock =
					(SendDlgItemMessage(hwnd, IDC_CONNECT_SOCK,
										BM_GETCHECK, 0, 0) == BST_CHECKED);

				EnableWindow(GetDlgItem(hwnd, IDC_PASSWORD), bConnectSock);
				EnableWindow(GetDlgItem(hwnd, IDC_PASSWORD_VIEWONLY), bConnectSock);

				HWND hPortNoAuto = GetDlgItem(hwnd, IDC_PORTNO_AUTO);
				EnableWindow(hPortNoAuto, bConnectSock);
				HWND hSpecDisplay = GetDlgItem(hwnd, IDC_SPECDISPLAY);
				EnableWindow(hSpecDisplay, bConnectSock);
				HWND hSpecPort = GetDlgItem(hwnd, IDC_SPECPORT);
				EnableWindow(hSpecPort, bConnectSock);

				EnableWindow(GetDlgItem(hwnd, IDC_DISPLAYNO), bConnectSock &&
					(SendMessage(hSpecDisplay, BM_GETCHECK, 0, 0) == BST_CHECKED));
				EnableWindow(GetDlgItem(hwnd, IDC_PORTRFB), bConnectSock &&
					(SendMessage(hSpecPort, BM_GETCHECK, 0, 0) == BST_CHECKED));
				EnableWindow(GetDlgItem(hwnd, IDC_PORTHTTP), bConnectSock &&
					(SendMessage(hSpecPort, BM_GETCHECK, 0, 0) == BST_CHECKED));
			}
			return TRUE;

		case IDC_POLL_FOREGROUND:
		case IDC_POLL_UNDER_CURSOR:
			// User has clicked on one of the polling mode buttons
			// affected by the pollconsole and pollonevent options
			{
				// Get the poll-mode buttons
				HWND hPollForeground = GetDlgItem(hwnd, IDC_POLL_FOREGROUND);
				HWND hPollUnderCursor = GetDlgItem(hwnd, IDC_POLL_UNDER_CURSOR);

				// Determine whether to enable the modifier options
				BOOL enabled = (SendMessage(hPollForeground, BM_GETCHECK, 0, 0) == BST_CHECKED) ||
					(SendMessage(hPollUnderCursor, BM_GETCHECK, 0, 0) == BST_CHECKED);

				HWND hPollConsoleOnly = GetDlgItem(hwnd, IDC_CONSOLE_ONLY);
				EnableWindow(hPollConsoleOnly, enabled);

				HWND hPollOnEventOnly = GetDlgItem(hwnd, IDC_ONEVENT_ONLY);
				EnableWindow(hPollOnEventOnly, enabled);
			}
			return TRUE;

		case IDC_PORTNO_AUTO:
			{
				EnableWindow(GetDlgItem(hwnd, IDC_DISPLAYNO), FALSE);
				EnableWindow(GetDlgItem(hwnd, IDC_PORTRFB), FALSE);
				EnableWindow(GetDlgItem(hwnd, IDC_PORTHTTP), FALSE);

				SetDlgItemText(hwnd, IDC_DISPLAYNO, "");
				SetDlgItemText(hwnd, IDC_PORTRFB, "");
				SetDlgItemText(hwnd, IDC_PORTHTTP, "");
			}
			return TRUE;

		case IDC_SPECDISPLAY:
			{
				EnableWindow(GetDlgItem(hwnd, IDC_DISPLAYNO), TRUE);
				EnableWindow(GetDlgItem(hwnd, IDC_PORTRFB), FALSE);
				EnableWindow(GetDlgItem(hwnd, IDC_PORTHTTP), FALSE);

				int display = PORT_TO_DISPLAY(_this->m_server->GetPort());
				if (display < 0 || display > 99)
					display = 0;
				SetDlgItemInt(hwnd, IDC_DISPLAYNO, display, FALSE);
				SetDlgItemInt(hwnd, IDC_PORTRFB, _this->m_server->GetPort(), FALSE);
				SetDlgItemInt(hwnd, IDC_PORTHTTP, _this->m_server->GetHttpPort(), FALSE);

				SetFocus(GetDlgItem(hwnd, IDC_DISPLAYNO));
				SendDlgItemMessage(hwnd, IDC_DISPLAYNO, EM_SETSEL, 0, (LPARAM)-1);
			}
			return TRUE;

		case IDC_SPECPORT:
			{
				EnableWindow(GetDlgItem(hwnd, IDC_DISPLAYNO), FALSE);
				EnableWindow(GetDlgItem(hwnd, IDC_PORTRFB), TRUE);
				EnableWindow(GetDlgItem(hwnd, IDC_PORTHTTP), TRUE);

				int d1 = PORT_TO_DISPLAY(_this->m_server->GetPort());
				int d2 = HPORT_TO_DISPLAY(_this->m_server->GetHttpPort());
				if (d1 == d2 && d1 >= 0 && d1 <= 99) {
					SetDlgItemInt(hwnd, IDC_DISPLAYNO, d1, FALSE);
				} else {
					SetDlgItemText(hwnd, IDC_DISPLAYNO, "");
				}
				SetDlgItemInt(hwnd, IDC_PORTRFB, _this->m_server->GetPort(), FALSE);
				SetDlgItemInt(hwnd, IDC_PORTHTTP, _this->m_server->GetHttpPort(), FALSE);

				SetFocus(GetDlgItem(hwnd, IDC_PORTRFB));
				SendDlgItemMessage(hwnd, IDC_PORTRFB, EM_SETSEL, 0, (LPARAM)-1);
			}
			return TRUE;

		}

		break;
	}
	return 0;
}

// Set display/port settings to the correct state
void
vncProperties::InitPortSettings(HWND hwnd)
{
	BOOL bConnectSock = m_server->SockConnected();
	BOOL bAutoPort = m_server->AutoPortSelect();
	UINT port_rfb = m_server->GetPort();
	UINT port_http = m_server->GetHttpPort();
	int d1 = PORT_TO_DISPLAY(port_rfb);
	int d2 = HPORT_TO_DISPLAY(port_http);
	BOOL bValidDisplay = (d1 == d2 && d1 >= 0 && d1 <= 99);

	CheckDlgButton(hwnd, IDC_PORTNO_AUTO,
		(bAutoPort) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwnd, IDC_SPECDISPLAY,
		(!bAutoPort && bValidDisplay) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwnd, IDC_SPECPORT,
		(!bAutoPort && !bValidDisplay) ? BST_CHECKED : BST_UNCHECKED);

	EnableWindow(GetDlgItem(hwnd, IDC_PORTNO_AUTO), bConnectSock);
	EnableWindow(GetDlgItem(hwnd, IDC_SPECDISPLAY), bConnectSock);
	EnableWindow(GetDlgItem(hwnd, IDC_SPECPORT), bConnectSock);

	if (bValidDisplay) {
		SetDlgItemInt(hwnd, IDC_DISPLAYNO, d1, FALSE);
	} else {
		SetDlgItemText(hwnd, IDC_DISPLAYNO, "");
	}
	SetDlgItemInt(hwnd, IDC_PORTRFB, port_rfb, FALSE);
	SetDlgItemInt(hwnd, IDC_PORTHTTP, port_http, FALSE);

	EnableWindow(GetDlgItem(hwnd, IDC_DISPLAYNO),
		bConnectSock && !bAutoPort && bValidDisplay);
	EnableWindow(GetDlgItem(hwnd, IDC_PORTRFB),
		bConnectSock && !bAutoPort && !bValidDisplay);
	EnableWindow(GetDlgItem(hwnd, IDC_PORTHTTP),
		bConnectSock && !bAutoPort && !bValidDisplay);
}

// Functions to load & save the settings
LONG
vncProperties::LoadInt(HKEY key, LPCSTR valname, LONG defval)
{
	LONG pref;
	ULONG type = REG_DWORD;
	ULONG prefsize = sizeof(pref);

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
		return;

	// Now try to get the per-user local key
	if (RegOpenKeyEx(hkLocal,
		username,
		0, KEY_READ,
		&hkLocalUser) != ERROR_SUCCESS)
		hkLocalUser = NULL;

	// Get the default key
	if (RegCreateKeyEx(hkLocal,
		"Default",
		0, REG_NONE, REG_OPTION_NON_VOLATILE,
		KEY_READ,
		NULL,
		&hkDefault,
		&dw) != ERROR_SUCCESS)
		hkDefault = NULL;

	// LOAD THE MACHINE-LEVEL PREFS

	// Logging/debugging prefs
	vnclog.Print(LL_INTINFO, VNCLOG("loading local-only settings\n"));
	vnclog.SetMode(LoadInt(hkLocal, "DebugMode", 0));
	vnclog.SetLevel(LoadInt(hkLocal, "DebugLevel", 0));

	// Disable Tray Icon
	m_server->SetDisableTrayIcon(LoadInt(hkLocal, "DisableTrayIcon", false));

	// Authentication required, loopback allowed, loopbackOnly
	m_server->SetLoopbackOnly(LoadInt(hkLocal, "LoopbackOnly", false));
	if (m_server->LoopbackOnly())
		m_server->SetLoopbackOk(true);
	else
		m_server->SetLoopbackOk(LoadInt(hkLocal, "AllowLoopback", false));
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
	m_pref_RemoveWallpaper=TRUE;
	m_alloweditclients = TRUE;
	m_allowshutdown = TRUE;
	m_allowproperties = TRUE;

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
	RegCloseKey(hkLocal);

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

	// Connection querying settings
	m_pref_QuerySetting=LoadInt(appkey, "QuerySetting", m_pref_QuerySetting);
	m_pref_QueryTimeout=LoadInt(appkey, "QueryTimeout", m_pref_QueryTimeout);
	m_pref_QueryAccept=LoadInt(appkey, "QueryAccept", m_pref_QueryAccept);
	m_pref_QueryAllowNoPass=LoadInt(appkey, "QueryAllowNoPass", m_pref_QueryAllowNoPass);

	// Load the primary password
	LoadPassword(appkey, m_pref_passwd, "Password");
	// Load the view-only password, default to the primary one
	memcpy(m_pref_passwd_viewonly, m_pref_passwd, MAXPWLEN);
	LoadPassword(appkey, m_pref_passwd_viewonly, "PasswordViewOnly");
	// CORBA Settings
	m_pref_CORBAConn=LoadInt(appkey, "CORBAConnect", m_pref_CORBAConn);

	// Remote access prefs
	m_pref_EnableRemoteInputs=LoadInt(appkey, "InputsEnabled", m_pref_EnableRemoteInputs);
	m_pref_LockSettings=LoadInt(appkey, "LockSetting", m_pref_LockSettings);
	m_pref_DisableLocalInputs=LoadInt(appkey, "LocalInputsDisabled", m_pref_DisableLocalInputs);

	// Polling prefs
	m_pref_PollUnderCursor=LoadInt(appkey, "PollUnderCursor", m_pref_PollUnderCursor);
	m_pref_PollForeground=LoadInt(appkey, "PollForeground", m_pref_PollForeground);
	m_pref_PollFullScreen=LoadInt(appkey, "PollFullScreen", m_pref_PollFullScreen);
	m_pref_PollConsoleOnly=LoadInt(appkey, "OnlyPollConsole", m_pref_PollConsoleOnly);
	m_pref_PollOnEventOnly=LoadInt(appkey, "OnlyPollOnEvent", m_pref_PollOnEventOnly);
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

	// Is the listening socket closing?
	if (!m_pref_SockConnect)
		m_server->SockConnect(m_pref_SockConnect);

	// Are inputs being disabled?
	if (!m_pref_EnableRemoteInputs)
		m_server->EnableRemoteInputs(m_pref_EnableRemoteInputs);
	if (m_pref_DisableLocalInputs)
		m_server->DisableLocalInputs(m_pref_DisableLocalInputs);

	// Update the password
	m_server->SetPassword(m_pref_passwd);
	m_server->SetPasswordViewOnly(m_pref_passwd_viewonly);

	// Now change the listening port settings
	m_server->SetAutoPortSelect(m_pref_AutoPortSelect);
	if (!m_pref_AutoPortSelect)
		m_server->SetPorts(m_pref_PortNumber, m_pref_HttpPortNumber);
	m_server->SockConnect(m_pref_SockConnect);

	// Set the beep options
	m_server->SetBeepConnect(m_pref_BeepConnect);
	m_server->SetBeepDisconnect(m_pref_BeepDisconnect);
	
	// Set the CORBA connection status
	m_server->CORBAConnect(m_pref_CORBAConn);

	// Remote access prefs
	m_server->EnableRemoteInputs(m_pref_EnableRemoteInputs);
	m_server->SetLockSettings(m_pref_LockSettings);
	m_server->DisableLocalInputs(m_pref_DisableLocalInputs);

	// Polling prefs
	m_server->PollUnderCursor(m_pref_PollUnderCursor);
	m_server->PollForeground(m_pref_PollForeground);
	m_server->PollFullScreen(m_pref_PollFullScreen);
	m_server->PollConsoleOnly(m_pref_PollConsoleOnly);
	m_server->PollOnEventOnly(m_pref_PollOnEventOnly);
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

	// Connection querying settings
	// FIXME: Currently query settings are saved in vncAdvancedProperties,
	//        but actually here is a better place.
	// SaveInt(appkey, "QuerySetting", m_server->QuerySetting());
	// SaveInt(appkey, "QueryTimeout", m_server->QueryTimeout());

	// Lock settings
	SaveInt(appkey, "LockSetting", m_server->LockSettings());

	// Wallpaper removal
	SaveInt(appkey, "RemoveWallpaper", m_server->RemoveWallpaperEnabled());

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
}
