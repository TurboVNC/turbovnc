/*
 * init.c
 *
 * Modified for XFree86 4.x by Alan Hourihane <alanh@fairlite.demon.co.uk>
 */

/*
 *  Copyright (C) 2009-2013 D. R. Commander.  All Rights Reserved.
 *  Copyright (C) 2010 University Corporation for Atmospheric Research.
 *                     All Rights Reserved.
 *  Copyright (C) 2005 Sun Microsystems, Inc.  All Rights Reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "X11/X.h"
#define NEED_EVENTS
#include "X11/Xproto.h"
#include "X11/Xos.h"
#include "scrnintstr.h"
#include "servermd.h"
#include "fb.h"
#include "mibstore.h"
#include "colormapst.h"
#include "gcstruct.h"
#include "input.h"
#include "mipointer.h"
#include "dixstruct.h"
#include "propertyst.h"
#include <Xatom.h>
#include <errno.h>
#include <sys/param.h>
#include "dix.h"
#include "micmap.h"
#include "globals.h"
#include "rfb.h"
#include <time.h>
#include "tvnc_version.h"

#define RFB_DEFAULT_WIDTH  1024
#define RFB_DEFAULT_HEIGHT 768
#define RFB_DEFAULT_DEPTH  24
#define RFB_DEFAULT_WHITEPIXEL 0
#define RFB_DEFAULT_BLACKPIXEL 1

rfbScreenInfo rfbScreen;
int rfbGCIndex;

static Bool initOutputCalled = FALSE;
static Bool noCursor = FALSE;
char *desktopName = "x11";

char rfbThisHost[256];

Atom VNC_LAST_CLIENT_ID;
Atom VNC_CONNECT;

Atom VNC_OTP;

#ifdef XVNC_AuthPAM
#define MAXUSERLEN 63

Atom VNC_ACL;
#endif

static HWEventQueueType alwaysCheckForInput[2] = { 0, 1 };
static HWEventQueueType *mieqCheckForInput[2];

static char primaryOrder[4] = "";
static int redBits, greenBits, blueBits;

static Bool rfbScreenInit(int index, ScreenPtr pScreen, int argc,
                          char **argv);
static int rfbKeybdProc(DeviceIntPtr pDevice, int onoff);
static int rfbMouseProc(DeviceIntPtr pDevice, int onoff);
static Bool CheckDisplayNumber(int n);

static Bool rfbAlwaysTrue();
static char *rfbAllocateFramebufferMemory(rfbScreenInfoPtr prfb);
static Bool rfbCursorOffScreen(ScreenPtr *ppScreen, int *x, int *y);
static void rfbCrossScreen(ScreenPtr pScreen, Bool entering);
static void rfbClientStateChange(CallbackListPtr *, pointer myData,
                                 pointer client);

static miPointerScreenFuncRec rfbPointerCursorFuncs = {
    rfbCursorOffScreen,
    rfbCrossScreen,
    miPointerWarpCursor
};


int inetdSock = -1;
static char inetdDisplayNumStr[10];

/* Interface address to bind to. */
struct in_addr interface;
struct in6_addr interface6;
int family = -1;


static void
PrintVersion(void)
{
    fprintf(stderr, "TurboVNC Server (Xvnc) %d-bit v"__VERSION" (build "__BUILD")\n",
        (int)sizeof(size_t)*8);
    fprintf(stderr, "Copyright (C) "__COPYRIGHT_YEAR" "__COPYRIGHT"\n");
    fprintf(stderr, __URLTEXT"\n\n");
}


/*
 * ddxProcessArgument is our first entry point and will be called at the
 * very start for each argument.  It is not called again on server reset.
 */

int
ddxProcessArgument (argc, argv, i)
    int argc;
    char *argv[];
    int i;
{
    static Bool firstTime = TRUE;

    if (firstTime)
    {
        rfbScreen.width  = RFB_DEFAULT_WIDTH;
        rfbScreen.height = RFB_DEFAULT_HEIGHT;
        rfbScreen.depth  = RFB_DEFAULT_DEPTH;
        rfbScreen.blackPixel = RFB_DEFAULT_BLACKPIXEL;
        rfbScreen.whitePixel = RFB_DEFAULT_WHITEPIXEL;
        rfbScreen.pfbMemory = NULL;
        gethostname(rfbThisHost, 255);
        interface.s_addr = htonl (INADDR_ANY);
        interface6 = in6addr_any;
        firstTime = FALSE;
    }

    if (strcasecmp (argv[i], "-geometry") == 0)     /* -geometry WxH */
    {
        if (i + 1 >= argc) UseMsg();
        if (sscanf(argv[i + 1], "%dx%d",
                   &rfbScreen.width, &rfbScreen.height) != 2) {
            ErrorF("Invalid geometry %s\n", argv[i + 1]);
            UseMsg();
        }
        return 2;
    }

    if (strcasecmp (argv[i], "-depth") == 0)        /* -depth D */
    {
        if (i + 1 >= argc) UseMsg();
        rfbScreen.depth = atoi(argv[i + 1]);
        return 2;
    }

    if (strcasecmp (argv[i], "-pixelformat") == 0) {
        if (i + 1 >= argc) UseMsg();
        if (sscanf(argv[i + 1], "%3s%1d%1d%1d", primaryOrder,
                   &redBits, &greenBits, &blueBits) < 4) {
            ErrorF("Invalid pixel format %s\n", argv[i + 1]);
            UseMsg();
        }

        if (strcasecmp(primaryOrder, "bgr") == 0) {
            int tmp = redBits;
            redBits = blueBits;
            blueBits = tmp;
        } else if (strcasecmp(primaryOrder, "rgb") != 0) {
            ErrorF("Invalid pixel format %s\n", argv[i + 1]);
            UseMsg();
        }

        return 2;
    }

    if (strcasecmp (argv[i], "-blackpixel") == 0) { /* -blackpixel n */
        if (i + 1 >= argc) UseMsg();
        rfbScreen.blackPixel = atoi(argv[i + 1]);
        return 2;
    }

    if (strcasecmp (argv[i], "-whitepixel") == 0) { /* -whitepixel n */
        if (i + 1 >= argc) UseMsg();
        rfbScreen.whitePixel = atoi(argv[i + 1]);
        return 2;
    }

    if (strcasecmp(argv[i], "-udpinputport") == 0) { /* -udpinputport port */
        if (i + 1 >= argc) UseMsg();
        udpPort = atoi(argv[i + 1]);
        return 2;
    }

    if (strcasecmp(argv[i], "-rfbport") == 0) {     /* -rfbport port */
        if (i + 1 >= argc) UseMsg();
        rfbPort = atoi(argv[i + 1]);
        return 2;
    }

    if (strcasecmp(argv[i], "-rfbwait") == 0) {     /* -rfbwait ms */
        if (i + 1 >= argc) UseMsg();
        rfbMaxClientWait = atoi(argv[i + 1]);
        return 2;
    }

    if (strcasecmp(argv[i], "-nocursor") == 0) {
        noCursor = TRUE;
        return 1;
    }

    if (strcasecmp(argv[i], "-rfbauth") == 0) {     /* -rfbauth passwd-file */
        if (i + 1 >= argc) UseMsg();
        rfbOptRfbauth = TRUE;
        rfbAuthPasswdFile = argv[i + 1];
        return 2;
    }

    if (strcasecmp(argv[i], "-otpauth") == 0) {
        rfbOptOtpauth = TRUE;
        return 1;
    }

    if (strcasecmp(argv[i], "-pamauth") == 0) {
        rfbOptPamauth = TRUE;
        return 1;
    }

    if (strcasecmp(argv[i], "-noreverse") == 0) {
        rfbAuthDisableRevCon = TRUE;
        return 1;
    }

    if (strcasecmp(argv[i], "-noclipboardsend") == 0) {
        rfbAuthDisableCBSend = TRUE;
        return 1;
    }

    if (strcasecmp(argv[i], "-noclipboardrecv") == 0) {
        rfbAuthDisableCBRecv = TRUE;
        return 1;
    }

    if (strcasecmp(argv[i], "-nocutbuffersync") == 0) {
        rfbSyncCutBuffer = FALSE;
        return 1;
    }

    if (strcasecmp(argv[i], "-idletimeout") == 0) { /* -idletimeout sec */
        if (i + 1 >= argc) UseMsg();
        rfbIdleTimeout = atoi(argv[i + 1]);
        return 2;
    }

    if (strcasecmp(argv[i], "-httpd") == 0) {
        if (i + 1 >= argc) UseMsg();
        httpDir = argv[i + 1];
        return 2;
    }

    if (strcasecmp(argv[i], "-httpport") == 0) {
        if (i + 1 >= argc) UseMsg();
        httpPort = atoi(argv[i + 1]);
        return 2;
    }

    if (strcasecmp(argv[i], "-deferupdate") == 0) { /* -deferupdate ms */
        if (i + 1 >= argc) UseMsg();
        rfbDeferUpdateTime = atoi(argv[i + 1]);
        return 2;
    }

    if (strcasecmp(argv[i], "-noflowcontrol") == 0) {
        rfbCongestionControl = FALSE;
        return 1;
    }

    if (strcasecmp(argv[i], "-alr") == 0) {
        if (i + 1 >= argc) UseMsg();
        rfbAutoLosslessRefresh = atof(argv[i + 1]);
        if (rfbAutoLosslessRefresh <= 0.0) UseMsg();
        return 2;
    }

    if (strcasecmp(argv[i], "-alrqual") == 0) {
        if (i + 1 >= argc) UseMsg();
        rfbALRQualityLevel = atoi(argv[i + 1]);
        if (rfbALRQualityLevel < 1 || rfbALRQualityLevel > 100) UseMsg();
        return 2;
    }

    if (strcasecmp(argv[i], "-alrsamp") == 0) {
        if (i + 1 >= argc) UseMsg();
        switch(toupper(argv[i + 1][0])) {
            case 'G': case '0':  rfbALRSubsampLevel = TVNC_GRAY;  break;
            case '1':  rfbALRSubsampLevel = TVNC_1X;  break;
            case '2':  rfbALRSubsampLevel = TVNC_2X;  break;
            case '4':  rfbALRSubsampLevel = TVNC_4X;  break;
            default:  UseMsg();
        }
        return 2;
    }

    if (strcasecmp(argv[i], "-interframe") == 0) {
        rfbInterframe = 1;
        return 1;
    }

    if (strcasecmp(argv[i], "-nointerframe") == 0) {
        rfbInterframe = 0;
        return 1;
    }

    if (strcasecmp(argv[i], "-economictranslate") == 0) {
        rfbEconomicTranslate = TRUE;
        return 1;
    }

    if (strcasecmp(argv[i], "-desktop") == 0) {     /* -desktop desktop-name */
        if (i + 1 >= argc) UseMsg();
        desktopName = argv[i + 1];
        return 2;
    }

    if (strcasecmp(argv[i], "-alwaysshared") == 0) {
        rfbAlwaysShared = TRUE;
        return 1;
    }

    if (strcasecmp(argv[i], "-nevershared") == 0) {
        rfbNeverShared = TRUE;
        return 1;
    }

    if (strcasecmp(argv[i], "-dontdisconnect") == 0) {
        rfbDontDisconnect = TRUE;
        return 1;
    }

    /* Run server in view-only mode - Ehud Karni SW */
    if (strcasecmp(argv[i], "-viewonly") == 0) {
        rfbViewOnly = TRUE;
        return 1;
    }

    if (strcasecmp(argv[i], "-localhost") == 0) {
        interface.s_addr = htonl (INADDR_LOOPBACK);
        interface6 = in6addr_loopback;
        return 1;
    }

    if (strcasecmp(argv[i], "-ipv6") == 0) {
        if (family == -1) family = AF_INET6;
        return 1;
    }

    if (strcasecmp(argv[i], "-interface") == 0) {   /* -interface ipaddr */
        struct addrinfo hints, *addr;
        if (i + 1 >= argc) {
            UseMsg();
            return 2;
        }
        if (interface.s_addr != htonl (INADDR_ANY) ||
            memcmp(&interface6, &in6addr_any, sizeof(interface6))) {
            /* Already set (-localhost?). */
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

    if (strcasecmp(argv[i], "-inetd") == 0) {       /* -inetd */ 
        int n;
        for (n = 1; n < 100; n++) {
            if (CheckDisplayNumber(n))
                break;
        }

        if (n >= 100)
            FatalError("-inetd: couldn't find free display number");

        sprintf(inetdDisplayNumStr, "%d", n);
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

    if (strcasecmp(argv[i], "-compatiblekbd") == 0) {
        compatibleKbd = TRUE;
        return 1;
    }

    if (strcasecmp(argv[i], "-version") == 0) {
        PrintVersion();
        exit(0);
    }

    if (inetdSock != -1 && argv[i][0] == ':') {
        FatalError("can't specify both -inetd and :displaynumber");
    }

    return 0;
}


/*
 * InitOutput is called every time the server resets.  It should call
 * AddScreen for each screen (but we only ever have one), and in turn this
 * will call rfbScreenInit.
 */

/* Common pixmap formats */

static PixmapFormatRec formats[MAXFORMATS] = {
        { 1,    1,      BITMAP_SCANLINE_PAD },
        { 4,    8,      BITMAP_SCANLINE_PAD },
        { 8,    8,      BITMAP_SCANLINE_PAD },
        { 15,   16,     BITMAP_SCANLINE_PAD },
        { 16,   16,     BITMAP_SCANLINE_PAD },
        { 24,   32,     BITMAP_SCANLINE_PAD },
#ifdef RENDER
        { 32,   32,     BITMAP_SCANLINE_PAD },
#endif
};
#ifdef RENDER
static int numFormats = 7;
#else
static int numFormats = 6;
#endif

void
InitOutput(screenInfo, argc, argv)
    ScreenInfo *screenInfo;
    int argc;
    char **argv;
{
    int i;
    initOutputCalled = TRUE;

    rfbLog("Desktop name '%s' (%s:%s)\n", desktopName, rfbThisHost, display);
    rfbLog("Protocol versions supported: 3.3, 3.7, 3.8, 3.7t, 3.8t\n");

    VNC_LAST_CLIENT_ID = MakeAtom("VNC_LAST_CLIENT_ID",
                                  strlen("VNC_LAST_CLIENT_ID"), TRUE);
    VNC_CONNECT = MakeAtom("VNC_CONNECT", strlen("VNC_CONNECT"), TRUE);

    if (rfbOptOtpauth)
            VNC_OTP = MakeAtom("VNC_OTP", strlen("VNC_OTP"), TRUE);

#ifdef XVNC_AuthPAM
    VNC_ACL = MakeAtom("VNC_ACL", strlen("VNC_ACL"), TRUE);
#endif

    rfbInitSockets();
    if (inetdSock == -1)
        httpInitSockets();
   
    /* initialize pixmap formats */

    screenInfo->imageByteOrder = IMAGE_BYTE_ORDER;
    screenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
    screenInfo->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
    screenInfo->bitmapBitOrder = BITMAP_BIT_ORDER;
    screenInfo->numPixmapFormats = numFormats;
    for (i = 0; i < numFormats; i++)
        screenInfo->formats[i] = formats[i];

    rfbGCIndex = AllocateGCPrivateIndex();
    if (rfbGCIndex < 0) {
        FatalError("InitOutput: AllocateGCPrivateIndex failed\n");
    }

    if (!AddCallback(&ClientStateCallback, rfbClientStateChange, NULL)) {
        rfbLog("InitOutput: AddCallback failed\n");
        return;
    }

    /* initialize screen */

    if (AddScreen(rfbScreenInit, argc, argv) == -1) {
        FatalError("Couldn't add screen");
    }
}


static Bool
rfbScreenInit(index, pScreen, argc, argv)
    int index;
    ScreenPtr pScreen;
    int argc;
    char ** argv;
{
    rfbScreenInfoPtr prfb = &rfbScreen;
    int dpix = 75, dpiy = 75;
    int ret;
    char *pbits;
    VisualPtr vis;
    extern int monitorResolution;
#ifdef RENDER
    PictureScreenPtr    ps;
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

    miClearVisualTypes();

    if (defaultColorVisualClass == -1)
        defaultColorVisualClass = TrueColor;

    if (!miSetVisualTypes(prfb->depth, miGetDefaultVisualMask(prfb->depth), 8,
                                                defaultColorVisualClass) )
        return FALSE;

    miSetPixmapDepths();

    switch (prfb->bitsPerPixel)
    {
    case 8:
        ret = fbScreenInit(pScreen, pbits, prfb->width, prfb->height,
                            dpix, dpiy, prfb->paddedWidthInBytes, 8);
        break;
    case 16:
        ret = fbScreenInit(pScreen, pbits, prfb->width, prfb->height,
                              dpix, dpiy, prfb->paddedWidthInBytes / 2, 16);
        if (prfb->depth == 15) {
            blueBits = 5; greenBits = 5; redBits = 5;
        } else {
            blueBits = 5; greenBits = 6; redBits = 5;
        }
        break;
    case 32:
        ret = fbScreenInit(pScreen, pbits, prfb->width, prfb->height,
                              dpix, dpiy, prfb->paddedWidthInBytes / 4, 32);
        blueBits = 8; greenBits = 8; redBits = 8;
        break;
    default:
        return FALSE;
    }

    if (!ret) return FALSE;

    fbInitializeBackingStore(pScreen);

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

    if (!AllocateGCPrivate(pScreen, rfbGCIndex, sizeof(rfbGCRec))) {
        FatalError("rfbScreenInit: AllocateGCPrivate failed\n");
    }

    prfb->cursorIsDrawn = FALSE;
    prfb->dontSendFramebufferUpdate = FALSE;

    prfb->CloseScreen = pScreen->CloseScreen;
    prfb->CreateGC = pScreen->CreateGC;
    prfb->PaintWindowBackground = pScreen->PaintWindowBackground;
    prfb->PaintWindowBorder = pScreen->PaintWindowBorder;
    prfb->CopyWindow = pScreen->CopyWindow;
    prfb->ClearToBackground = pScreen->ClearToBackground;
    prfb->RestoreAreas = pScreen->RestoreAreas;
#ifdef RENDER
    ps = GetPictureScreenIfSet(pScreen);
    if (ps)
        prfb->Composite = ps->Composite;
#endif
    pScreen->CloseScreen = rfbCloseScreen;
    pScreen->CreateGC = rfbCreateGC;
    pScreen->PaintWindowBackground = rfbPaintWindowBackground;
    pScreen->PaintWindowBorder = rfbPaintWindowBorder;
    pScreen->CopyWindow = rfbCopyWindow;
    pScreen->ClearToBackground = rfbClearToBackground;
    pScreen->RestoreAreas = rfbRestoreAreas;
#ifdef RENDER
    if (ps)
        ps->Composite = rfbComposite;
#endif

    pScreen->InstallColormap = rfbInstallColormap;
    pScreen->UninstallColormap = rfbUninstallColormap;
    pScreen->ListInstalledColormaps = rfbListInstalledColormaps;
    pScreen->StoreColors = rfbStoreColors;

    pScreen->SaveScreen = rfbAlwaysTrue;

    rfbDCInitialize(pScreen, &rfbPointerCursorFuncs);

    if (noCursor) {
        pScreen->DisplayCursor = rfbAlwaysTrue;
        prfb->cursorIsDrawn = TRUE;
    }

    pScreen->blackPixel = prfb->blackPixel;
    pScreen->whitePixel = prfb->whitePixel;

    rfbServerFormat.bitsPerPixel = prfb->bitsPerPixel;
    rfbServerFormat.depth = prfb->depth;
    rfbServerFormat.bigEndian = bigEndian;

    /* Find the root visual and set the server format */
    for (vis = pScreen->visuals; vis->vid != pScreen->rootVisual; vis++)
        ;
    rfbServerFormat.trueColour = (vis->class == TrueColor);

    if ( (vis->class == TrueColor) || (vis->class == DirectColor) ) {
        rfbServerFormat.redMax = vis->redMask >> vis->offsetRed;
        rfbServerFormat.greenMax = vis->greenMask >> vis->offsetGreen;
        rfbServerFormat.blueMax = vis->blueMask >> vis->offsetBlue;
        rfbServerFormat.redShift = vis->offsetRed;
        rfbServerFormat.greenShift = vis->offsetGreen;
        rfbServerFormat.blueShift = vis->offsetBlue;
    } else {
        rfbServerFormat.redMax
            = rfbServerFormat.greenMax 
            = rfbServerFormat.blueMax = 0;
        rfbServerFormat.redShift
            = rfbServerFormat.greenShift 
            = rfbServerFormat.blueShift = 0;
    }

    ret = fbCreateDefColormap(pScreen);

#ifdef RANDR
    miRandRInit(pScreen);
#endif

    return ret;

} /* end rfbScreenInit */



/*
 * InitInput is also called every time the server resets.  It is called after
 * InitOutput so we can assume that rfbInitSockets has already been called.
 */

void
InitInput(argc, argv)
    int argc;
    char *argv[];
{
    DeviceIntPtr p, k;
    k = AddInputDevice(rfbKeybdProc, TRUE);
    p = AddInputDevice(rfbMouseProc, TRUE);
    RegisterKeyboardDevice(k);
    RegisterPointerDevice(p);
    miRegisterPointerDevice(screenInfo.screens[0], p);
    (void)mieqInit ((DevicePtr)k, (DevicePtr)p);
    mieqCheckForInput[0] = checkForInput[0];
    mieqCheckForInput[1] = checkForInput[1];
    SetInputCheck(&alwaysCheckForInput[0], &alwaysCheckForInput[1]);
}


static int
rfbKeybdProc(pDevice, onoff)
    DeviceIntPtr pDevice;
    int onoff;
{
    KeySymsRec          keySyms;
    CARD8               modMap[MAP_LENGTH];
    DevicePtr pDev = (DevicePtr)pDevice;

    switch (onoff)
    {
    case DEVICE_INIT: 
        KbdDeviceInit(pDevice, &keySyms, modMap);
        InitKeyboardDeviceStruct(pDev, &keySyms, modMap,
                                 (BellProcPtr)rfbSendBell,
                                 (KbdCtrlProcPtr)NoopDDA);
            break;
    case DEVICE_ON: 
        pDev->on = TRUE;
        KbdDeviceOn();
        break;
    case DEVICE_OFF: 
        pDev->on = FALSE;
        KbdDeviceOff();
        break;
    case DEVICE_CLOSE:
        if (pDev->on)
            KbdDeviceOff();
        break;
    }
    return Success;
}

static int
rfbMouseProc(pDevice, onoff)
    DeviceIntPtr pDevice;
    int onoff;
{
    BYTE map[6];
    DevicePtr pDev = (DevicePtr)pDevice;

    switch (onoff)
    {
    case DEVICE_INIT:
        PtrDeviceInit();
        map[1] = 1;
        map[2] = 2;
        map[3] = 3;
        map[4] = 4;
        map[5] = 5;
        InitPointerDeviceStruct(pDev, map, 5, miPointerGetMotionEvents,
                                PtrDeviceControl,
                                miPointerGetMotionBufferSize());
        break;

    case DEVICE_ON:
        pDev->on = TRUE;
        PtrDeviceOn(pDevice);
        break;

    case DEVICE_OFF:
        pDev->on = FALSE;
        PtrDeviceOff();
        break;

    case DEVICE_CLOSE:
        if (pDev->on)
            PtrDeviceOff();
        break;
    }
    return Success;
}


Bool
LegalModifier(key, pDev)
    unsigned int key;
    DevicePtr   pDev;
{
    return TRUE;
}


void
ProcessInputEvents()
{
    rfbCheckFds();
    httpCheckFds();
    if (*mieqCheckForInput[0] != *mieqCheckForInput[1]) {
        mieqProcessInputEvents();
        miPointerUpdate();
    }
}


static Bool CheckDisplayNumber(int n)
{
    char fname[32];
    int sock;
    struct sockaddr_storage addr;
    struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)&addr;
    struct sockaddr_in *addr4 = (struct sockaddr_in *)&addr;

    sock = socket(AF_INET6, SOCK_STREAM, 0);

    memset(&addr, 0, sizeof(addr));
    addr6->sin6_family = AF_INET6;
    addr6->sin6_addr = in6addr_any;
    addr6->sin6_port = htons(6000 + n);
    if (bind(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in6)) < 0) {
        close(sock);
        return FALSE;
    }
    close(sock);

    sock = socket(AF_INET, SOCK_STREAM, 0);

    memset(&addr, 0, sizeof(addr));
    addr4->sin_family = AF_INET;
    addr4->sin_addr.s_addr = htonl(INADDR_ANY);
    addr4->sin_port = htons(6000 + n);
    if (bind(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0) {
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


void
rfbRootPropertyChange(PropertyPtr pProp)
{
    if ((pProp->propertyName == XA_CUT_BUFFER0) && (pProp->type == XA_STRING)
        && (pProp->format == 8) && rfbSyncCutBuffer)
    {
        char *str;
        str = (char *)xalloc(pProp->size + 1);
        if (!str)
            FatalError("rfbRootPropertyChange(): Memory allocation failure\n");
        strncpy(str, pProp->data, pProp->size);
        str[pProp->size] = 0;
        rfbGotXCutText(str, pProp->size);
        xfree(str);
        return;
    }

    if (
        !rfbAuthDisableRevCon &&
        (pProp->propertyName == VNC_CONNECT) && (pProp->type == XA_STRING)
            && (pProp->format == 8)
    ) {
        rfbClientPtr cl;
        char *colonPos;
        int port = 5500;
        char *host = (char *)Xalloc(pProp->size + 1);
        memcpy(host, pProp->data, pProp->size);
        host[pProp->size] = 0;
        colonPos = strrchr(host, ':');
        if (colonPos && colonPos < &host[strlen(host) - 1]) {
            port = atoi(colonPos + 1);
            *colonPos = 0;
        }

        cl = rfbReverseConnection(host, port);

        free(host);
    }

    if (
        rfbOptOtpauth &&
        (pProp->propertyName == VNC_OTP) && (pProp->type == XA_STRING) &&
        (pProp->format == 8)
    ) {
        if (pProp->size == 0) {
            if (rfbAuthOTPValue != NULL) {
                xfree(rfbAuthOTPValue);
                rfbAuthOTPValue = NULL;
                rfbLog("One time password(s) disabled\n");
            }

        } else if ((pProp->size == MAXPWLEN) || (pProp->size == (MAXPWLEN * 2))) {
            if (rfbAuthOTPValue != NULL)
                xfree(rfbAuthOTPValue);

            rfbAuthOTPValueLen = pProp->size;
            rfbAuthOTPValue = (char *) xalloc(pProp->size);
            memcpy(rfbAuthOTPValue, pProp->data, pProp->size);
        }

        memset(pProp->data, 0, pProp->size);
        /* delete the property? how? */
    }

#ifdef XVNC_AuthPAM
    if ((pProp->propertyName == VNC_ACL) && (pProp->type == XA_STRING) &&
        (pProp->format == 8) && (pProp->size > 1) && (pProp->size <= (MAXUSERLEN + 1))
    ) {
        /*
         * The first byte is a flag that selects revoke/add.
         * The remaining bytes are the name.
         */
        char*           p = (char *) xalloc(pProp->size);
        const char*     n = (const char*) pProp->data;

        memcpy(p, &n[1], pProp->size - 1);
        p[pProp->size - 1] = '\0';
        if ((n[0] & 1) == 0) {
            rfbAuthRevokeUser(p);
            xfree(p);

        } else {
            rfbAuthAddUser(p, (n[0] & 0x10) ? TRUE : FALSE);
        }

        memset(pProp->data, 0, pProp->size);
        /* delete the property? how? */
    }
#endif
}


int
rfbBitsPerPixel(depth)
    int depth;
{
    if (depth == 1) return 1;
    else if (depth <= 8) return 8;
    else if (depth <= 16) return 16;
    else return 32;
}


static Bool
rfbAlwaysTrue()
{
    return TRUE;
}


static char *
rfbAllocateFramebufferMemory(prfb)
    rfbScreenInfoPtr prfb;
{
    if (prfb->pfbMemory) return prfb->pfbMemory; /* already done */

    prfb->sizeInBytes = (prfb->paddedWidthInBytes * prfb->height);

    prfb->pfbMemory = (char *)Xalloc(prfb->sizeInBytes);

    return prfb->pfbMemory;
}


static Bool
rfbCursorOffScreen (ppScreen, x, y)
    ScreenPtr   *ppScreen;
    int         *x, *y;
{
    return FALSE;
}

static void
rfbCrossScreen (pScreen, entering)
    ScreenPtr   pScreen;
    Bool        entering;
{
}

static void
rfbClientStateChange(cbl, myData, clt)
    CallbackListPtr *cbl;
    pointer myData;
    pointer clt;
{
    dispatchException &= ~DE_RESET;     /* hack - force server not to reset */
}

void
ddxGiveUp()
{
    ShutdownTightThreads();
    Xfree(rfbScreen.pfbMemory);
    if (initOutputCalled) {
        char unixSocketName[32];
        sprintf(unixSocketName, "/tmp/.X11-unix/X%s", display);
        unlink(unixSocketName);
    }
}

void
AbortDDX()
{
    ddxGiveUp();
}

void
OsVendorInit()
{
    PrintVersion();
    rfbAuthInit();
    if (rfbMaxIdleTimeout > 0 && (rfbIdleTimeout > rfbMaxIdleTimeout
        || rfbIdleTimeout == 0)) {
        rfbIdleTimeout = rfbMaxIdleTimeout;
        rfbLog("NOTICE: idle timeout set to %d seconds per system policy\n",
            rfbIdleTimeout);
    }
    if (rfbIdleTimeout > 0) {
        idleTimer = TimerSet(idleTimer, 0, rfbIdleTimeout * 1000,
                             idleTimeoutCallback, NULL);
    }
}

void
OsVendorFatalError()
{
}

#ifdef DDXTIME /* from ServerOSDefines */
CARD64
GetTimeInMillis()
{
    struct timeval  tp;

    X_GETTIMEOFDAY(&tp);
    return(tp.tv_sec * 1000) + (tp.tv_usec / 1000);
}
#endif

void
ddxUseMsg()
{
    ErrorF("-geometry WxH          set framebuffer width & height\n");
    ErrorF("-depth D               set framebuffer depth\n");
    ErrorF("-pixelformat format    set pixel format (BGRnnn or RGBnnn)\n");
    ErrorF("-udpinputport port     UDP port for keyboard/pointer data\n");
    ErrorF("-rfbport port          TCP port for RFB protocol\n");
    ErrorF("-rfbwait time          max time in ms to wait for RFB client\n");
    ErrorF("-nocursor              don't display a cursor\n");
    ErrorF("-rfbauth passwd-file   enable VNC password authentication\n");
    ErrorF("-otpauth               enable one-time password (OTP) authentication\n");
    ErrorF("-pamauth               enable PAM user/password authentication\n");
    ErrorF("-noreverse             disable reverse connections\n");
    ErrorF("-noclipboardsend       disable server->client clipboard synchronization\n");
    ErrorF("-noclipboardrecv       disable client->server clipboard synchronization\n");
    ErrorF("-nocutbuffersync       disable clipboard synchronization for applications\n");
    ErrorF("                       that use the (obsolete) X cut buffer\n");
    ErrorF("-idletimeout S         exit if S seconds elapse with no VNC viewer connections\n");
    ErrorF("-httpd dir             serve files from the specified directory using HTTP\n");
    ErrorF("-httpport port         port for HTTP server\n");
    ErrorF("-deferupdate time      time in ms to defer updates (default 40)\n");
    ErrorF("-noflowcontrol         when continuous updates are enabled, send updates whether\n");
    ErrorF("                       or not the client is ready to receive them\n");
    ErrorF("-alr S                 enable automatic lossless refresh and set timer to S\n");
    ErrorF("                       seconds (S is floating point)\n");
    ErrorF("-interframe            always use interframe comparison\n");
    ErrorF("-nointerframe          never use interframe comparison\n");
    ErrorF("-economictranslate     less memory hungry translation\n");
    ErrorF("-desktop name          VNC desktop name (default x11)\n");
    ErrorF("-alwaysshared          always treat new clients as shared\n");
    ErrorF("-nevershared           never treat new clients as shared\n");
    ErrorF("-dontdisconnect        don't disconnect existing clients when a new non-shared\n"
           "                       connection comes in (refuse new connection instead)\n");
    ErrorF("-viewonly              only let clients view the remote desktop\n");
    ErrorF("-localhost             only allow connections from localhost\n");
    ErrorF("-interface ipaddr      only bind to specified interface address\n");
    ErrorF("-ipv6                  enable IPv6 support\n");
    ErrorF("-inetd                 Xvnc is launched by inetd\n");
    ErrorF("-compatiblekbd         set META key = ALT key as in the original VNC\n");
    ErrorF("-version               report Xvnc version on stderr\n");
    exit(1);
}

/*
 * rfbLog prints a time-stamped message to the log file (stderr).
 */

void rfbLog(char *format, ...)
{
    va_list args;
    char buf[256];
    time_t clock;

    va_start(args, format);

    time(&clock);
    strftime(buf, 255, "%d/%m/%Y %H:%M:%S ", localtime(&clock));
    fprintf(stderr, buf);

    vfprintf(stderr, format, args);
    fflush(stderr);

    va_end(args);
}

void rfbLogPerror(char *str)
{
    rfbLog("");
    perror(str);
}
