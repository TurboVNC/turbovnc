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


// VNCOptions.cpp: implementation of the VNCOptions class.

#include "stdhdrs.h"
#include "vncviewer.h"
#include "VNCOptions.h"
#include "Exception.h"

VNCOptions::VNCOptions()
{
	for (int i = rfbEncodingRaw; i<= LASTENCODING; i++)
		m_UseEnc[i] = true;
	
	m_UseEnc[3] = false;

	m_ViewOnly = false;
	m_FullScreen = false;
	m_Use8Bit = false;
	m_PreferredEncoding = rfbEncodingTight;
	m_SwapMouse = false;
	m_Emul3Buttons = true;
	m_Emul3Timeout = 100; // milliseconds
	m_Emul3Fuzz = 4;      // pixels away before emulation is cancelled
	m_Shared = true;
	m_DeiconifyOnBell = false;
	m_DisableClipboard = false;
	m_localCursor = DOTCURSOR;
	m_scaling = false;
	m_scale_num = 1;
	m_scale_den = 1;
	
	m_host[0] = '\0';
	m_port = -1;
	
	m_kbdname[0] = '\0';
	m_kbdSpecified = false;
	
	m_logLevel = 0;
	m_logToConsole = false;
	m_logToFile = false;
	m_logFilename[0] = '\0';
	
	m_delay=0;
	m_connectionSpecified = false;
	m_configSpecified = false;
	m_configFilename[0] = '\0';
	m_listening = false;
	m_listenPort = INCOMING_PORT_OFFSET;
	m_restricted = false;

	m_useCompressLevel = false;
	m_compressLevel = 6;
	m_enableJpegCompression = true;
	m_jpegQualityLevel = 6;
	m_requestShapeUpdates = true;
	m_ignoreShapeUpdates = false;

#ifdef UNDER_CE
	m_palmpc = false;
	
	// Check for PalmPC aspect 
	HDC temp_hdc = GetDC(NULL);
	int screen_width = GetDeviceCaps(temp_hdc, HORZRES);
	if (screen_width < 320)
	{
		m_palmpc = true;
	}
	ReleaseDC(NULL,temp_hdc);

	m_slowgdi = false;
#endif
}

VNCOptions& VNCOptions::operator=(VNCOptions& s)
{
	for (int i = rfbEncodingRaw; i<= LASTENCODING; i++)
		m_UseEnc[i] = s.m_UseEnc[i];
	
	m_ViewOnly			= s.m_ViewOnly;
	m_FullScreen		= s.m_FullScreen;
	m_Use8Bit			= s.m_Use8Bit;
	m_PreferredEncoding = s.m_PreferredEncoding;
	m_SwapMouse			= s.m_SwapMouse;
	m_Emul3Buttons		= s.m_Emul3Buttons;
	m_Emul3Timeout		= s.m_Emul3Timeout;
	m_Emul3Fuzz			= s.m_Emul3Fuzz;      // pixels away before emulation is cancelled
	m_Shared			= s.m_Shared;
	m_DeiconifyOnBell	= s.m_DeiconifyOnBell;
	m_DisableClipboard  = s.m_DisableClipboard;
	m_scaling			= s.m_scaling;
	m_scale_num			= s.m_scale_num;
	m_scale_den			= s.m_scale_den;
	m_localCursor		= s.m_localCursor;
	
	strcpy(m_host, s.m_host);
	m_port				= s.m_port;
	
	strcpy(m_kbdname, s.m_kbdname);
	m_kbdSpecified		= s.m_kbdSpecified;
	
	m_logLevel			= s.m_logLevel;
	m_logToConsole		= s.m_logToConsole;
	m_logToFile			= s.m_logToFile;
	strcpy(m_logFilename, s.m_logFilename);

	m_delay				= s.m_delay;
	m_connectionSpecified = s.m_connectionSpecified;
	m_configSpecified   = s.m_configSpecified;
	strcpy(m_configFilename, s.m_configFilename);

	m_listening			= s.m_listening;
	m_listenPort		= s.m_listenPort;
	m_restricted		= s.m_restricted;

	m_useCompressLevel		= s.m_useCompressLevel;
	m_compressLevel			= s.m_compressLevel;
	m_enableJpegCompression	= s.m_enableJpegCompression;
	m_jpegQualityLevel		= s.m_jpegQualityLevel;
	m_requestShapeUpdates	= s.m_requestShapeUpdates;
	m_ignoreShapeUpdates	= s.m_ignoreShapeUpdates;

#ifdef UNDER_CE
	m_palmpc			= s.m_palmpc;
	m_slowgdi			= s.m_slowgdi;
#endif
	return *this;
}

VNCOptions::~VNCOptions()
{
	
}

inline bool SwitchMatch(LPCTSTR arg, LPCTSTR swtch) {
	return (arg[0] == '-' || arg[0] == '/') &&
		(_tcsicmp(&arg[1], swtch) == 0);
}

static void ArgError(LPTSTR msg) {
    MessageBox(NULL,  msg, _T("Argument error"),MB_OK | MB_TOPMOST | MB_ICONSTOP);
}

// Greatest common denominator, by Euclid
int gcd(int a, int b) {
	if (a < b) return gcd(b,a);
	if (b == 0) return a;
	return gcd(b, a % b);
}

void VNCOptions::FixScaling() {
	if (m_scale_num < 1 || m_scale_den < 1) {
		MessageBox(NULL,  _T("Invalid scale factor - resetting to normal scale"), 
			_T("Argument error"),MB_OK | MB_TOPMOST | MB_ICONWARNING);
		m_scale_num = 1;
		m_scale_den = 1;	
		m_scaling = false;
	}
	int g = gcd(m_scale_num, m_scale_den);
	m_scale_num /= g;
	m_scale_den /= g;	
}

void VNCOptions::SetFromCommandLine(LPTSTR szCmdLine) {
	// We assume no quoting here.
	// Copy the command line - we don't know what might happen to the original
	int cmdlinelen = _tcslen(szCmdLine);
	if (cmdlinelen == 0) return;
	
	TCHAR *cmd = new TCHAR[cmdlinelen + 1];
	_tcscpy(cmd, szCmdLine);
	
	// Count the number of spaces
	// This may be more than the number of arguments, but that doesn't matter.
	int nspaces = 0;
	TCHAR *p = cmd;
	TCHAR *pos = cmd;
	while ( ( pos = _tcschr(p, ' ') ) != NULL ) {
		nspaces ++;
		p = pos + 1;
	}
	
	// Create the array to hold pointers to each bit of string
	TCHAR **args = new LPTSTR[nspaces + 1];
	
	// replace spaces with nulls and
	// create an array of TCHAR*'s which points to start of each bit.
	pos = cmd;
	int i = 0;
	args[i] = cmd;
	bool inquote=false;
	for (pos = cmd; *pos != 0; pos++) {
		// Arguments are normally separated by spaces, unless there's quoting
		if ((*pos == ' ') && !inquote) {
			*pos = '\0';
			p = pos + 1;
			args[++i] = p;
		}
		if (*pos == '"') {  
			if (!inquote) {      // Are we starting a quoted argument?
				args[i] = ++pos; // It starts just after the quote
			} else {
				*pos = '\0';     // Finish a quoted argument?
			}
			inquote = !inquote;
		}
	}
	i++;

	bool hostGiven = false, portGiven = false;
	// take in order.
	for (int j = 0; j < i; j++) {
		if ( SwitchMatch(args[j], _T("help")) ||
			SwitchMatch(args[j], _T("?")) ||
			SwitchMatch(args[j], _T("h"))) {
			ShowUsage();
			PostQuitMessage(1);
		} else if ( SwitchMatch(args[j], _T("listen"))) {
			m_listening = true;
			if (j+1 < i && args[j+1][0] >= '0' && args[j+1][0] <= '9') {
				if (_stscanf(args[j+1], _T("%d"), &m_listenPort) != 1) {
					ArgError(_T("Invalid listen port specified"));
					continue;
				}
				j++;
			}
		} else if ( SwitchMatch(args[j], _T("restricted"))) {
			m_restricted = true;
		} else if ( SwitchMatch(args[j], _T("viewonly"))) {
			m_ViewOnly = true;
		} else if ( SwitchMatch(args[j], _T("fullscreen"))) {
			m_FullScreen = true;
		} else if ( SwitchMatch(args[j], _T("8bit"))) {
			m_Use8Bit = true;
		} else if ( SwitchMatch(args[j], _T("shared"))) {
			m_Shared = true;
		} else if ( SwitchMatch(args[j], _T("noshared"))) {
			m_Shared = false;
		} else if ( SwitchMatch(args[j], _T("swapmouse"))) {
			m_SwapMouse = true;
		} else if ( SwitchMatch(args[j], _T("nocursor"))) {
			m_localCursor = NOCURSOR;
		} else if ( SwitchMatch(args[j], _T("dotcursor"))) {
			m_localCursor = DOTCURSOR;
		} else if ( SwitchMatch(args[j], _T("normalcursor"))) {
			m_localCursor = NORMALCURSOR;
		} else if ( SwitchMatch(args[j], _T("belldeiconify") )) {
			m_DeiconifyOnBell = true;
		} else if ( SwitchMatch(args[j], _T("emulate3") )) {
			m_Emul3Buttons = true;
		} else if ( SwitchMatch(args[j], _T("noemulate3") )) {
			m_Emul3Buttons = false;
		} else if ( SwitchMatch(args[j], _T("nojpeg") )) {
			m_enableJpegCompression = false;
		} else if ( SwitchMatch(args[j], _T("nocursorshape") )) {
			m_requestShapeUpdates = false;
		} else if ( SwitchMatch(args[j], _T("noremotecursor") )) {
			m_requestShapeUpdates = true;
			m_ignoreShapeUpdates = true;
		} else if ( SwitchMatch(args[j], _T("scale") )) {
			if (++j == i) {
				ArgError(_T("No scaling factor specified"));
				continue;
			}
			int numscales = _stscanf(args[j], _T("%d/%d"), &m_scale_num, &m_scale_den);
			if (numscales < 1) {
				ArgError(_T("Invalid scaling specified"));
				continue;
			}
			if (numscales == 1) 
				m_scale_den = 1; // needed if you're overriding a previous setting

		} else if ( SwitchMatch(args[j], _T("emulate3timeout") )) {
			if (++j == i) {
				ArgError(_T("No timeout specified"));
				continue;
			}
			if (_stscanf(args[j], _T("%d"), &m_Emul3Timeout) != 1) {
				ArgError(_T("Invalid timeout specified"));
				continue;
			}
			
		} else if ( SwitchMatch(args[j], _T("emulate3fuzz") )) {
			if (++j == i) {
				ArgError(_T("No fuzz specified"));
				continue;
			}
			if (_stscanf(args[j], _T("%d"), &m_Emul3Fuzz) != 1) {
				ArgError(_T("Invalid fuzz specified"));
				continue;
			}
			
		} else if ( SwitchMatch(args[j], _T("disableclipboard") )) {
			m_DisableClipboard = true;
		}
#ifdef UNDER_CE
		// Manual setting of palm vs hpc aspect ratio for dialog boxes.
		else if ( SwitchMatch(args[j], _T("hpc") )) {
			m_palmpc = false;
		} else if ( SwitchMatch(args[j], _T("palm") )) {
			m_palmpc = true;
		} else if ( SwitchMatch(args[j], _T("slow") )) {
			m_slowgdi = true;
		} 
#endif
		else if ( SwitchMatch(args[j], _T("delay") )) {
			if (++j == i) {
				ArgError(_T("No delay specified"));
				continue;
			}
			if (_stscanf(args[j], _T("%d"), &m_delay) != 1) {
				ArgError(_T("Invalid delay specified"));
				continue;
			}
			
		} else if ( SwitchMatch(args[j], _T("loglevel") )) {
			if (++j == i) {
				ArgError(_T("No loglevel specified"));
				continue;
			}
			if (_stscanf(args[j], _T("%d"), &m_logLevel) != 1) {
				ArgError(_T("Invalid loglevel specified"));
				continue;
			}
			
		} else if ( SwitchMatch(args[j], _T("console") )) {
			m_logToConsole = true;
		} else if ( SwitchMatch(args[j], _T("logfile") )) {
			if (++j == i) {
				ArgError(_T("No logfile specified"));
				continue;
			}
			if (_stscanf(args[j], _T("%s"), &m_logFilename) != 1) {
				ArgError(_T("Invalid logfile specified"));
				continue;
			} else {
				m_logToFile = true;
			}
		} else if ( SwitchMatch(args[j], _T("config") )) {
			if (++j == i) {
				ArgError(_T("No config file specified"));
				continue;
			}
			// The GetPrivateProfile* stuff seems not to like some relative paths
			_fullpath(m_configFilename, args[j], _MAX_PATH);
			if (_access(m_configFilename, 04)) {
				ArgError(_T("Can't open specified config file for reading."));
				PostQuitMessage(1);
				continue;
			} else {
				Load(m_configFilename);
				m_configSpecified = true;
			}
		} else if ( SwitchMatch(args[j], _T("encoding") )) {
			if (++j == i) {
				ArgError(_T("No encoding specified"));
				continue;
			}
			int enc = -1;
			if (_tcsicmp(args[j], _T("raw")) == 0) {
				enc = rfbEncodingRaw;
			} else if (_tcsicmp(args[j], _T("rre")) == 0) {
				enc = rfbEncodingRRE;
			} else if (_tcsicmp(args[j], _T("corre")) == 0) {
				enc = rfbEncodingCoRRE;
			} else if (_tcsicmp(args[j], _T("hextile")) == 0) {
				enc = rfbEncodingHextile;
			} else if (_tcsicmp(args[j], _T("zlib")) == 0) {
				enc = rfbEncodingZlib;
			} else if (_tcsicmp(args[j], _T("tight")) == 0) {
				enc = rfbEncodingTight;
			} else if (_tcsicmp(args[j], _T("zlibhex")) == 0) {
				enc = rfbEncodingZlibHex;
			} else {
				ArgError(_T("Invalid encoding specified"));
				continue;
			}
			if (enc != -1) {
				m_UseEnc[enc] = true;
				m_PreferredEncoding = enc;
			}
		} else if ( SwitchMatch(args[j], _T("compresslevel") )) {
			if (++j == i) {
				ArgError(_T("No compression level specified"));
				continue;
			}
			m_useCompressLevel = true;
			if (_stscanf(args[j], _T("%d"), &m_compressLevel) != 1) {
				ArgError(_T("Invalid compression level specified"));
				continue;
			}
		} else if ( SwitchMatch(args[j], _T("quality") )) {
			if (++j == i) {
				ArgError(_T("No image quality level specified"));
				continue;
			}
			if (_stscanf(args[j], _T("%d"), &m_jpegQualityLevel) != 1) {
				ArgError(_T("Invalid image quality level specified"));
				continue;
			}
		} else if ( SwitchMatch(args[j], _T("register") )) {
			Register();
			PostQuitMessage(0);
		} else {
			TCHAR phost[256];
			if (!ParseDisplay(args[j], phost, 255, &m_port)) {
				ShowUsage(_T("Invalid VNC server specified."));
				PostQuitMessage(1);
			} else {
				_tcscpy(m_host, phost);
				m_connectionSpecified = true;
			}
		}
	}       
	
	if (m_scale_num != 1 || m_scale_den != 1) 			
		m_scaling = true;

	// reduce scaling factors by greatest common denominator
	if (m_scaling) {
		FixScaling();
	}
	// tidy up
	delete [] cmd;
	delete [] args;
}

void saveInt(char *name, int value, char *fname) 
{
	char buf[4];
	sprintf(buf, "%d", value); 
	WritePrivateProfileString("options", name, buf, fname);
}

int readInt(char *name, int defval, char *fname)
{
	return GetPrivateProfileInt("options", name, defval, fname);
}

void VNCOptions::Save(char *fname)
{
	for (int i = rfbEncodingRaw; i<= LASTENCODING; i++) {
		char buf[128];
		sprintf(buf, "use_encoding_%d", i);
		saveInt(buf, m_UseEnc[i], fname);
	}
	saveInt("preferred_encoding",	m_PreferredEncoding,fname);
	saveInt("restricted",			m_restricted,		fname);
	saveInt("viewonly",				m_ViewOnly,			fname);
	saveInt("fullscreen",			m_FullScreen,		fname);
	saveInt("8bit",					m_Use8Bit,			fname);
	saveInt("shared",				m_Shared,			fname);
	saveInt("swapmouse",			m_SwapMouse,		fname);
	saveInt("belldeiconify",		m_DeiconifyOnBell,	fname);
	saveInt("emulate3",				m_Emul3Buttons,		fname);
	saveInt("emulate3timeout",		m_Emul3Timeout,		fname);
	saveInt("emulate3fuzz",			m_Emul3Fuzz,		fname);
	saveInt("disableclipboard",		m_DisableClipboard, fname);
	saveInt("localcursor",			m_localCursor,		fname);
	saveInt("scale_den",			m_scale_den,		fname);
	saveInt("scale_num",			m_scale_num,		fname);
	saveInt("cursorshape",			m_requestShapeUpdates, fname);
	saveInt("noremotecursor",		m_ignoreShapeUpdates, fname);
	if (m_useCompressLevel) {
		saveInt("compresslevel",	m_compressLevel,	fname);
	}
	if (m_enableJpegCompression) {
		saveInt("quality",			m_jpegQualityLevel,	fname);
	}
}

void VNCOptions::Load(char *fname)
{
	for (int i = rfbEncodingRaw; i<= LASTENCODING; i++) {
		char buf[128];
		sprintf(buf, "use_encoding_%d", i);
		m_UseEnc[i] =   readInt(buf, m_UseEnc[i], fname) != 0;
	}
	m_PreferredEncoding =	readInt("preferred_encoding", m_PreferredEncoding,	fname);
	m_restricted =			readInt("restricted",		m_restricted,	fname) != 0 ;
	m_ViewOnly =			readInt("viewonly",			m_ViewOnly,		fname) != 0;
	m_FullScreen =			readInt("fullscreen",		m_FullScreen,	fname) != 0;
	m_Use8Bit =				readInt("8bit",				m_Use8Bit,		fname) != 0;
	m_Shared =				readInt("shared",			m_Shared,		fname) != 0;
	m_SwapMouse =			readInt("swapmouse",		m_SwapMouse,	fname) != 0;
	m_DeiconifyOnBell =		readInt("belldeiconify",	m_DeiconifyOnBell, fname) != 0;
	m_Emul3Buttons =		readInt("emulate3",			m_Emul3Buttons, fname) != 0;
	m_Emul3Timeout =		readInt("emulate3timeout",	m_Emul3Timeout, fname);
	m_Emul3Fuzz =			readInt("emulate3fuzz",		m_Emul3Fuzz,    fname);
	m_DisableClipboard =	readInt("disableclipboard", m_DisableClipboard, fname) != 0;
	m_localCursor =			readInt("localcursor",		m_localCursor,	fname);
	m_scale_den =			readInt("scale_den",		m_scale_den,	fname);
	m_scale_num =			readInt("scale_num",		m_scale_num,	fname);
	m_requestShapeUpdates =	readInt("cursorshape",		m_requestShapeUpdates, fname) != 0;
	m_ignoreShapeUpdates =	readInt("noremotecursor",	m_ignoreShapeUpdates, fname) != 0;
	int level =				readInt("compresslevel",	-1,				fname);
	if (level != -1) {
		m_useCompressLevel = true;
		m_compressLevel = level;
	}
	level =					readInt("quality",			-1,				fname);
	m_enableJpegCompression = false;
	if (level != -1) {
		m_enableJpegCompression = true;
		m_jpegQualityLevel = level;
	}
}

// Record the path to the VNC viewer and the type
// of the .vnc files in the registry
void VNCOptions::Register()
{
	char keybuf[_MAX_PATH * 2 + 20];
	HKEY hKey, hKey2;
	if ( RegCreateKey(HKEY_CLASSES_ROOT, ".vnc", &hKey)  == ERROR_SUCCESS ) {
		RegSetValue(hKey, NULL, REG_SZ, "VncViewer.Config", 0);
		RegCloseKey(hKey);
	} else {
		vnclog.Print(0, "Failed to register .vnc extension\n");
	}

	char filename[_MAX_PATH];
	if (GetModuleFileName(NULL, filename, _MAX_PATH) == 0) {
		vnclog.Print(0, "Error getting vncviewer filename\n");
		return;
	}
	vnclog.Print(2, "Viewer is %s\n", filename);

	if ( RegCreateKey(HKEY_CLASSES_ROOT, "VncViewer.Config", &hKey)  == ERROR_SUCCESS ) {
		RegSetValue(hKey, NULL, REG_SZ, "VNCviewer Config File", 0);
		
		if ( RegCreateKey(hKey, "DefaultIcon", &hKey2)  == ERROR_SUCCESS ) {
			sprintf(keybuf, "%s,0", filename);
			RegSetValue(hKey2, NULL, REG_SZ, keybuf, 0);
			RegCloseKey(hKey2);
		}
		if ( RegCreateKey(hKey, "Shell\\open\\command", &hKey2)  == ERROR_SUCCESS ) {
			sprintf(keybuf, "\"%s\" -config \"%%1\"", filename);
			RegSetValue(hKey2, NULL, REG_SZ, keybuf, 0);
			RegCloseKey(hKey2);
		}

		RegCloseKey(hKey);
	}

	if ( RegCreateKey(HKEY_LOCAL_MACHINE, 
			"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\vncviewer.exe", 
			&hKey)  == ERROR_SUCCESS ) {
		RegSetValue(hKey, NULL, REG_SZ, filename, 0);
		RegCloseKey(hKey);
	}
}

void VNCOptions::ShowUsage(LPTSTR info) {
    TCHAR msg[2048];
    TCHAR *tmpinf = _T("");
    if (info != NULL) 
        tmpinf = info;
    _stprintf(msg, 
#ifdef UNDER_CE
		_T("%s\n\rUsage includes:\n\r")
			_T("vncviewer [/8bit] [/swapmouse] [/shared] [/belldeiconify] \n\r")
			_T(" [/hpc | /palm] [/slow] server[:display] \n\r")
			_T("For full details see documentation."),
#else
		_T("%s\n\rUsage includes:\n\r"
			"  vncviewer [/8bit] [/shared] [/noshared] [/swapmouse] \n\r"
			"      [/belldeiconify] [/listen] [/fullscreen] [/viewonly] \n\r"
			"      [/emulate3] [/scale a/b] [/config configfile] \n\r"
			"      [/encoding encname] [/compresslevel n] [/quality n] \n\r"
			"      [/nojpeg] [/nocursorshape] [/noremotecursor] server[:display]\n\r"
			"For full details see documentation."), 
#endif
        tmpinf);
    MessageBox(NULL,  msg, _T("VNC error"), MB_OK | MB_ICONSTOP | MB_TOPMOST);
}

// The dialog box allows you to change the session-specific parameters
int VNCOptions::DoDialog(bool running)
{
	m_running = running;
 	return DialogBoxParam(pApp->m_instance, DIALOG_MAKEINTRESOURCE(IDD_OPTIONDIALOG), 
		NULL, (DLGPROC) OptDlgProc, (LONG) this);
}

BOOL CALLBACK VNCOptions::OptDlgProc(  HWND hwnd,  UINT uMsg,  
									 WPARAM wParam, LPARAM lParam ) {
	// This is a static method, so we don't know which instantiation we're 
	// dealing with. But we can get a pseudo-this from the parameter to 
	// WM_INITDIALOG, which we therafter store with the window and retrieve
	// as follows:
	VNCOptions *_this = (VNCOptions *) GetWindowLong(hwnd, GWL_USERDATA);
	
	switch (uMsg) {
		
	case WM_INITDIALOG:
		{
			SetWindowLong(hwnd, GWL_USERDATA, lParam);
			_this = (VNCOptions *) lParam;
			// Initialise the controls
			for (int i = rfbEncodingRaw; i <= LASTENCODING; i++) {
				HWND hPref = GetDlgItem(hwnd, IDC_RAWRADIO + (i-rfbEncodingRaw));
				SendMessage(hPref, BM_SETCHECK, 
					(i== _this->m_PreferredEncoding), 0);
				EnableWindow(hPref, _this->m_UseEnc[i]);
			}
			
			HWND hCopyRect = GetDlgItem(hwnd, ID_SESSION_SET_CRECT);
			SendMessage(hCopyRect, BM_SETCHECK, _this->m_UseEnc[rfbEncodingCopyRect], 0);
			
			HWND hSwap = GetDlgItem(hwnd, ID_SESSION_SWAPMOUSE);
			SendMessage(hSwap, BM_SETCHECK, _this->m_SwapMouse, 0);
			
			HWND hDeiconify = GetDlgItem(hwnd, IDC_BELLDEICONIFY);
			SendMessage(hDeiconify, BM_SETCHECK, _this->m_DeiconifyOnBell, 0);

#ifndef UNDER_CE
			HWND hDisableClip = GetDlgItem(hwnd, IDC_DISABLECLIPBOARD);
			SendMessage(hDisableClip, BM_SETCHECK, _this->m_DisableClipboard, 0);
#endif			
			
			HWND h8bit = GetDlgItem(hwnd, IDC_8BITCHECK);
			SendMessage(h8bit, BM_SETCHECK, _this->m_Use8Bit, 0);
			
			HWND hShared = GetDlgItem(hwnd, IDC_SHARED);
			SendMessage(hShared, BM_SETCHECK, _this->m_Shared, 0);
			EnableWindow(hShared, !_this->m_running);
			 
			HWND hViewOnly = GetDlgItem(hwnd, IDC_VIEWONLY);
			SendMessage(hViewOnly, BM_SETCHECK, _this->m_ViewOnly, 0);


			HWND hScaling = GetDlgItem(hwnd, IDC_SCALING);
			SendMessage(hScaling, BM_SETCHECK, _this->m_scaling, 0);

			SetDlgItemInt( hwnd, IDC_SCALE_NUM, _this->m_scale_num, FALSE);
			SetDlgItemInt( hwnd, IDC_SCALE_DEN, _this->m_scale_den, FALSE);

#ifndef UNDER_CE
			HWND hFullScreen = GetDlgItem(hwnd, IDC_FULLSCREEN);
			SendMessage(hFullScreen, BM_SETCHECK, _this->m_FullScreen, 0);

 			HWND hEmulate = GetDlgItem(hwnd, IDC_EMULATECHECK);
 			SendMessage(hEmulate, BM_SETCHECK, _this->m_Emul3Buttons, 0);
#endif

			HWND hAllowCompressLevel = GetDlgItem(hwnd, IDC_ALLOW_COMPRESSLEVEL);
			SendMessage(hAllowCompressLevel, BM_SETCHECK, _this->m_useCompressLevel, 0);

			HWND hAllowJpeg = GetDlgItem(hwnd, IDC_ALLOW_JPEG);
			SendMessage(hAllowJpeg, BM_SETCHECK, _this->m_enableJpegCompression, 0);

			SetDlgItemInt( hwnd, IDC_COMPRESSLEVEL, _this->m_compressLevel, FALSE);
			SetDlgItemInt( hwnd, IDC_QUALITYLEVEL, _this->m_jpegQualityLevel, FALSE);

			HWND hRemoteCursor;
			if (_this->m_requestShapeUpdates && !_this->m_ignoreShapeUpdates) {
				hRemoteCursor = GetDlgItem(hwnd, IDC_CSHAPE_ENABLE_RADIO);
			} else if (_this->m_requestShapeUpdates) {
				hRemoteCursor = GetDlgItem(hwnd, IDC_CSHAPE_IGNORE_RADIO);
			} else {
				hRemoteCursor = GetDlgItem(hwnd, IDC_CSHAPE_DISABLE_RADIO);
			}
			SendMessage(hRemoteCursor, BM_SETCHECK,	true, 0);

			CentreWindow(hwnd);
			
			return TRUE;
		}
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			{
				for (int i = rfbEncodingRaw; i <= LASTENCODING; i++) {
					HWND hPref = GetDlgItem(hwnd, IDC_RAWRADIO+i-rfbEncodingRaw);
					if (SendMessage(hPref, BM_GETCHECK, 0, 0) == BST_CHECKED)
						_this->m_PreferredEncoding = i;
				}
				
				HWND hCopyRect = GetDlgItem(hwnd, ID_SESSION_SET_CRECT);
				_this->m_UseEnc[rfbEncodingCopyRect] =
					(SendMessage(hCopyRect, BM_GETCHECK, 0, 0) == BST_CHECKED);
				
				HWND hSwap = GetDlgItem(hwnd, ID_SESSION_SWAPMOUSE);
				_this->m_SwapMouse =
					(SendMessage(hSwap, BM_GETCHECK, 0, 0) == BST_CHECKED);
				
				HWND hDeiconify = GetDlgItem(hwnd, IDC_BELLDEICONIFY);
				_this->m_DeiconifyOnBell =
					(SendMessage(hDeiconify, BM_GETCHECK, 0, 0) == BST_CHECKED);
#ifndef UNDER_CE				
				HWND hDisableClip = GetDlgItem(hwnd, IDC_DISABLECLIPBOARD);
				_this->m_DisableClipboard =
					(SendMessage(hDisableClip, BM_GETCHECK, 0, 0) == BST_CHECKED);
#endif

				HWND h8bit = GetDlgItem(hwnd, IDC_8BITCHECK);
				_this->m_Use8Bit =
					(SendMessage(h8bit, BM_GETCHECK, 0, 0) == BST_CHECKED);
				
				HWND hShared = GetDlgItem(hwnd, IDC_SHARED);
				_this->m_Shared =
					(SendMessage(hShared, BM_GETCHECK, 0, 0) == BST_CHECKED);
				
				HWND hViewOnly = GetDlgItem(hwnd, IDC_VIEWONLY);
				_this->m_ViewOnly = 
					(SendMessage(hViewOnly, BM_GETCHECK, 0, 0) == BST_CHECKED);

				HWND hScaling = GetDlgItem(hwnd, IDC_SCALING);
				_this->m_scaling = 
					(SendMessage(hScaling, BM_GETCHECK, 0, 0) == BST_CHECKED);

				if (_this->m_scaling) {
					_this->m_scale_num = GetDlgItemInt( hwnd, IDC_SCALE_NUM, NULL, TRUE);
					_this->m_scale_den = GetDlgItemInt( hwnd, IDC_SCALE_DEN, NULL, TRUE);
					_this->FixScaling();
					if (_this->m_scale_num == 1 && _this->m_scale_den == 1)
						_this->m_scaling = false;
				} else {
					_this->m_scale_num = 1;
					_this->m_scale_den = 1;
				}

#ifndef UNDER_CE
				HWND hFullScreen = GetDlgItem(hwnd, IDC_FULLSCREEN);
				_this->m_FullScreen = 
					(SendMessage(hFullScreen, BM_GETCHECK, 0, 0) == BST_CHECKED);

 				HWND hEmulate = GetDlgItem(hwnd, IDC_EMULATECHECK);
 				_this->m_Emul3Buttons =
				  (SendMessage(hEmulate, BM_GETCHECK, 0, 0) == BST_CHECKED);
#endif

				HWND hAllowCompressLevel = GetDlgItem(hwnd, IDC_ALLOW_COMPRESSLEVEL);
				_this->m_useCompressLevel = 
					(SendMessage(hAllowCompressLevel, BM_GETCHECK, 0, 0) == BST_CHECKED);

				_this->m_compressLevel = GetDlgItemInt( hwnd, IDC_COMPRESSLEVEL, NULL, TRUE);
				if ( _this->m_compressLevel < 0 ) { _this->m_compressLevel = 0; }
				if ( _this->m_compressLevel > 9 ) { _this->m_compressLevel = 9; }

				HWND hAllowJpeg = GetDlgItem(hwnd, IDC_ALLOW_JPEG);
				_this->m_enableJpegCompression = 
					(SendMessage(hAllowJpeg, BM_GETCHECK, 0, 0) == BST_CHECKED);

				_this->m_jpegQualityLevel = GetDlgItemInt( hwnd, IDC_QUALITYLEVEL, NULL, TRUE);
				if ( _this->m_jpegQualityLevel < 0 ) { _this->m_jpegQualityLevel = 0; }
				if ( _this->m_jpegQualityLevel > 9 ) { _this->m_jpegQualityLevel = 9; }

				_this->m_requestShapeUpdates = false;
				_this->m_ignoreShapeUpdates = false;
				HWND hRemoteCursor = GetDlgItem(hwnd, IDC_CSHAPE_ENABLE_RADIO);
				if (SendMessage(hRemoteCursor, BM_GETCHECK, 0, 0) == BST_CHECKED) {
						_this->m_requestShapeUpdates = true;
				} else {
					hRemoteCursor = GetDlgItem(hwnd, IDC_CSHAPE_IGNORE_RADIO);
					if (SendMessage(hRemoteCursor, BM_GETCHECK, 0, 0) == BST_CHECKED) {
						_this->m_requestShapeUpdates = true;
						_this->m_ignoreShapeUpdates = true;
					}
				}

				EndDialog(hwnd, TRUE);
				
				return TRUE;
			}
		case IDCANCEL:
			EndDialog(hwnd, FALSE);
			return TRUE;
		}
		break;
        case WM_DESTROY:
			EndDialog(hwnd, FALSE);
			return TRUE;
	}
	return 0;
}

