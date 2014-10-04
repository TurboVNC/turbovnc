/*
 * rfbserver.c - deal with server-side of the RFB protocol.
 */

/*
 *  Copyright (C) 2009-2014 D. R. Commander.  All Rights Reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "windowstr.h"
#include "rfb.h"
#include "input.h"
#include "mipointer.h"
#include "sprite.h"

char updateBuf[UPDATE_BUF_SIZE];
int ublen;

rfbClientPtr rfbClientHead = NULL;
rfbClientPtr pointerClient = NULL;  /* Mutex for pointer events */

Bool rfbAlwaysShared = FALSE;
Bool rfbNeverShared = FALSE;
Bool rfbDontDisconnect = FALSE;
Bool rfbViewOnly = FALSE; /* run server in view only mode - Ehud Karni SW */
Bool rfbSyncCutBuffer = TRUE;
Bool rfbCongestionControl = TRUE;
double rfbAutoLosslessRefresh = 0.0;
int rfbALRQualityLevel = -1;
int rfbALRSubsampLevel = TVNC_1X;
int rfbCombineRect = 100;
Bool rfbInterframeDebug = FALSE;

static rfbClientPtr rfbNewClient(int sock);
static void rfbProcessClientProtocolVersion(rfbClientPtr cl);
static void rfbProcessClientInitMessage(rfbClientPtr cl);
static void rfbSendInteractionCaps(rfbClientPtr cl);
static void rfbProcessClientNormalMessage(rfbClientPtr cl);
static Bool rfbSendCopyRegion(rfbClientPtr cl, RegionPtr reg, int dx, int dy);
static Bool rfbSendLastRectMarker(rfbClientPtr cl);


/*
 * Idle timeout
 */

CARD32 rfbMaxIdleTimeout = 0;
CARD32 rfbIdleTimeout = 0;
OsTimerPtr idleTimer;

CARD64
idleTimeoutCallback(OsTimerPtr timer, CARD64 time, pointer arg)
{
    FatalError("TurboVNC server has been idle for %d seconds.  Exiting.\n",
               rfbIdleTimeout);
    return 0;
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
    return((double)__tv.tv_sec + (double)__tv.tv_usec * 0.000001);
}


/*
 * Auto Lossless Refresh
 */

static Bool putImageOnly = TRUE, alrCopyRect = TRUE;

CARD64
alrCallback(OsTimerPtr timer, CARD64 time, pointer arg)
{
    RegionRec copyRegionSave, modifiedRegionSave, requestedRegionSave,
        ifRegionSave;
    rfbClientPtr cl = (rfbClientPtr)arg;
    int tightCompressLevelSave, tightQualityLevelSave, copyDXSave, copyDYSave,
        tightSubsampLevelSave;
    RegionRec tmpRegion;

    SAFE_REGION_INIT(pScreen, &tmpRegion, NullBox, 0);
    if (putImageOnly && !cl->firstUpdate)
        REGION_INTERSECT(pScreen, &tmpRegion, &cl->alrRegion,
                         &cl->lossyRegion);
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
            REGION_UNION(pScreen, &cl->ifRegion, &cl->ifRegion,
                         &tmpRegion);
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


static CARD64
updateCallback(OsTimerPtr timer, CARD64 time, pointer arg)
{
    rfbClientPtr cl = (rfbClientPtr)arg;
    rfbSendFramebufferUpdate(cl);
    return 0;
}


/*
 * Interframe comparison
 */

int rfbInterframe = -1;  /* -1 = auto (determined by compression level) */

static Bool
InterframeOn(rfbClientPtr cl)
{
    if (!cl->compareFB) {
        if (!(cl->compareFB = (char *)xalloc(rfbScreen.paddedWidthInBytes *
                                             rfbScreen.height))) {
            rfbLogPerror("InterframeOn: couldn't allocate comparison buffer");
            return FALSE;
        }
        memset(cl->compareFB, 0, rfbScreen.paddedWidthInBytes *
               rfbScreen.height);
        REGION_INIT(pScreen, &cl->ifRegion, NullBox, 0);
        cl->firstCompare = TRUE;
        rfbLog("Interframe comparison enabled\n");
    }
    cl->fb = cl->compareFB;
    return TRUE;
}

static void
InterframeOff(rfbClientPtr cl)
{
    if (cl->compareFB) {
        xfree(cl->compareFB);
        REGION_UNINIT(pScreen, &cl->ifRegion);
        rfbLog("Interframe comparison disabled\n");
    }
    cl->compareFB = NULL;
    cl->fb = rfbScreen.pfbMemory;
    return;
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

void
rfbNewClientConnection(sock)
    int sock;
{
    rfbNewClient(sock);
}


/*
 * rfbReverseConnection is called to make an outward connection to a
 * "listening" RFB client.
 */

rfbClientPtr
rfbReverseConnection(host, port)
    char *host;
    int port;
{
    int sock;
    rfbClientPtr cl;

    if (rfbAuthDisableRevCon) {
        rfbLog("Reverse connections disabled\n");
        return (rfbClientPtr)NULL;
    }

    if ((sock = rfbConnect(host, port)) < 0)
        return (rfbClientPtr)NULL;

    cl = rfbNewClient(sock);

    if (cl) {
        cl->reverseConnection = TRUE;
    }

    return cl;
}


/*
 * rfbNewClient is called when a new connection has been made by whatever
 * means.
 */

static rfbClientPtr
rfbNewClient(sock)
    int sock;
{
    rfbProtocolVersionMsg pv;
    rfbClientPtr cl;
    BoxRec box;
    struct sockaddr_storage addr;
    socklen_t addrlen = sizeof(struct sockaddr_storage);
    char addrStr[INET6_ADDRSTRLEN];
    char *env = NULL;

    if (rfbClientHead == NULL) {
        /* no other clients - make sure we don't think any keys are pressed */
        KbdReleaseAllKeys();
    } else {
        rfbLog("  (other clients");
        for (cl = rfbClientHead; cl; cl = cl->next) {
            fprintf(stderr, " %s", cl->host);
        }
        fprintf(stderr, ")\n");
    }

    cl = (rfbClientPtr)xalloc(sizeof(rfbClientRec));
    if (cl == NULL) {
        rfbLog("rfbNewClient: out of memory");
        rfbCloseSock(cl->sock);
        return NULL;
    }

    memset(cl, 0, sizeof(rfbClientRec));

    cl->sock = sock;
    getpeername(sock, (struct sockaddr *)&addr, &addrlen);
    cl->host = strdup(sockaddr_string(&addr, addrStr, INET6_ADDRSTRLEN));

    /* Dispatch client input to rfbProcessClientProtocolVersion(). */
    cl->state = RFB_PROTOCOL_VERSION;

    cl->preferredEncoding = rfbEncodingTight;
    cl->correMaxWidth = 48;
    cl->correMaxHeight = 48;

    REGION_INIT(pScreen, &cl->copyRegion, NullBox, 0);

    box.x1 = box.y1 = 0;
    box.x2 = rfbScreen.width;
    box.y2 = rfbScreen.height;
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
    rfbClientHead = cl;

    rfbResetStats(cl);

    cl->zlibCompressLevel = 5;

    sprintf(pv, rfbProtocolVersionFormat, 3, 8);

    if (WriteExact(sock, pv, sz_rfbProtocolVersionMsg) < 0) {
        rfbLogPerror("rfbNewClient: write");
        rfbCloseSock(sock);
        return NULL;
    }

    if ((env = getenv("TVNC_PROFILE")) != NULL && !strcmp(env, "1"))
        rfbProfile = TRUE;

    if ((env = getenv("TVNC_IFDEBUG")) != NULL && !strcmp(env, "1"))
        rfbInterframeDebug = TRUE;

    if ((env = getenv("TVNC_COMBINERECT")) != NULL) {
        int combine = atoi(env);
        if (combine > 0 && combine <= 65000) rfbCombineRect = combine;
    }

    cl->firstUpdate = TRUE;

    if (rfbAutoLosslessRefresh > 0.0) {
        REGION_INIT(pScreen, &cl->lossyRegion, NullBox, 0);
        if ((env = getenv("TVNC_ALRALL")) != NULL && !strcmp(env, "1"))
            putImageOnly = FALSE;
        if ((env = getenv("TVNC_ALRCOPYRECT")) != NULL && !strcmp(env, "0"))
            alrCopyRect = FALSE;
        REGION_INIT(pScreen, &cl->alrRegion, NullBox, 0);
        REGION_INIT(pScreen, &cl->alrEligibleRegion, NullBox, 0);
    }

    if (rfbIdleTimeout > 0) {
        TimerCancel(idleTimer);
    }

    cl->baseRTT = cl->minRTT = (unsigned)-1;
    gettimeofday(&cl->lastWrite, NULL);
    REGION_INIT(pScreen, &cl->cuRegion, NullBox, 0);

    if (rfbInterframe == 1) {
        if (!InterframeOn(cl)) {
            rfbCloseSock(sock);
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

void
rfbClientConnectionGone(sock)
    int sock;
{
    rfbClientPtr cl, prev;
    int i;

    for (prev = NULL, cl = rfbClientHead; cl; prev = cl, cl = cl->next) {
        if (sock == cl->sock)
            break;
    }

    if (!cl) {
        rfbLog("rfbClientConnectionGone: unknown socket %d\n", sock);
        return;
    }

    TimerFree(cl->alrTimer);
    TimerFree(cl->deferredUpdateTimer);
    TimerFree(cl->updateTimer);
    TimerFree(cl->congestionTimer);

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
    if ( cl->compStreamInited == TRUE ) {
        deflateEnd( &(cl->compStream) );
    }

    for (i = 0; i < 4; i++) {
        if (cl->zsActive[i])
            deflateEnd(&cl->zsStruct[i]);
    }

    if (pointerClient == cl)
        pointerClient = NULL;

    if (prev)
        prev->next = cl->next;
    else
        rfbClientHead = cl->next;

    REGION_UNINIT(pScreen, &cl->copyRegion);
    REGION_UNINIT(pScreen, &cl->modifiedRegion);

    rfbPrintStats(cl);

    if (cl->translateLookupTable) free(cl->translateLookupTable);

    rfbFreeZrleData(cl);

    if (cl->cutText)
        xfree(cl->cutText);

    InterframeOff(cl);

    xfree(cl);

    if (rfbClientHead == NULL && rfbIdleTimeout > 0) {
        idleTimer = TimerSet(idleTimer, 0, rfbIdleTimeout * 1000,
                             idleTimeoutCallback, NULL);
    }
}


/*
 * rfbProcessClientMessage is called when there is data to read from a client.
 */

void
rfbProcessClientMessage(sock)
    int sock;
{
    rfbClientPtr cl, cl2;

    for (cl = rfbClientHead; cl; cl = cl->next) {
        if (sock == cl->sock)
            break;
    }

    if (!cl) {
        rfbLog("rfbProcessClientMessage: unknown socket %d\n", sock);
        rfbCloseSock(sock);
        return;
    }

    rfbCorkSock(sock);

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

    /* Make sure cl hasn't been freed */
    for (cl2 = rfbClientHead; cl2; cl2 = cl2->next) {
        if (cl2 == cl)
            break;
    }
    if (cl2 == NULL) return;

    if (cl->syncFence) {
      rfbSendFence(cl, cl->fenceFlags, cl->fenceDataLen, cl->fenceData);
      cl->syncFence = FALSE;
    }

    rfbUncorkSock(sock);
}


/*
 * rfbProcessClientProtocolVersion is called when the client sends its
 * protocol version.
 */

static void
rfbProcessClientProtocolVersion(cl)
    rfbClientPtr cl;
{
    rfbProtocolVersionMsg pv;
    int n, major, minor;

    if ((n = ReadExact(cl->sock, pv, sz_rfbProtocolVersionMsg)) <= 0) {
        if (n == 0)
            rfbLog("rfbProcessClientProtocolVersion: client gone\n");
        else
            rfbLogPerror("rfbProcessClientProtocolVersion: read");
        rfbCloseSock(cl->sock);
        return;
    }

    pv[sz_rfbProtocolVersionMsg] = 0;
    if (sscanf(pv, rfbProtocolVersionFormat, &major, &minor) != 2) {
        rfbLog("rfbProcessClientProtocolVersion: not a valid RFB client\n");
        rfbCloseSock(cl->sock);
        return;
    }
    if (major != 3) {
        rfbLog("Unsupported protocol version %d.%d\n", major, minor);
        rfbCloseSock(cl->sock);
        return;
    }

    /* Always use one of the three standard versions of the RFB protocol. */
    cl->protocol_minor_ver = minor;
    if (minor > 8) {            /* buggy client */
        cl->protocol_minor_ver = 8;
    } else if (minor > 3 && minor < 7) { /* non-standard client */
        cl->protocol_minor_ver = 3;
    } else if (minor < 3) {     /* ancient client */
        cl->protocol_minor_ver = 3;
    }
    if (cl->protocol_minor_ver != minor) {
        rfbLog("Non-standard protocol version 3.%d, using 3.%d instead\n",
               minor, cl->protocol_minor_ver);
    } else {
        rfbLog("Using protocol version 3.%d\n", cl->protocol_minor_ver);
    }

    /* TightVNC protocol extensions are not enabled yet. */
    cl->protocol_tightvnc = FALSE;

    rfbAuthNewClient(cl);
}


/*
 * rfbProcessClientInitMessage is called when the client sends its
 * initialisation message.
 */

static void
rfbProcessClientInitMessage(cl)
    rfbClientPtr cl;
{
    rfbClientInitMsg ci;
    char buf[256];
    rfbServerInitMsg *si = (rfbServerInitMsg *)buf;
    int len, n;
    rfbClientPtr otherCl, nextCl;

    if ((n = ReadExact(cl->sock, (char *)&ci, sz_rfbClientInitMsg)) <= 0) {
        if (n == 0)
            rfbLog("rfbProcessClientInitMessage: client gone\n");
        else
            rfbLogPerror("rfbProcessClientInitMessage: read");
        rfbCloseSock(cl->sock);
        return;
    }

    si->framebufferWidth = Swap16IfLE(rfbScreen.width);
    si->framebufferHeight = Swap16IfLE(rfbScreen.height);
    si->format = rfbServerFormat;
    si->format.redMax = Swap16IfLE(si->format.redMax);
    si->format.greenMax = Swap16IfLE(si->format.greenMax);
    si->format.blueMax = Swap16IfLE(si->format.blueMax);

    if (strlen(desktopName) > 128)      /* sanity check on desktop name len */
        desktopName[128] = 0;

    sprintf(buf + sz_rfbServerInitMsg, "%s", desktopName);

    len = strlen(buf + sz_rfbServerInitMsg);
    si->nameLength = Swap32IfLE(len);

    if (WriteExact(cl->sock, buf, sz_rfbServerInitMsg + len) < 0) {
        rfbLogPerror("rfbProcessClientInitMessage: write");
        rfbCloseSock(cl->sock);
        return;
    }

    if (cl->protocol_tightvnc)
        rfbSendInteractionCaps(cl); /* protocol 3.7t */

    /* Dispatch client input to rfbProcessClientNormalMessage(). */
    cl->state = RFB_NORMAL;

    if (!cl->reverseConnection &&
        (rfbNeverShared || (!rfbAlwaysShared && !ci.shared))) {

        if (rfbDontDisconnect) {
            for (otherCl = rfbClientHead; otherCl; otherCl = otherCl->next) {
                if ((otherCl != cl) && (otherCl->state == RFB_NORMAL)) {
                    rfbLog("-dontdisconnect: Not shared & existing client\n");
                    rfbLog("  refusing new client %s\n", cl->host);
                    rfbCloseSock(cl->sock);
                    return;
                }
            }
        } else {
            for (otherCl = rfbClientHead; otherCl; otherCl = nextCl) {
                nextCl = otherCl->next;
                if ((otherCl != cl) && (otherCl->state == RFB_NORMAL)) {
                    rfbLog("Not shared - closing connection to client %s\n",
                           otherCl->host);
                    rfbCloseSock(otherCl->sock);
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
#define N_ENC_CAPS  16

void
rfbSendInteractionCaps(cl)
    rfbClientPtr cl;
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
        rfbCloseSock(cl->sock);
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
        rfbCloseSock(cl->sock);
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
    if (i != N_ENC_CAPS) {
        rfbLog("rfbSendInteractionCaps: assertion failed, i != N_ENC_CAPS\n");
        rfbCloseSock(cl->sock);
        return;
    }

    /* Send header and capability lists */
    if (WriteExact(cl->sock, (char *)&intr_caps,
                   sz_rfbInteractionCapsMsg) < 0 ||
        WriteExact(cl->sock, (char *)&enc_list[0],
                   sz_rfbCapabilityInfo * N_ENC_CAPS) < 0) {
        rfbLogPerror("rfbSendInteractionCaps: write");
        rfbCloseSock(cl->sock);
        return;
    }

    /* Dispatch client input to rfbProcessClientNormalMessage(). */
    cl->state = RFB_NORMAL;
}


/*
 * rfbProcessClientNormalMessage is called when the client has sent a normal
 * protocol message.
 */

static void
rfbProcessClientNormalMessage(cl)
    rfbClientPtr cl;
{
    int n;
    rfbClientToServerMsg msg;
    char *str;

    if ((n = ReadExact(cl->sock, (char *)&msg, 1)) <= 0) {
        if (n != 0)
            rfbLogPerror("rfbProcessClientNormalMessage: read");
        rfbCloseSock(cl->sock);
        return;
    }

    switch (msg.type) {

    case rfbSetPixelFormat:

        if ((n = ReadExact(cl->sock, ((char *)&msg) + 1,
                           sz_rfbSetPixelFormatMsg - 1)) <= 0) {
            if (n != 0)
                rfbLogPerror("rfbProcessClientNormalMessage: read");
            rfbCloseSock(cl->sock);
            return;
        }

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
        if ((n = ReadExact(cl->sock, ((char *)&msg) + 1,
                           sz_rfbFixColourMapEntriesMsg - 1)) <= 0) {
            if (n != 0)
                rfbLogPerror("rfbProcessClientNormalMessage: read");
            rfbCloseSock(cl->sock);
            return;
        }
        rfbLog("rfbProcessClientNormalMessage: %s",
                "FixColourMapEntries unsupported\n");
        rfbCloseSock(cl->sock);
        return;


    case rfbSetEncodings:
    {
        int i;
        CARD32 enc;
        Bool firstFence = !cl->enableFence;
        Bool firstCU = !cl->enableCU;
        Bool logTightCompressLevel = FALSE;

        if ((n = ReadExact(cl->sock, ((char *)&msg) + 1,
                           sz_rfbSetEncodingsMsg - 1)) <= 0) {
            if (n != 0)
                rfbLogPerror("rfbProcessClientNormalMessage: read");
            rfbCloseSock(cl->sock);
            return;
        }

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
            if ((n = ReadExact(cl->sock, (char *)&enc, 4)) <= 0) {
                if (n != 0)
                    rfbLogPerror("rfbProcessClientNormalMessage: read");
                rfbCloseSock(cl->sock);
                return;
            }
            enc = Swap32IfLE(enc);

            switch (enc) {

            case rfbEncodingCopyRect:
                cl->useCopyRect = TRUE;
                break;
            case rfbEncodingRaw:
                if (cl->preferredEncoding == -1) {
                    cl->preferredEncoding = enc;
                    rfbLog("Using raw encoding for client %s\n",
                           cl->host);
                }
                break;
            case rfbEncodingRRE:
                if (cl->preferredEncoding == -1) {
                    cl->preferredEncoding = enc;
                    rfbLog("Using rre encoding for client %s\n",
                           cl->host);
                }
                break;
            case rfbEncodingCoRRE:
                if (cl->preferredEncoding == -1) {
                    cl->preferredEncoding = enc;
                    rfbLog("Using CoRRE encoding for client %s\n",
                           cl->host);
                }
                break;
            case rfbEncodingHextile:
                if (cl->preferredEncoding == -1) {
                    cl->preferredEncoding = enc;
                    rfbLog("Using hextile encoding for client %s\n",
                           cl->host);
                }
                break;
            case rfbEncodingZlib:
                if (cl->preferredEncoding == -1) {
                    cl->preferredEncoding = enc;
                    rfbLog("Using zlib encoding for client %s\n",
                           cl->host);
                }
              break;
            case rfbEncodingZRLE:
                if (cl->preferredEncoding == -1) {
                    cl->preferredEncoding = enc;
                    rfbLog("Using ZRLE encoding for client %s\n",
                           cl->host);
                }
              break;
            case rfbEncodingZYWRLE:
                if (cl->preferredEncoding == -1) {
                    cl->preferredEncoding = enc;
                    rfbLog("Using ZYWRLE encoding for client %s\n",
                           cl->host);
                }
              break;
            case rfbEncodingTight:
                if (cl->preferredEncoding == -1) {
                    cl->preferredEncoding = enc;
                    rfbLog("Using tight encoding for client %s\n",
                           cl->host);
                }
                break;
            case rfbEncodingXCursor:
                if (!cl->enableCursorShapeUpdates) {
                    rfbLog("Enabling X-style cursor updates for client %s\n",
                           cl->host);
                    cl->enableCursorShapeUpdates = TRUE;
                    cl->useRichCursorEncoding = FALSE;
                    cl->cursorWasChanged = TRUE;
                    break;
                }
            case rfbEncodingRichCursor:
                if (!cl->enableCursorShapeUpdates) {
                    rfbLog("Enabling full-color cursor updates for client "
                           "%s\n", cl->host);
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
                    rfbLog("Enabling LastRect protocol extension for client "
                           "%s\n", cl->host);
                    cl->enableLastRectEncoding = TRUE;
                }
                break;
            case rfbEncodingFence:
                if (!cl->enableFence) {
                    rfbLog("Enabling Fence protocol extension for client "
                           "%s\n", cl->host);
                    cl->enableFence = TRUE;
                }
                break;
            case rfbEncodingContinuousUpdates:
                if (!cl->enableCU) {
                    rfbLog("Enabling Continuous Updates protocol extension for client "
                           "%s\n", cl->host);
                    cl->enableCU = TRUE;
                }
                break;
            default:
                if ( enc >= (CARD32)rfbEncodingCompressLevel0 &&
                     enc <= (CARD32)rfbEncodingCompressLevel9 ) {
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
                                rfbCloseSock(cl->sock);
                                return;
                            }
                        } else
                            InterframeOff(cl);
                    }
                } else if ( enc >= (CARD32)rfbEncodingSubsamp1X &&
                         enc <= (CARD32)rfbEncodingSubsampGray ) {
                    cl->tightSubsampLevel = enc & 0xFF;
                    rfbLog("Using JPEG subsampling %d for client %s\n",
                           cl->tightSubsampLevel, cl->host);
                } else if ( enc >= (CARD32)rfbEncodingQualityLevel0 &&
                            enc <= (CARD32)rfbEncodingQualityLevel9 ) {
                    cl->tightQualityLevel = JPEG_QUAL[enc & 0x0F];
                    cl->tightSubsampLevel = JPEG_SUBSAMP[enc & 0x0F];
                    cl->imageQualityLevel = enc & 0x0F;
                    if (cl->preferredEncoding == rfbEncodingTight)
                        rfbLog("Using JPEG subsampling %d, Q%d for client %s\n",
                            cl->tightSubsampLevel, cl->tightQualityLevel,
                            cl->host);
                    else
                        rfbLog("Using image quality level %d for client %s\n",
                            cl->imageQualityLevel, cl->host);
                } else if ( enc >= (CARD32)rfbEncodingFineQualityLevel0 + 1 &&
                            enc <= (CARD32)rfbEncodingFineQualityLevel100 ) {
                    cl->tightQualityLevel = enc & 0xFF;
                    rfbLog("Using JPEG quality %d for client %s\n",
                           cl->tightQualityLevel, cl->host);
                } else {
                    rfbLog("rfbProcessClientNormalMessage: ignoring unknown "
                           "encoding %d (%x)\n", (int)enc, (int)enc);
                }
            }
        }

        if (cl->preferredEncoding == -1) {
            cl->preferredEncoding = rfbEncodingTight;
        }

        if (cl->preferredEncoding == rfbEncodingTight &&
            logTightCompressLevel)
            rfbLog("Using Tight compression level %d for client %s\n",
                   rfbTightCompressLevel(cl), cl->host);

        if (cl->enableCursorPosUpdates && !cl->enableCursorShapeUpdates) {
            rfbLog("Disabling cursor position updates for client %s\n",
                   cl->host);
            cl->enableCursorPosUpdates = FALSE;
        }

        if (cl->enableFence && firstFence)
            rfbSendFence(cl, rfbFenceFlagRequest, 0, NULL);

        if (cl->enableCU && cl->enableFence && firstCU)
            rfbSendEndOfCU(cl);

        return;
    }


    case rfbFramebufferUpdateRequest:
    {
        RegionRec tmpRegion;
        BoxRec box;

        if ((n = ReadExact(cl->sock, ((char *)&msg) + 1,
                           sz_rfbFramebufferUpdateRequestMsg-1)) <= 0) {
            if (n != 0)
                rfbLogPerror("rfbProcessClientNormalMessage: read");
            rfbCloseSock(cl->sock);
            return;
        }

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
            REGION_SUBTRACT(pScreen, &cl->copyRegion, &cl->copyRegion,
                            &tmpRegion);
            REGION_UNION(pScreen, &cl->ifRegion, &cl->ifRegion, &tmpRegion);
        }

        if (FB_UPDATE_PENDING(cl) &&
            (!cl->deferredUpdateScheduled || rfbDeferUpdateTime == 0 ||
             gettime() - cl->deferredUpdateStart >= (double)rfbDeferUpdateTime)) {
            rfbSendFramebufferUpdate(cl);
            cl->deferredUpdateScheduled = FALSE;
        }

        REGION_UNINIT(pScreen, &tmpRegion);
        return;
    }

    case rfbKeyEvent:

        cl->rfbKeyEventsRcvd++;

        if ((n = ReadExact(cl->sock, ((char *)&msg) + 1,
                           sz_rfbKeyEventMsg - 1)) <= 0) {
            if (n != 0)
                rfbLogPerror("rfbProcessClientNormalMessage: read");
            rfbCloseSock(cl->sock);
            return;
        }

        if (!rfbViewOnly && !cl->viewOnly) {
            KbdAddEvent(msg.ke.down, (KeySym)Swap32IfLE(msg.ke.key), cl);
        }
        return;


    case rfbPointerEvent:

        cl->rfbPointerEventsRcvd++;

        if ((n = ReadExact(cl->sock, ((char *)&msg) + 1,
                           sz_rfbPointerEventMsg - 1)) <= 0) {
            if (n != 0)
                rfbLogPerror("rfbProcessClientNormalMessage: read");
            rfbCloseSock(cl->sock);
            return;
        }

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

        if ((n = ReadExact(cl->sock, ((char *)&msg) + 1,
                           sz_rfbClientCutTextMsg - 1)) <= 0) {
            if (n != 0)
                rfbLogPerror("rfbProcessClientNormalMessage: read");
            rfbCloseSock(cl->sock);
            return;
        }

        msg.cct.length = Swap32IfLE(msg.cct.length);
        if (msg.cct.length > MAX_CUTTEXT_LEN) {
            rfbLog("Ignoring %d-byte clipboard update from client.  Max is %d bytes.\n",
                   msg.cct.length, MAX_CUTTEXT_LEN);
            if ((n = SkipExact(cl->sock, msg.cct.length)) <= 0) {
                if (n != 0)
                    rfbLogPerror("rfbProcessClientNormalMessage: read");
                rfbCloseSock(cl->sock);
                return;
            }
            return;
        }

        if (msg.cct.length <= 0) return;
        str = (char *)xalloc(msg.cct.length);
        if (str == NULL) {
            rfbLogPerror("rfbProcessClientNormalMessage: rfbClientCutText out of memory");
            rfbCloseSock(cl->sock);
            return;
        }

        if ((n = ReadExact(cl->sock, str, msg.cct.length)) <= 0) {
            if (n != 0)
                rfbLogPerror("rfbProcessClientNormalMessage: read");
            xfree(str);
            rfbCloseSock(cl->sock);
            return;
        }

        /* NOTE: We do not accept cut text from a view-only client */
        if (!rfbViewOnly && !cl->viewOnly && !rfbAuthDisableCBRecv) {
            vncClientCutText(str, msg.cct.length);
            if (rfbSyncCutBuffer) rfbSetXCutText(str, msg.cct.length);
        }

        xfree(str);
        return;

    case rfbEnableContinuousUpdates:
    {
        BoxRec box;

        if ((n = ReadExact(cl->sock, ((char *)&msg) + 1,
                           sz_rfbEnableContinuousUpdatesMsg - 1)) <= 0) {
            if (n != 0)
                rfbLogPerror("rfbProcessClientNormalMessage: read");
            rfbCloseSock(cl->sock);
            return;
        }

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
            rfbSendFramebufferUpdate(cl);
        } else
            rfbSendEndOfCU(cl);

        rfbLog("Continuous updates %s\n", cl->continuousUpdates ? "enabled":
                "disabled");
        return;
    }

    case rfbFence:
    {
        CARD32 flags;
        char data[64];

        if ((n = ReadExact(cl->sock, ((char *)&msg) + 1,
                           sz_rfbFenceMsg - 1)) <= 0) {
            if (n != 0)
                rfbLogPerror("rfbProcessClientNormalMessage: read");
            rfbCloseSock(cl->sock);
            return;
        }

        flags = Swap32IfLE(msg.f.flags);

        if ((n = ReadExact(cl->sock, data, msg.f.length)) <= 0) {
            if (n != 0)
                rfbLogPerror("rfbProcessClientNormalMessage: read");
            rfbCloseSock(cl->sock);
            return;
        }

        if (msg.f.length > sizeof(data))
            rfbLog("Ignoring fence.  Payload of %d bytes is too large.\n",
                   msg.f.length);
        else
            HandleFence(cl, flags, msg.f.length, data);
        return;
    }

    default:

        rfbLog("rfbProcessClientNormalMessage: unknown message type %d\n",
                msg.type);
        rfbLog(" ... closing connection\n");
        rfbCloseSock(cl->sock);
        return;
    }
}



/*
 * rfbSendFramebufferUpdate - send the currently pending framebuffer update to
 * the RFB client.
 */

Bool
rfbSendFramebufferUpdate(cl)
    rfbClientPtr cl;
{
    ScreenPtr pScreen = screenInfo.screens[0];
    int i;
    int nUpdateRegionRects;
    rfbFramebufferUpdateMsg *fu = (rfbFramebufferUpdateMsg *)updateBuf;
    RegionRec _updateRegion, *updateRegion = &_updateRegion, updateCopyRegion,
        idRegion;
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

    /*
     * If this client understands cursor shape updates, cursor should be
     * removed from the framebuffer. Otherwise, make sure it's put up.
     */

    if (cl->enableCursorShapeUpdates) {
        if (rfbScreen.cursorIsDrawn)
            rfbSpriteRemoveCursor(pScreen);
        if (!rfbScreen.cursorIsDrawn && cl->cursorWasChanged)
            sendCursorShape = TRUE;
    } else {
        if (!rfbScreen.cursorIsDrawn)
            rfbSpriteRestoreCursor(pScreen);
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
    REGION_UNION(pScreen, updateRegion, &cl->copyRegion,
                 &cl->modifiedRegion);

    if (cl->continuousUpdates)
        REGION_UNION(pScreen, &cl->requestedRegion, &cl->requestedRegion,
                     &cl->cuRegion);

    REGION_INTERSECT(pScreen, updateRegion, &cl->requestedRegion,
                     updateRegion);

    if (!REGION_NOTEMPTY(pScreen, updateRegion) &&
        !sendCursorShape && !sendCursorPos) {
        REGION_UNINIT(pScreen, updateRegion);
        return TRUE;
    }

    /* In continuous mode, we will be outputting at least three distinct
       messages.  We need to aggregate these in order to not clog up TCP's
       congestion window. */

    rfbCorkSock(cl->sock);

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

    if (cl->compareFB) {
        updateRegion = &cl->ifRegion;
        if (rfbInterframeDebug)
            REGION_INIT(pScreen, &idRegion, NullBox, 0);
        for (i = 0; i < REGION_NUM_RECTS(&_updateRegion); i++) {
            int x = REGION_RECTS(&_updateRegion)[i].x1;
            int y = REGION_RECTS(&_updateRegion)[i].y1;
            int w = REGION_RECTS(&_updateRegion)[i].x2 - x;
            int h = REGION_RECTS(&_updateRegion)[i].y2 - y, rows;
            int pitch = rfbScreen.paddedWidthInBytes;
            int ps = rfbServerFormat.bitsPerPixel / 8;
            char *src = &rfbScreen.pfbMemory[y * pitch + x * ps];
            char *dst = &cl->compareFB[y * pitch + x * ps];
            Bool different = FALSE;
            rows = h;
            while (rows--) {
                if (cl->firstCompare || memcmp(src, dst, w * ps)) {
                    memcpy(dst, src, w * ps);
                    different = TRUE;
                }
                src += pitch;
                dst += pitch;
            }
            if (different || rfbInterframeDebug) {
                RegionRec tmpRegion;
                REGION_INIT(pScreen, &tmpRegion,
                            &REGION_RECTS(&_updateRegion)[i], 1);
                if (!different && rfbInterframeDebug &&
                    !RECT_IN_REGION(pScreen, &cl->ifRegion,
                                    &REGION_RECTS(&_updateRegion)[i])) {
                    int pad = pitch - w * ps;
                    REGION_UNION(pScreen, &idRegion, &idRegion, &tmpRegion);
                    dst = &cl->compareFB[y * pitch + x * ps];
                    rows = h;
                    while (rows--) {
                        char *endOfRow = &dst[w * ps];
                        while (dst < endOfRow)
                            *dst++ ^= 0xFF;
                        dst += pad;
                    }
                }
                REGION_UNION(pScreen, &cl->ifRegion, &cl->ifRegion,
                             &tmpRegion);
                REGION_UNINIT(pScreen, &tmpRegion);
            }
            if (!different && rfbProfile) {
                idmpixels += (double)(w * h) / 1000000.;
                if (!rfbInterframeDebug)
                    mpixels += (double)(w * h) / 1000000.;
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
            nUpdateRegionRects += (((w-1) / cl->correMaxWidth + 1)
                                     * ((h-1) / cl->correMaxHeight + 1));
        }
    } else if (cl->preferredEncoding == rfbEncodingZlib) {
        nUpdateRegionRects = 0;

        for (i = 0; i < REGION_NUM_RECTS(updateRegion); i++) {
            int x = REGION_RECTS(updateRegion)[i].x1;
            int y = REGION_RECTS(updateRegion)[i].y1;
            int w = REGION_RECTS(updateRegion)[i].x2 - x;
            int h = REGION_RECTS(updateRegion)[i].y2 - y;
            nUpdateRegionRects += (((h-1) / (ZLIB_MAX_SIZE( w ) / w)) + 1);
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

        cl->rfbRawBytesEquivalent += (sz_rfbFramebufferUpdateRectHeader
                                      + w * (cl->format.bitsPerPixel / 8) * h);

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
                int pitch = rfbScreen.paddedWidthInBytes;
                int ps = rfbServerFormat.bitsPerPixel / 8;
                char *src = &rfbScreen.pfbMemory[y * pitch + x * ps];
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
            (CARD64)(rfbAutoLosslessRefresh * 1000.0), alrCallback, cl);
    }

    rfbUncorkSock(cl->sock);
    return TRUE;

    abort:
    if (!REGION_NIL(&updateCopyRegion))
        REGION_UNINIT(pScreen, &updateCopyRegion);
    if (rfbInterframeDebug && !REGION_NIL(&idRegion))
        REGION_UNINIT(pScreen, &idRegion);
    if (cl->compareFB) {
        REGION_EMPTY(pScreen, updateRegion);
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

static Bool
rfbSendCopyRegion(cl, reg, dx, dy)
    rfbClientPtr cl;
    RegionPtr reg;
    int dx, dy;
{
    int nrects, nrectsInBand, x_inc, y_inc, thisRect, firstInNextBand;
    int x, y, w, h;
    rfbFramebufferUpdateRectHeader rect;
    rfbCopyRect cr;

    nrects = REGION_NUM_RECTS(reg);

    if (dx <= 0) {
        x_inc = 1;
    } else {
        x_inc = -1;
    }

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
        SAFE_REGION_INIT(pScreen, &tmpRegion, NullBox, 0);
        REGION_COPY(pScreen, &tmpRegion, reg);
        REGION_TRANSLATE(pScreen, &tmpRegion, -dx, -dy);
        REGION_INTERSECT(pScreen, &tmpRegion, &cl->lossyRegion, &tmpRegion);
        if (REGION_NOTEMPTY(pScreen, &tmpRegion)) {
            REGION_UNION(pScreen, &cl->lossyRegion, &cl->lossyRegion, reg);
            REGION_UNION(pScreen, &cl->alrEligibleRegion,
                                  &cl->alrEligibleRegion, reg);
        }
        REGION_UNINIT(pScreen, &tmpRegion);
    }

    while (nrects > 0) {

        firstInNextBand = thisRect;
        nrectsInBand = 0;

        while ((nrects > 0) &&
               (REGION_RECTS(reg)[firstInNextBand].y1
                == REGION_RECTS(reg)[thisRect].y1))
        {
            firstInNextBand += y_inc;
            nrects--;
            nrectsInBand++;
        }

        if (x_inc != y_inc) {
            thisRect = firstInNextBand - y_inc;
        }

        while (nrectsInBand > 0) {
            if ((ublen + sz_rfbFramebufferUpdateRectHeader
                 + sz_rfbCopyRect) > UPDATE_BUF_SIZE)
            {
                if (!rfbSendUpdateBuf(cl))
                    return FALSE;
            }

            x = REGION_RECTS(reg)[thisRect].x1;
            y = REGION_RECTS(reg)[thisRect].y1;
            w = REGION_RECTS(reg)[thisRect].x2 - x;
            h = REGION_RECTS(reg)[thisRect].y2 - y;

            if (cl->compareFB) {
                int pitch = rfbScreen.paddedWidthInBytes;
                int ps = rfbServerFormat.bitsPerPixel / 8, rows = h;
                char *src = &rfbScreen.pfbMemory[y * pitch + x * ps];
                char *dst = &cl->compareFB[y * pitch + x * ps];
                while (rows--) {
                    memcpy(dst, src, w * ps);
                    src += pitch;
                    dst += pitch;
                }
                src = &rfbScreen.pfbMemory[(y - dy) * pitch + (x - dx) * ps];
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
            cl->rfbBytesSent[rfbEncodingCopyRect]
                += sz_rfbFramebufferUpdateRectHeader + sz_rfbCopyRect;

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

Bool
rfbSendRectEncodingRaw(cl, x, y, w, h)
    rfbClientPtr cl;
    int x, y, w, h;
{
    rfbFramebufferUpdateRectHeader rect;
    int nlines;
    int bytesPerLine = w * (cl->format.bitsPerPixel / 8);
    char *fbptr = (cl->fb + (rfbScreen.paddedWidthInBytes * y)
                   + (x * (rfbScreen.bitsPerPixel / 8)));

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

    memcpy(&updateBuf[ublen], (char *)&rect,
           sz_rfbFramebufferUpdateRectHeader);
    ublen += sz_rfbFramebufferUpdateRectHeader;

    cl->rfbRectanglesSent[rfbEncodingRaw]++;
    cl->rfbBytesSent[rfbEncodingRaw]
        += sz_rfbFramebufferUpdateRectHeader + bytesPerLine * h;

    nlines = (UPDATE_BUF_SIZE - ublen) / bytesPerLine;

    while (TRUE) {
        if (nlines > h)
            nlines = h;

        (*cl->translateFn)(cl->translateLookupTable, &rfbServerFormat,
                           &cl->format, fbptr, &updateBuf[ublen],
                           rfbScreen.paddedWidthInBytes, w, nlines);

        ublen += nlines * bytesPerLine;
        h -= nlines;

        if (h == 0)     /* rect fitted in buffer, do next one */
            return TRUE;

        /* buffer full - flush partial rect and do another nlines */

        if (!rfbSendUpdateBuf(cl))
            return FALSE;

        fbptr += (rfbScreen.paddedWidthInBytes * nlines);

        nlines = (UPDATE_BUF_SIZE - ublen) / bytesPerLine;
        if (nlines == 0) {
            rfbLog("rfbSendRectEncodingRaw: send buffer too small for %d "
                   "bytes per line\n", bytesPerLine);
            rfbCloseSock(cl->sock);
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

static Bool
rfbSendLastRectMarker(cl)
    rfbClientPtr cl;
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

    memcpy(&updateBuf[ublen], (char *)&rect,
           sz_rfbFramebufferUpdateRectHeader);
    ublen += sz_rfbFramebufferUpdateRectHeader;

    cl->rfbLastRectMarkersSent++;
    cl->rfbLastRectBytesSent += sz_rfbFramebufferUpdateRectHeader;

    return TRUE;
}


/*
 * Send the contents of updateBuf.  Returns 1 if successful, -1 if
 * not (errno should be set).
 */

Bool
rfbSendUpdateBuf(cl)
    rfbClientPtr cl;
{
    /*
    int i;
    for (i = 0; i < ublen; i++) {
        fprintf(stderr, "%02x ", ((unsigned char *)updateBuf)[i]);
    }
    fprintf(stderr, "\n");
    */

    if (ublen > 0 && WriteExact(cl->sock, updateBuf, ublen) < 0) {
        rfbLogPerror("rfbSendUpdateBuf: write");
        rfbCloseSock(cl->sock);
        return FALSE;
    }

    ublen = 0;
    return TRUE;
}



/*
 * rfbSendSetColourMapEntries sends a SetColourMapEntries message to the
 * client, using values from the currently installed colormap.
 */

Bool
rfbSendSetColourMapEntries(cl, firstColour, nColours)
    rfbClientPtr cl;
    int firstColour;
    int nColours;
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

    if (WriteExact(cl->sock, buf, len) < 0) {
        rfbLogPerror("rfbSendSetColourMapEntries: write");
        rfbCloseSock(cl->sock);
        return FALSE;
    }
    return TRUE;
}


/*
 * rfbSendBell sends a Bell message to all the clients.
 */

void
rfbSendBell()
{
    rfbClientPtr cl, nextCl;
    rfbBellMsg b;

    for (cl = rfbClientHead; cl; cl = nextCl) {
        nextCl = cl->next;
        if (cl->state != RFB_NORMAL)
          continue;
        b.type = rfbBell;
        if (WriteExact(cl->sock, (char *)&b, sz_rfbBellMsg) < 0) {
            rfbLogPerror("rfbSendBell: write");
            rfbCloseSock(cl->sock);
        }
    }
}


/*
 * rfbSendServerCutText sends a ServerCutText message to all the clients.
 */

void
rfbSendServerCutText(char *str, int len)
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
            xfree(cl->cutText);
        cl->cutText = strdup(str);
        cl->cutTextLen = len;
        sct.type = rfbServerCutText;
        sct.length = Swap32IfLE(len);
        if (WriteExact(cl->sock, (char *)&sct,
                       sz_rfbServerCutTextMsg) < 0) {
            rfbLogPerror("rfbSendServerCutText: write");
            rfbCloseSock(cl->sock);
            continue;
        }
        if (WriteExact(cl->sock, str, len) < 0) {
            rfbLogPerror("rfbSendServerCutText: write");
            rfbCloseSock(cl->sock);
        }
    }
}




/*****************************************************************************
 *
 * UDP can be used for keyboard and pointer events when the underlying
 * network is highly reliable.  This is really here to support ORL's
 * videotile, whose TCP implementation doesn't like sending lots of small
 * packets (such as 100s of pen readings per second!).
 */

void
rfbNewUDPConnection(sock)
    int sock;
{
    if (write(sock, &ptrAcceleration, 1) < 0) {
        rfbLogPerror("rfbNewUDPConnection: write");
    }
}

/*
 * Because UDP is a message based service, we can't read the first byte and
 * then the rest of the packet separately like we do with TCP.  We will always
 * get a whole packet delivered in one go, so we ask read() for the maximum
 * number of bytes we can possibly get.
 */

void
rfbProcessUDPInput(sock)
    int sock;
{
    int n;
    rfbClientToServerMsg msg;

    if ((n = read(sock, (char *)&msg, sizeof(msg))) <= 0) {
        if (n < 0) {
            rfbLogPerror("rfbProcessUDPInput: read");
        }
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
        if (!rfbViewOnly) {
            KbdAddEvent(msg.ke.down, (KeySym)Swap32IfLE(msg.ke.key), 0);
        }
        break;

    case rfbPointerEvent:
        if (n != sz_rfbPointerEventMsg) {
            rfbLog("rfbProcessUDPInput: ptr event incorrect length\n");
            rfbDisconnectUDPSock();
            return;
        }
        if (!rfbViewOnly) {
            PtrAddEvent(msg.pe.buttonMask,
                        Swap16IfLE(msg.pe.x), Swap16IfLE(msg.pe.y), 0);
        }
        break;

    default:
        rfbLog("rfbProcessUDPInput: unknown message type %d\n",
               msg.type);
        rfbDisconnectUDPSock();
    }
}
