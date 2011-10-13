/* $XdotOrg: xc/programs/Xserver/dix/main.c,v 1.14 2005/07/03 08:53:38 daniels Exp $ */
/* $XFree86: xc/programs/Xserver/dix/main.c,v 3.43 2003/10/30 21:21:02 herrb Exp $ */
/***********************************************************

Copyright 1987, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/* $Xorg: main.c,v 1.4 2001/02/09 02:04:40 xorgcvs Exp $ */

/* The panoramix components contained the following notice */
/*****************************************************************

Copyright (c) 1991, 1997 Digital Equipment Corporation, Maynard, Massachusetts.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
DIGITAL EQUIPMENT CORPORATION BE LIABLE FOR ANY CLAIM, DAMAGES, INCLUDING,
BUT NOT LIMITED TO CONSEQUENTIAL OR INCIDENTAL DAMAGES, OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of Digital Equipment Corporation
shall not be used in advertising or otherwise to promote the sale, use or other
dealings in this Software without prior written authorization from Digital
Equipment Corporation.

******************************************************************/

/* $TOG: main.c /main/86 1998/02/09 14:20:03 kaleb $ */

#define NEED_EVENTS
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xos.h>   /* for unistd.h  */
#include <X11/Xproto.h>
#include "scrnintstr.h"
#include "misc.h"
#include "os.h"
#include "windowstr.h"
#include "resource.h"
#include "dixstruct.h"
#include "gcstruct.h"
#include "extension.h"
#include "colormap.h"
#include "colormapst.h"
#include "cursorstr.h"
#include <X11/fonts/font.h>
#include "opaque.h"
#include "servermd.h"
#include "site.h"
#include "dixfont.h"
#include "extnsionst.h"
#ifdef XPRINT
#include "DiPrint.h"
#endif
#ifdef PANORAMIX
#include "panoramiXsrv.h"
#else
#include "dixevents.h"		/* InitEvents() */
#include "dispatch.h"		/* InitProcVectors() */
#endif

#ifdef DPMSExtension
#define DPMS_SERVER
#include <X11/extensions/dpms.h>
#include "dpmsproc.h"
#endif

extern int InitClientPrivates(ClientPtr client);

extern void Dispatch(void);

char *ConnectionInfo;
xConnSetupPrefix connSetupPrefix;

extern FontPtr defaultFont;
extern int screenPrivateCount;

extern void InitProcVectors(void);
extern Bool CreateGCperDepthArray(void);

#ifndef PANORAMIX
static
#endif
Bool CreateConnectionBlock(void);

static void FreeScreen(ScreenPtr);

PaddingInfo PixmapWidthPaddingInfo[33];

int connBlockScreenStart;

static int restart = 0;

void
NotImplemented(xEvent *from, xEvent *to)
{
    FatalError("Not implemented");
}

/*
 * Dummy entry for ReplySwapVector[]
 */

void
ReplyNotSwappd(
	ClientPtr pClient ,
	int size ,
	void * pbuf
	)
{
    FatalError("Not implemented");
}

/*
 * This array encodes the answer to the question "what is the log base 2
 * of the number of pixels that fit in a scanline pad unit?"
 * Note that ~0 is an invalid entry (mostly for the benefit of the reader).
 */
static int answer[6][4] = {
	/* pad   pad   pad     pad*/
	/*  8     16    32    64 */

	{   3,     4,    5 ,   6 },	/* 1 bit per pixel */
	{   1,     2,    3 ,   4 },	/* 4 bits per pixel */
	{   0,     1,    2 ,   3 },	/* 8 bits per pixel */
	{   ~0,    0,    1 ,   2 },	/* 16 bits per pixel */
	{   ~0,    ~0,   0 ,   1 },	/* 24 bits per pixel */
	{   ~0,    ~0,   0 ,   1 }	/* 32 bits per pixel */
};

/*
 * This array gives the answer to the question "what is the first index for
 * the answer array above given the number of bits per pixel?"
 * Note that ~0 is an invalid entry (mostly for the benefit of the reader).
 */
static int indexForBitsPerPixel[ 33 ] = {
	~0, 0, ~0, ~0,	/* 1 bit per pixel */
	1, ~0, ~0, ~0,	/* 4 bits per pixel */
	2, ~0, ~0, ~0,	/* 8 bits per pixel */
	~0,~0, ~0, ~0,
	3, ~0, ~0, ~0,	/* 16 bits per pixel */
	~0,~0, ~0, ~0,
	4, ~0, ~0, ~0,	/* 24 bits per pixel */
	~0,~0, ~0, ~0,
	5		/* 32 bits per pixel */
};

/*
 * This array gives the bytesperPixel value for cases where the number
 * of bits per pixel is a multiple of 8 but not a power of 2.
 */
static int answerBytesPerPixel[ 33 ] = {
	~0, 0, ~0, ~0,	/* 1 bit per pixel */
	0, ~0, ~0, ~0,	/* 4 bits per pixel */
	0, ~0, ~0, ~0,	/* 8 bits per pixel */
	~0,~0, ~0, ~0,
	0, ~0, ~0, ~0,	/* 16 bits per pixel */
	~0,~0, ~0, ~0,
	3, ~0, ~0, ~0,	/* 24 bits per pixel */
	~0,~0, ~0, ~0,
	0		/* 32 bits per pixel */
};

/*
 * This array gives the answer to the question "what is the second index for
 * the answer array above given the number of bits per scanline pad unit?"
 * Note that ~0 is an invalid entry (mostly for the benefit of the reader).
 */
static int indexForScanlinePad[ 65 ] = {
	~0, ~0, ~0, ~0,
	~0, ~0, ~0, ~0,
	 0, ~0, ~0, ~0,	/* 8 bits per scanline pad unit */
	~0, ~0, ~0, ~0,
	 1, ~0, ~0, ~0,	/* 16 bits per scanline pad unit */
	~0, ~0, ~0, ~0,
	~0, ~0, ~0, ~0,
	~0, ~0, ~0, ~0,
	 2, ~0, ~0, ~0,	/* 32 bits per scanline pad unit */
	~0, ~0, ~0, ~0,
	~0, ~0, ~0, ~0,
	~0, ~0, ~0, ~0,
	~0, ~0, ~0, ~0,
	~0, ~0, ~0, ~0,
	~0, ~0, ~0, ~0,
	~0, ~0, ~0, ~0,
	 3		/* 64 bits per scanline pad unit */
};

#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

int
main(int argc, char *argv[], char *envp[])
{
    int		i, j, k, error;
    char	*xauthfile;
    HWEventQueueType	alwaysCheckForInput[2];

    display = "0";

    InitGlobals();
#ifdef XPRINT
    PrinterInitGlobals();
#endif

    /* Quartz support on Mac OS X requires that the Cocoa event loop be in
     * the main thread. This allows the X server main to be called again
     * from another thread. */
#if defined(__DARWIN__) && defined(DARWIN_WITH_QUARTZ)
    DarwinHandleGUI(argc, argv, envp);
#endif

    /* Notice if we're restarted.  Probably this is because we jumped through
     * an uninitialized pointer */
    if (restart)
	FatalError("server restarted. Jumped through uninitialized pointer?\n");
    else
	restart = 1;

    CheckUserParameters(argc, argv, envp);

    CheckUserAuthorization();

#ifdef COMMANDLINE_CHALLENGED_OPERATING_SYSTEMS
    ExpandCommandLine(&argc, &argv);
#endif

    InitConnectionLimits();

    /* These are needed by some routines which are called from interrupt
     * handlers, thus have no direct calling path back to main and thus
     * can't be passed argc, argv as parameters */
    argcGlobal = argc;
    argvGlobal = argv;
    /* prep X authority file from environment; this can be overriden by a
     * command line option */
    xauthfile = getenv("XAUTHORITY");
    if (xauthfile) InitAuthorization (xauthfile);
    ProcessCommandLine(argc, argv);

    alwaysCheckForInput[0] = 0;
    alwaysCheckForInput[1] = 1;
    while(1)
    {
	serverGeneration++;
	ScreenSaverTime = defaultScreenSaverTime;
	ScreenSaverInterval = defaultScreenSaverInterval;
	ScreenSaverBlanking = defaultScreenSaverBlanking;
	ScreenSaverAllowExposures = defaultScreenSaverAllowExposures;
#ifdef DPMSExtension
	DPMSStandbyTime = defaultDPMSStandbyTime;
	DPMSSuspendTime = defaultDPMSSuspendTime;
	DPMSOffTime = defaultDPMSOffTime;
	DPMSEnabled = defaultDPMSEnabled;
	DPMSPowerLevel = 0;
#endif
	InitBlockAndWakeupHandlers();
	/* Perform any operating system dependent initializations you'd like */
	OsInit();		
	if(serverGeneration == 1)
	{
	    CreateWellKnownSockets();
	    InitProcVectors();
	    clients = (ClientPtr *)xalloc(MAXCLIENTS * sizeof(ClientPtr));
	    if (!clients)
		FatalError("couldn't create client array");
	    for (i=1; i<MAXCLIENTS; i++) 
		clients[i] = NullClient;
	    serverClient = (ClientPtr)xalloc(sizeof(ClientRec));
	    if (!serverClient)
		FatalError("couldn't create server client");
	    InitClient(serverClient, 0, (pointer)NULL);
	}
	else
	    ResetWellKnownSockets ();
	clients[0] = serverClient;
	currentMaxClients = 1;

	if (!InitClientResources(serverClient))      /* for root resources */
	    FatalError("couldn't init server resources");

	SetInputCheck(&alwaysCheckForInput[0], &alwaysCheckForInput[1]);
	screenInfo.arraySize = MAXSCREENS;
	screenInfo.numScreens = 0;
	screenInfo.numVideoScreens = -1;
	WindowTable = (WindowPtr *)xalloc(MAXSCREENS * sizeof(WindowPtr));
	if (!WindowTable)
	    FatalError("couldn't create root window table");

	/*
	 * Just in case the ddx doesnt supply a format for depth 1 (like qvss).
	 */
	j = indexForBitsPerPixel[ 1 ];
	k = indexForScanlinePad[ BITMAP_SCANLINE_PAD ];
	PixmapWidthPaddingInfo[1].padRoundUp = BITMAP_SCANLINE_PAD-1;
	PixmapWidthPaddingInfo[1].padPixelsLog2 = answer[j][k];
 	j = indexForBitsPerPixel[8]; /* bits per byte */
 	PixmapWidthPaddingInfo[1].padBytesLog2 = answer[j][k];
	PixmapWidthPaddingInfo[1].bitsPerPixel = 1;

	InitAtoms();
	InitEvents();
	InitGlyphCaching();
	ResetClientPrivates();
	ResetScreenPrivates();
	ResetWindowPrivates();
	ResetGCPrivates();
#ifdef PIXPRIV
	ResetPixmapPrivates();
#endif
	ResetColormapPrivates();
	ResetFontPrivateIndex();
	ResetDevicePrivateIndex();
	InitCallbackManager();
	InitVisualWrap();
	InitOutput(&screenInfo, argc, argv);
#ifdef XPRINT
	PrinterInitOutput(&screenInfo, argc, argv);
#endif

	if (screenInfo.numScreens < 1)
	    FatalError("no screens found");
	if (screenInfo.numVideoScreens < 0)
	    screenInfo.numVideoScreens = screenInfo.numScreens;
	InitExtensions(argc, argv);
	if (!InitClientPrivates(serverClient))
	    FatalError("failed to allocate serverClient devprivates");
	for (i = 0; i < screenInfo.numScreens; i++)
	{
	    ScreenPtr pScreen = screenInfo.screens[i];
	    if (!CreateScratchPixmapsForScreen(i))
		FatalError("failed to create scratch pixmaps");
	    if (pScreen->CreateScreenResources &&
		!(*pScreen->CreateScreenResources)(pScreen))
		FatalError("failed to create screen resources");
	    if (!CreateGCperDepth(i))
		FatalError("failed to create scratch GCs");
	    if (!CreateDefaultStipple(i))
		FatalError("failed to create default stipple");
	    if (!CreateRootWindow(pScreen))
		FatalError("failed to create root window");
	}
	InitInput(argc, argv);
	if (InitAndStartDevices() != Success)
	    FatalError("failed to initialize core devices");

	InitFonts();
	if (loadableFonts) {
	    SetFontPath(0, 0, (unsigned char *)defaultFontPath, &error);
	} else {
	    if (SetDefaultFontPath(defaultFontPath) != Success)
		ErrorF("failed to set default font path '%s'",
			defaultFontPath);
	}
	if (!SetDefaultFont(defaultTextFont))
	    FatalError("could not open default font '%s'", defaultTextFont);
	if (!(rootCursor = CreateRootCursor(defaultCursorFont, 0)))
	    FatalError("could not open default cursor font '%s'",
		       defaultCursorFont);
#ifdef DPMSExtension
 	/* check all screens, looking for DPMS Capabilities */
 	DPMSCapableFlag = DPMSSupported();
	if (!DPMSCapableFlag)
     	    DPMSEnabled = FALSE;
#endif

#ifdef PANORAMIX
	/*
	 * Consolidate window and colourmap information for each screen
	 */
	if (!noPanoramiXExtension)
	    PanoramiXConsolidate();
#endif

	for (i = 0; i < screenInfo.numScreens; i++)
	    InitRootWindow(WindowTable[i]);
	DefineInitialRootWindow(WindowTable[0]);
	SaveScreens(SCREEN_SAVER_FORCER, ScreenSaverReset);
#ifdef DPMSExtension
	SetDPMSTimers();
#endif

#ifdef PANORAMIX
	if (!noPanoramiXExtension) {
	    if (!PanoramiXCreateConnectionBlock())
		FatalError("could not create connection block info");
	} else
#endif
	{
	    if (!CreateConnectionBlock())
	    	FatalError("could not create connection block info");
	}

	Dispatch();

	/* Now free up whatever must be freed */
	if (screenIsSaved == SCREEN_SAVER_ON)
	    SaveScreens(SCREEN_SAVER_OFF, ScreenSaverReset);
	FreeScreenSaverTimer();
	CloseDownExtensions();

#ifdef PANORAMIX
	{
	    Bool remember_it = noPanoramiXExtension;
	    noPanoramiXExtension = TRUE;
	    FreeAllResources();
	    noPanoramiXExtension = remember_it;
	}
#else
	FreeAllResources();
#endif

	CloseDownDevices();
	for (i = screenInfo.numScreens - 1; i >= 0; i--)
	{
	    FreeScratchPixmapsForScreen(i);
	    FreeGCperDepth(i);
	    FreeDefaultStipple(i);
	    (* screenInfo.screens[i]->CloseScreen)(i, screenInfo.screens[i]);
	    FreeScreen(screenInfo.screens[i]);
	    screenInfo.numScreens = i;
	}
  	CloseDownEvents();
	xfree(WindowTable);
	WindowTable = NULL;
	FreeFonts();

#ifdef DPMSExtension
	FreeDPMSTimers();
#endif
	FreeAuditTimer();

	xfree(serverClient->devPrivates);
	serverClient->devPrivates = NULL;

	if (dispatchException & DE_TERMINATE)
	{
	    CloseWellKnownConnections();
	}

	OsCleanup((dispatchException & DE_TERMINATE) != 0);

	if (dispatchException & DE_TERMINATE)
	{
	    ddxGiveUp();
	    break;
	}

	xfree(ConnectionInfo);
	ConnectionInfo = NULL;
    }
    return(0);
}

static int  VendorRelease = VENDOR_RELEASE;
static char *VendorString = VENDOR_STRING;

void
SetVendorRelease(int release)
{
    VendorRelease = release;
}

void
SetVendorString(char *string)
{
    VendorString = string;
}

static int padlength[4] = {0, 3, 2, 1};

#ifndef PANORAMIX
static
#endif
Bool
CreateConnectionBlock()
{
    xConnSetup setup;
    xWindowRoot root;
    xDepth	depth;
    xVisualType visual;
    xPixmapFormat format;
    unsigned long vid;
    int i, j, k,
        lenofblock,
        sizesofar = 0;
    char *pBuf;

    
    /* Leave off the ridBase and ridMask, these must be sent with 
       connection */

    setup.release = VendorRelease;
    /*
     * per-server image and bitmap parameters are defined in Xmd.h
     */
    setup.imageByteOrder = screenInfo.imageByteOrder;

    setup.bitmapScanlineUnit = screenInfo.bitmapScanlineUnit;
    setup.bitmapScanlinePad = screenInfo.bitmapScanlinePad;

    setup.bitmapBitOrder = screenInfo.bitmapBitOrder;
    setup.motionBufferSize = NumMotionEvents();
    setup.numRoots = screenInfo.numScreens;
    setup.nbytesVendor = strlen(VendorString); 
    setup.numFormats = screenInfo.numPixmapFormats;
    setup.maxRequestSize = MAX_REQUEST_SIZE;
    QueryMinMaxKeyCodes(&setup.minKeyCode, &setup.maxKeyCode);
    
    lenofblock = sizeof(xConnSetup) + 
            ((setup.nbytesVendor + 3) & ~3) +
	    (setup.numFormats * sizeof(xPixmapFormat)) +
            (setup.numRoots * sizeof(xWindowRoot));
    ConnectionInfo = (char *) xalloc(lenofblock);
    if (!ConnectionInfo)
	return FALSE;

    memmove(ConnectionInfo, (char *)&setup, sizeof(xConnSetup));
    sizesofar = sizeof(xConnSetup);
    pBuf = ConnectionInfo + sizeof(xConnSetup);

    memmove(pBuf, VendorString, (int)setup.nbytesVendor);
    sizesofar += setup.nbytesVendor;
    pBuf += setup.nbytesVendor;
    i = padlength[setup.nbytesVendor & 3];
    sizesofar += i;
    while (--i >= 0)
	*pBuf++ = 0;
    
    for (i=0; i<screenInfo.numPixmapFormats; i++)
    {
	format.depth = screenInfo.formats[i].depth;
	format.bitsPerPixel = screenInfo.formats[i].bitsPerPixel;
	format.scanLinePad = screenInfo.formats[i].scanlinePad;
	memmove(pBuf, (char *)&format, sizeof(xPixmapFormat));
	pBuf += sizeof(xPixmapFormat);
	sizesofar += sizeof(xPixmapFormat);
    }

    connBlockScreenStart = sizesofar;
    for (i=0; i<screenInfo.numScreens; i++) 
    {
	ScreenPtr	pScreen;
	DepthPtr	pDepth;
	VisualPtr	pVisual;

	pScreen = screenInfo.screens[i];
	root.windowId = WindowTable[i]->drawable.id;
	root.defaultColormap = pScreen->defColormap;
	root.whitePixel = pScreen->whitePixel;
	root.blackPixel = pScreen->blackPixel;
	root.currentInputMask = 0;    /* filled in when sent */
	root.pixWidth = pScreen->width;
	root.pixHeight = pScreen->height;
	root.mmWidth = pScreen->mmWidth;
	root.mmHeight = pScreen->mmHeight;
	root.minInstalledMaps = pScreen->minInstalledCmaps;
	root.maxInstalledMaps = pScreen->maxInstalledCmaps; 
	root.rootVisualID = pScreen->rootVisual;		
	root.backingStore = pScreen->backingStoreSupport;
	root.saveUnders = pScreen->saveUnderSupport != NotUseful;
	root.rootDepth = pScreen->rootDepth;
	root.nDepths = pScreen->numDepths;
	memmove(pBuf, (char *)&root, sizeof(xWindowRoot));
	sizesofar += sizeof(xWindowRoot);
	pBuf += sizeof(xWindowRoot);

	pDepth = pScreen->allowedDepths;
	for(j = 0; j < pScreen->numDepths; j++, pDepth++)
	{
	    lenofblock += sizeof(xDepth) + 
		    (pDepth->numVids * sizeof(xVisualType));
	    pBuf = (char *)xrealloc(ConnectionInfo, lenofblock);
	    if (!pBuf)
	    {
		xfree(ConnectionInfo);
		return FALSE;
	    }
	    ConnectionInfo = pBuf;
	    pBuf += sizesofar;            
	    depth.depth = pDepth->depth;
	    depth.nVisuals = pDepth->numVids;
	    memmove(pBuf, (char *)&depth, sizeof(xDepth));
	    pBuf += sizeof(xDepth);
	    sizesofar += sizeof(xDepth);
	    for(k = 0; k < pDepth->numVids; k++)
	    {
		vid = pDepth->vids[k];
		for (pVisual = pScreen->visuals;
		     pVisual->vid != vid;
		     pVisual++)
		    ;
		visual.visualID = vid;
		visual.class = pVisual->class;
		visual.bitsPerRGB = pVisual->bitsPerRGBValue;
		visual.colormapEntries = pVisual->ColormapEntries;
		visual.redMask = pVisual->redMask;
		visual.greenMask = pVisual->greenMask;
		visual.blueMask = pVisual->blueMask;
		memmove(pBuf, (char *)&visual, sizeof(xVisualType));
		pBuf += sizeof(xVisualType);
		sizesofar += sizeof(xVisualType);
	    }
	}
    }
    connSetupPrefix.success = xTrue;
    connSetupPrefix.length = lenofblock/4;
    connSetupPrefix.majorVersion = X_PROTOCOL;
    connSetupPrefix.minorVersion = X_PROTOCOL_REVISION;
    return TRUE;
}

/*
	grow the array of screenRecs if necessary.
	call the device-supplied initialization procedure 
with its screen number, a pointer to its ScreenRec, argc, and argv.
	return the number of successfully installed screens.

*/

int
AddScreen(
    Bool	(* pfnInit)(
	int /*index*/,
	ScreenPtr /*pScreen*/,
	int /*argc*/,
	char ** /*argv*/
		),
    int argc,
    char **argv)
{

    int i;
    int scanlinepad, format, depth, bitsPerPixel, j, k;
    ScreenPtr pScreen;
#ifdef DEBUG
    void	(**jNI) ();
#endif /* DEBUG */

    i = screenInfo.numScreens;
    if (i == MAXSCREENS)
	return -1;

    pScreen = (ScreenPtr) xcalloc(1, sizeof(ScreenRec));
    if (!pScreen)
	return -1;

    pScreen->devPrivates = (DevUnion *)xcalloc(sizeof(DevUnion),
						screenPrivateCount);
    if (!pScreen->devPrivates && screenPrivateCount)
    {
	xfree(pScreen);
	return -1;
    }
    pScreen->myNum = i;
    pScreen->WindowPrivateLen = 0;
    pScreen->WindowPrivateSizes = (unsigned *)NULL;
    pScreen->totalWindowSize =
        ((sizeof(WindowRec) + sizeof(long) - 1) / sizeof(long)) * sizeof(long);
    pScreen->GCPrivateLen = 0;
    pScreen->GCPrivateSizes = (unsigned *)NULL;
    pScreen->totalGCSize =
        ((sizeof(GC) + sizeof(long) - 1) / sizeof(long)) * sizeof(long);
#ifdef PIXPRIV
    pScreen->PixmapPrivateLen = 0;
    pScreen->PixmapPrivateSizes = (unsigned *)NULL;
    pScreen->totalPixmapSize = BitmapBytePad(sizeof(PixmapRec)*8);
#endif
    pScreen->ClipNotify = 0;	/* for R4 ddx compatibility */
    pScreen->CreateScreenResources = 0;
    
#ifdef DEBUG
    for (jNI = &pScreen->QueryBestSize; 
	 jNI < (void (**) ()) &pScreen->SendGraphicsExpose;
	 jNI++)
	*jNI = NotImplemented;
#endif /* DEBUG */

    /*
     * This loop gets run once for every Screen that gets added,
     * but thats ok.  If the ddx layer initializes the formats
     * one at a time calling AddScreen() after each, then each
     * iteration will make it a little more accurate.  Worst case
     * we do this loop N * numPixmapFormats where N is # of screens.
     * Anyway, this must be called after InitOutput and before the
     * screen init routine is called.
     */
    for (format=0; format<screenInfo.numPixmapFormats; format++)
    {
 	depth = screenInfo.formats[format].depth;
 	bitsPerPixel = screenInfo.formats[format].bitsPerPixel;
  	scanlinepad = screenInfo.formats[format].scanlinePad;
 	j = indexForBitsPerPixel[ bitsPerPixel ];
  	k = indexForScanlinePad[ scanlinepad ];
 	PixmapWidthPaddingInfo[ depth ].padPixelsLog2 = answer[j][k];
 	PixmapWidthPaddingInfo[ depth ].padRoundUp =
 	    (scanlinepad/bitsPerPixel) - 1;
 	j = indexForBitsPerPixel[ 8 ]; /* bits per byte */
 	PixmapWidthPaddingInfo[ depth ].padBytesLog2 = answer[j][k];
	PixmapWidthPaddingInfo[ depth ].bitsPerPixel = bitsPerPixel;
	if (answerBytesPerPixel[bitsPerPixel])
	{
	    PixmapWidthPaddingInfo[ depth ].notPower2 = 1;
	    PixmapWidthPaddingInfo[ depth ].bytesPerPixel =
		answerBytesPerPixel[bitsPerPixel];
	}
	else
	{
	    PixmapWidthPaddingInfo[ depth ].notPower2 = 0;
	}
    }
  
    /* This is where screen specific stuff gets initialized.  Load the
       screen structure, call the hardware, whatever.
       This is also where the default colormap should be allocated and
       also pixel values for blackPixel, whitePixel, and the cursor
       Note that InitScreen is NOT allowed to modify argc, argv, or
       any of the strings pointed to by argv.  They may be passed to
       multiple screens. 
    */ 
    pScreen->rgf = ~0L;  /* there are no scratch GCs yet*/
    WindowTable[i] = NullWindow;
    screenInfo.screens[i] = pScreen;
    screenInfo.numScreens++;
    if (!(*pfnInit)(i, pScreen, argc, argv))
    {
	FreeScreen(pScreen);
	screenInfo.numScreens--;
	return -1;
    }
    return i;
}

static void
FreeScreen(ScreenPtr pScreen)
{
    xfree(pScreen->WindowPrivateSizes);
    xfree(pScreen->GCPrivateSizes);
#ifdef PIXPRIV
    xfree(pScreen->PixmapPrivateSizes);
#endif
    xfree(pScreen->devPrivates);
    xfree(pScreen);
}
