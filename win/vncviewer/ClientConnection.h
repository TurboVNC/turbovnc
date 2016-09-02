//  Copyright (C) 2010, 2012-2013, 2015-2016 D. R. Commander. All Rights Reserved.
//  Copyright (C) 2005-2006 Sun Microsystems, Inc. All Rights Reserved.
//  Copyright (C) 2004 Landmark Graphics Corporation. All Rights Reserved.
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

#ifndef CLIENTCONNECTION_H__
#define CLIENTCONNECTION_H__

#pragma once

#include "stdhdrs.h"
#include "omnithread.h"
#include "CapsContainer.h"
#include "VNCOptions.h"
#include "VNCviewerApp.h"
#include "KeyMap.h"
#include "ConnectingDialog.h"
#include "FileTransfer.h"
#include "zlib/zlib.h"
#include "turbojpeg.h"
#include "fbx.h"
#include "ScreenSet.h"
#include "ExtInputDevice.h"
#include "ExtInputEvent.h"
extern "C" {
#include "wintab/Utils.h"
#define PACKETDATA (PK_CURSOR | PK_X | PK_Y | PK_NORMAL_PRESSURE | \
  PK_ORIENTATION | PK_BUTTONS | PK_CHANGED)
#define PACKETMODE 0
#include "wintab/pktdef.h"
}

#include <map>

#define SETTINGS_KEY_NAME "Software\\TurboVNC\\VNCviewer\\Settings"
#define MAX_HOST_NAME_LEN 250

#define TIGHT_ZLIB_BUFFER_SIZE 512


#define WidthOf(rect) ((rect).right - (rect).left)
#define HeightOf(rect) ((rect).bottom - (rect).top)


class ClientConnection;
typedef void (ClientConnection:: *tightFilterFunc)(int, int, int);
typedef void (ClientConnection:: *setPixelsFunc)(char *, int, int, int, int);


typedef struct _UpdateList
{
  rfbFramebufferUpdateRectHeader region;
  COLORREF fillColor;
  struct _UpdateList *next;
  bool isFill;
} UpdateList;


class ClientConnection : public omni_thread
{
  public:
    ClientConnection(VNCviewerApp *pApp);
    ClientConnection(VNCviewerApp *pApp, SOCKET sock);
    ClientConnection(VNCviewerApp *pApp, LPTSTR host, int port);
    ClientConnection(VNCviewerApp *pApp, LPTSTR configFile);
    virtual ~ClientConnection();
    void Run();
    void KillThread();
    void CopyOptions(ClientConnection *source);
    int  LoadConnection(char *fname, bool sess);
    void UnloadConnection() { m_opts.m_configSpecified = false; }

    int m_port;
    char m_host[MAX_HOST_NAME_LEN];
    HWND m_hSess;

  private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam,
                                    LPARAM lParam);
    static LRESULT CALLBACK WndProc1(HWND hwnd, UINT iMsg, WPARAM wParam,
                                     LPARAM lParam);
    static LRESULT CALLBACK Proc(HWND hwnd, UINT iMsg, WPARAM wParam,
                                 LPARAM lParam);
    static LRESULT CALLBACK ScrollProc(HWND hwnd, UINT iMsg, WPARAM wParam,
                                       LPARAM lParam);
    void DoBlit();
    VNCviewerApp *m_pApp;
    ConnectingDialog *m_connDlg;

    bool m_enableFileTransfers;
    bool m_fileTransferDialogShown;
    friend class FileTransfer;
    FileTransfer *m_pFileTransfer;

    SOCKET m_sock;
    bool m_serverInitiated;
    HWND m_hwnd, m_hbands, m_hwnd1, m_hToolbar, m_hwndscroll;

    void Init(VNCviewerApp *pApp);
    void InitCapabilities();
    void CreateDisplay();
    HWND CreateToolbar();
    void GetConnectDetails();
    void Connect();
    void SetSocketOptions();
    void NegotiateProtocolVersion();
    void PerformAuthentication();
    int ReadSecurityType();
    int SelectSecurityType();
    void SetupTunneling();
    void PerformAuthenticationTight();
    void Authenticate(CARD32 authScheme);
    bool AuthenticateNone(char *errBuf, int errBufSize);
    bool AuthenticateVNC(char *errBuf, int errBufSize);
    bool AuthenticateUnixLogin(char *errBuf, int errBufSize);
    void ReadServerInit();
    void ReadInteractionCaps();
    void ReadCapabilityList(CapsContainer *caps, int count);
    void SendClientInit();
    void CreateLocalFramebuffer();
    void SaveConnection();
    void SaveConnectionHistory();

    void SetupPixelFormat();
    void SetFormatAndEncodings();
    void SendSetPixelFormat(rfbPixelFormat newFormat);

    void SendIncrementalFramebufferUpdateRequest();
    void SendFullFramebufferUpdateRequest();
    void SendAppropriateFramebufferUpdateRequest();
    void SendFramebufferUpdateRequest(int x, int y, int w, int h,
                                      bool incremental);

    void ProcessPointerEvent(int x, int y, DWORD keyflags, UINT msg);
    void SubProcessPointerEvent(int x, int y, DWORD keyflags);
    void SendPointerEvent(int x, int y, int buttonMask);
    void ProcessKeyEvent(int virtkey, DWORD keyData);
    void SendKeyEvent(CARD32 key, bool down);
    void SwitchOffKeys();
    void GrabKeyboard();
    void UngrabKeyboard();
    bool isKeyboardGrabbed();

    void ReadScreenUpdate();
    void Update(RECT *pRect);
    void SizeWindow(bool centered, bool resizeFullScreen = false,
                    bool manual = false);
    void PositionWindow(RECT &fullwinrect, bool centered,
                        bool dimensionsOnly = false);
    void PositionChildWindow();
    bool ScrollScreen(int dx, int dy);
    void UpdateScrollbars();
    void EnableFullControlOptions();
    void EnableAction(int id, bool enable);

    void InvalidateScreenRect(const RECT *pRect);

    void ReadRawRect(rfbFramebufferUpdateRectHeader *pfburh);
    void ReadCopyRect(rfbFramebufferUpdateRectHeader *pfburh);
    void ReadRRERect(rfbFramebufferUpdateRectHeader *pfburh);
    void ReadCoRRERect(rfbFramebufferUpdateRectHeader *pfburh);
    void ReadHextileRect(rfbFramebufferUpdateRectHeader *pfburh);
    void HandleHextileEncoding8(int x, int y, int w, int h);
    void HandleHextileEncoding16(int x, int y, int w, int h);
    void HandleHextileEncoding32(int x, int y, int w, int h);
    void ReadZlibRect(rfbFramebufferUpdateRectHeader *pfburh);

    void ReadNewFBSize(rfbFramebufferUpdateRectHeader *pfburh);
    void ReadExtendedDesktopSize(rfbFramebufferUpdateRectHeader *pfburh);
    void SendDesktopSize(int width, int height);

    void InitSetPixels(void);
    setPixelsFunc setPixels;
    void SetPixelsFullConv8(char *, int, int, int, int);
    void SetPixelsFullConv16(char *, int, int, int, int);
    void SetPixelsFullConv32(char *, int, int, int, int);
    void SetPixelsCopyLine(char *, int, int, int, int);
    void SetPixelsCopyPixel(char *, int, int, int, int);
    void SetPixelsSlow(char *, int, int, int, int);
    int m_srcpf;

    // ClientConnectionTight.cpp
    void ReadTightRect(rfbFramebufferUpdateRectHeader *pfburh);
    int ReadCompactLen();
    int InitFilterCopy (int rw, int rh);
    int InitFilterGradient (int rw, int rh);
    int InitFilterPalette (int rw, int rh);
    void FilterCopy8 (int srcx, int srcy, int numRows);
    void FilterCopy16 (int srcx, int srcy, int numRows);
    void FilterCopy24 (int srcx, int srcy, int numRows);
    void FilterCopy32 (int srcx, int srcy, int numRows);
    void FilterGradient8 (int srcx, int srcy, int numRows);
    void FilterGradient16 (int srcx, int srcy, int numRows);
    void FilterGradient24 (int srcx, int srcy, int numRows);
    void FilterGradient32 (int srcx, int srcy, int numRows);
    void FilterPalette (int srcx, int srcy, int numRows);
    void DecompressJpegRect(int x, int y, int w, int h);

    bool zlibDecompress(unsigned char *from_buf, unsigned char *to_buf,
                        unsigned int count, unsigned int size,
                        z_stream *decompressor);

    void ReadRBSRect(rfbFramebufferUpdateRectHeader *pfburh);
    BOOL DrawRBSRect8(int x, int y, int w, int h, CARD8 **pptr);
    BOOL DrawRBSRect16(int x, int y, int w, int h, CARD8 **pptr);
    BOOL DrawRBSRect32(int x, int y, int w, int h, CARD8 **pptr);

    // ClientConnectionCursor.cpp
    bool prevCursorSet;
    HDC m_hSavedAreaDC;
    HBITMAP m_hSavedAreaBitmap;
    COLORREF *rcSource;
    bool *rcMask;
    int rcHotX, rcHotY, rcWidth, rcHeight;
    int rcCursorX, rcCursorY;
    int rcLockX, rcLockY, rcLockWidth, rcLockHeight;
    bool rcCursorHidden, rcLockSet;

    void ReadCursorShape(rfbFramebufferUpdateRectHeader *pfburh);
    void ReadCursorPos(rfbFramebufferUpdateRectHeader *pfburh);
    void SoftCursorLockArea(int x, int y, int w, int h);
    void SoftCursorUnlockScreen();
    void SoftCursorMove(int x, int y);
    void SoftCursorFree();
    bool SoftCursorInLockedArea();
    void SoftCursorSaveArea();
    void SoftCursorRestoreArea();
    void SoftCursorDraw();
    void SoftCursorToScreen(RECT *screenArea, POINT *cursorOffset);

    // ClientConnectionFullScreen.cpp
    void SetFullScreenMode(bool enable, bool suppressPrompt = false);
    bool InFullScreenMode();
    void RealiseFullScreenMode(bool suppressPrompt);
    void GetFullScreenMetrics(RECT &screenArea, RECT &workArea,
                              int spanMode);
    void GetFullScreenMetrics(RECT &screenArea, RECT &workArea)
    {
      GetFullScreenMetrics(screenArea, workArea, m_opts.m_Span);
    }
    bool BumpScroll(int x, int y);

    // ClientConnectionClipboard.cpp
    void ProcessLocalClipboardChange();
    void UpdateLocalClipboard(char *buf, size_t len);
    void SendClientCutText(char *str, size_t len);
    void ReadServerCutText();

    void ReadSetColorMapEntries();
    void ReadBell();

    void SendRFBMsg(CARD8 msgType, void* data, int length);
    void ReadExact(char *buf, int bytes);
    void ReadString(char *buf, int length);
    void WriteExact(char *buf, int bytes);
    char *ReadFailureReason();

    // ClientConnectionFlowControl.cpp
    bool supportsCU;
    bool continuousUpdates;
    bool supportsFence;
    bool supportsSyncFence;
    bool pendingSyncFence;

    void HandleFence(CARD32 flags, unsigned len, const char *data);
    void SendEnableContinuousUpdates(bool enable, int x, int y, int w, int h);
    void SendFence(CARD32 flags, unsigned len, const char *data);
    void ReadFence(void);

    // Extended input device stuff
    bool supportsGII;
    std::list<ExtInputDevice> devices;
    HCTX m_hctx;
    DWORD m_wacomButtonMask;

    void ReadGII(void);
    void SendGIIVersion(void);
    void SendGIIDeviceCreate(ExtInputDevice &dev);
    void SendGIIEvent(UINT deviceID, ExtInputEvent &e);
    void AddInputDevice(ExtInputDevice &dev);
    void AssignInputDevice(int deviceOrigin);
    void CreateWacomGIIDevices(void);
    void TranslateWacomCoords(HWND hwnd, int localX, int localY, int &remoteX,
                              int &remoteY);
    void ConvertWacomTilt(ORIENTATION orient, int &tiltX, int &tiltY);

    // ClientConnectionTunnel.cpp
    void SetupSSHTunnel(void);

    // This is what controls the thread
    void * run_undetached(void* arg);
    bool m_bKillThread;

    // Utilities

    // These draw a solid rectangle on the bitmap.  They assume that the bitmap
    // is already selected into the DC and that the DC is locked, if necessary.
    // Normally, this is an inline call to a GDI method.
    inline void FillSolidRect(RECT *pRect, COLORREF color)
    {
      COLORREF oldbgcol = SetBkColor(m_hBitmapDC, color);
      // This is the call MFC uses for FillSolidRect.  Who am I to argue?
      ExtTextOut(m_hBitmapDC, 0, 0, ETO_OPAQUE, pRect, NULL, 0, NULL);
    };

    inline void FillSolidRect(int x, int y, int w, int h, COLORREF color) {
      RECT r;
      r.left = x;   r.right = x + w;
      r.top = y;    r.bottom = y + h;
      FillSolidRect(&r, color);
    };

    // how many other windows are owned by this process?
    unsigned int CountProcessOtherWindows();

    // Buffer for network operations
    void CheckBufferSize(size_t bufsize);
    char *m_netbuf;
    size_t m_netbufsize;
    omni_mutex m_bufferMutex, m_bitmapdcMutex,  m_clipMutex, m_readMutex,
               m_writeMutex, m_sockMutex, m_cursorMutex;

    // Buffer for zlib decompression
    void CheckZlibBufferSize(size_t bufsize);
    unsigned char *m_zlibbuf;
    size_t m_zlibbufsize;

    // Variables used by tight encoding:

    // Separate buffer for tight-compressed data
    char m_tightbuf[TIGHT_ZLIB_BUFFER_SIZE];

    // Four independent Zlib compression streams
    z_stream m_tightZlibStream[4];
    bool m_tightZlibStreamActive[4];

    // Tight filter stuff.  Should be initialized by filter initialization code
    tightFilterFunc m_tightCurrentFilter;
    bool m_tightCutZeros;
    int m_tightRectWidth, m_tightRectColors;
    CARD32 m_tightPalette[256];
    CARD8 m_tightPrevRow[2048 * 3 * sizeof(CARD16)];

    // Bitmap for local copy of screen, and DC for writing to it.
    HBITMAP m_hBitmap;
    HDC m_hBitmapDC;
    HPALETTE m_hPalette;

    // Keyboard mapper
    KeyMap m_keymap;
    std::map<UINT, KeyActionSpec> pressedKeys;

    // RFB settings
    VNCOptions m_opts;

    // Protocol capabilities
    CapsContainer m_tunnelCaps;     // known tunneling/encryption methods
    CapsContainer m_authCaps;       // known authentication schemes
    CapsContainer m_serverMsgCaps;  // known non-standard server messages
    CapsContainer m_clientMsgCaps;  // known non-standard client messages
    CapsContainer m_encodingCaps;   // known encodings besides Raw

    char *m_desktopName;
    void SetWindowTitle(void);
    void SetLastEncoding(int);

    unsigned char m_encPasswd[8];
    bool m_passwdSet;
    int m_authScheme;
    rfbServerInitMsg m_si;
    rfbPixelFormat m_myFormat, m_pendingFormat;
    // protocol version in use.
    int m_minorVersion;
    bool m_tightVncProtocol;
    bool m_threadStarted, m_running;
    // mid-connection format change requested
    bool m_pendingFormatChange;
    bool m_pendingEncodingChange;
    // Display connection info;
    void ShowConnInfo();

    // Window may be scrollable - these control the scroll position
    int m_hScrollPos, m_hScrollMax, m_vScrollPos, m_vScrollMax;

    // The size of the current client area
    int m_cliwidth, m_cliheight;
    // The size of a window needed to hold entire screen without scrollbars
    int m_fullwinwidth, m_fullwinheight;
    int m_winwidth, m_winheight;

    RECT savedRect;

    bool m_firstUpdate;

    // Desktop resizing
    bool m_supportsSetDesktopSize, m_pendingServerResize;
    ScreenSet m_screenLayout;
    bool m_waitingOnResizeTimer;
    UINT_PTR m_resizeTimer;
    int m_autoResizeWidth, m_autoResizeHeight;

    // Dormant basically means minimized;  updates will not be requested
    // while dormant.
    void SetDormant(bool newstate);
    bool m_dormant;

    // The number of bytes required to hold at least one pixel
    unsigned int m_minPixelBytes;
    // Next window in clipboard chain
    HWND m_hwndNextViewer;
    bool m_initialClipboardSeen;

    // Are we waiting on a timer for 3-button emulation?
    bool m_waitingOnEmulateTimer;
    // Or are we emulating the middle button now?
    bool m_emulatingMiddleButton;
    // 3-button emulation timer
    UINT_PTR m_emulate3ButtonsTimer;
    // Buttons pressed;  waiting for 3-button emulation timer
    DWORD m_emulateKeyFlags;
    int m_emulateButtonPressedX;
    int m_emulateButtonPressedY;

    tjhandle j;
    fbx_struct fb;

    // For double buffering
    UpdateList *list, *node, *tail;

    // Benchmark stuff
    double &tDecode, &tBlit, &tRead;
    unsigned long long &decodePixels, &blitPixels;
    unsigned long &decodeRect, &blits, &updates;
};


// Some handy classes for temporary GDI object selection.  These select objects
// when constructed and automatically release them when destructed.

class ObjectSelector
{
  public:
    ObjectSelector(HDC hdc, HGDIOBJ hobj)
    {
      m_hdc = hdc;  m_hOldObj = SelectObject(hdc, hobj);
    }

    ~ObjectSelector() { m_hOldObj = SelectObject(m_hdc, m_hOldObj); }

    HGDIOBJ m_hOldObj;
    HDC m_hdc;
};


class PaletteSelector
{
  public:
    PaletteSelector(HDC hdc, HPALETTE hpal)
    {
      m_hdc = hdc;
      m_hOldPal = SelectPalette(hdc, hpal, FALSE);
      RealizePalette(hdc);
    }

    ~PaletteSelector()
    {
      m_hOldPal = SelectPalette(m_hdc, m_hOldPal, FALSE);
      RealizePalette(m_hdc);
    }

    HPALETTE m_hOldPal;
    HDC m_hdc;
};


class TempDC
{
  public:
    TempDC(HWND hwnd) { m_hdc = GetDC(hwnd);  m_hwnd = hwnd; }
    ~TempDC() { ReleaseDC(m_hwnd, m_hdc); }
    operator HDC() { return m_hdc; };

    HDC m_hdc;
    HWND m_hwnd;
};


// Color decoding utility functions
// Define rs, rm, bs, bm, gs & gm before using, e.g. with the following:

#define SETUP_COLOR_SHORTCUTS  \
   CARD8 rs = m_myFormat.redShift;   CARD16 rm = m_myFormat.redMax;    \
   CARD8 gs = m_myFormat.greenShift; CARD16 gm = m_myFormat.greenMax;  \
   CARD8 bs = m_myFormat.blueShift;  CARD16 bm = m_myFormat.blueMax;   \

// read a pixel from the given address and return a color value
#define COLOR_FROM_PIXEL8_ADDRESS(p) (  \
  PALETTERGB( (int) (((*(CARD8 *)(p) >> rs) & rm) * 255 / rm),  \
              (int) (((*(CARD8 *)(p) >> gs) & gm) * 255 / gm),  \
              (int) (((*(CARD8 *)(p) >> bs) & bm) * 255 / bm) ))

#define COLOR_FROM_PIXEL16_ADDRESS(p) (  \
  PALETTERGB( (int) ((( *(CARD16 *)(p) >> rs) & rm) * 255 / rm),  \
              (int) ((( *(CARD16 *)(p) >> gs) & gm) * 255 / gm),  \
              (int) ((( *(CARD16 *)(p) >> bs) & bm) * 255 / bm) ))

#define COLOR_FROM_PIXEL24_ADDRESS(p) (  \
  PALETTERGB( (int) (((CARD8 *)(p))[0]),  \
              (int) (((CARD8 *)(p))[1]),  \
              (int) (((CARD8 *)(p))[2]) ))

#define COLOR_FROM_PIXEL32_ADDRESS(p) (  \
  PALETTERGB( (int) ((( *(CARD32 *)(p) >> rs) & rm) * 255 / rm),  \
              (int) ((( *(CARD32 *)(p) >> gs) & gm) * 255 / gm),  \
              (int) ((( *(CARD32 *)(p) >> bs) & bm) * 255 / bm) ))

// The following may be faster if you already have a pixel value of the
// appropriate size
#define COLOR_FROM_PIXEL8(p) (  \
  PALETTERGB( (int) (((p >> rs) & rm) * 255 / rm),  \
              (int) (((p >> gs) & gm) * 255 / gm),  \
              (int) (((p >> bs) & bm) * 255 / bm) ))

#define COLOR_FROM_PIXEL16(p) (  \
  PALETTERGB( (int) (((p >> rs) & rm) * 255 / rm),  \
              (int) (((p >> gs) & gm) * 255 / gm),  \
              (int) (((p >> bs) & bm) * 255 / bm) ))

#define COLOR_FROM_PIXEL32(p) (  \
  PALETTERGB( (int) (((p >> rs) & rm) * 255 / rm),  \
              (int) (((p >> gs) & gm) * 255 / gm),  \
              (int) (((p >> bs) & bm) * 255 / bm) ))


#define SETPIXEL(b, x, y, c) SetPixelV((b), (x), (y), (c))

#define SETPIXELS(buffer, bpp, x, y, w, h)  \
  (this->*setPixels)((char *)buffer, x, y, w, h);

#define SETPIXELS_NOCONV(buffer, x, y, w, h)  \
  (this->*setPixels)((char *)buffer, x, y, w, h);

#endif // CLIENTCONNECTION_H__
