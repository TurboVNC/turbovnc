//  Copyright (C) 2009-2013, 2015-2018, 2020 D. R. Commander.
//                                           All Rights Reserved.
//  Copyright 2009 Pierre Ossman for Cendio AB
//  Copyright (C) 2005-2008 Sun Microsystems, Inc. All Rights Reserved.
//  Copyright (C) 2004 Landmark Graphics Corporation. All Rights Reserved.
//  Copyright (C) 2003-2006 Constantin Kaplinsky. All Rights Reserved.
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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
//  USA.


// Many thanks to Randy Brown <rgb@inven.com> for providing the 3-button
// emulation code.

// This is the main source for a ClientConnection object.
// It handles almost everything to do with a connection to a server.
// The decoding of specific rectangle encodings is done in separate files.

#include "stdhdrs.h"

#include "vncviewer.h"

#include "omnithread.h"

#include "ClientConnection.h"
#include "SessionDialog.h"
#include "LoginAuthDialog.h"
#include "AboutBox.h"
#include "LowLevelHook.h"
#include "FileTransfer.h"
#include "commctrl.h"
#include "Exception.h"
extern "C" {
#include "vncauth.h"
#include "d3des.h"
}
#include "screenTypes.h"
#include "safestr.h"

#define INITIALNETBUFSIZE 4096
#define MAX_ENCODINGS 20
#define VWR_WND_CLASS_NAME "VNCviewer"


#define PF_EQ(x, y)  \
  ((x.bitsPerPixel == y.bitsPerPixel) && (x.depth == y.depth) &&  \
   ((x.bigEndian == y.bigEndian) || (x.bitsPerPixel == 8)) &&  \
   (x.trueColour == y.trueColour) &&  \
   (!x.trueColour ||  \
    ((x.redMax == y.redMax) && (x.redShift == y.redShift) &&  \
     (x.greenMax == y.greenMax) && (x.greenShift == y.greenShift) &&  \
     (x.blueMax == y.blueMax) && (x.blueShift == y.blueShift))))

const rfbPixelFormat vnc8bitFormat =
  { 8, 8, 0, 1, 7, 7, 3, 0, 3, 6, 0, 0 };
const rfbPixelFormat vnc16bitFormat =
  { 16, 16, 0, 1, 63, 31, 31, 0, 6, 11, 0, 0 };
const rfbPixelFormat vnc24bitFormat =
  { 32, 24, 0, 1, 255, 255, 255, 16, 8, 0, 0, 0 };



// *************************************************************************
//  A Client connection involves two threads - the main one, which sets up
//  connections and processes window messages and inputs, and a
//  client-specific one, which receives, decodes and draws output data
//  from the remote server.
//  This first section contains bits that are generally called by the main
//  program thread.
// *************************************************************************

#define BENCHMARKINIT  \
  tDecode(pApp->tDecode), tBlit(pApp->tBlit), tRead(pApp->tRead),  \
  decodePixels(pApp->decodePixels), blitPixels(pApp->blitPixels),  \
  decodeRect(pApp->decodeRect), blits(pApp->blits), updates(pApp->updates)


ClientConnection::ClientConnection(VNCviewerApp *pApp) : BENCHMARKINIT
{
  Init(pApp);
}


ClientConnection::ClientConnection(VNCviewerApp *pApp, SOCKET sock) :
  BENCHMARKINIT
{
  char hostname[NI_MAXHOST];
  Init(pApp);
  m_sock = sock;
  m_serverInitiated = true;
  struct sockaddr_storage svraddr;
  int sasize = sizeof(svraddr);
  if (getpeername(sock, (struct sockaddr *)&svraddr,
                  &sasize) != SOCKET_ERROR &&
      getnameinfo((struct sockaddr *)&svraddr, sasize, hostname, NI_MAXHOST,
                  NULL, 0, NI_NUMERICHOST) == 0) {
    SPRINTF(m_host, "%s", hostname);
    m_host[255] = 0;
    if (svraddr.ss_family == AF_INET6)
      m_port = ((sockaddr_in6 *)&svraddr)->sin6_port;
    else
      m_port = ((sockaddr_in *)&svraddr)->sin_port;
  } else {
    STRCPY(m_host, "(unknown)");
    m_port = 0;
  }
}


ClientConnection::ClientConnection(VNCviewerApp *pApp, LPTSTR host, int port) :
  BENCHMARKINIT
{
  Init(pApp);
  STRCPY(m_host, host);
  m_port = port;
}


void ClientConnection::Init(VNCviewerApp *pApp)
{
  m_hwnd = NULL;
  m_hwnd1 = NULL;
  m_hwndscroll = NULL;
  m_hToolbar = NULL;
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
  m_passwdSet = false;

  m_connDlg = NULL;

  m_enableFileTransfers = false;
  m_fileTransferDialogShown = false;
  m_pFileTransfer = new FileTransfer(this, m_pApp);

  // We take the initial conn options from the application defaults
  m_opts = m_pApp->m_options;

  m_sock = INVALID_SOCKET;
  m_bKillThread = false;
  m_threadStarted = true;
  m_running = false;
  m_pendingFormatChange = false;
  m_pendingEncodingChange = false;

  m_hScrollPos = 0;  m_vScrollPos = 0;

  m_waitingOnEmulateTimer = false;
  m_emulatingMiddleButton = false;

  for (int i = 0; i < 4; i++)
    m_tightZlibStreamActive[i] = false;

  prevCursorSet = false;
  rcCursorX = 0;
  rcCursorY = 0;

  // Create a buffer for various network operations
  CheckBufferSize(INITIALNETBUFSIZE);

  m_pApp->RegisterConnection(this);

  memset(&fb, 0, sizeof(fb));
  if ((j = tjInitDecompress()) == NULL)
    throw ErrorException(tjGetErrorStr());

  node = list = tail = NULL;

  supportsCU = false;
  continuousUpdates = false;
  supportsFence = false;
  supportsSyncFence = false;
  pendingSyncFence = false;

  m_supportsSetDesktopSize = false;
  m_waitingOnResizeTimer = false;
  m_checkLayout = false;
  m_serverXinerama = false;

  m_firstUpdate = true;

  tDecode = tBlit = tRead = 0.0;
  decodePixels = blitPixels = 0;
  decodeRect = blits = updates = 0;

  savedRect.left = savedRect.top = savedRect.right = savedRect.bottom = -1;

  m_hctx = 0;
  m_wacomButtonMask = 0;
}


void ClientConnection::InitCapabilities()
{
  // Supported authentication methods
  m_authCaps.Add(rfbAuthNone, rfbStandardVendor, sig_rfbAuthNone,
                 "No authentication");
  m_authCaps.Add(rfbAuthVNC, rfbStandardVendor, sig_rfbAuthVNC,
                 "Standard VNC authentication");
  m_authCaps.Add(rfbAuthUnixLogin, rfbTightVncVendor, sig_rfbAuthUnixLogin,
                 "Unix login authentication");

  // Known server->client message types
  m_serverMsgCaps.Add(rfbFileListData, rfbTightVncVendor,
                      sig_rfbFileListData, "File list data");
  m_serverMsgCaps.Add(rfbFileDownloadData, rfbTightVncVendor,
                      sig_rfbFileDownloadData, "File download data");
  m_serverMsgCaps.Add(rfbFileUploadCancel, rfbTightVncVendor,
                      sig_rfbFileUploadCancel, "File upload cancel request");
  m_serverMsgCaps.Add(rfbFileDownloadFailed, rfbTightVncVendor,
                      sig_rfbFileDownloadFailed,
                      "File download failure notification");

  // Known client->server message types
  m_clientMsgCaps.Add(rfbFileListRequest, rfbTightVncVendor,
                      sig_rfbFileListRequest, "File list request");
  m_clientMsgCaps.Add(rfbFileDownloadRequest, rfbTightVncVendor,
                      sig_rfbFileDownloadRequest, "File download request");
  m_clientMsgCaps.Add(rfbFileUploadRequest, rfbTightVncVendor,
                      sig_rfbFileUploadRequest, "File upload request");
  m_clientMsgCaps.Add(rfbFileUploadData, rfbTightVncVendor,
                      sig_rfbFileUploadData, "File upload data");
  m_clientMsgCaps.Add(rfbFileDownloadCancel, rfbTightVncVendor,
                      sig_rfbFileDownloadCancel,
                      "File download cancel request");
  m_clientMsgCaps.Add(rfbFileUploadFailed, rfbTightVncVendor,
                      sig_rfbFileUploadFailed,
                      "File upload failure notification");

  // Supported encoding types
  m_encodingCaps.Add(rfbEncodingCopyRect, rfbStandardVendor,
                     sig_rfbEncodingCopyRect, "Standard CopyRect encoding");
  m_encodingCaps.Add(rfbEncodingHextile, rfbStandardVendor,
                     sig_rfbEncodingHextile, "Standard Hextile encoding");
  m_encodingCaps.Add(rfbEncodingTight, rfbTightVncVendor,
                     sig_rfbEncodingTight,
                     "Tight encoding by Constantin Kaplinsky");

  // Supported pseudo-encoding types
  m_encodingCaps.Add(rfbEncodingCompressLevel0, rfbTightVncVendor,
                     sig_rfbEncodingCompressLevel0, "Compression level");
  m_encodingCaps.Add(rfbEncodingQualityLevel0, rfbTightVncVendor,
                     sig_rfbEncodingQualityLevel0, "JPEG quality level");
  m_encodingCaps.Add(rfbEncodingXCursor, rfbTightVncVendor,
                     sig_rfbEncodingXCursor, "X-style cursor shape update");
  m_encodingCaps.Add(rfbEncodingRichCursor, rfbTightVncVendor,
                     sig_rfbEncodingRichCursor,
                     "Rich-color cursor shape update");
  m_encodingCaps.Add(rfbEncodingPointerPos, rfbTightVncVendor,
                     sig_rfbEncodingPointerPos, "Pointer position update");
  m_encodingCaps.Add(rfbEncodingLastRect, rfbTightVncVendor,
                     sig_rfbEncodingLastRect, "LastRect protocol extension");
  m_encodingCaps.Add(rfbEncodingNewFBSize, rfbTightVncVendor,
                     sig_rfbEncodingNewFBSize, "Framebuffer size change");
  m_encodingCaps.Add(rfbEncodingFineQualityLevel0, rfbTurboVncVendor,
                     sig_rfbEncodingFineQualityLevel0,
                     "TurboJPEG fine-grained quality level");
  m_encodingCaps.Add(rfbEncodingSubsamp1X, rfbTurboVncVendor,
                     sig_rfbEncodingSubsamp1X, "TurboJPEG subsampling level");
}


// Run() creates the connection if necessary, does the initial negotiations,
// and then starts the thread that does the output (update) processing.
// If Run throws an Exception, the caller must delete the ClientConnection
// object.

void ClientConnection::Run()
{
  bool benchmark = (m_pApp->m_options.m_benchFile != NULL);

  if (!benchmark) {
    // Get the host name and port if we haven't got it
    if (m_port == -1) {
      GetConnectDetails();
    } else {
      if (m_pApp->m_options.m_listening)
        m_opts.LoadOpt(m_opts.m_display, KEY_VNCVIEWER_HISTORY);
    }

    if (strlen((const char *)m_pApp->m_options.m_encPasswd) > 0) {
      memcpy(m_encPasswd, m_pApp->m_options.m_encPasswd, 8);
      memset(m_pApp->m_options.m_encPasswd, 0, 8);
      m_passwdSet = true;
    }

    // Show the "Connecting..." dialog box
    m_connDlg = new ConnectingDialog(m_pApp->m_instance, m_opts.m_display);

    // Connect if we're not already connected
    if (m_sock == INVALID_SOCKET)
      Connect();

    SetSocketOptions();

    NegotiateProtocolVersion();

    PerformAuthentication();
  } else
    m_tightVncProtocol = false;

  // Set up windows, etc.
  CreateDisplay();

  if (!benchmark) SendClientInit();
  ReadServerInit();

  // Only for protocol version 3.7t
  if (m_tightVncProtocol) {
    // Determine which protocol messages and encodings are supported.
    ReadInteractionCaps();
    // Enable file transfers only if the server supports that.
    m_enableFileTransfers = false;
    if (m_clientMsgCaps.IsEnabled(rfbFileListRequest) &&
        m_serverMsgCaps.IsEnabled(rfbFileListData))
      m_enableFileTransfers = true;
  }

  // Close the "Connecting..." dialog box if not closed yet.
  if (m_connDlg != NULL) {
    delete m_connDlg;
    m_connDlg = NULL;
  }

  EnableFullControlOptions();

  EnableZoomOptions();

  CreateLocalFramebuffer();

  SetupPixelFormat();

  m_pendingEncodingChange = true;

  hotkeys.Init(m_opts.m_FSAltEnter,
               m_opts.m_desktopSize.mode != SIZE_AUTO && !m_opts.m_FitWindow);

  // This starts the worker thread.
  // The rest of the processing continues in run_undetached.
  start_undetached();
}


static WNDCLASS wndclass;  // FIXME!

void ClientConnection::CreateDisplay()
{
  WNDCLASS wndclass;

  wndclass.style          = 0;
  wndclass.lpfnWndProc    = ClientConnection::Proc;
  wndclass.cbClsExtra     = 0;
  wndclass.cbWndExtra     = 0;
  wndclass.hInstance      = m_pApp->m_instance;
  wndclass.hIcon          = (HICON)LoadIcon(m_pApp->m_instance,
                                            MAKEINTRESOURCE(IDI_MAINICON));
  wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
  wndclass.hbrBackground  = (HBRUSH)GetSysColorBrush(COLOR_BTNFACE);
  wndclass.lpszMenuName   = (LPCTSTR)NULL;
  wndclass.lpszClassName  = VWR_WND_CLASS_NAME;

  RegisterClass(&wndclass);

  wndclass.style          = 0;
  wndclass.lpfnWndProc    = ClientConnection::ScrollProc;
  wndclass.cbClsExtra     = 0;
  wndclass.cbWndExtra     = 0;
  wndclass.hInstance      = m_pApp->m_instance;
  wndclass.hIcon          = NULL;
  wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
  wndclass.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);
  wndclass.lpszMenuName   = (LPCTSTR)NULL;
  wndclass.lpszClassName  = "ScrollClass";

  RegisterClass(&wndclass);

  wndclass.style          = 0;
  wndclass.lpfnWndProc    = ClientConnection::Proc;
  wndclass.cbClsExtra     = 0;
  wndclass.cbWndExtra     = 0;
  wndclass.hInstance      = m_pApp->m_instance;
  wndclass.hIcon          = NULL;
  switch (m_pApp->m_options.m_localCursor) {
    case NOCURSOR:
      wndclass.hCursor    = LoadCursor(m_pApp->m_instance,
                                       MAKEINTRESOURCE(IDC_NOCURSOR));
      break;
    case SMALLCURSOR:
      wndclass.hCursor    = LoadCursor(m_pApp->m_instance,
                                       MAKEINTRESOURCE(IDC_SMALLDOT));
      break;
    case NORMALCURSOR:
      wndclass.hCursor    = LoadCursor(NULL, IDC_ARROW);
      break;
    case DOTCURSOR:
    default:
      wndclass.hCursor    = LoadCursor(m_pApp->m_instance,
                                       MAKEINTRESOURCE(IDC_DOTCURSOR));
  }
  wndclass.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);
  wndclass.lpszMenuName   = (LPCTSTR)NULL;
  wndclass.lpszClassName  = "ChildClass";

  RegisterClass(&wndclass);

  m_hwnd1 = CreateWindow(VWR_WND_CLASS_NAME, "VNCviewer",
                         WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_SIZEBOX |
                           WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CLIPCHILDREN,
                         CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                         CW_USEDEFAULT, NULL, NULL, m_pApp->m_instance, NULL);
  SetWindowLongPtr(m_hwnd1, GWLP_USERDATA, (LONG_PTR)this);
  SetWindowLongPtr(m_hwnd1, GWLP_WNDPROC,
                   (LONG_PTR)ClientConnection::WndProc1);
  ShowWindow(m_hwnd1, SW_HIDE);

  m_hwndscroll = CreateWindow("ScrollClass", NULL,
                              WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
                                WS_BORDER,
                              CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                              CW_USEDEFAULT, m_hwnd1, NULL, m_pApp->m_instance,
                              NULL);
  SetWindowLongPtr(m_hwndscroll, GWLP_USERDATA, (LONG_PTR)this);
  ShowWindow(m_hwndscroll, SW_HIDE);

  // Create a memory DC, which we'll use for drawing to
  // the local framebuffer
  m_hBitmapDC = CreateCompatibleDC(NULL);

  // Set a suitable palette
  if (GetDeviceCaps(m_hBitmapDC, RASTERCAPS) & RC_PALETTE) {
    vnclog.Print(3, "Palette-based display - %d entries, %d reserved\n",
                 GetDeviceCaps(m_hBitmapDC, SIZEPALETTE),
                 GetDeviceCaps(m_hBitmapDC, NUMRESERVED));
    BYTE buf[sizeof(LOGPALETTE) + 216 * sizeof(PALETTEENTRY)];
    LOGPALETTE *plp = (LOGPALETTE *)buf;
    int pepos = 0;
    for (int r = 5; r >= 0; r--) {
      for (int g = 5; g >= 0; g--) {
        for (int b = 5; b >= 0; b--) {
          plp->palPalEntry[pepos].peRed    = r * 255 / 5;
          plp->palPalEntry[pepos].peGreen  = g * 255 / 5;
          plp->palPalEntry[pepos].peBlue   = b * 255 / 5;
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
  HMENU hsysmenu = GetSystemMenu(m_hwnd1, FALSE);
  bool save_item_flags = (m_serverInitiated) ? MF_GRAYED : 0;
  AppendMenu(hsysmenu, MF_SEPARATOR, NULL, NULL);
  AppendMenu(hsysmenu, MF_STRING, IDC_OPTIONBUTTON,
             "&Options...\tCtrl-Alt-Shift-O");
  AppendMenu(hsysmenu, MF_STRING, ID_CONN_ABOUT,
             "Connection &info...\tCtrl-Alt-Shift-I");
  AppendMenu(hsysmenu, MF_SEPARATOR, NULL, NULL);
  AppendMenu(hsysmenu, MF_STRING, ID_REQUEST_REFRESH,
             "Request screen &refresh\tCtrl-Alt-Shift-R");
  AppendMenu(hsysmenu, MF_STRING, ID_REQUEST_LOSSLESS_REFRESH,
             "Request &lossless refresh\tCtrl-Alt-Shift-L");
  AppendMenu(hsysmenu, MF_SEPARATOR, NULL, NULL);
  AppendMenu(hsysmenu, MF_STRING, ID_FULLSCREEN,
             "&Full screen\tCtrl-Alt-Shift-F");
  AppendMenu(hsysmenu, MF_STRING, ID_DEFAULT_WINDOW_SIZE,
             "Default window si&ze/position\tCtrl-Alt-Shift-Z");
  AppendMenu(hsysmenu, MF_STRING, ID_ZOOM_IN,
             "Zoom in\tCtrl-Alt-Shift +");
  AppendMenu(hsysmenu, MF_STRING, ID_ZOOM_OUT,
             "Zoom out\tCtrl-Alt-Shift -");
  AppendMenu(hsysmenu, MF_STRING, ID_ZOOM_100,
             "Zoom 100%\tCtrl-Alt-Shift-0");
  AppendMenu(hsysmenu, MF_STRING, ID_TOOLBAR,
             "Show &toolbar\tCtrl-Alt-Shift-T");
  AppendMenu(hsysmenu, MF_SEPARATOR, NULL, NULL);
  AppendMenu(hsysmenu, MF_STRING, ID_TOGGLE_VIEWONLY,
             "&View only\tCtrl-Alt-Shift-V");
  AppendMenu(hsysmenu, MF_STRING, ID_TOGGLE_GRAB,
             "&Grab keyboard\tCtrl-Alt-Shift-G");
  if (!m_opts.m_restricted) {
    AppendMenu(hsysmenu, MF_STRING, ID_CONN_CTLALTDEL,
               "Send Ctrl-Alt-&Del");
    AppendMenu(hsysmenu, MF_STRING, ID_CONN_CTLESC,
               "Send Ctrl-Esc");
  }
  if (m_opts.m_FSAltEnter)
    AppendMenu(hsysmenu, MF_STRING, ID_CONN_ALTENTER,
               "Send Alt-Enter");
  AppendMenu(hsysmenu, MF_STRING, ID_CONN_CTLDOWN,
             "Ctrl key down");
  AppendMenu(hsysmenu, MF_STRING, ID_CONN_ALTDOWN,
             "Alt key down");
  AppendMenu(hsysmenu, MF_SEPARATOR, NULL, NULL);
  AppendMenu(hsysmenu, MF_STRING, ID_TOGGLE_CLIPBOARD,
             "&Clipboard transfer\tCtrl-Alt-Shift-C");
  AppendMenu(hsysmenu, MF_SEPARATOR, NULL, NULL);
  AppendMenu(hsysmenu, MF_STRING | MF_GRAYED, IDD_FILETRANSFER,
             "Transf&er files...\tCtrl-Alt-Shift-E");
  AppendMenu(hsysmenu, MF_SEPARATOR, NULL, NULL);
  AppendMenu(hsysmenu, MF_STRING, ID_NEWCONN,
             "&New connection...\tCtrl-Alt-Shift-N");
  AppendMenu(hsysmenu, save_item_flags, ID_CONN_SAVE_AS,
             "&Save connection info as...\tCtrl-Alt-Shift-S");
  AppendMenu(hsysmenu, MF_SEPARATOR, NULL, NULL);
  AppendMenu(hsysmenu, MF_STRING, IDD_APP_ABOUT,
             "&About TurboVNC Viewer...");
  if (m_opts.m_listening) {
    AppendMenu(hsysmenu, MF_SEPARATOR, NULL, NULL);
    AppendMenu(hsysmenu, MF_STRING, ID_CLOSEDAEMON,
               "Close &listener");
  }
  DrawMenuBar(m_hwnd1);

  m_hToolbar = CreateToolbar();

  m_hwnd = CreateWindow("ChildClass", NULL, WS_CHILD | WS_CLIPSIBLINGS,
                        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                        CW_USEDEFAULT, m_hwndscroll, NULL, m_pApp->m_instance,
                        NULL);
  ImmAssociateContext(m_hwnd, NULL);
  m_opts.m_hWindow = m_hwnd;
  hotkeys.SetWindow(m_hwnd1);
  ShowWindow(m_hwnd, SW_HIDE);
  if (m_opts.m_GrabKeyboard == TVNC_ALWAYS)
    GrabKeyboard(true);
  if (m_pApp->m_wacom) {
    LOGCONTEXT lc;
    AXIS axisX, axisY;
    HCTX hctx;

    gpWTInfoA(WTI_DEFCONTEXT, 0, &lc);
    lc.lcOptions |= CXO_MESSAGES;
    lc.lcPktData = PACKETDATA;
    lc.lcPktMode = PACKETMODE;
    lc.lcMoveMask = PACKETDATA;
    lc.lcBtnUpMask = lc.lcBtnDnMask;

    gpWTInfoA(WTI_DEVICES, DVC_X, &axisX);
    gpWTInfoA(WTI_DEVICES, DVC_Y, &axisY);
    lc.lcInOrgX = 0;
    lc.lcInOrgY = 0;
    lc.lcInExtX = axisX.axMax;
    lc.lcInExtY = axisY.axMax;

    hctx = gpWTOpenA(m_hwnd, &lc, TRUE);
    if (!hctx)
      vnclog.Print(-1, "Could not open tablet context\n");
    else
      m_hctx = hctx;
  }

  SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)this);
  SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, (LONG_PTR)ClientConnection::WndProc);

  if (pApp->m_options.m_toolbar)
    CheckMenuItem(GetSystemMenu(m_hwnd1, FALSE), ID_TOOLBAR,
                  MF_BYCOMMAND | MF_CHECKED);
  CheckMenuItem(GetSystemMenu(m_hwnd1, FALSE), ID_TOGGLE_VIEWONLY,
                MF_BYCOMMAND | (pApp->m_options.m_ViewOnly ?
                                MF_CHECKED : MF_UNCHECKED));
  CheckMenuItem(GetSystemMenu(m_hwnd1, FALSE), ID_TOGGLE_CLIPBOARD,
                MF_BYCOMMAND | (pApp->m_options.m_DisableClipboard ?
                                MF_UNCHECKED : MF_CHECKED));

  SaveConnectionHistory();
  // record which client created this window

  // We want to know when the clipboard changes, so insert ourselves into the
  // viewer chain.  But doing this will cause us to be notified immediately of
  // the current state.   We don't want to send that.
  m_initialClipboardSeen = false;
  m_hwndNextViewer = SetClipboardViewer(m_hwnd);
}


HWND ClientConnection::CreateToolbar()
{
  const int MAX_TOOLBAR_BUTTONS = 20;
  TBBUTTON but[MAX_TOOLBAR_BUTTONS];
  memset(but, 0, sizeof(but));
  int i = 0;

  but[i].iBitmap    = 0;
  but[i].idCommand  = IDC_OPTIONBUTTON;
  but[i].fsState    = TBSTATE_ENABLED;
  but[i++].fsStyle  = TBSTYLE_BUTTON;

  but[i].iBitmap    = 1;
  but[i].idCommand  = ID_CONN_ABOUT;
  but[i].fsState    = TBSTATE_ENABLED;
  but[i++].fsStyle  = TBSTYLE_BUTTON;

  but[i++].fsStyle  = TBSTYLE_SEP;

  but[i].iBitmap    = 2;
  but[i].idCommand  = ID_FULLSCREEN;
  but[i].fsState    = TBSTATE_ENABLED;
  but[i++].fsStyle  = TBSTYLE_BUTTON;

  but[i].iBitmap    = 3;
  but[i].idCommand  = ID_REQUEST_REFRESH;
  but[i].fsState    = TBSTATE_ENABLED;
  but[i++].fsStyle  = TBSTYLE_BUTTON;

  but[i].iBitmap    = 4;
  but[i].idCommand  = ID_REQUEST_LOSSLESS_REFRESH;
  but[i].fsState    = TBSTATE_ENABLED;
  but[i++].fsStyle  = TBSTYLE_BUTTON;

  but[i++].fsStyle  = TBSTYLE_SEP;

  if (!m_opts.m_restricted) {
    but[i].iBitmap    = 5;
    but[i].idCommand  = ID_CONN_CTLALTDEL;
    but[i].fsState    = TBSTATE_ENABLED;
    but[i++].fsStyle  = TBSTYLE_BUTTON;

    but[i].iBitmap    = 6;
    but[i].idCommand  = ID_CONN_CTLESC;
    but[i].fsState    = TBSTATE_ENABLED;
    but[i++].fsStyle  = TBSTYLE_BUTTON;
  }

  but[i].iBitmap    = 7;
  but[i].idCommand  = ID_CONN_CTLDOWN;
  but[i].fsState    = TBSTATE_ENABLED;
  but[i++].fsStyle  = TBSTYLE_CHECK;

  but[i].iBitmap    = 8;
  but[i].idCommand  = ID_CONN_ALTDOWN;
  but[i].fsState    = TBSTATE_ENABLED;
  but[i++].fsStyle  = TBSTYLE_CHECK;

  but[i++].fsStyle  = TBSTYLE_SEP;

  but[i].iBitmap    = 9;
  but[i].idCommand  = IDD_FILETRANSFER;
  but[i].fsState    = TBSTATE_INDETERMINATE;
  but[i++].fsStyle  = TBSTYLE_BUTTON;

  but[i++].fsStyle  = TBSTYLE_SEP;

  but[i].iBitmap    = 10;
  but[i].idCommand  = ID_NEWCONN;
  but[i].fsState    = TBSTATE_ENABLED;
  but[i++].fsStyle  = TBSTYLE_BUTTON;

  but[i].iBitmap    = 11;
  but[i].idCommand  = ID_CONN_SAVE_AS;
  but[i].fsState    = TBSTATE_ENABLED;
  but[i++].fsStyle  = TBSTYLE_BUTTON;

  but[i].iBitmap    = 12;
  but[i].idCommand  = ID_DISCONNECT;
  but[i].fsState    = TBSTATE_ENABLED;
  but[i++].fsStyle  = TBSTYLE_BUTTON;

  int numButtons = i;
  assert(numButtons <= MAX_TOOLBAR_BUTTONS);

  HWND hwndToolbar = CreateToolbarEx(m_hwnd1,
                                     WS_CHILD | TBSTYLE_TOOLTIPS |
                                       WS_CLIPSIBLINGS | TBSTYLE_FLAT,
                                     ID_TOOLBAR, 12, m_pApp->m_instance,
                                     IDB_BITMAP1, but, numButtons, 0, 0, 0, 0,
                                     sizeof(TBBUTTON));

  if (hwndToolbar != NULL)
    SendMessage(hwndToolbar, TB_SETINDENT, 4, 0);

  return hwndToolbar;
}


void ClientConnection::SaveConnectionHistory()
{
  if (m_serverInitiated)
    return;

  // Create or open the registry key for connection history.
  HKEY hKey;
  LONG result = RegCreateKeyEx(HKEY_CURRENT_USER, KEY_VNCVIEWER_HISTORY, 0,
                               NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                               NULL, &hKey, NULL);
  if (result != ERROR_SUCCESS)
    return;

  // Determine maximum number of connections to remember.
  const int maxEntries = pApp->m_options.m_historyLimit;
  if (maxEntries > 1024)
    return;

  // Allocate memory for the list of connections, 256 chars an entry.
  const int entryBufferSize = 256;
  const int connListBufferSize = maxEntries * entryBufferSize;
  char *connListBuffer = new char[connListBufferSize];
  memset(connListBuffer, 0, connListBufferSize * sizeof(char));

  // Index first characters of each entry for convenient access.
  char **connList = new char *[maxEntries];
  int i;
  for (i = 0; i < maxEntries; i++)
    connList[i] = &connListBuffer[i * entryBufferSize];

  // Read the list of connections and remove it from the registry.
  int numRead = 0;
  for (i = 0; i < maxEntries; i++) {
    char valueName[16];
    SPRINTF(valueName, "%d", i);
    LPBYTE bufPtr = (LPBYTE)connList[numRead];
    DWORD bufSize = (entryBufferSize - 1) * sizeof(char);
    LONG err = RegQueryValueEx(hKey, valueName, 0, 0, bufPtr, &bufSize);
    if (err == ERROR_SUCCESS) {
      if (connList[numRead][0] != '\0')
        numRead++;
      RegDeleteValue(hKey, valueName);
    }
  }

  // Save current connection first.
  const BYTE *newConnPtr = (const BYTE *)m_opts.m_display;
  DWORD newConnSize = (DWORD)((strlen(m_opts.m_display) + 1) * sizeof(char));
  RegSetValueEx(hKey, "0", 0, REG_SZ, newConnPtr, newConnSize);

  // Save the list of other connections.
  // Don't forget to exclude duplicates of current connection, and make
  // sure the number of entries written will not exceed maxEntries.
  int numWritten = 1;
  for (i = 0; i < numRead && numWritten < maxEntries; i++) {
    if (strcmp(connList[i], m_opts.m_display) != 0) {
      char keyName[16];
      SPRINTF(keyName, "%d", numWritten);
      LPBYTE bufPtr = (LPBYTE)connList[i];
      DWORD bufSize = (DWORD)((strlen(connList[i]) + 1) * sizeof(char));
      RegSetValueEx(hKey, keyName, 0, REG_SZ, bufPtr, bufSize);
      numWritten++;
    }
  }

  // If not everything is written due to the maxEntries limitation, then
  // delete settings for the connection that was not saved.  But make sure
  // we do not delete settings for the current connection.
  if (i < numRead) {
    if (strcmp(connList[i], m_opts.m_display) != 0)
      RegDeleteKey(hKey, connList[i]);
  }

  RegCloseKey(hKey);

  // Save connection options for current connection.
  m_opts.SaveOpt(m_opts.m_display, KEY_VNCVIEWER_HISTORY);
}


void ClientConnection::EnableFullControlOptions()
{
  if (m_opts.m_ViewOnly) {
    SwitchOffKeys();
    EnableAction(IDD_FILETRANSFER, false);
    EnableAction(ID_CONN_CTLALTDEL, false);
    EnableAction(ID_CONN_CTLDOWN, false);
    EnableAction(ID_CONN_ALTDOWN, false);
    EnableAction(ID_CONN_CTLESC, false);
    EnableAction(ID_CONN_ALTENTER, false);
  } else {
    EnableAction(IDD_FILETRANSFER, m_enableFileTransfers);
    EnableAction(ID_CONN_CTLALTDEL, true);
    EnableAction(ID_CONN_CTLDOWN, true);
    EnableAction(ID_CONN_ALTDOWN, true);
    EnableAction(ID_CONN_CTLESC, true);
    EnableAction(ID_CONN_ALTENTER, true);
  }
}


void ClientConnection::EnableZoomOptions()
{
  if (m_opts.m_desktopSize.mode == SIZE_AUTO || m_opts.m_FitWindow) {
    EnableAction(ID_ZOOM_IN, false);
    EnableAction(ID_ZOOM_OUT, false);
    EnableAction(ID_ZOOM_100, false);
  } else {
    EnableAction(ID_ZOOM_IN, true);
    EnableAction(ID_ZOOM_OUT, true);
    EnableAction(ID_ZOOM_100, true);
  }
}


void ClientConnection::EnableAction(int id, bool enable)
{
  if (enable) {
    EnableMenuItem(GetSystemMenu(m_hwnd1, FALSE), id,
                   MF_BYCOMMAND | MF_ENABLED);
    SendMessage(m_hToolbar, TB_SETSTATE, (WPARAM)id,
                (LPARAM)MAKELONG(TBSTATE_ENABLED, 0));
  } else {
    EnableMenuItem(GetSystemMenu(m_hwnd1, FALSE), id,
                   MF_BYCOMMAND | MF_GRAYED);
    SendMessage(m_hToolbar, TB_SETSTATE, (WPARAM)id,
                (LPARAM)MAKELONG(TBSTATE_INDETERMINATE, 0));
  }
}


void ClientConnection::SwitchOffKeys()
{
  CheckMenuItem(GetSystemMenu(m_hwnd1, FALSE), ID_CONN_ALTDOWN,
                MF_BYCOMMAND | MF_UNCHECKED);
  CheckMenuItem(GetSystemMenu(m_hwnd1, FALSE), ID_CONN_CTLDOWN,
                MF_BYCOMMAND | MF_UNCHECKED);
  SendMessage(m_hToolbar, TB_SETSTATE, (WPARAM)ID_CONN_CTLDOWN,
              (LPARAM)MAKELONG(TBSTATE_ENABLED, 0));
  SendMessage(m_hToolbar, TB_SETSTATE, (WPARAM)ID_CONN_ALTDOWN,
              (LPARAM)MAKELONG(TBSTATE_ENABLED, 0));

  for (std::map<UINT, KeyActionSpec>::iterator iter = pressedKeys.begin();
       iter != pressedKeys.end(); ++iter) {
    KeyActionSpec syms = iter->second;

    for (int i = 0; syms.keycodes[i] != XK_VoidSymbol && i < MaxKeysPerKey;
         i++) {
      SendKeyEvent(syms.keycodes[i], false);
      vnclog.Print(4, "Sent keysym %04x (release)\n", syms.keycodes[i]);
    }
  }
  pressedKeys.clear();
}


void ClientConnection::GetConnectDetails()
{
  if (m_opts.m_configSpecified) {
    LoadConnection(m_opts.m_configFilename, false);
  } else {
    SessionDialog sessdlg(&m_opts, this);
    if (!sessdlg.DoDialog())
      throw QuietException("User Cancelled");

    // Add new connection to the connection history only if the VNC host name
    // was entered interactively, as we should remember user input even if it
    // does not seem to be correct.  If the connection info was specified in
    // the command line or in a configuration file, it will be added after the
    // VNC connection is established successfully.
    SaveConnectionHistory();
  }

  // This is a bit of a hack:
  // The config file may set various things in the app-level defaults, which
  // we don't want to be used except for the first connection.  So we clear
  // them in the app defaults here.
  m_pApp->m_options.m_host[0] = '\0';
  m_pApp->m_options.m_port = -1;
  m_pApp->m_options.m_connectionSpecified = false;
  m_pApp->m_options.m_configSpecified = false;

  // We want to know when the clipboard changes, so insert ourselves into the
  // viewer chain.  But doing this will cause us to be notified immediately of
  // the current state.  We don't want to send that.
  m_initialClipboardSeen = false;
  m_hwndNextViewer = SetClipboardViewer(m_hwnd);
}


void ClientConnection::Connect()
{
  ADDRINFOA hints, *addr;
  char portname[10], *hostname = m_host;
  int res;

  if (m_opts.m_tunnel)
    SetupSSHTunnel();

  if (m_connDlg != NULL)
    m_connDlg->SetStatus("Connection initiated");

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  // Winsock will resolve localhost to ::1 if left to its own devices.  We
  // force localhost to resolve to 127.0.0.1 instead, unless the /ipv6 option
  // was specified.  This mimics the behavior of Java.
  if (!_stricmp(hostname, "localhost") && !m_opts.m_ipv6)
    hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  SPRINTF(portname, "%d", m_port);
  if (strlen(hostname) < 1)
    hostname = NULL;
  int gai_err = getaddrinfo(hostname, portname, &hints, &addr);
  if (gai_err != 0) {
    char msg[512];
    SPRINTF(msg, "Failed to get server address (%s):\n%s\n"
            "Did you type the host name correctly?", m_host,
            gai_strerror(gai_err));
    throw WarningException(msg);
  }

  m_sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
  if (m_sock == INVALID_SOCKET) {
    freeaddrinfo(addr);
    throw WarningException("Error creating socket");
  }
  int one = 1;

  res = connect(m_sock, addr->ai_addr, (int)addr->ai_addrlen);
  if (res == SOCKET_ERROR) {
    char msg[512];
    SPRINTF(msg, "Failed to connect to server (%.255s)", m_opts.m_display);
    freeaddrinfo(addr);
    throw WarningException(msg);
  }
  vnclog.Print(0, "Connected to %s port %d\n", m_host, m_port);

  freeaddrinfo(addr);

  if (m_connDlg != NULL)
    m_connDlg->SetStatus("Connection established");
}


void ClientConnection::SetSocketOptions()
{
  // Disable Nagle's algorithm
  BOOL nodelayval = TRUE;
  if (setsockopt(m_sock, IPPROTO_TCP, TCP_NODELAY, (const char *)&nodelayval,
                 sizeof(BOOL)))
    throw WarningException("Error disabling Nagle's algorithm");
}


void ClientConnection::NegotiateProtocolVersion()
{
  rfbProtocolVersionMsg pv;

  ReadExact(pv, sz_rfbProtocolVersionMsg);

  pv[sz_rfbProtocolVersionMsg] = 0;

  if (m_connDlg != NULL)
    m_connDlg->SetStatus("Server protocol version received");

  int majorVersion, minorVersion;
  if (sscanf_s(pv, rfbProtocolVersionFormat, &majorVersion,
               &minorVersion) != 2)
    throw WarningException("Invalid protocol");
  vnclog.Print(0, "RFB server supports protocol version 3.%d\n", minorVersion);

  if (majorVersion == 3 && minorVersion >= 8)
    m_minorVersion = 8;
  else if (majorVersion == 3 && minorVersion == 7)
    m_minorVersion = 7;
  else
    m_minorVersion = 3;

  m_tightVncProtocol = false;

  SPRINTF(pv, rfbProtocolVersionFormat, 3, m_minorVersion);

  WriteExact(pv, sz_rfbProtocolVersionMsg);

  if (m_connDlg != NULL)
    m_connDlg->SetStatus("Protocol version negotiated");

  vnclog.Print(0, "Connected to RFB server, using protocol version 3.%d\n",
               m_minorVersion);
}


// Negotiate authentication scheme and authenticate if necessary

void ClientConnection::PerformAuthentication()
{
  int secType;
  if (m_minorVersion >= 7)
    secType = SelectSecurityType();
  else
    secType = ReadSecurityType();

  switch (secType) {
    case rfbSecTypeNone:
      Authenticate(rfbAuthNone);
      m_authScheme = rfbAuthNone;
      break;
    case rfbSecTypeVncAuth:
      Authenticate(rfbAuthVNC);
      m_authScheme = rfbAuthVNC;
      break;
    case rfbSecTypeTight:
      m_tightVncProtocol = true;
      InitCapabilities();
      SetupTunneling();
      PerformAuthenticationTight();
      break;
    default:  // should never happen
      vnclog.Print(0, "Internal error: Invalid security type\n");
      throw ErrorException("Internal error: Invalid security type");
  }
}


// Read security type from the server (protocol 3.3)

int ClientConnection::ReadSecurityType()
{
  // Read the authentication scheme.
  CARD32 secType;
  ReadExact((char *)&secType, sizeof(secType));
  secType = Swap32IfLE(secType);

  if (secType == rfbSecTypeInvalid)
    throw WarningException(ReadFailureReason());

  if (secType != rfbSecTypeNone && secType != rfbSecTypeVncAuth) {
    vnclog.Print(0, "Unknown security type from RFB server: %d\n",
                 (int)secType);
    throw ErrorException("Unknown security type requested!");
  }

  if (m_connDlg != NULL)
    m_connDlg->SetStatus("Security type received");

  return (int)secType;
}


// Select security type from the server's list (protocol 3.7 and above)

int ClientConnection::SelectSecurityType()
{
  // Read the list of secutiry types.
  CARD8 nSecTypes;
  ReadExact((char *)&nSecTypes, sizeof(nSecTypes));
  if (nSecTypes == 0)
    throw WarningException(ReadFailureReason());

  char *secTypeNames[] = { "None", "VncAuth" };
  CARD8 knownSecTypes[] = { rfbSecTypeNone, rfbSecTypeVncAuth };
  int nKnownSecTypes = sizeof(knownSecTypes);
  CARD8 *secTypes = new CARD8[nSecTypes];
  ReadExact((char *)secTypes, nSecTypes);
  CARD8 secType = rfbSecTypeInvalid;

  if (m_connDlg != NULL)
    m_connDlg->SetStatus("List of security types received");

  // Find out if the server supports TightVNC protocol extensions
  int j;
  for (j = 0; j < (int)nSecTypes; j++) {
    if (secTypes[j] == rfbSecTypeTight) {
      secType = rfbSecTypeTight;
      WriteExact((char *)&secType, sizeof(secType));
      if (m_connDlg != NULL)
        m_connDlg->SetStatus("TightVNC protocol extensions enabled");
      vnclog.Print(8, "Enabling TightVNC protocol extensions\n");
      return rfbSecTypeTight;
    }
  }

  // Find first supported security type
  for (j = 0; j < (int)nSecTypes; j++) {
    for (int i = 0; i < nKnownSecTypes; i++) {
      if (secTypes[j] == knownSecTypes[i]) {
        secType = secTypes[j];
        WriteExact((char *)&secType, sizeof(secType));
        if (m_connDlg != NULL)
          m_connDlg->SetStatus("Security type requested");
        vnclog.Print(8, "Choosing security type %s(%d)\n", secTypeNames[i],
                     (int)secType);
        break;
      }
    }
    if (secType != rfbSecTypeInvalid) break;
  }

  if (secType == rfbSecTypeInvalid) {
    vnclog.Print(0, "Server did not offer supported security type\n");
    throw ErrorException("Server did not offer supported security type!");
  }

  return (int)secType;
}


// Setup tunneling (protocol 3.7t, 3.8t)

void ClientConnection::SetupTunneling()
{
  rfbTunnelingCapsMsg caps;
  ReadExact((char *)&caps, sz_rfbTunnelingCapsMsg);
  caps.nTunnelTypes = Swap32IfLE(caps.nTunnelTypes);

  if (caps.nTunnelTypes) {
    ReadCapabilityList(&m_tunnelCaps, caps.nTunnelTypes);
    if (m_connDlg != NULL)
      m_connDlg->SetStatus("List of tunneling capabilities received");

    // We cannot do tunneling yet.
    CARD32 tunnelType = Swap32IfLE(rfbNoTunneling);
    WriteExact((char *)&tunnelType, sizeof(tunnelType));
    if (m_connDlg != NULL)
      m_connDlg->SetStatus("Tunneling type requested");
  }
}


// Negotiate authentication scheme (protocol 3.7t, 3.8t)

void ClientConnection::PerformAuthenticationTight()
{
  rfbAuthenticationCapsMsg caps;
  ReadExact((char *)&caps, sz_rfbAuthenticationCapsMsg);
  caps.nAuthTypes = Swap32IfLE(caps.nAuthTypes);

  if (m_connDlg != NULL)
    m_connDlg->SetStatus("Header of authentication capability list received");

  if (!caps.nAuthTypes) {
    vnclog.Print(0, "No authentication needed\n");
    if (m_connDlg != NULL)
      m_connDlg->SetStatus("No authentication needed");
    Authenticate(rfbAuthNone);
    m_authScheme = rfbAuthNone;
  } else {
    ReadCapabilityList(&m_authCaps, caps.nAuthTypes);
    if (m_connDlg != NULL)
      m_connDlg->SetStatus("Authentication capability list received");

    CARD32 authScheme = 0, a;
    if (!m_opts.m_noUnixLogin && strlen(m_opts.m_user) > 0) {
      // Prefer Unix Login over other types
      for (int i = 0; i < m_authCaps.NumEnabled(); i++) {
        if (m_authCaps.GetByOrder(i) == rfbAuthUnixLogin) {
          authScheme = rfbAuthUnixLogin;
          break;
        }
      }
    }

    if (authScheme == 0) {
      // Try server's preferred authentication scheme.
      for (int i = 0; (authScheme == 0) && (i < m_authCaps.NumEnabled());
           i++) {
        a = m_authCaps.GetByOrder(i);
        switch (a) {
          case rfbAuthVNC:
          case rfbAuthNone:
            authScheme = a;
            break;
          case rfbAuthUnixLogin:
            if (!m_opts.m_noUnixLogin) authScheme = a;
            break;
          default:
            // unknown scheme - cannot use it
            continue;
        }
      }
    }

    if (authScheme == 0) {
      vnclog.Print(0, "No suitable authentication schemes offered by the server\n");
      throw ErrorException("No suitable authentication schemes offered by the server");
    }

    authScheme = Swap32IfLE(authScheme);
    WriteExact((char *)&authScheme, sizeof(authScheme));
    authScheme = Swap32IfLE(authScheme);  // convert it back
    if (m_connDlg != NULL)
      m_connDlg->SetStatus("Authentication scheme requested");
    Authenticate(authScheme);
    m_authScheme = authScheme;
  }
}


// The definition of a function implementing some authentication scheme.
// For an example, see ClientConnection::AuthenticateVNC, below.

typedef bool (ClientConnection::*AuthFunc)(char *, int);

// A wrapper function for different authentication schemes.

void ClientConnection::Authenticate(CARD32 authScheme)
{
  AuthFunc authFuncPtr;

  // Uncomment this if the "Connecting..." dialog box should be
  // closed prior to authentication.
  /***
  if (m_connDlg != NULL) {
    delete m_connDlg;
    m_connDlg = NULL;
  }
  ***/

  switch (authScheme) {
    case rfbAuthNone:
      authFuncPtr = &ClientConnection::AuthenticateNone;
      break;
    case rfbAuthVNC:
      authFuncPtr = &ClientConnection::AuthenticateVNC;
      break;
    case rfbAuthUnixLogin:
      authFuncPtr = &ClientConnection::AuthenticateUnixLogin;
      break;
    default:
      vnclog.Print(0, "Unknown authentication scheme: %d\n", (int)authScheme);
      throw ErrorException("Unknown authentication scheme!");
  }

  vnclog.Print(0, "Authentication scheme: %s\n",
               m_authCaps.GetDescription(authScheme));

  const int errorMsgSize = 256;
  CheckBufferSize(errorMsgSize);
  char *errorMsg = m_netbuf;
  bool wasError = !(this->*authFuncPtr)(errorMsg, errorMsgSize);

  // Report authentication error.
  if (wasError) {
    vnclog.Print(0, "%s\n", errorMsg);
    if (m_connDlg != NULL)
      m_connDlg->SetStatus("Error during authentication");
    throw AuthException(errorMsg);
  }

  CARD32 authResult;
  if (authScheme == rfbAuthNone && m_minorVersion < 8) {
    // In protocol versions prior to 3.8, "no authentication" is a
    // special case - no "security result" is sent by the server.
    authResult = rfbAuthOK;
  } else {
    ReadExact((char *)&authResult, 4);
    authResult = Swap32IfLE(authResult);
  }

  switch (authResult) {
    case rfbAuthOK:
      if (m_connDlg != NULL)
        m_connDlg->SetStatus("Authentication successful");
      vnclog.Print(0, "Authentication successful\n");
      return;
    case rfbAuthFailed:
      if (m_minorVersion >= 8)
        errorMsg = ReadFailureReason();
      else
        errorMsg = "Authentication failure";
      break;
    case rfbAuthTooMany:
      errorMsg = "Authentication failure, too many tries";
      break;
    default:
      snprintf(m_netbuf, m_netbufsize, "Unknown authentication result (%d)",
               (int)authResult);
      errorMsg = m_netbuf;
      break;
  }

  // Report authentication failure.
  vnclog.Print(0, "%s\n", errorMsg);
  if (m_connDlg != NULL)
    m_connDlg->SetStatus(errorMsg);
  throw AuthException(errorMsg);
}


// "Null" authentication.

bool ClientConnection::AuthenticateNone(char *errBuf, int errBufSize)
{
  return true;
}


// Standard VNC authentication.
//
// An authentication function should return false on error and true if
// the authentication process was successful.  Note that returning true
// does not mean that authentication was passed by the server; the
// server's result will be received and analyzed later.
// If false is returned, then a text error message should be copied
// to errorBuf[].  No more than errBufSize bytes should be copied into
// that buffer.

bool ClientConnection::AuthenticateVNC(char *errBuf, int errBufSize)
{
  CARD8 challenge[CHALLENGESIZE];
  ReadExact((char *)challenge, CHALLENGESIZE);

  char passwd[MAXPWLEN + 1];
  if (m_opts.m_autoPass) {
    char *cstatus = fgets(passwd, sizeof(passwd), stdin);
    size_t len    = strlen(passwd);
    if (cstatus == NULL || len == 0) {
      passwd[0] = '\0';
      snprintf(errBuf, errBufSize, "Empty password");
      return false;
    } else {
      if (len > 0 && passwd[len - 1] == '\n')
        passwd[len - 1] = '\0';
    }
  }
  // Was the password already specified in a config file?
  else if (m_passwdSet) {
    char *pw = vncDecryptPasswd(m_encPasswd);
    STRCPY(passwd, pw);
    free(pw);
  } else {
    LoginAuthDialog ad(m_opts.m_display, "Standard VNC Authentication");
    ad.DoDialog();
    STRCPY(passwd, ad.m_passwd);
    if (strlen(passwd) == 0) {
      snprintf(errBuf, errBufSize, "Empty password");
      return false;
    }
    if (strlen(passwd) > 8)
      passwd[8] = '\0';
    vncEncryptPasswd(m_encPasswd, passwd);
    m_passwdSet = true;
  }

  vncEncryptBytes(challenge, passwd);

  /* Lose the plain-text password from memory */
  memset(passwd, 0, strlen(passwd));

  WriteExact((char *)challenge, CHALLENGESIZE);

  return true;
}


// Unix Login authentication.

bool ClientConnection::AuthenticateUnixLogin(char *errBuf, int errBufSize)
{
  CARD32 t;

  char passwd[256] = "\0", user[256] = "\0";
  if (strlen(m_opts.m_user) > 0)
    STRCPY(user, m_opts.m_user);

  if (strlen(user) > 0 && m_opts.m_autoPass) {
    char *cstatus = fgets(passwd, sizeof(passwd), stdin);
    size_t len    = strlen(passwd);
    if (cstatus == NULL || len == 0) {
      passwd[0] = '\0';
      snprintf(errBuf, errBufSize, "Empty password");
      return false;
    } else {
      if (len > 0 && passwd[len - 1] == '\n')
        passwd[len - 1] = '\0';
    }
  } else {
    LoginAuthDialog ad(m_opts.m_display, "Unix Login Authentication", user);
    ad.DoDialog();
    STRCPY(user, ad.m_username);
    if (strlen(user) == 0) {
      snprintf(errBuf, errBufSize, "Empty user name");
      return false;
    }
    STRCPY(m_opts.m_user, user);

    STRCPY(passwd, ad.m_passwd);
    if (strlen(passwd) == 0) {
      snprintf(errBuf, errBufSize, "Empty password");
      return false;
    }
  }

  t = Swap32IfLE(strlen(user));
  WriteExact((char *)&t, sizeof(t));

  t = Swap32IfLE(strlen(passwd));
  WriteExact((char *)&t, sizeof(t));

  WriteExact(user, (int)strlen(user));
  WriteExact(passwd, (int)strlen(passwd));

  /* Lose the plain-text password from memory */
  memset(passwd, 0, strlen(passwd));

  return true;
}


void ClientConnection::SendClientInit()
{
  rfbClientInitMsg ci;
  ci.shared = m_opts.m_Shared;

  WriteExact((char *)&ci, sz_rfbClientInitMsg);

  if (m_connDlg != NULL)
    m_connDlg->SetStatus("Client initialization message sent");
}


void ClientConnection::ReadServerInit()
{
  ReadExact((char *)&m_si, sz_rfbServerInitMsg);

  if (m_connDlg != NULL)
    m_connDlg->SetStatus("Server initialization message received");

  m_si.framebufferWidth = Swap16IfLE(m_si.framebufferWidth);
  m_si.framebufferHeight = Swap16IfLE(m_si.framebufferHeight);
  m_si.format.redMax = Swap16IfLE(m_si.format.redMax);
  m_si.format.greenMax = Swap16IfLE(m_si.format.greenMax);
  m_si.format.blueMax = Swap16IfLE(m_si.format.blueMax);
  m_si.nameLength = Swap32IfLE(m_si.nameLength);

  m_desktopName = new char[m_si.nameLength + 2];

  ReadString(m_desktopName, m_si.nameLength);

  SetWindowTitle();

  vnclog.Print(0, "Desktop name \"%s\"\n", m_desktopName);
  vnclog.Print(1, "Geometry %d x %d depth %d\n", m_si.framebufferWidth,
               m_si.framebufferHeight, m_si.format.depth);

  SizeWindow(true, true, false);
}


void ClientConnection::SetLastEncoding(int enc)
{
  if (enc != m_opts.m_LastEncoding) {
    m_opts.m_LastEncoding = enc;
    SetWindowTitle();
  }
}


static const char *sampopt2str[TVNC_SAMPOPT] = {
  "1X", "4X", "2X", "Gray"
};


void ClientConnection::SetWindowTitle()
{
  char *title;
  size_t len = strlen(m_desktopName) + 80;
  title = new char[len];
  snprintf(title, len, "%s ", m_desktopName);
  if (m_opts.m_PreferredEncoding == rfbEncodingTight &&
      (m_opts.m_LastEncoding < 0 ||
       m_opts.m_LastEncoding == rfbEncodingTight)) {
    char zlibstr[80];
    zlibstr[0] = 0;
    if (!m_opts.m_enableJpegCompression) {
      SPRINTF(zlibstr, " + CL %d", m_opts.m_compressLevel);
      snprintf(&title[strlen(title)], len - strlen(title),
               "[Lossless Tight%s]", zlibstr);
    } else {
      SPRINTF(zlibstr, " + CL %d", m_opts.m_compressLevel);
      snprintf(&title[strlen(title)], len - strlen(title),
               "[Tight + JPEG %s Q%d%s]", sampopt2str[m_opts.m_subsampLevel],
               m_opts.m_jpegQualityLevel, zlibstr);
    }
  } else {
    int enc;
    if (m_opts.m_LastEncoding >= 0)
      enc = m_opts.m_LastEncoding;
    else
      enc = m_opts.m_PreferredEncoding;
    if (enc >= 0 && enc <= rfbEncodingHextile) {
      char encStr[6][8] = { "Raw", "", "", "", "", "Hextile" };
      snprintf(&title[strlen(title)], len - strlen(title), "[%s]",
               encStr[enc]);
    }
  }
  if (m_opts.m_scaling) {
    int sf = (m_opts.m_scale_num * 100) / m_opts.m_scale_den;
    if (sf != 100)
      snprintf(&title[strlen(title)], len - strlen(title), " - %d%%",
               (m_opts.m_scale_num * 100) / m_opts.m_scale_den);
  }
  SetWindowText(m_hwnd1, title);
  delete[] title;
}


// In protocols 3.7t/3.8t, the server informs us about supported
// protocol messages and encodings.  Here we read this information.

void ClientConnection::ReadInteractionCaps()
{
  // Read the counts of list items following
  rfbInteractionCapsMsg intr_caps;
  ReadExact((char *)&intr_caps, sz_rfbInteractionCapsMsg);
  intr_caps.nServerMessageTypes = Swap16IfLE(intr_caps.nServerMessageTypes);
  intr_caps.nClientMessageTypes = Swap16IfLE(intr_caps.nClientMessageTypes);
  intr_caps.nEncodingTypes = Swap16IfLE(intr_caps.nEncodingTypes);
  if (m_connDlg != NULL)
    m_connDlg->SetStatus("Interaction capability list header received");

  // Read the lists of server- and client-initiated messages
  ReadCapabilityList(&m_serverMsgCaps, intr_caps.nServerMessageTypes);
  ReadCapabilityList(&m_clientMsgCaps, intr_caps.nClientMessageTypes);
  ReadCapabilityList(&m_encodingCaps, intr_caps.nEncodingTypes);
  if (m_connDlg != NULL)
    m_connDlg->SetStatus("Interaction capability list received");
}


// Read the list of rfbCapabilityInfo structures and enable corresponding
// capabilities in the specified container.  The count argument specifies how
// many records to read from the socket.

void ClientConnection::ReadCapabilityList(CapsContainer *caps, int count)
{
  rfbCapabilityInfo msginfo;
  for (int i = 0; i < count; i++) {
    ReadExact((char *)&msginfo, sz_rfbCapabilityInfo);
    msginfo.code = Swap32IfLE(msginfo.code);
    caps->Enable(&msginfo);
  }
}


void ClientConnection::SizeWindow(bool centered, bool resizeFullScreen,
                                  bool manual)
{
  RECT fullwinrect, screenArea, workArea, winrect;

  if (InFullScreenMode() && !resizeFullScreen) {
    if (manual) {
      bool checkLayoutNow = false;

      GetFullScreenMetrics(screenArea, workArea);
      if (m_opts.m_desktopSize.mode == SIZE_AUTO) {
        GetWindowRect(m_hwnd1, &winrect);
        // If the window size is unchanged, check whether we need to force a
        // desktop resize message to inform the server of a new screen layout
        if (WidthOf(screenArea) == WidthOf(winrect) &&
            HeightOf(screenArea) == HeightOf(winrect)) {
          if (!EqualRect(&screenArea, &winrect))
            m_checkLayout = true;
          else
            // A WM_MOVE message is unlikely to be sent to the window, so we
            // need to check the layout immediately.
            checkLayoutNow = true;
        }
      }
      SetWindowPos(m_hwnd1, HWND_TOPMOST, screenArea.left, screenArea.top,
                   WidthOf(screenArea), HeightOf(screenArea), SWP_NOSIZE);
      if (checkLayoutNow) {
        int w = WidthOf(screenArea), h = HeightOf(screenArea);
        ScreenSet layout = ComputeScreenLayout(w, h);
        if (w >= 1 && h >= 1 && layout != m_screenLayout)
          SendDesktopSize(w, h, layout);
      }
    }
    return;
  }

  if (m_opts.m_desktopSize.mode == SIZE_AUTO && manual) {
    GetFullScreenMetrics(screenArea, workArea);
    fullwinrect = workArea;
    GetWindowRect(m_hwnd1, &winrect);
    // If the window size is unchanged, check whether we need to force a
    // desktop resize message to inform the server of a new screen layout
    if (WidthOf(fullwinrect) == WidthOf(winrect) &&
        HeightOf(fullwinrect) == HeightOf(winrect))
      m_checkLayout = true;
    PositionWindow(fullwinrect, centered);
    return;
  } else if (m_opts.m_scaling && !m_opts.m_FitWindow) {
    SetRect(&fullwinrect, 0, 0,
            m_si.framebufferWidth * m_opts.m_scale_num / m_opts.m_scale_den,
            m_si.framebufferHeight * m_opts.m_scale_num / m_opts.m_scale_den);
  } else {
    SetRect(&fullwinrect, 0, 0, m_si.framebufferWidth, m_si.framebufferHeight);
  }

  PositionWindow(fullwinrect, centered);
  PositionChildWindow();
  if (m_opts.m_FitWindow && !InFullScreenMode()) {
    RECT clirect;
    SetRect(&clirect, 0, 0, m_cliwidth, m_cliheight);
    PositionWindow(clirect, centered);
  }
}


void ClientConnection::PositionWindow(RECT &fullwinrect, bool centered,
                                      bool dimensionsOnly)
{
  // Find how large the desktop work area is
  RECT screenArea, workrect;
  GetFullScreenMetrics(screenArea, workrect);
  int workwidth = workrect.right - workrect.left;
  int workheight = workrect.bottom - workrect.top;

  AdjustWindowRectEx(&fullwinrect, GetWindowLong(m_hwnd, GWL_STYLE),
                     FALSE, GetWindowLong(m_hwnd, GWL_EXSTYLE));

  m_fullwinwidth = fullwinrect.right - fullwinrect.left;
  m_fullwinheight = fullwinrect.bottom - fullwinrect.top;

  AdjustWindowRectEx(&fullwinrect,
                     GetWindowLong(m_hwndscroll, GWL_STYLE) & ~WS_HSCROLL &
                       ~WS_VSCROLL & ~WS_BORDER,
                     FALSE, GetWindowLong(m_hwndscroll, GWL_EXSTYLE));
  AdjustWindowRectEx(&fullwinrect, GetWindowLong(m_hwnd1, GWL_STYLE),
                     FALSE, GetWindowLong(m_hwnd1, GWL_EXSTYLE));

  if (GetMenuState(GetSystemMenu(m_hwnd1, FALSE),
                   ID_TOOLBAR, MF_BYCOMMAND) == MF_CHECKED) {
    RECT rtb;
    GetWindowRect(m_hToolbar, &rtb);
    fullwinrect.bottom = fullwinrect.bottom + rtb.bottom - rtb.top - 3;
  }

  m_winwidth  = min(fullwinrect.right - fullwinrect.left, workwidth);
  m_winheight = min(fullwinrect.bottom - fullwinrect.top, workheight);

  if ((fullwinrect.right - fullwinrect.left > workwidth) &&
      (workheight - m_winheight >= 16) &&
      m_opts.m_desktopSize.mode != SIZE_AUTO)
    m_winheight = m_winheight + 16;

  if ((fullwinrect.bottom - fullwinrect.top > workheight) &&
      (workwidth - m_winwidth >= 16) &&
      m_opts.m_desktopSize.mode != SIZE_AUTO)
    m_winwidth = m_winwidth + 16;

  if (dimensionsOnly) return;

  int x, y;
  WINDOWPLACEMENT winplace;
  winplace.length = sizeof(WINDOWPLACEMENT);
  GetWindowPlacement(m_hwnd1, &winplace);
  if (centered) {
    x = (workwidth - m_winwidth) / 2 + workrect.left;
    y = (workheight - m_winheight) / 2 + workrect.top;
    if (winplace.showCmd != SW_SHOWMINIMIZED)
      winplace.showCmd = SW_SHOWNORMAL;
  } else {
    // Try to preserve current position if possible
    GetWindowPlacement(m_hwnd1, &winplace);
    if ((winplace.showCmd == SW_SHOWMAXIMIZED) ||
        (winplace.showCmd == SW_SHOWMINIMIZED)) {
      x = winplace.rcNormalPosition.left;
      y = winplace.rcNormalPosition.top;
    } else {
      RECT tmprect;
      GetWindowRect(m_hwnd1, &tmprect);
      x = tmprect.left;
      y = tmprect.top;
    }
    if (x + m_winwidth > workrect.right)
      x = workrect.right - m_winwidth;
    if (y + m_winheight > workrect.bottom)
      y = workrect.bottom - m_winheight;
  }
  winplace.rcNormalPosition.top = y;
  winplace.rcNormalPosition.left = x;
  winplace.rcNormalPosition.right = x + m_winwidth;
  winplace.rcNormalPosition.bottom = y + m_winheight;
  SetWindowPlacement(m_hwnd1, &winplace);
  SetForegroundWindow(m_hwnd1);
}


void ClientConnection::GetActualClientRect(RECT *rect)
{
  GetClientRect(m_hwnd1, rect);

  if (GetMenuState(GetSystemMenu(m_hwnd1, FALSE),
                   ID_TOOLBAR, MF_BYCOMMAND) == MF_CHECKED) {
    RECT rtb;
    GetWindowRect(m_hToolbar, &rtb);
    int rtbheight = HeightOf(rtb) - 3;
    rect->top += rtbheight;
  }
}


void ClientConnection::PositionChildWindow()
{
  RECT rparent;
  GetClientRect(m_hwnd1, &rparent);

  int parentwidth = rparent.right - rparent.left;
  int parentheight = rparent.bottom - rparent.top;

  if (GetMenuState(GetSystemMenu(m_hwnd1, FALSE),
                   ID_TOOLBAR, MF_BYCOMMAND) == MF_CHECKED) {
    RECT rtb;
    GetWindowRect(m_hToolbar, &rtb);
    int rtbheight = rtb.bottom - rtb.top - 3;
    SetWindowPos(m_hToolbar, HWND_TOP, rparent.left, rparent.top,
                 parentwidth, rtbheight, SWP_SHOWWINDOW);
    parentheight = parentheight - rtbheight;
    rparent.top = rparent.top + rtbheight;
  } else {
    ShowWindow(m_hToolbar, SW_HIDE);
  }

  m_autoResizeWidth = parentwidth;
  m_autoResizeHeight = parentheight;

  SetWindowPos(m_hwndscroll, HWND_TOP, rparent.left - 1, rparent.top - 1,
               parentwidth + 2, parentheight + 2, SWP_SHOWWINDOW);
  if (!m_opts.m_FitWindow) {
    if (InFullScreenMode()) {
      ShowScrollBar(m_hwndscroll, SB_HORZ, FALSE);
      ShowScrollBar(m_hwndscroll, SB_VERT, FALSE);
    } else {
      ShowScrollBar(m_hwndscroll, SB_VERT, parentheight < m_fullwinheight);
      ShowScrollBar(m_hwndscroll, SB_HORZ, parentwidth  < m_fullwinwidth);
      GetClientRect(m_hwndscroll, &rparent);
      parentwidth = rparent.right - rparent.left;
      parentheight = rparent.bottom - rparent.top;
      ShowScrollBar(m_hwndscroll, SB_VERT, parentheight < m_fullwinheight);
      ShowScrollBar(m_hwndscroll, SB_HORZ, parentwidth  < m_fullwinwidth);
      GetClientRect(m_hwndscroll, &rparent);
      parentwidth = rparent.right - rparent.left;
      parentheight = rparent.bottom - rparent.top;
    }
  } else {
    if (!IsIconic(m_hwnd1)) {
      ShowScrollBar(m_hwndscroll, SB_HORZ, FALSE);
      ShowScrollBar(m_hwndscroll, SB_VERT, FALSE);
      GetClientRect(m_hwndscroll, &rparent);
      parentwidth = rparent.right - rparent.left;
      parentheight = rparent.bottom - rparent.top;
      if ((parentwidth < 1) || (parentheight < 1))
        return;
      RECT fullwinrect;
      int den = max(m_si.framebufferWidth * 100 / parentwidth,
                    m_si.framebufferHeight * 100 / parentheight);
      SetRect(&fullwinrect, 0, 0,
              (m_si.framebufferWidth * 100 + den - 1) / den,
              (m_si.framebufferHeight * 100 + den - 1) / den);
      while ((fullwinrect.right - fullwinrect.left > parentwidth) ||
             (fullwinrect.bottom - fullwinrect.top > parentheight)) {
        den++;
        SetRect(&fullwinrect, 0, 0,
                (m_si.framebufferWidth * 100 + den - 1) / den,
                (m_si.framebufferHeight * 100 + den - 1) / den);
      }

      m_opts.m_scale_num = 100;
      m_opts.m_scale_den = den;

      m_opts.FixScaling();

      m_fullwinwidth = fullwinrect.right - fullwinrect.left;
      m_fullwinheight = fullwinrect.bottom - fullwinrect.top;
    }
  }

  int x, y;
  if (parentwidth > m_fullwinwidth)
    x = (parentwidth - m_fullwinwidth) / 2;
  else
    x = rparent.left;

  if (parentheight > m_fullwinheight)
    y = (parentheight - m_fullwinheight) / 2;
  else
    y = rparent.top;

  SetWindowPos(m_hwnd, HWND_TOP, x, y, min(parentwidth, m_fullwinwidth),
               min(parentheight, m_fullwinheight), SWP_SHOWWINDOW);

  m_cliwidth = min((int)parentwidth, (int)m_fullwinwidth);
  m_cliheight = min((int)parentheight, (int)m_fullwinheight);

  m_hScrollMax = m_fullwinwidth;
  m_vScrollMax = m_fullwinheight;

  int newhpos, newvpos;
  if (!m_opts.m_FitWindow) {
    newhpos = max(0, min(m_hScrollPos, m_hScrollMax - max(m_cliwidth, 0)));
    newvpos = max(0, min(m_vScrollPos, m_vScrollMax - max(m_cliheight, 0)));
  } else {
    newhpos = 0;
    newvpos = 0;
  }
  RECT clichild;
  GetClientRect(m_hwnd, &clichild);
  ScrollWindowEx(m_hwnd, m_hScrollPos - newhpos, m_vScrollPos - newvpos,
                 NULL, &clichild, NULL, NULL,  SW_INVALIDATE);

  m_hScrollPos = newhpos;
  m_vScrollPos = newvpos;
  if (!m_opts.m_FitWindow)
    UpdateScrollbars();
  else
    InvalidateRect(m_hwnd, NULL, FALSE);
  UpdateWindow(m_hwnd);

  if (m_opts.m_desktopSize.mode == SIZE_AUTO && !m_firstUpdate &&
      !m_pendingServerResize &&
      (m_si.framebufferWidth != m_autoResizeWidth ||
       m_si.framebufferHeight != m_autoResizeHeight))
    m_resizeTimer = SetTimer(m_hwnd, IDT_RESIZETIMER, 500, NULL);
}


void ClientConnection::CreateLocalFramebuffer()
{
  omni_mutex_lock l(m_bitmapdcMutex);

  // Remove old bitmap object if it already exists
  bool bitmapExisted = false;
  if (fb.bits != NULL)
    bitmapExisted = true;

  // We create a bitmap which has the same pixel characteristics as
  // the local display, in the hope that blitting will be faster.
  if (fbx_init(&fb, m_hwnd, m_si.framebufferWidth, m_si.framebufferHeight,
               1) == -1)
    throw ErrorException(fbx_geterrmsg());

  m_hBitmapDC = fb.hmdc;
  m_hBitmap = fb.hdib;

  // Select this bitmap into the DC with an appropriate palette
  ObjectSelector b(m_hBitmapDC, m_hBitmap);
  PaletteSelector p(m_hBitmapDC, m_hPalette);

  if (!bitmapExisted) {
    // Put a "please wait" message up initially
    RECT rect;
    SetRect(&rect, 0, 0, m_si.framebufferWidth, m_si.framebufferHeight);
    COLORREF bgcol = RGB(0xcc, 0xcc, 0xcc);
    FillSolidRect(&rect, bgcol);

    COLORREF oldbgcol  = SetBkColor(m_hBitmapDC, bgcol);
    COLORREF oldtxtcol = SetTextColor(m_hBitmapDC, RGB(0, 0, 64));
    rect.right = m_si.framebufferWidth / 2;
    rect.bottom = m_si.framebufferHeight / 2;

    DrawText(m_hBitmapDC, "Please wait - initial screen loading", -1, &rect,
             DT_SINGLELINE | DT_CENTER | DT_VCENTER);
    SetBkColor(m_hBitmapDC, oldbgcol);
    SetTextColor(m_hBitmapDC, oldtxtcol);
  }

  InvalidateRect(m_hwnd, NULL, FALSE);
}


void ClientConnection::SetupPixelFormat()
{
  // Have we requested a reduction to 8-bit?
  if (m_opts.m_Use8Bit) {

    vnclog.Print(2, "Requesting 8-bit true color\n");
    m_myFormat = vnc8bitFormat;

  // We don't support colormaps so we'll ask the server to convert
  } else if (!m_si.format.trueColour) {

    // We'll just request a standard 16-bit truecolor
    vnclog.Print(2, "Requesting 16-bit true color\n");
    m_myFormat = vnc16bitFormat;

  } else {

    // Normally we just use the sever's format suggestion
    m_myFormat = m_opts.m_benchFile ? vnc24bitFormat : m_si.format;

    // It's silly requesting more bits than our current display has, but
    // in fact it doesn't usually amount to much on the network.
    // Windows doesn't support 8-bit true color.
    // If our display is palette-based, we want more than 8 bit anyway,
    // unless we're going to start doing palette stuff at the server.
    // So the main use would be a 24-bit true color desktop being viewed
    // on a 16-bit true color display, and unless you have lots of images
    // and hence lots of raw-encoded stuff, the size of the pixel is not
    // going to make much difference.
    // We therefore don't bother with any restrictions, but here's the
    // start of the code if we wanted to do it.

    if (false) {

      // Get a DC for the root window
      TempDC hrootdc(NULL);
      int localBitsPerPixel = GetDeviceCaps(hrootdc, BITSPIXEL);
      int localRasterCaps   = GetDeviceCaps(hrootdc, RASTERCAPS);
      vnclog.Print(2, "Memory DC has depth of %d and %s pallete-based.\n",
                   localBitsPerPixel,
                   (localRasterCaps & RC_PALETTE) ? "is" : "is not");

      // If we're using truecolor, and the server has more bits than we do
      if (localBitsPerPixel > m_myFormat.depth &&
          !(localRasterCaps & RC_PALETTE))
        m_myFormat.depth = localBitsPerPixel;
    }
  }

  // The endianness will be set before sending
}


void ClientConnection::SetFormatAndEncodings()
{
  m_pendingEncodingChange = false;

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
  bool useSubsampLevel = false;
  int i;

  // Put the preferred encoding first, and change it if the
  // preferred encoding is not actually usable.
  for (i = LASTENCODING; i >= rfbEncodingRaw; i--) {
    if (m_opts.m_PreferredEncoding == i) {
      if (m_opts.m_UseEnc[i]) {
        encs[se->nEncodings++] = Swap32IfLE(i);
        if (i == rfbEncodingZlib || i == rfbEncodingTight ||
            i == rfbEncodingZlibHex)
          useCompressLevel = true;
        if (i == rfbEncodingTight)
          useSubsampLevel = true;
      } else {
        m_opts.m_PreferredEncoding--;
      }
    }
  }

  // Now we go through and put in all the other encodings in order.
  // We assume that the most recent encoding is the most desirable.
  for (i = LASTENCODING; i >= rfbEncodingRaw; i--) {
    if (m_opts.m_PreferredEncoding != i && m_opts.m_UseEnc[i]) {
      encs[se->nEncodings++] = Swap32IfLE(i);
      if (i == rfbEncodingZlib || i == rfbEncodingTight ||
          i == rfbEncodingZlibHex)
        useCompressLevel = true;
    }
  }

  // Request desired compression level, if applicable
  if (useCompressLevel && m_opts.m_compressLevel >= 0 &&
      m_opts.m_compressLevel <= 9)
    encs[se->nEncodings++] =
      Swap32IfLE(rfbEncodingCompressLevel0 + m_opts.m_compressLevel);

  // Request cursor shape updates, if enabled by user
  if (m_opts.m_requestShapeUpdates) {
    encs[se->nEncodings++] = Swap32IfLE(rfbEncodingRichCursor);
    encs[se->nEncodings++] = Swap32IfLE(rfbEncodingXCursor);
    if (!m_opts.m_ignoreShapeUpdates)
      encs[se->nEncodings++] = Swap32IfLE(rfbEncodingPointerPos);
  }

  // Request JPEG quality level, if JPEG compression was enabled by user
  if (m_opts.m_PreferredEncoding == rfbEncodingTight &&
      m_opts.m_enableJpegCompression && m_opts.m_jpegQualityLevel >= 1 &&
      m_opts.m_jpegQualityLevel <= 100) {
    int tightQualityLevel = m_opts.m_jpegQualityLevel / 10;
    if (tightQualityLevel > 9) tightQualityLevel = 9;
    encs[se->nEncodings++] =
      Swap32IfLE(rfbEncodingQualityLevel0 + tightQualityLevel);
    encs[se->nEncodings++] =
      Swap32IfLE(rfbEncodingFineQualityLevel0 + m_opts.m_jpegQualityLevel);
  } else if (m_opts.m_PreferredEncoding != rfbEncodingTight ||
             (m_opts.m_LastEncoding >= 0 &&
              m_opts.m_LastEncoding != rfbEncodingTight)) {
    int qualityLevel = m_opts.m_jpegQualityLevel;
    if (qualityLevel > 9) qualityLevel = 9;
    encs[se->nEncodings++] =
      Swap32IfLE(rfbEncodingQualityLevel0 + qualityLevel);
  }

  // Request desired subsampling level, if applicable
  if (useSubsampLevel && m_opts.m_enableJpegCompression &&
      m_opts.m_subsampLevel >= 0 && m_opts.m_subsampLevel <= TVNC_SAMPOPT - 1)
    encs[se->nEncodings++] =
      Swap32IfLE(rfbEncodingSubsamp1X + m_opts.m_subsampLevel);

  // Notify the server that we support LastRect and NewFBSize encodings
  encs[se->nEncodings++] = Swap32IfLE(rfbEncodingLastRect);
  encs[se->nEncodings++] = Swap32IfLE(rfbEncodingNewFBSize);
  encs[se->nEncodings++] = Swap32IfLE(rfbEncodingExtendedDesktopSize);
  if (m_opts.m_CU) {
    encs[se->nEncodings++] = Swap32IfLE(rfbEncodingContinuousUpdates);
    encs[se->nEncodings++] = Swap32IfLE(rfbEncodingFence);
  }
  encs[se->nEncodings++] = Swap32IfLE(rfbEncodingGII);

  len = sz_rfbSetEncodingsMsg + se->nEncodings * 4;

  se->nEncodings = Swap16IfLE(se->nEncodings);

  WriteExact((char *)buf, len);
}


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


void ClientConnection::CopyOptions(ClientConnection *source)
{
  this->m_opts = source->m_opts;
}


ClientConnection::~ClientConnection()
{
  if (m_hwnd1 != 0)
    DestroyWindow(m_hwnd1);

  if (m_connDlg != NULL)
    delete m_connDlg;

  if (m_sock != INVALID_SOCKET) {
    shutdown(m_sock, SD_BOTH);
    closesocket(m_sock);
    m_sock = INVALID_SOCKET;
  }

  if (m_desktopName != NULL) delete[] m_desktopName;
  delete[] m_netbuf;
  delete m_pFileTransfer;
  DeleteDC(m_hBitmapDC);
  if (m_hBitmap != NULL)
    DeleteObject(m_hBitmap);
  if (m_hPalette != NULL)
    DeleteObject(m_hPalette);

  m_pApp->DeregisterConnection(this);

  fbx_term(&fb);
  if (j) { tjDestroy(j);  j = NULL; }
}


// You can specify a dx & dy outside the limits;  the return value will
// tell you whether it actually scrolled.

bool ClientConnection::ScrollScreen(int dx, int dy)
{
  dx = max(dx, -m_hScrollPos);
  //dx = min(dx, m_hScrollMax - (m_cliwidth - 1) - m_hScrollPos);
  dx = min(dx, m_hScrollMax - (m_cliwidth) - m_hScrollPos);
  dy = max(dy, -m_vScrollPos);
  //dy = min(dy, m_vScrollMax - (m_cliheight - 1) - m_vScrollPos);
  dy = min(dy, m_vScrollMax - (m_cliheight) - m_vScrollPos);
  if (dx || dy) {
    m_hScrollPos += dx;
    m_vScrollPos += dy;
    RECT clirect;
    GetClientRect(m_hwnd, &clirect);
    ScrollWindowEx(m_hwnd, -dx, -dy, NULL, &clirect, NULL, NULL,
                   SW_INVALIDATE);
    UpdateScrollbars();
    UpdateWindow(m_hwnd);
    return true;
  }
  return false;
}


LRESULT CALLBACK ClientConnection::ScrollProc(HWND hwnd, UINT iMsg,
                                              WPARAM wParam, LPARAM lParam)
{
  // This is a static method, so we don't know which instantiation we're
  // dealing with.  But we've stored a 'pseudo-this' in the window data.
  ClientConnection *_this =
    (ClientConnection *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

  switch (iMsg) {
    case WM_HSCROLL:
    {
      int dx = 0;
      int pos = HIWORD(wParam);
      switch (LOWORD(wParam)) {
        case SB_LINEUP:
          dx = -2;  break;
        case SB_LINEDOWN:
          dx = 2;  break;
        case SB_PAGEUP:
          dx = _this->m_cliwidth * -1 / 4;  break;
        case SB_PAGEDOWN:
          dx = _this->m_cliwidth * 1 / 4;  break;
        case SB_THUMBPOSITION:
          dx = pos - _this->m_hScrollPos;
        case SB_THUMBTRACK:
          dx = pos - _this->m_hScrollPos;
      }
      if (!_this->m_opts.m_FitWindow)
        _this->ScrollScreen(dx, 0);
      return 0;
    }
    case WM_VSCROLL:
    {
      int dy = 0;
      int pos = HIWORD(wParam);
      switch (LOWORD(wParam)) {
        case SB_LINEUP:
          dy = -2;  break;
        case SB_LINEDOWN:
          dy = 2;  break;
        case SB_PAGEUP:
          dy = _this->m_cliheight * -1 / 4;  break;
        case SB_PAGEDOWN:
          dy = _this->m_cliheight * 1 / 4;  break;
        case SB_THUMBPOSITION:
          dy = pos - _this->m_vScrollPos;
        case SB_THUMBTRACK:
          dy = pos - _this->m_vScrollPos;
      }
      if (!_this->m_opts.m_FitWindow)
        _this->ScrollScreen(0, dy);
      return 0;
    }
  }
  return DefWindowProc(hwnd, iMsg, wParam, lParam);
}


#define COND_UNGRAB_KEYBOARD  \
  bool regrab = false;  \
  if (_this->isKeyboardGrabbed()) {  \
    _this->UngrabKeyboard(true);  \
    regrab = true;  \
  }

#define COND_REGRAB_KEYBOARD  \
  if (regrab) _this->GrabKeyboard(true);


LRESULT CALLBACK ClientConnection::WndProc1(HWND hwnd, UINT iMsg,
                                            WPARAM wParam, LPARAM lParam)
{
  // This is a static method, so we don't know which instantiation we're
  // dealing with.  But we've stored a 'pseudo-this' in the window data.
  ClientConnection *_this =
    (ClientConnection *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

  switch (iMsg) {
    case WM_ACTIVATE:
      if (_this->m_hctx) {
        gpWTEnable(_this->m_hctx, LOWORD(wParam));
        if (LOWORD(wParam))
          gpWTOverlap(_this->m_hctx, TRUE);
      }
      return 0;
    case WM_NOTIFY:
    {
      LPTOOLTIPTEXT TTStr = (LPTOOLTIPTEXT)lParam;
      if (TTStr->hdr.code != TTN_NEEDTEXT)
        return 0;

      switch (TTStr->hdr.idFrom) {
        case IDC_OPTIONBUTTON:
          TTStr->lpszText = "Options...";
          break;
        case ID_CONN_ABOUT:
          TTStr->lpszText = "Connection info...";
          break;
        case ID_FULLSCREEN:
          TTStr->lpszText = "Full screen";
          break;
        case ID_REQUEST_REFRESH:
          TTStr->lpszText = "Request screen refresh";
          break;
        case ID_REQUEST_LOSSLESS_REFRESH:
          TTStr->lpszText = "Request lossless refresh";
          break;
        case ID_CONN_CTLALTDEL:
          TTStr->lpszText = "Send Ctrl-Alt-Del";
          break;
        case ID_CONN_CTLESC:
          TTStr->lpszText = "Send Ctrl-Esc";
          break;
        case ID_CONN_ALTENTER:
          TTStr->lpszText = "Send Alt-Enter";
          break;
        case ID_CONN_CTLDOWN:
          TTStr->lpszText = "Send Ctrl key press/release";
          break;
        case ID_CONN_ALTDOWN:
          TTStr->lpszText = "Send Alt key press/release";
          break;
        case IDD_FILETRANSFER:
          TTStr->lpszText = "Transfer files...";
          break;
        case ID_NEWCONN:
          TTStr->lpszText = "New connection...";
          break;
        case ID_CONN_SAVE_AS:
          TTStr->lpszText = "Save connection info as...";
          break;
        case ID_DISCONNECT:
          TTStr->lpszText = "Disconnect";
          break;
      }
      return 0;
    }
    case WM_SETFOCUS:
      hotkeys.SetWindow(hwnd);
      SetFocus(_this->m_hwnd);
      return 0;
    case WM_COMMAND:
    case WM_SYSCOMMAND:
      switch (LOWORD(wParam)) {
        case SC_MINIMIZE:
          _this->SetDormant(true);
          break;
        case SC_RESTORE:
          _this->SetDormant(false);
          break;
        case ID_NEWCONN:
        {
          COND_UNGRAB_KEYBOARD
          _this->m_pApp->NewConnection();
          COND_REGRAB_KEYBOARD
          return 0;
        }
        case ID_DISCONNECT:
          SendMessage(hwnd, WM_CLOSE, 0, 0);
          return 0;
        case ID_TOOLBAR:
          if (GetMenuState(GetSystemMenu(_this->m_hwnd1, FALSE), ID_TOOLBAR,
                           MF_BYCOMMAND) == MF_CHECKED) {
            CheckMenuItem(GetSystemMenu(_this->m_hwnd1, FALSE), ID_TOOLBAR,
                          MF_BYCOMMAND | MF_UNCHECKED);
          } else {
            CheckMenuItem(GetSystemMenu(_this->m_hwnd1, FALSE), ID_TOOLBAR,
                          MF_BYCOMMAND | MF_CHECKED);
          }
          _this->SizeWindow(false);
          return 0;
        case ID_CONN_SAVE_AS:
        {
          COND_UNGRAB_KEYBOARD
          _this->SaveConnection();
          COND_REGRAB_KEYBOARD
          return 0;
        }
        case IDC_OPTIONBUTTON:
        {
          int prev_scale_num = _this->m_opts.m_scale_num;
          int prev_scale_den = _this->m_opts.m_scale_den;
          int prev_span = _this->m_opts.m_Span;
          bool prev_FullScreen = _this->m_opts.m_FullScreen;
          DesktopSize prev_desktopSize = _this->m_opts.m_desktopSize;

          COND_UNGRAB_KEYBOARD
          if (_this->m_opts.DoDialog(true)) {
            _this->m_pendingFormatChange = true;
            if (_this->m_opts.m_FitWindow) {
              _this->m_opts.m_scaling = true;
              _this->PositionChildWindow();
            } else {
              if (prev_scale_num != _this->m_opts.m_scale_num ||
                  prev_scale_den != _this->m_opts.m_scale_den ||
                  prev_span != _this->m_opts.m_Span ||
                  prev_desktopSize != _this->m_opts.m_desktopSize) {
                bool defaultSize = false;
                if (prev_desktopSize.mode != SIZE_AUTO &&
                    _this->m_opts.m_desktopSize.mode == SIZE_AUTO &&
                    prev_FullScreen == true &&
                    _this->m_opts.m_FullScreen == true)
                  defaultSize = true;
                // Resize the window if scaling factors or spanning mode or
                // desktop size were changed
                _this->SizeWindow(true, true, defaultSize);
                if (defaultSize)
                  _this->RealiseFullScreenMode(true);
                InvalidateRect(_this->m_hwnd, NULL, FALSE);
              }
            }
            if (prev_desktopSize != _this->m_opts.m_desktopSize)
              _this->m_firstUpdate = true;
            if (prev_FullScreen != _this->m_opts.m_FullScreen)
              _this->RealiseFullScreenMode(false);

            if ((_this->m_opts.m_GrabKeyboard == TVNC_ALWAYS && !regrab) ||
                (_this->m_opts.m_GrabKeyboard == TVNC_FS &&
                 (_this->m_opts.m_FullScreen != regrab)))
              regrab = !regrab;
          }

          if (_this->m_serverInitiated)
            _this->m_opts.SaveOpt(".listen", KEY_VNCVIEWER_HISTORY);
          else
            _this->m_opts.SaveOpt(_this->m_opts.m_display,
                                  KEY_VNCVIEWER_HISTORY);
          _this->EnableFullControlOptions();
          _this->EnableZoomOptions();
          hotkeys.Init(_this->m_opts.m_FSAltEnter,
                       _this->m_opts.m_desktopSize.mode != SIZE_AUTO &&
                       !_this->m_opts.m_FitWindow);
          _this->SetWindowTitle();
          if (SetForegroundWindow(_this->m_opts.m_hParent) != 0) return 0;
          COND_REGRAB_KEYBOARD
          return 0;
        }
        case IDD_APP_ABOUT:
        {
          COND_UNGRAB_KEYBOARD
          ShowAboutBox();
          COND_REGRAB_KEYBOARD
          return 0;
        }
        case IDD_FILETRANSFER:
        {
          COND_UNGRAB_KEYBOARD
          if (_this->m_clientMsgCaps.IsEnabled(rfbFileListRequest)) {
            if (!_this->m_fileTransferDialogShown) {
              _this->m_fileTransferDialogShown = true;
              _this->m_pFileTransfer->CreateFileTransferDialog();
            }
          }
          COND_REGRAB_KEYBOARD
          return 0;
        }
        case ID_CONN_ABOUT:
        {
          COND_UNGRAB_KEYBOARD
          _this->ShowConnInfo();
          COND_REGRAB_KEYBOARD
          return 0;
        }
        case ID_FULLSCREEN:
          _this->SetFullScreenMode(!_this->InFullScreenMode());
          return 0;
        case ID_FULLSCREEN_NODIALOG:
          _this->SetFullScreenMode(!_this->InFullScreenMode(), true);
          return 0;
        case ID_TOGGLE_GRAB:
          if (_this->isKeyboardGrabbed())
            _this->UngrabKeyboard(true);
          else
            _this->GrabKeyboard(true);
          return 0;
        case ID_TOGGLE_VIEWONLY:
          _this->m_opts.m_ViewOnly = !_this->m_opts.m_ViewOnly;
          CheckMenuItem(GetSystemMenu(_this->m_hwnd1, FALSE),
                        ID_TOGGLE_VIEWONLY,
                        MF_BYCOMMAND | (_this->m_opts.m_ViewOnly ?
                                        MF_CHECKED : MF_UNCHECKED));
          _this->EnableFullControlOptions();
          return 0;
        case ID_TOGGLE_CLIPBOARD:
          _this->m_opts.m_DisableClipboard = !_this->m_opts.m_DisableClipboard;
          CheckMenuItem(GetSystemMenu(_this->m_hwnd1, FALSE),
                        ID_TOGGLE_CLIPBOARD,
                        MF_BYCOMMAND | (_this->m_opts.m_DisableClipboard ?
                                        MF_UNCHECKED : MF_CHECKED));
          return 0;
        case ID_ZOOM_IN:
          if (_this->m_opts.m_desktopSize.mode != SIZE_AUTO &&
              !_this->m_opts.m_FitWindow) {
            int sf = (_this->m_opts.m_scale_num * 100) /
                     _this->m_opts.m_scale_den;
            if (sf < 100)
              sf = ((sf / 10) + 1) * 10;
            else if (sf >= 100 && sf <= 200)
              sf = ((sf / 25) + 1) * 25;
            else
              sf = ((sf / 50) + 1) * 50;
            if (sf > 400) sf = 400;
            _this->m_opts.m_scale_num = sf;
            _this->m_opts.m_scale_den = 100;
            _this->m_opts.m_scaling = (sf != 100);
            _this->SizeWindow(true, true, false);
            _this->SetWindowTitle();
            InvalidateRect(_this->m_hwnd, NULL, FALSE);
          }
          return 0;
        case ID_ZOOM_OUT:
          if (_this->m_opts.m_desktopSize.mode != SIZE_AUTO &&
              !_this->m_opts.m_FitWindow) {
            int sf = (_this->m_opts.m_scale_num * 100) /
                     _this->m_opts.m_scale_den;
            if (sf <= 100)
              sf = ((sf / 10) - 1) * 10;
            else if (sf >= 100 && sf <= 200)
              sf = ((sf / 25) - 1) * 25;
            else
              sf = ((sf / 50) - 1) * 50;
            if (sf < 10) sf = 10;
            _this->m_opts.m_scale_num = sf;
            _this->m_opts.m_scale_den = 100;
            _this->m_opts.m_scaling = (sf != 100);
            _this->SizeWindow(true, true, false);
            _this->SetWindowTitle();
            InvalidateRect(_this->m_hwnd, NULL, FALSE);
          }
          return 0;
        case ID_ZOOM_100:
          if (_this->m_opts.m_scaling) {
            _this->m_opts.m_scale_num = 100;
            _this->m_opts.m_scale_den = 100;
            _this->SizeWindow(true, true, false);
            _this->SetWindowTitle();
            InvalidateRect(_this->m_hwnd, NULL, FALSE);
          }
          return 0;
        case ID_DEFAULT_WINDOW_SIZE:
          // Reset window geometry to default (taking into account spanning
          // option)
          _this->SizeWindow(true, false, true);
          return 0;
        case ID_REQUEST_REFRESH:
          _this->SendFullFramebufferUpdateRequest();
          return 0;
        case ID_REQUEST_LOSSLESS_REFRESH:
        {
          int encoding = _this->m_opts.m_PreferredEncoding;
          int compressLevel = _this->m_opts.m_compressLevel;
          int qual = _this->m_opts.m_jpegQualityLevel;
          bool enablejpeg = _this->m_opts.m_enableJpegCompression;
          _this->m_opts.m_jpegQualityLevel = -1;
          _this->m_opts.m_enableJpegCompression = false;
          _this->m_opts.m_PreferredEncoding = rfbEncodingTight;
          _this->m_opts.m_compressLevel = 1;
          _this->m_pendingEncodingChange = true;
          _this->SendFullFramebufferUpdateRequest();
          _this->m_opts.m_jpegQualityLevel = qual;
          _this->m_opts.m_enableJpegCompression = enablejpeg;
          _this->m_opts.m_PreferredEncoding = encoding;
          _this->m_opts.m_compressLevel = compressLevel;
          _this->m_pendingEncodingChange = true;
          return 0;
        }
        case ID_CONN_SENDKEYDOWN:
          _this->SendKeyEvent((CARD32)lParam, true);
          return 0;
        case ID_CONN_SENDKEYUP:
          _this->SendKeyEvent((CARD32)lParam, false);
          return 0;
        case ID_CONN_CTLESC:
          _this->SendKeyEvent(XK_Control_L, true);
          _this->SendKeyEvent(XK_Escape,    true);
          _this->SendKeyEvent(XK_Escape,    false);
          _this->SendKeyEvent(XK_Control_L, false);
          return 0;
        case ID_CONN_ALTENTER:
          _this->SendKeyEvent(XK_Alt_L,  true);
          _this->SendKeyEvent(XK_Return, true);
          _this->SendKeyEvent(XK_Return, false);
          _this->SendKeyEvent(XK_Alt_L,  false);
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
          if (GetMenuState(GetSystemMenu(_this->m_hwnd1, FALSE),
                           ID_CONN_CTLDOWN, MF_BYCOMMAND) == MF_CHECKED) {
            _this->SendKeyEvent(XK_Control_L, false);
            CheckMenuItem(GetSystemMenu(_this->m_hwnd1, FALSE),
                          ID_CONN_CTLDOWN, MF_BYCOMMAND | MF_UNCHECKED);
            SendMessage(_this->m_hToolbar, TB_SETSTATE,
                        (WPARAM)ID_CONN_CTLDOWN,
                        (LPARAM)MAKELONG(TBSTATE_ENABLED, 0));
          } else {
            CheckMenuItem(GetSystemMenu(_this->m_hwnd1, FALSE),
                          ID_CONN_CTLDOWN, MF_BYCOMMAND | MF_CHECKED);
            SendMessage(_this->m_hToolbar, TB_SETSTATE,
                        (WPARAM)ID_CONN_CTLDOWN,
                        (LPARAM)MAKELONG(TBSTATE_CHECKED | TBSTATE_ENABLED,
                                         0));
            _this->SendKeyEvent(XK_Control_L, true);
          }
          return 0;
        case ID_CONN_ALTDOWN:
          if (GetMenuState(GetSystemMenu(_this->m_hwnd1, FALSE),
                           ID_CONN_ALTDOWN, MF_BYCOMMAND) == MF_CHECKED) {
            _this->SendKeyEvent(XK_Alt_L, false);
            CheckMenuItem(GetSystemMenu(_this->m_hwnd1, FALSE),
                          ID_CONN_ALTDOWN, MF_BYCOMMAND | MF_UNCHECKED);
            SendMessage(_this->m_hToolbar, TB_SETSTATE,
                        (WPARAM)ID_CONN_ALTDOWN,
                        (LPARAM)MAKELONG(TBSTATE_ENABLED, 0));
          } else {
            CheckMenuItem(GetSystemMenu(_this->m_hwnd1, FALSE),
                          ID_CONN_ALTDOWN, MF_BYCOMMAND | MF_CHECKED);
            SendMessage(_this->m_hToolbar, TB_SETSTATE,
                        (WPARAM)ID_CONN_ALTDOWN,
                        (LPARAM)MAKELONG(TBSTATE_CHECKED | TBSTATE_ENABLED,
                                         0));
            _this->SendKeyEvent(XK_Alt_L, true);
          }
          return 0;
        case ID_CLOSEDAEMON:
          if (MessageBox(NULL, "Are you sure you want to exit?",
                         "Closing VNCviewer",
                         MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES)
            PostQuitMessage(0);
          return 0;
      }
      break;
    case WM_KILLFOCUS:
      if (_this->m_opts.m_ViewOnly) return 0;
      _this->SwitchOffKeys();
      return 0;
    case WM_SIZE:
      _this->PositionChildWindow();
      return 0;
    case WM_MOVE:
      if (_this->m_checkLayout) {
        RECT clientRect;
        _this->GetActualClientRect(&clientRect);
        int w = WidthOf(clientRect), h = HeightOf(clientRect);
        ScreenSet layout = _this->ComputeScreenLayout(w, h);
        _this->m_checkLayout = false;
        if (w >= 1 && h >= 1 && layout != _this->m_screenLayout)
          _this->SendDesktopSize(w, h, layout);
      }
      return 0;
    case WM_CLOSE:
      // Close the worker thread as well
      _this->KillThread();
      DestroyWindow(hwnd);
      return 0;
    case WM_DESTROY:
    {
      // Remove us from the clipboard viewer chain
      BOOL res = ChangeClipboardChain(_this->m_hwnd, _this->m_hwndNextViewer);
      if (_this->m_serverInitiated)
        _this->m_opts.SaveOpt(".listen", KEY_VNCVIEWER_HISTORY);
      else
        _this->m_opts.SaveOpt(_this->m_opts.m_display, KEY_VNCVIEWER_HISTORY);

      if (_this->m_waitingOnEmulateTimer) {
        KillTimer(_this->m_hwnd, _this->m_emulate3ButtonsTimer);
        _this->m_waitingOnEmulateTimer = false;
      }

      if (_this->m_waitingOnResizeTimer) {
        KillTimer(_this->m_hwnd, _this->m_resizeTimer);
        _this->m_waitingOnResizeTimer = false;
      }

      _this->m_hwnd1 = 0;
      _this->m_hwnd = 0;
      _this->m_opts.m_hWindow = 0;

      if (_this->m_hctx) {
        gpWTClose(_this->m_hctx);
        _this->m_hctx = 0;
      }

      // We are currently in the main thread.
      // The worker thread should be about to finish if
      // it hasn't already. Wait for it.
      try {
        void *p;
        _this->join(&p);  // After joining, _this is no longer valid
      } catch (omni_thread_invalid) {
        // The thread probably hasn't been started yet,
      }
      return 0;
    }
  }
  return DefWindowProc(hwnd, iMsg, wParam, lParam);
}


LRESULT CALLBACK ClientConnection::Proc(HWND hwnd, UINT iMsg, WPARAM wParam,
                                        LPARAM lParam)
{
  return DefWindowProc(hwnd, iMsg, wParam, lParam);
}


LRESULT CALLBACK ClientConnection::WndProc(HWND hwnd, UINT iMsg, WPARAM wParam,
                                           LPARAM lParam)
{
  // This is a static method, so we don't know which instantiation we're
  // dealing with.  But we've stored a 'pseudo-this' in the window data.
  ClientConnection *_this =
    (ClientConnection *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
  char *env = NULL;  size_t envlen;  static int x = 100, y = 100;

  switch (iMsg) {
    case WM_FBUPDATERECVD:
      _this->SendAppropriateFramebufferUpdateRequest();
      if (_dupenv_s(&env, &envlen, "TVNC_FAKEMOUSE") == 0 && env != NULL) {
        if (!strcmp(env, "1")) {
          _this->SendPointerEvent(x, y, rfbButton1Mask | rfbButton3Mask);
          if (x == 100) x = 101;  else x = 100;
          if (y == 100) y = 101;  else y = 100;
        }
        free(env);
      }
      return 0;
    case WM_REGIONUPDATED:
      if (!_this->m_opts.m_benchFile) _this->DoBlit();
      return 0;
    case WM_PAINT:
      if (!_this->m_opts.m_benchFile) _this->DoBlit();
      return 0;
    case WM_TIMER:
      if (wParam == _this->m_emulate3ButtonsTimer) {
        _this->SubProcessPointerEvent(_this->m_emulateButtonPressedX,
                                      _this->m_emulateButtonPressedY,
                                      _this->m_emulateKeyFlags);
        KillTimer(hwnd, _this->m_emulate3ButtonsTimer);
        _this->m_waitingOnEmulateTimer = false;
      }
      if (wParam == _this->m_resizeTimer) {
        _this->SendDesktopSize(_this->m_autoResizeWidth,
                               _this->m_autoResizeHeight);
        KillTimer(hwnd, _this->m_resizeTimer);
        _this->m_waitingOnResizeTimer = false;
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
      if (GetFocus() != hwnd && GetFocus() != _this->m_hwnd1)
        return 0;
      SetFocus(hwnd);

      POINT coords;
      coords.x = (LONG)(short)LOWORD(lParam);
      coords.y = (LONG)(short)HIWORD(lParam);

      if (iMsg == WM_MOUSEWHEEL) {
        // Convert coordinates to position in our client area.
        // Make sure the pointer is inside the client area.
        if (WindowFromPoint(coords) != hwnd ||
            !ScreenToClient(hwnd, &coords) ||
            coords.x < 0 || coords.y < 0 ||
            coords.x >= _this->m_cliwidth || coords.y >= _this->m_cliheight)
          return 0;
      } else {
        // Make sure the high-order word in wParam is zero.
        wParam = MAKEWPARAM(LOWORD(wParam), 0);
      }

      if (_this->InFullScreenMode()) {
        if (_this->BumpScroll(coords.x, coords.y))
          return 0;
      }
      if (_this->m_opts.m_ViewOnly)
        return 0;

      _this->ProcessPointerEvent(coords.x, coords.y, (DWORD)wParam, iMsg);
      return 0;
    }

    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    {
      if (!_this->m_running) return 0;
      if (_this->m_opts.m_ViewOnly) return 0;
      bool down = (((DWORD)lParam & 0x80000000l) == 0);
      if ((int)wParam == 0x11) {
        if (!down) {
          CheckMenuItem(GetSystemMenu(_this->m_hwnd1, FALSE), ID_CONN_CTLDOWN,
                        MF_BYCOMMAND | MF_UNCHECKED);
          SendMessage(_this->m_hToolbar, TB_SETSTATE, (WPARAM)ID_CONN_CTLDOWN,
                      (LPARAM)MAKELONG(TBSTATE_ENABLED, 0));
        } else {
          CheckMenuItem(GetSystemMenu(_this->m_hwnd1, FALSE), ID_CONN_CTLDOWN,
                        MF_BYCOMMAND | MF_CHECKED);
          SendMessage(_this->m_hToolbar, TB_SETSTATE, (WPARAM)ID_CONN_CTLDOWN,
                      (LPARAM)MAKELONG(TBSTATE_CHECKED | TBSTATE_ENABLED, 0));
        }
      }
      if ((int)wParam == 0x12) {
        if (!down) {
          CheckMenuItem(GetSystemMenu(_this->m_hwnd1, FALSE), ID_CONN_ALTDOWN,
                        MF_BYCOMMAND | MF_UNCHECKED);
          SendMessage(_this->m_hToolbar, TB_SETSTATE, (WPARAM)ID_CONN_ALTDOWN,
                      (LPARAM)MAKELONG(TBSTATE_ENABLED, 0));
        } else {
          CheckMenuItem(GetSystemMenu(_this->m_hwnd1, FALSE), ID_CONN_ALTDOWN,
                        MF_BYCOMMAND | MF_CHECKED);
          SendMessage(_this->m_hToolbar, TB_SETSTATE, (WPARAM)ID_CONN_ALTDOWN,
                      (LPARAM)MAKELONG(TBSTATE_CHECKED | TBSTATE_ENABLED, 0));
        }
      }

      _this->ProcessKeyEvent((int)wParam, (DWORD)lParam);
      return 0;
    }
    case WM_CHAR:
    case WM_SYSCHAR:
    case WM_DEADCHAR:
    case WM_SYSDEADCHAR:
      return 0;
    case WM_SETFOCUS:
      if (_this->InFullScreenMode())
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 100, 100,
                     SWP_NOMOVE | SWP_NOSIZE);

      if (IsWindowVisible(_this->m_hwnd1)) {
        if (_this->shouldGrab() && !_this->isKeyboardGrabbed())
          vnclog.Print(6, "Keyboard focus regained. Re-grabbing keyboard.\n");
        else if (!_this->shouldGrab() && LowLevelHook::isActive())
          vnclog.Print(6, "Keyboard focus regained. Ungrabbing keyboard.\n");
        if (_this->shouldGrab()) _this->GrabKeyboard(true);
        else _this->UngrabKeyboard(true);
      }

      return 0;
    // Cancel modifiers when we lose focus
    case WM_KILLFOCUS:
    {
      if (!_this->m_running) return 0;
      if (_this->InFullScreenMode()) {
        // We must stop being topmost, but we want to choose our
        // position carefully.
        HWND foreground = GetForegroundWindow();
        HWND hwndafter = NULL;
        if ((foreground == NULL) ||
            (GetWindowLong(foreground, GWL_EXSTYLE) & WS_EX_TOPMOST))
          hwndafter = HWND_NOTOPMOST;
        else
          hwndafter = GetNextWindow(foreground, GW_HWNDNEXT);

        SetWindowPos(_this->m_hwnd1, hwndafter, 0, 0, 100, 100,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
      }
      vnclog.Print(6, "Losing focus - cancelling modifiers\n");
      _this->SwitchOffKeys();

      DWORD pid = 0;
      GetWindowThreadProcessId((HWND)wParam, &pid);

      if (LowLevelHook::isActive() && IsWindowVisible(_this->m_hwnd1) &&
          pid != GetProcessId(GetCurrentProcess())) {
        vnclog.Print(6, "Keyboard focus lost. Temporarily ungrabbing keyboard.\n");
        _this->UngrabKeyboard(false);
      }

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
      if ((HWND)wParam != hwnd) {
        // Need the window's DC for SelectPalette/RealizePalette
        TempDC hDC(hwnd);
        PaletteSelector p(hDC, _this->m_hPalette);
        // When updating the colors for an inactive window,
        // UpdateColors can be called because it is faster than
        // redrawing the client area (even though the results are
        // not as good)
        UpdateColors(hDC);
      }
      break;

    case WM_SETCURSOR:
    {
      // if we have the focus, let the cursor change as normal
      if (GetFocus() == hwnd)
        break;

      // if not, set to default system cursor
      SetCursor(LoadCursor(NULL, IDC_ARROW));
      return 0;
    }

    case WM_DRAWCLIPBOARD:
      if (!_this->m_opts.m_benchFile) _this->ProcessLocalClipboardChange();
      return 0;

    case WM_CHANGECBCHAIN:
    {
      // The clipboard chain is changing
      HWND hWndRemove = (HWND)wParam;      // handle of window being removed
      HWND hWndNext = (HWND)lParam;        // handle of next window in chain
      // If next window is closing, update our pointer.
      if (hWndRemove == _this->m_hwndNextViewer)
        _this->m_hwndNextViewer = hWndNext;
      // Otherwise, pass the message to the next link.
      else if (_this->m_hwndNextViewer != NULL)
        SendMessage(_this->m_hwndNextViewer, WM_CHANGECBCHAIN,
                    (WPARAM)hWndRemove, (LPARAM)hWndNext);
      return 0;
    }

    case WT_PACKET:
    {
      PACKET pkt;
      if (gpWTPacket((HCTX)lParam, (UINT)wParam, &pkt)) {
        ExtInputEvent e;

        if (pkt.pkChanged & PK_X || pkt.pkChanged & PK_Y ||
            pkt.pkChanged & PK_NORMAL_PRESSURE ||
            pkt.pkChanged & PK_ORIENTATION) {
          e.type = rfbGIIValuatorAbsolute;
          e.deviceID = pkt.pkCursor;
          e.numValuators = 5;
          _this->TranslateWacomCoords(hwnd, pkt.pkX, pkt.pkY, e.valuators[0],
                                      e.valuators[1]);
          e.valuators[2] = pkt.pkNormalPressure;
          _this->ConvertWacomTilt(pkt.pkOrientation, e.valuators[3],
                                  e.valuators[4]);
          _this->SendGIIEvent(pkt.pkCursor, e);
        }

        if (_this->m_wacomButtonMask != pkt.pkButtons &&
            pkt.pkChanged & PK_BUTTONS) {
          int maxButtons = 0;
          gpWTInfoA(WTI_CURSORS + pkt.pkCursor, CSR_BUTTONS, &maxButtons);
          for (int button = 0; button < maxButtons; button++) {
            if (pkt.pkButtons & (1 << button) &&
                !(_this->m_wacomButtonMask & (1 << button))) {
              e.type = rfbGIIButtonPress;
              e.buttonNumber = button + 1;
              _this->SendGIIEvent(pkt.pkCursor, e);
            } else if (!(pkt.pkButtons & (1 << button)) &&
                       _this->m_wacomButtonMask & (1 << button)) {
              e.type = rfbGIIButtonRelease;
              e.buttonNumber = button + 1;
              _this->SendGIIEvent(pkt.pkCursor, e);
            }
          }
          _this->m_wacomButtonMask = pkt.pkButtons;
        }
      }
      return 0;
    }
  }

  return DefWindowProc(hwnd, iMsg, wParam, lParam);
}


// ProcessPointerEvent handles the delicate case of emulating 3 buttons
// on a two button mouse, then passes events off to SubProcessPointerEvent.

void ClientConnection::ProcessPointerEvent(int x, int y, DWORD keyflags,
                                           UINT msg)
{
  if (m_opts.m_Emul3Buttons) {
    // XXX To be done:
    // If this is a left or right press, the user may be
    // about to press the other button to emulate a middle press.
    // We need to start a timer, and if it expires without any
    // further presses, then we send the button press.
    // If a press of the other button, or any release, comes in
    // before timer has expired, we cancel timer & take different action.
    if (m_waitingOnEmulateTimer) {
      if (msg == WM_LBUTTONUP || msg == WM_RBUTTONUP ||
          abs(x - m_emulateButtonPressedX) > m_opts.m_Emul3Fuzz ||
          abs(y - m_emulateButtonPressedY) > m_opts.m_Emul3Fuzz) {
        // if button was released or we moved too far, then cancel.
        // First let the server know where the button was pressed
        SubProcessPointerEvent(m_emulateButtonPressedX,
                               m_emulateButtonPressedY, m_emulateKeyFlags);
        // Then tell it where we are now
        SubProcessPointerEvent(x, y, keyflags);
      } else if ((msg == WM_LBUTTONDOWN && (m_emulateKeyFlags & MK_RBUTTON)) ||
                 (msg == WM_RBUTTONDOWN && (m_emulateKeyFlags & MK_LBUTTON))) {
        // Triggered an emulate;  remove left and right buttons, and put
        // in middle one.
        DWORD emulatekeys = keyflags & ~(MK_LBUTTON | MK_RBUTTON);
        emulatekeys |= MK_MBUTTON;
        SubProcessPointerEvent(x, y, emulatekeys);

        m_emulatingMiddleButton = true;
      } else {
        // handle movement normally & don't kill timer;
        // just remove the pressed button from the mask.
        DWORD keymask = m_emulateKeyFlags & (MK_LBUTTON | MK_RBUTTON);
        DWORD emulatekeys = keyflags & ~keymask;
        SubProcessPointerEvent(x, y, emulatekeys);
        return;
      }

      // if we reached here, we don't need the timer anymore.
      KillTimer(m_hwnd, m_emulate3ButtonsTimer);
      m_waitingOnEmulateTimer = false;
    } else if (m_emulatingMiddleButton) {
      if ((keyflags & MK_LBUTTON) == 0 && (keyflags & MK_RBUTTON) == 0) {
        // We finish emulation only when both buttons come back up.
        m_emulatingMiddleButton = false;
        SubProcessPointerEvent(x, y, keyflags);
      } else {
        // keep emulating.
        DWORD emulatekeys = keyflags & ~(MK_LBUTTON | MK_RBUTTON);
        emulatekeys |= MK_MBUTTON;
        SubProcessPointerEvent(x, y, emulatekeys);
      }
    } else {
      // Start considering emulation if we've pressed a button
      // and the other isn't pressed.
      if ((msg == WM_LBUTTONDOWN && !(keyflags & MK_RBUTTON)) ||
          (msg == WM_RBUTTONDOWN && !(keyflags & MK_LBUTTON))) {
        // Start timer for emulation.
        m_emulate3ButtonsTimer = SetTimer(m_hwnd, IDT_EMULATE3BUTTONSTIMER,
                                          m_opts.m_Emul3Timeout, NULL);

        if (!m_emulate3ButtonsTimer) {
          vnclog.Print(0, "Failed to create timer for emulating 3 buttons");
          PostMessage(m_hwnd1, WM_CLOSE, 0, 0);
          return;
        }

        m_waitingOnEmulateTimer = true;

        // Note that we don't send the event here;  we're batching it for
        // later.
        m_emulateKeyFlags = keyflags;
        m_emulateButtonPressedX = x;
        m_emulateButtonPressedY = y;
      } else {
        // just send event noramlly
        SubProcessPointerEvent(x, y, keyflags);
      }
    }
  } else {
    SubProcessPointerEvent(x, y, keyflags);
  }
}


// SubProcessPointerEvent takes windows positions and flags and converts
// them into VNC ones.

inline void ClientConnection::SubProcessPointerEvent(int x, int y,
                                                     DWORD keyflags)
{
  int mask;

  if (m_opts.m_SwapMouse)
    mask = (((keyflags & MK_LBUTTON) ? rfbButton1Mask : 0) |
            ((keyflags & MK_MBUTTON) ? rfbButton3Mask : 0) |
            ((keyflags & MK_RBUTTON) ? rfbButton2Mask : 0));
  else
    mask = (((keyflags & MK_LBUTTON) ? rfbButton1Mask : 0) |
            ((keyflags & MK_MBUTTON) ? rfbButton2Mask : 0) |
            ((keyflags & MK_RBUTTON) ? rfbButton3Mask : 0));

  if ((short)HIWORD(keyflags) > 0)
    mask |= rfbButton4Mask;
  else if ((short)HIWORD(keyflags) < 0)
    mask |= rfbButton5Mask;

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
    PostMessage(m_hwnd1, WM_CLOSE, 1, 0);
  }
}


inline void ClientConnection::SendPointerEvent(int x, int y, int buttonMask)
{
  rfbPointerEventMsg pe;

  if (m_opts.m_benchFile) return;

  pe.type = rfbPointerEvent;
  pe.buttonMask = buttonMask;
  if (x < 0) x = 0;
  if (y < 0) y = 0;
  SoftCursorMove(x, y);
  pe.x = Swap16IfLE(x);
  pe.y = Swap16IfLE(y);
  WriteExact((char *)&pe, sz_rfbPointerEventMsg);
}


// Normally, a single Windows key event will map onto a single RFB
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
//    a German user types this combination, the user doesn't mean Ctrl-@.
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

  // if virtkey is found in mapping table, send X equivalent
  // else
  //   try to convert directly to ASCII
  //   if result is in the range supported by X keysyms,
  //      raise any modifiers, send it, then restore mods
  //   else
  //      calculate what the ASCII code would be without mods, and
  //      send that

#ifdef _DEBUG
  char keyname[32];
  if (GetKeyNameText(keyData, keyname, 31))
    vnclog.Print(4, "Process key: %s (keyData %04x): ", keyname, keyData);
#endif

  try {

    if (!down) {
      std::map<UINT, KeyActionSpec>::iterator iter = pressedKeys.find(virtkey);

      if (iter == pressedKeys.end()) {
        vnclog.Print(4, "Unexpected release of key code %d\n", virtkey);
        return;
      }

      KeyActionSpec syms = iter->second;
      for (int i = 0; syms.keycodes[i] != XK_VoidSymbol && i < MaxKeysPerKey;
           i++) {
        SendKeyEvent(syms.keycodes[i], false);
        vnclog.Print(4, "Sent keysym %04x (release)\n", syms.keycodes[i]);
      }
      pressedKeys.erase(iter);
      return;
    }

    KeyActionSpec kas = m_keymap.PCtoX(virtkey, keyData);

    if (kas.releaseModifiers & KEYMAP_LCONTROL) {
      SendKeyEvent(XK_Control_L, false);
      vnclog.Print(5, "fake L Ctrl released\n");
    }
    if (kas.releaseModifiers & KEYMAP_LALT) {
      SendKeyEvent(XK_Alt_L, false);
      vnclog.Print(5, "fake L Alt released\n");
    }
    if (kas.releaseModifiers & KEYMAP_RCONTROL) {
      SendKeyEvent(XK_Control_R, false);
      vnclog.Print(5, "fake R Ctrl released\n");
    }
    if (kas.releaseModifiers & KEYMAP_RALT) {
      SendKeyEvent(XK_Alt_R, false);
      vnclog.Print(5, "fake R Alt released\n");
    }

    pressedKeys[virtkey] = kas;
    for (int i = 0; kas.keycodes[i] != XK_VoidSymbol && i < MaxKeysPerKey;
         i++) {
      SendKeyEvent(kas.keycodes[i], true);
      vnclog.Print(4, "Sent keysym %04x (press)\n", kas.keycodes[i]);
    }

    if (kas.releaseModifiers & KEYMAP_RALT) {
      SendKeyEvent(XK_Alt_R, true);
      vnclog.Print(5, "fake R Alt pressed\n");
    }
    if (kas.releaseModifiers & KEYMAP_RCONTROL) {
      SendKeyEvent(XK_Control_R, true);
      vnclog.Print(5, "fake R Ctrl pressed\n");
    }
    if (kas.releaseModifiers & KEYMAP_LALT) {
      SendKeyEvent(XK_Alt_L, true);
      vnclog.Print(5, "fake L Alt pressed\n");
    }
    if (kas.releaseModifiers & KEYMAP_LCONTROL) {
      SendKeyEvent(XK_Control_L, true);
      vnclog.Print(5, "fake L Ctrl pressed\n");
    }
  } catch (Exception &e) {
    e.Report();
    PostMessage(m_hwnd1, WM_CLOSE, 1, 0);
  }
}


inline void ClientConnection::SendKeyEvent(CARD32 key, bool down)
{
  rfbKeyEventMsg ke;

  if (m_opts.m_ViewOnly) return;

  ke.type = rfbKeyEvent;
  ke.down = down ? 1 : 0;
  ke.key = Swap32IfLE(key);
  WriteExact((char *)&ke, sz_rfbKeyEventMsg);
  vnclog.Print(6, "SendKeyEvent: key = x%04x status = %s\n", key,
               down ? "down" : "up");
}


inline void ClientConnection::GrabKeyboard(bool changeMenu)
{
  LowLevelHook::Activate(m_hwnd1);
  if (changeMenu)
    CheckMenuItem(GetSystemMenu(m_hwnd1, FALSE), ID_TOGGLE_GRAB,
                  MF_BYCOMMAND | MF_CHECKED);
}


inline void ClientConnection::UngrabKeyboard(bool changeMenu)
{
  LowLevelHook::Deactivate();
  if (changeMenu)
    CheckMenuItem(GetSystemMenu(m_hwnd1, FALSE), ID_TOGGLE_GRAB,
                  MF_BYCOMMAND | MF_UNCHECKED);
}


inline bool ClientConnection::isGrabSelected(void)
{
  return GetMenuState(GetSystemMenu(m_hwnd1, FALSE), ID_TOGGLE_GRAB,
                      MF_BYCOMMAND) == MF_CHECKED;
}


inline bool ClientConnection::isKeyboardGrabbed(void)
{
  return LowLevelHook::isActive(m_hwnd1);
}


inline bool ClientConnection::shouldGrab(void)
{
  return m_opts.m_GrabKeyboard == TVNC_ALWAYS ||
         (m_opts.m_GrabKeyboard == TVNC_MANUAL && isGrabSelected()) ||
         (m_opts.m_GrabKeyboard == TVNC_FS && InFullScreenMode());
}


void ClientConnection::SendClientCutText(char *str, size_t len)
{
  rfbClientCutTextMsg cct;

  cct.type = rfbClientCutText;
  cct.length = Swap32IfLE(len);
  omni_mutex_lock l(m_writeMutex);  // Ensure back-to-back writes are grouped
  WriteExact((char *)&cct, sz_rfbClientCutTextMsg);
  WriteExact(str, (int)len);
  vnclog.Print(6, "Sent %d bytes of clipboard\n", len);
}


// Copy any updated areas from the bitmap onto the screen.

inline void ClientConnection::DoBlit()
{
  double tBlitStart;

  if (m_hBitmap == NULL) return;
  if (!m_running) return;

  // No other threads can use bitmap DC
  omni_mutex_lock l(m_bitmapdcMutex);

  if (m_opts.m_benchFile) tBlitStart = getTime();

  PAINTSTRUCT ps;
  HDC hdc = BeginPaint(m_hwnd, &ps);

  // Select and realize hPalette
  PaletteSelector p(hdc, m_hPalette);
  ObjectSelector b(m_hBitmapDC, m_hBitmap);

  if (m_opts.m_delay) {
    // Display the area to be updated for debugging purposes
    COLORREF oldbgcol = SetBkColor(hdc, RGB(0, 0, 0));
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &ps.rcPaint, NULL, 0, NULL);
    SetBkColor(hdc, oldbgcol);
    Sleep(m_pApp->m_options.m_delay);
  }

  if (m_opts.m_scaling) {
    int n = m_opts.m_scale_num;
    int d = m_opts.m_scale_den;

    // We're about to do some scaling on these values before calling
    // StretchBlt().  We want to make sure that they divide nicely by n, so we
    // round them down and up appropriately.
    ps.rcPaint.left =   ((ps.rcPaint.left   + m_hScrollPos) / n * n) -
                        m_hScrollPos;
    ps.rcPaint.right =  ((ps.rcPaint.right  + m_hScrollPos + n - 1) / n * n) -
                        m_hScrollPos;
    ps.rcPaint.top =    ((ps.rcPaint.top    + m_vScrollPos) / n * n) -
                        m_vScrollPos;
    ps.rcPaint.bottom = ((ps.rcPaint.bottom + m_vScrollPos + n - 1) / n * n) -
                        m_vScrollPos;

    // This is supposed to give better results.  I think my driver ignores it?
    SetStretchBltMode(hdc, HALFTONE);

    // The docs say that you should call SetBrushOrgEx after SetStretchBltMode,
    // but they don't say what the arguments should be.
    SetBrushOrgEx(hdc, 0, 0, NULL);

    if (!StretchBlt(hdc, ps.rcPaint.left, ps.rcPaint.top,
                    ps.rcPaint.right - ps.rcPaint.left,
                    ps.rcPaint.bottom - ps.rcPaint.top,
                    m_hBitmapDC,
                    (ps.rcPaint.left + m_hScrollPos)     * d / n,
                    (ps.rcPaint.top + m_vScrollPos)      * d / n,
                    (ps.rcPaint.right - ps.rcPaint.left) * d / n,
                    (ps.rcPaint.bottom - ps.rcPaint.top) * d / n,
                    SRCCOPY)) {
      vnclog.Print(0, "Blit error %d\n", GetLastError());
      // throw ErrorException("Error in blit!\n");
    }
  } else {
    if (!BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top,
                ps.rcPaint.right - ps.rcPaint.left,
                ps.rcPaint.bottom - ps.rcPaint.top, m_hBitmapDC,
                ps.rcPaint.left + m_hScrollPos,
                ps.rcPaint.top + m_vScrollPos, SRCCOPY)) {
      vnclog.Print(0, "Blit error %d\n", GetLastError());
      // throw ErrorException("Error in blit!\n");
    }
  }

  EndPaint(m_hwnd, &ps);

  if (m_opts.m_benchFile) {
    tBlit += getTime() - tBlitStart;
    blitPixels += (ps.rcPaint.right - ps.rcPaint.left) *
                  (ps.rcPaint.bottom - ps.rcPaint.top);
    blits++;
  }
}


inline void ClientConnection::UpdateScrollbars()
{
  // We don't update the actual scrollbar info in full-screen mode,
  // because it causes them to flicker.
  bool setInfo = !InFullScreenMode();

  SCROLLINFO scri;
  scri.cbSize = sizeof(scri);
  scri.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
  scri.nMin = 0;
  scri.nMax = m_hScrollMax;
  scri.nPage = m_cliwidth;
  scri.nPos = m_hScrollPos;

  if (setInfo)
    SetScrollInfo(m_hwndscroll, SB_HORZ, &scri, TRUE);

  scri.cbSize = sizeof(scri);
  scri.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
  scri.nMin = 0;
  scri.nMax = m_vScrollMax;
  scri.nPage = m_cliheight;
  scri.nPos = m_vScrollPos;

  if (setInfo)
    SetScrollInfo(m_hwndscroll, SB_VERT, &scri, TRUE);
}


void ClientConnection::ShowConnInfo()
{
  char buf[2048];
  char kbdname[9];
  GetKeyboardLayoutName(kbdname);
  SPRINTF(buf,
          "Connected to:  %s\n\r"
          "Host:  %s  port:  %d\n\r\n\r"
          "Desktop geometry:  %d x %d x %d\n\r"
          "Using depth:  %d\n\r"
          "Continuous updates:  %s\n\r"
          "Current protocol version:  3.%d%s\n\r\n\r"
          "Current keyboard name:  %s\n\r",
          m_desktopName, m_host, m_port, m_si.framebufferWidth,
          m_si.framebufferHeight, m_si.format.depth, m_myFormat.depth,
          continuousUpdates ? "Enabled" : "Disabled",
          m_minorVersion, (m_tightVncProtocol ? "tight" : ""), kbdname);
  MessageBox(NULL, buf, "VNC connection info", MB_ICONINFORMATION | MB_OK);
}


// ********************************************************************
//  Methods after this point are generally called by the worker thread.
//  They finish the initialisation, then chiefly read data from the server.
// ********************************************************************

void *ClientConnection::run_undetached(void *arg)
{

  vnclog.Print(9, "Update-processing thread started\n");

  m_threadStarted = true;

  try {

    SendFullFramebufferUpdateRequest();

    RealiseFullScreenMode(false);

    m_running = true;
    UpdateWindow(m_hwnd1);

    while (!m_bKillThread) {

      // Look at the type of the message, but leave it in the buffer
      CARD8 msgType;
      if (m_opts.m_benchFile) {
        omni_mutex_lock l(m_readMutex);  // we need this if we're not using ReadExact
        size_t bytes = fread((char *)&msgType, 1, 1, m_opts.m_benchFile);
        if (bytes == 0)
          throw QuietException("End of session capture");
        if (bytes < 0) {
          vnclog.Print(3, "Error reading session capture: %d\n",
                       GetLastError());
          throw WarningException("Error reading session capture");
        }
        fseek(m_opts.m_benchFile, -1, SEEK_CUR);
      } else {
        omni_mutex_lock l(m_readMutex);  // we need this if we're not using ReadExact
        int bytes = recv(m_sock, (char *)&msgType, 1, MSG_PEEK);
        if (bytes == 0) {
          m_pFileTransfer->CloseUndoneFileTransfers();
          vnclog.Print(0, "Connection closed\n");
          throw WarningException("Connection closed");
        }
        if (bytes < 0) {
          m_pFileTransfer->CloseUndoneFileTransfers();
          vnclog.Print(3, "Socket error reading message: %d\n",
                       WSAGetLastError());
          throw WarningException("Error while waiting for server message");
        }
      }

      switch (msgType) {
        case rfbFramebufferUpdate:
          ReadScreenUpdate();
          break;
        case rfbSetColourMapEntries:
          ReadSetColorMapEntries();
          break;
        case rfbBell:
          ReadBell();
          break;
        case rfbServerCutText:
          ReadServerCutText();
          break;
        case rfbFileListData:
          m_pFileTransfer->ShowServerItems();
          break;
        case rfbFileDownloadData:
          m_pFileTransfer->FileTransferDownload();
          break;
        case rfbFileUploadCancel:
          m_pFileTransfer->ReadUploadCancel();
          break;
        case rfbFileDownloadFailed:
          m_pFileTransfer->ReadDownloadFailed();
          break;
        case rfbFence:
          ReadFence();
          break;
        case rfbEndOfContinuousUpdates:
        {
          // Consume the message type byte
          char dummy;
          ReadExact(&dummy, 1);
          supportsCU = true;
          break;
        }
        case rfbGIIServer:
          ReadGII();
          break;

        default:
          vnclog.Print(3, "Unknown message type x%02x\n", msgType);
          throw WarningException("Unhandled message type received!\n");
      }

    }

    vnclog.Print(4, "Update-processing thread finishing\n");

  } catch (WarningException &e) {
    m_running = false;
    if (!m_bKillThread) {
      PostMessage(m_hwnd1, WM_CLOSE, 1, 0);
      e.Report();
    } else
      PostMessage(m_hwnd1, WM_CLOSE, 0, 0);
  } catch (ErrorException &e) {
    m_running = false;
    e.Report();
    PostMessage(m_hwnd1, WM_CLOSE, 1, 0);
  } catch (QuietException &e) {
    m_running = false;
    e.Report();
    if (!strcmp(e.m_info, "End of session capture"))
      PostMessage(m_hwnd1, WM_CLOSE, 0, 0);
    else
      PostMessage(m_hwnd1, WM_CLOSE, 1, 0);
  }
  return this;
}


inline void ClientConnection::SendFramebufferUpdateRequest(int x, int y,
                                                           int w, int h,
                                                           bool incremental)
{
  rfbFramebufferUpdateRequestMsg fur;

  if (incremental && continuousUpdates)
    return;

  if (m_pendingEncodingChange) {
    SetFormatAndEncodings();
    m_pendingEncodingChange = false;
  }

  if (m_opts.m_benchFile) return;

  fur.type = rfbFramebufferUpdateRequest;
  fur.incremental = incremental ? 1 : 0;
  fur.x = Swap16IfLE(x);
  fur.y = Swap16IfLE(y);
  fur.w = Swap16IfLE(w);
  fur.h = Swap16IfLE(h);

  vnclog.Print(10, "Request %s update\n",
               incremental ? "incremental" : "full");
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
    vnclog.Print(3, "Requesting new pixel format\n");
    rfbPixelFormat oldFormat = m_myFormat;
    SetupPixelFormat();
    SoftCursorFree();
    SetFormatAndEncodings();
    m_pendingFormatChange = false;
    // If the pixel format has changed, request whole screen
    if (!PF_EQ(m_myFormat, oldFormat))
      SendFullFramebufferUpdateRequest();
    else
      SendIncrementalFramebufferUpdateRequest();
  } else {
    if (!m_dormant)
      SendIncrementalFramebufferUpdateRequest();
  }
}


void ClientConnection::ReadScreenUpdate()
{
  rfbFramebufferUpdateMsg sut;
  double tDecodeStart, tBlitStart, tReadOld, tBlitOld;

  ReadExact((char *)&sut, sz_rfbFramebufferUpdateMsg);
  sut.nRects = Swap16IfLE(sut.nRects);
  if (sut.nRects == 0) return;

  if (m_opts.m_DoubleBuffer) list = NULL;

  PostMessage(m_hwnd, WM_FBUPDATERECVD, 0, 0);

  updates++;

  for (int i = 0; i < sut.nRects; i++) {

    rfbFramebufferUpdateRectHeader surh;
    ReadExact((char *)&surh, sz_rfbFramebufferUpdateRectHeader);

    surh.encoding = Swap32IfLE(surh.encoding);
    surh.r.x = Swap16IfLE(surh.r.x);
    surh.r.y = Swap16IfLE(surh.r.y);
    surh.r.w = Swap16IfLE(surh.r.w);
    surh.r.h = Swap16IfLE(surh.r.h);

    if (surh.encoding == rfbEncodingLastRect) {
      if (m_opts.m_benchFile) tBlitStart = getTime();

      while (m_opts.m_DoubleBuffer && list != NULL) {
        rfbFramebufferUpdateRectHeader *r1;
        node = list;
        r1 = &node->region;

        if (r1->encoding == rfbEncodingTight ||
            r1->encoding == rfbEncodingRaw ||
            r1->encoding == rfbEncodingHextile) {
          SoftCursorLockArea(r1->r.x, r1->r.y, r1->r.w, r1->r.h);
          if (node->isFill) {
            omni_mutex_lock l(m_bitmapdcMutex);
            ObjectSelector b(m_hBitmapDC, m_hBitmap);
            PaletteSelector p(m_hBitmapDC, m_hPalette);
            FillSolidRect(r1->r.x, r1->r.y, r1->r.w, r1->r.h, node->fillColor);
          }
          RECT rect;
          SetRect(&rect, r1->r.x, r1->r.y,
                  r1->r.x + r1->r.w, r1->r.y + r1->r.h);
          InvalidateScreenRect(&rect);
        }
        list = list->next;
        free(node);
      }
      SoftCursorUnlockScreen();

      if (m_opts.m_benchFile) tBlit += getTime() - tBlitStart;
      break;
    }

    if (surh.encoding == rfbEncodingNewFBSize) {
      ReadNewFBSize(&surh);
      break;
    }

    if (surh.encoding == rfbEncodingExtendedDesktopSize) {
      ReadExtendedDesktopSize(&surh);
      continue;
    }

    if (surh.encoding == rfbEncodingXCursor ||
        surh.encoding == rfbEncodingRichCursor) {
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

    if (m_opts.m_DoubleBuffer) {
      node = (UpdateList *)malloc(sizeof(UpdateList));
      node->next = NULL;
      node->isFill = 0;
      memcpy(&(node->region), &surh, sz_rfbFramebufferUpdateRectHeader);
      if (list == NULL)
        tail = list = node;
      else {
        tail->next = node;
        tail = node;
      }
    }

    if (m_opts.m_benchFile) {
      tDecodeStart = getTime();
      tReadOld = tRead;
      tBlitOld = tBlit;
      decodePixels += surh.r.w * surh.r.h;
      decodeRect++;
    }

    switch (surh.encoding) {
      case rfbEncodingRaw:
        SetLastEncoding(surh.encoding);
        ReadRawRect(&surh);
        break;
      case rfbEncodingCopyRect:
        ReadCopyRect(&surh);
        break;
      case rfbEncodingHextile:
        SetLastEncoding(surh.encoding);
        ReadHextileRect(&surh);
        break;
      case rfbEncodingTight:
        SetLastEncoding(surh.encoding);
        ReadTightRect(&surh);
        break;
      default:
        vnclog.Print(0, "Unknown encoding %d - not supported!\n",
                     surh.encoding);
        break;
    }

    if (m_opts.m_benchFile)
      tDecode += getTime() - tDecodeStart - (tRead - tReadOld) -
                 (tBlit - tBlitOld);

    // Tell the system to update a screen rectangle.  Note that
    // InvalidateScreenRect member function knows about scaling.
    if (!m_opts.m_DoubleBuffer || surh.encoding == rfbEncodingCopyRect) {
      RECT rect;
      if (m_opts.m_benchFile) tBlitStart = getTime();
      SetRect(&rect, surh.r.x, surh.r.y,
              surh.r.x + surh.r.w, surh.r.y + surh.r.h);
      InvalidateScreenRect(&rect);
      if (m_opts.m_benchFile) tBlit += getTime() - tBlitStart;
    }

    // Now we may discard "soft cursor locks".
    SoftCursorUnlockScreen();
  }

  if (m_opts.m_DoubleBuffer) {
    if (m_opts.m_benchFile) tBlitStart = getTime();

    while (list != NULL) {
      rfbFramebufferUpdateRectHeader *r1;
      node = list;
      r1 = &node->region;

      if (r1->encoding == rfbEncodingTight || r1->encoding == rfbEncodingRaw ||
          r1->encoding == rfbEncodingHextile) {
        SoftCursorLockArea(r1->r.x, r1->r.y, r1->r.w, r1->r.h);
        if (node->isFill) {
          omni_mutex_lock l(m_bitmapdcMutex);
          ObjectSelector b(m_hBitmapDC, m_hBitmap);
          PaletteSelector p(m_hBitmapDC, m_hPalette);
          FillSolidRect(r1->r.x, r1->r.y, r1->r.w, r1->r.h, node->fillColor);
        }
        RECT rect;
        SetRect(&rect, r1->r.x, r1->r.y, r1->r.x + r1->r.w, r1->r.y + r1->r.h);
        InvalidateScreenRect(&rect);
      }
      list = list->next;
      free(node);
    }
    SoftCursorUnlockScreen();

    if (m_opts.m_benchFile) tBlit += getTime() - tBlitStart;
  }

  if (m_opts.m_benchFile)
    DoBlit();
  else
    // Inform the other thread that an update is needed.
    PostMessage(m_hwnd, WM_REGIONUPDATED, NULL, NULL);

  if (m_firstUpdate) {
    // We need fences in order to make extra update requests and continuous
    // updates "safe".  See HandleFence() for the next step.
    if (supportsFence)
      SendFence(rfbFenceFlagRequest | rfbFenceFlagSyncNext, 0, NULL);

    if (!m_supportsSetDesktopSize) {
      if (m_opts.m_desktopSize.mode == SIZE_AUTO)
        vnclog.Print(-1, "Disabling automatic desktop resizing because the server doesn't support it.\n");
      if (m_opts.m_desktopSize.mode == SIZE_MANUAL)
        vnclog.Print(-1, "Ignoring desktop resize request because the server doesn't support it.\n");
      m_opts.m_desktopSize.mode = SIZE_SERVER;
    }

    if (m_opts.m_desktopSize.mode == SIZE_MANUAL)
      SendDesktopSize(m_opts.m_desktopSize.width, m_opts.m_desktopSize.height);
    else if (m_opts.m_desktopSize.mode == SIZE_AUTO)
      SendDesktopSize(m_autoResizeWidth, m_autoResizeHeight);

    m_firstUpdate = false;
  }
}


void ClientConnection::SetDormant(bool newstate)
{
  vnclog.Print(5, "%s dormant mode\n", newstate ? "Entering" : "Leaving");
  m_dormant = newstate;
  if (!m_dormant)
    SendIncrementalFramebufferUpdateRequest();
}


// The server has copied some text to the clipboard - put it
// in the local clipboard too.

void ClientConnection::ReadServerCutText()
{
  rfbServerCutTextMsg sctm;
  vnclog.Print(6, "Read remote clipboard change\n");
  ReadExact((char *)&sctm, sz_rfbServerCutTextMsg);
  size_t len = Swap32IfLE(sctm.length);

  CheckBufferSize(len);
  if (len == 0)
    m_netbuf[0] = '\0';
  else
    ReadString(m_netbuf, (int)len);
  UpdateLocalClipboard(m_netbuf, len);
}


void ClientConnection::ReadSetColorMapEntries()
{
  // Currently, we read and silently ignore SetColorMapEntries.
  rfbSetColourMapEntriesMsg msg;
  vnclog.Print(3, "Read server color map entries (ignored)\n");
  ReadExact((char *)&msg, sz_rfbSetColourMapEntriesMsg);
  int numEntries = Swap16IfLE(msg.nColours);

  if (numEntries > 0) {
    size_t nBytes = 6 * numEntries;
    CheckBufferSize(nBytes);
    ReadExact(m_netbuf, (int)nBytes);
  }
}


void ClientConnection::ReadBell()
{
  rfbBellMsg bm;
  ReadExact((char *)&bm, sz_rfbBellMsg);

  if (!PlaySound("VNCViewerBell", NULL,
                 SND_APPLICATION | SND_ALIAS | SND_NODEFAULT | SND_ASYNC))
    Beep(440, 125);

  if (m_opts.m_DeiconifyOnBell) {
    if (IsIconic(m_hwnd1)) {
      SetDormant(false);
      ShowWindow(m_hwnd1, SW_SHOWNORMAL);
    }
  }

  vnclog.Print(6, "Bell!\n");
}


// General utilities -------------------------------------------------

// Reads the number of bytes specified into the buffer given

void ClientConnection::ReadExact(char *inbuf, int wanted)
{
  if (m_opts.m_benchFile != NULL) {
    double tReadStart = getTime();
    if (fread(inbuf, wanted, 1, m_opts.m_benchFile) < 1) {
      if (feof(m_opts.m_benchFile))
        throw QuietException("End of session capture");
      if (ferror(m_opts.m_benchFile)) {
        int err = GetLastError();
        vnclog.Print(1, "Error reading session capture: %d\n", err);
        throw WarningException("ReadExact: Error reading session capture");
      }
      m_running = false;
    }
    tRead += getTime() - tReadStart;
    return;
  }

  if (m_sock == INVALID_SOCKET && m_bKillThread)
    throw QuietException("Connection closed.");

  omni_mutex_lock l(m_readMutex);

  int offset = 0;
  vnclog.Print(10, "  reading %d bytes\n", wanted);

  while (wanted > 0) {

    int bytes = recv(m_sock, inbuf + offset, wanted, 0);
    if (bytes == 0) throw WarningException("Connection closed.");
    if (bytes == SOCKET_ERROR) {
      int err = GetLastError();
      vnclog.Print(1, "Socket error while reading %d\n", err);
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
  vnclog.Print(10, "Read a %d-byte string\n", length);
}


// Sends the number of bytes specified from the buffer

inline void ClientConnection::WriteExact(char *buf, int bytes)
{
  if (bytes == 0 || m_sock == INVALID_SOCKET || m_opts.m_benchFile)
    return;

  omni_mutex_lock l(m_writeMutex);
  vnclog.Print(10, "  writing %d bytes\n", bytes);

  int i = 0;
  int j;

  while (i < bytes) {

    j = send(m_sock, buf + i, bytes - i, 0);
    if (j == SOCKET_ERROR || j == 0) {
      LPVOID lpMsgBuf;
      int err = GetLastError();
      FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  // Default language
        (LPTSTR)&lpMsgBuf, 0, NULL);  // Process any inserts in lpMsgBuf.
      vnclog.Print(1, "Socket error %d: %s\n", err, lpMsgBuf);
      LocalFree(lpMsgBuf);
      m_running = false;

      throw WarningException("WriteExact: Socket error while writing.");
    }
    i += j;
  }
}


// Read the string describing the reason for a connection failure.
// This function reads the data into m_netbuf and returns that pointer
// as the beginning of the reason string.

char *ClientConnection::ReadFailureReason()
{
  CARD32 reasonLen;
  ReadExact((char *)&reasonLen, sizeof(reasonLen));
  reasonLen = Swap32IfLE(reasonLen);

  CheckBufferSize(reasonLen + 1);
  ReadString(m_netbuf, reasonLen);

  vnclog.Print(0, "RFB connection failed, reason: %s\n", m_netbuf);
  return m_netbuf;
}


// Makes sure netbuf is at least as big as the specified size.
// Note that netbuf itself may change as a result of this call.
// Throws an exception on failure.

void ClientConnection::CheckBufferSize(size_t bufsize)
{
  if (m_netbufsize > bufsize) return;

  // Don't try to allocate more than 2 gigabytes.
  if (bufsize >= 0x80000000) {
    vnclog.Print(1, "Requested buffer size is too big (%u bytes)\n",
                 (unsigned int)bufsize);
    throw WarningException("Requested buffer size is too big.");
  }

  omni_mutex_lock l(m_bufferMutex);

  char *newbuf = new char[bufsize + 256];
  if (newbuf == NULL)
    throw ErrorException("Insufficient memory to allocate network buffer.");

  if (m_netbuf != NULL)
    delete[] m_netbuf;
  m_netbuf = newbuf;
  m_netbufsize = bufsize + 256;
  vnclog.Print(4, "Buffer size expanded to %u\n", (unsigned int)m_netbufsize);
}


// Makes sure zlibbuf is at least as big as the specified size.
// Note that zlibbuf itself may change as a result of this call.
// Throws an exception on failure.

void ClientConnection::CheckZlibBufferSize(size_t bufsize)
{
  if (m_zlibbufsize > bufsize) return;

  // Don't try to allocate more than 2 gigabytes.
  if (bufsize >= 0x80000000) {
    vnclog.Print(1, "Requested zlib buffer size is too big (%u bytes)\n",
                 (unsigned int)bufsize);
    throw WarningException("Requested zlib buffer size is too big.");
  }

  // omni_mutex_lock l(m_bufferMutex);

  unsigned char *newbuf = new unsigned char[bufsize + 256];
  if (newbuf == NULL)
    throw ErrorException("Insufficient memory to allocate zlib buffer.");

  if (m_zlibbuf != NULL)
    delete[] m_zlibbuf;
  m_zlibbuf = newbuf;
  m_zlibbufsize = bufsize + 256;
  vnclog.Print(4, "Zlib buffer size expanded to %u\n",
               (unsigned int)m_zlibbufsize);
}


// Invalidate a screen rectangle, respecting scaling set by user.

void ClientConnection::InvalidateScreenRect(const RECT *pRect)
{
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
    int right  = (pRect->right  + d - 1) / d * d;  // round up
    int bottom = (pRect->bottom + d - 1) / d * d;  // round up

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


// Processing NewFBSize pseudo-rectangle.  Create new framebuffer of
// the size specified in pfburh->r.w and pfburh->r.h, and change the
// window size correspondingly.

void ClientConnection::ReadNewFBSize(rfbFramebufferUpdateRectHeader *pfburh)
{
  if (pfburh->r.w == 0 || pfburh->r.h == 0) {
    vnclog.Print(-1, "Ignoring invalid desktop size from server\n");
    return;
  }

  m_si.framebufferWidth = pfburh->r.w;
  m_si.framebufferHeight = pfburh->r.h;

  if (continuousUpdates)
    SendEnableContinuousUpdates(TRUE, 0, 0, m_si.framebufferWidth,
                                m_si.framebufferHeight);

  CreateLocalFramebuffer();

  if (m_opts.m_desktopSize.mode == SIZE_AUTO &&
      m_autoResizeWidth == pfburh->r.w && m_autoResizeHeight == pfburh->r.h) {
    RECT fullwinrect;
    SetRect(&fullwinrect, 0, 0, m_si.framebufferWidth, m_si.framebufferHeight);
    PositionWindow(fullwinrect, false, true);
    PositionChildWindow();
    return;
  }

  if (InFullScreenMode())
    m_pendingServerResize = true;
  SizeWindow(true, true, false);
  RealiseFullScreenMode(true);
  if (InFullScreenMode())
    m_pendingServerResize = false;
}


void ClientConnection::ReadExtendedDesktopSize(
  rfbFramebufferUpdateRectHeader *pfburh)
{
  struct { unsigned char numScreens, pad[3]; } s;
  unsigned int i;
  ScreenSet layout;

  ReadExact((char *)&s, sizeof(s));

  for (i = 0; i < s.numScreens; i++) {
    rfbScreenDesc screen;
    ReadExact((char *)&screen, sz_rfbScreenDesc);

    screen.id = Swap32IfLE(screen.id);
    screen.x = Swap16IfLE(screen.x);
    screen.y = Swap16IfLE(screen.y);
    screen.w = Swap16IfLE(screen.w);
    screen.h = Swap16IfLE(screen.h);
    screen.flags = Swap32IfLE(screen.flags);

    layout.add_screen(Screen(screen.id, screen.x, screen.y, screen.w, screen.h,
                             screen.flags));

    // Normally, the TurboVNC Viewer will not create a multi-screen viewer
    // window that extends beyond the unshared boundary of any physical screen
    // on the client.  This ensures that the window position and scrollbars
    // will be correct, regardless of the monitor layout.  However, if
    // automatic desktop resizing is enabled and the server supports Xinerama,
    // then the full-screen multi-screen viewer window should extend to the
    // bounding box of all physical screens, even if it extends beyond the
    // unshared boundary of some of them.  This ensures correct behavior if the
    // screens have different resolutions or are offset.  TurboVNC 2.0.x-2.1.x
    // supported the RFB extended desktop size message but not Xinerama, and
    // those versions also sent a screen ID of 0.  Thus, we do not assume that
    // the server supports Xinerama unless it sends a multi-screen layout or a
    // non-zero screen ID.
    if (i > 0 || screen.id != 0) m_serverXinerama = true;
  }

  layout.debugPrint("LAYOUT RECEIVED");

  int reason = pfburh->r.x, result = pfburh->r.y;

  if ((reason == (signed)reasonClient) && (result != (signed)resultSuccess)) {
    vnclog.Print(-1, "ReadExtendedDesktopSize failed: %d\n", result);
    return;
  }

  if (!layout.validate(pfburh->r.w, pfburh->r.h, true))
    vnclog.Print(-1, "Server sent us an invalid screen layout\n");

  m_supportsSetDesktopSize = true;
  m_screenLayout = layout;

  ReadNewFBSize(pfburh);
}

ScreenSet ClientConnection::ComputeScreenLayout(int width, int height)
{
  ScreenSet layout;
  char env[256];  size_t envlen;

  if (getenv_s(&envlen, env, 256, "TVNC_SINGLESCREEN") == 0 &&
      !strcmp(env, "1")) {
    layout = m_screenLayout;

    if (layout.num_screens() == 0)
      layout.add_screen(Screen());
    else if (layout.num_screens() != 1) {
      ScreenSet::iterator iter;
      while (true) {
        iter = layout.begin();
        ++iter;

        if (iter == layout.end())
          break;

        layout.remove_screen(iter->id);
      }
    }

    layout.begin()->dimensions.left = 0;
    layout.begin()->dimensions.top = 0;
    layout.begin()->dimensions.right = width;
    layout.begin()->dimensions.bottom = height;
  } else {
    RECT workArea, screenArea;
    layout = GetFullScreenMetrics(screenArea, workArea, m_opts.m_Span, false);
    layout.assignIDs(m_screenLayout);
  }

  return layout;
}

void ClientConnection::SendDesktopSize(int width, int height)
{
  ScreenSet layout;

  if (!m_supportsSetDesktopSize)
    return;

  if (m_opts.m_desktopSize.mode == SIZE_AUTO) {
    layout = ComputeScreenLayout(width, height);
  } else {
    if (m_opts.m_desktopSize.mode != SIZE_MANUAL ||
        m_opts.m_desktopSize.layout.num_screens() < 1) {
      vnclog.Print(-1, "ERROR: Unexpected desktop size configuration");
      return;
    }
    layout = m_opts.m_desktopSize.layout;
    // Map client screens to server screen IDs in the server's preferred order.
    // This allows us to control the server's screen order from the client.
    layout.assignIDs(m_screenLayout);
  }

  SendDesktopSize(width, height, layout);
}

void ClientConnection::SendDesktopSize(int width, int height, ScreenSet layout)
{
  ScreenSet::const_iterator iter;

  if (!m_supportsSetDesktopSize)
    return;

  if (!layout.validate(width, height, true)) {
    vnclog.Print(-1, "Invalid screen layout\n");
    return;
  }

  rfbSetDesktopSizeMsg msg;
  msg.type = rfbSetDesktopSize;
  msg.w = Swap16IfLE(width);
  msg.h = Swap16IfLE(height);
  msg.numScreens = layout.num_screens();
  omni_mutex_lock l(m_writeMutex);  // Ensure back-to-back writes are grouped
  WriteExact((char *)&msg, sz_rfbSetDesktopSizeMsg);

  for (iter = layout.begin(); iter != layout.end(); ++iter) {
    rfbScreenDesc screen;
    screen.id = Swap32IfLE(iter->id);
    screen.x = Swap16IfLE(iter->dimensions.left);
    screen.y = Swap16IfLE(iter->dimensions.top);
    screen.w = Swap16IfLE(WidthOf(iter->dimensions));
    screen.h = Swap16IfLE(HeightOf(iter->dimensions));
    screen.flags = Swap32IfLE(iter->flags);
    WriteExact((char *)&screen, sz_rfbScreenDesc);
  }

  layout.debugPrint("LAYOUT SENT");
}


void ClientConnection::InitSetPixels(void)
{
  SETUP_COLOR_SHORTCUTS;

  m_srcpf = -1;
  int srcps = m_myFormat.bitsPerPixel / 8;
  if (rs == 0 && gs == 8 && bs == 16) {
    if (srcps == 3) m_srcpf = FBX_RGB;
    else if (srcps == 4) m_srcpf = FBX_RGBA;

  } else if (rs == 16 && gs == 8 && bs == 0) {
    if (srcps == 3) m_srcpf = FBX_BGR;
    else if (srcps == 4) m_srcpf = FBX_BGRA;

  } else if (rs == 8 && gs == 16 && bs == 24 && srcps == 4)
    m_srcpf = FBX_ARGB;
  else if (rs == 24 && gs == 16 && bs == 8 && srcps == 4)
    m_srcpf = FBX_ABGR;

  if (m_srcpf < 0) {
    // Source is not 24-bit or 32-bit
    switch (m_myFormat.bitsPerPixel) {
      case 8:
        setPixels = &ClientConnection::SetPixelsFullConv8;  break;
      case 16:
        setPixels = &ClientConnection::SetPixelsFullConv16;  break;
      case 24:
      case 32:
        setPixels = &ClientConnection::SetPixelsFullConv32;  break;
      default:
        vnclog.Print(0, "Invalid number of bits per pixel: %d\n",
                     m_myFormat.bitsPerPixel);
        return;
    }
    return;
  }

  int srcbgr = fbx_bgr[m_srcpf], dstbgr = fbx_bgr[fb.format],
      dstps = fbx_ps[fb.format];

  if (srcbgr == dstbgr && srcps == dstps)
    setPixels = &ClientConnection::SetPixelsCopyLine;
  else {
    if (srcbgr == dstbgr) setPixels = &ClientConnection::SetPixelsCopyPixel;
    else setPixels = &ClientConnection::SetPixelsSlow;
  }
}


#define DEFINE_SETPIXELSFULL_FUNC(bpp)                                        \
void ClientConnection::SetPixelsFullConv##bpp(char *buffer, int x, int y,     \
                                              int srcw, int srch)             \
{                                                                             \
  SETUP_COLOR_SHORTCUTS;                                                      \
                                                                              \
  int w = srcw, h = srch;                                                     \
  if (x + w > fb.width) w = fb.width - x;                                     \
  if (y + h > fb.height) h = fb.height - y;                                   \
  if (w < 1 || h < 1) {                                                       \
    vnclog.Print(0, "Rectangle out of bounds for screen: %d,%d %dx%d\n",      \
                 x, y, srcw, srch);                                           \
    return;                                                                   \
  }                                                                           \
                                                                              \
  CARD32 drs = fbx_roffset[fb.format] * 8;                                    \
  CARD32 dgs = fbx_goffset[fb.format] * 8;                                    \
  CARD32 dbs = fbx_boffset[fb.format] * 8;                                    \
                                                                              \
  CARD##bpp *srcptr = (CARD##bpp *)buffer;                                    \
  CARD##bpp *srcfinal = &srcptr[srcw * h];                                    \
  CARD32 *dstptr = (CARD32 *)&fb.bits[y * fb.pitch + x * fbx_ps[fb.format]];  \
  int dstw = fb.pitch / fbx_ps[fb.format];                                    \
                                                                              \
  for (; srcptr < srcfinal; srcptr += srcw, dstptr += dstw) {                 \
    CARD##bpp *p = srcptr, *pfinal = &p[w];                                   \
    CARD32 *dstptr2 = dstptr;                                                 \
    for (; p < pfinal; p++, dstptr2++)                                        \
      *dstptr2 = (((((CARD32)(*p) >> rs) & rm) * 255 / rm) << drs) |          \
                 (((((CARD32)(*p) >> gs) & gm) * 255 / gm) << dgs) |          \
                 (((((CARD32)(*p) >> bs) & bm) * 255 / bm) << dbs);           \
  }                                                                           \
}

DEFINE_SETPIXELSFULL_FUNC(8)
DEFINE_SETPIXELSFULL_FUNC(16)
DEFINE_SETPIXELSFULL_FUNC(32)


void ClientConnection::SetPixelsCopyLine(char *buffer, int x, int y, int srcw,
                                         int srch)
{
  int w = srcw, h = srch;
  if (x + w > fb.width) w = fb.width - x;
  if (y + h > fb.height) h = fb.height - y;
  if (w < 1 || h < 1) {
    vnclog.Print(0, "Rectangle out of bounds for screen: %d,%d %dx%d\n", x, y,
                 srcw, srch);
    return;
  }

  int srcps = m_myFormat.bitsPerPixel / 8;
  int wps = w * srcps;
  int srcstride = srcps * srcw;
  char *srcptr = buffer,
       *dstptr = &fb.bits[y * fb.pitch + x * fbx_ps[fb.format]];
  char *srcfinal = &srcptr[srcstride * h];

  int srcaf = fbx_alphafirst[m_srcpf], dstaf = fbx_alphafirst[fb.format];
  if (srcaf) srcptr++;
  if (dstaf) dstptr++;
  if (srcaf || dstaf) wps--;

  for (; srcptr < srcfinal; srcptr += srcstride, dstptr += fb.pitch)
    memcpy(dstptr, srcptr, wps);
}


void ClientConnection::SetPixelsCopyPixel(char *buffer, int x, int y, int srcw,
                                          int srch)
{
  int w = srcw, h = srch;
  if (x + w > fb.width) w = fb.width - x;
  if (y + h > fb.height) h = fb.height - y;
  if (w < 1 || h < 1) {
    vnclog.Print(0, "Rectangle out of bounds for screen: %d,%d %dx%d\n", x, y,
                 srcw, srch);
    return;
  }

  int srcps = m_myFormat.bitsPerPixel / 8;
  int srcstride = srcps * srcw, dststride = fb.pitch;
  char *srcptr = buffer,
       *dstptr = &fb.bits[y * fb.pitch + x * fbx_ps[fb.format]];
  char *srcfinal = &srcptr[srcstride * h];

  int srcaf = fbx_alphafirst[m_srcpf], dstaf = fbx_alphafirst[fb.format];
  if (srcaf) srcptr++;
  if (dstaf) dstptr++;

  for (; srcptr < srcfinal; srcptr += srcstride, dstptr += dststride) {
    char *srcptr2 = srcptr, *src2final = &srcptr2[srcps * w],
         *dstptr2 = dstptr;
    for (; srcptr2 < src2final; srcptr2 += srcps, dstptr2 += fbx_ps[fb.format])
      memcpy(dstptr2, srcptr2, 3);
  }
}


void ClientConnection::SetPixelsSlow(char *buffer, int x, int y, int srcw,
                                     int srch)
{
  int w = srcw, h = srch;
  if (x + w > fb.width) w = fb.width - x;
  if (y + h > fb.height) h = fb.height - y;
  if (w < 1 || h < 1) {
    vnclog.Print(0, "Rectangle out of bounds for screen: %d,%d %dx%d\n", x, y,
                 srcw, srch);
    return;
  }

  int srcps = m_myFormat.bitsPerPixel / 8;
  int srcstride = srcps * srcw, dststride = fb.pitch;
  char *srcptr = buffer,
       *dstptr = &fb.bits[y * fb.pitch + x * fbx_ps[fb.format]];
  char *srcfinal = &srcptr[srcstride * h];

  for (; srcptr < srcfinal; srcptr += srcstride, dstptr += dststride) {
    char *srcptr2 = srcptr, *src2final = &srcptr2[srcps * w],
         *dstptr2 = dstptr;
    for (; srcptr2 < src2final; srcptr2 += srcps,
         dstptr2 += fbx_ps[fb.format]) {
      dstptr2[2] = srcptr2[0];
      dstptr2[1] = srcptr2[1];
      dstptr2[0] = srcptr2[2];
    }
  }
}
