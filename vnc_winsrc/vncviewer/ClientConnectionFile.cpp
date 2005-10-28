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
// whence you received this file, check http://www.uk.research.att.com/vnc or 
// contact the authors on vnc@uk.research.att.com for information on obtaining it.
//

#include "stdhdrs.h"
#include "vncviewer.h"
#include "ClientConnection.h"
#include "Exception.h"
#include "vncauth.h"

// This file contains the code for saving and loading connection info.

static OPENFILENAME ofn;

static void ofnInit()
{
	static char filter[] = "VNC files (*.vnc)\0*.vnc\0" \
						   "All files (*.*)\0*.*\0";
	memset((void *) &ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.lpstrFilter = filter;
	ofn.nMaxFile = _MAX_PATH;
	ofn.nMaxFileTitle = _MAX_FNAME + _MAX_EXT;
	ofn.lpstrDefExt = "vnc";
}

//
// SaveConnection
// Save info about current connection to a file
//

void ClientConnection::SaveConnection()
{
	vnclog.Print(2, _T("Saving connection info\n"));	
	char tname[_MAX_FNAME + _MAX_EXT];
	ofnInit();

	// Let's choose a reasonable file name based on hostname and port
	char fname[_MAX_PATH];

	// If the first character of the hostname is a letter, try to use
	// only the part of the hostname before the first dot
	char *ptr = NULL;
	if (isalpha(m_host[0])) {
		ptr = strchr(m_host, '.');
		if (ptr - m_host > 24)
			ptr = NULL;
	}
	if (ptr) {
		memcpy(fname, m_host, ptr - m_host);
		fname[ptr - m_host] = '\0';
	} else {
		sprintf(fname, "%.24s", m_host);
	}
	// Append the port number if it's not the default port
	if (PORT_TO_DISPLAY(m_port) != 0) {
		sprintf(&fname[strlen(fname)], "-%d", m_port);
	}
	// Finally, append the .vnc suffix (note there will be no buffer overrun)
	strcat(fname, ".vnc");

	ofn.hwndOwner = m_hwnd;
	ofn.lpstrFile = fname;
	ofn.lpstrFileTitle = tname; 
	ofn.Flags = OFN_HIDEREADONLY;
	if (!GetSaveFileName(&ofn)) {
		DWORD err = CommDlgExtendedError();
		char msg[1024]; 
		switch(err) {
		case 0:	// user cancelled
			break;
		case FNERR_INVALIDFILENAME:
			strcpy(msg, "Invalid filename");
			MessageBox(m_hwnd, msg, "Error saving file", MB_ICONERROR | MB_OK);
			break;
		default:
			vnclog.Print(0, "Error %d from GetSaveFileName\n", err);
			break;
		}
		return;
	}
	vnclog.Print(1, "Saving to %s\n", fname);	
	int ret = WritePrivateProfileString("connection", "host", m_host, fname);
	char buf[32];
	sprintf(buf, "%d", m_port);
	WritePrivateProfileString("connection", "port", buf, fname);
	buf[0] = '\0';
	if (m_authScheme == rfbAuthVNC || m_authScheme == rfbAuthExternal) {
		if (MessageBox(m_hwnd,
			"Do you want to save the password in this file?\n\r"
			"If you say Yes, anyone with access to this file could access your session\n\r"
			"and (potentially) discover your VNC password.",  
			"Security warning", 
			MB_YESNO | MB_ICONWARNING) == IDYES) 
		{
			if (m_authScheme == rfbAuthVNC) {
				for (int i = 0; i < MAXPWLEN; i++) {
					sprintf(buf+i*2, "%02x", (unsigned int) m_encPasswd[i]);
				}
				WritePrivateProfileString("connection", "password", buf, fname);
			} else {	// (m_authScheme == rfbAuthExternal)
				CARD8 usernameLen = m_encPasswdExt[0];
				CARD8 passwordLen = m_encPasswdExt[1];
				int len = (usernameLen + passwordLen + 7) & 0xFFFFFFF8;
				char *bigbuf = new char[(2 + len) * 2 + 1];
				for (int i = 0; i < 2 + len; i++) {
					sprintf(bigbuf+i*2, "%02x", (unsigned int) m_encPasswdExt[i]);
				}
				WritePrivateProfileString("connection", "password_ext", bigbuf, fname);
				delete[] bigbuf;
			}
		} else if (m_authScheme == rfbAuthExternal) {
			WritePrivateProfileString("connection", "username_ext", m_usernameExt, fname);
		}
	}
	m_opts.Save(fname);
	m_opts.Register();
}

// returns zero if successful
int ClientConnection::LoadConnection(char *fname, bool sess)
{
	if (sess) {
		char tname[_MAX_FNAME + _MAX_EXT];
		
		ofnInit();
		
		ofn.hwndOwner = m_hSess;
		ofn.lpstrFile = fname;
		ofn.lpstrFileTitle = tname;
		ofn.Flags = OFN_HIDEREADONLY;

		if (GetOpenFileName(&ofn) == 0) {
			return -1;
		}
	}
	if (GetPrivateProfileString("connection", "host", "", m_host, MAX_HOST_NAME_LEN, fname) == 0) {
		MessageBox(m_hwnd, "Error reading host name from file", "Config file error", MB_ICONERROR | MB_OK);
		return -1;
	}
	if ((m_port = GetPrivateProfileInt("connection", "port", 0, fname)) == 0) {
		MessageBox(m_hwnd, "Error reading port number from file", "Config file error", MB_ICONERROR | MB_OK);
		return -1;
	}
	FormatDisplay(m_port, m_opts.m_display, m_host);

	char buf[1026];
	m_encPasswd[0] = '\0';
	memset(m_encPasswdExt, 0, 2);
	if (GetPrivateProfileString("connection", "password", "", buf, 32, fname) > 0) {
		for (int i = 0; i < MAXPWLEN; i++)	{
			int x = 0;
			sscanf(buf+i*2, "%2x", &x);
			m_encPasswd[i] = (unsigned char) x;
		}
	}
	GetPrivateProfileString("connection", "username_ext", "", m_usernameExt, 256, fname);
	if (GetPrivateProfileString("connection", "password_ext", "", buf, 1026, fname) >= 2) {
		int len = strlen(buf) / 2;
		for (int i = 0; i < len; i++)	{
			int x = 0;
			sscanf(buf+i*2, "%2x", &x);
			m_encPasswdExt[i] = (unsigned char) x;
		}
	}
	if (sess) {
		m_opts.Load(fname);
		m_opts.Register();
	}
	return 0;
}
