// vncAdvancedProperties.cpp

// Implementation of the Advanced Properties dialog!

#include "stdhdrs.h"
#include "lmcons.h"
#include "vncService.h"

#include "WinVNC.h"
#include "vncAdvancedProperties.h"
#include "vncServer.h"
#include "vncPasswd.h"

const char WINVNC_REGISTRY_KEY [] = "Software\\ORL\\WinVNC3";
const char NO_OVERRIDE_ERR [] = "This machine has been preconfigured with WinVNC settings, "
								"which cannot be overridden by individual users.  "
								"The preconfigured settings may be modified only by a System Administrator.";
const char NO_CURRENT_USER_ERR [] = "The WinVNC settings for the current user are unavailable at present.";
const char CANNOT_EDIT_DEFAULT_PREFS [] = "You do not have sufficient priviliges to edit the default local WinVNC settings.";

// Constructor & Destructor
vncAdvancedProperties::vncAdvancedProperties()
{
	m_dlgvisible = FALSE;
	m_usersettings = TRUE;
}

vncAdvancedProperties::~vncAdvancedProperties()
{
}

// Initialization
BOOL
vncAdvancedProperties::Init(vncServer *server)
{
	// Save the server pointer
	m_server = server;
	
	// Load the settings from the registry
	Load(TRUE);

	return TRUE;
}

// Dialog box handling functions
void
vncAdvancedProperties::Show(BOOL show, BOOL usersettings)
{
	if (show)
	{
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
				vnclog.Print(LL_INTINFO, VNCLOG("show per-user Advanced Properties\n"));
			else
				vnclog.Print(LL_INTINFO, VNCLOG("show default system Advanced Properties\n"));

			// Load in the settings relevant to the user or system
			Load(usersettings);

				m_returncode_valid = FALSE;

				// Do the dialog box
				int result = DialogBoxParam(hAppInstance,
				    MAKEINTRESOURCE(IDD_ADVPROPERTIES), 
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
				
			// Load in all the settings
			Load(TRUE);
		}
	}
}

/*
BOOL vncAdvancedProperties::DoDialog()
{
	int retVal = DialogBoxParam(hAppInstance, MAKEINTRESOURCE(IDD_ADVPROPERTIES), 
		NULL, (DLGPROC) DialogProc, (LONG) this);
	delete this;
	return retVal;
}
*/

BOOL CALLBACK
vncAdvancedProperties::DialogProc(HWND hwnd,
						  UINT uMsg,
						  WPARAM wParam,
						  LPARAM lParam )
{
	// We use the dialog-box's USERDATA to store a _this pointer
	// This is set only once WM_INITDIALOG has been recieved, though!
	vncAdvancedProperties *_this = (vncAdvancedProperties *) GetWindowLong(hwnd, GWL_USERDATA);

	switch (uMsg)
	{

	case WM_INITDIALOG:
		{
			// Retrieve the Dialog box parameter and use it as a pointer
			// to the calling vncProperties object
			SetWindowLong(hwnd, GWL_USERDATA, lParam);
			_this = (vncAdvancedProperties *) lParam;

			// Set the dialog box's title to indicate which Properties we're editting
			if (_this->m_usersettings) {
				SetWindowText(hwnd, "WinVNC: Current User Advanced Properties");
			} else {
				SetWindowText(hwnd, "WinVNC: Default Local System Advanced Properties");
			}

			// Initialise the properties controls
			HWND hQuery = GetDlgItem(hwnd, IDQUERY);
			BOOL queryEnabled = (_this->m_server->QuerySetting() == 4);
			SendMessage(hQuery,
				BM_SETCHECK,
				queryEnabled,
				0);

			HWND hActionRefuse = GetDlgItem(hwnd, IDC_ACTION_REFUSE);
			HWND hActionAccept = GetDlgItem(hwnd, IDC_ACTION_ACCEPT);
			HWND hDefaultAction = (_this->m_server->QueryAccept()) ?
				hActionAccept : hActionRefuse;
			SendMessage(hDefaultAction, BM_SETCHECK, TRUE, 0);
			EnableWindow(hActionRefuse, queryEnabled);
			EnableWindow(hActionAccept, queryEnabled);

			HWND hQueryAllowNoPass = GetDlgItem(hwnd, IDQUERYALLOWNOPASS);
			SendMessage(hQueryAllowNoPass,
				BM_SETCHECK,
				_this->m_server->QueryAllowNoPass(),
				0);
			EnableWindow(hQueryAllowNoPass, queryEnabled);
			
			HWND hQueryTimeout = GetDlgItem(hwnd, IDQUERYTIMEOUT);
			EnableWindow(hQueryTimeout, queryEnabled);
			
			// Get the timeout
			char timeout[128];
			UINT t;
			t = _this->m_server->QueryTimeout();
			sprintf(timeout, "%d", (int)t);
		    SetDlgItemText(hwnd, IDQUERYTIMEOUT, (const char *) timeout);

			SendDlgItemMessage(hwnd, IDENABLEHTTPD,
				BM_SETCHECK,
				_this->m_server->HttpdEnabled(),
				0);

			HWND hLoopback = GetDlgItem(hwnd, IDALLOWLOOPBACK);
			BOOL loopbackEnabled = _this->m_server->LoopbackOk();
			SendMessage(hLoopback,
				BM_SETCHECK,
				loopbackEnabled,
				0);
			HWND hLoopbackOnly = GetDlgItem(hwnd, IDONLYLOOPBACK);
			SendMessage(hLoopbackOnly,
				BM_SETCHECK,
				_this->m_server->LoopbackOnly(),
				0);
			EnableWindow(hLoopbackOnly, loopbackEnabled);

			SendDlgItemMessage(hwnd, IDREQUIREAUTH,
				BM_SETCHECK,
				_this->m_server->AuthRequired(),
				0);

			int priority = _this->m_server->ConnectPriority();
			if (priority == 1)
				CheckDlgButton(hwnd, IDPRIORITY1, BST_CHECKED);
			if (priority == 2)
				CheckDlgButton(hwnd, IDPRIORITY2, BST_CHECKED);
			if (priority == 0)
				CheckDlgButton(hwnd, IDPRIORITY0, BST_CHECKED);

			BOOL logstate = (vnclog.GetMode() >= 2);
			if (logstate)
				CheckDlgButton(hwnd, IDLOG, BST_CHECKED);
			else
				CheckDlgButton(hwnd, IDLOG, BST_UNCHECKED);
			HWND hLogLots = GetDlgItem(hwnd, IDLOGLOTS);
			EnableWindow(hLogLots, logstate);
			if (vnclog.GetLevel() > 5)
				CheckDlgButton(hwnd, IDLOGLOTS, BST_CHECKED);
			else
				CheckDlgButton(hwnd, IDLOGLOTS, BST_UNCHECKED);
			
			
			SetForegroundWindow(hwnd);

			/////
			_this->m_dlgvisible = TRUE;

			return TRUE;
		}

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{

		case IDOK:
		case IDC_APPLY:
			{
				// Save the timeout
				char timeout[256];
				if (GetDlgItemText(hwnd, IDQUERYTIMEOUT, (LPSTR) &timeout, 256) == 0)
				{
				    _this->m_server->SetQueryTimeout(atoi(timeout));
				}
				else
				{
				    _this->m_server->SetQueryTimeout(atoi(timeout));
				}

				// Save the new settings to the server
				HWND hQuery = GetDlgItem(hwnd, IDQUERY);
				_this->m_server->SetQuerySetting(
					(SendMessage(hQuery, BM_GETCHECK, 0, 0) == BST_CHECKED)
					? 4 : 2
					);

				HWND hQueryAccept = GetDlgItem(hwnd, IDC_ACTION_ACCEPT);
				_this->m_server->SetQueryAccept(
					SendMessage(hQueryAccept, BM_GETCHECK, 0, 0) == BST_CHECKED
					);

				HWND hQueryAllowNoPass = GetDlgItem(hwnd, IDQUERYALLOWNOPASS);
				_this->m_server->SetQueryAllowNoPass(
					SendMessage(hQueryAllowNoPass, BM_GETCHECK, 0, 0) == BST_CHECKED
					);

				if (IsDlgButtonChecked(hwnd, IDPRIORITY1))
					_this->m_server->SetConnectPriority(1);
				if (IsDlgButtonChecked(hwnd, IDPRIORITY2))
					_this->m_server->SetConnectPriority(2);
				if (IsDlgButtonChecked(hwnd, IDPRIORITY0))
					_this->m_server->SetConnectPriority(0);

				_this->m_server->SetAuthRequired(IsDlgButtonChecked(hwnd, IDREQUIREAUTH));
				_this->m_server->SetHttpdEnabled(IsDlgButtonChecked(hwnd, IDENABLEHTTPD),
												 _this->m_server->HttpdParamsEnabled());
				_this->m_server->SetLoopbackOk(IsDlgButtonChecked(hwnd, IDALLOWLOOPBACK));
				_this->m_server->SetLoopbackOnly(IsDlgButtonChecked(hwnd, IDONLYLOOPBACK));

				if (IsDlgButtonChecked(hwnd, IDLOG))
					vnclog.SetMode(2);
				else
					vnclog.SetMode(0);

				if (IsDlgButtonChecked(hwnd, IDLOGLOTS))
					vnclog.SetLevel(10);
				else
					vnclog.SetLevel(2);

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

		case IDQUERY:
			{
				HWND hQuery = GetDlgItem(hwnd, IDQUERY);
				BOOL queryon =
					(SendMessage(hQuery, BM_GETCHECK, 0, 0) == BST_CHECKED);

				EnableWindow(GetDlgItem(hwnd, IDC_ACTION_REFUSE), queryon);
				EnableWindow(GetDlgItem(hwnd, IDC_ACTION_ACCEPT), queryon);
				EnableWindow(GetDlgItem(hwnd, IDQUERYTIMEOUT), queryon);
				EnableWindow(GetDlgItem(hwnd, IDQUERYALLOWNOPASS), queryon);
			}
			return TRUE;

		case IDLOG:
			{
				BOOL logon = IsDlgButtonChecked(hwnd, IDLOG);
				HWND hLogLots = GetDlgItem(hwnd, IDLOGLOTS);
				EnableWindow(hLogLots, logon);
			}
			return TRUE;

		case IDALLOWLOOPBACK:
			{
				BOOL loopon = IsDlgButtonChecked(hwnd, IDALLOWLOOPBACK);
				HWND hOnlyLoopback = GetDlgItem(hwnd, IDONLYLOOPBACK);
				EnableWindow(hOnlyLoopback, loopon);
				CheckDlgButton(hwnd, IDONLYLOOPBACK, BST_UNCHECKED);
			}
			return TRUE;

		}

		break;
	}
	return 0;
}

// Functions to load & save the settings
LONG
vncAdvancedProperties::LoadInt(HKEY key, LPCSTR valname, LONG defval)
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

char *
vncAdvancedProperties::LoadString(HKEY key, LPCSTR keyname)
{
	DWORD type = REG_SZ;
	DWORD buflen = 0;
	BYTE *buffer = 0;

	// Get the length of the string
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

	// Get the string data
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
vncAdvancedProperties::Load(BOOL usersettings)
{
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

	// Authentication required, httpd enabled, loopback allowed, loopbackOnly
	m_server->SetHttpdEnabled(LoadInt(hkLocal, "EnableHTTPDaemon", true),
							  LoadInt(hkLocal, "EnableURLParams", false));
	m_server->SetLoopbackOnly(LoadInt(hkLocal, "LoopbackOnly", false));
	if (m_server->LoopbackOnly())
		m_server->SetLoopbackOk(true);
	else
		m_server->SetLoopbackOk(LoadInt(hkLocal, "AllowLoopback", false));
	m_server->SetAuthRequired(LoadInt(hkLocal, "AuthRequired", true));
	m_server->SetConnectPriority(LoadInt(hkLocal, "ConnectPriority", 0));
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
	m_pref_QuerySetting=2;
	m_pref_QueryTimeout=30;
	m_pref_QueryAccept=FALSE;
	m_pref_QueryAllowNoPass=FALSE;

	// Load the local prefs for this user
	if (hkDefault != NULL)
	{
		vnclog.Print(LL_INTINFO, VNCLOG("loading DEFAULT local settings\n"));
		LoadUserPrefs(hkDefault);
	}

	// Are we being asked to load the user settings, or just the default local system settings?
	if (usersettings) {
		// We want the user settings, so load them!

		if (hkLocalUser != NULL)
		{
			vnclog.Print(LL_INTINFO, VNCLOG("loading \"%s\" local settings\n"), username);
			LoadUserPrefs(hkLocalUser);
		}

		// Now override the system settings with the user's settings
		// If the username is SYSTEM then don't try to load them, because there aren't any...
		if (strcmp(username, "SYSTEM") != 0)
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
vncAdvancedProperties::LoadUserPrefs(HKEY appkey)
{
	// LOAD USER PREFS FROM THE SELECTED KEY

	// Connection querying settings
	m_pref_QuerySetting=LoadInt(appkey, "QuerySetting", m_pref_QuerySetting);
	m_server->SetQuerySetting(m_pref_QuerySetting);
	m_pref_QueryTimeout=LoadInt(appkey, "QueryTimeout", m_pref_QueryTimeout);
	m_server->SetQueryTimeout(m_pref_QueryTimeout);
	m_pref_QueryAccept=LoadInt(appkey, "QueryAccept", m_pref_QueryAccept);
	m_server->SetQueryAccept(m_pref_QueryAccept);
	m_pref_QueryAllowNoPass=LoadInt(appkey, "QueryAllowNoPass", m_pref_QueryAllowNoPass);
	m_server->SetQueryAllowNoPass(m_pref_QueryAllowNoPass);
}

void
vncAdvancedProperties::ApplyUserPrefs()
{
	// APPLY THE CACHED PREFERENCES TO THE SERVER

	// Update the connection querying settings
	m_server->SetQuerySetting(m_pref_QuerySetting);
	m_server->SetQueryTimeout(m_pref_QueryTimeout);
	m_server->SetQueryAccept(m_pref_QueryAccept);
	m_server->SetQueryAllowNoPass(m_pref_QueryAllowNoPass);

	// Machine Preferences
	//int priority = 0;
	//if (IsDlgButtonChecked(hwnd, IDPRIORITY1))
	//	priority = 1;
	//if (IsDlgButtonChecked(hwnd, IDPRIORITY2))
	//	priority = 2;
	//m_server->SetConnectPriority(priority);

	//m_server->SetAuthRequired(IsDlgButtonChecked(hwnd, IDREQUIREAUTH));
	//m_server->SetHttpdEnabled(IsDlgButtonChecked(hwnd, IDENABLEHTTPD));
	//m_server->SetLoopbackOk(IsDlgButtonChecked(hwnd, IDALLOWLOOPBACK));
	//m_server->SetLoopbackOnly(IsDlgButtonChecked(hwnd, IDONLYLOOPBACK));

	//if (IsDlgButtonChecked(hwnd, IDLOG))
	//	vnclog.SetMode(2);
	//else
	//	vnclog.SetMode(0);

	//if (IsDlgButtonChecked(hwnd, IDLOGLOTS))
	//	vnclog.SetLevel(12);
	//else
	//	vnclog.SetLevel(2);
}

void
vncAdvancedProperties::SaveInt(HKEY key, LPCSTR valname, LONG val)
{
	RegSetValueEx(key, valname, 0, REG_DWORD, (LPBYTE) &val, sizeof(val));
}

void
vncAdvancedProperties::Save()
{
	HKEY appkey;
	DWORD dw;

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
	SaveInt(hkLocal, "DebugMode", vnclog.GetMode());
	SaveInt(hkLocal, "DebugLevel", vnclog.GetLevel());
	SaveInt(hkLocal, "LoopbackOnly", m_server->LoopbackOnly());
	SaveInt(hkLocal, "EnableHTTPDaemon", m_server->HttpdEnabled());
	SaveInt(hkLocal, "EnableURLParams", m_server->HttpdParamsEnabled());
	SaveInt(hkLocal, "AllowLoopback", m_server->LoopbackOk());
	SaveInt(hkLocal, "AuthRequired", m_server->AuthRequired());
	RegCloseKey(hkLocal);

	// Close the user registry hive, to allow it to unload if reqd
	RegCloseKey(HKEY_CURRENT_USER);
}

void
vncAdvancedProperties::SaveUserPrefs(HKEY appkey)
{
	// SAVE THE PER USER PREFS
	vnclog.Print(LL_INTINFO, VNCLOG("saving current settings to registry\n"));

	// Connection querying settings
	SaveInt(appkey, "QuerySetting", m_server->QuerySetting());
	SaveInt(appkey, "QueryTimeout", m_server->QueryTimeout());
	SaveInt(appkey, "QueryAccept", m_server->QueryAccept());
	SaveInt(appkey, "QueryAllowNoPass", m_server->QueryAllowNoPass());
}

