//  Copyright (C) 2010 D. R. Commander. All Rights Reserved.
//  Copyright (C) 2005-2006 Sun Microsystems, Inc. All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation All Rights Reserved.
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


#ifndef VNCOPTIONS_H__
#define VNCOPTIONS_H__

#pragma once

// The idea here was to use a slider control.  However, that seems
// to require MFC, which has not been used up to this point.  I 
// wonder why not?  WinCE compatibility maybe?
// #include <afxcmn.h>


#define LASTENCODING rfbEncodingZlibHex

#define NOCURSOR 0
#define DOTCURSOR 1
#define NORMALCURSOR 2
#define SMALLCURSOR 3
#define MAX_LEN_COMBO 5

#define KEY_VNCVIEWER_HISTORI _T("Software\\TurboVNC\\VNCviewer\\History")

struct COMBOSTRING {
		TCHAR NameString[41];
		int rfbEncoding;
		bool enableJpeg;
		int subsampLevel;
		int qualityLevel;
		int compressLevel;
};

class VNCOptions  
{
public:
	VNCOptions();
	VNCOptions& operator=(VNCOptions& s);
	virtual ~VNCOptions();
	
	// Save and load a set of options from a config file
	void Save(char *fname);
	void Load(char *fname);
	void VNCOptions::LoadOpt(char subkey[256],char keyname[256]);
	int VNCOptions::read(HKEY hkey,char *name,int retrn);
	void VNCOptions::save(HKEY hkey,char *name, int value);
	void VNCOptions::LoadGenOpt();
	void VNCOptions::SaveGenOpt();
	void VNCOptions::delkey(char subkey[256],char keyname[256]);
	void VNCOptions::SaveOpt(char subkey[256],char keyname[256]);
	// process options
	bool	m_listening;
	int		m_listenPort;
	TCHAR	m_display[256];
	bool	m_toolbar;
	bool	m_skipprompt;
	int		m_historyLimit;
	bool	m_connectionSpecified;
	bool	m_configSpecified;
	TCHAR   m_configFilename[_MAX_PATH];
	unsigned char m_encPasswd[8];
	bool	m_restricted;

	// default connection options - can be set through Dialog
	bool	m_ViewOnly;
	bool	m_FullScreen;
	bool	m_fastFSToggle;
	bool	m_Use8Bit;
	bool	m_DoubleBuffer;
	int		m_PreferredEncoding;
	bool	m_SwapMouse;
	bool    m_Emul3Buttons; 
 	int     m_Emul3Timeout;
 	int     m_Emul3Fuzz;
	bool	m_Shared;
	bool	m_DeiconifyOnBell;
	bool	m_DisableClipboard;
	int     m_localCursor;
	bool	m_scaling;
	bool	m_FitWindow;
	int		m_scale_num, m_scale_den; // Numerator & denominator
	int		m_subsampLevel;
	int		m_compressLevel;
	bool	m_enableJpegCompression;
	int		m_jpegQualityLevel;
	bool	m_requestShapeUpdates;
	bool	m_ignoreShapeUpdates;

	// Keyboard can be specified on command line as 8-digit hex
	TCHAR	m_kbdname[9];
	bool	m_kbdSpecified;

	// Connection options we don't do through the dialog
	// Which encodings do we allow?
	bool	m_UseEnc[LASTENCODING+1];

	TCHAR   m_host[256];
	int     m_port;

    // Logging
    int     m_logLevel;
    bool    m_logToFile, m_logToConsole;
    TCHAR   m_logFilename[_MAX_PATH];
    
	// for debugging purposes
	int m_delay;

#ifdef UNDER_CE
	// WinCE screen format for dialogs (Palm vs HPC)
	int	m_palmpc;
	// Use slow GDI rendering, but more accurate colours.
	int m_slowgdi;
#endif

	INT_PTR DoDialog(bool running = false);
	BOOL RaiseDialog();
	void CloseDialog();

	void SetFromCommandLine(LPTSTR szCmdLine);


	static BOOL CALLBACK DlgProc(HWND hwndDlg, UINT uMsg,
		WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK DlgProcConnOptions(HWND hwnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK DlgProcGlobalOptions(HWND hwnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam);
	static void Lim(HWND hwnd,int control,DWORD min, DWORD max);
	// Register() makes this viewer the app invoked for .vnc files
	static void Register();
	HWND m_hPageConnection, m_hPageGeneral, m_hTab, m_hParent, m_hWindow;
	void FixScaling();

private:
	void BrowseLogFile();
	void EnableCompress(HWND hwnd, bool enable);
	void EnableJpeg(HWND hwnd, bool enable);
	void EnableSubsamp(HWND hwnd, bool enable);
	void EnableLog(HWND hwnd, bool enable);
	void SetSubsampSlider(HWND hwnd, int subsamp);
	void SetQualitySlider(HWND hwnd, int subsamp);
	void SetCompressSlider(HWND hwnd, int subsamp);
	void SetComboBox(HWND hwnd);

	// Just for temporary use
	bool m_running;

	void setHistoryLimit(int newLimit);
};

#endif // VNCOPTIONS_H__

