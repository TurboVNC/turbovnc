/*
 * rfbserver.c - deal with server-side of the RFB protocol.
 */

/*
 *  Copyright (C) 2009-2019 D. R. Commander.  All Rights Reserved.
 *  Copyright (C) 2010 University Corporation for Atmospheric Research.
 *                     All Rights Reserved.
 *  Copyright (C) 2005-2008 Sun Microsystems, Inc.  All Rights Reserved.
 *  Copyright (C) 2004 Landmark Graphics Corporation.  All Rights Reserved.
 *  Copyright (C) 2000-2006 Constantin Kaplinsky.  All Rights Reserved.
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

/* #define GII_DEBUG */

char updateBuf[UPDATE_BUF_SIZE];
int ublen;

rfbClientPtr rfbClientHead = NULL;
rfbClientPtr pointerClient = NULL;  /* Mutex for pointer events */

Bool rfbAlwaysShared = FALSE;
Bool rfbNeverShared = FALSE;
Bool rfbDontDisconnect = TRUE;
Bool rfbViewOnly = FALSE;  /* run server in view only mode - Ehud Karni SW */
Bool rfbSyncCutBuffer = TRUE;
Bool rfbCongestionControl = TRUE;
double rfbAutoLosslessRefresh = 0.0;
int rfbALRQualityLevel = -1;
int rfbALRSubsampLevel = TVNC_1X;
int rfbCombineRect = 100;
int rfbICEBlockSize = 256;
Bool rfbInterframeDebug = FALSE;
int rfbMaxWidth = MAXSHORT, rfbMaxHeight = MAXSHORT;
int rfbMaxClipboard = MAX_CUTTEXT_LEN;
Bool rfbVirtualTablet = FALSE;
Bool rfbMT = TRUE;
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


/*
 * Session capture
 */

char *captureFile = NULL;

static void WriteCapture(int captureFD, char *buf, int len)
{
  if (write(captureFD, buf, len) < len)
    rfbLogPerror("WriteCapture: Could not write to capture file");
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

static BOOL rfbProfile = FALSE;
static double tUpdate = 0., tStart = -1., tElapsed, mpixels = 0.,
  idmpixels = 0.;
static unsigned long iter = 0;
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

static Bool putImageOnly = TRUE, alrCopyRect = TRUE;

static CARD32 alrCallback(OsTimerPtr timer, CARD32 time, pointer arg)
{
  RegionRec copyRegionSave, modifiedRegionSave, requestedRegionSave,
    ifRegionSave;
  rfbClientPtr cl = (rfbClientPtr)arg;
  int tightCompressLevelSave, tightQualityLevelSave, copyDXSave, copyDYSave,
    tightSubsampLevelSave;
  RegionRec tmpRegion;

  REGION_INIT(pScreen, &tmpRegion, NullBox, 0);
  if (putImageOnly && !cl->firstUpdate)
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

    if (!rfbSendFramebufferUpdate(cl)) return 0;

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


static CARD32 updateCallback(OsTimerPtr timer, CARD32 time, pointer arg)
{
  rfbClientPtr cl = (rfbClientPtr)arg;

  rfbSendFramebufferUpdate(cl);
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
    rfbClientRec cl;
    char temps[250];
    memset(temps, 0, 250);
    snprintf(temps, 250, "ID:%d", id);
    rfbLog("UltraVNC Repeater Mode II ID is %d\n", id);
    cl.sock = sock;
    if (WriteExact(&cl, temps, 250) < 0) {
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

  if (rfbClientHead == NULL && captureFile) {
    cl->captureFD = open(captureFile, O_CREAT | O_EXCL | O_WRONLY,
                         S_IRUSR | S_IWUSR);
    if (cl->captureFD < 0)
      rfbLogPerror("Could not open capture file");
    else
      rfbLog("Opened capture file %s\n", captureFile);
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

  cl->next = rfbClientHead;
  cl->prev = NULL;
  if (rfbClientHead)
    rfbClientHead->prev = cl;
  rfbClientHead = cl;

  rfbResetStats(cl);

  cl->zlibCompressLevel = 5;

  sprintf(pv, rfbProtocolVersionFormat, 3, 8);

  if (WriteExact(cl, pv, sz_rfbProtocolVersionMsg) < 0) {
    rfbLogPerror("rfbNewClient: write");
    rfbCloseClient(cl);
    return NULL;
  }

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
      putImageOnly = FALSE;
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

  cl->baseRTT = cl->minRTT = (unsigned)-1;
  gettimeofday(&cl->lastWrite, NULL);
  REGION_INIT(pScreen, &cl->cuRegion, NullBox, 0);

  if (rfbInterframe == 1) {
    if (!InterframeOn(cl)) {
      rfbCloseClient(cl);
      return NULL;
    }
  } else
    InterframeOff(cl);

  return cl;
}


/*
 * rfbClientConnectionGone is called from sockets.c just after a connection
 * has gone away.
 */

void rfbClientConnectionGone(rfbClientPtr cl)
{
  int i;

  if (cl->prev)
    cl->prev->next = cl->next;
  else
    rfbClientHead = cl->next;
  if (cl->next)
    cl->next->prev = cl->prev;

  TimerFree(cl->alrTimer);
  TimerFree(cl->deferredUpdateTimer);
  TimerFree(cl->updateTimer);
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

  if (pointerClient == cl)
    pointerClient = NULL;

  REGION_UNINIT(pScreen, &cl->copyRegion);
  REGION_UNINIT(pScreen, &cl->modifiedRegion);

  rfbPrintStats(cl);

  if (cl->translateLookupTable) free(cl->translateLookupTable);

  rfbFreeZrleData(cl);

  if (cl->cutText)
    free(cl->cutText);

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

  if (WriteExact(cl, buf, sz_rfbServerInitMsg + len) < 0) {
    rfbLogPerror("rfbProcessClientInitMessage: write");
    rfbCloseClient(cl);
    return;
  }

  if (cl->protocol_tightvnc)
    rfbSendInteractionCaps(cl);  /* protocol 3.7t */

  /* Dispatch client input to rfbProcessClientNormalMessage(). */
  cl->state = RFB_NORMAL;

  if (!cl->reverseConnection &&
      (rfbNeverShared || (!rfbAlwaysShared && !ci.shared))) {

    if (rfbDontDisconnect) {
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
  if (WriteExact(cl, (char *)&intr_caps,
                 sz_rfbInteractionCapsMsg) < 0 ||
      WriteExact(cl, (char *)&enc_list[0],
                 sz_rfbCapabilityInfo * N_ENC_CAPS) < 0) {
    rfbLogPerror("rfbSendInteractionCaps: write");
    rfbCloseClient(cl);
    return;
  }

  /* Dispatch client input to rfbProcessClientNormalMessage(). */
  cl->state = RFB_NORMAL;
}


/*
 * rfbProcessClientNormalMessage is called when the client has sent a normal
 * protocol message.
 */

#define READ(addr, numBytes)  \
  if ((n = ReadExact(cl, addr, numBytes)) <= 0) {  \
    if (n != 0)  \
      rfbLogPerror("rfbProcessClientNormalMessage: read");  \
    rfbCloseClient(cl);  \
    return;  \
  }

#define SKIP(numBytes)  \
  if ((n = SkipExact(cl, numBytes)) <= 0) {  \
    if (n != 0)  \
      rfbLogPerror("rfbProcessClientNormalMessage: skip");  \
    rfbCloseClient(cl);  \
    return;  \
  }

static void rfbProcessClientNormalMessage(rfbClientPtr cl)
{
  int n;
  rfbClientToServerMsg msg;
  char *str;

  READ((char *)&msg, 1)

  switch (msg.type) {

    case rfbSetPixelFormat:

      READ(((char *)&msg) + 1, sz_rfbSetPixelFormatMsg - 1)

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
      READ(((char *)&msg) + 1, sz_rfbFixColourMapEntriesMsg - 1)
      rfbLog("rfbProcessClientNormalMessage: FixColourMapEntries unsupported\n");
      rfbCloseClient(cl);
      return;

    case rfbSetEncodings:
    {
      int i;
      CARD32 enc;
      Bool firstFence = !cl->enableFence;
      Bool firstCU = !cl->enableCU;
      Bool firstGII = !cl->enableGII;
      Bool logTightCompressLevel = FALSE;

      READ(((char *)&msg) + 1, sz_rfbSetEncodingsMsg - 1)

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
        READ((char *)&enc, 4)
        enc = Swap32IfLE(enc);

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
              rfbLog("Enabling GII extension for client %s\n", cl->host);
              cl->enableGII = TRUE;
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

      if (cl->enableFence && firstFence) {
        if (!rfbSendFence(cl, rfbFenceFlagRequest, 0, NULL))
          return;
      }

      if (cl->enableCU && cl->enableFence && firstCU) {
        if (!rfbSendEndOfCU(cl))
          return;
      }

      if (cl->enableGII && firstGII) {
        /* Send GII server version message to all clients */
        rfbGIIServerVersionMsg msg;

        msg.type = rfbGIIServer;
        /* We always send as big endian to make things easier on the Java
           viewer. */
        msg.endianAndSubType = rfbGIIVersion | rfbGIIBE;
        msg.length = Swap16IfLE(sz_rfbGIIServerVersionMsg - 4);
        msg.maximumVersion = msg.minimumVersion = Swap16IfLE(1);

        if (WriteExact(cl, (char *)&msg, sz_rfbGIIServerVersionMsg) < 0) {
          rfbLogPerror("rfbProcessClientNormalMessage: write");
          rfbCloseClient(cl);
          return;
        }
      }

      return;
    }  /* rfbSetEncodings */

    case rfbFramebufferUpdateRequest:
    {
      RegionRec tmpRegion;
      BoxRec box;

      READ(((char *)&msg) + 1, sz_rfbFramebufferUpdateRequestMsg - 1)

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

      READ(((char *)&msg) + 1, sz_rfbKeyEventMsg - 1)

      if (!rfbViewOnly && !cl->viewOnly)
        KeyEvent((KeySym)Swap32IfLE(msg.ke.key), msg.ke.down);

      return;

    case rfbPointerEvent:

      cl->rfbPointerEventsRcvd++;

      READ(((char *)&msg) + 1, sz_rfbPointerEventMsg - 1)

      if (pointerClient && (pointerClient != cl))
        return;

      if (msg.pe.buttonMask == 0)
        pointerClient = NULL;
      else
        pointerClient = cl;

      if (!rfbViewOnly && !cl->viewOnly) {
        cl->cursorX = (int)Swap16IfLE(msg.pe.x);
        cl->cursorY = (int)Swap16IfLE(msg.pe.y);
        PtrAddEvent(msg.pe.buttonMask, cl->cursorX, cl->cursorY, cl);
      }
      return;

    case rfbClientCutText:
    {
      int ignoredBytes = 0;

      READ(((char *)&msg) + 1, sz_rfbClientCutTextMsg - 1)

      msg.cct.length = Swap32IfLE(msg.cct.length);
      if (msg.cct.length > rfbMaxClipboard) {
        rfbLog("Truncating %d-byte clipboard update to %d bytes.\n",
               msg.cct.length, rfbMaxClipboard);
        ignoredBytes = msg.cct.length - rfbMaxClipboard;
        msg.cct.length = rfbMaxClipboard;
      }

      if (msg.cct.length <= 0) return;
      str = (char *)malloc(msg.cct.length);
      if (str == NULL) {
        rfbLogPerror("rfbProcessClientNormalMessage: rfbClientCutText out of memory");
        rfbCloseClient(cl);
        return;
      }

      if ((n = ReadExact(cl, str, msg.cct.length)) <= 0) {
        if (n != 0)
          rfbLogPerror("rfbProcessClientNormalMessage: read");
        free(str);
        rfbCloseClient(cl);
        return;
      }

      if (ignoredBytes > 0) {
        if ((n = SkipExact(cl, ignoredBytes)) <= 0) {
          if (n != 0)
            rfbLogPerror("rfbProcessClientNormalMessage: read");
          free(str);
          rfbCloseClient(cl);
          return;
        }
      }

      /* NOTE: We do not accept cut text from a view-only client */
      if (!rfbViewOnly && !cl->viewOnly && !rfbAuthDisableCBRecv) {
        vncClientCutText(str, msg.cct.length);
        if (rfbSyncCutBuffer) rfbSetXCutText(str, msg.cct.length);
      }

      free(str);
      return;
    }

    case rfbEnableContinuousUpdates:
    {
      BoxRec box;

      READ(((char *)&msg) + 1, sz_rfbEnableContinuousUpdatesMsg - 1)

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

      READ(((char *)&msg) + 1, sz_rfbFenceMsg - 1)

      flags = Swap32IfLE(msg.f.flags);

      READ(data, msg.f.length)

      if (msg.f.length > sizeof(data))
        rfbLog("Ignoring fence.  Payload of %d bytes is too large.\n",
               msg.f.length);
      else
        HandleFence(cl, flags, msg.f.length, data);
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

      READ(((char *)&msg) + 1, sz_rfbSetDesktopSizeMsg - 1)

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

        READ((char *)&screen->s, sizeof(rfbScreenDesc))
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

      READ((char *)&endianAndSubType, 1);
      littleEndian = (endianAndSubType & rfbGIIBE) ? 0 : 1;
      subType = endianAndSubType & ~rfbGIIBE;

      switch (subType) {

        case rfbGIIVersion:

          READ((char *)&msg.giicv.length, sz_rfbGIIClientVersionMsg - 2);
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
          int i;
          rfbDevInfo dev;
          rfbGIIDeviceCreatedMsg dcmsg;

          memset(&dev, 0, sizeof(dev));
          dcmsg.deviceOrigin = 0;

          READ((char *)&msg.giidc.length, sz_rfbGIIDeviceCreateMsg - 2);
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
#ifdef GII_DEBUG
          rfbLog("    Vendor ID: %d\n", msg.giidc.vendorID);
          rfbLog("    Product ID: %d\n", msg.giidc.productID);
          rfbLog("    Event mask: %.8x\n", msg.giidc.canGenerate);
          rfbLog("    Registers: %d\n", msg.giidc.numRegisters);
          rfbLog("    Valuators: %d\n", msg.giidc.numValuators);
          rfbLog("    Buttons: %d\n", msg.giidc.numButtons);
#endif

          if (msg.giidc.length != sz_rfbGIIDeviceCreateMsg - 4 +
              msg.giidc.numValuators * sz_rfbGIIValuator) {
            rfbLog("ERROR: Malformed GII device create message\n");
            rfbCloseClient(cl);
            return;
          }

          if (msg.giidc.numButtons > MAX_BUTTONS) {
            rfbLog("GII device create ERROR: %d buttons exceeds max of %d\n",
                   msg.giidc.numButtons, MAX_BUTTONS);
            SKIP(msg.giidc.numValuators * sz_rfbGIIValuator);
            goto sendMessage;
          }
          if (msg.giidc.numValuators > MAX_VALUATORS) {
            rfbLog("GII device create ERROR: %d valuators exceeds max of %d\n",
                   msg.giidc.numValuators, MAX_VALUATORS);
            SKIP(msg.giidc.numValuators * sz_rfbGIIValuator);
            goto sendMessage;
          }

          memcpy(&dev.name, msg.giidc.deviceName, 32);
          dev.numButtons = msg.giidc.numButtons;
          dev.numValuators = msg.giidc.numValuators;
          dev.eventMask = msg.giidc.canGenerate;
          dev.mode =
            (dev.eventMask & rfbGIIValuatorAbsoluteMask) ? Absolute : Relative;
          dev.productID = msg.giidc.productID;

          if (dev.mode == Relative) {
            rfbLog("GII device create ERROR: relative valuators not supported (yet)\n");
            SKIP(msg.giidc.numValuators * sz_rfbGIIValuator);
            goto sendMessage;
          }

          for (i = 0; i < dev.numValuators; i++) {
            rfbGIIValuator *v = &dev.valuators[i];
            READ((char *)v, sz_rfbGIIValuator);
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

#ifdef GII_DEBUG
            rfbLog("    Valuator: %s (%s)\n", v->longName, v->shortName);
            rfbLog("        Index: %d\n", v->index);
            rfbLog("        Range: min = %d, center = %d, max = %d\n",
                   v->rangeMin, v->rangeCenter, v->rangeMax);
            rfbLog("        SI unit: %d\n", v->siUnit);
            rfbLog("        SI add: %d\n", v->siAdd);
            rfbLog("        SI multiply: %d\n", v->siMul);
            rfbLog("        SI divide: %d\n", v->siDiv);
            rfbLog("        SI shift: %d\n", v->siShift);
#endif
          }

          for (i = 0; i < cl->numDevices; i++) {
            if (!strcmp(dev.name, cl->devices[i].name)) {
              rfbLog("Device \'%s\' already exists with GII device ID %d\n",
                     dev.name, i + 1);
              dcmsg.deviceOrigin = Swap32IfLE(i + 1);
              goto sendMessage;
            }
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

          if (WriteExact(cl, (char *)&dcmsg, sz_rfbGIIDeviceCreatedMsg) < 0) {
            rfbLogPerror("rfbProcessClientNormalMessage: write");
            rfbCloseClient(cl);
            return;
          }

          break;
        }

        case rfbGIIDeviceDestroy:

          READ((char *)&msg.giidd.length, sz_rfbGIIDeviceDestroyMsg - 2);
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

          READ((char *)&length, sizeof(CARD16));
          if (littleEndian != *(const char *)&rfbEndianTest)
            length = Swap16(length);

          while (length > 0) {
            CARD8 eventSize, eventType;

            READ((char *)&eventSize, 1);
            READ((char *)&eventType, 1);

            switch (eventType) {

              case rfbGIIButtonPress:
              case rfbGIIButtonRelease:
              {
                rfbGIIButtonEvent b;
                rfbDevInfo *dev;

                READ((char *)&b.pad, sz_rfbGIIButtonEvent - 2);
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
#ifdef GII_DEBUG
                rfbLog("Device %d button %d %s\n", b.deviceOrigin,
                       b.buttonNumber,
                       eventType == rfbGIIButtonPress ? "PRESS" : "release");
                fflush(stderr);
#endif
                ExtInputAddEvent(dev, eventType == rfbGIIButtonPress ?
                                 ButtonPress : ButtonRelease, b.buttonNumber);
                break;
              }

              case rfbGIIValuatorRelative:
              case rfbGIIValuatorAbsolute:
              {
                rfbGIIValuatorEvent v;
                int i;
                rfbDevInfo *dev;

                READ((char *)&v.pad, sz_rfbGIIValuatorEvent - 2);
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
                if ((eventType == rfbGIIValuatorRelative &&
                     (dev->eventMask & rfbGIIValuatorRelativeMask) == 0) ||
                    (eventType == rfbGIIValuatorAbsolute &&
                     (dev->eventMask & rfbGIIValuatorAbsoluteMask) == 0)) {
                  rfbLog("ERROR: Device %d cannot generate GII valuator events\n",
                         v.deviceOrigin);
                  rfbCloseClient(cl);
                  return;
                }
                if (v.first + v.count > dev->numValuators) {
                  rfbLog("ERROR: GII valuator event for device %d exceeds valuator count (%d)\n",
                         v.deviceOrigin, dev->numValuators);
                  rfbCloseClient(cl);
                  return;
                }
#ifdef GII_DEBUG
                rfbLog("Device %d Valuator %s first=%d count=%d:\n",
                       v.deviceOrigin,
                       eventType == rfbGIIValuatorRelative ? "rel" : "ABS",
                       v.first, v.count);
#endif
                for (i = v.first; i < v.first + v.count; i++) {
                  READ((char *)&dev->values[i], sizeof(int));
                  if (littleEndian != *(const char *)&rfbEndianTest)
                    dev->values[i] = Swap32((CARD32)dev->values[i]);
#ifdef GII_DEBUG
                  fprintf(stderr, "v[%d]=%d ", i, dev->values[i]);
#endif
                }
#ifdef GII_DEBUG
                fprintf(stderr, "\n");
#endif
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
  double tUpdateStart = 0.0;

  TimerCancel(cl->updateTimer);

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

  if (rfbCongestionControl && rfbIsCongested(cl)) {
    cl->updateTimer = TimerSet(cl->updateTimer, 0, 50, updateCallback, cl);
    return TRUE;
  }

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
   * If this client understands cursor shape updates, cursor should be
   * removed from the framebuffer. Otherwise, make sure it's put up.
   */

  if (cl->enableCursorShapeUpdates) {
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
      !sendCursorPos) {
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

  if (cl->compareFB) {
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
                            !!sendCursorShape + !!sendCursorPos);
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

  if (cl->compareFB) {
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
    iter++;

    if (tElapsed > 5.) {
      rfbLog("%.2f updates/sec,  %.2f Mpixels/sec,  %.3f Mbits/sec\n",
             (double)iter / tElapsed, mpixels / tElapsed,
             (double)sendBytes / 125000. / tElapsed);
      rfbLog("Time/update:  Encode = %.3f ms,  Other = %.3f ms\n",
             tUpdate / (double)iter * 1000.,
             (tElapsed - tUpdate) / (double)iter * 1000.);
      if (cl->compareFB) {
        rfbLog("Identical Mpixels/sec:  %.2f  (%f %%)\n",
               (double)idmpixels / tElapsed, idmpixels / mpixels * 100.0);
        idmpixels = 0.;
      }
      tUpdate = 0.;
      iter = 0;
      mpixels = 0.;
      sendBytes = 0;
      tStart = gettime();
    }
  }

  if (rfbAutoLosslessRefresh > 0.0 &&
      (!putImageOnly || REGION_NOTEMPTY(pScreen, &cl->alrEligibleRegion) ||
       cl->firstUpdate)) {
    if (putImageOnly)
      REGION_UNION(pScreen, &cl->alrRegion, &cl->alrRegion,
                   &cl->alrEligibleRegion);
    REGION_EMPTY(pScreen, &cl->alrEligibleRegion);
    cl->alrTimer = TimerSet(cl->alrTimer, 0,
                            (CARD32)(rfbAutoLosslessRefresh * 1000.0),
                            alrCallback, cl);
  }

  rfbUncorkSock(cl->sock);
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

  if (ublen > 0 && WriteExact(cl, updateBuf, ublen) < 0) {
    rfbLogPerror("rfbSendUpdateBuf: write");
    rfbCloseClient(cl);
    return FALSE;
  }

  if (cl->captureEnable && cl->captureFD >= 0 && ublen > 0)
    WriteCapture(cl->captureFD, updateBuf, ublen);

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

  if (WriteExact(cl, buf, len) < 0) {
    rfbLogPerror("rfbSendSetColourMapEntries: write");
    rfbCloseClient(cl);
    return FALSE;
  }

  if (cl->captureFD >= 0)
    WriteCapture(cl->captureFD, buf, len);

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
    if (WriteExact(cl, (char *)&b, sz_rfbBellMsg) < 0) {
      rfbLogPerror("rfbSendBell: write");
      rfbCloseClient(cl);
      continue;
    }
    if (cl->captureFD >= 0)
      WriteCapture(cl->captureFD, (char *)&b, sz_rfbBellMsg);
  }
}


/*
 * rfbSendServerCutText sends a ServerCutText message to all the clients.
 */

void rfbSendServerCutText(char *str, int len)
{
  rfbClientPtr cl, nextCl;
  rfbServerCutTextMsg sct;

  if (rfbViewOnly || rfbAuthDisableCBSend || !str || len <= 0)
    return;

  for (cl = rfbClientHead; cl; cl = nextCl) {
    nextCl = cl->next;
    if (cl->state != RFB_NORMAL || cl->viewOnly)
      continue;
    if (cl->cutTextLen == len && cl->cutText && !memcmp(cl->cutText, str, len))
      continue;
    if (cl->cutText)
      free(cl->cutText);
    cl->cutText = rfbAlloc(len);
    memcpy(cl->cutText, str, len);
    cl->cutTextLen = len;
    memset(&sct, 0, sz_rfbServerCutTextMsg);
    sct.type = rfbServerCutText;
    sct.length = Swap32IfLE(len);
    if (WriteExact(cl, (char *)&sct, sz_rfbServerCutTextMsg) < 0) {
      rfbLogPerror("rfbSendServerCutText: write");
      rfbCloseClient(cl);
      continue;
    }
    if (WriteExact(cl, str, len) < 0) {
      rfbLogPerror("rfbSendServerCutText: write");
      rfbCloseClient(cl);
      continue;
    }
    if (cl->captureFD >= 0)
      WriteCapture(cl->captureFD, str, len);
  }
  LogMessage(X_DEBUG, "Sent server clipboard: '%.*s%s' (%d bytes)\n",
             len <= 20 ? len : 20, str, len <= 20 ? "" : "...", len);
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
  if (WriteExact(cl, (char *)&fu, sz_rfbFramebufferUpdateMsg) < 0) {
    rfbLogPerror("rfbSendDesktopSize: write");
    rfbCloseClient(cl);
    return FALSE;
  }

  rh.encoding = Swap32IfLE(rfbEncodingNewFBSize);
  rh.r.x = rh.r.y = 0;
  rh.r.w = Swap16IfLE(rfbFB.width);
  rh.r.h = Swap16IfLE(rfbFB.height);
  if (WriteExact(cl, (char *)&rh, sz_rfbFramebufferUpdateRectHeader) < 0) {
    rfbLogPerror("rfbSendDesktopSize: write");
    rfbCloseClient(cl);
    return FALSE;
  }

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
  /* Error messages can only be sent with the EDS extension */
  if (!cl->enableExtDesktopSize && cl->result != rfbEDSResultSuccess)
    return TRUE;

  memset(&fu, 0, sz_rfbFramebufferUpdateMsg);
  fu.type = rfbFramebufferUpdate;
  fu.nRects = Swap16IfLE(1);
  if (WriteExact(cl, (char *)&fu, sz_rfbFramebufferUpdateMsg) < 0) {
    rfbLogPerror("rfbSendExtDesktopSize: write");
    rfbCloseClient(cl);
    return FALSE;
  }

  /* Send the ExtendedDesktopSize message, if the client supports it.
     The TigerVNC Viewer, in particular, requires this, or it won't
     enable remote desktop resize. */
  rh.encoding = Swap32IfLE(rfbEncodingExtendedDesktopSize);
  rh.r.x = Swap16IfLE(cl->reason);
  rh.r.y = Swap16IfLE(cl->result);
  rh.r.w = Swap16IfLE(rfbFB.width);
  rh.r.h = Swap16IfLE(rfbFB.height);
  if (WriteExact(cl, (char *)&rh, sz_rfbFramebufferUpdateRectHeader) < 0) {
    rfbLogPerror("rfbSendExtDesktopSize: write");
    rfbCloseClient(cl);
    return FALSE;
  }

  xorg_list_for_each_entry(iter, &rfbScreens, entry) {
    if (iter->output->crtc && iter->output->crtc->mode)
      numScreens[0]++;
  }
  if (numScreens[0] < 1) {
    numScreens[0] = 1;
    fakeScreen = TRUE;
  }

  if (WriteExact(cl, (char *)numScreens, 4) < 0) {
    rfbLogPerror("rfbSendExtDesktopSize: write");
    rfbCloseClient(cl);
    return FALSE;
  }

  if (fakeScreen) {
    rfbScreenInfo screen = *xorg_list_first_entry(&rfbScreens, rfbScreenInfo,
                                                  entry);
    screen.s.id = Swap32IfLE(screen.s.id);
    screen.s.x = screen.s.y = 0;
    screen.s.w = Swap16IfLE(rfbFB.width);
    screen.s.h = Swap16IfLE(rfbFB.height);
    screen.s.flags = Swap32IfLE(screen.s.flags);
    if (WriteExact(cl, (char *)&screen.s, sz_rfbScreenDesc) < 0) {
      rfbLogPerror("rfbSendExtDesktopSize: write");
      rfbCloseClient(cl);
      return FALSE;
    }
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
        if (WriteExact(cl, (char *)&screen.s, sz_rfbScreenDesc) < 0) {
          rfbLogPerror("rfbSendExtDesktopSize: write");
          rfbCloseClient(cl);
          return FALSE;
        }
      }
    }
  }

  return TRUE;
}


/*****************************************************************************
 *
 * UDP can be used for keyboard and pointer events when the underlying
 * network is highly reliable.  This is really here to support ORL's
 * videotile, whose TCP implementation doesn't like sending lots of small
 * packets (such as 100s of pen readings per second!).
 */

void rfbNewUDPConnection(int sock)
{
  if (write(sock, &ptrAcceleration, 1) < 0)
    rfbLogPerror("rfbNewUDPConnection: write");
}

/*
 * Because UDP is a message based service, we can't read the first byte and
 * then the rest of the packet separately like we do with TCP.  We will always
 * get a whole packet delivered in one go, so we ask read() for the maximum
 * number of bytes we can possibly get.
 */

void rfbProcessUDPInput(int sock)
{
  int n;
  rfbClientToServerMsg msg;

  if ((n = read(sock, (char *)&msg, sizeof(msg))) <= 0) {
    if (n < 0)
      rfbLogPerror("rfbProcessUDPInput: read");
    rfbDisconnectUDPSock();
    return;
  }

  switch (msg.type) {
    case rfbKeyEvent:
      if (n != sz_rfbKeyEventMsg) {
        rfbLog("rfbProcessUDPInput: key event incorrect length\n");
        rfbDisconnectUDPSock();
        return;
      }
      if (!rfbViewOnly)
        KeyEvent((KeySym)Swap32IfLE(msg.ke.key), msg.ke.down);
      break;

    case rfbPointerEvent:
      if (n != sz_rfbPointerEventMsg) {
        rfbLog("rfbProcessUDPInput: ptr event incorrect length\n");
        rfbDisconnectUDPSock();
        return;
      }
      if (!rfbViewOnly)
        PtrAddEvent(msg.pe.buttonMask, Swap16IfLE(msg.pe.x),
                    Swap16IfLE(msg.pe.y), 0);
      break;

    default:
      rfbLog("rfbProcessUDPInput: unknown message type %d\n", msg.type);
      rfbDisconnectUDPSock();
  }
}
