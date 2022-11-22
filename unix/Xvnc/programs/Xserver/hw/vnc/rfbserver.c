/*
 * rfbserver.c - deal with server-side of the RFB protocol.
 */

/* Copyright (C) 2009-2022 D. R. Commander.  All Rights Reserved.
 * Copyright (C) 2015-2017, 2020-2021 Pierre Ossman for Cendio AB.
 *                                    All Rights Reserved.
 * Copyright (C) 2021 AnatoScope SA.  All Rights Reserved.
 * Copyright (C) 2011 Joel Martin
 * Copyright (C) 2010 University Corporation for Atmospheric Research.
 *                    All Rights Reserved.
 * Copyright (C) 2005-2008 Sun Microsystems, Inc.  All Rights Reserved.
 * Copyright (C) 2000-2006 Constantin Kaplinsky.  All Rights Reserved.
 * Copyright (C) 2004 Landmark Graphics Corporation.  All Rights Reserved.
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

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include "windowstr.h"
#include "rfb.h"
#include "sprite.h"

char updateBuf[UPDATE_BUF_SIZE];
int ublen;

rfbClientPtr rfbClientHead = NULL;
/* The client that is currently dragging the pointer
   This serves as a mutex for RFB pointer events. */
rfbClientPtr pointerDragClient = NULL;
/* The client that last moved the pointer
   Other clients will automatically receive cursor updates via the traditional
   mechanism of drawing the cursor into the framebuffer (AKA "server-side
   cursor rendering") so they can track the pointer's movement regardless of
   whether cursor shape updates (AKA "client-side cursor rendering") are
   enabled. */
rfbClientPtr pointerOwner = NULL;

Bool rfbAlwaysShared = FALSE;
Bool rfbNeverShared = FALSE;
Bool rfbDisconnect = FALSE;
Bool rfbViewOnly = FALSE;  /* run server in view only mode - Ehud Karni SW */
Bool rfbCongestionControl = TRUE;
double rfbAutoLosslessRefresh = 0.0;
Bool rfbALRAll = FALSE;
int rfbALRQualityLevel = -1;
int rfbALRSubsampLevel = TVNC_1X;
int rfbCombineRect = 100;
int rfbICEBlockSize = 256;
Bool rfbInterframeDebug = FALSE;
Bool rfbGIIDebug = FALSE;
int rfbMaxWidth = MAXSHORT, rfbMaxHeight = MAXSHORT;
int rfbMaxClipboard = MAX_CUTTEXT_LEN;
Bool rfbVirtualTablet = FALSE;
#if !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined(__OpenBSD__) && !defined(__DragonFly__)
Bool rfbMT = TRUE;
#else
/* Multithreaded Tight encoding segfaults on FreeBSD, for unknown reasons. */
Bool rfbMT = FALSE;
#endif
int rfbNumThreads = 0;

static rfbClientPtr rfbNewClient(int sock);
static void rfbProcessClientProtocolVersion(rfbClientPtr cl);
static void rfbProcessClientInitMessage(rfbClientPtr cl);
static void rfbSendInteractionCaps(rfbClientPtr cl);
static void rfbProcessClientNormalMessage(rfbClientPtr cl);
static Bool rfbSendCopyRegion(rfbClientPtr cl, RegionPtr reg, int dx, int dy);
static Bool rfbSendLastRectMarker(rfbClientPtr cl);
Bool rfbSendDesktopSize(rfbClientPtr cl);
Bool rfbSendExtDesktopSize(rfbClientPtr cl);
static Bool rfbSendQEMUExtKeyEventRect(rfbClientPtr cl);
static Bool rfbSendLEDState(rfbClientPtr cl);


/*
 * Session capture
 */

char *rfbCaptureFile = NULL;

void rfbWriteCapture(int captureFD, char *buf, int len)
{
  if (write(captureFD, buf, len) < len)
    rfbLogPerror("rfbWriteCapture: Could not write to capture file");
}


/*
 * Idle timeout
 */

CARD32 rfbMaxIdleTimeout = 0;
CARD32 rfbIdleTimeout = 0;
static double idleTimeout = -1.0;

void IdleTimerSet(void)
{
  idleTimeout = gettime() + (double)rfbIdleTimeout;
}

static void IdleTimerCancel(void)
{
  idleTimeout = -1.0;
}

void IdleTimerCheck(void)
{
  if (idleTimeout >= 0.0 && gettime() >= idleTimeout)
    FatalError("TurboVNC session has been idle for %u seconds.  Exiting.",
               (unsigned int)rfbIdleTimeout);
}


/*
 * Profiling stuff
 */

BOOL rfbProfile = FALSE;
static double tUpdate = 0., tStart = -1., tElapsed, mpixels = 0.,
  idmpixels = 0.;
static unsigned long updates = 0;
unsigned long long sendBytes = 0;

double gettime(void)
{
  struct timeval __tv;

  gettimeofday(&__tv, (struct timezone *)NULL);
  return (double)__tv.tv_sec + (double)__tv.tv_usec * 0.000001;
}


/*
 * Auto Lossless Refresh
 */

static Bool alrCopyRect = TRUE;

static CARD32 alrCallback(OsTimerPtr timer, CARD32 time, pointer arg)
{
  RegionRec copyRegionSave, modifiedRegionSave, requestedRegionSave,
    ifRegionSave;
  rfbClientPtr cl = (rfbClientPtr)arg;
  int tightCompressLevelSave, tightQualityLevelSave, copyDXSave, copyDYSave,
    tightSubsampLevelSave;
  RegionRec tmpRegion;

  REGION_INIT(pScreen, &tmpRegion, NullBox, 0);
  if (!rfbALRAll && !cl->firstUpdate)
    REGION_INTERSECT(pScreen, &tmpRegion, &cl->alrRegion, &cl->lossyRegion);
  else
    REGION_COPY(pScreen, &tmpRegion, &cl->lossyRegion);
  if (cl->firstUpdate) cl->firstUpdate = FALSE;

  if (REGION_NOTEMPTY(pScreen, &tmpRegion)) {

    tightCompressLevelSave = cl->tightCompressLevel;
    tightQualityLevelSave = cl->tightQualityLevel;
    tightSubsampLevelSave = cl->tightSubsampLevel;
    copyDXSave = cl->copyDX;
    copyDYSave = cl->copyDY;
    REGION_INIT(pScreen, &copyRegionSave, NullBox, 0);
    REGION_COPY(pScreen, &copyRegionSave, &cl->copyRegion);
    REGION_INIT(pScreen, &modifiedRegionSave, NullBox, 0);
    REGION_COPY(pScreen, &modifiedRegionSave, &cl->modifiedRegion);
    REGION_INIT(pScreen, &requestedRegionSave, NullBox, 0);
    REGION_COPY(pScreen, &requestedRegionSave, &cl->requestedRegion);
    REGION_INIT(pScreen, &ifRegionSave, NullBox, 0);
    REGION_COPY(pScreen, &ifRegionSave, &cl->ifRegion);

    cl->tightCompressLevel = 1;
    cl->tightQualityLevel = rfbALRQualityLevel;
    cl->tightSubsampLevel = rfbALRSubsampLevel;
    cl->copyDX = cl->copyDY = 0;
    REGION_EMPTY(pScreen, &cl->copyRegion);
    REGION_EMPTY(pScreen, &cl->modifiedRegion);
    REGION_UNION(pScreen, &cl->modifiedRegion, &cl->modifiedRegion,
                 &tmpRegion);
    REGION_EMPTY(pScreen, &cl->requestedRegion);
    REGION_UNION(pScreen, &cl->requestedRegion, &cl->requestedRegion,
                 &tmpRegion);
    if (cl->compareFB) {
      REGION_EMPTY(pScreen, &cl->ifRegion);
      REGION_UNION(pScreen, &cl->ifRegion, &cl->ifRegion, &tmpRegion);
    }

    cl->inALR = TRUE;
    if (!rfbSendFramebufferUpdate(cl)) return 0;
    cl->inALR = FALSE;

    REGION_EMPTY(pScreen, &cl->lossyRegion);
    REGION_EMPTY(pScreen, &cl->alrRegion);
    cl->tightCompressLevel = tightCompressLevelSave;
    cl->tightQualityLevel = tightQualityLevelSave;
    cl->tightSubsampLevel = tightSubsampLevelSave;
    cl->copyDX = copyDXSave;
    cl->copyDY = copyDYSave;
    REGION_COPY(pScreen, &cl->copyRegion, &copyRegionSave);
    REGION_COPY(pScreen, &cl->modifiedRegion, &modifiedRegionSave);
    REGION_COPY(pScreen, &cl->requestedRegion, &requestedRegionSave);
    REGION_UNINIT(pScreen, &copyRegionSave);
    REGION_UNINIT(pScreen, &modifiedRegionSave);
    REGION_UNINIT(pScreen, &requestedRegionSave);
    if (cl->compareFB) {
      REGION_COPY(pScreen, &cl->ifRegion, &ifRegionSave);
      REGION_UNINIT(pScreen, &ifRegionSave);
    }
  }

  REGION_UNINIT(pScreen, &tmpRegion);
  return 0;
}


/*
 * Interframe comparison
 */

int rfbInterframe = -1;  /* -1 = auto (determined by compression level) */

Bool InterframeOn(rfbClientPtr cl)
{
  if (!cl->compareFB) {
    if (!(cl->compareFB =
          (char *)malloc(rfbFB.paddedWidthInBytes * rfbFB.height))) {
      rfbLogPerror("InterframeOn: couldn't allocate comparison buffer");
      return FALSE;
    }
    memset(cl->compareFB, 0, rfbFB.paddedWidthInBytes * rfbFB.height);
    REGION_INIT(pScreen, &cl->ifRegion, NullBox, 0);
    cl->firstCompare = TRUE;
    rfbLog("Interframe comparison enabled\n");
  }
  cl->fb = cl->compareFB;
  return TRUE;
}

void InterframeOff(rfbClientPtr cl)
{
  if (cl->compareFB) {
    free(cl->compareFB);
    REGION_UNINIT(pScreen, &cl->ifRegion);
    rfbLog("Interframe comparison disabled\n");
  }
  cl->compareFB = NULL;
  cl->fb = rfbFB.pfbMemory;
}


/*
 * Map of quality levels to provide compatibility with TightVNC/TigerVNC
 * clients
 */

static int JPEG_QUAL[10] = {
  15, 29, 41, 42, 62, 77, 79, 86, 92, 100
};

static int JPEG_SUBSAMP[10] = {
  1, 1, 1, 2, 2, 2, 0, 0, 0, 0
};


/*
 * rfbNewClientConnection is called from sockets.c when a new connection
 * comes in.
 */

void rfbNewClientConnection(int sock)
{
  rfbNewClient(sock);
}


/*
 * rfbReverseConnection is called to make an outward connection to a
 * "listening" RFB client.
 */

rfbClientPtr rfbReverseConnection(char *host, int port, int id)
{
  int sock;
  rfbClientPtr cl;

  if (rfbAuthDisableRevCon) {
    rfbLog("Reverse connections disabled\n");
    return (rfbClientPtr)NULL;
  }

  if ((sock = rfbConnect(host, port)) < 0)
    return (rfbClientPtr)NULL;

  if (id > 0) {
    rfbClientRec clTemp;
    char temps[250];
    memset(temps, 0, 250);
    snprintf(temps, 250, "ID:%d", id);
    rfbLog("UltraVNC Repeater Mode II ID is %d\n", id);
    clTemp.sock = sock;
    if (WriteExact(&clTemp, temps, 250) < 0) {
      rfbLogPerror("rfbReverseConnection: write");
      rfbCloseSock(sock);
      return NULL;
    }
  }

  cl = rfbNewClient(sock);

  if (cl)
    cl->reverseConnection = TRUE;

  return cl;
}


/*
 * rfbNewClient is called when a new connection has been made by whatever
 * means.
 */

static rfbClientPtr rfbNewClient(int sock)
{
  rfbProtocolVersionMsg pv;
  rfbClientPtr cl;
  BoxRec box;
  rfbSockAddr addr;
  socklen_t addrlen = sizeof(struct sockaddr_storage);
  char addrStr[INET6_ADDRSTRLEN];
  char *env = NULL;
  int np = sysconf(_SC_NPROCESSORS_CONF);

  if (rfbClientHead == NULL)
    /* no other clients - make sure we don't think any keys are pressed */
    KbdReleaseAllKeys();

  cl = (rfbClientPtr)rfbAlloc0(sizeof(rfbClientRec));

  if (rfbClientHead == NULL && rfbCaptureFile) {
    cl->captureFD = open(rfbCaptureFile, O_CREAT | O_EXCL | O_WRONLY,
                         S_IRUSR | S_IWUSR);
    if (cl->captureFD < 0)
      rfbLogPerror("Could not open capture file");
    else
      rfbLog("Opened capture file %s\n", rfbCaptureFile);
  } else
    cl->captureFD = -1;

  cl->sock = sock;
  getpeername(sock, &addr.u.sa, &addrlen);
  cl->host = strdup(sockaddr_string(&addr, addrStr, INET6_ADDRSTRLEN));

  /* Dispatch client input to rfbProcessClientProtocolVersion(). */
  cl->state = RFB_PROTOCOL_VERSION;

  cl->preferredEncoding = rfbEncodingTight;
  cl->correMaxWidth = 48;
  cl->correMaxHeight = 48;

  REGION_INIT(pScreen, &cl->copyRegion, NullBox, 0);

  box.x1 = box.y1 = 0;
  box.x2 = rfbFB.width;
  box.y2 = rfbFB.height;
  REGION_INIT(pScreen, &cl->modifiedRegion, &box, 0);

  REGION_INIT(pScreen, &cl->requestedRegion, NullBox, 0);

  cl->deferredUpdateStart = gettime();

  cl->format = rfbServerFormat;
  cl->translateFn = rfbTranslateNone;

  cl->tightCompressLevel = TIGHT_DEFAULT_COMPRESSION;
  cl->tightSubsampLevel = TIGHT_DEFAULT_SUBSAMP;
  cl->tightQualityLevel = -1;
  cl->imageQualityLevel = -1;

  cl->ledState = rfbLEDUnknown;

  cl->next = rfbClientHead;
  cl->prev = NULL;
  if (rfbClientHead)
    rfbClientHead->prev = cl;
  rfbClientHead = cl;

  rfbResetStats(cl);

  cl->zlibCompressLevel = 5;

  xorg_list_init(&cl->pings);

  /*
   * Wait a few ms for the client to send WebSockets connection
   * (TLS/SSL or plain)
   */
  if (!webSocketsCheck(cl)) {
    /* Error reporting handled in webSocketsHandshake */
    rfbCloseClient(cl);
    return NULL;
  }

  sprintf(pv, rfbProtocolVersionFormat, 3, 8);

  WRITE_OR_CLOSE(pv, sz_rfbProtocolVersionMsg, return NULL);

  if ((env = getenv("TVNC_PROFILE")) != NULL && !strcmp(env, "1"))
    rfbProfile = TRUE;

  if ((env = getenv("TVNC_ICEDEBUG")) != NULL && !strcmp(env, "1"))
    rfbInterframeDebug = TRUE;

  if ((env = getenv("TVNC_ICEBLOCKSIZE")) != NULL) {
    int iceBlockSize = atoi(env);
    if (iceBlockSize >= 0) rfbICEBlockSize = iceBlockSize;
  }

  if ((env = getenv("TVNC_COMBINERECT")) != NULL) {
    int combine = atoi(env);
    if (combine > 0 && combine <= 65000) rfbCombineRect = combine;
  }

  cl->firstUpdate = TRUE;
  /* The TigerVNC Viewer won't enable remote desktop resize until it receives
     a desktop resize message from the server, so we give it one with the
     first FBU. */
  cl->reason = rfbEDSReasonServer;
  cl->result = rfbEDSResultSuccess;

  if (rfbAutoLosslessRefresh > 0.0) {
    REGION_INIT(pScreen, &cl->lossyRegion, NullBox, 0);
    if ((env = getenv("TVNC_ALRALL")) != NULL && !strcmp(env, "1"))
      rfbALRAll = TRUE;
    if ((env = getenv("TVNC_ALRCOPYRECT")) != NULL && !strcmp(env, "0"))
      alrCopyRect = FALSE;
    REGION_INIT(pScreen, &cl->alrRegion, NullBox, 0);
    REGION_INIT(pScreen, &cl->alrEligibleRegion, NullBox, 0);
  }

  if ((env = getenv("TVNC_MT")) != NULL && !strcmp(env, "0"))
    rfbMT = FALSE;

  if ((env = getenv("TVNC_NTHREADS")) != NULL && strlen(env) >= 1) {
    int temp = atoi(env);
    if (temp >= 1 && temp <= MAX_ENCODING_THREADS)
      rfbNumThreads = temp;
    else
      rfbLog("WARNING: Invalid value of TVNC_NTHREADS (%s) ignored\n", env);
  }

  if (np == -1 && rfbMT) {
    rfbLog("WARNING: Could not determine CPU count.  Multithreaded encoding disabled.\n");
    rfbMT = FALSE;
  }
  if (!rfbMT) rfbNumThreads = 1;
  else if (rfbNumThreads < 1) rfbNumThreads = min(np, 4);
  if (rfbNumThreads > np) {
    rfbLog("NOTICE: Encoding thread count has been clamped to CPU count\n");
    rfbNumThreads = np;
  }

  if (rfbIdleTimeout > 0)
    IdleTimerCancel();

  cl->baseRTT = cl->minRTT = cl->minCongestedRTT = (unsigned)-1;
  REGION_INIT(pScreen, &cl->cuRegion, NullBox, 0);

  if (rfbInterframe == 1) {
    if (!InterframeOn(cl)) {
      rfbCloseClient(cl);
      return NULL;
    }
  } else
    InterframeOff(cl);

  cl->clipFlags = rfbExtClipUTF8 | rfbExtClipRTF | rfbExtClipHTML |
                  rfbExtClipRequest | rfbExtClipNotify | rfbExtClipProvide;
  cl->clipSizes[0] = 20 * 1024 * 1024;

  return cl;
}


/*
 * rfbClientConnectionGone is called from sockets.c just after a connection
 * has gone away.
 */

void rfbClientConnectionGone(rfbClientPtr cl)
{
  int i;
  rfbRTTInfo *rttInfo, *tmp;
  rfbClientPtr client;
  Bool inList = FALSE;

  if (cl->prev)
    cl->prev->next = cl->next;
  else
    rfbClientHead = cl->next;
  if (cl->next)
    cl->next->prev = cl->prev;

  TimerFree(cl->alrTimer);
  TimerFree(cl->deferredUpdateTimer);
  TimerFree(cl->congestionTimer);

#ifdef XVNC_AuthPAM
  rfbPAMEnd(cl);
#endif
  if (cl->login != NULL) {
    rfbLog("Client %s (%s) gone\n", cl->login, cl->host);
    free(cl->login);
  } else {
    rfbLog("Client %s gone\n", cl->host);
  }
  free(cl->host);

  ShutdownTightThreads();

  if (rfbAutoLosslessRefresh > 0.0) {
    REGION_UNINIT(pScreen, &cl->lossyRegion);
    REGION_UNINIT(pScreen, &cl->alrRegion);
    REGION_UNINIT(pScreen, &cl->alrEligibleRegion);
  }

  /* Release the compression state structures if any. */
  if (cl->compStreamInited == TRUE)
    deflateEnd(&(cl->compStream));

  for (i = 0; i < 4; i++) {
    if (cl->zsActive[i])
      deflateEnd(&cl->zsStruct[i]);
  }

  if (pointerDragClient == cl)
    pointerDragClient = NULL;

  if (pointerOwner == cl)
    pointerOwner = NULL;

  REGION_UNINIT(pScreen, &cl->copyRegion);
  REGION_UNINIT(pScreen, &cl->modifiedRegion);

  rfbPrintStats(cl);

  free(cl->translateLookupTable);

  rfbFreeZrleData(cl);

  rfbHandleClipboardAnnounce(cl, FALSE);

  free(cl->clientClipboard);
  xorg_list_for_each_entry(client, &clipboardRequestors, entry) {
    if (client == cl) inList = TRUE;
  }
  if (inList) xorg_list_del(&cl->entry);

  xorg_list_for_each_entry_safe(rttInfo, tmp, &cl->pings, entry) {
    xorg_list_del(&rttInfo->entry);
    free(rttInfo);
  }

  InterframeOff(cl);

  i = cl->numDevices;
  while (i-- > 0)
    RemoveExtInputDevice(cl, 0);

  if (cl->captureFD >= 0)
    close(cl->captureFD);

  free(cl);

  if (rfbClientHead == NULL && rfbIdleTimeout > 0)
    IdleTimerSet();
}


/*
 * rfbProcessClientMessage is called when there is data to read from a client.
 */

void rfbProcessClientMessage(rfbClientPtr cl)
{
  rfbCorkSock(cl->sock);

  if (cl->pendingSyncFence) {
    cl->syncFence = TRUE;
    cl->pendingSyncFence = FALSE;
  }

  switch (cl->state) {
    case RFB_PROTOCOL_VERSION:
      rfbProcessClientProtocolVersion(cl);
      break;
    case RFB_SECURITY_TYPE:     /* protocol versions 3.7 and above */
      rfbProcessClientSecurityType(cl);
      break;
    case RFB_TUNNELING_TYPE:    /* protocol versions 3.7t, 3.8t */
      rfbProcessClientTunnelingType(cl);
      break;
    case RFB_AUTH_TYPE:         /* protocol versions 3.7t, 3.8t */
      rfbProcessClientAuthType(cl);
      break;
#if USETLS
    case RFB_TLS_HANDSHAKE:
      rfbAuthTLSHandshake(cl);
      break;
#endif
    case RFB_AUTHENTICATION:
      rfbAuthProcessResponse(cl);
      break;
    case RFB_INITIALISATION:
      cl->ledState = rfbLEDState;
      rfbInitFlowControl(cl);
      rfbProcessClientInitMessage(cl);
      break;
    default:
      rfbProcessClientNormalMessage(cl);
  }

  CHECK_CLIENT_PTR(cl, return)

  if (cl->syncFence) {
    if (!rfbSendFence(cl, cl->fenceFlags, cl->fenceDataLen, cl->fenceData))
      return;
    cl->syncFence = FALSE;
  }

  rfbUncorkSock(cl->sock);
}


/*
 * rfbProcessClientProtocolVersion is called when the client sends its
 * protocol version.
 */

static void rfbProcessClientProtocolVersion(rfbClientPtr cl)
{
  rfbProtocolVersionMsg pv;
  int n, major, minor;

  if ((n = ReadExact(cl, pv, sz_rfbProtocolVersionMsg)) <= 0) {
    if (n == 0)
      rfbLog("rfbProcessClientProtocolVersion: client gone\n");
    else
      rfbLogPerror("rfbProcessClientProtocolVersion: read");
    rfbCloseClient(cl);
    return;
  }

  pv[sz_rfbProtocolVersionMsg] = 0;
  if (sscanf(pv, rfbProtocolVersionFormat, &major, &minor) != 2) {
    rfbLog("rfbProcessClientProtocolVersion: not a valid RFB client\n");
    rfbCloseClient(cl);
    return;
  }
  if (major != 3) {
    rfbLog("Unsupported protocol version %d.%d\n", major, minor);
    rfbCloseClient(cl);
    return;
  }

  /* Always use one of the three standard versions of the RFB protocol. */
  cl->protocol_minor_ver = minor;

  if (minor > 8)                        /* buggy client */
    cl->protocol_minor_ver = 8;
  else if (minor > 3 && minor < 7)      /* non-standard client */
    cl->protocol_minor_ver = 3;
  else if (minor < 3)                   /* ancient client */
    cl->protocol_minor_ver = 3;

  if (cl->protocol_minor_ver != minor)
    rfbLog("Non-standard protocol version 3.%d, using 3.%d instead\n", minor,
           cl->protocol_minor_ver);
  else
    rfbLog("Using protocol version 3.%d\n", cl->protocol_minor_ver);

  /* TightVNC protocol extensions are not enabled yet. */
  cl->protocol_tightvnc = FALSE;

  rfbAuthNewClient(cl);
}


/*
 * rfbProcessClientInitMessage is called when the client sends its
 * initialisation message.
 */

static void rfbProcessClientInitMessage(rfbClientPtr cl)
{
  rfbClientInitMsg ci;
  char buf[256];
  rfbServerInitMsg *si = (rfbServerInitMsg *)buf;
  int len, n;
  rfbClientPtr otherCl, nextCl;

  if ((n = ReadExact(cl, (char *)&ci, sz_rfbClientInitMsg)) <= 0) {
    if (n == 0)
      rfbLog("rfbProcessClientInitMessage: client gone\n");
    else
      rfbLogPerror("rfbProcessClientInitMessage: read");
    rfbCloseClient(cl);
    return;
  }

  si->framebufferWidth = Swap16IfLE(rfbFB.width);
  si->framebufferHeight = Swap16IfLE(rfbFB.height);
  si->format = rfbServerFormat;
  si->format.redMax = Swap16IfLE(si->format.redMax);
  si->format.greenMax = Swap16IfLE(si->format.greenMax);
  si->format.blueMax = Swap16IfLE(si->format.blueMax);

  if (strlen(desktopName) > 128)        /* sanity check on desktop name len */
    desktopName[128] = 0;

  sprintf(buf + sz_rfbServerInitMsg, "%s", desktopName);

  len = strlen(buf + sz_rfbServerInitMsg);
  si->nameLength = Swap32IfLE(len);

  WRITE_OR_CLOSE(buf, sz_rfbServerInitMsg + len, return);

  if (cl->protocol_tightvnc)
    rfbSendInteractionCaps(cl);  /* protocol 3.7t */

  /* Dispatch client input to rfbProcessClientNormalMessage(). */
  cl->state = RFB_NORMAL;

  if (!cl->reverseConnection &&
      (rfbNeverShared || (!rfbAlwaysShared && !ci.shared))) {

    if (!rfbDisconnect) {
      for (otherCl = rfbClientHead; otherCl; otherCl = otherCl->next) {
        if ((otherCl != cl) && (otherCl->state == RFB_NORMAL)) {
          rfbLog("-dontdisconnect: Not shared & existing client\n");
          rfbLog("  refusing new client %s\n", cl->host);
          rfbCloseClient(cl);
          return;
        }
      }
    } else {
      for (otherCl = rfbClientHead; otherCl; otherCl = nextCl) {
        nextCl = otherCl->next;
        if ((otherCl != cl) && (otherCl->state == RFB_NORMAL)) {
          rfbLog("Not shared - closing connection to client %s\n",
                 otherCl->host);
          rfbCloseClient(otherCl);
        }
      }
    }
  }
}


/*
 * rfbSendInteractionCaps is called after sending the server
 * initialisation message, only if TightVNC protocol extensions were
 * enabled (protocol versions 3.7t, 3.8t). In this function, we send
 * the lists of supported protocol messages and encodings.
 */

/* Update these constants on changing capability lists below! */
#define N_SMSG_CAPS  0
#define N_CMSG_CAPS  0
#define N_ENC_CAPS  17

void rfbSendInteractionCaps(rfbClientPtr cl)
{
  rfbInteractionCapsMsg intr_caps;
  rfbCapabilityInfo enc_list[N_ENC_CAPS];
  int i;

  /* Fill in the header structure sent prior to capability lists. */
  intr_caps.nServerMessageTypes = Swap16IfLE(N_SMSG_CAPS);
  intr_caps.nClientMessageTypes = Swap16IfLE(N_CMSG_CAPS);
  intr_caps.nEncodingTypes = Swap16IfLE(N_ENC_CAPS);
  intr_caps.pad = 0;

  /* Supported server->client message types. */
  /* For future file transfer support:
  i = 0;
  SetCapInfo(&smsg_list[i++], rfbFileListData,           rfbTightVncVendor);
  SetCapInfo(&smsg_list[i++], rfbFileDownloadData,       rfbTightVncVendor);
  SetCapInfo(&smsg_list[i++], rfbFileUploadCancel,       rfbTightVncVendor);
  SetCapInfo(&smsg_list[i++], rfbFileDownloadFailed,     rfbTightVncVendor);
  if (i != N_SMSG_CAPS) {
    rfbLog("rfbSendInteractionCaps: assertion failed, i != N_SMSG_CAPS\n");
    rfbCloseClient(cl);
    return;
  }
  */

  /* Supported client->server message types. */
  /* For future file transfer support:
  i = 0;
  SetCapInfo(&cmsg_list[i++], rfbFileListRequest,        rfbTightVncVendor);
  SetCapInfo(&cmsg_list[i++], rfbFileDownloadRequest,    rfbTightVncVendor);
  SetCapInfo(&cmsg_list[i++], rfbFileUploadRequest,      rfbTightVncVendor);
  SetCapInfo(&cmsg_list[i++], rfbFileUploadData,         rfbTightVncVendor);
  SetCapInfo(&cmsg_list[i++], rfbFileDownloadCancel,     rfbTightVncVendor);
  SetCapInfo(&cmsg_list[i++], rfbFileUploadFailed,       rfbTightVncVendor);
  if (i != N_CMSG_CAPS) {
    rfbLog("rfbSendInteractionCaps: assertion failed, i != N_CMSG_CAPS\n");
    rfbCloseClient(cl);
    return;
  }
  */

  /* Encoding types. */
  i = 0;
  SetCapInfo(&enc_list[i++],  rfbEncodingCopyRect,       rfbStandardVendor);
  SetCapInfo(&enc_list[i++],  rfbEncodingRRE,            rfbStandardVendor);
  SetCapInfo(&enc_list[i++],  rfbEncodingCoRRE,          rfbStandardVendor);
  SetCapInfo(&enc_list[i++],  rfbEncodingHextile,        rfbStandardVendor);
  SetCapInfo(&enc_list[i++],  rfbEncodingZlib,           rfbTridiaVncVendor);
  SetCapInfo(&enc_list[i++],  rfbEncodingZRLE,           rfbTridiaVncVendor);
  SetCapInfo(&enc_list[i++],  rfbEncodingZYWRLE,         rfbTridiaVncVendor);
  SetCapInfo(&enc_list[i++],  rfbEncodingTight,          rfbTightVncVendor);
  SetCapInfo(&enc_list[i++],  rfbEncodingCompressLevel0, rfbTightVncVendor);
  SetCapInfo(&enc_list[i++],  rfbEncodingQualityLevel0,  rfbTightVncVendor);
  SetCapInfo(&enc_list[i++],  rfbEncodingFineQualityLevel0, rfbTurboVncVendor);
  SetCapInfo(&enc_list[i++],  rfbEncodingSubsamp1X,         rfbTurboVncVendor);
  SetCapInfo(&enc_list[i++],  rfbEncodingXCursor,        rfbTightVncVendor);
  SetCapInfo(&enc_list[i++],  rfbEncodingRichCursor,     rfbTightVncVendor);
  SetCapInfo(&enc_list[i++],  rfbEncodingPointerPos,     rfbTightVncVendor);
  SetCapInfo(&enc_list[i++],  rfbEncodingLastRect,       rfbTightVncVendor);
  SetCapInfo(&enc_list[i++],  rfbGIIServer,              rfbGIIVendor);
  if (i != N_ENC_CAPS) {
    rfbLog("rfbSendInteractionCaps: assertion failed, i != N_ENC_CAPS\n");
    rfbCloseClient(cl);
    return;
  }

  /* Send header and capability lists */
  WRITE_OR_CLOSE((char *)&intr_caps, sz_rfbInteractionCapsMsg, return);
  WRITE_OR_CLOSE((char *)&enc_list[0], sz_rfbCapabilityInfo * N_ENC_CAPS,
                 return);

  /* Dispatch client input to rfbProcessClientNormalMessage(). */
  cl->state = RFB_NORMAL;
}


/*
 * rfbProcessClientNormalMessage is called when the client has sent a normal
 * protocol message.
 */

static void rfbProcessClientNormalMessage(rfbClientPtr cl)
{
  rfbClientToServerMsg msg;
  char *str;

  READ_OR_CLOSE((char *)&msg, 1, return);

  switch (msg.type) {

    case rfbSetPixelFormat:

      READ_OR_CLOSE(((char *)&msg) + 1, sz_rfbSetPixelFormatMsg - 1, return);

      cl->format.bitsPerPixel = msg.spf.format.bitsPerPixel;
      cl->format.depth = msg.spf.format.depth;
      cl->format.bigEndian = (msg.spf.format.bigEndian ? 1 : 0);
      cl->format.trueColour = (msg.spf.format.trueColour ? 1 : 0);
      cl->format.redMax = Swap16IfLE(msg.spf.format.redMax);
      cl->format.greenMax = Swap16IfLE(msg.spf.format.greenMax);
      cl->format.blueMax = Swap16IfLE(msg.spf.format.blueMax);
      cl->format.redShift = msg.spf.format.redShift;
      cl->format.greenShift = msg.spf.format.greenShift;
      cl->format.blueShift = msg.spf.format.blueShift;

      cl->readyForSetColourMapEntries = TRUE;

      rfbSetTranslateFunction(cl);
      return;

    case rfbFixColourMapEntries:
      READ_OR_CLOSE(((char *)&msg) + 1, sz_rfbFixColourMapEntriesMsg - 1,
                    return);
      rfbLog("rfbProcessClientNormalMessage: FixColourMapEntries unsupported\n");
      rfbCloseClient(cl);
      return;

    case rfbSetEncodings:
    {
      int i;
      CARD32 enc;
      Bool firstExtClipboard = !cl->enableExtClipboard;
      Bool firstFence = !cl->enableFence;
      Bool firstCU = !cl->enableCU;
      Bool firstGII = !cl->enableGII;
      Bool firstQEMUExtKeyEvent = !cl->enableQEMUExtKeyEvent;
      Bool firstLEDState = !SUPPORTS_LED_STATE(cl);
      Bool logTightCompressLevel = FALSE;

      READ_OR_CLOSE(((char *)&msg) + 1, sz_rfbSetEncodingsMsg - 1, return);

      msg.se.nEncodings = Swap16IfLE(msg.se.nEncodings);

      cl->preferredEncoding = -1;
      cl->useCopyRect = FALSE;
      cl->enableCursorShapeUpdates = FALSE;
      cl->enableCursorPosUpdates = FALSE;
      cl->enableLastRectEncoding = FALSE;
      cl->tightCompressLevel = TIGHT_DEFAULT_COMPRESSION;
      cl->tightSubsampLevel = TIGHT_DEFAULT_SUBSAMP;
      cl->tightQualityLevel = -1;
      cl->imageQualityLevel = -1;

      for (i = 0; i < msg.se.nEncodings; i++) {
        READ32_OR_CLOSE(enc, return);

        switch (enc) {

          case rfbEncodingCopyRect:
            cl->useCopyRect = TRUE;
            break;
          case rfbEncodingRaw:
            if (cl->preferredEncoding == -1) {
              cl->preferredEncoding = enc;
              rfbLog("Using raw encoding for client %s\n", cl->host);
            }
            break;
          case rfbEncodingRRE:
            if (cl->preferredEncoding == -1) {
              cl->preferredEncoding = enc;
              rfbLog("Using rre encoding for client %s\n", cl->host);
            }
            break;
          case rfbEncodingCoRRE:
            if (cl->preferredEncoding == -1) {
              cl->preferredEncoding = enc;
              rfbLog("Using CoRRE encoding for client %s\n", cl->host);
            }
            break;
          case rfbEncodingHextile:
            if (cl->preferredEncoding == -1) {
              cl->preferredEncoding = enc;
              rfbLog("Using hextile encoding for client %s\n", cl->host);
            }
            break;
          case rfbEncodingZlib:
            if (cl->preferredEncoding == -1) {
              cl->preferredEncoding = enc;
              rfbLog("Using zlib encoding for client %s\n", cl->host);
            }
            break;
          case rfbEncodingZRLE:
            if (cl->preferredEncoding == -1) {
              cl->preferredEncoding = enc;
              rfbLog("Using ZRLE encoding for client %s\n", cl->host);
            }
            break;
          case rfbEncodingZYWRLE:
            if (cl->preferredEncoding == -1) {
              cl->preferredEncoding = enc;
              rfbLog("Using ZYWRLE encoding for client %s\n", cl->host);
            }
            break;
          case rfbEncodingTight:
            if (cl->preferredEncoding == -1) {
              cl->preferredEncoding = enc;
              rfbLog("Using tight encoding for client %s\n", cl->host);
            }
            break;
          case rfbEncodingXCursor:
            if (!cl->enableCursorShapeUpdates) {
              rfbLog("Enabling X-style cursor updates for client %s\n",
                     cl->host);
              cl->enableCursorShapeUpdates = TRUE;
              cl->useRichCursorEncoding = FALSE;
              cl->cursorWasChanged = TRUE;
            }
            break;
          case rfbEncodingRichCursor:
            if (!cl->enableCursorShapeUpdates) {
              rfbLog("Enabling full-color cursor updates for client %s\n",
                     cl->host);
              cl->enableCursorShapeUpdates = TRUE;
              cl->useRichCursorEncoding = TRUE;
              cl->cursorWasChanged = TRUE;
            }
            break;
          case rfbEncodingPointerPos:
            if (!cl->enableCursorPosUpdates) {
              rfbLog("Enabling cursor position updates for client %s\n",
                     cl->host);
              cl->enableCursorPosUpdates = TRUE;
              cl->cursorWasMoved = TRUE;
              cl->cursorX = -1;
              cl->cursorY = -1;
            }
            break;
          case rfbEncodingLastRect:
            if (!cl->enableLastRectEncoding) {
              rfbLog("Enabling LastRect protocol extension for client %s\n",
                     cl->host);
              cl->enableLastRectEncoding = TRUE;
            }
            break;
          case rfbEncodingExtendedClipboard:
           if (!cl->enableExtClipboard) {
              rfbLog("Enabling Extended Clipboard protocol extension for client %s\n",
                     cl->host);
              cl->enableExtClipboard = TRUE;
            }
            break;
          case rfbEncodingFence:
            if (!cl->enableFence) {
              rfbLog("Enabling Fence protocol extension for client %s\n",
                     cl->host);
              cl->enableFence = TRUE;
            }
            break;
          case rfbEncodingContinuousUpdates:
            if (!cl->enableCU) {
              rfbLog("Enabling Continuous Updates protocol extension for client %s\n",
                     cl->host);
              cl->enableCU = TRUE;
            }
            break;
          case rfbEncodingNewFBSize:
            if (!cl->enableDesktopSize) {
              if (!rfbAuthDisableRemoteResize) {
                rfbLog("Enabling Desktop Size protocol extension for client %s\n",
                       cl->host);
                cl->enableDesktopSize = TRUE;
              } else
                rfbLog("WARNING: Remote desktop resizing disabled per system policy.\n");
            }
            break;
          case rfbEncodingExtendedDesktopSize:
            if (!cl->enableExtDesktopSize) {
              if (!rfbAuthDisableRemoteResize) {
                rfbLog("Enabling Extended Desktop Size protocol extension for client %s\n",
                       cl->host);
                cl->enableExtDesktopSize = TRUE;
              } else
                rfbLog("WARNING: Remote desktop resizing disabled per system policy.\n");
            }
            break;
          case rfbEncodingGII:
            if (!cl->enableGII) {
              rfbLog("Enabling GII protocol extension for client %s\n", cl->host);
              cl->enableGII = TRUE;
            }
            break;
          case rfbEncodingQEMUExtendedKeyEvent:
            if (!cl->enableQEMUExtKeyEvent && enableQEMUExtKeyEvent) {
              rfbLog("Enabling QEMU Extended Key Event protocol extension for client %s\n", cl->host);
              cl->enableQEMUExtKeyEvent = TRUE;
            }
            break;
          case rfbEncodingQEMULEDState:
            if (!cl->enableQEMULEDState && enableQEMUExtKeyEvent) {
              rfbLog("Enabling QEMU LED State protocol extension for client %s\n", cl->host);
              cl->enableQEMULEDState = TRUE;
            }
            break;
          case rfbEncodingVMwareLEDState:
            if (!cl->enableVMwareLEDState && enableQEMUExtKeyEvent) {
              rfbLog("Enabling VMware LED State protocol extension for client %s\n", cl->host);
              cl->enableVMwareLEDState = TRUE;
            }
            break;
          default:
            if (enc >= (CARD32)rfbEncodingCompressLevel0 &&
                enc <= (CARD32)rfbEncodingCompressLevel9) {
              cl->zlibCompressLevel = enc & 0x0F;
              cl->tightCompressLevel = enc & 0x0F;
              if (cl->preferredEncoding == rfbEncodingTight)
                logTightCompressLevel = TRUE;
              else
                rfbLog("Using compression level %d for client %s\n",
                       cl->tightCompressLevel, cl->host);
              if (rfbInterframe == -1) {
                if (cl->tightCompressLevel >= 5) {
                  if (!InterframeOn(cl)) {
                    rfbCloseClient(cl);
                    return;
                  }
                } else
                  InterframeOff(cl);
              }
            } else if (enc >= (CARD32)rfbEncodingSubsamp1X &&
                       enc <= (CARD32)rfbEncodingSubsampGray) {
              cl->tightSubsampLevel = enc & 0xFF;
              rfbLog("Using JPEG subsampling %d for client %s\n",
                     cl->tightSubsampLevel, cl->host);
            } else if (enc >= (CARD32)rfbEncodingQualityLevel0 &&
                       enc <= (CARD32)rfbEncodingQualityLevel9) {
              cl->tightQualityLevel = JPEG_QUAL[enc & 0x0F];
              cl->tightSubsampLevel = JPEG_SUBSAMP[enc & 0x0F];
              cl->imageQualityLevel = enc & 0x0F;
              if (cl->preferredEncoding == rfbEncodingTight)
                rfbLog("Using JPEG subsampling %d, Q%d for client %s\n",
                       cl->tightSubsampLevel, cl->tightQualityLevel, cl->host);
              else
                rfbLog("Using image quality level %d for client %s\n",
                       cl->imageQualityLevel, cl->host);
            } else if (enc >= (CARD32)rfbEncodingFineQualityLevel0 + 1 &&
                       enc <= (CARD32)rfbEncodingFineQualityLevel100) {
              cl->tightQualityLevel = enc & 0xFF;
              rfbLog("Using JPEG quality %d for client %s\n",
                     cl->tightQualityLevel, cl->host);
            } else {
              rfbLog("rfbProcessClientNormalMessage: ignoring unknown encoding %d (%x)\n",
                     (int)enc, (int)enc);
            }
        }  /* switch (enc) */
      }  /* for (i = 0; i < msg.se.nEncodings; i++) */

      if (cl->preferredEncoding == -1)
        cl->preferredEncoding = rfbEncodingTight;

      if (cl->preferredEncoding == rfbEncodingTight && logTightCompressLevel)
        rfbLog("Using Tight compression level %d for client %s\n",
               rfbTightCompressLevel(cl), cl->host);

      if (cl->enableCursorPosUpdates && !cl->enableCursorShapeUpdates) {
        rfbLog("Disabling cursor position updates for client %s\n", cl->host);
        cl->enableCursorPosUpdates = FALSE;
      }

      if (cl->enableExtClipboard && firstExtClipboard) {
        CARD32 sizes[] = { 0 };

        rfbSendClipboardCaps(cl, rfbExtClipUTF8 | rfbExtClipRequest |
                             rfbExtClipPeek | rfbExtClipNotify |
                             rfbExtClipProvide, sizes);
      }

      if (cl->enableFence && firstFence) {
        char type = 0;
        if (!rfbSendFence(cl, rfbFenceFlagRequest, sizeof(type), &type))
          return;
      }

      if (cl->enableCU && cl->enableFence && firstCU) {
        if (!rfbSendEndOfCU(cl))
          return;
      }

      if (cl->enableGII && firstGII) {
        /* Send GII server version message to all clients */
        rfbGIIServerVersionMsg svmsg;

        svmsg.type = rfbGIIServer;
        /* We always send as big endian to make things easier on the Java
           viewer. */
        svmsg.endianAndSubType = rfbGIIVersion | rfbGIIBE;
        svmsg.length = Swap16IfLE(sz_rfbGIIServerVersionMsg - 4);
        svmsg.maximumVersion = svmsg.minimumVersion = Swap16IfLE(1);

        WRITE_OR_CLOSE(&svmsg, sz_rfbGIIServerVersionMsg, return);
      }

      if (cl->enableQEMUExtKeyEvent && firstQEMUExtKeyEvent) {
        if (!SUPPORTS_LED_STATE(cl)) {
          rfbLog("WARNING: Disabling QEMU Extended Key Event extension because neither LED state\n");
          rfbLog("  extension is supported by the client.\n");
          cl->enableQEMUExtKeyEvent = FALSE;
        } else
          cl->pendingQEMUExtKeyEventRect = TRUE;
      }

      if (SUPPORTS_LED_STATE(cl) && firstLEDState &&
          cl->ledState != rfbLEDUnknown) {
        if (!cl->enableQEMUExtKeyEvent) {
          rfbLog("WARNING: Disabling LED state extensions because the QEMU Extended Key Event\n");
          rfbLog("  extension is not supported by the client.\n");
          cl->enableQEMULEDState = cl->enableVMwareLEDState = FALSE;
        } else
          cl->pendingLEDState = TRUE;
      }

      return;
    }  /* rfbSetEncodings */

    case rfbFramebufferUpdateRequest:
    {
      RegionRec tmpRegion;
      BoxRec box;

      READ_OR_CLOSE(((char *)&msg) + 1, sz_rfbFramebufferUpdateRequestMsg - 1,
                    return);

      box.x1 = Swap16IfLE(msg.fur.x);
      box.y1 = Swap16IfLE(msg.fur.y);
      box.x2 = box.x1 + Swap16IfLE(msg.fur.w);
      box.y2 = box.y1 + Swap16IfLE(msg.fur.h);
      SAFE_REGION_INIT(pScreen, &tmpRegion, &box, 0);

      if (!msg.fur.incremental || !cl->continuousUpdates)
        REGION_UNION(pScreen, &cl->requestedRegion, &cl->requestedRegion,
                     &tmpRegion);

      if (!cl->readyForSetColourMapEntries) {
        /* client hasn't sent a SetPixelFormat so is using server's */
        cl->readyForSetColourMapEntries = TRUE;
        if (!cl->format.trueColour) {
          if (!rfbSetClientColourMap(cl, 0, 0)) {
            REGION_UNINIT(pScreen, &tmpRegion);
            return;
          }
        }
      }

      if (!msg.fur.incremental) {
        REGION_UNION(pScreen, &cl->modifiedRegion, &cl->modifiedRegion,
                     &tmpRegion);
        REGION_SUBTRACT(pScreen, &cl->copyRegion, &cl->copyRegion, &tmpRegion);
        REGION_UNION(pScreen, &cl->ifRegion, &cl->ifRegion, &tmpRegion);
        cl->pendingExtDesktopResize = TRUE;
      }

      if (FB_UPDATE_PENDING(cl) &&
          (!cl->deferredUpdateScheduled || rfbDeferUpdateTime == 0 ||
           gettime() - cl->deferredUpdateStart >=
           (double)rfbDeferUpdateTime)) {
        if (rfbSendFramebufferUpdate(cl))
          cl->deferredUpdateScheduled = FALSE;
      }

      REGION_UNINIT(pScreen, &tmpRegion);
      return;
    }

    case rfbKeyEvent:

      cl->rfbKeyEventsRcvd++;

      READ_OR_CLOSE(((char *)&msg) + 1, sz_rfbKeyEventMsg - 1, return);

      if (!rfbViewOnly && !cl->viewOnly)
        KeyEvent((KeySym)Swap32IfLE(msg.ke.key), msg.ke.down);

      return;

    case rfbPointerEvent:

      cl->rfbPointerEventsRcvd++;

      READ_OR_CLOSE(((char *)&msg) + 1, sz_rfbPointerEventMsg - 1, return);

      if (pointerDragClient && (pointerDragClient != cl))
        return;

      if (msg.pe.buttonMask == 0)
        pointerDragClient = NULL;
      else
        pointerDragClient = cl;

      if (!rfbViewOnly && !cl->viewOnly) {
        cl->cursorX = (int)Swap16IfLE(msg.pe.x);
        cl->cursorY = (int)Swap16IfLE(msg.pe.y);

        /* If the pointer was most recently moved by another client, we set
           pointerOwner to NULL here so that the client that is currently
           moving the pointer (cl), assuming it understands cursor shape
           updates, will receive a cursor shape update with the last known
           pointer position. */
        if (pointerOwner != cl)
          pointerOwner = NULL;

        PtrAddEvent(msg.pe.buttonMask, cl->cursorX, cl->cursorY, cl);

        pointerOwner = cl;
      }
      return;

    case rfbClientCutText:
    {
      int ignoredBytes = 0;

      READ_OR_CLOSE(((char *)&msg) + 1, sz_rfbClientCutTextMsg - 1, return);

      msg.cct.length = Swap32IfLE(msg.cct.length);

      if (cl->enableExtClipboard && msg.cct.length & 0x80000000) {
        int len = -msg.cct.length;
        rfbReadExtClipboard(cl, len);
        return;
      }

      if (msg.cct.length > rfbMaxClipboard) {
        rfbLog("Truncating %d-byte incoming clipboard update to %d bytes.\n",
               msg.cct.length, rfbMaxClipboard);
        ignoredBytes = msg.cct.length - rfbMaxClipboard;
        msg.cct.length = rfbMaxClipboard;
      }

      if (msg.cct.length <= 0) return;
      str = (char *)rfbAlloc(msg.cct.length);

      READ_OR_CLOSE(str, msg.cct.length, free(str);  return);

      if (ignoredBytes > 0)
        SKIP_OR_CLOSE(ignoredBytes, free(str);  return);

      /* NOTE: We do not accept cut text from a view-only client */
      if (!rfbViewOnly && !cl->viewOnly && !rfbAuthDisableCBRecv)
        rfbHandleClientCutText(cl, str, msg.cct.length);

      free(str);
      return;
    }

    case rfbEnableContinuousUpdates:
    {
      BoxRec box;

      READ_OR_CLOSE(((char *)&msg) + 1, sz_rfbEnableContinuousUpdatesMsg - 1,
                    return);

      if (!cl->enableFence || !cl->enableCU) {
        rfbLog("Ignoring request to enable continuous updates because the client does not\n");
        rfbLog("support the flow control extensions.\n");
        return;
      }

      box.x1 = Swap16IfLE(msg.ecu.x);
      box.y1 = Swap16IfLE(msg.ecu.y);
      box.x2 = box.x1 + Swap16IfLE(msg.ecu.w);
      box.y2 = box.y1 + Swap16IfLE(msg.ecu.h);
      SAFE_REGION_INIT(pScreen, &cl->cuRegion, &box, 0);

      cl->continuousUpdates = msg.ecu.enable;
      if (cl->continuousUpdates) {
        REGION_EMPTY(pScreen, &cl->requestedRegion);
        if (!rfbSendFramebufferUpdate(cl))
          return;
      } else {
        if (!rfbSendEndOfCU(cl))
          return;
      }

      rfbLog("Continuous updates %s\n",
             cl->continuousUpdates ? "enabled" : "disabled");
      return;
    }

    case rfbFence:
    {
      CARD32 flags;
      char data[64];

      READ_OR_CLOSE(((char *)&msg) + 1, sz_rfbFenceMsg - 1, return);

      flags = Swap32IfLE(msg.f.flags);

      if (msg.f.length > sizeof(data)) {
        rfbLog("Ignoring fence.  Payload of %d bytes is too large.\n",
               msg.f.length);
        SKIP_OR_CLOSE(msg.f.length, return)
      } else {
        READ_OR_CLOSE(data, msg.f.length, return);
        HandleFence(cl, flags, msg.f.length, data);
      }

      return;
    }

    #define EDSERROR(format, args...) {  \
      if (!strlen(errMsg))  \
        snprintf(errMsg, 256, "Desktop resize ERROR: "format"\n", args);  \
      result = rfbEDSResultInvalid;  \
    }

    case rfbSetDesktopSize:
    {
      int i;
      struct xorg_list newScreens;
      rfbClientPtr cl2;
      int result = rfbEDSResultSuccess;
      char errMsg[256] = "\0";
      ScreenPtr pScreen = screenInfo.screens[0];

      READ_OR_CLOSE(((char *)&msg) + 1, sz_rfbSetDesktopSizeMsg - 1, return);

      if (msg.sds.numScreens < 1)
        EDSERROR("Requested number of screens %d is invalid",
                 msg.sds.numScreens);

      msg.sds.w = Swap16IfLE(msg.sds.w);
      msg.sds.h = Swap16IfLE(msg.sds.h);
      if (msg.sds.w < 1 || msg.sds.h < 1)
        EDSERROR("Requested framebuffer dimensions %dx%d are invalid",
                 msg.sds.w, msg.sds.h);

      xorg_list_init(&newScreens);
      for (i = 0; i < msg.sds.numScreens; i++) {
        rfbScreenInfo *screen = rfbNewScreen(0, 0, 0, 0, 0, 0);

        READ_OR_CLOSE((char *)&screen->s, sizeof(rfbScreenDesc), return);
        screen->s.id = Swap32IfLE(screen->s.id);
        screen->s.x = Swap16IfLE(screen->s.x);
        screen->s.y = Swap16IfLE(screen->s.y);
        screen->s.w = Swap16IfLE(screen->s.w);
        screen->s.h = Swap16IfLE(screen->s.h);
        screen->s.flags = Swap32IfLE(screen->s.flags);
        if (screen->s.w < 1 || screen->s.h < 1)
          EDSERROR("Screen 0x%.8x requested dimensions %dx%d are invalid",
                   (unsigned int)screen->s.id, screen->s.w, screen->s.h);
        if (screen->s.x >= msg.sds.w || screen->s.y >= msg.sds.h ||
            screen->s.x + screen->s.w > msg.sds.w ||
            screen->s.y + screen->s.h > msg.sds.h)
          EDSERROR("Screen 0x%.8x requested geometry %dx%d+%d+%d exceeds requested framebuffer dimensions",
                   (unsigned int)screen->s.id, screen->s.w, screen->s.h,
                   screen->s.x, screen->s.y);
        if (rfbFindScreenID(&newScreens, screen->s.id)) {
          EDSERROR("Screen 0x%.8x duplicate ID", (unsigned int)screen->s.id);
          free(screen);
        } else
          rfbAddScreen(&newScreens, screen);
      }

      if (cl->viewOnly) {
        rfbLog("NOTICE: Ignoring remote desktop resize request from a view-only client.\n");
        result = rfbEDSResultProhibited;
      } else if (result == rfbEDSResultSuccess) {
        result = ResizeDesktop(pScreen, cl, msg.sds.w, msg.sds.h, &newScreens);
        if (result == rfbEDSResultSuccess)
          return;
      } else
        rfbLog(errMsg);

      rfbRemoveScreens(&newScreens);

      /* Send back the error only to the requesting client.  This loop is
         necessary because the client may have been shut down as a result of
         an error in ResizeDesktop(). */
      for (cl2 = rfbClientHead; cl2; cl2 = cl2->next) {
        if (cl2 == cl) {
          cl2->pendingExtDesktopResize = TRUE;
          cl2->reason = rfbEDSReasonClient;
          cl2->result = result;
          rfbSendFramebufferUpdate(cl2);
          break;
        }
      }

      return;
    }

    case rfbGIIClient:
    {
      CARD8 endianAndSubType, littleEndian, subType;

      READ_OR_CLOSE((char *)&endianAndSubType, 1, return);
      littleEndian = (endianAndSubType & rfbGIIBE) ? 0 : 1;
      subType = endianAndSubType & ~rfbGIIBE;

      switch (subType) {

        case rfbGIIVersion:

          READ_OR_CLOSE((char *)&msg.giicv.length,
                        sz_rfbGIIClientVersionMsg - 2, return);
          if (littleEndian != *(const char *)&rfbEndianTest) {
            msg.giicv.length = Swap16(msg.giicv.length);
            msg.giicv.version = Swap16(msg.giicv.version);
          }
          if (msg.giicv.length != sz_rfbGIIClientVersionMsg - 4 ||
              msg.giicv.version < 1) {
            rfbLog("ERROR: Malformed GII client version message\n");
            rfbCloseClient(cl);
            return;
          }
          rfbLog("Client supports GII version %d\n", msg.giicv.version);
          break;

        case rfbGIIDeviceCreate:
        {
          int i, t;
          rfbDevInfo dev;
          rfbGIIDeviceCreatedMsg dcmsg;

          memset(&dev, 0, sizeof(dev));
          for (t = 0; t < UVNCGII_MAX_TOUCHES; t++)
            dev.active_touches_uvnc[t][2] = -1;
          dcmsg.deviceOrigin = 0;

          READ_OR_CLOSE((char *)&msg.giidc.length,
                        sz_rfbGIIDeviceCreateMsg - 2, return);
          if (littleEndian != *(const char *)&rfbEndianTest) {
            msg.giidc.length = Swap16(msg.giidc.length);
            msg.giidc.vendorID = Swap32(msg.giidc.vendorID);
            msg.giidc.productID = Swap32(msg.giidc.productID);
            msg.giidc.canGenerate = Swap32(msg.giidc.canGenerate);
            msg.giidc.numRegisters = Swap32(msg.giidc.numRegisters);
            msg.giidc.numValuators = Swap32(msg.giidc.numValuators);
            msg.giidc.numButtons = Swap32(msg.giidc.numButtons);
          }

          rfbLog("GII Device Create: %s\n", msg.giidc.deviceName);
          if (rfbGIIDebug) {
            rfbLog("    Vendor ID: %d\n", msg.giidc.vendorID);
            rfbLog("    Product ID: %d\n", msg.giidc.productID);
            rfbLog("    Event mask: %.8x\n", msg.giidc.canGenerate);
            rfbLog("    Registers: %d\n", msg.giidc.numRegisters);
            rfbLog("    Valuators: %d\n", msg.giidc.numValuators);
            rfbLog("    Buttons: %d\n", msg.giidc.numButtons);
          }

          if (msg.giidc.length != sz_rfbGIIDeviceCreateMsg - 4 +
              msg.giidc.numValuators * sz_rfbGIIValuator) {
            rfbLog("ERROR: Malformed GII device create message\n");
            rfbCloseClient(cl);
            return;
          }

          if (msg.giidc.numButtons > MAX_BUTTONS) {
            rfbLog("GII device create ERROR: %d buttons exceeds max of %d\n",
                   msg.giidc.numButtons, MAX_BUTTONS);
            SKIP_OR_CLOSE(msg.giidc.numValuators * sz_rfbGIIValuator, return);
            goto sendMessage;
          }
          if (msg.giidc.numValuators > MAX_VALUATORS) {
            rfbLog("GII device create ERROR: %d valuators exceeds max of %d\n",
                   msg.giidc.numValuators, MAX_VALUATORS);
            SKIP_OR_CLOSE(msg.giidc.numValuators * sz_rfbGIIValuator, return);
            goto sendMessage;
          }

          memcpy(&dev.name, msg.giidc.deviceName, 32);
          dev.numButtons = msg.giidc.numButtons;
          dev.numTouches = msg.giidc.numRegisters;
          dev.numValuators = msg.giidc.numValuators;
          dev.eventMask = msg.giidc.canGenerate;
          dev.mode =
            (dev.eventMask & rfbGIIValuatorAbsoluteMask) ? Absolute : Relative;
          dev.productID = msg.giidc.productID;

          if (dev.mode == Relative) {
            rfbLog("GII device create ERROR: relative valuators not supported (yet)\n");
            SKIP_OR_CLOSE(msg.giidc.numValuators * sz_rfbGIIValuator, return);
            goto sendMessage;
          }

          for (i = 0; i < dev.numValuators; i++) {
            rfbGIIValuator *v = &dev.valuators[i];
            READ_OR_CLOSE((char *)v, sz_rfbGIIValuator, return);
            if (littleEndian != *(const char *)&rfbEndianTest) {
              v->index = Swap32(v->index);
              v->rangeMin = Swap32((CARD32)v->rangeMin);
              v->rangeCenter = Swap32((CARD32)v->rangeCenter);
              v->rangeMax = Swap32((CARD32)v->rangeMax);
              v->siUnit = Swap32(v->siUnit);
              v->siAdd = Swap32((CARD32)v->siAdd);
              v->siMul = Swap32((CARD32)v->siMul);
              v->siDiv = Swap32((CARD32)v->siDiv);
              v->siShift = Swap32((CARD32)v->siShift);
            }

            if (rfbGIIDebug) {
              rfbLog("    Valuator: %s (%s)\n", v->longName, v->shortName);
              rfbLog("        Index: %d\n", v->index);
              rfbLog("        Range: min = %d, center = %d, max = %d\n",
                     v->rangeMin, v->rangeCenter, v->rangeMax);
              rfbLog("        SI unit: %d\n", v->siUnit);
              rfbLog("        SI add: %d\n", v->siAdd);
              rfbLog("        SI multiply: %d\n", v->siMul);
              rfbLog("        SI divide: %d\n", v->siDiv);
              rfbLog("        SI shift: %d\n", v->siShift);
            }
          }

          for (i = 0; i < cl->numDevices; i++) {
            if (!strcmp(dev.name, cl->devices[i].name)) {
              rfbLog("Device \'%s\' already exists with GII device ID %d\n",
                     dev.name, i + 1);
              dcmsg.deviceOrigin = Swap32IfLE(i + 1);
              goto sendMessage;
            }
          }

          /* The UltraVNC Viewer sends multitouch events by way of the GII RFB
             extension, but it uses a different GII valuator event format than
             that of the TurboVNC Viewer.  Thus, we create fake valuators
             similar to those used by the TurboVNC Viewer, and we map the
             UltraVNC Viewer's multitouch GII valuator events to those fake
             valuators.  "TCVNC-MT" is the stock UltraVNC Viewer, and
             "HMI_Emb_VNC_Viewer" is the SINUMERIK VNC client, which is based
             on UltraVNC. */
          if ((!strcmp(dev.name, "TCVNC-MT") &&
               !strcmp((char *)dev.valuators[0].longName,
                       "TCVNC Multitouch Device") &&
               !strcmp((char *)dev.valuators[0].shortName, "TMD") &&
               msg.giidc.vendorID == 0x0908 && dev.productID == 0x000b) ||
              (!strcmp(dev.name, "HMI_Emb_VNC_Viewer") &&
               !strcmp((char *)dev.valuators[0].longName,
                       "HMI Embedded VNC Viewer Redirection") &&
               !strcmp((char *)dev.valuators[0].shortName, "EmbV") &&
               msg.giidc.vendorID == 0x0908 && dev.productID == 0x00737772)) {
            dev.multitouch_uvnc = TRUE;
            dev.numTouches = dev.numButtons;
            if (dev.numTouches > UVNCGII_MAX_TOUCHES) {
              rfbLog("WARNING: Requested number of simultaneous touches (%d) exceeds maximum of 10.\n",
                     dev.numTouches);
              rfbLog("    Additional touches will be ignored.\n");
            }
            dev.numButtons = 7;
            dev.numValuators = 4;
            dev.mode = Absolute;
            dev.productID = rfbGIIDevTypeTouch;

            dev.valuators[0].index = 0;
            snprintf((char *)dev.valuators[0].longName, 75,
                     "Abs MT Position X");
            snprintf((char *)dev.valuators[0].shortName, 5, "0");
            dev.valuators[0].rangeMax = 65535;
            dev.valuators[0].rangeCenter = dev.valuators[0].rangeMax / 2;
            dev.valuators[0].siUnit = rfbGIIUnitLength;

            dev.valuators[1].index = 1;
            snprintf((char *)dev.valuators[1].longName, 75,
                     "Abs MT Position Y");
            snprintf((char *)dev.valuators[1].shortName, 5, "1");
            dev.valuators[1].rangeMax = 65535;
            dev.valuators[1].rangeCenter = dev.valuators[1].rangeMax / 2;
            dev.valuators[1].siUnit = rfbGIIUnitLength;

            dev.valuators[2].index = 2;
            snprintf((char *)dev.valuators[2].longName, 75,
                     "__TURBOVNC FAKE TOUCH ID__");
            snprintf((char *)dev.valuators[2].shortName, 5, "TFTI");
            dev.valuators[2].rangeMax = INT_MAX;
            dev.valuators[2].rangeCenter = dev.valuators[2].rangeMax / 2;

            dev.valuators[3].index = 3;
            snprintf((char *)dev.valuators[3].longName, 75,
                     "__TURBOVNC FAKE TOUCH TYPE__");
            snprintf((char *)dev.valuators[3].shortName, 5, "TFTT");
            dev.valuators[3].rangeMax = 5;
            dev.valuators[3].rangeCenter = dev.valuators[3].rangeMax / 2;
          }

          if (rfbVirtualTablet || AddExtInputDevice(&dev)) {
            memcpy(&cl->devices[cl->numDevices], &dev, sizeof(dev));
            cl->numDevices++;
            dcmsg.deviceOrigin = Swap32IfLE(cl->numDevices);
          }
          rfbLog("GII device ID = %d\n", cl->numDevices);

          sendMessage:
          /* Send back a GII device created message */
          dcmsg.type = rfbGIIServer;
          /* We always send as big endian to make things easier on the Java
             viewer. */
          dcmsg.endianAndSubType = rfbGIIDeviceCreate | rfbGIIBE;
          dcmsg.length = Swap16IfLE(sz_rfbGIIDeviceCreatedMsg - 4);

          WRITE_OR_CLOSE(&dcmsg, sz_rfbGIIDeviceCreatedMsg, return);

          break;
        }

        case rfbGIIDeviceDestroy:

          READ_OR_CLOSE((char *)&msg.giidd.length,
                        sz_rfbGIIDeviceDestroyMsg - 2, return);
          if (littleEndian != *(const char *)&rfbEndianTest) {
            msg.giidd.length = Swap16(msg.giidd.length);
            msg.giidd.deviceOrigin = Swap32(msg.giidd.deviceOrigin);
          }
          if (msg.giidd.length != sz_rfbGIIDeviceDestroyMsg - 4) {
            rfbLog("ERROR: Malformed GII device create message\n");
            rfbCloseClient(cl);
            return;
          }

          RemoveExtInputDevice(cl, msg.giidd.deviceOrigin - 1);

          break;

        case rfbGIIEvent:
        {
          CARD16 length;

          READ_OR_CLOSE((char *)&length, sizeof(CARD16), return);
          if (littleEndian != *(const char *)&rfbEndianTest)
            length = Swap16(length);

          while (length > 0) {
            CARD8 eventSize, eventType;

            READ_OR_CLOSE((char *)&eventSize, 1, return);
            READ_OR_CLOSE((char *)&eventType, 1, return);

            switch (eventType) {

              case rfbGIIButtonPress:
              case rfbGIIButtonRelease:
              {
                rfbGIIButtonEvent b;
                rfbDevInfo *dev;

                READ_OR_CLOSE((char *)&b.pad, sz_rfbGIIButtonEvent - 2,
                              return);
                if (littleEndian != *(const char *)&rfbEndianTest) {
                  b.deviceOrigin = Swap32(b.deviceOrigin);
                  b.buttonNumber = Swap32(b.buttonNumber);
                }
                if (eventSize != sz_rfbGIIButtonEvent || b.deviceOrigin <= 0 ||
                    b.buttonNumber < 1) {
                  rfbLog("ERROR: Malformed GII button event\n");
                  rfbCloseClient(cl);
                  return;
                }
                if (eventSize > length) {
                  rfbLog("ERROR: Malformed GII event message\n");
                  rfbCloseClient(cl);
                  return;
                }
                length -= eventSize;
                if (b.deviceOrigin < 1 || b.deviceOrigin > cl->numDevices) {
                  rfbLog("ERROR: GII button event from non-existent device %d\n",
                         b.deviceOrigin);
                  rfbCloseClient(cl);
                  return;
                }
                dev = &cl->devices[b.deviceOrigin - 1];
                if ((eventType == rfbGIIButtonPress &&
                     (dev->eventMask & rfbGIIButtonPressMask) == 0) ||
                    (eventType == rfbGIIButtonRelease &&
                     (dev->eventMask & rfbGIIButtonReleaseMask) == 0)) {
                  rfbLog("ERROR: Device %d can't generate GII button events\n",
                         b.deviceOrigin);
                  rfbCloseClient(cl);
                  return;
                }
                if (b.buttonNumber > dev->numButtons) {
                  rfbLog("ERROR: GII button %d event for device %d exceeds button count (%d)\n",
                         b.buttonNumber, b.deviceOrigin, dev->numButtons);
                  rfbCloseClient(cl);
                  return;
                }
                if (rfbGIIDebug) {
                  rfbLog("Device %d button %d %s\n", b.deviceOrigin,
                         b.buttonNumber,
                         eventType == rfbGIIButtonPress ? "PRESS" : "release");
                  fflush(stderr);
                }
                ExtInputAddEvent(dev, eventType == rfbGIIButtonPress ?
                                 ButtonPress : ButtonRelease, b.buttonNumber);
                break;
              }

              case rfbGIIValuatorRelative:
              case rfbGIIValuatorAbsolute:
              {
                rfbGIIValuatorEvent v;
                int i, t;
                rfbDevInfo *dev;

                READ_OR_CLOSE((char *)&v.pad, sz_rfbGIIValuatorEvent - 2,
                              return);
                if (littleEndian != *(const char *)&rfbEndianTest) {
                  v.deviceOrigin = Swap32(v.deviceOrigin);
                  v.first = Swap32(v.first);
                  v.count = Swap32(v.count);
                }
                if (eventSize !=
                    sz_rfbGIIValuatorEvent + sizeof(int) * v.count) {
                  rfbLog("ERROR: Malformed GII valuator event\n");
                  rfbCloseClient(cl);
                  return;
                }
                if (eventSize > length) {
                  rfbLog("ERROR: Malformed GII event message\n");
                  rfbCloseClient(cl);
                  return;
                }
                length -= eventSize;
                if (v.deviceOrigin < 1 || v.deviceOrigin > cl->numDevices) {
                  rfbLog("ERROR: GII valuator event from non-existent device %d\n",
                         v.deviceOrigin);
                  rfbCloseClient(cl);
                  return;
                }
                dev = &cl->devices[v.deviceOrigin - 1];
                /* The UltraVNC Viewer sends absolute valuator events but
                   labels them as relative valuator events. */
                if (dev->multitouch_uvnc &&
                    eventType == rfbGIIValuatorRelative)
                  eventType = rfbGIIValuatorAbsolute;
                if ((eventType == rfbGIIValuatorRelative &&
                     (dev->eventMask & rfbGIIValuatorRelativeMask) == 0) ||
                    (eventType == rfbGIIValuatorAbsolute &&
                     (dev->eventMask & rfbGIIValuatorAbsoluteMask) == 0)) {
                  rfbLog("ERROR: Device %d cannot generate GII valuator events\n",
                         v.deviceOrigin);
                  rfbCloseClient(cl);
                  return;
                }

                /* Parse the UltraVNC Viewer's multitouch GII valuator event
                   format */
                if (dev->multitouch_uvnc) {
                  CARD32 numTouchEvents = v.first, numValues = v.count;
                  static const char *touch_type_string[6] = {
                    "TouchBegin", "TouchUpdate", "TouchEnd",
                    "TouchBegin (pointer emulation)",
                    "TouchUpdate (pointer emulation)",
                    "TouchEnd (pointer emulation)"
                  };

                  if (rfbGIIDebug)
                    rfbLog("Device %d count=%d:\n", v.deviceOrigin,
                           numTouchEvents);
                  for (i = 0; i < numTouchEvents; i++) {
                    CARD32 formatFlags, expectedValues = 1, dummy;
                    CARD64 dummy64;

                    READ_OR_CLOSE((char *)&formatFlags, sizeof(CARD32),
                                  return);
                    if (littleEndian != *(const char *)&rfbEndianTest)
                      formatFlags = Swap32(formatFlags);
                    if (rfbGIIDebug)
                      rfbLog("  %d: format flags = 0x%.8x\n", i, formatFlags);
                    if ((formatFlags & 0xFF) == 0x11) expectedValues += 2;
                    if (formatFlags & UVNCGII_S1_FLAG) expectedValues++;
                    if (formatFlags & UVNCGII_PR_FLAG) expectedValues++;
                    if (formatFlags & UVNCGII_TI_FLAG) expectedValues++;
                    if (formatFlags & UVNCGII_HC_FLAG) expectedValues += 2;
                    /* Some implementations of the UltraVNC Viewer hard-code
                       rfbGIIValuatorEvent.count to
                       6 * rfbGIIValuatorEvent.first regardless of the number
                       of DWORDs actually sent, so we have to be lenient here.
                       As long as the viewer sends the number of DWORDs
                       specified by the format flags, everything should still
                       work. */
                    if (expectedValues * numTouchEvents != numValues) {
                      static int alreadyWarned = 0;
                      if (!alreadyWarned) {
                        rfbLog("WARNING: Malformed GII valuator event\n");
                        rfbLog("    (Count should be %d, not %d.)\n",
                               expectedValues * numTouchEvents, numValues);
                        alreadyWarned = 1;
                      }
                    }

                    if ((formatFlags & 0xFF) == 0x1F) {
                      /* Empty multitouch event received.  End all active
                         touches. */
                      for (t = 0; t < numValues - 1; t++)
                        READ_OR_CLOSE((char *)&dummy, sizeof(CARD32), return);
                      for (t = 0; t < UVNCGII_MAX_TOUCHES; t++) {
                        if (dev->active_touches_uvnc[t][2] != -1) {
                          dev->valFirst = 0;
                          dev->valCount = 4;
                          dev->values[0] =
                            dev->active_touches_uvnc[t][0];  /* X */
                          dev->values[1] =
                            dev->active_touches_uvnc[t][1];  /* Y */
                          dev->values[2] =
                            dev->active_touches_uvnc[t][2];  /* ID */
                          if (dev->active_touches_uvnc[t][3] >=
                              rfbGIITouchBeginEP)
                            dev->values[3] = rfbGIITouchEndEP;  /* Type */
                          else
                            dev->values[3] = rfbGIITouchEnd;
                          if (rfbGIIDebug) {
                            rfbLog("  %d: touch ID = %d\n", i, dev->values[2]);
                            rfbLog("  %d: touch position = %d, %d\n", i,
                                   dev->values[0], dev->values[1]);
                            rfbLog("  %d: touch type = %d [%s]\n", i,
                                   dev->values[3],
                                   dev->values[3] >= 0 && dev->values[3] < 6 ?
                                   touch_type_string[dev->values[3]] : "");
                          }
                          ExtInputAddEvent(dev, MotionNotify, 0);
                          dev->active_touches_uvnc[t][2] = -1;
                        }
                      }
                      continue;
                    } else if ((formatFlags & 0xFF) != 0x11) {
                      rfbLog("ERROR: Unsupported multitouch event format\n");
                      rfbCloseClient(cl);
                      return;
                    }

                    READ_OR_CLOSE((char *)&dev->values[2], sizeof(int),
                                  return);  /* ID */
                    if (littleEndian != *(const char *)&rfbEndianTest)
                      dev->values[2] = Swap32(dev->values[2]);
                    if (rfbGIIDebug)
                      rfbLog("  %d: touch ID = %d\n", i, dev->values[2]);

                    if (formatFlags & 0x10) {
                      CARD32 touchPos;

                      READ_OR_CLOSE((char *)&touchPos, sizeof(CARD32), return);
                      if (littleEndian != *(const char *)&rfbEndianTest)
                        touchPos = Swap32(touchPos);
                      dev->values[0] = touchPos >> 16;  /* X */
                      dev->values[0] =
                        (int)round((double)dev->values[0] * 65535.0 /
                                   (double)rfbFB.width);
                      dev->values[1] = touchPos & 0xFFFF;  /* Y */
                      dev->values[1] =
                        (int)round((double)dev->values[1] * 65535.0 /
                                   (double)rfbFB.height);
                      if (rfbGIIDebug)
                        rfbLog("  %d: touch position = %d, %d\n", i,
                               dev->values[0], dev->values[1]);
                    }

                    /* Ignore touch area */
                    if (formatFlags & UVNCGII_S1_FLAG)
                      READ_OR_CLOSE((char *)&dummy, sizeof(CARD32), return);
                    /* Ignore touch pressure */
                    if (formatFlags & UVNCGII_PR_FLAG)
                      READ_OR_CLOSE((char *)&dummy, sizeof(CARD32), return);
                    /* Ignore timestamp */
                    if (formatFlags & UVNCGII_TI_FLAG)
                      READ_OR_CLOSE((char *)&dummy, sizeof(CARD32), return);
                    /* Ignore high-resolution timestamp */
                    if (formatFlags & UVNCGII_HC_FLAG)
                      READ_OR_CLOSE((char *)&dummy64, sizeof(CARD64), return);

                    dev->values[3] = -1;
                    for (t = 0; t < UVNCGII_MAX_TOUCHES; t++) {
                      if (dev->active_touches_uvnc[t][2] == dev->values[2]) {
                        if (formatFlags & UVNCGII_PF_FLAG) {
                          if (formatFlags & UVNCGII_IF_FLAG)
                            dev->values[3] = rfbGIITouchUpdateEP;
                          else
                            dev->values[3] = rfbGIITouchUpdate;
                          memcpy(dev->active_touches_uvnc[t], dev->values,
                                 4 * sizeof(int));
                        } else {
                          if (formatFlags & UVNCGII_IF_FLAG)
                            dev->values[3] = rfbGIITouchEndEP;
                          else
                            dev->values[3] = rfbGIITouchEnd;
                          dev->active_touches_uvnc[t][2] = -1;
                        }
                        break;
                      }
                    }
                    if (dev->values[3] == -1) {
                      if (formatFlags & UVNCGII_IF_FLAG)
                        dev->values[3] = rfbGIITouchBeginEP;
                      else
                        dev->values[3] = rfbGIITouchBegin;
                      for (t = 0; t < UVNCGII_MAX_TOUCHES; t++) {
                        if (dev->active_touches_uvnc[t][2] == -1) {
                          memcpy(dev->active_touches_uvnc[t], dev->values,
                                 4 * sizeof(int));
                          break;
                        }
                      }
                    }
                    if (rfbGIIDebug)
                      rfbLog("  %d: touch type = %d [%s]\n", i, dev->values[3],
                             dev->values[3] >= 0 && dev->values[3] < 6 ?
                             touch_type_string[dev->values[3]] : "");

                    dev->valFirst = 0;
                    dev->valCount = 4;
                    ExtInputAddEvent(dev, MotionNotify, 0);
                  }
                  break;
                }

                if (v.first + v.count > dev->numValuators) {
                  rfbLog("ERROR: GII valuator event for device %d exceeds valuator count (%d)\n",
                         v.deviceOrigin, dev->numValuators);
                  rfbCloseClient(cl);
                  return;
                }
                if (rfbGIIDebug)
                  rfbLog("Device %d Valuator %s first=%d count=%d:\n",
                         v.deviceOrigin,
                         eventType == rfbGIIValuatorRelative ? "rel" : "ABS",
                         v.first, v.count);
                for (i = v.first; i < v.first + v.count; i++) {
                  READ_OR_CLOSE((char *)&dev->values[i], sizeof(int), return);
                  if (littleEndian != *(const char *)&rfbEndianTest)
                    dev->values[i] = Swap32((CARD32)dev->values[i]);
                  if (rfbGIIDebug)
                    fprintf(stderr, "v[%d]=%d ", i, dev->values[i]);
                }
                if (rfbGIIDebug)
                  fprintf(stderr, "\n");
                if (v.count > 0) {
                  dev->valFirst = v.first;
                  dev->valCount = v.count;
                  dev->mode = eventType == rfbGIIValuatorAbsolute ?
                              Absolute : Relative;
                  ExtInputAddEvent(dev, MotionNotify, 0);
                }
                break;
              }
              default:
                rfbLog("ERROR: This server cannot handle GII event type %d\n",
                       eventType);
                rfbCloseClient(cl);
                return;
            }  /* switch (eventType) */
          }  /* while (length > 0) */
          if (length != 0) {
            rfbLog("ERROR: Malformed GII event message\n");
            rfbCloseClient(cl);
            return;
          }
          break;
        }  /* rfbGIIEvent */
      }  /* switch (subType) */
      return;
    }  /* rfbGIIClient */

    case rfbQEMU:
    {
      READ_OR_CLOSE(((char *)&msg) + 1, sz_rfbQEMUExtendedKeyEventMsg - 1,
                    return);

      if (msg.qemueke.subType != 0) {
        rfbLog("ERROR: This server cannot handle QEMU message subtype %d\n",
               msg.qemueke.subType);
        rfbCloseClient(cl);
        return;
      }
      msg.qemueke.down = Swap16IfLE(msg.qemueke.down);
      msg.qemueke.keysym = Swap32IfLE(msg.qemueke.keysym);
      msg.qemueke.keycode = Swap32IfLE(msg.qemueke.keycode);
      if (!msg.qemueke.keycode) {
        rfbLog("Ignoring QEMU extended key event without key code.\n");
        return;
      }

      if (!rfbViewOnly && !cl->viewOnly)
        ExtKeyEvent(msg.qemueke.keysym, msg.qemueke.keycode, msg.qemueke.down);

      break;
    }

    default:

      rfbLog("rfbProcessClientNormalMessage: unknown message type %d\n",
             msg.type);
      rfbLog(" ... closing connection\n");
      rfbCloseClient(cl);
      return;
  }  /* switch (msg.type) */
}


/*
 * rfbSendFramebufferUpdate - send the currently pending framebuffer update to
 * the RFB client.
 */

Bool rfbSendFramebufferUpdate(rfbClientPtr cl)
{
  ScreenPtr pScreen = screenInfo.screens[0];
  int i;
  int nUpdateRegionRects;
  rfbFramebufferUpdateMsg *fu = (rfbFramebufferUpdateMsg *)updateBuf;
  RegionRec _updateRegion, *updateRegion = &_updateRegion, updateCopyRegion,
    idRegion;
  Bool emptyUpdateRegion = FALSE;
  rfbClientPtr cl2;
  int dx, dy;
  Bool sendCursorShape = FALSE;
  Bool sendCursorPos = FALSE;
  Bool redundantUpdate = FALSE;
  double tUpdateStart = 0.0;

  rfbUpdatePosition(cl, cl->sockOffset);

  /*
   * We're in the middle of processing a command that's supposed to be
   * synchronised. Allowing an update to slip out right now might violate
   * that synchronisation.
   */

  if (cl->syncFence) return TRUE;

  if (cl->state != RFB_NORMAL) return TRUE;

  if (rfbProfile) {
    tUpdateStart = gettime();
    if (tStart < 0.) tStart = tUpdateStart;
  }

  /* Check that we actually have some space on the link and retry in a
     bit if things are congested. */

  if (rfbCongestionControl && rfbIsCongested(cl) && !cl->inALR)
    return TRUE;

  /* In continuous mode, we will be outputting at least three distinct
     messages.  We need to aggregate these in order to not clog up TCP's
     congestion window. */

  rfbCorkSock(cl->sock);

  if (cl->pendingExtDesktopResize) {
    if (!rfbSendExtDesktopSize(cl)) return FALSE;
    cl->pendingExtDesktopResize = FALSE;
  }

  if (cl->pendingDesktopResize) {
    if (!rfbSendDesktopSize(cl)) return FALSE;
    cl->pendingDesktopResize = FALSE;
  }

  if (rfbFB.blockUpdates) {
    rfbUncorkSock(cl->sock);
    return TRUE;
  }

  /*
   * If this client understands cursor shape updates and owns the pointer or is
   * about to own the pointer, then the cursor should be removed from the
   * framebuffer.  Otherwise, make sure it's drawn.
   */

  if (cl->enableCursorShapeUpdates && (!pointerOwner || pointerOwner == cl)) {
    if (rfbFB.cursorIsDrawn)
      rfbSpriteRemoveCursorAllDev(pScreen);
    if (!rfbFB.cursorIsDrawn && cl->cursorWasChanged)
      sendCursorShape = TRUE;
  } else {
    if (!rfbFB.cursorIsDrawn)
      rfbSpriteRestoreCursorAllDev(pScreen);
  }

  /*
   * Do we plan to send cursor position update?
   */

  if (cl->enableCursorPosUpdates && cl->cursorWasMoved)
    sendCursorPos = TRUE;

  /*
   * The modifiedRegion may overlap the destination copyRegion.  We remove
   * any overlapping bits from the copyRegion (since they'd only be
   * overwritten anyway).
   */

  REGION_SUBTRACT(pScreen, &cl->copyRegion, &cl->copyRegion,
                  &cl->modifiedRegion);

  /*
   * The client is interested in the region requestedRegion.  The region
   * which should be updated now is the intersection of requestedRegion
   * and the union of modifiedRegion and copyRegion.  If it's empty then
   * no update is needed.
   */

  REGION_INIT(pScreen, updateRegion, NullBox, 0);
  REGION_UNION(pScreen, updateRegion, &cl->copyRegion, &cl->modifiedRegion);

  if (cl->continuousUpdates)
    REGION_UNION(pScreen, &cl->requestedRegion, &cl->requestedRegion,
                 &cl->cuRegion);

  REGION_INTERSECT(pScreen, updateRegion, &cl->requestedRegion, updateRegion);

  if (!REGION_NOTEMPTY(pScreen, updateRegion) && !sendCursorShape &&
      !sendCursorPos && !cl->pendingQEMUExtKeyEventRect &&
      !cl->pendingLEDState) {
    REGION_UNINIT(pScreen, updateRegion);
    return TRUE;
  }

  /*
   * We assume that the client doesn't have any pixel data outside the
   * requestedRegion.  In other words, both the source and destination of a
   * copy must lie within requestedRegion.  So the region we can send as a
   * copy is the intersection of the copyRegion with both the requestedRegion
   * and the requestedRegion translated by the amount of the copy.  We set
   * updateCopyRegion to this.
   */

  REGION_INIT(pScreen, &updateCopyRegion, NullBox, 0);
  REGION_INTERSECT(pScreen, &updateCopyRegion, &cl->copyRegion,
                   &cl->requestedRegion);
  REGION_TRANSLATE(pScreen, &cl->requestedRegion, cl->copyDX, cl->copyDY);
  REGION_INTERSECT(pScreen, &updateCopyRegion, &updateCopyRegion,
                   &cl->requestedRegion);
  dx = cl->copyDX;
  dy = cl->copyDY;

  /*
   * Next we remove updateCopyRegion from updateRegion so that updateRegion
   * is the part of this update which is sent as ordinary pixel data (i.e not
   * a copy).
   */

  REGION_SUBTRACT(pScreen, updateRegion, updateRegion, &updateCopyRegion);

  /*
   * Finally we leave modifiedRegion to be the remainder (if any) of parts of
   * the screen which are modified but outside the requestedRegion.  We also
   * empty both the requestedRegion and the copyRegion - note that we never
   * carry over a copyRegion for a future update.
   */

  REGION_UNION(pScreen, &cl->modifiedRegion, &cl->modifiedRegion,
               &cl->copyRegion);
  REGION_SUBTRACT(pScreen, &cl->modifiedRegion, &cl->modifiedRegion,
                  updateRegion);
  REGION_SUBTRACT(pScreen, &cl->modifiedRegion, &cl->modifiedRegion,
                  &updateCopyRegion);

  REGION_EMPTY(pScreen, &cl->requestedRegion);
  REGION_EMPTY(pScreen, &cl->copyRegion);
  cl->copyDX = 0;
  cl->copyDY = 0;

  /*
   * Now send the update.
   */

  if (REGION_NUM_RECTS(updateRegion) > rfbCombineRect) {
    RegionRec combinedUpdateRegion;
    REGION_INIT(pScreen, &combinedUpdateRegion,
                REGION_EXTENTS(pScreen, updateRegion), 1);
    REGION_UNINIT(pScreen, updateRegion);
    REGION_INIT(pScreen, updateRegion,
                REGION_EXTENTS(pScreen, &combinedUpdateRegion), 1);
    REGION_UNINIT(pScreen, &combinedUpdateRegion);
  }

  if ((updateRegion->extents.x2 > pScreen->width ||
       updateRegion->extents.y2 > pScreen->height) &&
      REGION_NUM_RECTS(updateRegion) > 0) {
    rfbLog("WARNING: Framebuffer update at %d,%d with dimensions %dx%d has been clipped to the screen boundaries\n",
           updateRegion->extents.x1, updateRegion->extents.y1,
           updateRegion->extents.x2 - updateRegion->extents.x1,
           updateRegion->extents.y2 - updateRegion->extents.y1);
    ClipToScreen(pScreen, updateRegion);
  }

  if (cl->compareFB && !cl->inALR) {
    if ((cl->ifRegion.extents.x2 > pScreen->width ||
         cl->ifRegion.extents.y2 > pScreen->height) &&
        REGION_NUM_RECTS(&cl->ifRegion) > 0)
      ClipToScreen(pScreen, &cl->ifRegion);

    updateRegion = &cl->ifRegion;
    emptyUpdateRegion = TRUE;
    if (rfbInterframeDebug)
      REGION_INIT(pScreen, &idRegion, NullBox, 0);
    for (i = 0; i < REGION_NUM_RECTS(&_updateRegion); i++) {
      int x = REGION_RECTS(&_updateRegion)[i].x1;
      int y = REGION_RECTS(&_updateRegion)[i].y1;
      int w = REGION_RECTS(&_updateRegion)[i].x2 - x;
      int h = REGION_RECTS(&_updateRegion)[i].y2 - y;
      int pitch = rfbFB.paddedWidthInBytes;
      int ps = rfbServerFormat.bitsPerPixel / 8;
      char *src = &rfbFB.pfbMemory[y * pitch + x * ps];
      char *dst = &cl->compareFB[y * pitch + x * ps];
      int row, col;
      int hBlockSize = rfbICEBlockSize == 0 ? w : rfbICEBlockSize;
      int vBlockSize = rfbICEBlockSize == 0 ? h : rfbICEBlockSize;

      for (row = 0; row < h; row += vBlockSize) {
        for (col = 0; col < w; col += hBlockSize) {

          Bool different = FALSE;
          int compareWidth = min(hBlockSize, w - col);
          int compareHeight = min(vBlockSize, h - row);
          int rows = compareHeight;
          char *srcPtr = &src[row * pitch + col * ps];
          char *dstPtr = &dst[row * pitch + col * ps];

          while (rows--) {
            if (cl->firstCompare ||
                memcmp(srcPtr, dstPtr, compareWidth * ps)) {
              memcpy(dstPtr, srcPtr, compareWidth * ps);
              different = TRUE;
            }
            srcPtr += pitch;
            dstPtr += pitch;
          }
          if (different || rfbInterframeDebug) {
            RegionRec tmpRegion;
            BoxRec box;
            box.x1 = x + col;
            box.y1 = y + row;
            box.x2 = box.x1 + compareWidth;
            box.y2 = box.y1 + compareHeight;

            REGION_INIT(pScreen, &tmpRegion, &box, 1);
            if (!different && rfbInterframeDebug &&
                !RECT_IN_REGION(pScreen, &cl->ifRegion, &box)) {
              int pad = pitch - compareWidth * ps;

              dstPtr = &dst[row * pitch + col * ps];
              REGION_UNION(pScreen, &idRegion, &idRegion, &tmpRegion);
              rows = compareHeight;

              while (rows--) {
                char *endOfRow = &dstPtr[compareWidth * ps];
                while (dstPtr < endOfRow)
                  *dstPtr++ ^= 0xFF;
                dstPtr += pad;
              }
            }
            REGION_UNION(pScreen, &cl->ifRegion, &cl->ifRegion, &tmpRegion);
            REGION_UNINIT(pScreen, &tmpRegion);
          }
          if (!different && rfbProfile) {
            idmpixels += (double)(compareWidth * compareHeight) / 1000000.;
            if (!rfbInterframeDebug)
              mpixels += (double)(compareWidth * compareHeight) / 1000000.;
          }
        }
      }
    }
    REGION_UNINIT(pScreen, &_updateRegion);
    REGION_NULL(pScreen, &_updateRegion);
    cl->firstCompare = FALSE;

    /* The Windows TurboVNC Viewer (and probably some other VNC viewers as
       well) will ignore any empty FBUs and stop sending FBURs when it
       receives one.  If CU is not active, then this causes the viewer to
       stop receiving updates until something else, such as a mouse cursor
       change, triggers a new FBUR.  Thus, if the ICE culls all of the
       pixels in this update, we send a 1-pixel FBU rather than an empty
       one. */
    if (REGION_NUM_RECTS(updateRegion) == 0) {
      BoxRec box;
      box.x1 = box.y1 = 0;
      box.x2 = box.y2 = 1;
      REGION_UNINIT(pScreen, updateRegion);
      REGION_INIT(pScreen, updateRegion, &box, 1);
      redundantUpdate = TRUE;
    }
  }

  if (!rfbSendRTTPing(cl))
    goto abort;

  cl->rfbFramebufferUpdateMessagesSent++;

  if (cl->preferredEncoding == rfbEncodingCoRRE) {
    nUpdateRegionRects = 0;

    for (i = 0; i < REGION_NUM_RECTS(updateRegion); i++) {
      int x = REGION_RECTS(updateRegion)[i].x1;
      int y = REGION_RECTS(updateRegion)[i].y1;
      int w = REGION_RECTS(updateRegion)[i].x2 - x;
      int h = REGION_RECTS(updateRegion)[i].y2 - y;
      nUpdateRegionRects += (((w - 1) / cl->correMaxWidth + 1) *
                             ((h - 1) / cl->correMaxHeight + 1));
    }
  } else if (cl->preferredEncoding == rfbEncodingZlib) {
    nUpdateRegionRects = 0;

    for (i = 0; i < REGION_NUM_RECTS(updateRegion); i++) {
      int x = REGION_RECTS(updateRegion)[i].x1;
      int y = REGION_RECTS(updateRegion)[i].y1;
      int w = REGION_RECTS(updateRegion)[i].x2 - x;
      int h = REGION_RECTS(updateRegion)[i].y2 - y;
      nUpdateRegionRects += (((h - 1) / (ZLIB_MAX_SIZE(w) / w)) + 1);
    }
  } else if (cl->preferredEncoding == rfbEncodingTight) {
    nUpdateRegionRects = 0;

    for (i = 0; i < REGION_NUM_RECTS(updateRegion); i++) {
      int x = REGION_RECTS(updateRegion)[i].x1;
      int y = REGION_RECTS(updateRegion)[i].y1;
      int w = REGION_RECTS(updateRegion)[i].x2 - x;
      int h = REGION_RECTS(updateRegion)[i].y2 - y;
      int n = rfbNumCodedRectsTight(cl, x, y, w, h);
      if (n == 0) {
        nUpdateRegionRects = 0xFFFF;
        break;
      }
      nUpdateRegionRects += n;
    }
  } else {
    nUpdateRegionRects = REGION_NUM_RECTS(updateRegion);
  }

  fu->type = rfbFramebufferUpdate;
  if (nUpdateRegionRects != 0xFFFF) {
    fu->nRects = Swap16IfLE(REGION_NUM_RECTS(&updateCopyRegion) +
                            nUpdateRegionRects +
                            !!sendCursorShape + !!sendCursorPos +
                            !!cl->pendingQEMUExtKeyEventRect +
                            !!cl->pendingLEDState);
  } else {
    fu->nRects = 0xFFFF;
  }
  ublen = sz_rfbFramebufferUpdateMsg;

  cl->captureEnable = TRUE;

  if (sendCursorShape) {
    cl->cursorWasChanged = FALSE;
    if (!rfbSendCursorShape(cl, pScreen))
      goto abort;
  }

  if (sendCursorPos) {
    cl->cursorWasMoved = FALSE;
    if (!rfbSendCursorPos(cl, pScreen))
      goto abort;
  }

  if (cl->pendingQEMUExtKeyEventRect) {
    if (!rfbSendQEMUExtKeyEventRect(cl))
      goto abort;
    cl->pendingQEMUExtKeyEventRect = FALSE;
  }

  if (cl->pendingLEDState) {
    if (!rfbSendLEDState(cl))
      goto abort;
    cl->pendingLEDState = FALSE;
  }

  if (REGION_NOTEMPTY(pScreen, &updateCopyRegion)) {
    if (!rfbSendCopyRegion(cl, &updateCopyRegion, dx, dy))
      goto abort;
  }

  REGION_UNINIT(pScreen, &updateCopyRegion);
  REGION_NULL(pScreen, &updateCopyRegion);

  for (i = 0; i < REGION_NUM_RECTS(updateRegion); i++) {
    int x = REGION_RECTS(updateRegion)[i].x1;
    int y = REGION_RECTS(updateRegion)[i].y1;
    int w = REGION_RECTS(updateRegion)[i].x2 - x;
    int h = REGION_RECTS(updateRegion)[i].y2 - y;

    cl->rfbRawBytesEquivalent += (sz_rfbFramebufferUpdateRectHeader +
                                 w * (cl->format.bitsPerPixel / 8) * h);

    if (rfbProfile) mpixels += (double)w * (double)h / 1000000.;

    switch (cl->preferredEncoding) {
      case rfbEncodingRaw:
        if (!rfbSendRectEncodingRaw(cl, x, y, w, h))
          goto abort;
        break;
      case rfbEncodingRRE:
        if (!rfbSendRectEncodingRRE(cl, x, y, w, h))
          goto abort;
        break;
      case rfbEncodingCoRRE:
        if (!rfbSendRectEncodingCoRRE(cl, x, y, w, h))
          goto abort;
        break;
      case rfbEncodingHextile:
        if (!rfbSendRectEncodingHextile(cl, x, y, w, h))
          goto abort;
        break;
      case rfbEncodingZlib:
        if (!rfbSendRectEncodingZlib(cl, x, y, w, h))
          goto abort;
        break;
      case rfbEncodingZRLE:
      case rfbEncodingZYWRLE:
        if (!rfbSendRectEncodingZRLE(cl, x, y, w, h))
          goto abort;
        break;
      case rfbEncodingTight:
        if (!rfbSendRectEncodingTight(cl, x, y, w, h))
          goto abort;
        break;
    }
  }

  if (cl->compareFB && !cl->inALR) {
    if (rfbInterframeDebug) {
      for (i = 0; i < REGION_NUM_RECTS(&idRegion); i++) {
        int x = REGION_RECTS(&idRegion)[i].x1;
        int y = REGION_RECTS(&idRegion)[i].y1;
        int w = REGION_RECTS(&idRegion)[i].x2 - x;
        int h = REGION_RECTS(&idRegion)[i].y2 - y, rows;
        int pitch = rfbFB.paddedWidthInBytes;
        int ps = rfbServerFormat.bitsPerPixel / 8;
        char *src = &rfbFB.pfbMemory[y * pitch + x * ps];
        char *dst = &cl->compareFB[y * pitch + x * ps];
        rows = h;
        while (rows--) {
          memcpy(dst, src, w * ps);
          src += pitch;
          dst += pitch;
        }
      }
      REGION_UNINIT(pScreen, &idRegion);
      REGION_NULL(pScreen, &idRegion);
    }
    REGION_EMPTY(pScreen, updateRegion);
  } else {
    REGION_UNINIT(pScreen, updateRegion);
    REGION_NULL(pScreen, updateRegion);
  }

  if (nUpdateRegionRects == 0xFFFF && !rfbSendLastRectMarker(cl))
    goto abort;

  if (!rfbSendUpdateBuf(cl))
    goto abort;

  cl->captureEnable = FALSE;

  if (!rfbSendRTTPing(cl))
    goto abort;

  if (rfbProfile) {
    tUpdate += gettime() - tUpdateStart;
    tElapsed = gettime() - tStart;
    updates++;

    if (tElapsed > 5.) {
      rfbLog("%.2f updates/sec,  %.2f Mpixels/sec,  %.3f Mbits/sec\n",
             (double)updates / tElapsed, mpixels / tElapsed,
             (double)sendBytes / 125000. / tElapsed);
      rfbLog("Time/update:  Encode = %.3f ms,  Other = %.3f ms\n",
             tUpdate / (double)updates * 1000.,
             (tElapsed - tUpdate) / (double)updates * 1000.);
      if (cl->compareFB) {
        rfbLog("Identical Mpixels/sec:  %.2f  (%f %%)\n",
               (double)idmpixels / tElapsed, idmpixels / mpixels * 100.0);
        idmpixels = 0.;
      }
      tUpdate = 0.;
      updates = 0;
      mpixels = 0.;
      sendBytes = 0;
      tStart = gettime();
    }
  }

  if (rfbAutoLosslessRefresh > 0.0 && !redundantUpdate && !cl->inALR &&
      (rfbALRAll || REGION_NOTEMPTY(pScreen, &cl->alrEligibleRegion) ||
       cl->firstUpdate)) {
    CARD32 timeout = (CARD32)(rfbAutoLosslessRefresh * 1000.0);

    if (!rfbALRAll)
      REGION_UNION(pScreen, &cl->alrRegion, &cl->alrRegion,
                   &cl->alrEligibleRegion);
    REGION_EMPTY(pScreen, &cl->alrEligibleRegion);
    /* If the ALR timeout is less than the network round-trip time, then
       temporarily increase the timeout to avoid thrashing. */
    if (cl->minRTT != (unsigned)-1)
      timeout = max(timeout, (CARD32)((double)cl->minRTT * 1.5));
    else if (cl->baseRTT != (unsigned)-1)
      timeout = max(timeout, (CARD32)((double)cl->baseRTT * 1.5));
    cl->alrTimer = TimerSet(cl->alrTimer, 0, timeout, alrCallback, cl);
  }

  rfbUncorkSock(cl->sock);
  rfbUpdatePosition(cl, cl->sockOffset);
  return TRUE;

  abort:
  if (!REGION_NIL(&updateCopyRegion))
    REGION_UNINIT(pScreen, &updateCopyRegion);
  if (rfbInterframeDebug && !REGION_NIL(&idRegion))
    REGION_UNINIT(pScreen, &idRegion);
  if (emptyUpdateRegion) {
    /* Make sure cl hasn't been freed */
    for (cl2 = rfbClientHead; cl2; cl2 = cl2->next) {
      if (cl2 == cl) {
        REGION_EMPTY(pScreen, updateRegion);
        break;
      }
    }
  } else if (!REGION_NIL(&_updateRegion)) {
    REGION_UNINIT(pScreen, &_updateRegion);
  }
  return FALSE;
}


/*
 * Send the copy region as a string of CopyRect encoded rectangles.
 * The only slightly tricky thing is that we should send the messages in
 * the correct order so that an earlier CopyRect will not corrupt the source
 * of a later one.
 */

static Bool rfbSendCopyRegion(rfbClientPtr cl, RegionPtr reg, int dx, int dy)
{
  int nrects, nrectsInBand, x_inc, y_inc, thisRect, firstInNextBand;
  int x, y, w, h;
  rfbFramebufferUpdateRectHeader rect;
  rfbCopyRect cr;
  ScreenPtr pScreen = screenInfo.screens[0];

  nrects = REGION_NUM_RECTS(reg);

  if (dx <= 0)
    x_inc = 1;
  else
    x_inc = -1;

  if (dy <= 0) {
    thisRect = 0;
    y_inc = 1;
  } else {
    thisRect = nrects - 1;
    y_inc = -1;
  }

  /* If the source region intersects the lossy region, then we know that the
     destination region is about to become lossy, so we add it to the lossy
     region. */
  if (rfbAutoLosslessRefresh > 0.0 && alrCopyRect &&
      REGION_NOTEMPTY(pScreen, reg)) {
    RegionRec tmpRegion;
    REGION_INIT(pScreen, &tmpRegion, NullBox, 0);
    REGION_COPY(pScreen, &tmpRegion, reg);
    REGION_TRANSLATE(pScreen, &tmpRegion, -dx, -dy);
    REGION_INTERSECT(pScreen, &tmpRegion, &cl->lossyRegion, &tmpRegion);
    if (REGION_NOTEMPTY(pScreen, &tmpRegion)) {
      REGION_UNION(pScreen, &cl->lossyRegion, &cl->lossyRegion, reg);
      REGION_UNION(pScreen, &cl->alrEligibleRegion, &cl->alrEligibleRegion,
                   reg);
    }
    REGION_UNINIT(pScreen, &tmpRegion);
  }

  if (reg->extents.x2 > pScreen->width || reg->extents.y2 > pScreen->height)
    rfbLog("WARNING: CopyRect dest at %d,%d with dimensions %dx%d exceeds screen boundaries\n",
           reg->extents.x1, reg->extents.y1,
           reg->extents.x2 - reg->extents.x1,
           reg->extents.y2 - reg->extents.y1);

  while (nrects > 0) {

    firstInNextBand = thisRect;
    nrectsInBand = 0;

    while ((nrects > 0) &&
           (REGION_RECTS(reg)[firstInNextBand].y1 ==
            REGION_RECTS(reg)[thisRect].y1)) {
      firstInNextBand += y_inc;
      nrects--;
      nrectsInBand++;
    }

    if (x_inc != y_inc)
      thisRect = firstInNextBand - y_inc;

    while (nrectsInBand > 0) {
      if ((ublen + sz_rfbFramebufferUpdateRectHeader + sz_rfbCopyRect) >
          UPDATE_BUF_SIZE) {
        if (!rfbSendUpdateBuf(cl))
          return FALSE;
      }

      x = REGION_RECTS(reg)[thisRect].x1;
      y = REGION_RECTS(reg)[thisRect].y1;
      w = REGION_RECTS(reg)[thisRect].x2 - x;
      h = REGION_RECTS(reg)[thisRect].y2 - y;

      if (cl->compareFB) {
        int pitch = rfbFB.paddedWidthInBytes;
        int ps = rfbServerFormat.bitsPerPixel / 8, rows = h;
        char *src = &rfbFB.pfbMemory[y * pitch + x * ps];
        char *dst = &cl->compareFB[y * pitch + x * ps];
        while (rows--) {
          memcpy(dst, src, w * ps);
          src += pitch;
          dst += pitch;
        }
        src = &rfbFB.pfbMemory[(y - dy) * pitch + (x - dx) * ps];
        dst = &cl->compareFB[(y - dy) * pitch + (x - dx) * ps];
        rows = h;
        while (rows--) {
          memcpy(dst, src, w * ps);
          src += pitch;
          dst += pitch;
        }
      }

      rect.r.x = Swap16IfLE(x);
      rect.r.y = Swap16IfLE(y);
      rect.r.w = Swap16IfLE(w);
      rect.r.h = Swap16IfLE(h);
      rect.encoding = Swap32IfLE(rfbEncodingCopyRect);

      memcpy(&updateBuf[ublen], (char *)&rect,
             sz_rfbFramebufferUpdateRectHeader);
      ublen += sz_rfbFramebufferUpdateRectHeader;

      cr.srcX = Swap16IfLE(x - dx);
      cr.srcY = Swap16IfLE(y - dy);

      memcpy(&updateBuf[ublen], (char *)&cr, sz_rfbCopyRect);
      ublen += sz_rfbCopyRect;

      cl->rfbRectanglesSent[rfbEncodingCopyRect]++;
      cl->rfbBytesSent[rfbEncodingCopyRect] +=
        sz_rfbFramebufferUpdateRectHeader + sz_rfbCopyRect;

      thisRect += x_inc;
      nrectsInBand--;
    }

    thisRect = firstInNextBand;
  }

  return TRUE;
}


/*
 * Send a given rectangle in raw encoding (rfbEncodingRaw).
 */

Bool rfbSendRectEncodingRaw(rfbClientPtr cl, int x, int y, int w, int h)
{
  rfbFramebufferUpdateRectHeader rect;
  int nlines;
  int bytesPerLine = w * (cl->format.bitsPerPixel / 8);
  char *fbptr =
    (cl->fb + (rfbFB.paddedWidthInBytes * y) + (x * (rfbFB.bitsPerPixel / 8)));

  /* Flush the buffer to guarantee correct alignment for translateFn(). */
  if (ublen > 0) {
    if (!rfbSendUpdateBuf(cl))
      return FALSE;
  }

  rect.r.x = Swap16IfLE(x);
  rect.r.y = Swap16IfLE(y);
  rect.r.w = Swap16IfLE(w);
  rect.r.h = Swap16IfLE(h);
  rect.encoding = Swap32IfLE(rfbEncodingRaw);

  memcpy(&updateBuf[ublen], (char *)&rect, sz_rfbFramebufferUpdateRectHeader);
  ublen += sz_rfbFramebufferUpdateRectHeader;

  cl->rfbRectanglesSent[rfbEncodingRaw]++;
  cl->rfbBytesSent[rfbEncodingRaw] +=
    sz_rfbFramebufferUpdateRectHeader + bytesPerLine * h;

  nlines = (UPDATE_BUF_SIZE - ublen) / bytesPerLine;

  while (TRUE) {
    if (nlines > h)
      nlines = h;

    (*cl->translateFn) (cl->translateLookupTable, &rfbServerFormat,
                        &cl->format, fbptr, &updateBuf[ublen],
                        rfbFB.paddedWidthInBytes, w, nlines);

    ublen += nlines * bytesPerLine;
    h -= nlines;

    if (h == 0)         /* rect fitted in buffer, do next one */
      return TRUE;

    /* buffer full - flush partial rect and do another nlines */

    if (!rfbSendUpdateBuf(cl))
      return FALSE;

    fbptr += (rfbFB.paddedWidthInBytes * nlines);

    nlines = (UPDATE_BUF_SIZE - ublen) / bytesPerLine;
    if (nlines == 0) {
      rfbLog("rfbSendRectEncodingRaw: send buffer too small for %d bytes per line\n",
             bytesPerLine);
      rfbCloseClient(cl);
      return FALSE;
    }
  }
}


/*
 * Send an empty rectangle with encoding field set to value of
 * rfbEncodingLastRect to notify client that this is the last
 * rectangle in framebuffer update ("LastRect" extension of RFB
 * protocol).
 */

static Bool rfbSendLastRectMarker(rfbClientPtr cl)
{
  rfbFramebufferUpdateRectHeader rect;

  if (ublen + sz_rfbFramebufferUpdateRectHeader > UPDATE_BUF_SIZE) {
    if (!rfbSendUpdateBuf(cl))
      return FALSE;
  }

  rect.encoding = Swap32IfLE(rfbEncodingLastRect);
  rect.r.x = 0;
  rect.r.y = 0;
  rect.r.w = 0;
  rect.r.h = 0;

  memcpy(&updateBuf[ublen], (char *)&rect, sz_rfbFramebufferUpdateRectHeader);
  ublen += sz_rfbFramebufferUpdateRectHeader;

  cl->rfbLastRectMarkersSent++;
  cl->rfbLastRectBytesSent += sz_rfbFramebufferUpdateRectHeader;

  return TRUE;
}


/*
 * Send the contents of updateBuf.  Returns 1 if successful, -1 if
 * not (errno should be set).
 */

Bool rfbSendUpdateBuf(rfbClientPtr cl)
{
  /*
  int i;
  for (i = 0; i < ublen; i++) {
    fprintf(stderr, "%02x ", ((unsigned char *)updateBuf)[i]);
  }
  fprintf(stderr, "\n");
  */

  if (ublen > 0)
    WRITE_OR_CLOSE(updateBuf, ublen, return FALSE);

  if (cl->captureEnable && cl->captureFD >= 0 && ublen > 0)
    rfbWriteCapture(cl->captureFD, updateBuf, ublen);

  ublen = 0;
  return TRUE;
}


/*
 * rfbSendSetColourMapEntries sends a SetColourMapEntries message to the
 * client, using values from the currently installed colormap.
 */

Bool rfbSendSetColourMapEntries(rfbClientPtr cl, int firstColour, int nColours)
{
  char buf[sz_rfbSetColourMapEntriesMsg + 256 * 3 * 2];
  rfbSetColourMapEntriesMsg *scme = (rfbSetColourMapEntriesMsg *)buf;
  CARD16 *rgb = (CARD16 *)(&buf[sz_rfbSetColourMapEntriesMsg]);
  EntryPtr pent;
  int i, len;

  scme->type = rfbSetColourMapEntries;

  scme->firstColour = Swap16IfLE(firstColour);
  scme->nColours = Swap16IfLE(nColours);

  len = sz_rfbSetColourMapEntriesMsg;

  pent = (EntryPtr)&rfbInstalledColormap->red[firstColour];
  for (i = 0; i < nColours; i++) {
    if (pent->fShared) {
      rgb[i * 3] = Swap16IfLE(pent->co.shco.red->color);
      rgb[i * 3 + 1] = Swap16IfLE(pent->co.shco.green->color);
      rgb[i * 3 + 2] = Swap16IfLE(pent->co.shco.blue->color);
    } else {
      rgb[i * 3] = Swap16IfLE(pent->co.local.red);
      rgb[i * 3 + 1] = Swap16IfLE(pent->co.local.green);
      rgb[i * 3 + 2] = Swap16IfLE(pent->co.local.blue);
    }
    pent++;
  }

  len += nColours * 3 * 2;

  WRITE_OR_CLOSE(buf, len, return FALSE);

  if (cl->captureFD >= 0)
    rfbWriteCapture(cl->captureFD, buf, len);

  return TRUE;
}


/*
 * rfbSendBell sends a Bell message to all the clients.
 */

void rfbSendBell(void)
{
  rfbClientPtr cl, nextCl;
  rfbBellMsg b;

  for (cl = rfbClientHead; cl; cl = nextCl) {
    nextCl = cl->next;
    if (cl->state != RFB_NORMAL)
      continue;
    b.type = rfbBell;
    WRITE_OR_CLOSE((char *)&b, sz_rfbBellMsg, continue);
    if (cl->captureFD >= 0)
      rfbWriteCapture(cl->captureFD, (char *)&b, sz_rfbBellMsg);
  }
}


/*
 * rfbSendDesktopSize sends a DesktopSize message to a specific client.
 */

Bool rfbSendDesktopSize(rfbClientPtr cl)
{
  rfbFramebufferUpdateRectHeader rh;
  rfbFramebufferUpdateMsg fu;

  if (!cl->enableDesktopSize)
    return TRUE;

  memset(&fu, 0, sz_rfbFramebufferUpdateMsg);
  fu.type = rfbFramebufferUpdate;
  fu.nRects = Swap16IfLE(1);
  WRITE_OR_CLOSE((char *)&fu, sz_rfbFramebufferUpdateMsg, return FALSE);

  rh.encoding = Swap32IfLE(rfbEncodingNewFBSize);
  rh.r.x = rh.r.y = 0;
  rh.r.w = Swap16IfLE(rfbFB.width);
  rh.r.h = Swap16IfLE(rfbFB.height);
  WRITE_OR_CLOSE((char *)&rh, sz_rfbFramebufferUpdateRectHeader, return FALSE);

  return TRUE;
}


/*
 * rfbSendExtDesktopSize sends an extended desktop size message to a specific
 * client.
 */

Bool rfbSendExtDesktopSize(rfbClientPtr cl)
{
  rfbFramebufferUpdateRectHeader rh;
  rfbFramebufferUpdateMsg fu;
  CARD8 numScreens[4] = { 0, 0, 0, 0 };
  rfbScreenInfo *iter;
  BOOL fakeScreen = FALSE;

  if (!cl->enableExtDesktopSize)
    return TRUE;

  memset(&fu, 0, sz_rfbFramebufferUpdateMsg);
  fu.type = rfbFramebufferUpdate;
  fu.nRects = Swap16IfLE(1);
  WRITE_OR_CLOSE((char *)&fu, sz_rfbFramebufferUpdateMsg, return FALSE);

  /* Send the ExtendedDesktopSize message, if the client supports it.
     The TigerVNC Viewer, in particular, requires this, or it won't
     enable remote desktop resize. */
  rh.encoding = Swap32IfLE(rfbEncodingExtendedDesktopSize);
  rh.r.x = Swap16IfLE(cl->reason);
  rh.r.y = Swap16IfLE(cl->result);
  rh.r.w = Swap16IfLE(rfbFB.width);
  rh.r.h = Swap16IfLE(rfbFB.height);
  WRITE_OR_CLOSE((char *)&rh, sz_rfbFramebufferUpdateRectHeader, return FALSE);

  xorg_list_for_each_entry(iter, &rfbScreens, entry) {
    if (iter->output->crtc && iter->output->crtc->mode)
      numScreens[0]++;
  }
  if (numScreens[0] < 1) {
    numScreens[0] = 1;
    fakeScreen = TRUE;
  }

  WRITE_OR_CLOSE((char *)numScreens, 4, return FALSE);

  if (fakeScreen) {
    rfbScreenInfo screen = *xorg_list_first_entry(&rfbScreens, rfbScreenInfo,
                                                  entry);
    screen.s.id = Swap32IfLE(screen.s.id);
    screen.s.x = screen.s.y = 0;
    screen.s.w = Swap16IfLE(rfbFB.width);
    screen.s.h = Swap16IfLE(rfbFB.height);
    screen.s.flags = Swap32IfLE(screen.s.flags);
    WRITE_OR_CLOSE((char *)&screen.s, sz_rfbScreenDesc, return FALSE);
  } else {
    xorg_list_for_each_entry(iter, &rfbScreens, entry) {
      rfbScreenInfo screen = *iter;

      if (screen.output->crtc && screen.output->crtc->mode) {
        screen.s.id = Swap32IfLE(screen.s.id);
        screen.s.x = Swap16IfLE(screen.s.x);
        screen.s.y = Swap16IfLE(screen.s.y);
        screen.s.w = Swap16IfLE(screen.s.w);
        screen.s.h = Swap16IfLE(screen.s.h);
        screen.s.flags = Swap32IfLE(screen.s.flags);
        WRITE_OR_CLOSE((char *)&screen.s, sz_rfbScreenDesc, return FALSE);
      }
    }
  }

  return TRUE;
}


/*
 * rfbSendQEMUExtKeyEventRect sends a pseudo-rectangle to the client to
 * acknowledge the server's support of the QEMU Extended Key Event extension.
 */

static Bool rfbSendQEMUExtKeyEventRect(rfbClientPtr cl)
{
  rfbFramebufferUpdateRectHeader rect;

  if (!cl->enableQEMUExtKeyEvent)
    return TRUE;

  if (ublen + sz_rfbFramebufferUpdateRectHeader > UPDATE_BUF_SIZE) {
    if (!rfbSendUpdateBuf(cl))
      return FALSE;
  }

  rect.encoding = Swap32IfLE(rfbEncodingQEMUExtendedKeyEvent);
  rect.r.x = 0;
  rect.r.y = 0;
  rect.r.w = 0;
  rect.r.h = 0;

  memcpy(&updateBuf[ublen], (char *)&rect, sz_rfbFramebufferUpdateRectHeader);
  ublen += sz_rfbFramebufferUpdateRectHeader;

  return rfbSendUpdateBuf(cl);
}


/*
 * rfbSendLEDState sends a pseudo-rectangle to the client with the server's
 * lock key state.
 */

static Bool rfbSendLEDState(rfbClientPtr cl)
{
  rfbFramebufferUpdateRectHeader rect;

  if (!SUPPORTS_LED_STATE(cl))
    return TRUE;
  if (cl->ledState == rfbLEDUnknown)
    return TRUE;

  rect.r.x = 0;
  rect.r.y = 0;
  rect.r.w = 0;
  rect.r.h = 0;

  if (cl->enableQEMULEDState) {

    CARD8 state = cl->ledState;

    rect.encoding = Swap32IfLE(rfbEncodingQEMULEDState);

    if (ublen + sz_rfbFramebufferUpdateRectHeader + sizeof(state) >
        UPDATE_BUF_SIZE) {
      if (!rfbSendUpdateBuf(cl))
        return FALSE;
    }

    memcpy(&updateBuf[ublen], (char *)&rect,
           sz_rfbFramebufferUpdateRectHeader);
    ublen += sz_rfbFramebufferUpdateRectHeader;
    memcpy(&updateBuf[ublen], (char *)&state, sizeof(state));
    ublen += sizeof(state);

  } else {

    CARD32 state = Swap32IfLE(cl->ledState);

    rect.encoding = Swap32IfLE((CARD32)rfbEncodingVMwareLEDState);

    if (ublen + sz_rfbFramebufferUpdateRectHeader + sizeof(state) >
        UPDATE_BUF_SIZE) {
      if (!rfbSendUpdateBuf(cl))
        return FALSE;
    }

    memcpy(&updateBuf[ublen], (char *)&rect,
           sz_rfbFramebufferUpdateRectHeader);
    ublen += sz_rfbFramebufferUpdateRectHeader;
    memcpy(&updateBuf[ublen], (char *)&state, sizeof(state));
    ublen += sizeof(state);

  }

  return rfbSendUpdateBuf(cl);
}
