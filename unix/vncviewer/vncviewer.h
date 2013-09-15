/*
 *  Copyright (C) 2010, 2012-2013 D. R. Commander.  All Rights Reserved.
 *  Copyright (C) 2010 University Corporation for Atmospheric Research.
 *                     All Rights Reserved.
 *  Copyright (C) 2005-2008 Sun Microsystems, Inc.  All Rights Reserved.
 *  Copyright (C) 2000, 2001 Const Kaplinsky.  All Rights Reserved.
 *  Copyright (C) 2000 Tridia Corporation.  All Rights Reserved.
 *  Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
 *
 *  This is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this software; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 *  USA.
 */

#ifndef _POSIX_PTHREAD_SEMANTICS
#define _POSIX_PTHREAD_SEMANTICS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xmd.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xmu/StdSel.h>
#include "rfbproto.h"
#include "caps.h"


extern int endianTest;

#define Swap16IfLE(s) \
    (*(char *)&endianTest ? ((((s) & 0xff) << 8) | (((s) >> 8) & 0xff)) : (s))

#define Swap32IfLE(l) \
    (*(char *)&endianTest ? ((((l) & 0xff000000) >> 24) | \
                             (((l) & 0x00ff0000) >> 8)  | \
                             (((l) & 0x0000ff00) << 8)  | \
                             (((l) & 0x000000ff) << 24))  : (l))

#define MAX_ENCODINGS 20

#define LISTEN_PORT_OFFSET 5500
#define TUNNEL_PORT_OFFSET 5500
#define SERVER_PORT_OFFSET 5900

#define DEFAULT_SSH_CMD "/usr/bin/ssh"
#define DEFAULT_TUNNEL_CMD  \
  (DEFAULT_SSH_CMD " -f -L %L:localhost:%R %H sleep 20")
#define DEFAULT_VIA_CMD     \
  (DEFAULT_SSH_CMD " -f -L %L:%H:%R %G sleep 20")

/* Multi-threading stuff */
#define TVNC_MAXTHREADS 8

/*
 * The enum is ordered in this way so as to maintain backward compatibility
 * with TVNC 0.3.x
 */
#define TVNC_SAMPOPT 4
enum {TVNC_1X = 0, TVNC_4X, TVNC_2X, TVNC_GRAY};

#define TVNC_GRABOPT 3
enum {TVNC_FS = 0, TVNC_ALWAYS, TVNC_MANUAL};


/* argsresources.c */

typedef struct {
  Bool shareDesktop;
  Bool viewOnly;
  Bool fullScreen;
  Bool fsAltEnter;
  String grabKeyboardString;
  int grabKeyboard;
  Bool raiseOnBeep;

  String encodingsString;
  String subsampString;

  Bool useBGR233;
  int nColors;
  Bool useSharedColors;
  Bool forceOwnCmap;
  Bool forceTrueColor;
  int requestedDepth;

  Bool useShm;

  int wmDecorationWidth;
  int wmDecorationHeight;

  char *userLogin;
  Bool noUnixLogin;

  char *passwordFile;
  Bool passwordDialog;
  Bool userPwdDialog;

  int rawDelay;
  int copyRectDelay;

  Bool debug;

  int popupButtonCount;

  int bumpScrollTime;
  int bumpScrollPixels;

  int compressLevel;
  int qualityLevel;
  int subsampLevel;
  Bool enableJPEG;
  Bool cursorShape;
  Bool useX11Cursor;
  Bool useRichCursor;
  Bool autoPass;

  Bool doubleBuffer;
  Bool continuousUpdates;

  char *configFile;

} AppData;

extern AppData appData;

extern char *fallback_resources[];
extern char vncServerHost[];
extern int vncServerPort;
extern Bool listenSpecified;
extern int listenPort;

extern XrmOptionDescRec cmdLineOptions[];
extern int numCmdLineOptions;

extern void removeArgs(int *argc, char** argv, int idx, int nargs);
extern void usage(void);
extern void GetArgsAndResources(int argc, char **argv);


/* color.c */

extern unsigned long BGR233ToPixel[];

extern Colormap cmap;
extern Visual *vis;
extern unsigned int visdepth, visbpp;

extern void SetVisualAndCmap();


/* cursor.c */

extern Bool HandleCursorShape(int xhot, int yhot, int width, int height,
                              CARD32 enc);


/* desktop.c */

extern Atom wmDeleteWindow;
extern Widget form, viewport, desktop;
extern Window desktopWin;
extern Cursor dotCursor;
extern GC gc;
extern GC srcGC, dstGC;
extern Dimension dpyWidth, dpyHeight;

extern void DesktopInitBeforeRealization();
extern void DesktopInitAfterRealization();
extern void SendRFBEvent(Widget w, XEvent *event, String *params,
                         Cardinal *num_params);
extern void CopyDataToImage(char *buf, int x, int y, int width, int height);
extern void CopyImageToScreen(int x, int y, int width, int height);
extern void SynchroniseScreen();
extern void QualHigh(Widget, XEvent *, String *, Cardinal *);
extern void QualMed(Widget, XEvent *, String *, Cardinal *);
extern void QualLow(Widget, XEvent *, String *, Cardinal *);
extern void QualLossless(Widget, XEvent *, String *, Cardinal *);
extern void QualLosslessWAN(Widget, XEvent *, String *, Cardinal *);
extern void LosslessRefresh(Widget, XEvent *, String *, Cardinal *);


/* dialogs.c */

extern void ServerDialogDone(Widget w, XEvent *event, String *params,
                             Cardinal *num_params);
extern char *DoServerDialog();
extern void PasswordDialogDone(Widget w, XEvent *event, String *params,
                             Cardinal *num_params);
extern char *DoPasswordDialog();

extern void UserPwdSetFocus(Widget w, XEvent *event, String *params,
                             Cardinal *num_params);
extern void UserPwdNextField(Widget w, XEvent *event, String *params,
                             Cardinal *num_params);
extern void UserPwdDialogDone(Widget w, XEvent *event, String *params,
                             Cardinal *num_params);
extern void DoUserPwdDialog(char** user, char** password);


/* flowcontrol.c */

extern Bool supportsCU;
extern Bool supportsFence;
extern Bool supportsSyncFence;
extern Bool continuousUpdates;

extern Bool HandleFence(CARD32 flags, unsigned len, const char *data);
extern Bool SendEnableContinuousUpdates(Bool enable, int x, int y, int w,
                                        int h);
extern Bool SendFence(CARD32 flags, unsigned len, const char data[]);


/* fullscreen.c */

extern void ToggleFullScreen(Widget w, XEvent *event, String *params,
                             Cardinal *num_params);
extern void ToggleGrabKeyboard(Widget w, XEvent *event, String *params,
                             Cardinal *num_params);
extern void SetFullScreenState(Widget w, XEvent *event, String *params,
                               Cardinal *num_params);
extern void SetGrabKeyboardState(Widget w, XEvent *event, String *params,
                                 Cardinal *num_params);
extern Bool BumpScroll(XEvent *ev);
extern void FullScreenOn();
extern void FullScreenOff();
extern void GrabKeyboard();
extern void UngrabKeyboard();


/* listen.c */

extern void listenForIncomingConnections();


/* misc.c */

extern void ToplevelInitBeforeRealization();
extern void ToplevelInitAfterRealization();
extern Time TimeFromEvent(XEvent *ev);
extern void Pause(Widget w, XEvent *event, String *params,
                  Cardinal *num_params);
extern void RunCommand(Widget w, XEvent *event, String *params,
                       Cardinal *num_params);
extern void Quit(Widget w, XEvent *event, String *params,
                 Cardinal *num_params);
extern void Cleanup();
extern void UpdateTitleString(char *str, int len);
extern void SetLastEncoding(int enc);
extern Bool RunBenchmark(void);


/* popup.c */

extern Widget popup;
extern void ShowPopup(Widget w, XEvent *event, String *params,
                      Cardinal *num_params);
extern void HidePopup(Widget w, XEvent *event, String *params,
                      Cardinal *num_params);
extern void CreatePopup();
extern void SetCUState(Widget w, XEvent *ev, String *params,
                       Cardinal *num_params);
extern void SetViewOnlyState(Widget w, XEvent *ev, String *params,
                             Cardinal *num_params);


/* rfbproto.c */

extern int rfbsock;
extern Bool canUseCoRRE;
extern Bool canUseHextile;
extern char *desktopName;
extern rfbPixelFormat myFormat;
extern rfbServerInitMsg si;
extern char *serverCutText;
extern Bool newServerCutText;
extern Bool encodingChange;

extern Bool ConnectToRFBServer(const char *hostname, int port);
extern Bool InitialiseRFBConnection();
extern Bool SendIncrementalFramebufferUpdateRequest();
extern Bool SendFramebufferUpdateRequest(int x, int y, int w, int h,
                                         Bool incremental);
extern Bool SendPointerEvent(int x, int y, int buttonMask);
extern Bool SendKeyEvent(CARD32 key, Bool down);
extern Bool SendClientCutText(char *str, int len);
extern Bool HandleRFBServerMessage();

extern void PrintPixelFormat(rfbPixelFormat *format);

extern void ToggleCU(Widget w, XEvent *ev, String *params,
                     Cardinal *num_params);
extern void ToggleViewOnly(Widget w, XEvent *ev, String *params,
                     Cardinal *num_params);
extern Bool ReadServerInitMessage(void);

extern BOOL rfbProfile;
extern double tRecv, tDecode, tBlit;
extern unsigned long long decodePixels, blitPixels, recvBytes;
extern unsigned long decodeRect, blitRect, updates;
extern double gettime(void);

typedef struct _UpdateList {
   rfbFramebufferUpdateRectHeader region;
   XGCValues gcv;
   struct _UpdateList *next;
   Bool isFill;
   Bool isCopyRect;
   int crx, cry;
} UpdateList;

extern UpdateList *list, *node, *tail;


/* selection.c */

extern void InitialiseSelection();
extern void SelectionToVNC(Widget w, XEvent *event, String *params,
                           Cardinal *num_params);
extern void SelectionFromVNC(Widget w, XEvent *event, String *params,
                             Cardinal *num_params);


/* shm.c */

extern XImage *CreateShmImage();
extern void ShmCleanup();


/* sockets.c */

extern Bool errorMessageOnReadFailure;

extern Bool ReadFromRFBServer(char *out, unsigned int n);

extern Bool WriteExact(int sock, char *buf, int n);
extern int FindFreeTcpPort(void);
extern int ListenAtTcpPort(int port);
extern int ConnectToTcpAddr(const char *hostname, int port);
extern int AcceptTcpConnection(int listenSock);
extern Bool SetNonBlocking(int sock);

extern Bool SameMachine(int sock);

extern int family;


/* tunnel.c */

extern Bool tunnelSpecified;

extern Bool createTunnel(int *argc, char **argv, int tunnelArgIndex);


/* vncviewer.c */

extern char *programName;
extern XtAppContext appContext;
extern Display* dpy;
extern Widget toplevel;
extern FILE *benchFile;
extern int benchIter;
extern int benchWarmup;
extern long benchFileStart;
