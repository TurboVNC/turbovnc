/*
 * rfbserver.c - deal with server-side of the RFB protocol.
 */

/*
 *  Copyright (C) 2000-2002 Constantin Kaplinsky.  All Rights Reserved.
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 *  USA.
 */

/* Use ``#define CORBA'' to enable CORBA control interface */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "windowstr.h"
#include "rfb.h"
#include "input.h"
#include "mipointer.h"
#include "sprite.h"

#ifdef CORBA
#include <vncserverctrl.h>
#endif

char updateBuf[UPDATE_BUF_SIZE];
int ublen;

rfbClientPtr rfbClientHead = NULL;
rfbClientPtr pointerClient = NULL;  /* Mutex for pointer events */

Bool rfbAlwaysShared = FALSE;
Bool rfbNeverShared = FALSE;
Bool rfbDontDisconnect = FALSE;
Bool rfbViewOnly = FALSE; /* run server in view only mode - Ehud Karni SW */

static rfbClientPtr rfbNewClient(int sock);
static void rfbProcessClientProtocolVersion(rfbClientPtr cl);
static void rfbProcessClientNormalMessage(rfbClientPtr cl);
static void rfbProcessClientInitMessage(rfbClientPtr cl);
static Bool rfbSendCopyRegion(rfbClientPtr cl, RegionPtr reg, int dx, int dy);
static Bool rfbSendLastRectMarker(rfbClientPtr cl);


/*
 * rfbNewClientConnection is called from sockets.c when a new connection
 * comes in.
 */

void
rfbNewClientConnection(sock)
    int sock;
{
    rfbClientPtr cl;

    cl = rfbNewClient(sock);

#ifdef CORBA
    if (cl != NULL)
	newConnection(cl, (KEYBOARD_DEVICE|POINTER_DEVICE), 1, 1, 1);
#endif
}


/*
 * rfbReverseConnection is called by the CORBA stuff to make an outward
 * connection to a "listening" RFB client.
 */

rfbClientPtr
rfbReverseConnection(host, port)
    char *host;
    int port;
{
    int sock;
    rfbClientPtr cl;

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
    struct sockaddr_in addr;
    int addrlen = sizeof(struct sockaddr_in);
    int i;

    if (rfbClientHead == NULL) {
	/* no other clients - make sure we don't think any keys are pressed */
	KbdReleaseAllKeys();
    } else {
	rfbLog("  (other clients");
	for (cl = rfbClientHead; cl; cl = cl->next) {
	    fprintf(stderr," %s",cl->host);
	}
	fprintf(stderr,")\n");
    }

    cl = (rfbClientPtr)xalloc(sizeof(rfbClientRec));

    cl->sock = sock;
    getpeername(sock, (struct sockaddr *)&addr, &addrlen);
    cl->host = strdup(inet_ntoa(addr.sin_addr));

    cl->state = RFB_PROTOCOL_VERSION;

    cl->viewOnly = FALSE;
    cl->reverseConnection = FALSE;
    cl->readyForSetColourMapEntries = FALSE;
    cl->useCopyRect = FALSE;
    cl->preferredEncoding = rfbEncodingRaw;
    cl->correMaxWidth = 48;
    cl->correMaxHeight = 48;

    REGION_INIT(pScreen,&cl->copyRegion,NullBox,0);
    cl->copyDX = 0;
    cl->copyDY = 0;

    box.x1 = box.y1 = 0;
    box.x2 = rfbScreen.width;
    box.y2 = rfbScreen.height;
    REGION_INIT(pScreen,&cl->modifiedRegion,&box,0);

    REGION_INIT(pScreen,&cl->requestedRegion,NullBox,0);

    cl->deferredUpdateScheduled = FALSE;
    cl->deferredUpdateTimer = NULL;

    cl->format = rfbServerFormat;
    cl->translateFn = rfbTranslateNone;
    cl->translateLookupTable = NULL;

    cl->tightCompressLevel = TIGHT_DEFAULT_COMPRESSION;
    cl->tightQualityLevel = -1;
    for (i = 0; i < 4; i++)
        cl->zsActive[i] = FALSE;

    cl->enableCursorShapeUpdates = FALSE;
    cl->enableCursorPosUpdates = FALSE;
    cl->enableLastRectEncoding = FALSE;

    cl->next = rfbClientHead;
    rfbClientHead = cl;

    rfbResetStats(cl);

    cl->compStreamInited = FALSE;
    cl->compStream.total_in = 0;
    cl->compStream.total_out = 0;
    cl->compStream.zalloc = Z_NULL;
    cl->compStream.zfree = Z_NULL;
    cl->compStream.opaque = Z_NULL;

    cl->zlibCompressLevel = 5;

    sprintf(pv,rfbProtocolVersionFormat,rfbProtocolMajorVersion,
	    rfbProtocolMinorVersion);

    if (WriteExact(sock, pv, sz_rfbProtocolVersionMsg) < 0) {
	rfbLogPerror("rfbNewClient: write");
	rfbCloseSock(sock);
	return NULL;
    }

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
	rfbLog("rfbClientConnectionGone: unknown socket %d\n",sock);
	return;
    }

    rfbLog("Client %s gone\n",cl->host);
    free(cl->host);

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

#ifdef CORBA
    destroyConnection(cl);
#endif

    if (prev)
	prev->next = cl->next;
    else
	rfbClientHead = cl->next;

    REGION_UNINIT(pScreen,&cl->copyRegion);
    REGION_UNINIT(pScreen,&cl->modifiedRegion);
    TimerFree(cl->deferredUpdateTimer);

    rfbPrintStats(cl);

    if (cl->translateLookupTable) free(cl->translateLookupTable);

    xfree(cl);
}


/*
 * rfbProcessClientMessage is called when there is data to read from a client.
 */

void
rfbProcessClientMessage(sock)
    int sock;
{
    rfbClientPtr cl;

    for (cl = rfbClientHead; cl; cl = cl->next) {
	if (sock == cl->sock)
	    break;
    }

    if (!cl) {
	rfbLog("rfbProcessClientMessage: unknown socket %d\n",sock);
	rfbCloseSock(sock);
	return;
    }

#ifdef CORBA
    if (isClosePending(cl)) {
	rfbLog("Closing connection to client %s\n", cl->host);
	rfbCloseSock(sock);
	return;
    }
#endif

    switch (cl->state) {
    case RFB_PROTOCOL_VERSION:
	rfbProcessClientProtocolVersion(cl);
	return;
    case RFB_AUTHENTICATION:
	rfbAuthProcessClientMessage(cl);
	return;
    case RFB_INITIALISATION:
	rfbProcessClientInitMessage(cl);
	return;
    default:
	rfbProcessClientNormalMessage(cl);
	return;
    }
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
    char failureReason[256];

    if ((n = ReadExact(cl->sock, pv, sz_rfbProtocolVersionMsg)) <= 0) {
	if (n == 0)
	    rfbLog("rfbProcessClientProtocolVersion: client gone\n");
	else
	    rfbLogPerror("rfbProcessClientProtocolVersion: read");
	rfbCloseSock(cl->sock);
	return;
    }

    pv[sz_rfbProtocolVersionMsg] = 0;
    if (sscanf(pv,rfbProtocolVersionFormat,&major,&minor) != 2) {
	rfbLog("rfbProcessClientProtocolVersion: not a valid RFB client\n");
	rfbCloseSock(cl->sock);
	return;
    }
    rfbLog("Protocol version %d.%d\n", major, minor);

    if (major != rfbProtocolMajorVersion) {
	/* Major version mismatch - send a ConnFailed message */

	rfbLog("Major version mismatch\n");
	sprintf(failureReason,
		"RFB protocol version mismatch - server %d.%d, client %d.%d",
		rfbProtocolMajorVersion,rfbProtocolMinorVersion,major,minor);
	rfbClientConnFailed(cl, failureReason);
	return;
    }

    if (minor != rfbProtocolMinorVersion) {
	/* Minor version mismatch - warn but try to continue */
	rfbLog("Ignoring minor version mismatch\n");
    }

    rfbAuthNewClient(cl);
}


/*
 * rfbClientConnFailed is called when a client connection has failed either
 * because it talks the wrong protocol or it has failed authentication.
 */

void
rfbClientConnFailed(cl, reason)
    rfbClientPtr cl;
    char *reason;
{
    char *buf;
    int len = strlen(reason);

    buf = (char *)xalloc(8 + len);
    ((CARD32 *)buf)[0] = Swap32IfLE(rfbConnFailed);
    ((CARD32 *)buf)[1] = Swap32IfLE(len);
    memcpy(buf + 8, reason, len);

    if (WriteExact(cl->sock, buf, 8 + len) < 0)
	rfbLogPerror("rfbClientConnFailed: write");
    xfree(buf);
    rfbCloseSock(cl->sock);
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
    struct passwd *user;
    int len, n;
    rfbClientPtr otherCl, nextCl;

    if ((n = ReadExact(cl->sock, (char *)&ci,sz_rfbClientInitMsg)) <= 0) {
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

    user = getpwuid(getuid());

    if (strlen(desktopName) > 128)	/* sanity check on desktop name len */
	desktopName[128] = 0;

    if (user) {
	sprintf(buf + sz_rfbServerInitMsg, "%s's %s desktop (%s:%s)",
		user->pw_name, desktopName, rfbThisHost, display);
    } else {
	sprintf(buf + sz_rfbServerInitMsg, "%s desktop (%s:%s)",
		desktopName, rfbThisHost, display);
    }
    len = strlen(buf + sz_rfbServerInitMsg);
    si->nameLength = Swap32IfLE(len);

    if (WriteExact(cl->sock, buf, sz_rfbServerInitMsg + len) < 0) {
	rfbLogPerror("rfbProcessClientInitMessage: write");
	rfbCloseSock(cl->sock);
	return;
    }

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
	cl->tightQualityLevel = -1;

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
	    case rfbEncodingTight:
		if (cl->preferredEncoding == -1) {
		    cl->preferredEncoding = enc;
		    rfbLog("Using tight encoding for client %s\n",
			   cl->host);
		}
		break;
	    case rfbEncodingXCursor:
		rfbLog("Enabling X-style cursor updates for client %s\n",
		       cl->host);
		cl->enableCursorShapeUpdates = TRUE;
		cl->useRichCursorEncoding = FALSE;
		cl->cursorWasChanged = TRUE;
		break;
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
	    default:
		if ( enc >= (CARD32)rfbEncodingCompressLevel0 &&
		     enc <= (CARD32)rfbEncodingCompressLevel9 ) {
		    cl->zlibCompressLevel = enc & 0x0F;
		    cl->tightCompressLevel = enc & 0x0F;
		    rfbLog("Using compression level %d for client %s\n",
			   cl->tightCompressLevel, cl->host);
		} else if ( enc >= (CARD32)rfbEncodingQualityLevel0 &&
			    enc <= (CARD32)rfbEncodingQualityLevel9 ) {
		    cl->tightQualityLevel = enc & 0x0F;
		    rfbLog("Using image quality level %d for client %s\n",
			   cl->tightQualityLevel, cl->host);
		} else {
		    rfbLog("rfbProcessClientNormalMessage: ignoring unknown "
			   "encoding %d\n", (int)enc);
		}
	    }
	}

	if (cl->preferredEncoding == -1) {
	    cl->preferredEncoding = rfbEncodingRaw;
	}

	if (cl->enableCursorPosUpdates && !cl->enableCursorShapeUpdates) {
	    rfbLog("Disabling cursor position updates for client %s\n",
		   cl->host);
	    cl->enableCursorPosUpdates = FALSE;
	}

	return;
    }


    case rfbFramebufferUpdateRequest:
    {
	RegionRec tmpRegion;
	BoxRec box;

#ifdef CORBA
	addCapability(cl, DISPLAY_DEVICE);
#endif

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
	SAFE_REGION_INIT(pScreen,&tmpRegion,&box,0);

	REGION_UNION(pScreen, &cl->requestedRegion, &cl->requestedRegion,
		     &tmpRegion);

	if (!cl->readyForSetColourMapEntries) {
	    /* client hasn't sent a SetPixelFormat so is using server's */
	    cl->readyForSetColourMapEntries = TRUE;
	    if (!cl->format.trueColour) {
		if (!rfbSetClientColourMap(cl, 0, 0)) {
		    REGION_UNINIT(pScreen,&tmpRegion);
		    return;
		}
	    }
	}

	if (!msg.fur.incremental) {
	    REGION_UNION(pScreen,&cl->modifiedRegion,&cl->modifiedRegion,
			 &tmpRegion);
	    REGION_SUBTRACT(pScreen,&cl->copyRegion,&cl->copyRegion,
			    &tmpRegion);
	}

	if (FB_UPDATE_PENDING(cl)) {
	    rfbSendFramebufferUpdate(cl);
	}

	REGION_UNINIT(pScreen,&tmpRegion);
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

#ifdef CORBA
	addCapability(cl, KEYBOARD_DEVICE);

	if (!isKeyboardEnabled(cl))
	    return;
#endif
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

#ifdef CORBA
	addCapability(cl, POINTER_DEVICE);

	if (!isPointerEnabled(cl))
	    return;
#endif

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

	str = (char *)xalloc(msg.cct.length);

	if ((n = ReadExact(cl->sock, str, msg.cct.length)) <= 0) {
	    if (n != 0)
		rfbLogPerror("rfbProcessClientNormalMessage: read");
	    xfree(str);
	    rfbCloseSock(cl->sock);
	    return;
	}

	/* NOTE: We do not accept cut text from a view-only client */
	if (!cl->viewOnly)
	    rfbSetXCutText(str, msg.cct.length);

	xfree(str);
	return;


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
    RegionRec updateRegion, updateCopyRegion;
    int dx, dy;
    Bool sendCursorShape = FALSE;
    Bool sendCursorPos = FALSE;

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

    REGION_INIT(pScreen,&updateRegion,NullBox,0);
    REGION_UNION(pScreen, &updateRegion, &cl->copyRegion,
		 &cl->modifiedRegion);
    REGION_INTERSECT(pScreen, &updateRegion, &cl->requestedRegion,
		     &updateRegion);

    if ( !REGION_NOTEMPTY(pScreen,&updateRegion) &&
	 !sendCursorShape && !sendCursorPos ) {
	REGION_UNINIT(pScreen,&updateRegion);
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

    REGION_INIT(pScreen,&updateCopyRegion,NullBox,0);
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

    REGION_SUBTRACT(pScreen, &updateRegion, &updateRegion, &updateCopyRegion);

    /*
     * Finally we leave modifiedRegion to be the remainder (if any) of parts of
     * the screen which are modified but outside the requestedRegion.  We also
     * empty both the requestedRegion and the copyRegion - note that we never
     * carry over a copyRegion for a future update.
     */

    REGION_UNION(pScreen, &cl->modifiedRegion, &cl->modifiedRegion,
		 &cl->copyRegion);
    REGION_SUBTRACT(pScreen, &cl->modifiedRegion, &cl->modifiedRegion,
		    &updateRegion);
    REGION_SUBTRACT(pScreen, &cl->modifiedRegion, &cl->modifiedRegion,
		    &updateCopyRegion);

    REGION_EMPTY(pScreen, &cl->requestedRegion);
    REGION_EMPTY(pScreen, &cl->copyRegion);
    cl->copyDX = 0;
    cl->copyDY = 0;

    /*
     * Now send the update.
     */

    cl->rfbFramebufferUpdateMessagesSent++;

    if (cl->preferredEncoding == rfbEncodingCoRRE) {
	nUpdateRegionRects = 0;

	for (i = 0; i < REGION_NUM_RECTS(&updateRegion); i++) {
	    int x = REGION_RECTS(&updateRegion)[i].x1;
	    int y = REGION_RECTS(&updateRegion)[i].y1;
	    int w = REGION_RECTS(&updateRegion)[i].x2 - x;
	    int h = REGION_RECTS(&updateRegion)[i].y2 - y;
	    nUpdateRegionRects += (((w-1) / cl->correMaxWidth + 1)
				     * ((h-1) / cl->correMaxHeight + 1));
	}
    } else if (cl->preferredEncoding == rfbEncodingZlib) {
	nUpdateRegionRects = 0;

	for (i = 0; i < REGION_NUM_RECTS(&updateRegion); i++) {
	    int x = REGION_RECTS(&updateRegion)[i].x1;
	    int y = REGION_RECTS(&updateRegion)[i].y1;
	    int w = REGION_RECTS(&updateRegion)[i].x2 - x;
	    int h = REGION_RECTS(&updateRegion)[i].y2 - y;
	    nUpdateRegionRects += (((h-1) / (ZLIB_MAX_SIZE( w ) / w)) + 1);
	}
    } else if (cl->preferredEncoding == rfbEncodingTight) {
	nUpdateRegionRects = 0;

	for (i = 0; i < REGION_NUM_RECTS(&updateRegion); i++) {
	    int x = REGION_RECTS(&updateRegion)[i].x1;
	    int y = REGION_RECTS(&updateRegion)[i].y1;
	    int w = REGION_RECTS(&updateRegion)[i].x2 - x;
	    int h = REGION_RECTS(&updateRegion)[i].y2 - y;
	    int n = rfbNumCodedRectsTight(cl, x, y, w, h);
	    if (n == 0) {
		nUpdateRegionRects = 0xFFFF;
		break;
	    }
	    nUpdateRegionRects += n;
	}
    } else {
	nUpdateRegionRects = REGION_NUM_RECTS(&updateRegion);
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
	    return FALSE;
    }

    if (sendCursorPos) {
	cl->cursorWasMoved = FALSE;
	if (!rfbSendCursorPos(cl, pScreen))
 	    return FALSE;
    }

    if (REGION_NOTEMPTY(pScreen,&updateCopyRegion)) {
	if (!rfbSendCopyRegion(cl,&updateCopyRegion,dx,dy)) {
	    REGION_UNINIT(pScreen,&updateRegion);
	    REGION_UNINIT(pScreen,&updateCopyRegion);
	    return FALSE;
	}
    }

    REGION_UNINIT(pScreen,&updateCopyRegion);

    for (i = 0; i < REGION_NUM_RECTS(&updateRegion); i++) {
	int x = REGION_RECTS(&updateRegion)[i].x1;
	int y = REGION_RECTS(&updateRegion)[i].y1;
	int w = REGION_RECTS(&updateRegion)[i].x2 - x;
	int h = REGION_RECTS(&updateRegion)[i].y2 - y;

	cl->rfbRawBytesEquivalent += (sz_rfbFramebufferUpdateRectHeader
				      + w * (cl->format.bitsPerPixel / 8) * h);

	switch (cl->preferredEncoding) {
	case rfbEncodingRaw:
	    if (!rfbSendRectEncodingRaw(cl, x, y, w, h)) {
		REGION_UNINIT(pScreen,&updateRegion);
		return FALSE;
	    }
	    break;
	case rfbEncodingRRE:
	    if (!rfbSendRectEncodingRRE(cl, x, y, w, h)) {
		REGION_UNINIT(pScreen,&updateRegion);
		return FALSE;
	    }
	    break;
	case rfbEncodingCoRRE:
	    if (!rfbSendRectEncodingCoRRE(cl, x, y, w, h)) {
		REGION_UNINIT(pScreen,&updateRegion);
		return FALSE;
	    }
	    break;
	case rfbEncodingHextile:
	    if (!rfbSendRectEncodingHextile(cl, x, y, w, h)) {
		REGION_UNINIT(pScreen,&updateRegion);
		return FALSE;
	    }
	    break;
	case rfbEncodingZlib:
	    if (!rfbSendRectEncodingZlib(cl, x, y, w, h)) {
		REGION_UNINIT(pScreen,&updateRegion);
		return FALSE;
	    }
	    break;
	case rfbEncodingTight:
	    if (!rfbSendRectEncodingTight(cl, x, y, w, h)) {
		REGION_UNINIT(pScreen,&updateRegion);
		return FALSE;
	    }
	    break;
	}
    }

    REGION_UNINIT(pScreen,&updateRegion);

    if (nUpdateRegionRects == 0xFFFF && !rfbSendLastRectMarker(cl))
	return FALSE;

    if (!rfbSendUpdateBuf(cl))
	return FALSE;

    return TRUE;
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
    char *fbptr = (rfbScreen.pfbMemory + (rfbScreen.paddedWidthInBytes * y)
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

    memcpy(&updateBuf[ublen], (char *)&rect,sz_rfbFramebufferUpdateRectHeader);
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

	if (h == 0)	/* rect fitted in buffer, do next one */
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

    memcpy(&updateBuf[ublen], (char *)&rect,sz_rfbFramebufferUpdateRectHeader);
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
	fprintf(stderr,"%02x ",((unsigned char *)updateBuf)[i]);
    }
    fprintf(stderr,"\n");
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
	    rgb[i*3] = Swap16IfLE(pent->co.shco.red->color);
	    rgb[i*3+1] = Swap16IfLE(pent->co.shco.green->color);
	    rgb[i*3+2] = Swap16IfLE(pent->co.shco.blue->color);
	} else {
	    rgb[i*3] = Swap16IfLE(pent->co.local.red);
	    rgb[i*3+1] = Swap16IfLE(pent->co.local.green);
	    rgb[i*3+2] = Swap16IfLE(pent->co.local.blue);
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

    for (cl = rfbClientHead; cl; cl = nextCl) {
	nextCl = cl->next;
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
