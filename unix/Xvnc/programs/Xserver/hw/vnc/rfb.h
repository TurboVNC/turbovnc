/*
 * rfb.h - header file for RFB DDX implementation.
 */

/* Copyright (C) 2010-2022 D. R. Commander.  All Rights Reserved.
 * Copyright (C) 2011, 2015 Pierre Ossman for Cendio AB.  All Rights Reserved.
 * Copyright (C) 2011 Joel Martin
 * Copyright (C) 2011 Gernot Tenchio
 * Copyright (C) 2010 University Corporation for Atmospheric Research.
 *                    All Rights Reserved.
 * Copyright (C) 2000-2004 Const Kaplinsky.  All Rights Reserved.
 * Copyright (C) 2000 Tridia Corporation.  All Rights Reserved.
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

#ifndef __RFB_H__
#define __RFB_H__


#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "scrnintstr.h"
#include "colormapst.h"
#include "gcstruct.h"
#include "osdep.h"
#include "list.h"
#include <rfbproto.h>
#include <turbovnc_gii.h>
#include <vncauth.h>
#include <zlib.h>
#include <stdarg.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#ifdef RENDER
#include "picturestr.h"
#endif
#ifdef RANDR
#include "randrstr.h"
#endif
#include "mipointer.h"
#include "input.h"
#ifdef XVNC_AuthPAM
#ifdef __APPLE__
#include <AvailabilityMacros.h>
#endif
#if defined(__APPLE__) && defined(MAC_OS_X_VERSION_MAX_ALLOWED) && MAC_OS_X_VERSION_MAX_ALLOWED < 1060
#include <pam/pam_appl.h>
#else
#include <security/pam_appl.h>
#endif
#endif

/* It's a good idea to keep these values a bit greater than required. */
#define MAX_ENCODINGS 18
#define MAX_SECURITY_TYPES 4
#define MAX_TUNNELING_CAPS 16
#define MAX_AUTH_CAPS 16
#define MAX_VENCRYPT_SUBTYPES 16


#define DEFAULT_AUTH_MAX_FAILS 5
#define DEFAULT_AUTH_FAIL_TIMEOUT 10

/* Protect ourself against a denial of service */
#define MAX_CUTTEXT_LEN (1 * 1024 * 1024)

#define DEFAULT_DEFER_UPDATE_TIME 40

/* Maximum number of threads to use for multithreaded encoding, regardless of
   the CPU count */
#define MAX_ENCODING_THREADS 8

/* Maximum number of client connections.  The default of 100 should be more
   than enough for most use cases.  The ceiling is set to 500 to give us plenty
   of room to avoid exceeding the Xvnc process's allotment of file descriptors,
   which is usually 1024 on Linux systems. */
#define DEFAULT_MAX_CONNECTIONS 100
#define MAX_MAX_CONNECTIONS 500

#define DEFAULT_MAX_CLIENT_WAIT 20000


/*
 * Per-screen (framebuffer) structure.  There is only one of these, since we
 * don't allow the X server to have multiple screens.
 */

typedef struct {
  int width;
  int paddedWidthInBytes;
  int height;
  int depth;
  int bitsPerPixel;
  int sizeInBytes;
  char *pfbMemory;
  Pixel blackPixel;
  Pixel whitePixel;

  /* The following two members are used to minimise the amount of unnecessary
     drawing caused by cursor movement.  Whenever any drawing affects the
     part of the screen where the cursor is, the cursor is removed first and
     then the drawing is done (this is what the sprite routines test for).
     Afterwards, however, we do not replace the cursor, even when the cursor
     is logically being moved across the screen.  We only draw the cursor
     again just as we are about to send the client a framebuffer update.

     We need to be careful when removing and drawing the cursor because of
     their relationship with the normal drawing routines.  The drawing
     routines can invoke the cursor routines, but also the cursor routines
     themselves end up invoking drawing routines.

     Removing the cursor (rfbSpriteRemoveCursor) is eventually achieved by
     doing a CopyArea from a pixmap to the screen, where the pixmap contains
     the saved contents of the screen under the cursor.  Before doing this,
     however, we set cursorIsDrawn to FALSE.  Then, when CopyArea is called,
     it sees that cursorIsDrawn is FALSE and so doesn't feel the need to
     (recursively!) remove the cursor before doing it.

     Putting up the cursor (rfbSpriteRestoreCursor) involves a call to
     PushPixels.  While this is happening, cursorIsDrawn must be FALSE so
     that PushPixels doesn't think it has to remove the cursor first.
     Obviously cursorIsDrawn is set to TRUE afterwards.

     Another problem we face is that drawing routines sometimes cause a
     framebuffer update to be sent to the RFB client.  When the RFB client is
     already waiting for a framebuffer update and some drawing to the
     framebuffer then happens, the drawing routine sees that the client is
     ready, so it calls rfbSendFramebufferUpdate.  If the cursor is not drawn
     at this stage, it must be put up, and so rfbSpriteRestoreCursor is
     called.  However, if the original drawing routine was actually called
     from within rfbSpriteRestoreCursor or rfbSpriteRemoveCursor we don't
     want this to happen.  So both the cursor routines set
     dontSendFramebufferUpdate to TRUE, and all the drawing routines check
     this before calling rfbSendFramebufferUpdate. */

  Bool cursorIsDrawn;                /* TRUE if the cursor is currently
                                        drawn */
  Bool dontSendFramebufferUpdate;    /* TRUE while removing or drawing the
                                        cursor */
  Bool blockUpdates;                 /* TRUE while resizing the screen */

  /* wrapped screen functions */

  CloseScreenProcPtr                    CloseScreen;
  CreateGCProcPtr                       CreateGC;
  CopyWindowProcPtr                     CopyWindow;
  ClearToBackgroundProcPtr              ClearToBackground;
#ifdef RENDER
  CompositeProcPtr                      Composite;
  GlyphsProcPtr                         Glyphs;
#endif
  InstallColormapProcPtr                InstallColormap;
  UninstallColormapProcPtr              UninstallColormap;
  ListInstalledColormapsProcPtr         ListInstalledColormaps;
  StoreColorsProcPtr                    StoreColors;
  SaveScreenProcPtr                     SaveScreen;

} rfbFBInfo, *rfbFBInfoPtr;


typedef struct _Res {
  int w, h;
} Res;

typedef struct {
  RROutputPtr output;
  Bool idAssigned, used;
  Res prefRes;
  rfbScreenDesc s;
  struct xorg_list entry;
} rfbScreenInfo, *rfbScreenInfoPtr;

typedef struct {
  union {
    struct sockaddr sa;
    struct sockaddr_storage ss;
    struct sockaddr_in sin;
    struct sockaddr_in6 sin6;
  } u;
} rfbSockAddr;

typedef struct {
  struct timeval tv;
  unsigned pos, extra;
  char congested;
  struct xorg_list entry;
} rfbRTTInfo;


/*
 * rfbTranslateFnType is the type of translation functions.
 */

struct rfbClientRec;
typedef void (*rfbTranslateFnType) (char *table, rfbPixelFormat *in,
                                    rfbPixelFormat *out,
                                    char *iptr, char *optr,
                                    int bytesBetweenInputLines,
                                    int width, int height);


/*
 * Extended input device structure.
 */

typedef struct {
  char name[32];
  int numButtons;
  Bool multitouch;
  int numTouches;
  int numValuators;
  int mode;
  CARD32 eventMask;
  CARD32 productID;
  DeviceIntPtr pDev;
  rfbGIIValuator valuators[MAX_VALUATORS];
  int values[MAX_VALUATORS];
  CARD32 valFirst, valCount;
} rfbDevInfo, *rfbDevInfoPtr;


/*
 * Per-client structure.
 */

#if USETLS
typedef struct _rfbSslCtx rfbSslCtx;
#endif
typedef struct _wsCtx wsCtx;

typedef struct rfbClientRec {

  int sock;
  char *host;
  char *login;

  int protocol_minor_ver;       /* RFB protocol minor version in use */
  Bool protocol_tightvnc;       /* TightVNC protocol extensions enabled */

  /* Possible client states: */

  enum {
    RFB_PROTOCOL_VERSION,       /* establishing protocol version */
    RFB_SECURITY_TYPE,          /* negotiating security (RFB v.3.7) */
    RFB_TUNNELING_TYPE,         /* establishing tunneling (RFB v.3.7t) */
    RFB_AUTH_TYPE,              /* negotiating authentication (RFB v.3.7t) */
    RFB_AUTHENTICATION,         /* authenticating (VNC authentication) */
#if USETLS
    RFB_TLS_HANDSHAKE,          /* waiting for TLS handshake to complete */
#endif
    RFB_INITIALISATION,         /* sending initialisation messages */
    RFB_NORMAL                  /* normal protocol messages */
  } state;

  Bool viewOnly;                /* Do not accept input from this client. */

  Bool reverseConnection;

  Bool readyForSetColourMapEntries;

  Bool useCopyRect;
  int preferredEncoding;
  int correMaxWidth, correMaxHeight;

  /* The list of security types sent to this client (protocol 3.7).
     Note that the first entry is the number of list items following. */

  CARD8 securityTypes[MAX_SECURITY_TYPES + 1];

  /* Lists of capability codes sent to clients. We remember these
     lists to restrict clients from choosing those tunneling and
     authentication types that were not advertised. */

  int nAuthCaps;
  CARD32 authCaps[MAX_AUTH_CAPS];
  CARD8 selectedAuthType;

  /* This is not useful while we don't support tunneling:
  int nTunnelingCaps;
  CARD32 tunnelingCaps[MAX_TUNNELING_CAPS]; */

  /* The following member is only used during VNC authentication */

  CARD8 authChallenge[CHALLENGESIZE];

#ifdef XVNC_AuthPAM
  pam_handle_t *pamHandle;
#endif

  /* The following members represent the update needed to get the client's
     framebuffer from its present state to the current state of our
     framebuffer.

     If the client does not accept CopyRect encoding then the update is
     simply represented as the region of the screen which has been modified
     (modifiedRegion).

     If the client does accept CopyRect encoding, then the update consists of
     two parts.  First we have a single copy from one region of the screen to
     another (the destination of the copy is copyRegion), and second we have
     the region of the screen which has been modified in some other way
     (modifiedRegion).

     Although the copy is of a single region, this region may have many
     rectangles.  When sending an update, the copyRegion is always sent
     before the modifiedRegion.  This is because the modifiedRegion may
     overlap parts of the screen which are in the source of the copy.

     In fact during normal processing, the modifiedRegion may even overlap
     the destination copyRegion.  Just before an update is sent we remove
     from the copyRegion anything in the modifiedRegion. */

  RegionRec copyRegion;         /* the destination region of the copy */
  int copyDX, copyDY;           /* the translation by which the copy happens */

  RegionRec modifiedRegion;     /* the region of the screen modified in any
                                   other way */

  /* As part of the FramebufferUpdateRequest, a client can express interest
     in a subrectangle of the whole framebuffer.  This is stored in the
     requestedRegion member.  In the normal case this is the whole
     framebuffer if the client is ready, empty if it's not. */

  RegionRec requestedRegion;

  /* The following members represent the state of the "deferred update" timer
     - when the framebuffer is modified and the client is ready, in most
     cases it is more efficient to defer sending the update by a few
     milliseconds so that several changes to the framebuffer can be combined
     into a single update. */

  Bool deferredUpdateScheduled;
  OsTimerPtr deferredUpdateTimer;
  double deferredUpdateStart;

  /* translateFn points to the translation function which is used to copy
     and translate a rectangle from the framebuffer to an output buffer. */

  rfbTranslateFnType translateFn;

  char *translateLookupTable;

  rfbPixelFormat format;

  /* statistics */

  long long rfbBytesSent[MAX_ENCODINGS];
  int rfbRectanglesSent[MAX_ENCODINGS];
  int rfbLastRectMarkersSent;
  long long rfbLastRectBytesSent;
  long long rfbCursorShapeBytesSent;
  int rfbCursorShapeUpdatesSent;
  long long rfbCursorPosBytesSent;
  int rfbCursorPosUpdatesSent;
  int rfbFramebufferUpdateMessagesSent;
  long long rfbRawBytesEquivalent;
  int rfbKeyEventsRcvd;
  int rfbPointerEventsRcvd;

  /* zlib encoding -- necessary compression state info per client */

  struct z_stream_s compStream;
  Bool compStreamInited;

  CARD32 zlibCompressLevel;

  /* ZRLE encoding */

  void *zrleData;
  int zywrleLevel;
  int zywrleBuf[rfbZRLETileWidth * rfbZRLETileHeight];
  char *zrleBeforeBuf;
  void *paletteHelper;

  /* tight encoding -- preserve zlib streams' state for each client */

  z_stream zsStruct[4];
  Bool zsActive[4];
  int zsLevel[4];
  int tightCompressLevel;
  int tightSubsampLevel;
  int tightQualityLevel;
  int imageQualityLevel;

  Bool enableLastRectEncoding;      /* client supports LastRect encoding */
  Bool enableCursorShapeUpdates;    /* client supports cursor shape updates */
  Bool enableCursorPosUpdates;      /* client supports PointerPos updates */
  Bool enableExtClipboard;          /* client supports Extended Clipboard
                                       extension */
  Bool enableCU;                    /* client supports Continuous Updates */
  Bool enableFence;                 /* client supports fence extension */
  Bool enableDesktopSize;           /* client supports desktop size
                                       extension */
  Bool enableExtDesktopSize;        /* client supports extended desktop size
                                       extension */
  Bool enableGII;                   /* client supports GII extension */
  Bool enableQEMUExtKeyEvent;       /* client supports QEMU Extended Key Event
                                       extension */
  Bool enableQEMULEDState;          /* client supports QEMU LED State
                                       extension */
  Bool enableVMwareLEDState;        /* client supports VMware LED State
                                       extension */
  Bool useRichCursorEncoding;       /* rfbEncodingRichCursor is preferred */
  Bool cursorWasChanged;            /* cursor shape update should be sent */
  Bool cursorWasMoved;              /* cursor position update should be sent */

  int cursorX, cursorY;             /* client's cursor position */

  Bool firstUpdate, inALR;
  OsTimerPtr alrTimer;
  RegionRec lossyRegion, alrRegion, alrEligibleRegion;

  /* Interframe comparison */
  char *compareFB, *fb;
  Bool firstCompare;
  RegionRec ifRegion;

  struct rfbClientRec *prev, *next;

  /* Clipboard */
  char *clientClipboard;
  CARD32 clipFlags, clipSizes[16];
  Bool hasLocalClipboard, unsolicitedClipboardAttempt;
  struct xorg_list entry;

  /* flow control extensions */

  Bool continuousUpdates;
  RegionRec cuRegion;

  Bool pendingSyncFence, syncFence;
  CARD32 fenceFlags;
  unsigned fenceDataLen;
  char fenceData[64];

  unsigned lastPosition, extraBuffer;
  struct timeval lastUpdate, lastSent;
  unsigned baseRTT, congWindow;
  Bool inSlowStart;
  int sockOffset;
  struct xorg_list pings;
  OsTimerPtr congestionTimer;
  rfbRTTInfo lastPong;
  struct timeval lastPongArrival;
  int measurements;
  struct timeval lastAdjustment;
  unsigned minRTT, minCongestedRTT;

  Bool pendingDesktopResize, pendingExtDesktopResize;
  int reason, result;

  /* Server-side key mapping */
  Bool pendingQEMUExtKeyEventRect, pendingLEDState;
  unsigned int ledState;

#if USETLS
  rfbSslCtx *sslctx;
#endif
  wsCtx     *wsctx;

  /* Extended input device support */
  rfbDevInfo devices[MAXDEVICES];
  int numDevices;

  /* Session capture */
  int captureFD;
  Bool captureEnable;

} rfbClientRec, *rfbClientPtr;


/*
 * This macro is used to test whether there is a framebuffer update needing to
 * be sent to the client.
 */

#define FB_UPDATE_PENDING(cl)  \
  ((!(cl)->enableCursorShapeUpdates && !rfbFB.cursorIsDrawn) ||  \
   ((cl)->enableCursorShapeUpdates && (cl)->cursorWasChanged) ||  \
   ((cl)->enableCursorPosUpdates && (cl)->cursorWasMoved) ||  \
   REGION_NOTEMPTY((pScreen), &(cl)->copyRegion) ||  \
   REGION_NOTEMPTY((pScreen), &(cl)->modifiedRegion))

/*
 * This macro creates an empty region (ie. a region with no areas) if it is
 * given a rectangle with a width or height of zero. It appears that
 * REGION_INTERSECT does not quite do the right thing with zero-width
 * rectangles, but it should with completely empty regions.
 */

#define SAFE_REGION_INIT(pscreen, preg, rect, size) {  \
  if ( ( (rect)->x2 == (rect)->x1 ) || ( (rect)->y2 == (rect)->y1 ) ) {  \
    REGION_INIT( (pscreen), (preg), NullBox, 0 );  \
  } else {  \
    REGION_INIT( (pscreen), (preg), (rect), (size) );  \
  }  \
}

/*
 * This macro is used to test whether a particular client pointer has been
 * freed and, if so, to take appropriate action.
 */
#define CHECK_CLIENT_PTR(cl, action) {  \
  rfbClientPtr cl2;  \
  for (cl2 = rfbClientHead; cl2; cl2 = cl2->next) {  \
    if (cl2 == cl)  \
      break;  \
  }  \
  if (cl2 == NULL) action;  \
}

/*
 * This macro is used to test whether either LED state extension is supported
 * by a given client.
 */
#define SUPPORTS_LED_STATE(cl)  \
  (cl->enableQEMULEDState || cl->enableVMwareLEDState)

/*
 * An rfbGCRec is where we store the pointers to the original GC funcs and ops
 * which we wrap (NULL means not wrapped).
 */

typedef struct {
  const GCFuncs *wrapFuncs;
  const GCOps *wrapOps;
} rfbGCRec, *rfbGCPtr;



/*
 * Macros for endian swapping.
 */

#define Swap16(s) ((((s) & 0xff) << 8) | (((s) >> 8) & 0xff))

#define Swap32(l) (((l) >> 24) |  \
                   (((l) & 0x00ff0000) >> 8)  |  \
                   (((l) & 0x0000ff00) << 8)  |  \
                   ((l) << 24))

static const int rfbEndianTest = 1;

#define Swap16IfLE(s) (*(const char *)&rfbEndianTest ? Swap16(s) : (s))

#define Swap32IfLE(l) (*(const char *)&rfbEndianTest ? Swap32(l) : (l))


/*
 * Macro to fill in an rfbCapabilityInfo structure (protocol 3.7t).
 * Normally, using macros is no good, but this macro saves us from
 * writing constants twice -- it constructs signature names from codes.
 * Note that "code_sym" argument should be a single symbol, not an expression.
 */

#define SetCapInfo(cap_ptr, code_sym, vendor) {  \
  rfbCapabilityInfo *pcap;  \
  pcap = (cap_ptr);  \
  pcap->code = Swap32IfLE((CARD32)code_sym);  \
  memcpy(pcap->vendorSignature, (vendor), sz_rfbCapabilityInfoVendor);  \
  memcpy(pcap->nameSignature, sig_##code_sym, sz_rfbCapabilityInfoName);  \
}


/*
 * Macros for reading/writing data from/to clients
 */

#define READ_OR_CLOSE(addr, numBytes, failAction) {  \
  int __n;  \
  if ((__n = ReadExact(cl, addr, numBytes)) <= 0) {  \
    if (__n != 0) {  \
      char __temps[80];  \
      snprintf(__temps, 80, "%s (%d): read error", __FUNCTION__, __LINE__);  \
      rfbLogPerror(__temps);  \
    }  \
    rfbCloseClient(cl);  \
    failAction;  \
  }  \
}

#define READ32_OR_CLOSE(var, failAction) {  \
  CARD32 __tmp;  \
  READ_OR_CLOSE((char *)&__tmp, sizeof(CARD32), failAction);  \
  var = Swap32IfLE(__tmp);  \
}

#define SKIP_OR_CLOSE(numBytes, failAction) {  \
  int __n;  \
  if ((__n = SkipExact(cl, numBytes)) <= 0) {  \
    if (__n != 0) {  \
      char __temps[80];  \
      snprintf(__temps, 80, "%s (%d): skip error", __FUNCTION__, __LINE__);  \
      rfbLogPerror(__temps);  \
    }  \
    rfbCloseClient(cl);  \
    failAction;  \
  }  \
}

#define WRITE_OR_CLOSE(addr, numBytes, failAction) {  \
  if (WriteExact(cl, (char *)(addr), numBytes) < 0) {  \
    char __temps[80];  \
    snprintf(__temps, 80, "%s (%d): write error", __FUNCTION__, __LINE__);  \
    rfbLogPerror(__temps);  \
    rfbCloseClient(cl);  \
    failAction;  \
  }  \
}

#define WRITE32_OR_CLOSE(value, failAction) {  \
  CARD32 __tmp = value;  \
  __tmp = Swap32IfLE(__tmp);  \
  WRITE_OR_CLOSE((char *)&__tmp, sizeof(CARD32), failAction);  \
}


/*
 * These can be used to add profiling information anywhere within the code
 */

#define ProfileInit()  \
  static double __start, __mpixels = 0.;  \
  static int __first = 1;  \
  double __elapsed;  \
  static int __iter = 0;  \
  if (__first) { __start = gettime();  __first = 0; }

#define ProfileCount(message, pixels, time)  \
  __iter++;  \
  __mpixels += (double)(pixels) / 1000000.;  \
  if ((__elapsed = gettime() - __start) >= (double)(time)) {  \
    rfbLog(message":  %f/sec  %f Mpixels/sec\n",  \
           (double)__iter / __elapsed, __mpixels / __elapsed);  \
    __start = gettime();  \
    __iter = 0;  __mpixels = 0.;  \
  }


/*
 * These can be used to add tracing information anywhere within the code
 */

extern int traceLevel;

#define OPENTRACE(message) {  \
  rfbLog("+ %s %s\n", __FUNCTION__, message);  \
  traceLevel++;  \
}

#define CLOSETRACE(message) {  \
  traceLevel--;  \
  rfbLog("- %s %s\n", __FUNCTION__, message);  \
}


/* auth.c */

void rfbAuthInit(void);
void rfbAuthProcessResponse(rfbClientPtr cl);
void rfbAuthParseCommandLine(char *buf);
void rfbAuthListAvailableSecurityTypes(void);
extern char *rfbAuthConfigFile;

extern Bool  rfbOptOtpAuth(void);
extern Bool  rfbOptPamAuth(void);
extern Bool  rfbOptRfbAuth(void);

extern char *rfbAuthOTPValue;
extern int   rfbAuthOTPValueLen;
extern Bool  rfbAuthDisableRemoteResize;
extern Bool  rfbAuthDisableRevCon;
extern Bool  rfbAuthDisableCBSend;
extern Bool  rfbAuthDisableCBRecv;
extern Bool  rfbAuthDisableX11TCP;

#ifdef XVNC_AuthPAM
extern void rfbAuthAddUser(const char *name, Bool viewOnly);
extern void rfbAuthRevokeUser(const char *name);
#endif

extern char *rfbAuthPasswdFile;
#if USETLS
extern char *rfbAuthX509Cert;
extern char *rfbAuthX509Key;
extern char *rfbAuthCipherSuites;
#endif

extern void rfbAuthNewClient(rfbClientPtr cl);
extern void rfbProcessClientSecurityType(rfbClientPtr cl);
extern void rfbProcessClientTunnelingType(rfbClientPtr cl);
extern void rfbProcessClientAuthType(rfbClientPtr cl);
extern void rfbVncAuthProcessResponse(rfbClientPtr cl);
#if USETLS
extern void rfbAuthTLSHandshake(rfbClientPtr cl);
#endif

extern void rfbClientConnFailed(rfbClientPtr cl, char *reason);
extern void rfbClientAuthFailed(rfbClientPtr cl, char *reason);
extern void rfbClientAuthSucceeded(rfbClientPtr cl, CARD32 authType);

/* Functions to prevent too many successive authentication failures */
extern int rfbAuthMaxFails;
extern CARD32 rfbAuthFailTimeout;

extern struct xorg_list rfbAuthFails;

extern Bool rfbAuthConsiderBlocking(char *host);
extern void rfbAuthUnblock(char *host);
extern Bool rfbAuthIsBlocked(char *host);


/* authpam.c */

#ifdef XVNC_AuthPAM
extern void rfbPAMEnd(rfbClientPtr cl);

extern Bool rfbAuthPAMSession;
extern Bool rfbAuthDisablePAMSession;

Bool rfbPAMAuthenticate(rfbClientPtr cl, const char *svc, const char *user,
                        const char *pwd, const char **emsg);
#endif


/* cmap.c */

extern ColormapPtr rfbInstalledColormap;

extern int rfbListInstalledColormaps(ScreenPtr pScreen, Colormap *pmaps);
extern void rfbInstallColormap(ColormapPtr pmap);
extern void rfbUninstallColormap(ColormapPtr pmap);
extern void rfbStoreColors(ColormapPtr pmap, int ndef, xColorItem *pdefs);


/* corre.c */

extern Bool rfbSendRectEncodingCoRRE(rfbClientPtr cl, int x, int y, int w,
                                     int h);


/* cursor.c */

extern Bool rfbSendCursorShape(rfbClientPtr cl, ScreenPtr pScreen);
extern Bool rfbSendCursorPos(rfbClientPtr cl, ScreenPtr pScreen);


/* cutpaste.c */

extern Bool rfbSyncPrimary;
extern struct xorg_list clipboardRequestors;

extern void rfbHandleClientCutText(rfbClientPtr cl, const char *str, int len);
extern void rfbHandleClipboardAnnounce(rfbClientPtr cl, Bool available);
extern void rfbReadExtClipboard(rfbClientPtr cl, int len);
extern void rfbSendClipboardCaps(rfbClientPtr cl, CARD32 caps,
                                 const CARD32 *lengths);
extern int vncConvertSelection(ClientPtr client, Atom selection, Atom target,
                               Atom property, Window requestor, CARD32 time,
                               const char *data);
extern Window vncGetSelectionWindow(void);
extern void vncHandleSelection(Atom selection, Atom target, Atom property,
                               Atom requestor, TimeStamp time);
extern void vncSelectionInit(void);


/* dispcur.c */

extern Bool rfbDCInitialize(ScreenPtr, miPointerScreenFuncPtr);


/* draw.c */

extern int rfbDeferUpdateTime;

extern void ClipToScreen(ScreenPtr pScreen, RegionPtr pRegion);
void PrintRegion(ScreenPtr pScreen, RegionPtr reg, const char *msg);

#ifdef RENDER
extern void rfbComposite(CARD8 op, PicturePtr pSrc, PicturePtr pMask,
                         PicturePtr pDst, INT16 xSrc, INT16 ySrc, INT16 xMask,
                         INT16 yMask, INT16 xDst, INT16 yDst, CARD16 width,
                         CARD16 height);

extern void rfbGlyphs(CARD8 op, PicturePtr pSrc, PicturePtr pDst,
                      PictFormatPtr maskFormat, INT16 xSrc, INT16 ySrc,
                      int nlists, GlyphListPtr lists, GlyphPtr *glyphs);
#endif

extern Bool rfbCloseScreen(ScreenPtr);
extern Bool rfbCreateGC(GCPtr);
extern void rfbPaintWindowBackground(WindowPtr, RegionPtr, int what);
extern void rfbPaintWindowBorder(WindowPtr, RegionPtr, int what);
extern void rfbCopyWindow(WindowPtr, DDXPointRec, RegionPtr);
extern void rfbClearToBackground(WindowPtr, int x, int y, int w, int h,
                                 Bool generateExposures);
extern RegionPtr rfbRestoreAreas(WindowPtr, RegionPtr);


/* flowcontrol.c */

extern void rfbInitFlowControl(rfbClientPtr cl);
extern void rfbUpdatePosition(rfbClientPtr cl, unsigned pos);
extern Bool rfbSendRTTPing(rfbClientPtr cl);
extern Bool rfbIsCongested(rfbClientPtr cl);
extern Bool rfbSendFence(rfbClientPtr cl, CARD32 flags, unsigned len,
                         const char *data);
extern void HandleFence(rfbClientPtr cl, CARD32 flags, unsigned len,
                        const char *data);
extern Bool rfbSendEndOfCU(rfbClientPtr cl);


/* hextile.c */

extern Bool rfbSendRectEncodingHextile(rfbClientPtr cl, int x, int y, int w,
                                       int h);


/* init.c */

extern char *desktopName;
extern char rfbThisHost[];
extern Atom VNC_LAST_CLIENT_ID;

extern rfbFBInfo rfbFB;
extern DevPrivateKeyRec rfbGCKey;
extern rfbDevInfo virtualTabletTouch;
extern rfbDevInfo virtualTabletStylus;
extern rfbDevInfo virtualTabletEraser;
extern rfbDevInfo virtualTabletPad;

extern int inetdSock;
extern struct in_addr interface;
extern struct in6_addr interface6;
extern int family;

extern int rfbLEDState;

extern int rfbBitsPerPixel(int depth);
extern void rfbLog(char *format, ...);
extern void rfbLogPerror(char *str);
extern void rfbRootPropertyChange(PropertyPtr pProp);

extern Bool AddExtInputDevice(rfbDevInfo *dev);
extern void RemoveExtInputDevice(rfbClientPtr cl, int index);

extern void *rfbAlloc(size_t size);
extern void *rfbAlloc0(size_t size);
extern void *rfbRealloc(void *ptr, size_t size);


/* kbdptr.c */

extern Bool compatibleKbd;
extern Bool enableQEMUExtKeyEvent;
extern unsigned char ptrAcceleration;

extern void PtrDeviceOn(DeviceIntPtr);
extern void PtrDeviceControl(DevicePtr, PtrCtrl *);
extern void PtrAddEvent(int buttonMask, int x, int y, rfbClientPtr cl);
extern void ExtInputAddEvent(rfbDevInfoPtr dev, int type, int buttons);

extern void KbdDeviceInit(DeviceIntPtr);
extern void ExtKeyEvent(KeySym keySym, unsigned keycode, BOOL down);
extern void KeyEvent(KeySym keySym, Bool down);
extern void KbdReleaseAllKeys(void);
extern void QEMUExtKeyboardEventInit(void);

extern char *stristr(const char *s1, const char *s2);


/* nvctrlext.c */

extern char *nvCtrlDisplay;


/* randr.c */

#ifdef RANDR
extern int ResizeDesktop(ScreenPtr pScreen, rfbClientPtr cl, int w, int h,
                         struct xorg_list *newScreens);
extern Bool vncRRInit(ScreenPtr);
extern void vncRRDeinit(ScreenPtr);
#endif


/* rfbscreen.c */

extern struct xorg_list rfbScreens;

void rfbAddScreen(struct xorg_list *list, rfbScreenInfo *screen);
void rfbClipScreens(struct xorg_list *list, int w, int h);
void rfbDupeScreens(struct xorg_list *newList, struct xorg_list *list);
rfbScreenInfo *rfbFindScreen(struct xorg_list *list, CARD16 x, CARD16 y,
                             CARD16 w, CARD16 h);
rfbScreenInfo *rfbFindScreenID(struct xorg_list *list, CARD32 id);
rfbScreenInfo *rfbNewScreen(CARD32 id, CARD16 x, CARD16 y, CARD16 w, CARD16 h,
                            CARD32 flags);
void rfbRemoveScreen(rfbScreenInfo *screen);
void rfbRemoveScreens(struct xorg_list *list);


/* rfbserver.c */

/*
 * UPDATE_BUF_SIZE must be big enough to send at least one whole line of the
 * framebuffer.  So for a max screen width of say 2K with 32-bit pixels this
 * means 8K minimum.
 */

#define UPDATE_BUF_SIZE 30000
extern char updateBuf[UPDATE_BUF_SIZE];
extern int ublen;

extern double gettime(void);

extern rfbClientPtr rfbClientHead;
extern rfbClientPtr pointerDragClient;
extern rfbClientPtr pointerOwner;

extern CARD32 rfbMaxIdleTimeout;
extern CARD32 rfbIdleTimeout;
extern void IdleTimerSet(void);
extern void IdleTimerCheck(void);
extern Bool InterframeOn(rfbClientPtr cl);
extern void InterframeOff(rfbClientPtr);
extern BOOL rfbProfile;

extern int rfbMaxWidth, rfbMaxHeight;

extern Bool rfbAlwaysShared;
extern Bool rfbNeverShared;
extern Bool rfbDisconnect;
extern Bool rfbViewOnly;  /* run server in view-only mode - Ehud Karni SW */
extern Bool rfbCongestionControl;
extern double rfbAutoLosslessRefresh;
extern Bool rfbALRAll;
extern int rfbALRQualityLevel;
extern int rfbALRSubsampLevel;
extern int rfbInterframe;
extern int rfbMaxClipboard;
extern Bool rfbVirtualTablet;

/* Multithreading params specified on the command line or in the environment */
extern Bool rfbMT;
extern int rfbNumThreads;

extern char *rfbCaptureFile;

#define debugregion(r, m)  \
  rfbLog(m" %d, %d %d x %d\n", (r).extents.x1, (r).extents.y1,  \
         (r).extents.x2 - (r).extents.x1, (r).extents.y2 - (r).extents.y1)

extern void rfbNewClientConnection(int sock);
extern rfbClientPtr rfbReverseConnection(char *host, int port, int id);
extern void rfbClientConnectionGone(rfbClientPtr cl);
extern void rfbProcessClientMessage(rfbClientPtr cl);
extern Bool rfbSendFramebufferUpdate(rfbClientPtr cl);
extern Bool rfbSendRectEncodingRaw(rfbClientPtr cl, int x, int y, int w,
                                   int h);
extern Bool rfbSendUpdateBuf(rfbClientPtr cl);
extern Bool rfbSendSetColourMapEntries(rfbClientPtr cl, int firstColour,
                                       int nColours);
extern void rfbSendBell(void);
extern void rfbWriteCapture(int captureFD, char *buf, int len);


#if USETLS

/* rfbssl_*.c */

extern CARD32 rfbTLSKeyLength;

rfbSslCtx *rfbssl_init(rfbClientPtr cl, Bool anon);
int rfbssl_accept(rfbClientPtr cl);
int rfbssl_pending(rfbClientPtr cl);
int rfbssl_peek(rfbClientPtr cl, char *buf, int bufsize);
int rfbssl_read(rfbClientPtr cl, char *buf, int bufsize);
int rfbssl_write(rfbClientPtr cl, const char *buf, int bufsize);
void rfbssl_destroy(rfbClientPtr cl);
char *rfbssl_geterr(void);

#endif


/* rre.c */

extern Bool rfbSendRectEncodingRRE(rfbClientPtr cl, int x, int y, int w,
                                   int h);


/* sockets.c */

extern int rfbMaxClientConnections;
extern int rfbMaxClientWait;

extern int rfbPort;
extern const char *rfbUDSPath;
extern int rfbUDSMode;
extern Bool rfbUDSCreated;
extern int rfbListenSock;

extern void rfbInitSockets(void);
extern void rfbCloseSock(int);
extern void rfbCloseClient(rfbClientPtr cl);
extern int rfbConnect(char *host, int port);
extern void rfbCorkSock(int sock);
extern void rfbUncorkSock(int sock);

extern int PeekExactTimeout(rfbClientPtr cl, char *buf, int len, int timeout);
extern int ReadExact(rfbClientPtr cl, char *buf, int len);
extern int ReadExactTimeout(rfbClientPtr cl, char *buf, int len, int timeout);
extern int SkipExact(rfbClientPtr cl, int len);
extern int WriteExact(rfbClientPtr cl, char *buf, int len);
extern int ConnectToTcpAddr(char *host, int port);

extern const char *sockaddr_string(rfbSockAddr *addr, char *buf, int len);


/* stats.c */

extern void rfbResetStats(rfbClientPtr cl);
extern void rfbPrintStats(rfbClientPtr cl);


/* strsep.c */
#ifndef HAVE_STRSEP
char *strsep(char **stringp, const char *delim);
#endif


/* tight.c */

#define TVNC_SAMPOPT 4
enum { TVNC_1X = 0, TVNC_4X, TVNC_2X, TVNC_GRAY };
static const char subsampStr[TVNC_SAMPOPT][5] = { "1X", "4X", "2X", "Gray" };
#define TIGHT_DEFAULT_COMPRESSION  1
#define TIGHT_DEFAULT_SUBSAMP      TVNC_1X
#define TIGHT_DEFAULT_QUALITY      95

extern int rfbNumCodedRectsTight(rfbClientPtr cl, int x, int y, int w, int h);
extern Bool rfbSendRectEncodingTight(rfbClientPtr cl, int x, int y, int w,
                                     int h);
extern int rfbTightCompressLevel(rfbClientPtr cl);
extern void ShutdownTightThreads(void);


/* translate.c */

extern Bool rfbEconomicTranslate;
extern rfbPixelFormat rfbServerFormat;

extern void rfbTranslateNone(char *table, rfbPixelFormat *in,
                             rfbPixelFormat *out,
                             char *iptr, char *optr,
                             int bytesBetweenInputLines,
                             int width, int height);
extern Bool rfbSetTranslateFunction(rfbClientPtr cl);
extern void rfbSetClientColourMaps(int firstColour, int nColours);
extern Bool rfbSetClientColourMap(rfbClientPtr cl, int firstColour,
                                  int nColours);


/* websockets.c */

extern Bool webSocketsCheck(rfbClientPtr cl);
extern int webSocketsEncode(rfbClientPtr cl, const char *src, int len,
                            char **dst);
extern int webSocketsDecode(rfbClientPtr cl, char *dst, int len);
extern Bool webSocketsHasDataInBuffer(rfbClientPtr cl);
extern void webSocketsFree(rfbClientPtr cl);


/* zlib.c */

/* Minimum zlib rectangle size in bytes.  Anything smaller will
 * not compress well due to overhead.
 */
#define VNC_ENCODE_ZLIB_MIN_COMP_SIZE (17)

/* Set maximum zlib rectangle size in pixels.  Always allow at least
 * two scan lines.
 */
#define ZLIB_MAX_RECT_SIZE (128 * 256)
#define ZLIB_MAX_SIZE(min) ((( min * 2 ) > ZLIB_MAX_RECT_SIZE ) ?  \
                            ( min * 2 ) : ZLIB_MAX_RECT_SIZE )

extern Bool rfbSendRectEncodingZlib(rfbClientPtr cl, int x, int y, int w,
                                    int h);


/* zrle.c */
extern Bool rfbSendRectEncodingZRLE(rfbClientPtr cl, int x, int y, int w,
                                    int h);
void rfbFreeZrleData(rfbClientPtr cl);


#endif  /* __RFB_H__ */
