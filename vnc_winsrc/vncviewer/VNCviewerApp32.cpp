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

#include "VNCviewerApp32.h"
#include "vncviewer.h"
#include "Exception.h"

// --------------------------------------------------------------------------
VNCviewerApp32::VNCviewerApp32(HINSTANCE hInstance, PSTR szCmdLine) :
	VNCviewerApp(hInstance, szCmdLine)
{

	m_pdaemon = NULL;

	// Load a requested keyboard layout
	if (m_options.m_kbdSpecified) {
		HKL hkl = LoadKeyboardLayout(  m_options.m_kbdname, 
			KLF_ACTIVATE | KLF_REPLACELANG | KLF_REORDER  );
		if (hkl == NULL) {
			MessageBox(NULL, _T("Error loading specified keyboard layout"), 
				_T("VNC info"), MB_OK | MB_ICONSTOP);
			exit(1);
		}
	}

	// Start listening daemons if requested
	
	if ((m_options.m_listening) && (FindWindow("VNCviewer Daemon", 0) == NULL)) {
		vnclog.Print(3, _T("In listening mode - staring daemons\n"));
		ListenMode();
	} else {
		m_options.m_listening = false;
	}

	RegisterSounds();
}

	
// These should maintain a list of connections.
// FIXME: Eliminate duplicated code, see the following three functions.

void VNCviewerApp32::NewConnection() {
	int retries = 0;
	ClientConnection *pcc;
	ClientConnection *old_pcc;

	pcc = new ClientConnection(this);
	while (retries < MAX_AUTH_RETRIES) {
		try {
			pcc->Run();
			return;
		} catch (AuthException &e) {
			e.Report();
			pcc->UnloadConnection();
			// If the connection count drops to zero, the app exits.
			old_pcc = pcc;
			pcc = new ClientConnection(this);
			// Get the previous options for the next try.
			pcc->CopyOptions(old_pcc);
			delete old_pcc;
		} catch (Exception &e) {
			e.Report();
			break;
		}
		retries++;
	}
	delete pcc;
}

void VNCviewerApp32::NewConnection(TCHAR *host, int port) {
	int retries = 0;
	ClientConnection *pcc;
	ClientConnection *old_pcc;

	pcc = new ClientConnection(this, host, port);
	while (retries < MAX_AUTH_RETRIES) {
		try {
			pcc->Run();
			return;
		} catch (AuthException &e) {
			e.Report();
			// If the connection count drops to zero, the app exits.
			old_pcc = pcc;
			pcc = new ClientConnection(this, host, port);
			// Get the previous options for the next try.
			pcc->CopyOptions(old_pcc);
			delete old_pcc;
		} catch (Exception &e) {
			e.Report();	
			break;
		}
		retries++;
	}
	delete pcc;
}

void VNCviewerApp32::NewConnection(SOCKET sock) {
	int retries = 0;
	ClientConnection *pcc;
	ClientConnection *old_pcc;

	pcc = new ClientConnection(this, sock);
	while (retries < MAX_AUTH_RETRIES) {
		try {
			pcc->Run();
			return;
		} catch (AuthException &e) {
			e.Report();
			// If the connection count drops to zero, the app exits.
			old_pcc = pcc;
			pcc = new ClientConnection(this, sock);
			// Get the previous options for the next try.
			pcc->CopyOptions(old_pcc);
			delete old_pcc;
		} catch (Exception &e) {
			e.Report();
			break;
		}
		retries++;
	}
	delete pcc;
}

void VNCviewerApp32::ListenMode() {

	try {
		m_pdaemon = new Daemon(m_options.m_listenPort);
	} catch (WarningException &e) {
		char msg[1024];
		sprintf(msg, "Error creating listening daemon:\n\r(%s)\n\r%s",
				e.m_info, "Perhaps another VNCviewer is already running?");
		MessageBox(NULL, msg, "VNCviewer error", MB_OK | MB_ICONSTOP);
		exit(1);
	}
}

// Register the Bell sound event

const char* BELL_APPL_KEY_NAME  = "AppEvents\\Schemes\\Apps\\VNCviewer";
const char* BELL_LABEL = "VNCviewerBell";

void VNCviewerApp32::RegisterSounds() {
	
	HKEY hBellKey;
	char keybuf[256];
	
	sprintf(keybuf, "AppEvents\\EventLabels\\%s", BELL_LABEL);
	// First create a label for it
	if ( RegCreateKey(HKEY_CURRENT_USER, keybuf, &hBellKey)  == ERROR_SUCCESS ) {
		RegSetValue(hBellKey, NULL, REG_SZ, "Bell", 0);
		RegCloseKey(hBellKey);
		
		// Then put the detail in the app-specific area
		
		if ( RegCreateKey(HKEY_CURRENT_USER, BELL_APPL_KEY_NAME, &hBellKey)  == ERROR_SUCCESS ) {
			
			sprintf(keybuf, "%s\\%s", BELL_APPL_KEY_NAME, BELL_LABEL);
			RegCreateKey(HKEY_CURRENT_USER, keybuf, &hBellKey);
			RegSetValue(hBellKey, NULL, REG_SZ, "Bell", 0);
			RegCloseKey(hBellKey);
			
			sprintf(keybuf, "%s\\%s\\.current", BELL_APPL_KEY_NAME, BELL_LABEL);
			if (RegOpenKey(HKEY_CURRENT_USER, keybuf, &hBellKey) != ERROR_SUCCESS) {
				RegCreateKey(HKEY_CURRENT_USER, keybuf, &hBellKey);
				RegSetValue(hBellKey, NULL, REG_SZ, "ding.wav", 0);
			}
			RegCloseKey(hBellKey);
			
			sprintf(keybuf, "%s\\%s\\.default", BELL_APPL_KEY_NAME, BELL_LABEL);
			if (RegOpenKey(HKEY_CURRENT_USER, keybuf, &hBellKey) != ERROR_SUCCESS) {
				RegCreateKey(HKEY_CURRENT_USER, keybuf, &hBellKey);
				RegSetValue(hBellKey, NULL, REG_SZ, "ding.wav", 0);
			}
			RegCloseKey(hBellKey);
		}
		
	} 
	
}

bool VNCviewerApp32::ProcessDialogMessage(MSG *pmsg)
{
	if (!m_dialogs.empty()) {
		omni_mutex_lock l(m_dialogsMutex);
		std::list<HWND>::iterator iter;
		for (iter = m_dialogs.begin(); iter != m_dialogs.end(); iter++) {
			if (IsDialogMessage(*iter, pmsg))
				return true;
		}
	}
	return false;
}

VNCviewerApp32::~VNCviewerApp32() {
	// We don't need to clean up pcc if the thread has been joined.
	if (m_pdaemon != NULL) delete m_pdaemon;
}
	
