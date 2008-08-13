//  Copyright (C) 2005-2006 Sun Microsystems, Inc. All Rights Reserved.
//  Copyright (C) 2004 Landmark Graphics Corporation. All Rights Reserved.
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
#include "Htmlhelp.h"
#include "commctrl.h"
#include "AboutBox.h"
extern "C" {
#include "vncauth.h"
}

VNCOptions::VNCOptions()
{
	for (int i = rfbEncodingRaw; i<= LASTENCODING; i++)
		m_UseEnc[i] = false;
	
	m_UseEnc[rfbEncodingRaw] = true;
	m_UseEnc[rfbEncodingHextile] = true;
	m_UseEnc[rfbEncodingTight] = true;
	m_UseEnc[rfbEncodingCopyRect] = true;

	m_ViewOnly = false;
	m_FullScreen = false;
	m_toolbar = true;
	m_historyLimit = 32;
	m_skipprompt = false;
	m_Use8Bit = false;
	m_DoubleBuffer = true;
	m_PreferredEncoding = rfbEncodingTight;
	m_SwapMouse = false;
	m_Emul3Buttons = true;
	m_Emul3Timeout = 100; // milliseconds
	m_Emul3Fuzz = 4;      // pixels away before emulation is cancelled
	m_Shared = true;
	m_DeiconifyOnBell = false;
	m_DisableClipboard = false;
	m_localCursor = DOTCURSOR;
	m_FitWindow = false;
	m_scaling = false;
	m_scale_num = 100;
	m_scale_den = 100;
	
	m_display[0] = '\0';
	m_host[0] = '\0';
	m_port = -1;
	m_via_host[0] = '\0';
	m_hWindow = 0;
	m_kbdname[0] = '\0';
	m_kbdSpecified = false;
	
	m_logLevel = 0;
	m_logToConsole = false;
	m_logToFile = false;
	strcpy(m_logFilename ,"vncviewer.log");
	
	m_delay=0;
	m_connectionSpecified = false;
	m_configSpecified = false;
	m_configFilename[0] = '\0';
	m_listening = false;
	m_listenPort = INCOMING_PORT_OFFSET;
	m_restricted = false;

	m_subsampLevel = TVNC_1X;
	m_compressLevel = 1;
	m_enableJpegCompression = true;
	m_jpegQualityLevel = 95;
	m_requestShapeUpdates = true;
	m_ignoreShapeUpdates = false;

	m_encPasswd[0] = '\0';

	LoadGenOpt();

	
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
	m_DoubleBuffer      = s.m_DoubleBuffer;
	m_PreferredEncoding = s.m_PreferredEncoding;
	m_SwapMouse			= s.m_SwapMouse;
	m_Emul3Buttons		= s.m_Emul3Buttons;
	m_Emul3Timeout		= s.m_Emul3Timeout;
	m_Emul3Fuzz			= s.m_Emul3Fuzz;      // pixels away before emulation is cancelled
	m_Shared			= s.m_Shared;
	m_DeiconifyOnBell	= s.m_DeiconifyOnBell;
	m_DisableClipboard  = s.m_DisableClipboard;
	m_scaling			= s.m_scaling;
	m_FitWindow			= s.m_FitWindow;
	m_scale_num			= s.m_scale_num;
	m_scale_den			= s.m_scale_den;
	m_localCursor		= s.m_localCursor;
	m_toolbar			= s.m_toolbar;
	strcpy(m_display, s.m_display);
	strcpy(m_host, s.m_host);
	m_port				= s.m_port;
	m_hWindow			= s.m_hWindow;

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

	m_subsampLevel			= s.m_subsampLevel;
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
	if (a < b) return gcd(b, a);
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
		
	TCHAR CommLine[256] ;
	int f = 0;
	_tcscpy(CommLine, szCmdLine);

	if (_tcsstr( CommLine, "/listen") != NULL ||
        _tcsstr( CommLine, "-listen") != NULL) {
		LoadOpt(".listen", KEY_VNCVIEWER_HISTORI);
		f = 1;
	}
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
	int j;
	for (j = 0; j < i; j++) {
		TCHAR phost[256];
		if (ParseDisplay(args[j], phost, 255, &m_port) && (f == 0) &&
			(_tcsstr( args[j], "/") == NULL))
			LoadOpt(args[j], KEY_VNCVIEWER_HISTORI);
	}
	bool hostGiven = false, portGiven = false;
	// take in order.
	for (j = 0; j < i; j++) {
		if ( SwitchMatch(args[j], _T("help")) ||
			SwitchMatch(args[j], _T("?")) ||
			SwitchMatch(args[j], _T("h"))) {
			ShowHelpBox(_T("TurboVNC Usage Help"));
			exit(1);
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
		} else if ( SwitchMatch(args[j], _T("notoolbar"))) {
			m_toolbar = false;
		} else if ( SwitchMatch(args[j], _T("8bit"))) {
			m_Use8Bit = true;
		} else if ( SwitchMatch(args[j], _T("no8bit"))) {
			m_Use8Bit = false;
		} else if ( SwitchMatch(args[j], _T("singlebuffer"))) {
			m_DoubleBuffer = false;
		} else if ( SwitchMatch(args[j], _T("doublebuffer"))) {
			m_DoubleBuffer = true;
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
		} else if ( SwitchMatch(args[j], _T("smalldotcursor"))) {
			m_localCursor = SMALLCURSOR;
		} else if ( SwitchMatch(args[j], _T("normalcursor"))) {			
			m_localCursor= NORMALCURSOR;
		} else if ( SwitchMatch(args[j], _T("belldeiconify") )) {
			m_DeiconifyOnBell = true;
		} else if ( SwitchMatch(args[j], _T("emulate3") )) {
			m_Emul3Buttons = true;
		} else if ( SwitchMatch(args[j], _T("noemulate3") )) {
			m_Emul3Buttons = false;
		} else if ( SwitchMatch(args[j], _T("jpeg") )) {
			m_enableJpegCompression = true;
		} else if ( SwitchMatch(args[j], _T("nojpeg") )) {
			m_enableJpegCompression = false;
		} else if ( SwitchMatch(args[j], _T("nocursorshape") )) {
			m_requestShapeUpdates = false;
		} else if ( SwitchMatch(args[j], _T("noremotecursor") )) {
			m_requestShapeUpdates = true;
			m_ignoreShapeUpdates = true;
		} else if ( SwitchMatch(args[j], _T("fitwindow") )) {
			m_FitWindow = true;
		} else if ( SwitchMatch(args[j], _T("scale") )) {
			if (++j == i) {
				ArgError(_T("No scaling factor specified"));
				continue;
			}
			int numscales = _stscanf(args[j], _T("%d"), &m_scale_num);
			if (numscales < 1) {
				ArgError(_T("Invalid scaling specified"));
				continue;
			}
				m_scale_den = 100;					
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
				continue;
			} else {
				Load(m_configFilename);
				m_configSpecified = true;
			}
		} else if ( SwitchMatch(args[j], _T("via") )) {
			if (++j == i) {
				ArgError(_T("Host name not specified with -via option"));
				continue;
			}
			if (_tcslen(args[j]) >= sizeof(m_via_host)/sizeof(TCHAR)) {
				ArgError(_T("Host name at -via option is too long"));
				continue;
			}
			_tcscpy(m_via_host, args[j]);
		} else if ( SwitchMatch(args[j], _T("copyrect") )) {
			m_UseEnc[rfbEncodingCopyRect] = true;
		} else if ( SwitchMatch(args[j], _T("nocopyrect") )) {
			m_UseEnc[rfbEncodingCopyRect] = false;
		} else if ( SwitchMatch(args[j], _T("encoding") )) {
			if (++j == i) {
				ArgError(_T("No encoding specified"));
				continue;
			}
			int enc = -1;
			if (_tcsicmp(args[j], _T("raw")) == 0) {
				enc = rfbEncodingRaw;
			} else if (_tcsicmp(args[j], _T("hextile")) == 0) {
				enc = rfbEncodingHextile;
			} else if (_tcsicmp(args[j], _T("tight")) == 0) {
				enc = rfbEncodingTight;
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
			int level = -1;
			if (_stscanf(args[j], _T("%d"), &level) != 1
				|| level < 0 || level > 9) {
				ArgError(_T("Invalid compression level specified"));
				continue;
			}
			m_compressLevel = level;
		} else if ( SwitchMatch(args[j], _T("samp") )) {
			if (++j == i) {
				ArgError(_T("No subsampling specified"));
				continue;
			}
			int subsamp=-1;
			switch(_totupper(args[j][0])) {
				case '0': case 'G':  subsamp = TVNC_GRAY;  break;
				case '1':  subsamp = TVNC_1X;  break;
				case '2':  subsamp = TVNC_2X;  break;
				case '4':  subsamp = TVNC_4X;  break;
			}
			if(subsamp==-1) {
				ArgError(_T("Invalid subsampling specified"));
				continue;
			}
			m_subsampLevel = subsamp;
		} else if ( SwitchMatch(args[j], _T("quality") )) {
			if (++j == i) {
				ArgError(_T("No image quality level specified"));
				continue;
			}
			int quality=-1;
			if (_stscanf(args[j], _T("%d"), &quality) != 1
				|| quality<1 || quality>100) {
				ArgError(_T("Invalid image quality level specified"));
				continue;
			}
			m_jpegQualityLevel = quality;
		} else if ( SwitchMatch(args[j], _T("password") )) {
			if (++j == i) {
				ArgError(_T("No password specified"));
				continue;
			}
			char passwd[MAXPWLEN + 1];
			strncpy(passwd, args[j], MAXPWLEN);
			passwd[MAXPWLEN]='\0';
			if(strlen(passwd)>8) passwd[8]='\0';
			vncEncryptPasswd(m_encPasswd, passwd);
			memset(passwd, 0, MAXPWLEN);
		} else if ( SwitchMatch(args[j], _T("register") )) {
			Register();
			exit(1);
		} else {
			TCHAR phost[256];
			if (!ParseDisplay(args[j], phost, 255, &m_port)) {
				ArgError(_T("Invalid VNC server specified."));
				continue;
			} else {
				_tcscpy(m_host, phost);
				_tcscpy(m_display, args[j]);
				m_connectionSpecified = true;
			}
		}
	}       
	// reduce scaling factors by greatest common denominator
	FixScaling();

	m_scaling = (m_scale_num != 1 || m_scale_den != 1 || m_FitWindow);			

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
	saveInt("doublebuffer",			m_DoubleBuffer,		fname);
	saveInt("shared",				m_Shared,			fname);
	saveInt("swapmouse",			m_SwapMouse,		fname);
	saveInt("belldeiconify",		m_DeiconifyOnBell,	fname);
	saveInt("emulate3",				m_Emul3Buttons,		fname);
	saveInt("emulate3timeout",		m_Emul3Timeout,		fname);
	saveInt("emulate3fuzz",			m_Emul3Fuzz,		fname);
	saveInt("disableclipboard",		m_DisableClipboard, fname);
	saveInt("localcursor",			m_localCursor,		fname);
	saveInt("fitwindow",			m_FitWindow,		fname);
	saveInt("scale_den",			m_scale_den,		fname);
	saveInt("scale_num",			m_scale_num,		fname);
	saveInt("cursorshape",			m_requestShapeUpdates, fname);
	saveInt("noremotecursor",		m_ignoreShapeUpdates, fname);	
	saveInt("compresslevel", m_compressLevel,	fname);	
	saveInt("subsampling", m_subsampLevel,	fname);	
	saveInt("quality", m_enableJpegCompression?	m_jpegQualityLevel : -1,	fname);

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
	m_DoubleBuffer =		readInt("doublebuffer",		m_DoubleBuffer,	fname) != 0;
	m_Shared =				readInt("shared",			m_Shared,		fname) != 0;
	m_SwapMouse =			readInt("swapmouse",		m_SwapMouse,	fname) != 0;
	m_DeiconifyOnBell =		readInt("belldeiconify",	m_DeiconifyOnBell, fname) != 0;
	m_Emul3Buttons =		readInt("emulate3",			m_Emul3Buttons, fname) != 0;
	m_Emul3Timeout =		readInt("emulate3timeout",	m_Emul3Timeout, fname);
	m_Emul3Fuzz =			readInt("emulate3fuzz",		m_Emul3Fuzz,    fname);
	m_DisableClipboard =	readInt("disableclipboard", m_DisableClipboard, fname) != 0;
	m_localCursor =			readInt("localcursor",		m_localCursor,	fname);
	m_FitWindow =			readInt("fitwindow",		m_FitWindow,	fname) != 0;
	m_scale_den =			readInt("scale_den",		m_scale_den,	fname);
	m_scale_num =			readInt("scale_num",		m_scale_num,	fname);
	m_requestShapeUpdates =	readInt("cursorshape",		m_requestShapeUpdates, fname) != 0;
	m_ignoreShapeUpdates =	readInt("noremotecursor",	m_ignoreShapeUpdates, fname) != 0;
	int level =				readInt("compresslevel",	-1,				fname);
	if (level != -1) {
		m_compressLevel = level;
	}
	level =				readInt("subsampling",	-1,				fname);
	if (level != -1) {
		m_subsampLevel = level;
	}
	level =					readInt("quality",			-1,				fname);
	m_enableJpegCompression = true;
	if (level == -1) {
		m_enableJpegCompression = false;
	} else {
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

// The dialog box allows you to change the session-specific parameters
int VNCOptions::DoDialog(bool running)
{
	m_running = running;
	return DialogBoxParam(pApp->m_instance, DIALOG_MAKEINTRESOURCE(IDD_PARENT), 
							NULL, (DLGPROC) DlgProc, (LONG) this); 	
}

BOOL CALLBACK VNCOptions::DlgProc(HWND hwndDlg, UINT uMsg,
										WPARAM wParam, LPARAM lParam)
{
	// We use the dialog-box's USERDATA to store a _this pointer
	// This is set only once WM_INITDIALOG has been recieved, though!
	VNCOptions *_this = (VNCOptions *) GetWindowLong(hwndDlg, GWL_USERDATA);

	switch (uMsg) {
	case WM_INITDIALOG: 
		{
			// Retrieve the Dialog box parameter and use it as a pointer
			// to the calling VNCOptions object
			SetWindowLong(hwndDlg, GWL_USERDATA, lParam);
			VNCOptions *_this = (VNCOptions *) lParam;
			InitCommonControls();
			CentreWindow(hwndDlg);

			_this->m_hParent = hwndDlg;
			_this->m_hTab = GetDlgItem(hwndDlg, IDC_TAB);

			TCITEM item;
			item.mask = TCIF_TEXT; 
			item.pszText="Connection";
			TabCtrl_InsertItem(_this->m_hTab, 0, &item);
			item.pszText = "Globals";
			TabCtrl_InsertItem(_this->m_hTab, 1, &item);

			_this->m_hPageConnection = CreateDialogParam(pApp->m_instance,
				MAKEINTRESOURCE(IDD_OPTIONDIALOG),
				hwndDlg,
				(DLGPROC)_this->DlgProcConnOptions,
				(LONG)_this);

			_this->m_hPageGeneral = CreateDialogParam(pApp->m_instance, 
				MAKEINTRESOURCE(IDD_GENERAL_OPTION),
				hwndDlg,
				(DLGPROC)_this->DlgProcGlobalOptions,
				(LONG)_this);

			// Position child dialogs, to fit the Tab control's display area
			RECT rc;
			GetWindowRect(_this->m_hTab, &rc);
			MapWindowPoints(NULL, hwndDlg, (POINT *)&rc, 2);
			TabCtrl_AdjustRect(_this->m_hTab, FALSE, &rc);
			SetWindowPos(_this->m_hPageConnection, HWND_TOP, rc.left, rc.top,
						 rc.right - rc.left, rc.bottom - rc.top,
						 SWP_SHOWWINDOW);
			SetWindowPos(_this->m_hPageGeneral, HWND_TOP, rc.left, rc.top,
						 rc.right - rc.left, rc.bottom - rc.top,
						 SWP_HIDEWINDOW);

			return TRUE;
		}

	case WM_HELP:	
		help.Popup(lParam);
		return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam))	{
		case IDOK:
			SetFocus(GetDlgItem(hwndDlg, IDOK));
			SendMessage(_this->m_hPageConnection, WM_COMMAND, IDC_OK, 0);
			SendMessage(_this->m_hPageGeneral, WM_COMMAND, IDC_OK, 0);
			EndDialog(hwndDlg, TRUE);
			return TRUE;
		case IDCANCEL:			
			EndDialog(hwndDlg, FALSE);
			return TRUE;
		}
		return FALSE;

	case WM_NOTIFY:
		{
			LPNMHDR pn = (LPNMHDR)lParam;
			switch (pn->code) {		
			case TCN_SELCHANGE:
				switch (pn->idFrom) {
				case IDC_TAB:
					int i = TabCtrl_GetCurFocus(_this->m_hTab);
					switch (i) {
					case 0:
						ShowWindow(_this->m_hPageConnection, SW_SHOW);
						SetFocus(_this->m_hPageConnection);
						return 0;
					case 1:
						ShowWindow(_this->m_hPageGeneral, SW_SHOW);
						SetFocus(_this->m_hPageGeneral);
						return 0;						
					}
					return 0;
				}
				return 0;
			case TCN_SELCHANGING:
				switch (pn->idFrom) {
				case IDC_TAB:
					int i = TabCtrl_GetCurFocus(_this->m_hTab);
					switch (i) {
					case 0:
						ShowWindow(_this->m_hPageConnection, SW_HIDE);
						break;
					case 1:
						ShowWindow(_this->m_hPageGeneral, SW_HIDE);
						break;
					}
					return 0;
				}
				return 0;
			}
			return 0;
		}
			
	}
    return 0;
}

static COMBOSTRING rfbcombo[MAX_LEN_COMBO] = {
	"Tight + Perceptually Lossless JPEG (LAN)", rfbEncodingTight, true, TVNC_1X, 95, -1,
	"Tight + Medium Quality JPEG", rfbEncodingTight, true, TVNC_2X, 80, -1,
	"Tight + Low Quality JPEG (WAN)", rfbEncodingTight, true, TVNC_4X, 30, -1,
	"Lossless Tight (Gigabit)", rfbEncodingTight, false, -1, -1, 0,
	"Lossless Tight + Zlib (WAN)", rfbEncodingTight, false, -1, -1, 1
};

static const char *sampopt2str[TVNC_SAMPOPT] = {
	"None", "4x", "2x", "Gray"
};

static const int sliderpos2sampopt[TVNC_SAMPOPT] = {
	TVNC_GRAY, TVNC_4X, TVNC_2X, TVNC_1X
};

static const int sampopt2sliderpos[TVNC_SAMPOPT] = {
	3, 1, 2, 0
};

static const TCHAR *encodingString[17] = {
	"Raw", "CopyRect", "RRE", "", "CoRRE", "Hextile", "Zlib", "Tight", "ZlibHex",
	"", "", "", "", "", "", "", "ZRLE"
};

static int slidermax = 1;

BOOL CALLBACK VNCOptions::DlgProcConnOptions(HWND hwnd, UINT uMsg,
											 WPARAM wParam, LPARAM lParam) {
	// This is a static method, so we don't know which instantiation we're 
	// dealing with. But we can get a pseudo-this from the parameter to 
	// WM_INITDIALOG, which we therafter store with the window and retrieve
	// as follows:
	VNCOptions *_this = (VNCOptions *) GetWindowLong(hwnd, GWL_USERDATA);
	
	switch (uMsg) {
		
	case WM_INITDIALOG: 
		{
			SetWindowLong(hwnd, GWL_USERDATA, lParam);
			VNCOptions *_this = (VNCOptions *) lParam;

			// Initialise the controls
			
			HWND hListBox = GetDlgItem(hwnd, IDC_ENCODING);
			int i;
			for (i = 0; i <= (MAX_LEN_COMBO - 1); i++) {			
				SendMessage(hListBox, CB_INSERTSTRING, 
							(WPARAM)i, 
							(LPARAM)(int FAR*)rfbcombo[i].NameString);
			}
			if (_this->m_PreferredEncoding != rfbEncodingTight) {
				SendMessage(hListBox, CB_INSERTSTRING, MAX_LEN_COMBO,
					(LPARAM)encodingString[_this->m_PreferredEncoding]);
				EnableWindow(hListBox, 0);
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
			
			HWND hDB = GetDlgItem(hwnd, IDC_DBCHECK);
			SendMessage(hDB, BM_SETCHECK, _this->m_DoubleBuffer, 0);
			
			HWND hShared = GetDlgItem(hwnd, IDC_SHARED);
			SendMessage(hShared, BM_SETCHECK, _this->m_Shared, 0);
			EnableWindow(hShared, !_this->m_running);
			
			HWND hViewOnly = GetDlgItem(hwnd, IDC_VIEWONLY);
			SendMessage(hViewOnly, BM_SETCHECK, _this->m_ViewOnly, 0);
			char scalecombo[8][20] = {
				"25","50","75","90","100","125","150","Auto"
			};
			HWND hScalEdit = GetDlgItem(hwnd, IDC_SCALE_EDIT);
			for (i = 0; i <= 7; i++) {
				SendMessage(hScalEdit, CB_INSERTSTRING, (WPARAM)i,
							(LPARAM)(int FAR*)scalecombo[i]);
			}
			if (_this->m_FitWindow) {
				SetDlgItemText(hwnd, IDC_SCALE_EDIT, "Auto");
			} else {	
				SetDlgItemInt(hwnd, IDC_SCALE_EDIT, (( _this->m_scale_num*100) / _this->m_scale_den), FALSE);
			}
			
#ifndef UNDER_CE
			HWND hFullScreen = GetDlgItem(hwnd, IDC_FULLSCREEN);
			SendMessage(hFullScreen, BM_SETCHECK, _this->m_FullScreen, 0);
			
			HWND hEmulate = GetDlgItem(hwnd, IDC_EMULATECHECK);
			SendMessage(hEmulate, BM_SETCHECK, _this->m_Emul3Buttons, 0);
#endif
			
			EnableWindow(hSwap,!_this->m_ViewOnly);
			EnableWindow(hEmulate,!_this->m_ViewOnly);

			HWND hAllowJpeg = GetDlgItem(hwnd, IDC_ALLOW_JPEG);
			SendMessage(hAllowJpeg, BM_SETCHECK, _this->m_enableJpegCompression, 0);
			
			EnableWindow(hAllowJpeg, _this->m_PreferredEncoding == rfbEncodingTight);
			
			if (_this->m_compressLevel > 1) slidermax = 9;
			HWND hCompressLevel = GetDlgItem(hwnd, IDC_COMPRESSLEVEL);
			SendMessage(hCompressLevel, TBM_SETRANGE, TRUE,
				(LPARAM) MAKELONG(0, slidermax));
			_this->SetCompressSlider(hwnd, _this->m_compressLevel);

			_this->EnableCompress(hwnd,
				(_this->m_PreferredEncoding == rfbEncodingTight &&
					!_this->m_enableJpegCompression) );

			HWND hSubsampLevel = GetDlgItem(hwnd, IDC_SUBSAMPLEVEL);
			SendMessage(hSubsampLevel, TBM_SETRANGE, TRUE, (LPARAM) MAKELONG(0, 3)); 
			_this->SetSubsampSlider(hwnd, _this->m_subsampLevel);

			_this->EnableSubsamp(hwnd,
				(_this->m_PreferredEncoding == rfbEncodingTight &&
					_this->m_enableJpegCompression) );

			HWND hJpeg = GetDlgItem(hwnd, IDC_QUALITYLEVEL);
			SendMessage(hJpeg, TBM_SETRANGE, TRUE, (LPARAM) MAKELONG(1, 100));
			_this->SetQualitySlider(hwnd, _this->m_jpegQualityLevel);
			
			_this->EnableJpeg(hwnd, _this->m_PreferredEncoding == rfbEncodingTight
				&& _this->m_enableJpegCompression);
			
			HWND hRemoteCursor;
			if (_this->m_requestShapeUpdates && !_this->m_ignoreShapeUpdates) {
				hRemoteCursor = GetDlgItem(hwnd, IDC_CSHAPE_ENABLE_RADIO);
			} else if (_this->m_requestShapeUpdates) {
				hRemoteCursor = GetDlgItem(hwnd, IDC_CSHAPE_IGNORE_RADIO);
			} else {
				hRemoteCursor = GetDlgItem(hwnd, IDC_CSHAPE_DISABLE_RADIO);
			}
			SendMessage(hRemoteCursor, BM_SETCHECK,	true, 0);			

			if (_this->m_PreferredEncoding != rfbEncodingTight)
				SendMessage(hListBox, CB_SETCURSEL, MAX_LEN_COMBO, 0);
			else
				_this->SetComboBox(hwnd);

			return TRUE;
		}

	case WM_COMMAND: 
		switch (LOWORD(wParam)) {
		case IDC_SCALE_EDIT:
			switch (HIWORD(wParam)) {
			case CBN_KILLFOCUS:
				Lim(hwnd, IDC_SCALE_EDIT, 1, 150);
				return 0;
			}
			return 0;
		
		case IDC_DBCHECK:
			switch (HIWORD(wParam)) {
			case BN_CLICKED:
				HWND hDB = GetDlgItem(hwnd, IDC_DBCHECK);
				if (SendMessage(hDB, BM_GETCHECK, 0, 0) != 0) {
					SendMessage(hDB, BM_SETCHECK, FALSE, 0);
				} else {
					SendMessage(hDB, BM_SETCHECK, TRUE, 0);
				}
				_this->SetComboBox(hwnd);
				return 0;
			}
			return 0;
		case ID_SESSION_SET_CRECT:
			switch (HIWORD(wParam)) {
			case BN_CLICKED:
				HWND hCR = GetDlgItem(hwnd, ID_SESSION_SET_CRECT);
				if (SendMessage(hCR, BM_GETCHECK, 0, 0) != 0) {
					SendMessage(hCR, BM_SETCHECK, FALSE, 0);
				} else {
					SendMessage(hCR, BM_SETCHECK, TRUE, 0);
				}
				_this->SetComboBox(hwnd);
				return 0;
			}
			return 0;
		case IDC_ALLOW_JPEG:
			switch (HIWORD(wParam)) {
			case BN_CLICKED:
				HWND hAllowJpeg = GetDlgItem(hwnd, IDC_ALLOW_JPEG);				
				if (SendMessage(hAllowJpeg, BM_GETCHECK, 0, 0) == 0) {
					_this->EnableJpeg(hwnd, TRUE);
					_this->EnableSubsamp(hwnd, TRUE);
					_this->EnableCompress(hwnd, FALSE);
					SendMessage(hAllowJpeg, BM_SETCHECK, TRUE, 0);
				} else {
					_this->EnableJpeg(hwnd, FALSE);
					_this->EnableSubsamp(hwnd, FALSE);
					_this->EnableCompress(hwnd, TRUE);
					SendMessage(hAllowJpeg, BM_SETCHECK, FALSE, 0);
				}
				_this->SetComboBox(hwnd);
				return 0;
			}
			return 0;
		case IDC_VIEWONLY:
		switch (HIWORD(wParam)) {
			case BN_CLICKED:
				HWND hViewOnly = GetDlgItem(hwnd, IDC_VIEWONLY);
				HWND hSwap = GetDlgItem(hwnd, ID_SESSION_SWAPMOUSE);
				HWND hEmulate = GetDlgItem(hwnd, IDC_EMULATECHECK);
				if (SendMessage(hViewOnly, BM_GETCHECK, 0, 0) == 0) {
					EnableWindow( hSwap, FALSE);
					EnableWindow( hEmulate, FALSE);
					SendMessage(hViewOnly, BM_SETCHECK, TRUE, 0);
				} else {
					EnableWindow( hSwap, TRUE);
					EnableWindow( hEmulate, TRUE);
					SendMessage(hViewOnly, BM_SETCHECK, FALSE, 0);
				}
				return 0;
			}
			return 0;
		case IDC_OK: 
			{		 
				HWND hListBox = GetDlgItem(hwnd, IDC_ENCODING);
				int i = SendMessage(hListBox, CB_GETCURSEL, 0, 0);
				if (i < MAX_LEN_COMBO)
					_this->m_PreferredEncoding = rfbcombo[i].rfbEncoding;					
				HWND hScalEdit = GetDlgItem(hwnd, IDC_SCALE_EDIT);
				i = GetDlgItemInt(hwnd, IDC_SCALE_EDIT, NULL, FALSE);
				if (i > 0) {
					_this->m_scale_num = i;
					_this->m_scale_den = 100;
					_this->FixScaling();
					_this->m_FitWindow = false;
					_this->m_scaling = (_this->m_scale_num > 1) || (_this->m_scale_den > 1);
				} else {
					TCHAR buf[20];
					GetDlgItemText(hwnd, IDC_SCALE_EDIT, buf, 20);
					if (strcmp(buf, "Auto") == 0) {
						_this->m_FitWindow = true;
						_this->m_scaling = true;
					} else {
						_this->m_FitWindow = false;
					}
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
				
				HWND hDB = GetDlgItem(hwnd, IDC_DBCHECK);
				_this->m_DoubleBuffer =
					(SendMessage(hDB, BM_GETCHECK, 0, 0) == BST_CHECKED);
				
				HWND hShared = GetDlgItem(hwnd, IDC_SHARED);
				_this->m_Shared =
					(SendMessage(hShared, BM_GETCHECK, 0, 0) == BST_CHECKED);
				
				HWND hViewOnly = GetDlgItem(hwnd, IDC_VIEWONLY);
				_this->m_ViewOnly = 
					(SendMessage(hViewOnly, BM_GETCHECK, 0, 0) == BST_CHECKED);				
#ifndef UNDER_CE
				HWND hFullScreen = GetDlgItem(hwnd, IDC_FULLSCREEN);
				_this->m_FullScreen = 
					(SendMessage(hFullScreen, BM_GETCHECK, 0, 0) == BST_CHECKED);
				
				HWND hEmulate = GetDlgItem(hwnd, IDC_EMULATECHECK);
				_this->m_Emul3Buttons =
					(SendMessage(hEmulate, BM_GETCHECK, 0, 0) == BST_CHECKED);
#endif
				
				HWND hCompressLevel = GetDlgItem(hwnd, IDC_COMPRESSLEVEL);
				_this->m_compressLevel = SendMessage(hCompressLevel, TBM_GETPOS, 0, 0);
				
				
				HWND hSubsampLevel = GetDlgItem(hwnd, IDC_SUBSAMPLEVEL);
				_this->m_subsampLevel = sliderpos2sampopt[SendMessage(hSubsampLevel, TBM_GETPOS, 0, 0)];
				
				
				HWND hAllowJpeg = GetDlgItem(hwnd, IDC_ALLOW_JPEG);
				_this->m_enableJpegCompression = 
					(SendMessage(hAllowJpeg, BM_GETCHECK, 0, 0) == BST_CHECKED);
				HWND hJpeg = GetDlgItem(hwnd, IDC_QUALITYLEVEL);
				_this->m_jpegQualityLevel = SendMessage(hJpeg,TBM_GETPOS , 0, 0);
				
				
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
				return 0;
			}
		case IDC_ENCODING:
			switch (HIWORD(wParam)) {
			case CBN_SELCHANGE:
				{			
					HWND hCompress = GetDlgItem(hwnd, IDC_COMPRESSLEVEL);					
					HWND hJpegSubsamp = GetDlgItem(hwnd, IDC_SUBSAMPLEVEL);					
					HWND hJpegQuality = GetDlgItem(hwnd, IDC_QUALITYLEVEL);					
					HWND hAllowJpeg = GetDlgItem(hwnd, IDC_ALLOW_JPEG);					
					HWND hCopyRect = GetDlgItem(hwnd, ID_SESSION_SET_CRECT);
					HWND hListBox = GetDlgItem(hwnd, IDC_ENCODING);
					int i = SendMessage(hListBox ,CB_GETCURSEL, 0, 0);
					if (i != MAX_LEN_COMBO) {
						int count = SendMessage(hListBox, CB_GETCOUNT, 0, 0);
						while (count > MAX_LEN_COMBO)
							count = SendMessage(hListBox, CB_DELETESTRING, MAX_LEN_COMBO, 0);
					}
					else break;
					if (rfbcombo[i].enableJpeg) {
						_this->EnableJpeg(hwnd, 1);
						_this->EnableSubsamp(hwnd, 1);
						_this->EnableCompress(hwnd, 0);
						SendMessage(hAllowJpeg, BM_SETCHECK, 1, 0);
						_this->SetSubsampSlider(hwnd, rfbcombo[i].subsampLevel);
						_this->SetQualitySlider(hwnd, rfbcombo[i].qualityLevel);
					} else {
						_this->EnableJpeg(hwnd, 0);
						_this->EnableSubsamp(hwnd, 0);
						_this->EnableCompress(hwnd, 1);
						SendMessage(hAllowJpeg, BM_SETCHECK, 0, 0);
						_this->SetCompressSlider(hwnd, rfbcombo[i].compressLevel);
					}
					SendMessage(hCopyRect, BM_SETCHECK, 1, 0);
					return 0;
				}
			}
			return 0;
		}		
		return 0;	
	case WM_HSCROLL: 
		{			
			DWORD dwPos ;    // current position of slider 
			
			HWND hCompressLevel = GetDlgItem(hwnd, IDC_COMPRESSLEVEL);
			HWND hSubsampLevel = GetDlgItem(hwnd, IDC_SUBSAMPLEVEL);
			HWND hJpeg = GetDlgItem(hwnd, IDC_QUALITYLEVEL);
			if (HWND(lParam) == hCompressLevel) {
				dwPos = SendMessage(hCompressLevel, TBM_GETPOS, 0, 0);
				if (dwPos == 0)
					SetDlgItemText(hwnd, IDC_STATIC_LEVEL, "None");
				else
					SetDlgItemInt(hwnd, IDC_STATIC_LEVEL, dwPos, FALSE);
			}
			if (HWND(lParam) == hSubsampLevel) {
				dwPos = SendMessage(hSubsampLevel, TBM_GETPOS, 0, 0);
				SetDlgItemText(hwnd, IDC_STATIC_SLEVEL,
					sampopt2str[sliderpos2sampopt[dwPos]]);
			}
			if (HWND(lParam) == hJpeg) {
				dwPos = SendMessage(hJpeg, TBM_GETPOS, 0, 0);
				SetDlgItemInt(hwnd, IDC_STATIC_QUALITY, dwPos, FALSE);
			}
			_this->SetComboBox(hwnd);
			return 0;
		}
	case WM_HELP:	
		help.Popup(lParam);
		return 0;
	}
	return 0;
}

void VNCOptions::EnableCompress(HWND hwnd, bool enable)
{
	HWND hfast = GetDlgItem(hwnd, IDC_STATIC_FAST);
	HWND hlevel = GetDlgItem(hwnd, IDC_STATIC_LEVEL);
	HWND htextlevel = GetDlgItem(hwnd, IDC_STATIC_COMPRESS);
	HWND hbest = GetDlgItem(hwnd, IDC_STATIC_BEST);
	HWND hCompressLevel = GetDlgItem(hwnd, IDC_COMPRESSLEVEL);
	EnableWindow(hCompressLevel,enable);
	EnableWindow(hfast, enable);
	EnableWindow(hlevel,enable);
	EnableWindow(htextlevel, enable);
	EnableWindow(hbest, enable);
}

void VNCOptions::EnableJpeg(HWND hwnd, bool enable)
{
	HWND hpoor = GetDlgItem(hwnd, IDC_STATIC_POOR);
	HWND hqbest = GetDlgItem(hwnd, IDC_STATIC_QBEST);
	HWND hqualitytext = GetDlgItem(hwnd, IDC_STATIC_QUAL);
	HWND hquality = GetDlgItem(hwnd, IDC_STATIC_QUALITY);
	HWND hJpeg = GetDlgItem(hwnd, IDC_QUALITYLEVEL);
	EnableWindow( hJpeg, enable);
	EnableWindow(hpoor, enable);
	EnableWindow(hqbest, enable);
	EnableWindow(hqualitytext, enable);
	EnableWindow(hquality, enable);
}

void VNCOptions::EnableSubsamp(HWND hwnd, bool enable)
{
	HWND hfast = GetDlgItem(hwnd, IDC_STATIC_SFAST);
	HWND hbest = GetDlgItem(hwnd, IDC_STATIC_SBEST);
	HWND htextlevel = GetDlgItem(hwnd, IDC_STATIC_SLEVEL);
	HWND hsubsamp = GetDlgItem(hwnd, IDC_STATIC_SUBSAMP);
	HWND hSubsampLevel = GetDlgItem(hwnd, IDC_SUBSAMPLEVEL);
	EnableWindow(hSubsampLevel, enable);
	EnableWindow(hfast, enable);
	EnableWindow(hbest, enable);
	EnableWindow(htextlevel, enable);
	EnableWindow(hsubsamp, enable);
}

void VNCOptions::SetSubsampSlider(HWND hwnd, int subsamp)
{
	HWND hJpegSubsamp = GetDlgItem(hwnd, IDC_SUBSAMPLEVEL);
	SendMessage(hJpegSubsamp, TBM_SETPOS, TRUE, sampopt2sliderpos[subsamp]);
	if(subsamp>=0 && subsamp<=TVNC_SAMPOPT-1)
		SetDlgItemText(hwnd, IDC_STATIC_SLEVEL, sampopt2str[subsamp]);
}

void VNCOptions::SetQualitySlider(HWND hwnd, int quality)
{
	HWND hJpegQuality = GetDlgItem(hwnd, IDC_QUALITYLEVEL);
	SendMessage(hJpegQuality, TBM_SETPOS, TRUE, quality);
	for (long i = 10; i < 100; i += 10)
		SendMessage(hJpegQuality, TBM_SETTIC, 0, (LPARAM)i);
	if(quality>=1 && quality<=100)
		SetDlgItemInt(hwnd, IDC_STATIC_QUALITY, quality, FALSE);
}

void VNCOptions::SetCompressSlider(HWND hwnd, int compress)
{
	HWND hCompress = GetDlgItem(hwnd, IDC_COMPRESSLEVEL);
	SendMessage(hCompress, TBM_SETPOS, TRUE, compress);
	if (compress == 0)
		SetDlgItemText(hwnd, IDC_STATIC_LEVEL, "None");
	else
		SetDlgItemInt(hwnd, IDC_STATIC_LEVEL, compress, FALSE);
}

void VNCOptions::SetComboBox(HWND hwnd)
{
	HWND hCompressLevel = GetDlgItem(hwnd, IDC_COMPRESSLEVEL);
	HWND hQualityLevel = GetDlgItem(hwnd, IDC_QUALITYLEVEL);
	HWND hSubsampLevel = GetDlgItem(hwnd, IDC_SUBSAMPLEVEL);
	HWND hAllowJpeg = GetDlgItem(hwnd, IDC_ALLOW_JPEG);
	HWND hCopyRect = GetDlgItem(hwnd, ID_SESSION_SET_CRECT);
	HWND hListBox = GetDlgItem(hwnd, IDC_ENCODING);
	int compressLevel = SendMessage(hCompressLevel, TBM_GETPOS, 0, 0);
	int qualityLevel = SendMessage(hQualityLevel, TBM_GETPOS, 0, 0);
	int subsampLevel = sliderpos2sampopt[SendMessage(hSubsampLevel, TBM_GETPOS, 0, 0)];
	int allowJpeg = SendMessage(hAllowJpeg, BM_GETCHECK, 0, 0);
	int copyRect = SendMessage(hCopyRect, BM_GETCHECK, 0, 0);
	int i;

	for (i = 0; i <= (MAX_LEN_COMBO - 1); i++) {			
		if (rfbcombo[i].rfbEncoding == m_PreferredEncoding && copyRect) {
			if (rfbcombo[i].enableJpeg == true) {
				if (allowJpeg && rfbcombo[i].subsampLevel == subsampLevel
					&& rfbcombo[i].qualityLevel == qualityLevel) {
					SendMessage(hListBox, CB_SETCURSEL, i, 0);
					break;
				}
			} else {
				if (!allowJpeg && rfbcombo[i].compressLevel == compressLevel) {
					SendMessage(hListBox, CB_SETCURSEL, i, 0);
					break;
				}
			}
		}
	}
	if (i == MAX_LEN_COMBO) {
		int count = SendMessage(hListBox, CB_GETCOUNT, 0, 0);
		if (count <= MAX_LEN_COMBO)
			SendMessage(hListBox, CB_INSERTSTRING, MAX_LEN_COMBO, (LPARAM)"Custom");
		SendMessage(hListBox, CB_SETCURSEL, MAX_LEN_COMBO, 0);
	}
	else {
		int count = SendMessage(hListBox, CB_GETCOUNT, 0, 0);
		while (count > MAX_LEN_COMBO)
			count = SendMessage(hListBox, CB_DELETESTRING, MAX_LEN_COMBO, 0);
	}
}

BOOL CALLBACK VNCOptions::DlgProcGlobalOptions(HWND hwnd, UINT uMsg,
											   WPARAM wParam, LPARAM lParam)
{
	VNCOptions *_this = (VNCOptions *) GetWindowLong(hwnd, GWL_USERDATA);
	
	switch (uMsg) {
		
	case WM_INITDIALOG: 
		{					
			SetWindowLong(hwnd, GWL_USERDATA, lParam);
			VNCOptions *_this = (VNCOptions *) lParam;
			// Initialise the controls
	
			HWND hDotCursor = GetDlgItem(hwnd, IDC_DOTCURSOR_RADIO);
			SendMessage(hDotCursor, BM_SETCHECK, 
						(pApp->m_options.m_localCursor == DOTCURSOR), 0);

			HWND hSmallCursor = GetDlgItem(hwnd, IDC_SMALLDOTCURSOR_RADIO);
			SendMessage(hSmallCursor, BM_SETCHECK, 
						(pApp->m_options.m_localCursor == SMALLCURSOR), 0);
			
			HWND hNoCursor = GetDlgItem(hwnd, IDC_NOCURSOR_RADIO);
			SendMessage(hNoCursor, BM_SETCHECK, 
						(pApp->m_options.m_localCursor == NOCURSOR), 0);
			
			HWND hNormalCursor = GetDlgItem(hwnd, IDC_NORMALCURSOR_RADIO);
			SendMessage(hNormalCursor, BM_SETCHECK, 
						(pApp->m_options.m_localCursor == NORMALCURSOR), 0);
			
			HWND hMessage = GetDlgItem(hwnd, IDC_CHECK_MESSAGE);
			SendMessage(hMessage, BM_SETCHECK, !pApp->m_options.m_skipprompt, 0);
			
			HWND hToolbar = GetDlgItem(hwnd, IDC_CHECK_TOOLBAR);
			SendMessage(hToolbar, BM_SETCHECK, pApp->m_options.m_toolbar, 0);
			
			HWND hEditList = GetDlgItem(hwnd, IDC_EDIT_AMOUNT_LIST);
			SetDlgItemInt( hwnd, IDC_EDIT_AMOUNT_LIST, pApp->m_options.m_historyLimit, FALSE);
			
			HWND hSpin1 = GetDlgItem(hwnd, IDC_SPIN1);
			SendMessage(hSpin1, UDM_SETBUDDY, (WPARAM) (HWND)hEditList, 0);
			
			HWND hChec = GetDlgItem(hwnd, IDC_CHECK_LOG_FILE);
			HWND hEditFile = GetDlgItem(hwnd, IDC_EDIT_LOG_FILE);
			HWND hEditLevel = GetDlgItem(hwnd, IDC_EDIT_LOG_LEVEL);
			
			HWND hSpin2 = GetDlgItem(hwnd, IDC_SPIN2);
			SendMessage(hSpin2, UDM_SETBUDDY, (WPARAM) (HWND)hEditLevel, 0);
			
			HWND hListenPort = GetDlgItem(hwnd, IDC_LISTEN_PORT);
			SetDlgItemInt( hwnd, IDC_LISTEN_PORT, pApp->m_options.m_listenPort, FALSE);
			
			HWND hSpin3 = GetDlgItem(hwnd, IDC_SPIN3);
			SendMessage(hSpin3, UDM_SETBUDDY, (WPARAM) (HWND)hListenPort, 0);
			
			SendMessage(hChec, BM_SETCHECK, pApp->m_options.m_logToFile, 0);
			_this->EnableLog(hwnd, !(SendMessage(hChec, BM_GETCHECK, 0, 0) == 0));				
			SetDlgItemInt( hwnd, IDC_EDIT_LOG_LEVEL, pApp->m_options.m_logLevel, FALSE);
			SetDlgItemText( hwnd, IDC_EDIT_LOG_FILE, pApp->m_options.m_logFilename);
			return TRUE;
		}
	case WM_HELP:
		help.Popup(lParam);
		return 0;
	case WM_COMMAND: 
		switch (LOWORD(wParam)) {
		case IDC_LISTEN_PORT:
			switch (HIWORD(wParam)) {
			case EN_KILLFOCUS:
				Lim(hwnd, IDC_LISTEN_PORT, 1, 65536);				
				return 0;
			}
			return 0;
		case IDC_EDIT_AMOUNT_LIST:
			switch (HIWORD(wParam)) {
			case EN_KILLFOCUS:
				Lim(hwnd, IDC_EDIT_AMOUNT_LIST, 0, 64);				
				return 0;
			}
			return 0;
		case IDC_EDIT_LOG_LEVEL:
			switch (HIWORD(wParam)) {
			case EN_KILLFOCUS:
				Lim(hwnd, IDC_EDIT_LOG_LEVEL, 0, 12);				
				return 0;
			}
			return 0;
		case IDC_LOG_BROWSE:
			_this->BrowseLogFile();
			SetDlgItemText(hwnd, IDC_EDIT_LOG_FILE,
						_this->m_logFilename);
			return 0;
		case IDC_BUTTON_CLEAR_LIST: 
			{
				HKEY hRegKey;
				TCHAR value[80];
				TCHAR data[80];
				DWORD valuesize=80;
				DWORD datasize=80;
				DWORD index=0;
				
				RegOpenKey(HKEY_CURRENT_USER,
					KEY_VNCVIEWER_HISTORI, &hRegKey);
				
				while (RegEnumValue(hRegKey, index, value, &valuesize,
					NULL, NULL, (LPBYTE)data, &datasize) == ERROR_SUCCESS) {
					pApp->m_options.delkey(data, KEY_VNCVIEWER_HISTORI);
					RegDeleteValue(hRegKey, value);
					valuesize = 80;
					datasize = 80;
				}
				
				RegCloseKey(hRegKey);
				return 0;
			}
		case IDC_OK:
			{			
				HWND hDotCursor = GetDlgItem(hwnd, IDC_DOTCURSOR_RADIO);
				HWND hNoCursor = GetDlgItem(hwnd, IDC_NOCURSOR_RADIO);
				HWND hNormalCursor = GetDlgItem(hwnd, IDC_NORMALCURSOR_RADIO);
				HWND hSmallCursor = GetDlgItem(hwnd, IDC_SMALLDOTCURSOR_RADIO);
				char buf[80];
				HWND hMessage = GetDlgItem(hwnd, IDC_CHECK_MESSAGE);
				HWND hToolbar = GetDlgItem(hwnd, IDC_CHECK_TOOLBAR);
				HWND hChec = GetDlgItem(hwnd, IDC_CHECK_LOG_FILE);

				if (SendMessage(hDotCursor, BM_GETCHECK, 0, 0) == BST_CHECKED) {
					pApp->m_options.m_localCursor = DOTCURSOR;
					SetClassLong(_this->m_hWindow, GCL_HCURSOR,
									(long)LoadCursor(pApp->m_instance, 
									MAKEINTRESOURCE(IDC_DOTCURSOR)));
				}
				if (SendMessage(hSmallCursor, BM_GETCHECK, 0, 0) == BST_CHECKED) {
					pApp->m_options.m_localCursor = SMALLCURSOR;
					SetClassLong(_this->m_hWindow, GCL_HCURSOR,
									(long)LoadCursor(pApp->m_instance, 
									MAKEINTRESOURCE(IDC_SMALLDOT)));
				}
				if (SendMessage(hNoCursor, BM_GETCHECK, 0, 0) == BST_CHECKED) {
					pApp->m_options.m_localCursor = NOCURSOR;
					SetClassLong(_this->m_hWindow, GCL_HCURSOR,
									(long)LoadCursor(pApp->m_instance, 
									MAKEINTRESOURCE(IDC_NOCURSOR)));
				}
				if (SendMessage(hNormalCursor, BM_GETCHECK, 0, 0) == BST_CHECKED) {
					pApp->m_options.m_localCursor = NORMALCURSOR;
					SetClassLong(_this->m_hWindow, GCL_HCURSOR,
									(long)LoadCursor(NULL,IDC_ARROW));
				}
				
				if (SendMessage(hChec, BM_GETCHECK, 0, 0) == 0) {
					pApp->m_options.m_logToFile = false;
				} else {
					pApp->m_options.m_logToFile = true;
					pApp->m_options.m_logLevel = GetDlgItemInt(hwnd,
						IDC_EDIT_LOG_LEVEL, NULL, FALSE);
					GetDlgItemText(hwnd, IDC_EDIT_LOG_FILE,
						buf, 80); 
					strcpy(pApp->m_options.m_logFilename, buf);
					
					vnclog.SetLevel(pApp->m_options.m_logLevel);
					vnclog.SetFile(pApp->m_options.m_logFilename);
				}
				
				if (SendMessage(hMessage, BM_GETCHECK, 0, 0) == 0) {
					pApp->m_options.m_skipprompt = true;
				} else {
					pApp->m_options.m_skipprompt = false;
				}				
				if (SendMessage(hToolbar, BM_GETCHECK, 0, 0) == 0) {
					pApp->m_options.m_toolbar = false;					
				} else {
					pApp->m_options.m_toolbar = true;					
				}
				
				pApp->m_options.m_historyLimit = GetDlgItemInt(hwnd,
					IDC_EDIT_AMOUNT_LIST, NULL, FALSE);
				pApp->m_options.m_listenPort = GetDlgItemInt(hwnd,
					IDC_LISTEN_PORT, NULL, FALSE);
				pApp->m_options.SaveGenOpt();

				return 0;
			}
		case IDC_CHECK_LOG_FILE:
			switch (HIWORD(wParam)) {
			case BN_CLICKED:				
				HWND hChec = GetDlgItem(hwnd, IDC_CHECK_LOG_FILE);				
				if (SendMessage(hChec, BM_GETCHECK, 0, 0) == 0){
					_this->EnableLog(hwnd, TRUE);					
					SendMessage(hChec, BM_SETCHECK, TRUE, 0);
				} else {
					_this->EnableLog(hwnd, FALSE);
					SendMessage(hChec, BM_SETCHECK, FALSE, 0);
				} 
				return 0;
			}
		}
		return 0;				
	case WM_NOTIFY: 
		{		
			LPNMHDR pn = (LPNMHDR)lParam;
			switch (pn->code) {
			case UDN_DELTAPOS: 
				{	int max, min;
					NMUPDOWN lpnmud = *(LPNMUPDOWN) lParam;
					NMHDR hdr = lpnmud.hdr;
					HWND hCtrl = (HWND)SendMessage(hdr.hwndFrom, UDM_GETBUDDY, 0, 0);
					long ctrl = GetDlgCtrlID(hCtrl);
					int h = GetDlgItemInt( hwnd, ctrl, NULL, TRUE);
					if (lpnmud.iDelta > 0) {
						if (h != 0) {
							h = h - 1;
						}
					} else {
						h = h + 1;	
					}
					SetDlgItemInt( hwnd, ctrl, h, FALSE);
					switch (ctrl) {
					case IDC_EDIT_AMOUNT_LIST:
						min = 0;
						max = 64;
						break;
					case IDC_EDIT_LOG_LEVEL:
						min = 0;
						max = 12;
						break;
					case IDC_LISTEN_PORT:
						min = 1;
						max = 65536;
						break;
					}
					_this->Lim(hwnd, ctrl, min, max);
					return 0;
				}			
			}
			return 0;
		}
	}
	return 0;
}

void VNCOptions::EnableLog(HWND hwnd, bool enable)
{
	HWND hlevel = GetDlgItem(hwnd, IDC_STATIC_LOG_LEVEL);
	HWND hEditFile = GetDlgItem(hwnd, IDC_EDIT_LOG_FILE);
	HWND hEditLevel = GetDlgItem(hwnd, IDC_EDIT_LOG_LEVEL);
	HWND hLogBrowse = GetDlgItem(hwnd, IDC_LOG_BROWSE);
	
	EnableWindow(hEditFile, enable);
	EnableWindow(hEditLevel, enable);
	EnableWindow(hlevel, enable);
	EnableWindow(hLogBrowse, enable);
}

void VNCOptions::Lim(HWND hwnd, int control, DWORD min, DWORD max)
{
	DWORD buf;
	int error;
	buf=GetDlgItemInt(hwnd, control,
					&error, FALSE);
	if (buf > max && error) {
		buf = max;
		SetDlgItemInt(hwnd, control,
					buf, FALSE);
	}
	if (buf < min && error) {
		buf = min;
		SetDlgItemInt(hwnd, control,
					buf, FALSE);
	}
}

void VNCOptions::LoadOpt(char subkey[256], char keyname[256])
{
	HKEY RegKey;
	TCHAR key[80];
	_tcscpy(key, keyname);
	_tcscat(key, "\\");
	_tcscat(key, subkey);
	 RegOpenKeyEx(HKEY_CURRENT_USER, key, 0,  
		 KEY_ALL_ACCESS,  &RegKey);
	for (int i = rfbEncodingRaw; i <= LASTENCODING; i++) {
		char buf[128];
		sprintf(buf, "use_encoding_%d", i);
		m_UseEnc[i] =   read(RegKey, buf, m_UseEnc[i] ) != 0;
	}
	m_PreferredEncoding =	read(RegKey, "preferred_encoding",m_PreferredEncoding    );
	if (m_PreferredEncoding != rfbEncodingTight)
		m_PreferredEncoding = rfbEncodingTight;
	m_restricted =			read(RegKey, "restricted",        m_restricted           ) != 0;
	m_ViewOnly =			read(RegKey, "viewonly",	      m_ViewOnly             ) != 0;
	m_FullScreen =			read(RegKey, "fullscreen",        m_FullScreen           ) != 0;
	m_Use8Bit =				read(RegKey, "8bit",	          m_Use8Bit              ) != 0;
	m_DoubleBuffer =		read(RegKey, "doublebuffer",	  m_DoubleBuffer              ) != 0;
	m_Shared =				read(RegKey, "shared",            m_Shared               ) != 0;
	m_SwapMouse =			read(RegKey, "swapmouse",         m_SwapMouse	         ) != 0;
	m_DeiconifyOnBell =		read(RegKey, "belldeiconify",     m_DeiconifyOnBell      ) != 0;
	m_Emul3Buttons =		read(RegKey, "emulate3",	      m_Emul3Buttons         ) != 0;
	m_Emul3Timeout =		read(RegKey, "emulate3timeout",   m_Emul3Timeout         );
	m_Emul3Fuzz =			read(RegKey, "emulate3fuzz",	  m_Emul3Fuzz            );
	m_DisableClipboard =	read(RegKey, "disableclipboard",  m_DisableClipboard     ) != 0;
	m_FitWindow =			read(RegKey, "fitwindow",		  m_FitWindow		     ) != 0;
	m_scale_den =			read(RegKey, "scale_den",         m_scale_den	         );
	m_scale_num =			read(RegKey, "scale_num",         m_scale_num	         );
	m_requestShapeUpdates =	read(RegKey, "cursorshape",       m_requestShapeUpdates	 ) != 0;
	m_ignoreShapeUpdates =	read(RegKey, "noremotecursor",    m_ignoreShapeUpdates   ) != 0;
	int level		 =		read(RegKey, "compresslevel",     -1				     );
	if (level != -1) {
		m_compressLevel = level;
		if (m_compressLevel > 1) m_compressLevel = 1;
	}
	level		 =		read(RegKey, "subsampling",     -1				     );
	if (level != -1) {
		m_subsampLevel = level;
	}
	m_scaling =				read(RegKey, "scaling",			  m_scaling  ) != 0;	
		
	m_enableJpegCompression = true;
	level =					read(RegKey, "quality",			  95);
	if (level == -1) {
		m_enableJpegCompression = false;
	} else {
		m_jpegQualityLevel = level;
	}
	RegCloseKey(RegKey);
}

int VNCOptions::read(HKEY hkey, char *name, int retrn)
{
	DWORD buflen = 4;
	DWORD buf = 0;
	if(RegQueryValueEx(hkey ,(LPTSTR)name , 
            NULL, NULL, 
            (LPBYTE) &buf, (LPDWORD) &buflen) != ERROR_SUCCESS) {
		return retrn;
	} else {
		return buf;
	}
}

void VNCOptions::SaveOpt(char subkey[256], char keyname[256])
{
	DWORD dispos;
	HKEY RegKey;
	TCHAR key[80];
	_tcscpy(key, keyname);
	_tcscat(key, "\\");
	_tcscat(key, subkey);
	RegCreateKeyEx(HKEY_CURRENT_USER, key, 0, NULL, 
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &RegKey, &dispos);
	for (int i = rfbEncodingRaw; i <= LASTENCODING; i++) {
		char buf[128];
		sprintf(buf, "use_encoding_%d", i);
		save(RegKey, buf, m_UseEnc[i] );
	}
	save(RegKey, "preferred_encoding",	m_PreferredEncoding);
	save(RegKey, "restricted",			m_restricted		);
	save(RegKey, "viewonly",			m_ViewOnly			);
	save(RegKey, "fullscreen",			m_FullScreen		);
	save(RegKey, "scaling",				m_scaling			);
	save(RegKey, "8bit",				m_Use8Bit			);
	save(RegKey, "doublebuffer",		m_DoubleBuffer		);
	save(RegKey, "shared",				m_Shared			);
	save(RegKey, "swapmouse",			m_SwapMouse			);
	save(RegKey, "belldeiconify",		m_DeiconifyOnBell	);
	save(RegKey, "emulate3",			m_Emul3Buttons		);
	save(RegKey, "emulate3timeout",		m_Emul3Timeout		);
	save(RegKey, "emulate3fuzz",		m_Emul3Fuzz			);
	save(RegKey, "disableclipboard",	m_DisableClipboard  );
	save(RegKey, "fitwindow",			m_FitWindow			);
	save(RegKey, "scale_den",			m_scale_den			);
	save(RegKey, "scale_num",			m_scale_num			);
	save(RegKey, "cursorshape",			m_requestShapeUpdates );
	save(RegKey, "noremotecursor",		m_ignoreShapeUpdates );	
	save(RegKey, "compresslevel", m_compressLevel );
	save(RegKey, "subsampling", m_subsampLevel );
	save(RegKey, "quality",	m_enableJpegCompression ? m_jpegQualityLevel : -1);
	
	
	RegCloseKey(RegKey);
}

void VNCOptions::delkey(char subkey[256], char keyname[256])
{
		
	TCHAR key[80];
	_tcscpy(key, keyname);
	_tcscat(key, "\\");
	_tcscat(key, subkey);
	RegDeleteKey (HKEY_CURRENT_USER, key);
}

void VNCOptions::save(HKEY hkey, char *name, int value) 
{
	RegSetValueEx( hkey, name, 
            NULL, REG_DWORD, 
            (CONST BYTE *)&value, 4);
}

void VNCOptions::LoadGenOpt()
{
	HKEY hRegKey;
		
	if ( RegOpenKey(HKEY_CURRENT_USER,
					SETTINGS_KEY_NAME, &hRegKey) != ERROR_SUCCESS ) {
		hRegKey = NULL;
	} else {
		DWORD buffer;
		DWORD buffersize = sizeof(buffer);
		DWORD valtype;
		if ( RegQueryValueEx( hRegKey, "SkipFullScreenPrompt", NULL, &valtype, 
				(LPBYTE)&buffer, &buffersize) == ERROR_SUCCESS) {			
			m_skipprompt = buffer == 1;				
		}		
		if ( RegQueryValueEx( hRegKey, "NoToolbar", NULL, &valtype, 
				(LPBYTE)&buffer, &buffersize) == ERROR_SUCCESS) {
			m_toolbar = buffer == 1;
		}
		if ( RegQueryValueEx( hRegKey, "LogToFile", NULL, &valtype, 
				(LPBYTE)&buffer, &buffersize) == ERROR_SUCCESS) {
				m_logToFile = buffer == 1;
		}
		if ( RegQueryValueEx( hRegKey, "HistoryLimit", NULL, &valtype, 
				(LPBYTE)&buffer, &buffersize) == ERROR_SUCCESS) {
			m_historyLimit = buffer;
		}
		if ( RegQueryValueEx( hRegKey, "LocalCursor", NULL, &valtype, 
				(LPBYTE)&buffer, &buffersize) == ERROR_SUCCESS) {
			m_localCursor = buffer;
		}
		if ( RegQueryValueEx( hRegKey, "LogLevel", NULL, &valtype, 
				(LPBYTE)&buffer, &buffersize) == ERROR_SUCCESS) {
			m_logLevel = buffer;
		}
		if ( RegQueryValueEx( hRegKey, "ListenPort", NULL, &valtype, 
				(LPBYTE)&buffer, &buffersize) == ERROR_SUCCESS) {
			m_listenPort = buffer;
		}
		TCHAR buf[_MAX_PATH];
		buffersize=_MAX_PATH;
		if (RegQueryValueEx( hRegKey, "LogFileName", NULL, &valtype, 
				(LPBYTE) &buf, &buffersize) == ERROR_SUCCESS) {
			strcpy(m_logFilename, buf);
		}
		RegCloseKey(hRegKey);
	}	
}

void VNCOptions::SaveGenOpt()
{
	HKEY hRegKey;
	DWORD buffer;
	TCHAR buf[80];

	RegCreateKey(HKEY_CURRENT_USER,
					SETTINGS_KEY_NAME, &hRegKey);
	RegSetValueEx( hRegKey, "LocalCursor", 
					NULL, REG_DWORD, 
					(CONST BYTE *)&m_localCursor,
					4);
				
	RegSetValueEx( hRegKey, "HistoryLimit", 
					NULL, REG_DWORD, 
					(CONST BYTE *)&m_historyLimit,
					4);
				
	RegSetValueEx( hRegKey, "LogLevel", 
					NULL, REG_DWORD, 
					(CONST BYTE *)&m_logLevel,
					4);
				
	strcpy(buf, m_logFilename);
	RegSetValueEx( hRegKey, "LogFileName", 
					NULL, REG_SZ , 
					(CONST BYTE *)buf, (_tcslen(buf)+1));				
				
	RegSetValueEx( hRegKey, "ListenPort", 
					NULL, REG_DWORD, 
					(CONST BYTE *)&m_listenPort,
					4);
	if (m_logToFile) {
		buffer = 1;
	} else {
		buffer = 0;
	}
	RegSetValueEx( hRegKey, "LogToFile" , 
					NULL, REG_DWORD, 
					(CONST BYTE *)&buffer,
					4);
	if (m_skipprompt) {
		buffer = 1;
	} else {
		buffer = 0;
	}
	RegSetValueEx( hRegKey, "SkipFullScreenPrompt", 
					NULL,REG_DWORD, 
					(CONST BYTE *)&buffer,
					4);
	if (m_toolbar) {
		buffer = 1;
	} else {
		buffer = 0;
	}
	RegSetValueEx( hRegKey, "NoToolbar", 
					NULL, REG_DWORD, 
					(CONST BYTE *)&buffer,
					4);
				
	RegCloseKey(hRegKey);
}

void VNCOptions::BrowseLogFile()
{
	OPENFILENAME ofn;
	char filter[] = "Log files (*.log)\0*.log\0" \
						   "All files (*.*)\0*.*\0";
	char tname[_MAX_FNAME + _MAX_EXT];
	memset((void *) &ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.lpstrFilter = filter;
	ofn.nMaxFile = _MAX_PATH;
	ofn.nMaxFileTitle = _MAX_FNAME + _MAX_EXT;
	ofn.lpstrDefExt = "log";	
	ofn.hwndOwner = m_hParent;
	ofn.lpstrFile = pApp->m_options.m_logFilename;
	ofn.lpstrFileTitle = tname;
	ofn.lpstrTitle = "Log file";
	ofn.Flags = OFN_HIDEREADONLY;
	if (!GetSaveFileName(&ofn)) {
		DWORD err = CommDlgExtendedError();
		char msg[1024]; 
		switch(err) {
		case 0:	// user cancelled
			break;
		case FNERR_INVALIDFILENAME:
			strcpy(msg, "Invalid filename");
			MessageBox(m_hParent, msg, "Error log file", MB_ICONERROR | MB_OK);
			break;
		default:
			vnclog.Print(0, "Error %d from GetSaveFileName\n", err);
			break;
		}
		return;
	}
}
