/*
 * init.c
 *
 * Modified for XFree86 4.x by Alan Hourihane <alanh@fairlite.demon.co.uk>
 */

/* Copyright (C) 2009-2023 D. R. Commander.  All Rights Reserved.
 * Copyright (C) 2021 Steffen Kieß
 * Copyright (C) 2016-2017 Pierre Ossman for Cendio AB.  All Rights Reserved.
 * Copyright (C) 2010 University Corporation for Atmospheric Research.
 *                    All Rights Reserved.
 * Copyright (C) 2005 Sun Microsystems, Inc.  All Rights Reserved.
 * Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

/*

Copyright (c) 1993  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include "servermd.h"
#include "extension.h"
#include "fb.h"
#include "dixstruct.h"
#include "propertyst.h"
#include "xserver-properties.h"
#include "exevents.h"
#include <X11/Xatom.h>
#ifdef GLXEXT
#include "glx_extinit.h"
#endif
#include "micmap.h"
#include "eventstr.h"
#include "rfb.h"
#include <time.h>
#include "tvnc_version.h"
#include "input-xkb.h"
#include "xkbsrv.h"
#include "registry.h"
#define XSERV_t
#define TRANS_SERVER
#define TRANS_REOPEN
#include <X11/Xtrans/Xtrans.h>
#include <xkb-config.h>

#define RFB_DEFAULT_WIDTH  1024
#define RFB_DEFAULT_HEIGHT 768
#define RFB_DEFAULT_DEPTH  24
#define RFB_DEFAULT_WHITEPIXEL 0
#define RFB_DEFAULT_BLACKPIXEL 1

#define DEFAULT_DESKTOP_NAME "x11"

#ifdef GLXEXT
extern char *dri_driver_path;
#endif
#if defined(X_REGISTRY_REQUEST) && !defined(TURBOVNC_STATIC_XORG_PATHS)
extern char registry_path[PATH_MAX];
#endif

rfbFBInfo rfbFB;
DevPrivateKeyRec rfbGCKey;

static Bool initOutputCalled = FALSE;
static Bool noCursor = FALSE;
char *desktopName = DEFAULT_DESKTOP_NAME;
int traceLevel = 0;
int rfbLEDState = (int)rfbLEDUnknown;

char rfbThisHost[256];

Atom VNC_LAST_CLIENT_ID;
Atom VNC_CONNECT;
Atom VNC_OTP;
#ifdef NVCONTROL
Atom VNC_NVCDISPLAY;
#endif

#ifdef XVNC_AuthPAM
#define MAXUSERLEN 63
Atom VNC_ACL;
#endif

static char primaryOrder[4] = "";
static int redBits, greenBits, blueBits;

static Bool rfbScreenInit(ScreenPtr pScreen, int argc, char **argv);
static int rfbKeybdProc(DeviceIntPtr pDevice, int onoff);
static int rfbMouseProc(DeviceIntPtr pDevice, int onoff);
static int rfbExtInputProc(DeviceIntPtr pDevice, int onoff);
static Bool CheckDisplayNumber(int n);

static Bool rfbAlwaysTrue(void);
char *rfbAllocateFramebufferMemory(rfbFBInfoPtr prfb);
static Bool rfbCursorOffScreen(ScreenPtr *ppScreen, int *x, int *y);
static void rfbCrossScreen(ScreenPtr pScreen, Bool entering);
static void rfbClientStateChange(CallbackListPtr *, pointer myData,
                                 pointer client);

static miPointerScreenFuncRec rfbPointerCursorFuncs = {
  rfbCursorOffScreen,
  rfbCrossScreen,
  miPointerWarpCursor
};

XkbRMLVOSet rmlvo = { XKB_DFLT_RULES, XKB_DFLT_MODEL, XKB_DFLT_LAYOUT,
                      XKB_DFLT_VARIANT, XKB_DFLT_OPTIONS };

int inetdSock = -1;
static char inetdDisplayNumStr[10];

/* Interface address to bind to */
struct in_addr interface;
struct in6_addr interface6;
int family = -1;


static void PrintVersion(void)
{
  fprintf(stderr, "TurboVNC Server (Xvnc) %d-bit v"__VERSION" (build "__BUILD")\n",
          (int)sizeof(size_t) * 8);
  fprintf(stderr, "Copyright (C) "__COPYRIGHT_YEAR" "__COPYRIGHT_MSG"\n");
  fprintf(stderr, __URLTEXT"\n\n");
}


static void InitRFB(void)
{
  static Bool firstTime = TRUE;

  if (firstTime) {
    rfbFB.width = RFB_DEFAULT_WIDTH;
    rfbFB.height = RFB_DEFAULT_HEIGHT;
    xorg_list_init(&rfbScreens);
    rfbAddScreen(&rfbScreens,
                 rfbNewScreen(0, 0, 0, rfbFB.width, rfbFB.height, 0));
    xorg_list_init(&rfbAuthFails);
    rfbFB.depth = RFB_DEFAULT_DEPTH;
    rfbFB.blackPixel = RFB_DEFAULT_BLACKPIXEL;
    rfbFB.whitePixel = RFB_DEFAULT_WHITEPIXEL;
    rfbFB.pfbMemory = NULL;
    gethostname(rfbThisHost, 255);
    interface.s_addr = htonl(INADDR_ANY);
    interface6 = in6addr_any;
    firstTime = FALSE;
  }
}


/*
 * ddxProcessArgument is our first entry point and will be called at the
 * very start for each argument.  It is not called again on server reset.
 */

int ddxProcessArgument(int argc, char *argv[], int i)
{
  InitRFB();

  /***** TurboVNC connection options *****/

  #define REQUIRE_ARG() {  \
    if (i + 1 >= argc) {  \
      UseMsg();  \
      exit(1);  \
    }  \
  }

  if (strcasecmp(argv[i], "-alwaysshared") == 0) {
    rfbAlwaysShared = TRUE;
    return 1;
  }

  if (strcasecmp(argv[i], "-capture") == 0) {
    REQUIRE_ARG();
    rfbCaptureFile = strdup(argv[i + 1]);
    return 2;
  }

  if (strcasecmp(argv[i], "-deferupdate") == 0) {  /* -deferupdate ms */
    REQUIRE_ARG();
    rfbDeferUpdateTime = atoi(argv[i + 1]);
    return 2;
  }

  if (strcasecmp(argv[i], "-desktop") == 0) {  /* -desktop desktop-name */
    REQUIRE_ARG();
    desktopName = argv[i + 1];
    return 2;
  }

  if (strcasecmp(argv[i], "-disconnect") == 0) {
    rfbDisconnect = TRUE;
    return 1;
  }

  if (strcasecmp(argv[i], "-dontdisconnect") == 0) {
    rfbDisconnect = FALSE;
    return 1;
  }

  if (strcasecmp(argv[i], "-idletimeout") == 0) {  /* -idletimeout sec */
    REQUIRE_ARG();
    rfbIdleTimeout = atoi(argv[i + 1]);
    return 2;
  }

  if (strcasecmp(argv[i], "-inetd") == 0) {  /* -inetd */
    int n;
    for (n = 1; n < 100; n++) {
      if (CheckDisplayNumber(n))
        break;
    }

    if (n >= 100)
      FatalError("-inetd: couldn't find free display number");

    snprintf(inetdDisplayNumStr, 10, "%d", n);
    display = inetdDisplayNumStr;

    /* fds 0, 1 and 2 (stdin, out and err) are all the same socket to the
       RFB client.  OsInit() closes stdout and stdin, and we don't want
       stderr to go to the RFB client, so make the client socket 3 and
       close stderr.  OsInit() will redirect stderr logging to an
       appropriate log file or /dev/null if that doesn't work. */
    dup2(0, 3);
    inetdSock = 3;
    close(2);

    return 1;
  }

  if (strcasecmp(argv[i], "-interface") == 0) {  /* -interface ipaddr */
    struct addrinfo hints, *addr;
    REQUIRE_ARG();
    if (interface.s_addr != htonl(INADDR_ANY) ||
        memcmp(&interface6, &in6addr_any, sizeof(interface6))) {
      /* Already set (-localhost?) */
      return 2;
    }
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo(argv[i + 1], NULL, &hints, &addr) == 0) {
      struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)addr->ai_addr;
      struct sockaddr_in *addr4 = (struct sockaddr_in *)addr->ai_addr;
      family = addr->ai_family;
      if (family == AF_INET6)
        interface6 = addr6->sin6_addr;
      else
        interface.s_addr = addr4->sin_addr.s_addr;
    }
    return 2;
  }

  if (strcasecmp(argv[i], "-ipv6") == 0) {
    if (family == -1) family = AF_INET6;
    return 1;
  }

  if (strcasecmp(argv[i], "-localhost") == 0) {
    interface.s_addr = htonl(INADDR_LOOPBACK);
    interface6 = in6addr_loopback;
    return 1;
  }

  if (strcasecmp(argv[i], "-maxclipboard") == 0) {
    REQUIRE_ARG();
    rfbMaxClipboard = atoi(argv[i + 1]);
    return 2;
  }

  if (strcasecmp(argv[i], "-maxconnections") == 0) {
    REQUIRE_ARG();
    rfbMaxClientConnections = atoi(argv[i + 1]);
    if (rfbMaxClientConnections < 1 ||
        rfbMaxClientConnections > MAX_MAX_CONNECTIONS) {
      UseMsg();
      exit(1);
    }
    return 2;
  }

  if (strcasecmp(argv[i], "-nevershared") == 0) {
    rfbNeverShared = TRUE;
    return 1;
  }

  if (strcasecmp(argv[i], "-noclipboardrecv") == 0) {
    rfbAuthDisableCBRecv = TRUE;
    return 1;
  }

  if (strcasecmp(argv[i], "-noclipboardsend") == 0) {
    rfbAuthDisableCBSend = TRUE;
    return 1;
  }

  if (strcasecmp(argv[i], "-noflowcontrol") == 0) {
    rfbCongestionControl = FALSE;
    return 1;
  }

  if (strcasecmp(argv[i], "-noprimarysync") == 0) {
    rfbSyncPrimary = FALSE;
    return 1;
  }

  if (strcasecmp(argv[i], "-noreverse") == 0) {
    rfbAuthDisableRevCon = TRUE;
    return 1;
  }

  if (strcasecmp(argv[i], "-rfbport") == 0) {  /* -rfbport port */
    REQUIRE_ARG();
    rfbPort = atoi(argv[i + 1]);
    return 2;
  }

  if (strcasecmp(argv[i], "-rfbunixpath") == 0) {  /* -rfbunixpath path */
    REQUIRE_ARG();
    rfbUDSPath = argv[i + 1];
    return 2;
  }

  if (strcasecmp(argv[i], "-rfbunixmode") == 0) {  /* -rfbunixmode mode */
    char *ptr;
    REQUIRE_ARG();
    ptr = argv[i + 1];
    rfbUDSMode = strtol(argv[i + 1], &ptr, 8);
    if (*argv[i + 1] == 0 || *ptr != 0 || rfbUDSMode < 0 || rfbUDSMode > 0777)
      FatalError("Invalid mode %s\n", argv[i + 1]);
    return 2;
  }

  if (strcasecmp(argv[i], "-rfbwait") == 0) {  /* -rfbwait ms */
    REQUIRE_ARG();
    rfbMaxClientWait = atoi(argv[i + 1]);
    return 2;
  }

  /***** TurboVNC input options *****/

  if (strcasecmp(argv[i], "-compatiblekbd") == 0) {
    compatibleKbd = TRUE;
    return 1;
  }

  if (strcasecmp(argv[i], "-nocursor") == 0) {
    noCursor = TRUE;
    return 1;
  }

  /* Run server in view-only mode - Ehud Karni SW */
  if (strcasecmp(argv[i], "-viewonly") == 0) {
    rfbViewOnly = TRUE;
    return 1;
  }

  if (strcasecmp(argv[i], "-virtualtablet") == 0) {
    rfbVirtualTablet = TRUE;
    return 1;
  }

  if (strcasecmp(argv[i], "-xkblayout") == 0) {
    REQUIRE_ARG();
    rmlvo.layout = argv[i + 1];
    return 2;
  }

  if (strcasecmp(argv[i], "-xkbmodel") == 0) {
    REQUIRE_ARG();
    rmlvo.model = argv[i + 1];
    return 2;
  }

  if (strcasecmp(argv[i], "-xkboptions") == 0) {
    REQUIRE_ARG();
    rmlvo.options = argv[i + 1];
    return 2;
  }

  if (strcasecmp(argv[i], "-xkbrules") == 0) {
    REQUIRE_ARG();
    rmlvo.rules = argv[i + 1];
    return 2;
  }

  if (strcasecmp(argv[i], "-xkbvariant") == 0) {
    REQUIRE_ARG();
    rmlvo.variant = argv[i + 1];
    return 2;
  }

  /***** TurboVNC display options *****/

  if (strcasecmp(argv[i], "-blackpixel") == 0) {  /* -blackpixel n */
    REQUIRE_ARG();
    rfbFB.blackPixel = atoi(argv[i + 1]);
    return 2;
  }

  if (strcasecmp(argv[i], "-depth") == 0) {  /* -depth D */
    REQUIRE_ARG();
    rfbFB.depth = atoi(argv[i + 1]);
    return 2;
  }

#ifndef TURBOVNC_STATIC_XORG_PATHS
  if (strcasecmp(argv[i], "-dridir") == 0) {
#ifdef GLXEXT
    REQUIRE_ARG();
    dri_driver_path = strdup(argv[i + 1]);
#endif
    return 2;
  }
#endif

  if (strcasecmp(argv[i], "-geometry") == 0) {
    /* -geometry WxH or W0xH0+X0+Y0[,W1xH1+X1+Y1,...] */
    char *str, *token;
    int index = 0, w, h, x, y;
    int r = INT_MIN, b = INT_MIN;
    REQUIRE_ARG();
    str = argv[i + 1];
    while ((token = strsep(&str, ",")) != NULL) {
      x = y = 0;
      if ((sscanf(token, "%dx%d+%d+%d", &w, &h, &x, &y) != 4 &&
           sscanf(token, "%dx%d", &w, &h) != 2) ||
          x < 0 || y < 0 || w < 1 || h < 1)
        FatalError("Invalid geometry %s\n", token);
      if (rfbFindScreen(&rfbScreens, x, y, w, h))
        continue;
      if (index == 0) {
        rfbScreenInfo *screen;
        screen = xorg_list_first_entry(&rfbScreens, rfbScreenInfo, entry);
        screen->s.x = x;  screen->s.y = y;
        screen->s.w = w;  screen->s.h = h;
      } else
        rfbAddScreen(&rfbScreens, rfbNewScreen(0, x, y, w, h, 0));
      if (x + w > r) r = x + w;
      if (y + h > b) b = y + h;
      index++;
      if (index > 255)
        FatalError("Cannot create more than 255 screens\n");
    }
    rfbFB.width = r;
    rfbFB.height = b;
    return 2;
  }

#ifdef NVCONTROL
  if (strcasecmp(argv[i], "-nvcontrol") == 0) {
    REQUIRE_ARG();
    nvCtrlDisplay = strdup(argv[i + 1]);
    noNVCTRLExtension = FALSE;
    return 2;
  }
#endif

  if (strcasecmp(argv[i], "-pixelformat") == 0) {
    REQUIRE_ARG();
    if (sscanf(argv[i + 1], "%3s%1d%1d%1d", primaryOrder,
               &redBits, &greenBits, &blueBits) < 4) {
      ErrorF("Invalid pixel format %s\n", argv[i + 1]);
      UseMsg();
      exit(1);
    }

    if (strcasecmp(primaryOrder, "bgr") == 0) {
      int tmp = redBits;
      redBits = blueBits;
      blueBits = tmp;
    } else if (strcasecmp(primaryOrder, "rgb") != 0) {
      ErrorF("Invalid pixel format %s\n", argv[i + 1]);
      UseMsg();
      exit(1);
    }

    return 2;
  }

  if (strcasecmp(argv[i], "-whitepixel") == 0) {  /* -whitepixel n */
    REQUIRE_ARG();
    rfbFB.whitePixel = atoi(argv[i + 1]);
    return 2;
  }

  /***** TurboVNC encoding options *****/

  if (strcasecmp(argv[i], "-alr") == 0) {
    REQUIRE_ARG();
    rfbAutoLosslessRefresh = atof(argv[i + 1]);
    if (rfbAutoLosslessRefresh <= 0.0) {
      UseMsg();
      exit(1);
    }
    return 2;
  }

  if (strcasecmp(argv[i], "-alrqual") == 0) {
    REQUIRE_ARG();
    rfbALRQualityLevel = atoi(argv[i + 1]);
    if (rfbALRQualityLevel < 1 || rfbALRQualityLevel > 100) {
      UseMsg();
      exit(1);
    }
    return 2;
  }

  if (strcasecmp(argv[i], "-alrsamp") == 0) {
    int s;
    REQUIRE_ARG();
    for (s = 0; s < TVNC_SAMPOPT; s++) {
      if (toupper(argv[i + 1][0]) == toupper(subsampStr[s][0]) ||
          (s == TVNC_GRAY && argv[i + 1][0] == '0')) {
        rfbALRSubsampLevel = s;  break;
      }
    }
    if (s >= TVNC_SAMPOPT) {
      UseMsg();
      exit(1);
    }
    return 2;
  }

  if (strcasecmp(argv[i], "-economictranslate") == 0) {
    rfbEconomicTranslate = TRUE;
    return 1;
  }

  if (strcasecmp(argv[i], "-interframe") == 0) {
    rfbInterframe = 1;
    return 1;
  }

#if !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined(__OpenBSD__) && !defined(__DragonFly__)
  if (strcasecmp(argv[i], "-nomt") == 0) {
    rfbMT = FALSE;
    return 1;
  }
#endif

  if (strcasecmp(argv[i], "-nointerframe") == 0) {
    rfbInterframe = 0;
    return 1;
  }

#if !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined(__OpenBSD__) && !defined(__DragonFly__)
  if (strcasecmp(argv[i], "-nthreads") == 0) {
    REQUIRE_ARG();
    rfbNumThreads = atoi(argv[i + 1]);
    if (rfbNumThreads < 1 || rfbNumThreads > MAX_ENCODING_THREADS) {
      UseMsg();
      exit(1);
    }
    return 2;
  }
#endif

  /***** TurboVNC security and authentication options *****/

  if (strcasecmp(argv[i], "-maxauthfails") == 0) {
    REQUIRE_ARG();
    rfbAuthMaxFails = atoi(argv[i + 1]);
    if (rfbAuthMaxFails < 0) {
      UseMsg();
      exit(1);
    }
    return 2;
  }

  if (strcasecmp(argv[i], "-authfailtimeout") == 0) {
    REQUIRE_ARG();
    rfbAuthFailTimeout = atoi(argv[i + 1]);
    if (rfbAuthFailTimeout < 1) {
      UseMsg();
      exit(1);
    }
    return 2;
  }

#ifdef XVNC_AuthPAM
  if (strcasecmp(argv[i], "-pamsession") == 0) {
    rfbAuthPAMSession = TRUE;
    return 1;
  }
#endif

  if (strcasecmp(argv[i], "-rfbauth") == 0) {  /* -rfbauth passwd-file */
    REQUIRE_ARG();
    rfbAuthPasswdFile = argv[i + 1];
    return 2;
  }

  if (strcasecmp(argv[i], "-securitytypes") == 0) {
    REQUIRE_ARG();
    rfbAuthParseCommandLine(argv[i + 1]);
    return 2;
  }

#if USETLS
  if (strcasecmp(argv[i], "-x509cert") == 0) {
    REQUIRE_ARG();
    rfbAuthX509Cert = argv[i + 1];
    return 2;
  }

  if (strcasecmp(argv[i], "-x509key") == 0) {
    REQUIRE_ARG();
    rfbAuthX509Key = argv[i + 1];
    return 2;
  }
#endif

  /***** TurboVNC miscellaneous options *****/

  if (strcasecmp(argv[i], "-giidebug") == 0) {
    rfbGIIDebug = TRUE;
    return 1;
  }

#ifndef TURBOVNC_STATIC_XORG_PATHS
  if (strcasecmp(argv[i], "-registrydir") == 0) {
#ifdef X_REGISTRY_REQUEST
    REQUIRE_ARG();
    snprintf(registry_path, PATH_MAX, "%s/protocol.txt", argv[i + 1]);
#endif
    return 2;
  }
#endif

  if (strcasecmp(argv[i], "-verbose") == 0) {
    LogSetParameter(XLOG_VERBOSITY, X_DEBUG);
    return 1;
  }

  if (strcasecmp(argv[i], "-version") == 0) {
    PrintVersion();
    exit(0);
  }

  if (inetdSock != -1 && argv[i][0] == ':')
    FatalError("can't specify both -inetd and :displaynumber");

  return 0;
}


/*
 * InitOutput is called every time the server resets.  It should call
 * AddScreen for each screen (but we only ever have one), and in turn this
 * will call rfbScreenInit.
 */

/* Common pixmap formats */

static PixmapFormatRec formats[MAXFORMATS] = {
  { 1,  1,  BITMAP_SCANLINE_PAD },
  { 4,  8,  BITMAP_SCANLINE_PAD },
  { 8,  8,  BITMAP_SCANLINE_PAD },
  { 15, 16, BITMAP_SCANLINE_PAD },
  { 16, 16, BITMAP_SCANLINE_PAD },
  { 24, 32, BITMAP_SCANLINE_PAD },
#ifdef RENDER
  { 32, 32, BITMAP_SCANLINE_PAD },
  { 32, 32, BITMAP_SCANLINE_PAD },
#endif
};
#ifdef RENDER
static int numFormats = 7;
#else
static int numFormats = 6;
#endif


static void rfbBlockHandler(void *blockData, void *timeout)
{
  IdleTimerCheck();
}

static void rfbWakeupHandler(void *blockData, int result)
{
}


void InitOutput(ScreenInfo *pScreenInfo, int argc, char **argv)
{
  int i;

  initOutputCalled = TRUE;
#ifdef GLXEXT
  xorgGlxCreateVendor();
#endif

  rfbLog("Desktop name '%s' (%s:%s)\n", desktopName, rfbThisHost, display);
  rfbLog("Protocol versions supported: 3.3, 3.7, 3.8, 3.7t, 3.8t\n");

  VNC_LAST_CLIENT_ID =
    MakeAtom("VNC_LAST_CLIENT_ID", strlen("VNC_LAST_CLIENT_ID"), TRUE);
  VNC_CONNECT = MakeAtom("VNC_CONNECT", strlen("VNC_CONNECT"), TRUE);

  if (rfbOptOtpAuth())
    VNC_OTP = MakeAtom("VNC_OTP", strlen("VNC_OTP"), TRUE);

#ifdef XVNC_AuthPAM
  VNC_ACL = MakeAtom("VNC_ACL", strlen("VNC_ACL"), TRUE);
#endif

#ifdef NVCONTROL
  VNC_NVCDISPLAY = MakeAtom("VNC_NVCDISPLAY", strlen("VNC_NVCDISPLAY"), TRUE);
#endif

  rfbInitSockets();

  /* Initialize pixmap formats */

  pScreenInfo->imageByteOrder = IMAGE_BYTE_ORDER;
  pScreenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
  pScreenInfo->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
  pScreenInfo->bitmapBitOrder = BITMAP_BIT_ORDER;
  if (rfbFB.depth == 30) {
    numFormats++;
#ifdef RENDER
    formats[numFormats - 2] = (PixmapFormatRec) {
#else
    formats[numFormats - 1] = (PixmapFormatRec) {
#endif
      .depth = 30, .bitsPerPixel = 32,
      .scanlinePad = BITMAP_SCANLINE_PAD
    };
  }
  pScreenInfo->numPixmapFormats = numFormats;
  for (i = 0; i < numFormats; i++)
    pScreenInfo->formats[i] = formats[i];

  if (!AddCallback(&ClientStateCallback, rfbClientStateChange, NULL)) {
    rfbLog("InitOutput: AddCallback failed\n");
    return;
  }

  /* Initialize screen */

  if (AddScreen(rfbScreenInit, argc, argv) == -1)
    FatalError("Couldn't add screen");

  RegisterBlockAndWakeupHandlers(rfbBlockHandler, rfbWakeupHandler, 0);
}


static Bool rfbScreenInit(ScreenPtr pScreen, int argc, char **argv)
{
  rfbFBInfoPtr prfb = &rfbFB;
  int dpix = 96, dpiy = 96;
  int ret;
  char *pbits;
  VisualPtr vis;
#ifdef RENDER
  PictureScreenPtr ps;
#endif
  BOOL bigEndian = !(*(char *)&rfbEndianTest);

  if (monitorResolution != 0) {
    dpix = monitorResolution;
    dpiy = monitorResolution;
  }

  prfb->paddedWidthInBytes = PixmapBytePad(prfb->width, prfb->depth);
  prfb->bitsPerPixel = rfbBitsPerPixel(prfb->depth);
  pbits = rfbAllocateFramebufferMemory(prfb);
  if (!pbits) return FALSE;

  switch (prfb->depth) {
    case 8:
      miSetVisualTypesAndMasks(8, (1 << StaticGray) | (1 << GrayScale) |
                               (1 << StaticColor) | (1 << PseudoColor) |
                               (1 << TrueColor) | (1 << DirectColor),
                               8, PseudoColor, 0, 0, 0);
      break;
    case 16:
      miSetVisualTypesAndMasks(16, (1 << TrueColor) | (1 << DirectColor),
                               8, TrueColor, 0xf800, 0x07e0, 0x001f);
      break;
    case 24:
      miSetVisualTypesAndMasks(24, (1 << TrueColor) | (1 << DirectColor),
                               8, TrueColor, 0xff0000, 0x00ff00, 0x0000ff);
      break;
    case 30:
      miSetVisualTypesAndMasks(30, (1 << TrueColor) | (1 << DirectColor),
                               10, TrueColor, 0x3ff00000, 0x000ffc00,
                               0x000003ff);
      break;
    case 32:
      miSetVisualTypesAndMasks(32, (1 << TrueColor) | (1 << DirectColor),
                               8, TrueColor, 0xff000000, 0x00ff0000,
                               0x0000ff00);
      break;
    default:
      rfbLog("Depth %d not supported\n", prfb->depth);
      return FALSE;
  }

  miSetPixmapDepths();

  switch (prfb->bitsPerPixel) {
    case 8:
      ret = fbScreenInit(pScreen, pbits, prfb->width, prfb->height, dpix, dpiy,
                         prfb->paddedWidthInBytes, 8);
      break;
    case 16:
      ret = fbScreenInit(pScreen, pbits, prfb->width, prfb->height, dpix, dpiy,
                         prfb->paddedWidthInBytes / 2, 16);
      blueBits = 5;  greenBits = 6;  redBits = 5;
      break;
    case 32:
      if (prfb->depth == 30) {
        VisualPtr visuals;
        DepthPtr depths;
        int nVisuals, nDepths, rootDepth = 0;
        VisualID defaultVisual;

        if (!fbSetupScreen(pScreen, pbits, prfb->width, prfb->height,
                           dpix, dpiy, prfb->paddedWidthInBytes / 4, 32))
          ret = FALSE;
        if (!fbInitVisuals(&visuals, &depths, &nVisuals, &nDepths, &rootDepth,
                           &defaultVisual, ((unsigned long)1 << 31), 10))
          ret = FALSE;
        if (!miScreenInit(pScreen, pbits, prfb->width, prfb->height,
                          dpix, dpiy, prfb->paddedWidthInBytes / 4, rootDepth,
                          nDepths, depths, defaultVisual, nVisuals, visuals))
          ret = FALSE;
        pScreen->CloseScreen = fbCloseScreen;
        blueBits = greenBits = redBits = 10;
        ret = TRUE;
      } else {
        ret = fbScreenInit(pScreen, pbits, prfb->width, prfb->height,
                           dpix, dpiy, prfb->paddedWidthInBytes / 4, 32);
        blueBits = greenBits = redBits = 8;
      }
      break;
    default:
      return FALSE;
  }

  if (!ret) return FALSE;

  if (prfb->bitsPerPixel > 8) {
    int xBits = prfb->bitsPerPixel - redBits - greenBits - blueBits;
    if (strcasecmp(primaryOrder, "bgr") == 0) {
      if (bigEndian)
        rfbLog("Framebuffer: XBGR %d/%d/%d/%d\n",
               xBits, blueBits, greenBits, redBits);
      else
        rfbLog("Framebuffer: RGBX %d/%d/%d/%d\n",
               redBits, greenBits, blueBits, xBits);
      vis = pScreen->visuals + pScreen->numVisuals;
      while (--vis >= pScreen->visuals) {
        if ((vis->class | DynamicClass) == DirectColor) {
          vis->offsetRed = 0;
          vis->redMask = (1 << redBits) - 1;
          vis->offsetGreen = redBits;
          vis->greenMask = ((1 << greenBits) - 1) << vis->offsetGreen;
          vis->offsetBlue = redBits + greenBits;
          vis->blueMask = ((1 << blueBits) - 1) << vis->offsetBlue;
        }
      }
    } else {
      if (bigEndian)
        rfbLog("Framebuffer: XRGB %d/%d/%d/%d\n",
               xBits, redBits, greenBits, blueBits);
      else
        rfbLog("Framebuffer: BGRX %d/%d/%d/%d\n",
               blueBits, greenBits, redBits, xBits);
      vis = pScreen->visuals + pScreen->numVisuals;
      while (--vis >= pScreen->visuals) {
        if ((vis->class | DynamicClass) == DirectColor) {
          vis->offsetBlue = 0;
          vis->blueMask = (1 << blueBits) - 1;
          vis->offsetGreen = blueBits;
          vis->greenMask = ((1 << greenBits) - 1) << vis->offsetGreen;
          vis->offsetRed = blueBits + greenBits;
          vis->redMask = ((1 << redBits) - 1) << vis->offsetRed;
        }
      }
    }
  }

  if (prfb->bitsPerPixel > 4)
    fbPictureInit(pScreen, 0, 0);

  if (!dixRegisterPrivateKey(&rfbGCKey, PRIVATE_GC, sizeof(rfbGCRec)))
    FatalError("rfbScreenInit: dixRegisterPrivateKey failed");

  prfb->cursorIsDrawn = FALSE;
  prfb->dontSendFramebufferUpdate = FALSE;

  prfb->CloseScreen = pScreen->CloseScreen;
  prfb->CreateGC = pScreen->CreateGC;
  prfb->CopyWindow = pScreen->CopyWindow;
  prfb->ClearToBackground = pScreen->ClearToBackground;
#ifdef RENDER
  ps = GetPictureScreenIfSet(pScreen);
  if (ps) {
    prfb->Composite = ps->Composite;
    prfb->Glyphs = ps->Glyphs;
  }
#endif
  prfb->InstallColormap = pScreen->InstallColormap;
  prfb->UninstallColormap = pScreen->UninstallColormap;
  prfb->ListInstalledColormaps = pScreen->ListInstalledColormaps;
  prfb->StoreColors = pScreen->StoreColors;
  prfb->SaveScreen = pScreen->SaveScreen;

  pScreen->CloseScreen = rfbCloseScreen;
  pScreen->CreateGC = rfbCreateGC;
  pScreen->CopyWindow = rfbCopyWindow;
  pScreen->ClearToBackground = rfbClearToBackground;
#ifdef RENDER
  if (ps) {
    ps->Composite = rfbComposite;
    ps->Glyphs = rfbGlyphs;
  }
#endif
  pScreen->InstallColormap = rfbInstallColormap;
  pScreen->UninstallColormap = rfbUninstallColormap;
  pScreen->ListInstalledColormaps = rfbListInstalledColormaps;
  pScreen->StoreColors = rfbStoreColors;
  pScreen->SaveScreen = (SaveScreenProcPtr)rfbAlwaysTrue;

  rfbDCInitialize(pScreen, &rfbPointerCursorFuncs);

  if (noCursor) {
    pScreen->DisplayCursor = (DisplayCursorProcPtr)rfbAlwaysTrue;
    prfb->cursorIsDrawn = TRUE;
  }

  pScreen->blackPixel = prfb->blackPixel;
  pScreen->whitePixel = prfb->whitePixel;

  rfbServerFormat.bitsPerPixel = prfb->bitsPerPixel;
  rfbServerFormat.depth = prfb->depth;
  rfbServerFormat.bigEndian = bigEndian;

  /* Find the root visual and set the server format */
  for (vis = pScreen->visuals; vis->vid != pScreen->rootVisual; vis++);
  rfbServerFormat.trueColour = (vis->class == TrueColor);

  if ((vis->class == TrueColor) || (vis->class == DirectColor)) {
    rfbServerFormat.redMax = vis->redMask >> vis->offsetRed;
    rfbServerFormat.greenMax = vis->greenMask >> vis->offsetGreen;
    rfbServerFormat.blueMax = vis->blueMask >> vis->offsetBlue;
    rfbServerFormat.redShift = vis->offsetRed;
    rfbServerFormat.greenShift = vis->offsetGreen;
    rfbServerFormat.blueShift = vis->offsetBlue;
  } else {
    rfbServerFormat.redMax = rfbServerFormat.greenMax =
      rfbServerFormat.blueMax = 0;
    rfbServerFormat.redShift = rfbServerFormat.greenShift =
      rfbServerFormat.blueShift = 0;
  }

  ret = fbCreateDefColormap(pScreen);

#ifdef RANDR
  if (!vncRRInit(pScreen)) return FALSE;
#endif

  rfbLog("Maximum clipboard transfer size: %d bytes\n", rfbMaxClipboard);

  return ret;

}  /* end rfbScreenInit */


/*
 * These values are appropriate for Wacom tablets.  Unknown if they will work
 * for anything else.
 */

rfbDevInfo virtualTabletTouch =
  { "TurboVNC virtual tablet touch", 16, FALSE, 0, 6, Absolute, 0,
    rfbGIIDevTypeTouch, NULL,
    { { 0, AXIS_LABEL_PROP_ABS_X, "0", 0, 2048, 4096, rfbGIIUnitLength,
        0, 1, 26000, 0 },
      { 0, AXIS_LABEL_PROP_ABS_Y, "1", 0, 2048, 4096, rfbGIIUnitLength,
        0, 1, 41000, 0 },
      { 0, AXIS_LABEL_PROP_ABS_PRESSURE, "2", 0, 1024, 2048, rfbGIIUnitLength,
        0, 1, 1, 0 },
      { 0, "None", "3", 0, 0, 1, rfbGIIUnitLength,
        0, 1, 1, 0 },
      { 0, "None", "4", 0, 0, 1, rfbGIIUnitLength,
        0, 1, 1, 0 },
      { 0, "None", "5", 0, 0, 1, rfbGIIUnitLength,
        0, 1, 1, 0 } }
  };

rfbDevInfo virtualTabletStylus =
  { "TurboVNC virtual tablet stylus", 16, FALSE, 0, 6, Absolute, 0,
    rfbGIIDevTypeStylus, NULL,
    { { 0, AXIS_LABEL_PROP_ABS_X, "0", 0, 15748, 31496, rfbGIIUnitLength,
        0, 1, 200000, 0 },
      { 0, AXIS_LABEL_PROP_ABS_Y, "1", 0, 9843, 19685, rfbGIIUnitLength,
        0, 1, 200000, 0 },
      { 0, AXIS_LABEL_PROP_ABS_PRESSURE, "2", 0, 1024, 2048, rfbGIIUnitLength,
        0, 1, 1, 0 },
      { 0, AXIS_LABEL_PROP_ABS_TILT_X, "3", -64, 0, 63, rfbGIIUnitLength,
        0, 1, 57, 0 },
      { 0, AXIS_LABEL_PROP_ABS_TILT_Y, "4", -64, 0, 63, rfbGIIUnitLength,
        0, 1, 57, 0 },
      { 0, AXIS_LABEL_PROP_ABS_WHEEL, "5", -900, 0, 899, rfbGIIUnitLength,
        0, 1, 1, 0 } }
  };

rfbDevInfo virtualTabletEraser =
  { "TurboVNC virtual tablet eraser", 16, FALSE, 0, 6, Absolute, 0,
    rfbGIIDevTypeEraser, NULL,
    { { 0, AXIS_LABEL_PROP_ABS_X, "0", 0, 15748, 31496, rfbGIIUnitLength,
        0, 1, 200000, 0 },
      { 0, AXIS_LABEL_PROP_ABS_Y, "1", 0, 9843, 19685, rfbGIIUnitLength,
        0, 1, 200000, 0 },
      { 0, AXIS_LABEL_PROP_ABS_PRESSURE, "2", 0, 1024, 2048, rfbGIIUnitLength,
        0, 1, 1, 0 },
      { 0, AXIS_LABEL_PROP_ABS_TILT_X, "3", -64, 0, 63, rfbGIIUnitLength,
        0, 1, 57, 0 },
      { 0, AXIS_LABEL_PROP_ABS_TILT_Y, "4", -64, 0, 63, rfbGIIUnitLength,
        0, 1, 57, 0 },
      { 0, "None", "5", 0, 0, 1, rfbGIIUnitLength,
        0, 1, 1, 0 } }
  };

rfbDevInfo virtualTabletPad =
  { "TurboVNC virtual tablet pad", 16, FALSE, 0, 6, Absolute, 0,
    rfbGIIDevTypePad, NULL,
    { { 0, AXIS_LABEL_PROP_ABS_X, "0", 0, 0, 0, rfbGIIUnitLength,
        0, 1, 0, 0 },
      { 0, AXIS_LABEL_PROP_ABS_Y, "1", 0, 0, 0, rfbGIIUnitLength,
        0, 1, 0, 0 },
      { 0, "None", "2", 0, 0, 1, rfbGIIUnitLength,
        0, 1, 1, 0 },
      { 0, "None", "3", 0, 0, 1, rfbGIIUnitLength,
        0, 1, 1, 0 },
      { 0, "None", "4", 0, 0, 1, rfbGIIUnitLength,
        0, 1, 1, 0 },
      { 0, AXIS_LABEL_PROP_ABS_WHEEL, "5", 0, 36, 71, rfbGIIUnitLength,
        0, 1, 1, 0 } }
  };


/*
 * InitInput is also called every time the server resets.  It is called after
 * InitOutput, so we can assume that rfbInitSockets has already been called.
 */

void InitInput(int argc, char *argv[])
{
  DeviceIntPtr p, k;

  if (AllocDevicePair(serverClient, "TurboVNC", &p, &k, rfbMouseProc,
                      rfbKeybdProc, FALSE) != Success)
    FatalError("Could not initialize TurboVNC input devices");

  if (ActivateDevice(p, TRUE) != Success || ActivateDevice(k, TRUE) != Success)
    FatalError("Could not activate TurboVNC input devices");

  if (!EnableDevice(p, TRUE) || !EnableDevice(k, TRUE))
    FatalError("Could not enable TurboVNC input devices");

  mieqInit();
  mieqSetHandler(ET_KeyPress, vncXkbProcessDeviceEvent);
  mieqSetHandler(ET_KeyRelease, vncXkbProcessDeviceEvent);

  if (rfbVirtualTablet) {
    if (!AddExtInputDevice(&virtualTabletTouch))
      FatalError("Could not create TurboVNC virtual tablet touch device");
    if (!AddExtInputDevice(&virtualTabletStylus))
      FatalError("Could not create TurboVNC virtual tablet stylus device");
    if (!AddExtInputDevice(&virtualTabletEraser))
      FatalError("Could not create TurboVNC virtual tablet eraser device");
    if (!AddExtInputDevice(&virtualTabletPad))
      FatalError("Could not create TurboVNC virtual tablet pad device");
  }
}


Bool AddExtInputDevice(rfbDevInfo *dev)
{
  int i;
  Atom btn_labels[MAX_BUTTONS], axes_labels[MAX_VALUATORS];
  BYTE map[MAX_BUTTONS + 1];
  DeviceIntPtr devtmp;
  int numValuators = dev->numValuators;

  if (numValuators > 2) {
    rfbGIIValuator *idVal = &dev->valuators[dev->numValuators - 2];
    rfbGIIValuator *typeVal = &dev->valuators[dev->numValuators - 1];

    if (!strcmp((char *)idVal->longName, "__TURBOVNC FAKE TOUCH ID__") &&
        !strcmp((char *)idVal->shortName, "TFTI") &&
        idVal->rangeMin == 0 && idVal->rangeMax == INT_MAX &&
        !strcmp((char *)typeVal->longName, "__TURBOVNC FAKE TOUCH TYPE__") &&
        !strcmp((char *)typeVal->shortName, "TFTT") &&
        typeVal->rangeMin == 0 && typeVal->rangeMax == 5) {
      numValuators -= 2;
      dev->multitouch = TRUE;
    }
  }

  for (devtmp = inputInfo.devices; devtmp; devtmp = devtmp->next) {
    if (!strcmp(devtmp->name, dev->name)) {
      rfbLog("Device \'%s\' already exists\n", dev->name);
      dev->pDev = devtmp;
      return TRUE;
    }
  }

  if ((dev->pDev =
       AddInputDevice(serverClient, rfbExtInputProc, TRUE)) == NULL) {
    rfbLog("ERROR: Could not add extended input device\n");
    goto bailout;
  }

  if (asprintf(&dev->pDev->name, "%s", dev->name) < 0) {
    rfbLogPerror("ERROR: Could not initialize extended input device");
    goto bailout;
  }
  dev->pDev->public.processInputProc = ProcessOtherEvent;
  dev->pDev->public.realInputProc = ProcessOtherEvent;
  XkbSetExtension(dev->pDev, ProcessPointerEvent);
  dev->pDev->deviceGrab.ActivateGrab = ActivatePointerGrab;
  dev->pDev->deviceGrab.DeactivateGrab = DeactivatePointerGrab;
  dev->pDev->coreEvents = TRUE;
  dev->pDev->spriteInfo->spriteOwner = TRUE;
  dev->pDev->lastSlave = NULL;
  dev->pDev->last.slave = NULL;
  dev->pDev->type = SLAVE;
  switch (dev->productID) {
    case rfbGIIDevTypeCursor:
      dev->pDev->xinput_type = XA_CURSOR;
      break;
    case rfbGIIDevTypeStylus:
      dev->pDev->xinput_type = MakeAtom("STYLUS", strlen("STYLUS"), 1);
      break;
    case rfbGIIDevTypeEraser:
      dev->pDev->xinput_type = MakeAtom("ERASER", strlen("ERASER"), 1);
      break;
    case rfbGIIDevTypeTouch:
      dev->pDev->xinput_type = MakeAtom("TOUCH", strlen("TOUCH"), 1);
      break;
    case rfbGIIDevTypePad:
      dev->pDev->xinput_type = MakeAtom("PAD", strlen("PAD"), 1);
      break;
    default:
      if (stristr(dev->name, "cursor"))
        dev->pDev->xinput_type = XA_CURSOR;
      else if (stristr(dev->name, "stylus"))
        dev->pDev->xinput_type = MakeAtom("STYLUS", strlen("STYLUS"), 1);
      else if (stristr(dev->name, "eraser"))
        dev->pDev->xinput_type = MakeAtom("ERASER", strlen("ERASER"), 1);
      else if (stristr(dev->name, "touch"))
        dev->pDev->xinput_type = MakeAtom("TOUCH", strlen("TOUCH"), 1);
      else if (stristr(dev->name, "pad"))
        dev->pDev->xinput_type = MakeAtom("PAD", strlen("PAD"), 1);
      break;
  }

  for (i = 0; i < dev->numButtons; i++) {
    char name[11];
    map[i + 1] = i + 1;
    snprintf(name, 11, "Button %d", i + 1);
    btn_labels[i] = MakeAtom(name, strlen(name), TRUE);
  }

  for (i = 0; i < numValuators; i++) {
    char *name = (char *)dev->valuators[i].longName;
    axes_labels[i] = MakeAtom(name, strlen(name), TRUE);
  }
  InitPointerDeviceStruct((DevicePtr)dev->pDev, map, dev->numButtons,
                          btn_labels, (PtrCtrlProcPtr)NoopDDA,
                          GetMotionHistorySize(), numValuators, axes_labels);
  if (dev->multitouch)
    InitTouchClassDeviceStruct(dev->pDev, dev->numTouches, XIDirectTouch,
                               numValuators);
  InitPointerAccelerationScheme(dev->pDev, PtrAccelNoOp);
  for (i = 0; i < numValuators; i++) {
    int res = dev->valuators[i].siDiv;
    InitValuatorAxisStruct(dev->pDev, i, axes_labels[i],
                           dev->valuators[i].rangeMin,
                           dev->valuators[i].rangeMax, res, res, res,
                           dev->mode);
  }
  if (ActivateDevice(dev->pDev, TRUE) != Success) {
    rfbLog("ERROR: Could not activate extended input device\n");
    goto bailout;
  }
  if (!EnableDevice(dev->pDev, TRUE)) {
    rfbLog("ERROR: Could not enable extended input device\n");
    goto bailout;
  }

  return TRUE;

  bailout:
  if (dev->pDev) {
    free(dev->pDev->name);
    dev->pDev->name = NULL;
    RemoveDevice(dev->pDev, FALSE);
    dev->pDev = NULL;
  }
  return FALSE;
}


void RemoveExtInputDevice(rfbClientPtr cl, int index)
{
  rfbClientPtr cl2;
  int i;
  Bool canDelete = TRUE;

  if (index < 0 || index >= cl->numDevices)
    return;

  for (cl2 = rfbClientHead; cl2; cl2 = cl2->next) {
    if (cl2 == cl)
      continue;
    for (i = 0; i < cl2->numDevices; i++) {
      if (!strcmp(cl2->devices[i].name, cl->devices[index].name))
        canDelete = FALSE;
    }
  }
  if (canDelete) {
    rfbLog("Device \'%s\' no longer used by any clients.  Deleting.\n",
           cl->devices[index].name);
    if (cl->devices[index].pDev)
      RemoveDevice(cl->devices[index].pDev, FALSE);
    for (i = index; i < cl->numDevices - 1; i++)
      memcpy(&cl->devices[i], &cl->devices[i + 1], sizeof(rfbDevInfo));
    if (cl->numDevices > 0)
      cl->numDevices--;
  }
}


void CloseInput(void)
{
}


static void rfbKeyboardBell(int volume, DeviceIntPtr pDev, void *ctrl,
                            int feedbackClass)
{
  if (volume > 0)
    rfbSendBell();
}

static void rfbKeyboardCtrl(DeviceIntPtr device, KeybdCtrl *ctrl)
{
  rfbClientPtr cl, nextCl;
  int state = 0;

  if (ctrl->leds & (1 << 0))
    state |= rfbLEDCapsLock;
  if (ctrl->leds & (1 << 1))
    state |= rfbLEDNumLock;
  if (ctrl->leds & (1 << 2))
    state |= rfbLEDScrollLock;

  if (state == rfbLEDState)
    return;

  rfbLEDState = state;

  for (cl = rfbClientHead; cl; cl = nextCl) {
    nextCl = cl->next;

    if (cl->state != RFB_NORMAL || !SUPPORTS_LED_STATE(cl))
      continue;

    cl->ledState = rfbLEDState;
    cl->pendingLEDState = TRUE;

    rfbSendFramebufferUpdate(cl);
  }
}

static int rfbKeybdProc(DeviceIntPtr pDevice, int onoff)
{
  DevicePtr pDev = (DevicePtr)pDevice;

  switch (onoff) {
    case DEVICE_INIT:
      KbdDeviceInit(pDevice);
      InitKeyboardDeviceStruct(pDevice, &rmlvo, rfbKeyboardBell,
                               rfbKeyboardCtrl);
      QEMUExtKeyboardEventInit();
      break;
    case DEVICE_ON:
      pDev->on = TRUE;
      break;
    case DEVICE_OFF:
      pDev->on = FALSE;
      break;
  }
  return Success;
}


static int rfbMouseProc(DeviceIntPtr pDevice, int onoff)
{
  BYTE map[6];
  DevicePtr pDev = (DevicePtr)pDevice;

  switch (onoff) {
    case DEVICE_INIT:
    {
      Atom btn_labels[5], axes_labels[2];

      map[1] = 1;
      map[2] = 2;
      map[3] = 3;
      map[4] = 4;
      map[5] = 5;

      btn_labels[0] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_LEFT);
      btn_labels[1] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_MIDDLE);
      btn_labels[2] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_RIGHT);
      btn_labels[3] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_WHEEL_UP);
      btn_labels[4] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_WHEEL_DOWN);

      axes_labels[0] = XIGetKnownProperty(AXIS_LABEL_PROP_REL_X);
      axes_labels[1] = XIGetKnownProperty(AXIS_LABEL_PROP_REL_Y);

      InitPointerDeviceStruct(pDev, map, 5, btn_labels,
                              (PtrCtrlProcPtr)NoopDDA,
                              GetMotionHistorySize(), 2, axes_labels);
      break;
    }

    case DEVICE_ON:
      pDev->on = TRUE;
      PtrDeviceOn(pDevice);
      break;

    case DEVICE_OFF:
      pDev->on = FALSE;
      break;
  }
  return Success;
}


int rfbExtInputProc(DeviceIntPtr pDevice, int onoff)
{
  DevicePtr pDev = (DevicePtr)pDevice;

  switch (onoff) {
    case DEVICE_INIT:
      break;

    case DEVICE_ON:
      pDev->on = TRUE;
      break;

    case DEVICE_OFF:
      pDev->on = FALSE;
      break;
  }
  return Success;
}


Bool LegalModifier(unsigned int key, DeviceIntPtr pDev)
{
  return TRUE;
}


void ProcessInputEvents(void)
{
  static Bool inetdInitDone = FALSE;

  if (!inetdInitDone && inetdSock != -1) {
    rfbNewClientConnection(inetdSock);
    inetdInitDone = TRUE;
  }
  mieqProcessInputEvents();
}


static Bool CheckDisplayNumber(int n)
{
  char fname[32];
  int sock;
  rfbSockAddr addr;

  sock = socket(AF_INET6, SOCK_STREAM, 0);

  memset(&addr, 0, sizeof(addr));
  addr.u.sin6.sin6_family = AF_INET6;
  addr.u.sin6.sin6_addr = in6addr_any;
  addr.u.sin6.sin6_port = htons(6000 + n);
  if (bind(sock, &addr.u.sa, sizeof(struct sockaddr_in6)) < 0) {
    close(sock);
    return FALSE;
  }
  close(sock);

  sock = socket(AF_INET, SOCK_STREAM, 0);

  memset(&addr, 0, sizeof(addr));
  addr.u.sin.sin_family = AF_INET;
  addr.u.sin.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.u.sin.sin_port = htons(6000 + n);
  if (bind(sock, &addr.u.sa, sizeof(struct sockaddr_in)) < 0) {
    close(sock);
    return FALSE;
  }
  close(sock);

  sprintf(fname, "/tmp/.X%d-lock", n);
  if (access(fname, F_OK) == 0)
    return FALSE;

  sprintf(fname, "/tmp/.X11-unix/X%d", n);
  if (access(fname, F_OK) == 0)
    return FALSE;

  return TRUE;
}


void rfbRootPropertyChange(PropertyPtr pProp)
{
  if (!rfbAuthDisableRevCon && (pProp->propertyName == VNC_CONNECT) &&
      (pProp->type == XA_STRING) && (pProp->format == 8)) {
    char *colonPos;
    int port = 5500;
    char *host = (char *)rfbAlloc(pProp->size + 1);
    memcpy(host, pProp->data, pProp->size);
    host[pProp->size] = 0;
    colonPos = strrchr(host, ':');
    if (colonPos && colonPos < &host[strlen(host) - 1]) {
      port = atoi(colonPos + 1);
      *colonPos = 0;
    }

    rfbReverseConnection(host, port, -1);

    free(host);
  }

  if (rfbOptOtpAuth() && (pProp->propertyName == VNC_OTP) &&
      (pProp->type == XA_STRING) && (pProp->format == 8)) {
    if (pProp->size == 0) {
      if (rfbAuthOTPValue != NULL) {
        free(rfbAuthOTPValue);
        rfbAuthOTPValue = NULL;
        rfbLog("One time password(s) disabled\n");
      }

    } else if ((pProp->size == MAXPWLEN) || (pProp->size == (MAXPWLEN * 2))) {
      free(rfbAuthOTPValue);
      rfbAuthOTPValueLen = pProp->size;
      rfbAuthOTPValue = (char *)rfbAlloc(pProp->size);
      memcpy(rfbAuthOTPValue, pProp->data, pProp->size);
    }

    memset(pProp->data, 0, pProp->size);
    /* Delete the property?  How? */
  }

#ifdef XVNC_AuthPAM
  if ((pProp->propertyName == VNC_ACL) && (pProp->type == XA_STRING) &&
      (pProp->format == 8) && (pProp->size > 1) &&
      (pProp->size <= (MAXUSERLEN + 1))) {

    /*
     * The first byte is a flag that selects revoke/add.
     * The remaining bytes are the name.
     */
    char *p = (char *)rfbAlloc(pProp->size);
    const char *n = (const char *)pProp->data;

    memcpy(p, &n[1], pProp->size - 1);
    p[pProp->size - 1] = '\0';
    if ((n[0] & 1) == 0) {
      rfbAuthRevokeUser(p);
      free(p);

    } else {
      rfbAuthAddUser(p, (n[0] & 0x10) ? TRUE : FALSE);
    }

    memset(pProp->data, 0, pProp->size);
    /* Delete the property?  How? */
  }
#endif

#ifdef NVCONTROL
  if ((pProp->propertyName == VNC_NVCDISPLAY) && (pProp->type == XA_STRING) &&
      (pProp->format == 8) && (pProp->size > 1) && (pProp->size <= 1024)) {
    free(nvCtrlDisplay);
    nvCtrlDisplay = (char *)rfbAlloc(pProp->size + 1);
    memcpy(nvCtrlDisplay, pProp->data, pProp->size);
    nvCtrlDisplay[pProp->size] = '\0';
  }
#endif
}


int rfbBitsPerPixel(int depth)
{
  if (depth == 1) return 1;
  else if (depth <= 8) return 8;
  else if (depth <= 16) return 16;
  else return 32;
}


static Bool rfbAlwaysTrue(void)
{
  return TRUE;
}


char *rfbAllocateFramebufferMemory(rfbFBInfoPtr prfb)
{
  if (prfb->pfbMemory) return prfb->pfbMemory;  /* already done */

  prfb->sizeInBytes = (prfb->paddedWidthInBytes * prfb->height);

  prfb->pfbMemory = rfbAlloc0(prfb->sizeInBytes);

  return prfb->pfbMemory;
}


static Bool rfbCursorOffScreen(ScreenPtr *ppScreen, int *x, int *y)
{
  return FALSE;
}


static void rfbCrossScreen(ScreenPtr pScreen, Bool entering)
{
}


static void rfbClientStateChange(CallbackListPtr *cbl, pointer myData,
                                 pointer clt)
{
  dispatchException &= ~DE_RESET;       /* hack - force server not to reset */
}


void ddxGiveUp(enum ExitCode error)
{
#ifdef XVNC_AuthPAM
  rfbClientPtr cl;

  for (cl = rfbClientHead; cl; cl = cl->next)
    rfbPAMEnd(cl);
#endif
  ShutdownTightThreads();
  free(rfbFB.pfbMemory);
  if (initOutputCalled) {
    char unixSocketName[32];
    sprintf(unixSocketName, "/tmp/.X11-unix/X%s", display);
    unlink(unixSocketName);
  }
  if (rfbUDSPath && rfbUDSCreated)
    unlink(rfbUDSPath);
}


void AbortDDX(enum ExitCode error)
{
  ddxGiveUp(error);
}


void DDXRingBell(int percent, int pitch, int duration)
{
  if (percent > 0)
    rfbSendBell();
}


void OsVendorInit(void)
{
  InitRFB();
  PrintVersion();
  rfbAuthInit();
  if (rfbAuthDisableX11TCP) {
    if (_XSERVTransNoListen("tcp"))
      FatalError("Failed to disable listen for tcp transport");
  }

  if (rfbMaxIdleTimeout > 0 &&
      (rfbIdleTimeout > rfbMaxIdleTimeout || rfbIdleTimeout == 0)) {
    rfbIdleTimeout = rfbMaxIdleTimeout;
    rfbLog("NOTICE: idle timeout set to %d seconds per system policy\n",
           rfbIdleTimeout);
  }
  if (rfbIdleTimeout > 0)
    IdleTimerSet();
  if (rfbFB.width > rfbMaxWidth || rfbFB.height > rfbMaxHeight) {
    rfbFB.width = min(rfbFB.width, rfbMaxWidth);
    rfbFB.height = min(rfbFB.height, rfbMaxHeight);
    rfbLog("NOTICE: desktop size clamped to %dx%d per system policy\n",
           rfbFB.width, rfbFB.height);
  }
  rfbClipScreens(&rfbScreens, rfbFB.width, rfbFB.height);
  if (xorg_list_is_empty(&rfbScreens))
    FatalError("All screens are outside of the framebuffer (%dx%d)",
               rfbFB.width, rfbFB.height);
#ifdef XVNC_AuthPAM
  if (rfbAuthDisablePAMSession && rfbAuthPAMSession) {
    rfbLog("NOTICE: PAM sessions disabled per system policy\n");
    rfbAuthPAMSession = FALSE;
  }
#endif
}


void OsVendorFatalError(const char *f, va_list args)
{
}


void ddxUseMsg(void)
{
  ErrorF("\nTurboVNC connection options\n");
  ErrorF("===========================\n");
  ErrorF("-alwaysshared          always treat new connections as shared\n");
  ErrorF("-capture file          capture the data sent to the first connected viewer to\n");
  ErrorF("                       the specified file\n");
  ErrorF("-deferupdate time      time in ms to defer updates [default: %d]\n",
         DEFAULT_DEFER_UPDATE_TIME);
  ErrorF("-desktop name          VNC desktop name [default: %s]\n",
         DEFAULT_DESKTOP_NAME);
  ErrorF("-disconnect            disconnect existing viewers when a new non-shared\n"
         "                       connection comes in, rather than refusing the new\n"
         "                       connection\n");
  ErrorF("-idletimeout S         exit if S seconds elapse with no VNC viewer connections\n");
  ErrorF("-inetd                 Xvnc is launched by inetd\n");
  ErrorF("-interface ipaddr      only bind to specified interface address\n");
  ErrorF("-ipv6                  enable IPv6 support\n");
  ErrorF("-localhost             only allow connections from localhost\n");
  ErrorF("-maxclipboard B        set max. clipboard transfer size to B bytes\n");
  ErrorF("                       [default: %d]\n", MAX_CUTTEXT_LEN);
  ErrorF("-maxconnections N      allow no more than N (1 <= N <= %d) simultaneous VNC\n",
         MAX_MAX_CONNECTIONS);
  ErrorF("                       viewer connections [default: %d]\n",
         DEFAULT_MAX_CONNECTIONS);
  ErrorF("-nevershared           never treat new connections as shared\n");
  ErrorF("-noclipboardrecv       disable client->server clipboard synchronization\n");
  ErrorF("-noclipboardsend       disable server->client clipboard synchronization\n");
  ErrorF("-noflowcontrol         when continuous updates are enabled, send updates\n");
  ErrorF("                       whether or not the viewer is ready to receive them\n");
  ErrorF("-noprimarysync         disable clipboard synchronization with the PRIMARY\n");
  ErrorF("                       selection (typically used when pasting with the middle\n");
  ErrorF("                       mouse button)\n");
  ErrorF("-noreverse             disable reverse connections\n");
  ErrorF("-rfbport port          TCP port for RFB connections\n");
  ErrorF("-rfbunixpath path      path to Unix domain socket for RFB connections\n");
  ErrorF("-rfbunixmode mode      Unix domain socket permissions\n");
  ErrorF("-rfbwait time          max time in ms to wait for a send/receive operation\n");
  ErrorF("                       to/from a connected viewer to complete [default: %d]\n",
         DEFAULT_MAX_CLIENT_WAIT);

  ErrorF("\nTurboVNC input options\n");
  ErrorF("======================\n");
  ErrorF("-compatiblekbd         set META key = ALT key as in the original VNC\n");
  ErrorF("-nocursor              don't display a cursor\n");
  ErrorF("-viewonly              only let viewers view, not control, the remote desktop\n");
  ErrorF("-virtualtablet         set up virtual stylus and eraser devices for this\n");
  ErrorF("                       session, to emulate a Wacom tablet, and map all\n");
  ErrorF("                       extended input events from all viewers to these devices\n");
  ErrorF("                       (see man page)\n");
  ErrorF("-xkblayout layout      set XKEYBOARD layout [default: %s]\n",
         XKB_DFLT_LAYOUT);
  ErrorF("-xkbmodel model        set XKEYBOARD model [default: %s]\n",
         XKB_DFLT_MODEL);
  ErrorF("-xkboptions options    set XKEYBOARD options [default: %s]\n",
         XKB_DFLT_OPTIONS);
  ErrorF("-xkbrules rules        set XKEYBOARD rules [default: %s]\n",
         XKB_DFLT_RULES);
  ErrorF("-xkbvariant variant    set XKEYBOARD variant [default: %s]\n",
         XKB_DFLT_VARIANT);

  ErrorF("\nTurboVNC display options\n");
  ErrorF("========================\n");
  ErrorF("-depth D               set framebuffer depth\n");
#ifndef TURBOVNC_STATIC_XORG_PATHS
  ErrorF("-dridir dir            specify directory containing the swrast Mesa driver\n");
#endif
  ErrorF("-geometry WxH          set framebuffer width & height (single-screen)\n");
  ErrorF("-geometry W0xH0+X0+Y0[,W1xH1+X1+Y1,...,WnxHn+Xn+Yn]\n");
  ErrorF("                       set multi-screen geometry (see man page)\n");
#ifdef NVCONTROL
  ErrorF("-nvcontrol display     set up a virtual NV-CONTROL extension and redirect\n");
  ErrorF("                       NV-CONTROL requests to the specified X display\n");
#endif
  ErrorF("-pixelformat format    set pixel format (BGRnnn or RGBnnn)\n");

  ErrorF("\nTurboVNC encoding options\n");
  ErrorF("=========================\n");
  ErrorF("-alr S                 enable automatic lossless refresh and set timer to S\n");
  ErrorF("                       seconds (S is floating point)\n");
  ErrorF("-alrqual Q             send automatic lossless refresh as a JPEG image with\n");
  ErrorF("                       quality Q, rather than as a mathematically lossless\n");
  ErrorF("                       image\n");
  ErrorF("-alrsamp S             specify chroma subsampling factor for automatic lossless\n");
  ErrorF("                       refresh JPEG images (S = 1x, 2x, 4x, or gray)\n");
  ErrorF("-economictranslate     use less memory-hungry pixel format translation if\n");
  ErrorF("                       depth=16\n");
  ErrorF("-interframe            always use interframe comparison\n");
  ErrorF("-nointerframe          never use interframe comparison\n");
#if !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined(__OpenBSD__) && !defined(__DragonFly__)
  ErrorF("-nomt                  disable multithreaded Tight encoding\n");
  ErrorF("-nthreads N            specify number of threads (1 <= N <= %d) to use with\n",
         MAX_ENCODING_THREADS);
  ErrorF("                       multithreaded Tight encoding [default: 1 per CPU core,\n");
  ErrorF("                       max. 4]\n");
#endif

  ErrorF("\nTurboVNC security and authentication options\n");
  ErrorF("============================================\n");
  ErrorF("-maxauthfails N        allow N consecutive VNC/OTP auth failures (0 = no limit)\n");
  ErrorF("                       from a client's IP address before connections from that\n");
  ErrorF("                       address are temporarily blocked [default: %d]\n",
         DEFAULT_AUTH_MAX_FAILS);
  ErrorF("-authfailtimeout S     block connections initially for S seconds (doubling with\n");
  ErrorF("                       each subsequent consecutive VNC/OTP auth failure) from\n");
  ErrorF("                       the IP address of a client that exceeds the max number\n");
  ErrorF("                       of consecutive VNC/OTP auth failures [default: %d]\n",
         DEFAULT_AUTH_FAIL_TIMEOUT);
#ifdef XVNC_AuthPAM
  ErrorF("-pamsession            create a new PAM session for each viewer that\n");
  ErrorF("                       authenticates using the username/password of the user\n");
  ErrorF("                       who owns the TurboVNC session, and leave the PAM session\n");
  ErrorF("                       open until the viewer disconnects\n");
#endif
  ErrorF("-rfbauth passwd-file   specify password file for VNC Password authentication\n");
  ErrorF("-securitytypes types   list of security types that the server should support\n");
  rfbAuthListAvailableSecurityTypes();
#if USETLS
  ErrorF("-x509cert file         specify filename of X.509 signed certificate\n");
  ErrorF("-x509key file          specify filename of X.509 private key\n");
#endif

  ErrorF("\nTurboVNC miscellaneous options\n");
  ErrorF("==============================\n");
#ifndef TURBOVNC_STATIC_XORG_PATHS
  ErrorF("-registrydir dir       specify directory containing protocol.txt\n");
#endif
  ErrorF("-verbose               print all X.org errors, warnings, and messages\n");
  ErrorF("-version               report Xvnc version on stderr\n\n");
}


/*
 * rfbLog prints a time-stamped message to the log file (stderr.)
 */

void rfbLog(char *format, ...)
{
  va_list args;
  char buf[256];
  time_t clock;
  int i;

  va_start(args, format);

  time(&clock);
  strftime(buf, 255, "%d/%m/%Y %H:%M:%S ", localtime(&clock));
  for (i = 0; i < traceLevel; i++)
    snprintf(&buf[strlen(buf)], 256 - strlen(buf), "  ");
  fputs(buf, stderr);

  vfprintf(stderr, format, args);
  fflush(stderr);

  va_end(args);
}


void rfbLogPerror(char *str)
{
  rfbLog("");
  perror(str);
}


/*
 * These functions wrap malloc() and realloc() and immediately abort the server
 * if memory allocation fails.
 */

void *rfbAlloc(size_t size)
{
  void *mem = malloc(size);

  if (!mem)
    FatalError("Memory allocation failure");
  return mem;
}


void *rfbAlloc0(size_t size)
{
  void *mem = rfbAlloc(size);

  memset(mem, 0, size);
  return mem;
}


void *rfbRealloc(void *ptr, size_t size)
{
  void *mem = realloc(ptr, size);

  if (!mem)
    FatalError("Memory allocation failure");
  return mem;
}
