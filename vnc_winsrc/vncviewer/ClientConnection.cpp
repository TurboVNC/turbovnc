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


// Many thanks to Randy Brown <rgb@inven.com> for providing the 3-button
// emulation code.

// This is the main source for a ClientConnection object.
// It handles almost everything to do with a connection to a server.
// The decoding of specific rectangle encodings is done in separate files.

#include "stdhdrs.h"

#include "vncviewer.h"

#ifdef UNDER_CE
#include "omnithreadce.h"
#define SD_BOTH 0x02
#else
#include "omnithread.h"
#endif

#include "ClientConnection.h"
#include "SessionDialog.h"
#include "AuthDialog.h"
#include "AboutBox.h"

#include "Exception.h"
extern "C" {
	#include "vncauth.h"
}

#define INITIALNETBUFSIZE 4096
#define MAX_ENCODINGS 20
#define VWR_WND_CLASS_NAME _T("VNCviewer")

/*
 * Macro to compare pixel formats.
 */

#define PF_EQ(x,y)							\
	((x.bitsPerPixel == y.bitsPerPixel) &&				\
	 (x.depth == y.depth) &&					\
	 ((x.bigEndian == y.bigEndian) || (x.bitsPerPixel == 8)) &&	\
	 (x.trueColour == y.trueColour) &&				\
	 (!x.trueColour || ((x.redMax == y.redMax) &&			\
			    (x.greenMax == y.greenMax) &&		\
			    (x.blueMax == y.blueMax) &&			\
			    (x.redShift == y.redShift) &&		\
			    (x.greenShift == y.greenShift) &&		\
			    (x.blueShift == y.blueShift))))

const rfbPixelFormat vnc8bitFormat = {8, 8, 0, 1, 7,7,3, 0,3,6,0,0};
const rfbPixelFormat vnc16bitFormat = {16, 16, 0, 1, 63, 31, 31, 0,6,11,0,0};


// *************************************************************************
//  A Client connection involves two threads - the main one which sets up
//  connections and processes window messages and inputs, and a 
//  client-specific one which receives, decodes and draws output data 
//  from the remote server.
//  This first section contains bits which are generally called by the main
//  program thread.
// *************************************************************************

ClientConnection::ClientConnection(VNCviewerApp *pApp) 
{
	Init(pApp);
}

ClientConnection::ClientConnection(VNCviewerApp *pApp, SOCKET sock) 
{
	Init(pApp);
	m_sock = sock;
	m_serverInitiated = true;
	struct sockaddr_in svraddr;
	int sasize = sizeof(svraddr);
	if (getpeername(sock, (struct sockaddr *) &svraddr, 
		&sasize) != SOCKET_ERROR) {
		_stprintf(m_host, _T("%d.%d.%d.%d"), 
			svraddr.sin_addr.S_un.S_un_b.s_b1, 
			svraddr.sin_addr.S_un.S_un_b.s_b2, 
			svraddr.sin_addr.S_un.S_un_b.s_b3, 
			svraddr.sin_addr.S_un.S_un_b.s_b4);
		m_port = svraddr.sin_port;
	} else {
		_tcscpy(m_host,_T("(unknown)"));
		m_port = 0;
	};
}

ClientConnection::ClientConnection(VNCviewerApp *pApp, LPTSTR host, int port)
{
	Init(pApp);
	_tcsncpy(m_host, host, MAX_HOST_NAME_LEN);
	m_port = port;
}

void ClientConnection::Init(VNCviewerApp *pApp)
{
	m_hwnd = 0;
	m_desktopName = NULL;
	m_port = -1;
	m_serverInitiated = false;
	m_netbuf = NULL;
	m_netbufsize = 0;
	m_zlibbuf = NULL;
	m_zlibbufsize = 0;
	m_hwndNextViewer = NULL;	
	m_pApp = pApp;
	m_dormant = false;
	m_hBitmapDC = NULL;
	m_hBitmap = NULL;
	m_hPalette = NULL;
	m_encPasswd[0] = '\0';

	// We take the initial conn options from the application defaults
	m_opts = m_pApp->m_options;

	m_sock = INVALID_SOCKET;
	m_bKillThread = false;
	m_threadStarted = true;
	m_running = false;
	m_pendingFormatChange = false;

	m_hScrollPos = 0; m_vScrollPos = 0;

	m_waitingOnEmulateTimer = false;
	m_emulatingMiddleButton = false;

	m_decompStreamInited = false;

	m_decompStreamRaw.total_in = ZLIBHEX_DECOMP_UNINITED;
	m_decompStreamEncoded.total_in = ZLIBHEX_DECOMP_UNINITED;

	for (int i = 0; i < 4; i++)
		m_tightZlibStreamActive[i] = false;

	prevCursorSet = false;
	rcCursorX = 0;
	rcCursorY = 0;

	// Create a buffer for various network operations
	CheckBufferSize(INITIALNETBUFSIZE);

	m_pApp->RegisterConnection(this);
}

// 
// Run() creates the connection if necessary, does the initial negotiations
// and then starts the thread running which does the output (update) processing.
// If Run throws an Exception, the caller must delete the ClientConnection object.
//

void ClientConnection::Run()
{
	// Get the host name and port if we haven't got it
	if (m_port == -1) 
		GetConnectDetails();

	// Connect if we're not already connected
	if (m_sock == INVALID_SOCKET) 
		Connect();

	SetSocketOptions();

	NegotiateProtocolVersion();
	
	Authenticate();

	// Set up windows etc 
	CreateDisplay();

	SendClientInit();
	
	ReadServerInit();
	
	CreateLocalFramebuffer();
	
	SetupPixelFormat();
	
	SetFormatAndEncodings();

	// This starts the worker thread.
	// The rest of the processing continues in run_undetached.
	start_undetached();
}

void ClientConnection::CreateDisplay() 
{
	// Create the window
	WNDCLASS wndclass;

	wndclass.style			= 0;
	wndclass.lpfnWndProc	= ClientConnection::WndProc;
	wndclass.cbClsExtra		= 0;
	wndclass.cbWndExtra		= 0;
	wndclass.hInstance		= m_pApp->m_instance;
	wndclass.hIcon			= LoadIcon(m_pApp->m_instance, MAKEINTRESOURCE(IDI_MAINICON));
	switch (m_opts.m_localCursor) {
	case NOCURSOR:
		wndclass.hCursor		= LoadCursor(m_pApp->m_instance, MAKEINTRESOURCE(IDC_NOCURSOR));
		break;
	case NORMALCURSOR:
		wndclass.hCursor		= LoadCursor(NULL, IDC_ARROW);
		break;
	case DOTCURSOR:
	default:
		wndclass.hCursor		= LoadCursor(m_pApp->m_instance, MAKEINTRESOURCE(IDC_DOTCURSOR));
	}
	wndclass.hbrBackground	= (HBRUSH) GetStockObject(BLACK_BRUSH);
    wndclass.lpszMenuName	= (const TCHAR *) NULL;
	wndclass.lpszClassName	= VWR_WND_CLASS_NAME;

	RegisterClass(&wndclass);

#ifdef _WIN32_WCE
	const DWORD winstyle = WS_VSCROLL | WS_HSCROLL | WS_CAPTION | WS_SYSMENU;
#else
	const DWORD winstyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | 
	  WS_MINIMIZEBOX | WS_THICKFRAME | WS_VSCROLL | WS_HSCROLL;
#endif

	m_hwnd = CreateWindow(VWR_WND_CLASS_NAME,
			      _T("VNCviewer"),
			      winstyle,
			      CW_USEDEFAULT,
			      CW_USEDEFAULT,
			      CW_USEDEFAULT,       // x-size
			      CW_USEDEFAULT,       // y-size
			      NULL,                // Parent handle
			      NULL,                // Menu handle
			      m_pApp->m_instance,
			      NULL);

	ShowWindow(m_hwnd, SW_HIDE);

	// record which client created this window
	SetWindowLong(m_hwnd, GWL_USERDATA, (LONG) this);


	// Create a memory DC which we'll use for drawing to
	// the local framebuffer
	m_hBitmapDC = CreateCompatibleDC(NULL);

	// Set a suitable palette up
	if (GetDeviceCaps(m_hBitmapDC, RASTERCAPS) & RC_PALETTE) {
		vnclog.Print(3, _T("Palette-based display - %d entries, %d reserved\n"), 
			GetDeviceCaps(m_hBitmapDC, SIZEPALETTE), GetDeviceCaps(m_hBitmapDC, NUMRESERVED));
		BYTE buf[sizeof(LOGPALETTE)+216*sizeof(PALETTEENTRY)];
		LOGPALETTE *plp = (LOGPALETTE *) buf;
		int pepos = 0;
		for (int r = 5; r >= 0; r--) {
			for (int g = 5; g >= 0; g--) {
				for (int b = 5; b >= 0; b--) {
					plp->palPalEntry[pepos].peRed   = r * 255 / 5; 	
					plp->palPalEntry[pepos].peGreen = g * 255 / 5;
					plp->palPalEntry[pepos].peBlue  = b * 255 / 5;
					plp->palPalEntry[pepos].peFlags  = NULL;
					pepos++;
				}
			}
		}
		plp->palVersion = 0x300;
		plp->palNumEntries = 216;
		m_hPalette = CreatePalette(plp);
	}

	// Add stuff to System menu
	HMENU hsysmenu = GetSystemMenu(m_hwnd, FALSE);
	if (!m_opts.m_restricted) {
		AppendMenu(hsysmenu, MF_STRING, IDC_OPTIONBUTTON,	_T("Connection &options..."));
		AppendMenu(hsysmenu, MF_STRING, ID_CONN_ABOUT,		_T("Connection &info"));
		AppendMenu(hsysmenu, MF_STRING, ID_REQUEST_REFRESH,	_T("Request screen &refresh"));

		AppendMenu(hsysmenu, MF_SEPARATOR, NULL, NULL);
		AppendMenu(hsysmenu, MF_STRING, ID_FULLSCREEN,		_T("&Full screen"));
		AppendMenu(hsysmenu, MF_SEPARATOR, NULL, NULL);
 		AppendMenu(hsysmenu, MF_STRING, ID_CONN_CTLALTDEL,	_T("Send Ctl-Alt-Del"));
		AppendMenu(hsysmenu, MF_STRING, ID_CONN_CTLDOWN,	_T("Ctrl Down"));
		AppendMenu(hsysmenu, MF_STRING, ID_CONN_CTLUP,		_T("Ctrl Up"));
		AppendMenu(hsysmenu, MF_STRING, ID_CONN_ALTDOWN,	_T("Alt Down"));
		AppendMenu(hsysmenu, MF_STRING, ID_CONN_ALTUP,		_T("Alt Up"));
		AppendMenu(hsysmenu, MF_SEPARATOR, NULL, NULL);
		AppendMenu(hsysmenu, MF_STRING, ID_NEWCONN,			_T("Ne&w connection..."));
		AppendMenu(hsysmenu, MF_STRING | (m_serverInitiated ? MF_GRAYED : 0), 
			ID_CONN_SAVE_AS,	_T("Save connection info &as..."));
	}
    AppendMenu(hsysmenu, MF_SEPARATOR, NULL, NULL);
	AppendMenu(hsysmenu, MF_STRING, IDD_APP_ABOUT,		_T("&About VNCviewer..."));
	if (m_opts.m_listening) {
		AppendMenu(hsysmenu, MF_SEPARATOR, NULL, NULL);
		AppendMenu(hsysmenu, MF_STRING, ID_CLOSEDAEMON, _T("Close listening &daemon"));
	}
	DrawMenuBar(m_hwnd);

	// Set up clipboard watching
#ifndef _WIN32_WCE
	// We want to know when the clipboard changes, so
	// insert ourselves in the viewer chain. But doing
	// this will cause us to be notified immediately of
	// the current state.
	// We don't want to send that.
	m_initialClipboardSeen = false;
	m_hwndNextViewer = SetClipboardViewer(m_hwnd); 	
#endif
}

void ClientConnection::GetConnectDetails()
{
	if (m_opts.m_configSpecified) {
		LoadConnection(m_opts.m_configFilename);
	} else {
		SessionDialog sessdlg(&m_opts);
		if (!sessdlg.DoDialog()) {
			throw QuietException("User Cancelled");
		}
		_tcsncpy(m_host, sessdlg.m_host, MAX_HOST_NAME_LEN);
		m_port = sessdlg.m_port;
	}
	// This is a bit of a hack: 
	// The config file may set various things in the app-level defaults which 
	// we don't want to be used except for the first connection. So we clear them
	// in the app defaults here.
	m_pApp->m_options.m_host[0] = '\0';
	m_pApp->m_options.m_port = -1;
	m_pApp->m_options.m_connectionSpecified = false;
	m_pApp->m_options.m_configSpecified = false;

}

void ClientConnection::Connect()
{
	struct sockaddr_in thataddr;
	int res;
	
	m_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (m_sock == INVALID_SOCKET) throw WarningException(_T("Error creating socket"));
	int one = 1;
	
	// The host may be specified as a dotted address "a.b.c.d"
	// Try that first
	thataddr.sin_addr.s_addr = inet_addr(m_host);
	
	// If it wasn't one of those, do gethostbyname
	if (thataddr.sin_addr.s_addr == INADDR_NONE) {
		LPHOSTENT lphost;
		lphost = gethostbyname(m_host);
		
		if (lphost == NULL) { 
			throw WarningException("Failed to get server address.\n\r"
				"Did you type the host name correctly?"); 
		};
		thataddr.sin_addr.s_addr = ((LPIN_ADDR) lphost->h_addr)->s_addr;
	};
	
	thataddr.sin_family = AF_INET;
	thataddr.sin_port = htons(m_port);
	res = connect(m_sock, (LPSOCKADDR) &thataddr, sizeof(thataddr));
	if (res == SOCKET_ERROR) throw WarningException("Failed to connect to server");
	vnclog.Print(0, _T("Connected to %s port %d\n"), m_host, m_port);

}

void ClientConnection::SetSocketOptions() {
	// Disable Nagle's algorithm
	BOOL nodelayval = TRUE;
	if (setsockopt(m_sock, IPPROTO_TCP, TCP_NODELAY, (const char *) &nodelayval, sizeof(BOOL)))
		throw WarningException("Error disabling Nagle's algorithm");
}


void ClientConnection::NegotiateProtocolVersion()
{
	rfbProtocolVersionMsg pv;

   /* if the connection is immediately closed, don't report anything, so
       that pmw's monitor can make test connections */

    try {
		ReadExact(pv, sz_rfbProtocolVersionMsg);
	} catch (Exception &e) {
		vnclog.Print(0, _T("Error reading protocol version: %s\n"), e.m_info);
		throw QuietException(e.m_info);
	}

    pv[sz_rfbProtocolVersionMsg] = 0;

	// XXX This is a hack.  Under CE we just return to the server the
	// version number it gives us without parsing it.  
	// Too much hassle replacing sscanf for now. Fix this!
#ifdef UNDER_CE
	m_majorVersion = rfbProtocolMajorVersion;
	m_minorVersion = rfbProtocolMinorVersion;
#else
    if (sscanf(pv,rfbProtocolVersionFormat,&m_majorVersion,&m_minorVersion) != 2) {
		throw WarningException(_T("Invalid protocol"));
    }
    vnclog.Print(0, _T("RFB server supports protocol version %d.%d\n"),
	    m_majorVersion,m_minorVersion);

    if ((m_majorVersion == 3) && (m_minorVersion < 3)) {
		
        /* if server is 3.2 we can't use the new authentication */
		vnclog.Print(0, _T("Can't use IDEA authentication\n"));
        /* This will be reported later if authentication is requested*/

    } else {
		
        /* any other server version, just tell the server what we want */
		m_majorVersion = rfbProtocolMajorVersion;
		m_minorVersion = rfbProtocolMinorVersion;

    }

    sprintf(pv,rfbProtocolVersionFormat,m_majorVersion,m_minorVersion);
#endif

    WriteExact(pv, sz_rfbProtocolVersionMsg);

	vnclog.Print(0, _T("Connected to RFB server, using protocol version %d.%d\n"),
		rfbProtocolMajorVersion, rfbProtocolMinorVersion);
}

void ClientConnection::Authenticate()
{
	CARD32 authScheme, reasonLen, authResult;
    CARD8 challenge[CHALLENGESIZE];
	
	ReadExact((char *)&authScheme, 4);
    authScheme = Swap32IfLE(authScheme);
	
    switch (authScheme) {
		
    case rfbConnFailed:
		ReadExact((char *)&reasonLen, 4);
		reasonLen = Swap32IfLE(reasonLen);
		
		CheckBufferSize(reasonLen+1);
		ReadString(m_netbuf, reasonLen);
		
		vnclog.Print(0, _T("RFB connection failed, reason: %s\n"), m_netbuf);
		throw WarningException(m_netbuf);
        break;
		
    case rfbNoAuth:
		vnclog.Print(0, _T("No authentication needed\n"));
		break;
		
    case rfbVncAuth:
		{
            if ((m_majorVersion == 3) && (m_minorVersion < 3)) {
                /* if server is 3.2 we can't use the new authentication */
                vnclog.Print(0, _T("Can't use IDEA authentication\n"));

                MessageBox(NULL, 
                    _T("Sorry - this server uses an older authentication scheme\n\r")
                    _T("which is no longer supported."), 
                    _T("Protocol Version error"), 
                    MB_OK | MB_ICONSTOP | MB_SETFOREGROUND | MB_TOPMOST);

                throw WarningException("Can't use IDEA authentication any more!");
            }

			ReadExact((char *)challenge, CHALLENGESIZE);
			
			char passwd[256];
			// Was the password already specified in a config file?
			if (strlen((const char *) m_encPasswd)>0) {
				char *pw = vncDecryptPasswd(m_encPasswd);
				strcpy(passwd, pw);
				free(pw);
			} else {
				AuthDialog ad;
				ad.DoDialog();	
#ifndef UNDER_CE
				strcpy(passwd, ad.m_passwd);
#else
				int origlen = _tcslen(ad.m_passwd);
				int newlen = WideCharToMultiByte(
					CP_ACP,    // code page
					0,         // performance and mapping flags
					ad.m_passwd, // address of wide-character string
					origlen,   // number of characters in string
					passwd,    // address of buffer for new string
					255,       // size of buffer
					NULL, NULL );
				
				passwd[newlen]= '\0';
#endif
				if (strlen(passwd) == 0) {
					vnclog.Print(0, _T("Password had zero length\n"));
					throw AuthException("Empty password");
				}
				if (strlen(passwd) > 8) {
					passwd[8] = '\0';
				}
				vncEncryptPasswd(m_encPasswd, passwd);
			}				
	
			vncEncryptBytes(challenge, passwd);

			/* Lose the plain-text password from memory */
			for (int i=0; i< (int) strlen(passwd); i++) {
				passwd[i] = '\0';
			}
			
			WriteExact((char *) challenge, CHALLENGESIZE);
			ReadExact((char *) &authResult, 4);
			
			authResult = Swap32IfLE(authResult);
			
			switch (authResult) {
			case rfbVncAuthOK:
				vnclog.Print(0, _T("VNC authentication succeeded\n"));
				break;
			case rfbVncAuthFailed:
				vnclog.Print(0, _T("VNC authentication failed!\n"));
				throw AuthException("VNC authentication failed!");
			case rfbVncAuthTooMany:
				vnclog.Print(0, _T("VNC authentication failed - too many tries!\n"));
				throw WarningException(
					"VNC authentication failed - too many tries!");
			default:
				vnclog.Print(0, _T("Unknown VNC authentication result: %d\n"),
					(int)authResult);
				throw ErrorException("Unknown VNC authentication result!");
			}
			break;
		}
		
	default:
		vnclog.Print(0, _T("Unknown authentication scheme from RFB server: %d\n"),
			(int)authScheme);
		throw ErrorException("Unknown authentication scheme!");
    }
}

void ClientConnection::SendClientInit()
{
    rfbClientInitMsg ci;
	ci.shared = m_opts.m_Shared;

    WriteExact((char *)&ci, sz_rfbClientInitMsg);
}

void ClientConnection::ReadServerInit()
{
    ReadExact((char *)&m_si, sz_rfbServerInitMsg);
	
    m_si.framebufferWidth = Swap16IfLE(m_si.framebufferWidth);
    m_si.framebufferHeight = Swap16IfLE(m_si.framebufferHeight);
    m_si.format.redMax = Swap16IfLE(m_si.format.redMax);
    m_si.format.greenMax = Swap16IfLE(m_si.format.greenMax);
    m_si.format.blueMax = Swap16IfLE(m_si.format.blueMax);
    m_si.nameLength = Swap32IfLE(m_si.nameLength);
	
    m_desktopName = new TCHAR[m_si.nameLength + 2];

#ifdef UNDER_CE
    char *deskNameBuf = new char[m_si.nameLength + 2];

	ReadString(deskNameBuf, m_si.nameLength);
    
	MultiByteToWideChar( CP_ACP,   MB_PRECOMPOSED, 
			     deskNameBuf, m_si.nameLength,
			     m_desktopName, m_si.nameLength+1);
    delete deskNameBuf;
#else
    ReadString(m_desktopName, m_si.nameLength);
#endif
    
	SetWindowText(m_hwnd, m_desktopName);	

	vnclog.Print(0, _T("Desktop name \"%s\"\n"),m_desktopName);
	vnclog.Print(1, _T("Geometry %d x %d depth %d\n"),
		m_si.framebufferWidth, m_si.framebufferHeight, m_si.format.depth );
	SetWindowText(m_hwnd, m_desktopName);	

	SizeWindow(true);
}

void ClientConnection::SizeWindow(bool centered)
{
	// Find how large the desktop work area is
	RECT workrect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &workrect, 0);
	int workwidth = workrect.right -  workrect.left;
	int workheight = workrect.bottom - workrect.top;
	vnclog.Print(2, _T("Screen work area is %d x %d\n"), workwidth, workheight);

	// Size the window.
	// Let's find out how big a window would be needed to display the
	// whole desktop (assuming no scrollbars).

	RECT fullwinrect;
	SetRect(&fullwinrect, 0, 0, m_si.framebufferWidth * m_opts.m_scale_num / m_opts.m_scale_den, 
								m_si.framebufferHeight* m_opts.m_scale_num / m_opts.m_scale_den);
	AdjustWindowRectEx(&fullwinrect,
					   GetWindowLong(m_hwnd, GWL_STYLE) & ~WS_VSCROLL & ~WS_HSCROLL,
					   FALSE, GetWindowLong(m_hwnd, GWL_EXSTYLE));
	m_fullwinwidth = fullwinrect.right - fullwinrect.left;
	m_fullwinheight = fullwinrect.bottom - fullwinrect.top;

	// Make the window size limited by the desktop size
	m_winwidth  = min(m_fullwinwidth,  workwidth);
	m_winheight = min(m_fullwinheight, workheight);

	int x = workrect.left + (workwidth - m_winwidth) / 2;
	int y = workrect.top + (workheight - m_winheight) / 2;
	if (!centered) {
		// Try to preserve current position if possible
		RECT tmprect;
		if (GetWindowRect(m_hwnd, &tmprect)) {
			x = tmprect.left;
			y = tmprect.top;
			if (x + m_winwidth > workrect.right)
				x = workrect.right - m_winwidth;
			if (y + m_winheight > workrect.bottom)
				y = workrect.bottom - m_winheight;
		}
	}

	SetWindowPos(m_hwnd, HWND_TOP, x, y, m_winwidth, m_winheight, SWP_SHOWWINDOW);
	SetForegroundWindow(m_hwnd);
}

// We keep a local copy of the whole screen.  This is not strictly necessary
// for VNC, but makes scrolling & deiconifying much smoother.

void ClientConnection::CreateLocalFramebuffer() {
	omni_mutex_lock l(m_bitmapdcMutex);

	// Remove old bitmap object if it already exists
	bool bitmapExisted = false;
	if (m_hBitmap != NULL) {
		DeleteObject(m_hBitmap);
		bitmapExisted = true;
	}

	// We create a bitmap which has the same pixel characteristics as
	// the local display, in the hope that blitting will be faster.
	
	TempDC hdc(m_hwnd);
	m_hBitmap = ::CreateCompatibleBitmap(hdc, m_si.framebufferWidth,
										 m_si.framebufferHeight);
	
	if (m_hBitmap == NULL)
		throw WarningException("Error creating local image of screen.");
	
	// Select this bitmap into the DC with an appropriate palette
	ObjectSelector b(m_hBitmapDC, m_hBitmap);
	PaletteSelector p(m_hBitmapDC, m_hPalette);
	
	// Put a "please wait" message up initially
	RECT rect;
	SetRect(&rect, 0,0, m_si.framebufferWidth, m_si.framebufferHeight);
	COLORREF bgcol = RGB(0xcc, 0xcc, 0xcc);
	FillSolidRect(&rect, bgcol);
	
	if (!bitmapExisted) {
		COLORREF oldbgcol  = SetBkColor(m_hBitmapDC, bgcol);
		COLORREF oldtxtcol = SetTextColor(m_hBitmapDC, RGB(0,0,64));
		rect.right = m_si.framebufferWidth / 2;
		rect.bottom = m_si.framebufferHeight / 2;
	
		DrawText (m_hBitmapDC, _T("Please wait - initial screen loading"), -1, &rect,
				  DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		SetBkColor(m_hBitmapDC, oldbgcol);
		SetTextColor(m_hBitmapDC, oldtxtcol);
	}
	
	InvalidateRect(m_hwnd, NULL, FALSE);
}

void ClientConnection::SetupPixelFormat() {
	// Have we requested a reduction to 8-bit?
    if (m_opts.m_Use8Bit) {		
      
		vnclog.Print(2, _T("Requesting 8-bit truecolour\n"));  
		m_myFormat = vnc8bitFormat;
    
		// We don't support colormaps so we'll ask the server to convert
    } else if (!m_si.format.trueColour) {
        
        // We'll just request a standard 16-bit truecolor
        vnclog.Print(2, _T("Requesting 16-bit truecolour\n"));
        m_myFormat = vnc16bitFormat;
        
    } else {

		// Normally we just use the sever's format suggestion
		m_myFormat = m_si.format;

		// It's silly requesting more bits than our current display has, but
		// in fact it doesn't usually amount to much on the network.
		// Windows doesn't support 8-bit truecolour.
		// If our display is palette-based, we want more than 8 bit anyway,
		// unless we're going to start doing palette stuff at the server.
		// So the main use would be a 24-bit true-colour desktop being viewed
		// on a 16-bit true-colour display, and unless you have lots of images
		// and hence lots of raw-encoded stuff, the size of the pixel is not
		// going to make much difference.
		//   We therefore don't bother with any restrictions, but here's the
		// start of the code if we wanted to do it.

		if (false) {
		
			// Get a DC for the root window
			TempDC hrootdc(NULL);
			int localBitsPerPixel = GetDeviceCaps(hrootdc, BITSPIXEL);
			int localRasterCaps	  = GetDeviceCaps(hrootdc, RASTERCAPS);
			vnclog.Print(2, _T("Memory DC has depth of %d and %s pallete-based.\n"), 
				localBitsPerPixel, (localRasterCaps & RC_PALETTE) ? "is" : "is not");
			
			// If we're using truecolor, and the server has more bits than we do
			if ( (localBitsPerPixel > m_myFormat.depth) && 
				! (localRasterCaps & RC_PALETTE)) {
				m_myFormat.depth = localBitsPerPixel;

				// create a bitmap compatible with the current display
				// call GetDIBits twice to get the colour info.
				// set colour masks and shifts
				
			}
		}
	}

	// The endian will be set before sending
}

void ClientConnection::SetFormatAndEncodings()
{
	// Set pixel format to myFormat

	rfbSetPixelFormatMsg spf;

	spf.type = rfbSetPixelFormat;
	spf.format = m_myFormat;
	spf.format.redMax = Swap16IfLE(spf.format.redMax);
	spf.format.greenMax = Swap16IfLE(spf.format.greenMax);
	spf.format.blueMax = Swap16IfLE(spf.format.blueMax);
	spf.format.bigEndian = 0;

	WriteExact((char *)&spf, sz_rfbSetPixelFormatMsg);

	// The number of bytes required to hold at least one pixel.
	m_minPixelBytes = (m_myFormat.bitsPerPixel + 7) >> 3;

	// Set encodings
	char buf[sz_rfbSetEncodingsMsg + MAX_ENCODINGS * 4];
	rfbSetEncodingsMsg *se = (rfbSetEncodingsMsg *)buf;
	CARD32 *encs = (CARD32 *)(&buf[sz_rfbSetEncodingsMsg]);
	int len = 0;

	se->type = rfbSetEncodings;
	se->nEncodings = 0;

	bool useCompressLevel = false;
	int i;

	// Put the preferred encoding first, and change it if the
	// preferred encoding is not actually usable.
	for (i = LASTENCODING; i >= rfbEncodingRaw; i--)
	{
		if (m_opts.m_PreferredEncoding == i) {
			if (m_opts.m_UseEnc[i]) {
				encs[se->nEncodings++] = Swap32IfLE(i);
				if ( i == rfbEncodingZlib ||
					 i == rfbEncodingTight ||
					 i == rfbEncodingZlibHex ) {
					useCompressLevel = true;
				}
			} else {
				m_opts.m_PreferredEncoding--;
			}
		}
	}

	// Now we go through and put in all the other encodings in order.
	// We do rather assume that the most recent encoding is the most
	// desirable!
	for (i = LASTENCODING; i >= rfbEncodingRaw; i--)
	{
		if ( (m_opts.m_PreferredEncoding != i) &&
			 (m_opts.m_UseEnc[i]))
		{
			encs[se->nEncodings++] = Swap32IfLE(i);
			if ( i == rfbEncodingZlib ||
				 i == rfbEncodingTight ||
				 i == rfbEncodingZlibHex ) {
				useCompressLevel = true;
			}
		}
	}

	// Request desired compression level if applicable
	if ( useCompressLevel && m_opts.m_useCompressLevel &&
		 m_opts.m_compressLevel >= 0 &&
		 m_opts.m_compressLevel <= 9) {
		encs[se->nEncodings++] = Swap32IfLE( rfbEncodingCompressLevel0 +
											 m_opts.m_compressLevel );
	}

	// Request cursor shape updates if enabled by user
	if (m_opts.m_requestShapeUpdates) {
		encs[se->nEncodings++] = Swap32IfLE(rfbEncodingXCursor);
		encs[se->nEncodings++] = Swap32IfLE(rfbEncodingRichCursor);
		if (!m_opts.m_ignoreShapeUpdates)
			encs[se->nEncodings++] = Swap32IfLE(rfbEncodingPointerPos);
	}

	// Request JPEG quality level if JPEG compression was enabled by user
	if ( m_opts.m_enableJpegCompression &&
		 m_opts.m_jpegQualityLevel >= 0 &&
		 m_opts.m_jpegQualityLevel <= 9) {
		encs[se->nEncodings++] = Swap32IfLE( rfbEncodingQualityLevel0 +
											 m_opts.m_jpegQualityLevel );
	}

	// Notify the server that we support LastRect and NewFBSize encodings
	encs[se->nEncodings++] = Swap32IfLE(rfbEncodingLastRect);
	encs[se->nEncodings++] = Swap32IfLE(rfbEncodingNewFBSize);

	len = sz_rfbSetEncodingsMsg + se->nEncodings * 4;

	se->nEncodings = Swap16IfLE(se->nEncodings);

	WriteExact((char *) buf, len);
}

// Closing down the connection.
// Close the socket, kill the thread.
void ClientConnection::KillThread()
{
	m_bKillThread = true;
	m_running = false;

	if (m_sock != INVALID_SOCKET) {
		shutdown(m_sock, SD_BOTH);
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
	}
}

// Get the RFB options from another connection.
void ClientConnection::CopyOptions(ClientConnection *source)
{
	this->m_opts = source->m_opts;
}

ClientConnection::~ClientConnection()
{
	if (m_hwnd != 0)
		DestroyWindow(m_hwnd);

	if (m_sock != INVALID_SOCKET) {
		shutdown(m_sock, SD_BOTH);
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
	}

	if (m_desktopName != NULL) delete [] m_desktopName;
	delete [] m_netbuf;
	DeleteDC(m_hBitmapDC);
	if (m_hBitmap != NULL)
		DeleteObject(m_hBitmap);
	if (m_hPalette != NULL)
		DeleteObject(m_hPalette);
	
	m_pApp->DeregisterConnection(this);
}

// You can specify a dx & dy outside the limits; the return value will
// tell you whether it actually scrolled.
bool ClientConnection::ScrollScreen(int dx, int dy) 
{
	dx = max(dx, -m_hScrollPos);
	//dx = min(dx, m_hScrollMax-(m_cliwidth-1)-m_hScrollPos);
	dx = min(dx, m_hScrollMax-(m_cliwidth)-m_hScrollPos);
	dy = max(dy, -m_vScrollPos);
	//dy = min(dy, m_vScrollMax-(m_cliheight-1)-m_vScrollPos);
	dy = min(dy, m_vScrollMax-(m_cliheight)-m_vScrollPos);
	if (dx || dy) {
		m_hScrollPos += dx;
		m_vScrollPos += dy;
		RECT clirect;
		GetClientRect(m_hwnd, &clirect);
		ScrollWindowEx(m_hwnd, -dx, -dy, NULL, &clirect, NULL, NULL,  SW_INVALIDATE);
		UpdateScrollbars();
		UpdateWindow(m_hwnd);
		return true;
	}
	return false;
}

// Process windows messages

LRESULT CALLBACK ClientConnection::WndProc(HWND hwnd, UINT iMsg, 
					   WPARAM wParam, LPARAM lParam) {
	
	// This is a static method, so we don't know which instantiation we're 
	// dealing with.  But we've stored a 'pseudo-this' in the window data.
	ClientConnection *_this = (ClientConnection *) GetWindowLong(hwnd, GWL_USERDATA);

	switch (iMsg) {

	case WM_CREATE:
		return 0;

	case WM_REGIONUPDATED:
		_this->DoBlit();
		_this->SendAppropriateFramebufferUpdateRequest();
		return 0;

	case WM_PAINT:
		_this->DoBlit();
		return 0;

	case WM_TIMER:
	  if (wParam == _this->m_emulate3ButtonsTimer)
	    {
	      _this->SubProcessPointerEvent( 
					    _this->m_emulateButtonPressedX,
					    _this->m_emulateButtonPressedY,
					    _this->m_emulateKeyFlags);
	      KillTimer(_this->m_hwnd, _this->m_emulate3ButtonsTimer);
	      _this->m_waitingOnEmulateTimer = false;
	    }
	  return 0;
 
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_MOUSEWHEEL:
		{
			if (!_this->m_running)
				return 0;
			if (GetFocus() != hwnd)
				return 0;

			POINT coords;
			coords.x = LOWORD(lParam);
			coords.y = HIWORD(lParam);

			if (iMsg == WM_MOUSEWHEEL) {
				// Convert coordinates to position in our client area,
				// make sure the pointer is inside the client area.
				if ( WindowFromPoint(coords) != hwnd ||
					 !ScreenToClient(hwnd, &coords) ||
					 coords.x < 0 || coords.y < 0 ||
					 coords.x >= _this->m_cliwidth ||
					 coords.y >= _this->m_cliheight ) {
					return 0;
				}
			} else {
				// Make sure the high-order word in wParam is zero.
				wParam = MAKEWPARAM(LOWORD(wParam), 0);
			}

			if (_this->InFullScreenMode()) {
				if (_this->BumpScroll(coords.x, coords.y))
					return 0;
			}
			if ( _this->m_opts.m_ViewOnly)
				return 0;

			_this->ProcessPointerEvent(coords.x, coords.y, wParam, iMsg);
			return 0;
		}

	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		{
			if (!_this->m_running) return 0;
			if ( _this->m_opts.m_ViewOnly) return 0;
            _this->ProcessKeyEvent((int) wParam, (DWORD) lParam);
			return 0;
		}

	case WM_CHAR:
	case WM_SYSCHAR:
#ifdef UNDER_CE
        {
            int key = wParam;
            vnclog.Print(4,_T("CHAR msg : %02x\n"), key);
            // Control keys which are in the Keymap table will already
            // have been handled.
            if (key == 0x0D  ||  // return
                key == 0x20 ||   // space
                key == 0x08)     // backspace
                return 0;

            if (key < 32) key += 64;  // map ctrl-keys onto alphabet
            if (key > 32 && key < 127) {
                _this->SendKeyEvent(wParam & 0xff, true);
                _this->SendKeyEvent(wParam & 0xff, false);
            }
            return 0;
        }
#endif
	case WM_DEADCHAR:
	case WM_SYSDEADCHAR:
	  return 0;

	case WM_SETFOCUS:
		if (_this->InFullScreenMode())
			SetWindowPos(hwnd, HWND_TOPMOST, 0,0,100,100, SWP_NOMOVE | SWP_NOSIZE);
		return 0;
	// Cacnel modifiers when we lose focus
	case WM_KILLFOCUS:
		{
			if (!_this->m_running) return 0;
			if (_this->InFullScreenMode()) {
				// We must top being topmost, but we want to choose our
				// position carefully.
				HWND foreground = GetForegroundWindow();
				HWND hwndafter = NULL;
				if ((foreground == NULL) || 
					(GetWindowLong(foreground, GWL_EXSTYLE) & WS_EX_TOPMOST)) {
					hwndafter = HWND_NOTOPMOST;
				} else {
					hwndafter = GetNextWindow(foreground, GW_HWNDNEXT); 
				}

				SetWindowPos(hwnd, hwndafter, 0,0,100,100, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			}
			vnclog.Print(6, _T("Losing focus - cancelling modifiers\n"));
			_this->SendKeyEvent(XK_Alt_L,     false);
			_this->SendKeyEvent(XK_Control_L, false);
			_this->SendKeyEvent(XK_Shift_L,   false);
			_this->SendKeyEvent(XK_Alt_R,     false);
			_this->SendKeyEvent(XK_Control_R, false);
			_this->SendKeyEvent(XK_Shift_R,   false);
			return 0;
		}
	case WM_CLOSE:
		{

			// Close the worker thread as well
			_this->KillThread();
			DestroyWindow(hwnd);
			return 0;
		}

	case WM_DESTROY:
		{
#ifndef UNDER_CE
			// Remove us from the clipboard viewer chain
			BOOL res = ChangeClipboardChain( hwnd, _this->m_hwndNextViewer);
#endif
			if (_this->m_waitingOnEmulateTimer)
			  {
			    KillTimer(_this->m_hwnd, _this->m_emulate3ButtonsTimer);
			    _this->m_waitingOnEmulateTimer = false;
			  }
  			
			_this->m_hwnd = 0;
			// We are currently in the main thread.
			// The worker thread should be about to finish if
			// it hasn't already. Wait for it.
			try {
				void *p;
				_this->join(&p);  // After joining, _this is no longer valid
			} catch (omni_thread_invalid& e) {
				// The thread probably hasn't been started yet,
			}
			
			return 0;
		}
		
	case WM_WINDOWPOSCHANGED:
	case WM_SIZE:
		{
			// Calculate window dimensions
			RECT rect;
			GetWindowRect(hwnd, &rect);
			// update these for the record
			_this->m_winwidth = rect.right - rect.left;
			_this->m_winheight = rect.bottom - rect.top;

			// If the current window size would be large enough to hold the
			// whole screen without scrollbars, or if we're full-screen,
			// we turn them off.  Under CE, the scroll bars are unchangeable.

			#ifndef UNDER_CE
			if (_this->InFullScreenMode() ||
				_this->m_winwidth  >= _this->m_fullwinwidth  &&
				_this->m_winheight >= _this->m_fullwinheight ) {
				ShowScrollBar(hwnd, SB_HORZ, FALSE);
				ShowScrollBar(hwnd, SB_VERT, FALSE);
			} else {
				ShowScrollBar(hwnd, SB_HORZ, TRUE);
				ShowScrollBar(hwnd, SB_VERT, TRUE);
			}
			#endif

            // Update these for the record
			// And consider that in full-screen mode the window
			// is actually bigger than the remote screen.
			GetClientRect(hwnd, &rect);
			_this->m_cliwidth = min( (int)(rect.right - rect.left),
									 (int)_this->m_si.framebufferWidth );
			_this->m_cliheight = min( (int)(rect.bottom - rect.top),
									 (int)_this->m_si.framebufferHeight );

			_this->m_hScrollMax = _this->m_si.framebufferWidth * _this->m_opts.m_scale_num / _this->m_opts.m_scale_den;
			_this->m_vScrollMax = _this->m_si.framebufferHeight* _this->m_opts.m_scale_num / _this->m_opts.m_scale_den;
            
			int newhpos, newvpos;
			newhpos = max(0, min(_this->m_hScrollPos, 
								 _this->m_hScrollMax - max(_this->m_cliwidth, 0)));
			newvpos = max(0, min(_this->m_vScrollPos, 
				                 _this->m_vScrollMax - max(_this->m_cliheight, 0)));

			ScrollWindowEx(hwnd, _this->m_hScrollPos-newhpos, _this->m_vScrollPos-newvpos,
				NULL, &rect, NULL, NULL,  SW_INVALIDATE);
			
			_this->m_hScrollPos = newhpos;
			_this->m_vScrollPos = newvpos;
           	_this->UpdateScrollbars();

			return 0;
		}

	case WM_HSCROLL:
		{
			int dx = 0;
			int pos = HIWORD(wParam);
			switch (LOWORD(wParam)) {
			case SB_LINEUP:
				dx = -2; break;
			case SB_LINEDOWN:
				dx = 2; break;
			case SB_PAGEUP:
				dx = _this->m_cliwidth * -1/4; break;
			case SB_PAGEDOWN:
				dx = _this->m_cliwidth * 1/4; break;
			case SB_THUMBPOSITION:
				dx = pos - _this->m_hScrollPos;
			case SB_THUMBTRACK:
				dx = pos - _this->m_hScrollPos;
			}
			_this->ScrollScreen(dx,0);
			return 0;
		}

	case WM_VSCROLL:
		{
			int dy = 0;
			int pos = HIWORD(wParam);
			switch (LOWORD(wParam)) {
			case SB_LINEUP:
				dy = -2; break;
			case SB_LINEDOWN:
				dy = 2; break;
			case SB_PAGEUP:
				dy = _this->m_cliheight * -1/4; break;
			case SB_PAGEDOWN:
				dy = _this->m_cliheight * 1/4; break;
			case SB_THUMBPOSITION:
				dy = pos - _this->m_vScrollPos;
			case SB_THUMBTRACK:
				dy = pos - _this->m_vScrollPos;
			}
			_this->ScrollScreen(0,dy);
			return 0;
		}

    case WM_QUERYNEWPALETTE:
        {
			TempDC hDC(hwnd);
			
			// Select and realize hPalette
			PaletteSelector p(hDC, _this->m_hPalette);
			InvalidateRect(hwnd, NULL, FALSE);
			UpdateWindow(hwnd);

			return TRUE;
        }

	case WM_PALETTECHANGED:
		// If this application did not change the palette, select
		// and realize this application's palette
		if ((HWND) wParam != hwnd)
		{
			// Need the window's DC for SelectPalette/RealizePalette
			TempDC hDC(hwnd);
			PaletteSelector p(hDC, _this->m_hPalette);
			// When updating the colors for an inactive window,
			// UpdateColors can be called because it is faster than
			// redrawing the client area (even though the results are
			// not as good)
				#ifndef UNDER_CE
				UpdateColors(hDC);
				#else
				InvalidateRect(hwnd, NULL, FALSE);
				UpdateWindow(hwnd);
				#endif

		}
        break;

#ifndef UNDER_CE
	case WM_SIZING:
		{
			// Don't allow sizing larger than framebuffer
			RECT *lprc = (LPRECT) lParam;
			switch (wParam) {
			case WMSZ_RIGHT: 
			case WMSZ_TOPRIGHT:
			case WMSZ_BOTTOMRIGHT:
				lprc->right = min(lprc->right, lprc->left + _this->m_fullwinwidth+1);
				break;
			case WMSZ_LEFT:
			case WMSZ_TOPLEFT:
			case WMSZ_BOTTOMLEFT:
				lprc->left = max(lprc->left, lprc->right - _this->m_fullwinwidth);
				break;
			}
			
			switch (wParam) {
			case WMSZ_TOP:
			case WMSZ_TOPLEFT:
			case WMSZ_TOPRIGHT:
				lprc->top = max(lprc->top, lprc->bottom - _this->m_fullwinheight);
				break;
			case WMSZ_BOTTOM:
			case WMSZ_BOTTOMLEFT:
			case WMSZ_BOTTOMRIGHT:
				lprc->bottom = min(lprc->bottom, lprc->top + _this->m_fullwinheight);
				break;
			}

			return 0;
		}
	
	case WM_SETCURSOR:
		{
			// if we have the focus, let the cursor change as normal
			if (GetFocus() == hwnd) 
				break;

			// if not, set to default system cursor
			SetCursor( LoadCursor(NULL, IDC_ARROW));
			return 0;
		}

	case WM_SYSCOMMAND:
		{
			switch (LOWORD(wParam)) {
			case SC_MINIMIZE:
				_this->SetDormant(true);
				break;
			case SC_RESTORE:
				_this->SetDormant(false);
				break;
            case ID_NEWCONN:
				_this->m_pApp->NewConnection();
				return 0;
            case ID_CONN_SAVE_AS:
				_this->SaveConnection();
				return 0;
			case IDC_OPTIONBUTTON: 
				{
					int prev_scale_num = _this->m_opts.m_scale_num;
					int prev_scale_den = _this->m_opts.m_scale_den;

					if (_this->m_opts.DoDialog(true)) {
						_this->m_pendingFormatChange = true;

						if (prev_scale_num != _this->m_opts.m_scale_num ||
							prev_scale_den != _this->m_opts.m_scale_den) {
							// Resize the window if scaling factors were changed
							_this->SizeWindow(false);
							InvalidateRect(hwnd, NULL, TRUE);
							// Make the window correspond to the requested state
							_this->RealiseFullScreenMode(true);
						}
					}
				}
				return 0;
			case IDD_APP_ABOUT:
				ShowAboutBox();
				return 0;
			case ID_CONN_ABOUT:
				_this->ShowConnInfo();
				return 0;
			case ID_FULLSCREEN: 
				// Toggle full screen mode
				_this->SetFullScreenMode(!_this->InFullScreenMode());
				return 0;
			case ID_REQUEST_REFRESH: 
				// Request a full-screen update
				_this->SendFullFramebufferUpdateRequest();
				return 0;

			case ID_CONN_CTLALTDEL:
				_this->SendKeyEvent(XK_Control_L, true);
				_this->SendKeyEvent(XK_Alt_L,     true);
				_this->SendKeyEvent(XK_Delete,    true);
				_this->SendKeyEvent(XK_Delete,    false);
				_this->SendKeyEvent(XK_Alt_L,     false);
				_this->SendKeyEvent(XK_Control_L, false);
				return 0;
            case ID_CONN_CTLDOWN:
                _this->SendKeyEvent(XK_Control_L, true);
                return 0;
            case ID_CONN_CTLUP:
				_this->SendKeyEvent(XK_Control_L, false);
				return 0;
			case ID_CONN_ALTDOWN:
                _this->SendKeyEvent(XK_Alt_L, true);
				return 0;
			case ID_CONN_ALTUP:
                _this->SendKeyEvent(XK_Alt_L, false);
                return 0;
			case ID_CLOSEDAEMON:
				if (MessageBox(NULL, _T("Are you sure you want to exit?"), 
						_T("Closing VNCviewer"), 
						MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES)
					PostQuitMessage(0);
				return 0;
			}
			break;
		}

	case WM_DRAWCLIPBOARD:
		_this->ProcessLocalClipboardChange();
		return 0;

	case WM_CHANGECBCHAIN:
		{
			// The clipboard chain is changing
			HWND hWndRemove = (HWND) wParam;     // handle of window being removed 
			HWND hWndNext = (HWND) lParam;       // handle of next window in chain 
			// If next window is closing, update our pointer.
			if (hWndRemove == _this->m_hwndNextViewer)  
				_this->m_hwndNextViewer = hWndNext;  
			// Otherwise, pass the message to the next link.  
			else if (_this->m_hwndNextViewer != NULL) 
				::SendMessage(_this->m_hwndNextViewer, WM_CHANGECBCHAIN, 
				(WPARAM) hWndRemove,  (LPARAM) hWndNext );  
			return 0;

		}
#endif
	}

	return DefWindowProc(hwnd, iMsg, wParam, lParam);
	
	// We know about an unused variable here.
#pragma warning(disable : 4101)
}

#pragma warning(default : 4101)

// ProcessPointerEvent handles the delicate case of emulating 3 buttons
// on a two button mouse, then passes events off to SubProcessPointerEvent.

void
ClientConnection::ProcessPointerEvent(int x, int y, DWORD keyflags, UINT msg) 
{
	if (m_opts.m_Emul3Buttons) {
		// XXX To be done:
		// If this is a left or right press, the user may be 
		// about to press the other button to emulate a middle press.
		// We need to start a timer, and if it expires without any
		// further presses, then we send the button press. 
		// If a press of the other button, or any release, comes in
		// before timer has expired, we cancel timer & take different action.
	  if (m_waitingOnEmulateTimer)
	    {
	      if (msg == WM_LBUTTONUP || msg == WM_RBUTTONUP ||
		  abs(x - m_emulateButtonPressedX) > m_opts.m_Emul3Fuzz ||
		  abs(y - m_emulateButtonPressedY) > m_opts.m_Emul3Fuzz)
		{
		  // if button released or we moved too far then cancel.
		  // First let the remote know where the button was down
		  SubProcessPointerEvent(
					 m_emulateButtonPressedX, 
					 m_emulateButtonPressedY, 
					 m_emulateKeyFlags);
		  // Then tell it where we are now
		  SubProcessPointerEvent(x, y, keyflags);
		}
	      else if (
		       (msg == WM_LBUTTONDOWN && (m_emulateKeyFlags & MK_RBUTTON))
		       || (msg == WM_RBUTTONDOWN && (m_emulateKeyFlags & MK_LBUTTON)))
		{
		  // Triggered an emulate; remove left and right buttons, put
		  // in middle one.
		  DWORD emulatekeys = keyflags & ~(MK_LBUTTON|MK_RBUTTON);
		  emulatekeys |= MK_MBUTTON;
		  SubProcessPointerEvent(x, y, emulatekeys);
		  
		  m_emulatingMiddleButton = true;
		}
	      else
		{
		  // handle movement normally & don't kill timer.
		  // just remove the pressed button from the mask.
		  DWORD keymask = m_emulateKeyFlags & (MK_LBUTTON|MK_RBUTTON);
		  DWORD emulatekeys = keyflags & ~keymask;
		  SubProcessPointerEvent(x, y, emulatekeys);
		  return;
		}
	      
	      // if we reached here, we don't need the timer anymore.
	      KillTimer(m_hwnd, m_emulate3ButtonsTimer);
	      m_waitingOnEmulateTimer = false;
	    }
	  else if (m_emulatingMiddleButton)
	    {
	      if ((keyflags & MK_LBUTTON) == 0 && (keyflags & MK_RBUTTON) == 0)
		{
		  // We finish emulation only when both buttons come back up.
		  m_emulatingMiddleButton = false;
		  SubProcessPointerEvent(x, y, keyflags);
		}
	      else
		{
		  // keep emulating.
		  DWORD emulatekeys = keyflags & ~(MK_LBUTTON|MK_RBUTTON);
		  emulatekeys |= MK_MBUTTON;
		  SubProcessPointerEvent(x, y, emulatekeys);
		}
	    }
	  else
	    {
	      // Start considering emulation if we've pressed a button
	      // and the other isn't pressed.
	      if ( (msg == WM_LBUTTONDOWN && !(keyflags & MK_RBUTTON))
		   || (msg == WM_RBUTTONDOWN && !(keyflags & MK_LBUTTON)))
		{
		  // Start timer for emulation.
		  m_emulate3ButtonsTimer = 
		    SetTimer(
			     m_hwnd, 
			     IDT_EMULATE3BUTTONSTIMER, 
			     m_opts.m_Emul3Timeout, 
			     NULL);
		  
		  if (!m_emulate3ButtonsTimer)
		    {
		      vnclog.Print(0, _T("Failed to create timer for emulating 3 buttons"));
		      PostMessage(m_hwnd, WM_CLOSE, 0, 0);
		      return;
		    }
		  
		  m_waitingOnEmulateTimer = true;
		  
		  // Note that we don't send the event here; we're batching it for
		  // later.
		  m_emulateKeyFlags = keyflags;
		  m_emulateButtonPressedX = x;
		  m_emulateButtonPressedY = y;
		}
	      else
		{
		  // just send event noramlly
		  SubProcessPointerEvent(x, y, keyflags);
		}
	    }
 	}
	else
	  {
	    SubProcessPointerEvent(x, y, keyflags);
	  }
}

// SubProcessPointerEvent takes windows positions and flags and converts 
// them into VNC ones.

inline void
ClientConnection::SubProcessPointerEvent(int x, int y, DWORD keyflags)
{
	int mask;
  
	if (m_opts.m_SwapMouse) {
		mask = ( ((keyflags & MK_LBUTTON) ? rfbButton1Mask : 0) |
				 ((keyflags & MK_MBUTTON) ? rfbButton3Mask : 0) |
				 ((keyflags & MK_RBUTTON) ? rfbButton2Mask : 0) );
	} else {
		mask = ( ((keyflags & MK_LBUTTON) ? rfbButton1Mask : 0) |
				 ((keyflags & MK_MBUTTON) ? rfbButton2Mask : 0) |
				 ((keyflags & MK_RBUTTON) ? rfbButton3Mask : 0) );
	}

	if ((short)HIWORD(keyflags) > 0) {
		mask |= rfbButton4Mask;
	} else if ((short)HIWORD(keyflags) < 0) {
		mask |= rfbButton5Mask;
	}
	
	try {
		int x_scaled =
			(x + m_hScrollPos) * m_opts.m_scale_den / m_opts.m_scale_num;
		int y_scaled =
			(y + m_vScrollPos) * m_opts.m_scale_den / m_opts.m_scale_num;

		SendPointerEvent(x_scaled, y_scaled, mask);

		if ((short)HIWORD(keyflags) != 0) {
			// Immediately send a "button-up" after mouse wheel event.
			mask &= !(rfbButton4Mask | rfbButton5Mask);
			SendPointerEvent(x_scaled, y_scaled, mask);
		}
	} catch (Exception &e) {
		e.Report();
		PostMessage(m_hwnd, WM_CLOSE, 0, 0);
	}
}

//
// SendPointerEvent.
//

inline void
ClientConnection::SendPointerEvent(int x, int y, int buttonMask)
{
    rfbPointerEventMsg pe;

    pe.type = rfbPointerEvent;
    pe.buttonMask = buttonMask;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
	SoftCursorMove(x, y);
    pe.x = Swap16IfLE(x);
    pe.y = Swap16IfLE(y);
	WriteExact((char *)&pe, sz_rfbPointerEventMsg);
}

//
// ProcessKeyEvent
//
// Normally a single Windows key event will map onto a single RFB
// key message, but this is not always the case.  Much of the stuff
// here is to handle AltGr (=Ctrl-Alt) on international keyboards.
// Example cases:
//
//    We want Ctrl-F to be sent as:
//      Ctrl-Down, F-Down, F-Up, Ctrl-Up.
//    because there is no keysym for ctrl-f, and because the ctrl
//    will already have been sent by the time we get the F.
//
//    On German keyboards, @ is produced using AltGr-Q, which is
//    Ctrl-Alt-Q.  But @ is a valid keysym in its own right, and when
//    a German user types this combination, he doesn't mean Ctrl-@.
//    So for this we will send, in total:
//
//      Ctrl-Down, Alt-Down,   
//                 (when we get the AltGr pressed)
//
//      Alt-Up, Ctrl-Up, @-Down, Ctrl-Down, Alt-Down 
//                 (when we discover that this is @ being pressed)
//
//      Alt-Up, Ctrl-Up, @-Up, Ctrl-Down, Alt-Down
//                 (when we discover that this is @ being released)
//
//      Alt-Up, Ctrl-Up
//                 (when the AltGr is released)

inline void ClientConnection::ProcessKeyEvent(int virtkey, DWORD keyData)
{
    bool down = ((keyData & 0x80000000l) == 0);

    // if virtkey found in mapping table, send X equivalent
    // else
    //   try to convert directly to ascii
    //   if result is in range supported by X keysyms,
    //      raise any modifiers, send it, then restore mods
    //   else
    //      calculate what the ascii would be without mods
    //      send that

#ifdef _DEBUG
#ifdef UNDER_CE
	char *keyname="";
#else
    char keyname[32];
    if (GetKeyNameText(  keyData,keyname, 31)) {
        vnclog.Print(4, _T("Process key: %s (keyData %04x): "), keyname, keyData);
    };
#endif
#endif

	try {
		KeyActionSpec kas = m_keymap.PCtoX(virtkey, keyData);    
		
		if (kas.releaseModifiers & KEYMAP_LCONTROL) {
			SendKeyEvent(XK_Control_L, false );
			vnclog.Print(5, _T("fake L Ctrl raised\n"));
		}
		if (kas.releaseModifiers & KEYMAP_LALT) {
			SendKeyEvent(XK_Alt_L, false );
			vnclog.Print(5, _T("fake L Alt raised\n"));
		}
		if (kas.releaseModifiers & KEYMAP_RCONTROL) {
			SendKeyEvent(XK_Control_R, false );
			vnclog.Print(5, _T("fake R Ctrl raised\n"));
		}
		if (kas.releaseModifiers & KEYMAP_RALT) {
			SendKeyEvent(XK_Alt_R, false );
			vnclog.Print(5, _T("fake R Alt raised\n"));
		}
		
		for (int i = 0; kas.keycodes[i] != XK_VoidSymbol && i < MaxKeysPerKey; i++) {
			SendKeyEvent(kas.keycodes[i], down );
			vnclog.Print(4, _T("Sent keysym %04x (%s)\n"), 
				kas.keycodes[i], down ? _T("press") : _T("release"));
		}
		
		if (kas.releaseModifiers & KEYMAP_RALT) {
			SendKeyEvent(XK_Alt_R, true );
			vnclog.Print(5, _T("fake R Alt pressed\n"));
		}
		if (kas.releaseModifiers & KEYMAP_RCONTROL) {
			SendKeyEvent(XK_Control_R, true );
			vnclog.Print(5, _T("fake R Ctrl pressed\n"));
		}
		if (kas.releaseModifiers & KEYMAP_LALT) {
			SendKeyEvent(XK_Alt_L, false );
			vnclog.Print(5, _T("fake L Alt pressed\n"));
		}
		if (kas.releaseModifiers & KEYMAP_LCONTROL) {
			SendKeyEvent(XK_Control_L, false );
			vnclog.Print(5, _T("fake L Ctrl pressed\n"));
		}
	} catch (Exception &e) {
		e.Report();
		PostMessage(m_hwnd, WM_CLOSE, 0, 0);
	}

}

//
// SendKeyEvent
//

inline void
ClientConnection::SendKeyEvent(CARD32 key, bool down)
{
    rfbKeyEventMsg ke;

    ke.type = rfbKeyEvent;
    ke.down = down ? 1 : 0;
    ke.key = Swap32IfLE(key);
    WriteExact((char *)&ke, sz_rfbKeyEventMsg);
    vnclog.Print(6, _T("SendKeyEvent: key = x%04x status = %s\n"), key, 
        down ? _T("down") : _T("up"));
}

#ifndef UNDER_CE
//
// SendClientCutText
//

void ClientConnection::SendClientCutText(char *str, int len)
{
    rfbClientCutTextMsg cct;

    cct.type = rfbClientCutText;
    cct.length = Swap32IfLE(len);
    WriteExact((char *)&cct, sz_rfbClientCutTextMsg);
	WriteExact(str, len);
	vnclog.Print(6, _T("Sent %d bytes of clipboard\n"), len);
}
#endif

// Copy any updated areas from the bitmap onto the screen.

inline void ClientConnection::DoBlit() 
{
	if (m_hBitmap == NULL) return;
	if (!m_running) return;
				
	// No other threads can use bitmap DC
	omni_mutex_lock l(m_bitmapdcMutex);

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(m_hwnd, &ps);

	// Select and realize hPalette
	PaletteSelector p(hdc, m_hPalette);
	ObjectSelector b(m_hBitmapDC, m_hBitmap);
			
	if (m_opts.m_delay) {
		// Display the area to be updated for debugging purposes
		COLORREF oldbgcol = SetBkColor(hdc, RGB(0,0,0));
		::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &ps.rcPaint, NULL, 0, NULL);
		SetBkColor(hdc,oldbgcol);
		::Sleep(m_pApp->m_options.m_delay);
	}
	
	if (m_opts.m_scaling) {
		int n = m_opts.m_scale_num;
		int d = m_opts.m_scale_den;
		
		// We're about to do some scaling on these values in the StretchBlt
		// We want to make sure that they divide nicely by n so we round them
		// down and up appropriately.
		ps.rcPaint.left =   ((ps.rcPaint.left   + m_hScrollPos) / n * n)         - m_hScrollPos;
		ps.rcPaint.right =  ((ps.rcPaint.right  + m_hScrollPos + n - 1) / n * n) - m_hScrollPos;
		ps.rcPaint.top =    ((ps.rcPaint.top    + m_vScrollPos) / n * n)         - m_vScrollPos;
		ps.rcPaint.bottom = ((ps.rcPaint.bottom + m_vScrollPos + n - 1) / n * n) - m_vScrollPos;
		
		// This is supposed to give better results.  I think my driver ignores it?
		SetStretchBltMode(hdc, HALFTONE);
		// The docs say that you should call SetBrushOrgEx after SetStretchBltMode, 
		// but not what the arguments should be.
		SetBrushOrgEx(hdc, 0,0, NULL);
		
		if (!StretchBlt(
			hdc, 
			ps.rcPaint.left, 
			ps.rcPaint.top, 
			ps.rcPaint.right-ps.rcPaint.left, 
			ps.rcPaint.bottom-ps.rcPaint.top, 
			m_hBitmapDC, 
			(ps.rcPaint.left+m_hScrollPos)     * d / n, 
			(ps.rcPaint.top+m_vScrollPos)      * d / n,
			(ps.rcPaint.right-ps.rcPaint.left) * d / n, 
			(ps.rcPaint.bottom-ps.rcPaint.top) * d / n, 
			SRCCOPY)) 
		{
			vnclog.Print(0, _T("Blit error %d\n"), GetLastError());
			// throw ErrorException("Error in blit!\n");
		};
	} else {
		if (!BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, 
			ps.rcPaint.right-ps.rcPaint.left, ps.rcPaint.bottom-ps.rcPaint.top, 
			m_hBitmapDC, ps.rcPaint.left+m_hScrollPos, ps.rcPaint.top+m_vScrollPos, SRCCOPY)) 
		{
			vnclog.Print(0, _T("Blit error %d\n"), GetLastError());
			// throw ErrorException("Error in blit!\n");
		}
	}
	
	EndPaint(m_hwnd, &ps);
}

inline void ClientConnection::UpdateScrollbars() 
{
	// We don't update the actual scrollbar info in full-screen mode
	// because it causes them to flicker.
	bool setInfo = !InFullScreenMode();

	SCROLLINFO scri;
	scri.cbSize = sizeof(scri);
	scri.fMask = SIF_ALL;
	scri.nMin = 0;
	scri.nMax = m_hScrollMax; 
	scri.nPage= m_cliwidth;
	scri.nPos = m_hScrollPos; 
	
	if (setInfo) 
		SetScrollInfo(m_hwnd, SB_HORZ, &scri, TRUE);
	
	scri.cbSize = sizeof(scri);
	scri.fMask = SIF_ALL;
	scri.nMin = 0;
	scri.nMax = m_vScrollMax;     
	scri.nPage= m_cliheight;
	scri.nPos = m_vScrollPos; 
	
	if (setInfo) 
		SetScrollInfo(m_hwnd, SB_VERT, &scri, TRUE);
}


void ClientConnection::ShowConnInfo()
{
	TCHAR buf[2048];
#ifndef UNDER_CE
	char kbdname[9];
	GetKeyboardLayoutName(kbdname);
#else
	TCHAR *kbdname = _T("(n/a)");
#endif
	_stprintf(
		buf,
		_T("Connected to: %s:\n\r")
		_T("Host: %s port: %d\n\r\n\r")
		_T("Desktop geometry: %d x %d x %d\n\r")
		_T("Using depth: %d\n\r")
		_T("Current protocol version: %d.%d\n\r\n\r")
		_T("Current keyboard name: %s\n\r"),
		m_desktopName, m_host, m_port,
		m_si.framebufferWidth, m_si.framebufferHeight, m_si.format.depth,
		m_myFormat.depth,
		m_majorVersion, m_minorVersion,
		kbdname);
	MessageBox(NULL, buf, _T("VNC connection info"), MB_ICONINFORMATION | MB_OK);
}

// ********************************************************************
//  Methods after this point are generally called by the worker thread.
//  They finish the initialisation, then chiefly read data from the server.
// ********************************************************************


void* ClientConnection::run_undetached(void* arg) {

	vnclog.Print(9,_T("Update-processing thread started\n"));

	m_threadStarted = true;

	try {

		SendFullFramebufferUpdateRequest();

		RealiseFullScreenMode(false);

		m_running = true;
		UpdateWindow(m_hwnd);
		
		while (!m_bKillThread) {
			
			// Look at the type of the message, but leave it in the buffer 
			CARD8 msgType;
			{
			  omni_mutex_lock l(m_readMutex);  // we need this if we're not using ReadExact
			  int bytes = recv(m_sock, (char *) &msgType, 1, MSG_PEEK);
			  if (bytes == 0) {
			    vnclog.Print(0, _T("Connection closed\n") );
			    throw WarningException(_T("Connection closed"));
			  }
			  if (bytes < 0) {
			    vnclog.Print(3, _T("Socket error reading message: %d\n"), WSAGetLastError() );
			    throw WarningException("Error while waiting for server message");
			  }
			}
				
			switch (msgType) {
			case rfbFramebufferUpdate:
				ReadScreenUpdate();
				break;
			case rfbSetColourMapEntries:
		        vnclog.Print(3, _T("rfbSetColourMapEntries read but not supported\n") );
				throw WarningException("Unhandled SetColormap message type received!\n");
				break;
			case rfbBell:
				ReadBell();
				break;
			case rfbServerCutText:
				ReadServerCutText();
				break;

			// - VERSION 3.5 and above messages
			case rfbEnableExtensionRequest:
				ReadEnableExtension();
				break;
			default:
				if (msgType >= rfbExtensionData) {
					ReadExtensionData();
				} else {
	                vnclog.Print(3, _T("Unknown message type x%02x\n"), msgType );
					throw WarningException("Unhandled message type received!\n");
				}
				break;

			/*
			default:
                vnclog.Print(3, _T("Unknown message type x%02x\n"), msgType );
				throw WarningException("Unhandled message type received!\n");
			*/
			}

		}
        
        vnclog.Print(4, _T("Update-processing thread finishing\n") );

	} catch (WarningException &e) {
		m_running = false;
		PostMessage(m_hwnd, WM_CLOSE, 0, 0);
		if (!m_bKillThread) {
			e.Report();
		}
	} catch (QuietException &e) {
		m_running = false;
		e.Report();
		PostMessage(m_hwnd, WM_CLOSE, 0, 0);
	} 
	return this;
}


//
// Requesting screen updates from the server
//

inline void
ClientConnection::SendFramebufferUpdateRequest(int x, int y, int w, int h, bool incremental)
{
    rfbFramebufferUpdateRequestMsg fur;

    fur.type = rfbFramebufferUpdateRequest;
    fur.incremental = incremental ? 1 : 0;
    fur.x = Swap16IfLE(x);
    fur.y = Swap16IfLE(y);
    fur.w = Swap16IfLE(w);
    fur.h = Swap16IfLE(h);

	vnclog.Print(10, _T("Request %s update\n"), incremental ? _T("incremental") : _T("full"));
    WriteExact((char *)&fur, sz_rfbFramebufferUpdateRequestMsg);
}

inline void ClientConnection::SendIncrementalFramebufferUpdateRequest()
{
    SendFramebufferUpdateRequest(0, 0, m_si.framebufferWidth,
					m_si.framebufferHeight, true);
}

inline void ClientConnection::SendFullFramebufferUpdateRequest()
{
    SendFramebufferUpdateRequest(0, 0, m_si.framebufferWidth,
					m_si.framebufferHeight, false);
}


void ClientConnection::SendAppropriateFramebufferUpdateRequest()
{
	if (m_pendingFormatChange) {
		vnclog.Print(3, _T("Requesting new pixel format\n") );
		rfbPixelFormat oldFormat = m_myFormat;
		SetupPixelFormat();
		SoftCursorFree();
		SetFormatAndEncodings();
		m_pendingFormatChange = false;
		// If the pixel format has changed, request whole screen
		if (!PF_EQ(m_myFormat, oldFormat)) {
			SendFullFramebufferUpdateRequest();	
		} else {
			SendIncrementalFramebufferUpdateRequest();
		}
	} else {
		if (!m_dormant)
			SendIncrementalFramebufferUpdateRequest();
	}
}


// A ScreenUpdate message has been received

void ClientConnection::ReadScreenUpdate() {

	rfbFramebufferUpdateMsg sut;
	ReadExact((char *) &sut, sz_rfbFramebufferUpdateMsg);
    sut.nRects = Swap16IfLE(sut.nRects);
	if (sut.nRects == 0) return;
	
	for (int i=0; i < sut.nRects; i++) {

		rfbFramebufferUpdateRectHeader surh;
		ReadExact((char *) &surh, sz_rfbFramebufferUpdateRectHeader);

		surh.encoding = Swap32IfLE(surh.encoding);
		surh.r.x = Swap16IfLE(surh.r.x);
		surh.r.y = Swap16IfLE(surh.r.y);
		surh.r.w = Swap16IfLE(surh.r.w);
		surh.r.h = Swap16IfLE(surh.r.h);

		if (surh.encoding == rfbEncodingLastRect)
			break;
		if (surh.encoding == rfbEncodingNewFBSize) {
			ReadNewFBSize(&surh);
			break;
		}

		if ( surh.encoding == rfbEncodingXCursor ||
			 surh.encoding == rfbEncodingRichCursor ) {
			ReadCursorShape(&surh);
			continue;
		}

		if (surh.encoding == rfbEncodingPointerPos) {
			ReadCursorPos(&surh);
			continue;
		}

		// If *Cursor encoding is used, we should prevent collisions
		// between framebuffer updates and cursor drawing operations.
		SoftCursorLockArea(surh.r.x, surh.r.y, surh.r.w, surh.r.h);

		switch (surh.encoding) {
		case rfbEncodingRaw:
			ReadRawRect(&surh);
			break;
		case rfbEncodingCopyRect:
			ReadCopyRect(&surh);
			break;
		case rfbEncodingRRE:
			ReadRRERect(&surh);
			break;
		case rfbEncodingCoRRE:
			ReadCoRRERect(&surh);
			break;
		case rfbEncodingHextile:
			ReadHextileRect(&surh);
			break;
		case rfbEncodingZlib:
			ReadZlibRect(&surh);
			break;
		case rfbEncodingTight:
			ReadTightRect(&surh);
			break;
		case rfbEncodingZlibHex:
			ReadZlibHexRect(&surh);
			break;
		default:
			vnclog.Print(0, _T("Unknown encoding %d - not supported!\n"), surh.encoding);
			break;
		}

		// Tell the system to update a screen rectangle. Note that
		// InvalidateScreenRect member function knows about scaling.
		RECT rect;
		SetRect(&rect, surh.r.x, surh.r.y,
				surh.r.x + surh.r.w, surh.r.y + surh.r.h);
		InvalidateScreenRect(&rect);

		// Now we may discard "soft cursor locks".
		SoftCursorUnlockScreen();
	}	

	// Inform the other thread that an update is needed.
	PostMessage(m_hwnd, WM_REGIONUPDATED, NULL, NULL);
}	

void ClientConnection::SetDormant(bool newstate)
{
	vnclog.Print(5, _T("%s dormant mode\n"), newstate ? _T("Entering") : _T("Leaving"));
	m_dormant = newstate;
	if (!m_dormant)
		SendIncrementalFramebufferUpdateRequest();
}

// The server has copied some text to the clipboard - put it 
// in the local clipboard too.

void ClientConnection::ReadServerCutText() {
	rfbServerCutTextMsg sctm;
	vnclog.Print(6, _T("Read remote clipboard change\n"));
	ReadExact((char *) &sctm, sz_rfbServerCutTextMsg);
	int len = Swap32IfLE(sctm.length);
	
	CheckBufferSize(len);
	if (len == 0) {
		m_netbuf[0] = '\0';
	} else {
		ReadString(m_netbuf, len);
	}
	UpdateLocalClipboard(m_netbuf, len);
}


void ClientConnection::ReadBell() {
	rfbBellMsg bm;
	ReadExact((char *) &bm, sz_rfbBellMsg);

	#ifdef UNDER_CE
	MessageBeep( MB_OK );
	#else

	if (! ::PlaySound("VNCViewerBell", NULL, 
		SND_APPLICATION | SND_ALIAS | SND_NODEFAULT | SND_ASYNC) ) {
		::Beep(440, 125);
	}
	#endif
	if (m_opts.m_DeiconifyOnBell) {
		if (IsIconic(m_hwnd)) {
			SetDormant(false);
			ShowWindow(m_hwnd, SW_SHOWNORMAL);
		}
	}
	vnclog.Print(6, _T("Bell!\n"));
}

void ClientConnection::ReadEnableExtension() {
	rfbEnableExtensionRequestMsg eer;
	ReadExact((char *) &eer, sz_rfbEnableExtensionRequestMsg);

	const CARD32 length = Swap32IfLE(eer.length);
	CheckBufferSize(length);
	if (length != 0) {
		ReadExact(m_netbuf, length);
	}

	/* *** Enable the extension here */
}

void ClientConnection::ReadExtensionData() {
	rfbExtensionDataMsg ed;
	ReadExact((char *) &ed, sz_rfbExtensionDataMsg);

	const CARD32 length = Swap32IfLE(ed.length);
	CheckBufferSize(length);
	if (length != 0) {
		ReadExact(m_netbuf, length);
	}

	/* *** Pass the data to the extension here */
}


// General utilities -------------------------------------------------

// Reads the number of bytes specified into the buffer given

inline void ClientConnection::ReadExact(char *inbuf, int wanted)
{
	omni_mutex_lock l(m_readMutex);

	int offset = 0;
    vnclog.Print(10, _T("  reading %d bytes\n"), wanted);
	
	while (wanted > 0) {

		int bytes = recv(m_sock, inbuf+offset, wanted, 0);
		if (bytes == 0) throw WarningException("Connection closed.");
		if (bytes == SOCKET_ERROR) {
			int err = ::GetLastError();
			vnclog.Print(1, _T("Socket error while reading %d\n"), err);
			m_running = false;
			throw WarningException("ReadExact: Socket error while reading.");
		}
		wanted -= bytes;
		offset += bytes;

	}
}

// Read the number of bytes and return them zero terminated in the buffer 
inline void ClientConnection::ReadString(char *buf, int length)
{
	if (length > 0)
		ReadExact(buf, length);
	buf[length] = '\0';
    vnclog.Print(10, _T("Read a %d-byte string\n"), length);
}


// Sends the number of bytes specified from the buffer
inline void ClientConnection::WriteExact(char *buf, int bytes)
{
	if (bytes == 0) return;
	omni_mutex_lock l(m_writeMutex);
	vnclog.Print(10, _T("  writing %d bytes\n"), bytes);

	int i = 0;
    int j;

    while (i < bytes) {

		j = send(m_sock, buf+i, bytes-i, 0);
		if (j == SOCKET_ERROR || j==0) {
			LPVOID lpMsgBuf;
			int err = ::GetLastError();
			FormatMessage(     
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM |     
				FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
				err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf, 0, NULL ); // Process any inserts in lpMsgBuf.
			vnclog.Print(1, _T("Socket error %d: %s\n"), err, lpMsgBuf);
			LocalFree( lpMsgBuf );
			m_running = false;

			throw WarningException("WriteExact: Socket error while writing.");
		}
		i += j;
    }
}

// Makes sure netbuf is at least as big as the specified size.
// Note that netbuf itself may change as a result of this call.
// Throws an exception on failure.
void ClientConnection::CheckBufferSize(int bufsize)
{
	if (m_netbufsize > bufsize) return;

	omni_mutex_lock l(m_bufferMutex);

	char *newbuf = new char[bufsize+256];;
	if (newbuf == NULL) {
		throw ErrorException("Insufficient memory to allocate network buffer.");
	}

	// Only if we're successful...

	if (m_netbuf != NULL)
		delete [] m_netbuf;
	m_netbuf = newbuf;
	m_netbufsize=bufsize + 256;
	vnclog.Print(4, _T("bufsize expanded to %d\n"), m_netbufsize);
}

// Makes sure zlibbuf is at least as big as the specified size.
// Note that zlibbuf itself may change as a result of this call.
// Throws an exception on failure.
void ClientConnection::CheckZlibBufferSize(int bufsize)
{
	unsigned char *newbuf;

	if (m_zlibbufsize > bufsize) return;

	// omni_mutex_lock l(m_bufferMutex);

	newbuf = (unsigned char *)new char[bufsize+256];
	if (newbuf == NULL) {
		throw ErrorException("Insufficient memory to allocate zlib buffer.");
	}

	// Only if we're successful...

	if (m_zlibbuf != NULL)
		delete [] m_zlibbuf;
	m_zlibbuf = newbuf;
	m_zlibbufsize=bufsize + 256;
	vnclog.Print(4, _T("zlibbufsize expanded to %d\n"), m_zlibbufsize);


}

//
// Invalidate a screen rectangle respecting scaling set by user.
//

void ClientConnection::InvalidateScreenRect(const RECT *pRect) {
	RECT rect;

	// If we're scaling, we transform the coordinates of the rectangle
	// received into the corresponding window coords, and invalidate
	// *that* region.

	if (m_opts.m_scaling) {
		// First, we adjust coords to avoid rounding down when scaling.
		int n = m_opts.m_scale_num;
		int d = m_opts.m_scale_den;
		int left   = (pRect->left / d) * d;
		int top    = (pRect->top  / d) * d;
		int right  = (pRect->right  + d - 1) / d * d; // round up
		int bottom = (pRect->bottom + d - 1) / d * d; // round up

		// Then we scale the rectangle, which should now give whole numbers.
		rect.left   = (left   * n / d) - m_hScrollPos;
		rect.top    = (top    * n / d) - m_vScrollPos;
		rect.right  = (right  * n / d) - m_hScrollPos;
		rect.bottom = (bottom * n / d) - m_vScrollPos;
	} else {
		rect.left   = pRect->left   - m_hScrollPos;
		rect.top    = pRect->top    - m_vScrollPos;
		rect.right  = pRect->right  - m_hScrollPos;
		rect.bottom = pRect->bottom - m_vScrollPos;
	}
	InvalidateRect(m_hwnd, &rect, FALSE);
}

//
// Processing NewFBSize pseudo-rectangle. Create new framebuffer of
// the size specified in pfburh->r.w and pfburh->r.h, and change the
// window size correspondingly.
//

void ClientConnection::ReadNewFBSize(rfbFramebufferUpdateRectHeader *pfburh)
{
	m_si.framebufferWidth = pfburh->r.w;
	m_si.framebufferHeight = pfburh->r.h;

	CreateLocalFramebuffer();

	SizeWindow(false);
	RealiseFullScreenMode(true);
}

