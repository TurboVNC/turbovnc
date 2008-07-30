/*
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 *  USA.
 */

/*
 * rfbproto.c - functions to deal with client side of RFB protocol.
 */

#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <vncviewer.h>
#include <vncauth.h>
#include <zlib.h>
#include "turbojpeg.h"

static void InitCapabilities(void);
static Bool SetupTunneling(void);
static int ReadSecurityType(void);
static int SelectSecurityType(void);
static Bool PerformAuthenticationTight(void);
static Bool AuthenticateVNC(void);
static Bool AuthenticateNone(void);
static Bool ReadAuthenticationResult(void);
static Bool ReadInteractionCaps(void);
static Bool ReadCapabilityList(CapsContainer *caps, int count);

static Bool HandleHextile8(int rx, int ry, int rw, int rh);
static Bool HandleHextile16(int rx, int ry, int rw, int rh);
static Bool HandleHextile32(int rx, int ry, int rw, int rh);
static Bool HandleTight8(int rx, int ry, int rw, int rh);
static Bool HandleTight16(int rx, int ry, int rw, int rh);
static Bool HandleTight32(int rx, int ry, int rw, int rh);

static void ReadConnFailedReason(void);
static long ReadCompactLen (void);

extern void UpdateQual(void);
extern Bool HandleCursorPos(int, int);

int rfbsock;
char *desktopName;
rfbPixelFormat myFormat;
rfbServerInitMsg si;
char *serverCutText = NULL;
Bool newServerCutText = False;
tjhandle tjhnd=NULL;

/* For double buffering */
UpdateList *list, *node, *tail;

int endianTest = 1;

static int protocolMinorVersion;
static Bool tightVncProtocol = False;
static CapsContainer *tunnelCaps;    /* known tunneling/encryption methods */
static CapsContainer *authCaps;	     /* known authentication schemes       */
static CapsContainer *serverMsgCaps; /* known non-standard server messages */
static CapsContainer *clientMsgCaps; /* known non-standard client messages */
static CapsContainer *encodingCaps;  /* known encodings besides Raw        */


/* Note that the CoRRE encoding uses this buffer and assumes it is big enough
   to hold 255 * 255 * 32 bits -> 260100 bytes.  640*480 = 307200 bytes.
   Hextile also assumes it is big enough to hold 16 * 16 * 32 bits.
   Tight encoding assumes BUFFER_SIZE is at least 16384 bytes. */

#define BUFFER_SIZE (640*480)
static char buffer[BUFFER_SIZE];

static char *compressedData = NULL;
static char *uncompressedData = NULL;


/* The zlib encoding requires expansion/decompression/deflation of the
   compressed data in the "buffer" above into another, result buffer.
   However, the size of the result buffer can be determined precisely
   based on the bitsPerPixel, height and width of the rectangle.  We
   allocate this buffer one time to be the full size of the buffer. */

static int raw_buffer_size = -1;
static char *raw_buffer;

static z_stream decompStream;
static Bool decompStreamInited = False;


/*
 * Variables for the ``tight'' encoding implementation.
 */

/* Separate buffer for compressed data. */
#define ZLIB_BUFFER_SIZE 512
static char zlib_buffer[ZLIB_BUFFER_SIZE];

/* Four independent compression streams for zlib library. */
static z_stream zlibStream[4];
static Bool zlibStreamActive[4] = {
  False, False, False, False
};

/* Filter stuff. Should be initialized by filter initialization code. */
static Bool cutZeros;
static int rectWidth, rectColors;
static char tightPalette[256*4];
static CARD8 tightPrevRow[2048*3*sizeof(CARD16)];

#define SETUP_COLOR_SHORTCUTS \
  CARD8 rs = m_myFormat.redShift;   CARD16 rm = m_myFormat.redMax;   \
  CARD8 gs = m_myFormat.greenShift; CARD16 gm = m_myFormat.greenMax; \
  CARD8 bs = m_myFormat.blueShift;  CARD16 bm = m_myFormat.blueMax;  \
  CARD8 drs = 0, drm = image->red_mask;  \
  CARD8 dgs = 0, dgm = image->green_mask;  \
  CARD8 dbs = 0, dbm = image->blue_mask;  \
  int ps = image->bits_per_pixel / 8;  \
  int pitch = image->bytes_per_line;  \
  do {if (drm & 1) break;  drm>>=1;  drs++;}  \
  do {if (dgm & 1) break;  dgm>>=1;  dgs++;}  \
  do {if (dbm & 1) break;  dbm>>=1;  dbs++;}

/*
 * InitCapabilities.
 */

static void
InitCapabilities(void)
{
  tunnelCaps    = CapsNewContainer();
  authCaps      = CapsNewContainer();
  serverMsgCaps = CapsNewContainer();
  clientMsgCaps = CapsNewContainer();
  encodingCaps  = CapsNewContainer();

  /* Supported authentication methods */
  CapsAdd(authCaps, rfbAuthNone, rfbStandardVendor, sig_rfbAuthNone,
	  "No authentication");
  CapsAdd(authCaps, rfbAuthVNC, rfbStandardVendor, sig_rfbAuthVNC,
	  "Standard VNC password authentication");

  /* Supported encoding types */
  CapsAdd(encodingCaps, rfbEncodingCopyRect, rfbStandardVendor,
	  sig_rfbEncodingCopyRect, "Standard CopyRect encoding");
  CapsAdd(encodingCaps, rfbEncodingHextile, rfbStandardVendor,
	  sig_rfbEncodingHextile, "Standard Hextile encoding");
  CapsAdd(encodingCaps, rfbEncodingTight, rfbTightVncVendor,
	  sig_rfbEncodingTight, "Tight encoding by Constantin Kaplinsky");

  /* Supported "fake" encoding types */
  CapsAdd(encodingCaps, rfbEncodingCompressLevel0, rfbTightVncVendor,
	  sig_rfbEncodingCompressLevel0, "Compression level");
  CapsAdd(encodingCaps, rfbEncodingQualityLevel0, rfbTightVncVendor,
	  sig_rfbEncodingQualityLevel0, "JPEG quality level");
  CapsAdd(encodingCaps, rfbEncodingXCursor, rfbTightVncVendor,
	  sig_rfbEncodingXCursor, "X-style cursor shape update");
  CapsAdd(encodingCaps, rfbEncodingRichCursor, rfbTightVncVendor,
	  sig_rfbEncodingRichCursor, "Rich-color cursor shape update");
  CapsAdd(encodingCaps, rfbEncodingPointerPos, rfbTightVncVendor,
	  sig_rfbEncodingPointerPos, "Pointer position update");
  CapsAdd(encodingCaps, rfbEncodingLastRect, rfbTightVncVendor,
	  sig_rfbEncodingLastRect, "LastRect protocol extension");
  CapsAdd(encodingCaps, rfbJpegQualityLevel1, rfbTurboVncVendor,
	  sig_rfbJpegQualityLevel1, "TurboJPEG quality level");
  CapsAdd(encodingCaps, rfbJpegSubsamp1X, rfbTurboVncVendor,
	  sig_rfbJpegSubsamp1X, "TurboJPEG subsampling level");
}


/*
 * ConnectToRFBServer.
 */

Bool
ConnectToRFBServer(const char *hostname, int port)
{
  unsigned int host;

  if (!StringToIPAddr(hostname, &host)) {
    fprintf(stderr,"Couldn't convert '%s' to host address\n", hostname);
    return False;
  }

  rfbsock = ConnectToTcpAddr(host, port);

  if (rfbsock < 0) {
    fprintf(stderr,"Unable to connect to VNC server\n");
    return False;
  }

  return SetNonBlocking(rfbsock);
}


/*
 * InitialiseRFBConnection.
 */

Bool
InitialiseRFBConnection(void)
{
  rfbProtocolVersionMsg pv;
  int server_major, server_minor;
  rfbClientInitMsg ci;
  int secType;

  /* if the connection is immediately closed, don't report anything, so
       that pmw's monitor can make test connections */

  if (listenSpecified)
    errorMessageOnReadFailure = False;

  if (!ReadFromRFBServer(pv, sz_rfbProtocolVersionMsg))
    return False;

  errorMessageOnReadFailure = True;

  pv[sz_rfbProtocolVersionMsg] = 0;

  if (sscanf(pv, rfbProtocolVersionFormat,
	     &server_major, &server_minor) != 2) {
    fprintf(stderr,"Not a valid VNC server\n");
    return False;
  }

  if (server_major == 3 && server_minor >= 8) {
    /* the server supports protocol 3.8 or higher version */
    protocolMinorVersion = 8;
  } else if (server_major == 3 && server_minor == 7) {
    /* the server supports protocol 3.7 */
    protocolMinorVersion = 7;
  } else {
    /* any other server version, request the standard 3.3 */
    protocolMinorVersion = 3;
  }

  fprintf(stderr, "Connected to RFB server, using protocol version 3.%d\n",
	  protocolMinorVersion);

  sprintf(pv, rfbProtocolVersionFormat, 3, protocolMinorVersion);

  if (!WriteExact(rfbsock, pv, sz_rfbProtocolVersionMsg))
    return False;

  /* Read or select the security type. */
  if (protocolMinorVersion >= 7) {
    secType = SelectSecurityType();
  } else {
    secType = ReadSecurityType();
  }
  if (secType == rfbSecTypeInvalid)
    return False;

  switch (secType) {
  case rfbSecTypeNone:
    if (!AuthenticateNone())
      return False;
    break;
  case rfbSecTypeVncAuth:
    if (!AuthenticateVNC())
      return False;
    break;
  case rfbSecTypeTight:
    tightVncProtocol = True;
    InitCapabilities();
    if (!SetupTunneling())
      return False;
    if (!PerformAuthenticationTight())
      return False;
    break;
  default:                      /* should never happen */
    fprintf(stderr, "Internal error: Invalid security type\n");
    return False;
  }

  ci.shared = (appData.shareDesktop ? 1 : 0);

  if (!WriteExact(rfbsock, (char *)&ci, sz_rfbClientInitMsg))
    return False;

  if (!ReadFromRFBServer((char *)&si, sz_rfbServerInitMsg))
    return False;

  si.framebufferWidth = Swap16IfLE(si.framebufferWidth);
  si.framebufferHeight = Swap16IfLE(si.framebufferHeight);
  si.format.redMax = Swap16IfLE(si.format.redMax);
  si.format.greenMax = Swap16IfLE(si.format.greenMax);
  si.format.blueMax = Swap16IfLE(si.format.blueMax);
  si.nameLength = Swap32IfLE(si.nameLength);

  /* FIXME: Check arguments to malloc() calls. */
  desktopName = malloc(si.nameLength + 1);
  if (!desktopName) {
    fprintf(stderr, "Error allocating memory for desktop name, %lu bytes\n",
            (unsigned long)si.nameLength);
    return False;
  }

  if (!ReadFromRFBServer(desktopName, si.nameLength)) return False;

  desktopName[si.nameLength] = 0;

  fprintf(stderr,"Desktop name \"%s\"\n",desktopName);

  fprintf(stderr,"VNC server default format:\n");
  PrintPixelFormat(&si.format);

  if (tightVncProtocol) {
    /* Read interaction capabilities (protocol 3.7t, 3.8t) */
    if (!ReadInteractionCaps())
      return False;
  }

  return True;
}


/*
 * Read security type from the server (protocol 3.3)
 */

static int
ReadSecurityType(void)
{
  CARD32 secType;

  /* Read the security type */
  if (!ReadFromRFBServer((char *)&secType, sizeof(secType)))
    return rfbSecTypeInvalid;

  secType = Swap32IfLE(secType);

  if (secType == rfbSecTypeInvalid) {
    ReadConnFailedReason();
    return rfbSecTypeInvalid;
  }

  if (secType != rfbSecTypeNone && secType != rfbSecTypeVncAuth) {
    fprintf(stderr, "Unknown security type from RFB server: %d\n",
            (int)secType);
    return rfbSecTypeInvalid;
  }

  return (int)secType;
}


/*
 * Select security type from the server's list (protocol 3.7 and above)
 */

static int
SelectSecurityType(void)
{
  CARD8 nSecTypes;
  char *secTypeNames[] = {"None", "VncAuth"};
  CARD8 knownSecTypes[] = {rfbSecTypeNone, rfbSecTypeVncAuth};
  int nKnownSecTypes = sizeof(knownSecTypes);
  CARD8 *secTypes;
  CARD8 secType = rfbSecTypeInvalid;
  int i, j;

  /* Read the list of secutiry types. */
  if (!ReadFromRFBServer((char *)&nSecTypes, sizeof(nSecTypes)))
    return rfbSecTypeInvalid;

  if (nSecTypes == 0) {
    ReadConnFailedReason();
    return rfbSecTypeInvalid;
  }

  secTypes = malloc(nSecTypes);
  if (!ReadFromRFBServer((char *)secTypes, nSecTypes))
    return rfbSecTypeInvalid;

  /* Find out if the server supports TightVNC protocol extensions */
  for (j = 0; j < (int)nSecTypes; j++) {
    if (secTypes[j] == rfbSecTypeTight) {
      free(secTypes);
      secType = rfbSecTypeTight;
      if (!WriteExact(rfbsock, (char *)&secType, sizeof(secType)))
        return rfbSecTypeInvalid;
      fprintf(stderr, "Enabling TightVNC protocol extensions\n");
      return rfbSecTypeTight;
    }
  }

  /* Find first supported security type */
  for (j = 0; j < (int)nSecTypes; j++) {
    for (i = 0; i < nKnownSecTypes; i++) {
      if (secTypes[j] == knownSecTypes[i]) {
        secType = secTypes[j];
        if (!WriteExact(rfbsock, (char *)&secType, sizeof(secType))) {
          free(secTypes);
          return rfbSecTypeInvalid;
        }
        break;
      }
    }
    if (secType != rfbSecTypeInvalid) break;
  }

  free(secTypes);

  if (secType == rfbSecTypeInvalid)
    fprintf(stderr, "Server did not offer supported security type\n");

  return (int)secType;
}


/*
 * Setup tunneling (protocol 3.7t, 3.8t).
 */

static Bool
SetupTunneling(void)
{
  rfbTunnelingCapsMsg caps;
  CARD32 tunnelType;

  /* In protocols 3.7t/3.8t, the server informs us about
     supported tunneling methods. Here we read this information. */

  if (!ReadFromRFBServer((char *)&caps, sz_rfbTunnelingCapsMsg))
    return False;

  caps.nTunnelTypes = Swap32IfLE(caps.nTunnelTypes);

  if (caps.nTunnelTypes) {
    if (!ReadCapabilityList(tunnelCaps, caps.nTunnelTypes))
      return False;

    /* We cannot do tunneling anyway yet. */
    tunnelType = Swap32IfLE(rfbNoTunneling);
    if (!WriteExact(rfbsock, (char *)&tunnelType, sizeof(tunnelType)))
      return False;
  }

  return True;
}


/*
 * Negotiate authentication scheme (protocol 3.7t, 3.8t)
 */

static Bool
PerformAuthenticationTight(void)
{
  rfbAuthenticationCapsMsg caps;
  CARD32 authScheme;
  int i;

  /* In protocols 3.7t/3.8t, the server informs us about supported
     authentication schemes. Here we read this information. */

  if (!ReadFromRFBServer((char *)&caps, sz_rfbAuthenticationCapsMsg))
    return False;

  caps.nAuthTypes = Swap32IfLE(caps.nAuthTypes);

  /* Special case - empty capability list stands for no authentication. */
  if (!caps.nAuthTypes)
    return AuthenticateNone();

  if (!ReadCapabilityList(authCaps, caps.nAuthTypes))
    return False;

  /* Try server's preferred authentication scheme. */
  for (i = 0; i < CapsNumEnabled(authCaps); i++) {
    authScheme = CapsGetByOrder(authCaps, i);
    if (authScheme != rfbAuthVNC && authScheme != rfbAuthNone)
      continue;                 /* unknown scheme - cannot use it */
    authScheme = Swap32IfLE(authScheme);
    if (!WriteExact(rfbsock, (char *)&authScheme, sizeof(authScheme)))
      return False;
    authScheme = Swap32IfLE(authScheme); /* convert it back */

    switch (authScheme) {
    case rfbAuthNone:
      return AuthenticateNone();
    case rfbAuthVNC:
      return AuthenticateVNC();
    default:                      /* should never happen */
      fprintf(stderr, "Internal error: Invalid authentication type\n");
      return False;
    }
  }

  fprintf(stderr, "No suitable authentication schemes offered by server\n");
  return False;
}


/*
 * Null authentication.
 */

static Bool
AuthenticateNone(void)
{
  fprintf(stderr, "No authentication needed\n");

  if (protocolMinorVersion >= 8) {
    if (!ReadAuthenticationResult())
      return False;
  }

  return True;
}


/*
 * Standard VNC authentication.
 */

static Bool
AuthenticateVNC(void)
{
  CARD32 authScheme;
  CARD8 challenge[CHALLENGESIZE];
  char *passwd;
  char  buffer[64];
  char* cstatus;
  int   len;

  fprintf(stderr, "Performing standard VNC authentication\n");

  if (!ReadFromRFBServer((char *)challenge, CHALLENGESIZE))
    return False;

  if (appData.passwordFile) {
    passwd = vncDecryptPasswdFromFile(appData.passwordFile);
    if (!passwd) {
      fprintf(stderr, "Cannot read valid password from file \"%s\"\n",
	      appData.passwordFile);
      return False;
    }
  } else if (appData.autoPass) {
    passwd = buffer;
    cstatus = fgets(buffer, sizeof buffer, stdin);
    if (cstatus == NULL)
       buffer[0] = '\0';
    else
    {
       len = strlen(buffer);
       if (len > 0 && buffer[len - 1] == '\n')
	  buffer[len - 1] = '\0';
    }
  } else if (appData.passwordDialog) {
    passwd = DoPasswordDialog();
  } else {
    passwd = getpass("Password: ");
  }

  if (!passwd || strlen(passwd) == 0) {
    fprintf(stderr, "Reading password failed\n");
    return False;
  }
  if (strlen(passwd) > 8) {
    passwd[8] = '\0';
  }

  vncEncryptBytes(challenge, passwd);

  /* Lose the password from memory */
  memset(passwd, '\0', strlen(passwd));

  if (!WriteExact(rfbsock, (char *)challenge, CHALLENGESIZE))
    return False;

  return ReadAuthenticationResult();
}

/*
 * Read and report the result of authentication.
 */

static Bool
ReadAuthenticationResult(void)
{
  CARD32 authResult;

  if (!ReadFromRFBServer((char *)&authResult, 4))
    return False;

  authResult = Swap32IfLE(authResult);

  switch (authResult) {
  case rfbAuthOK:
    fprintf(stderr, "Authentication successful\n");
    break;
  case rfbAuthFailed:
    if (protocolMinorVersion >= 8) {
      ReadConnFailedReason();
    } else {
      fprintf(stderr, "Authentication failure\n");
    }
    return False;
  case rfbAuthTooMany:
    fprintf(stderr, "Authentication failure, too many tries\n");
    return False;
  default:
    fprintf(stderr, "Unknown result of authentication (%d)\n",
	    (int)authResult);
    return False;
  }

  return True;
}

/*
 * In protocols 3.7t/3.8t, the server informs us about supported
 * protocol messages and encodings. Here we read this information.
 */

static Bool
ReadInteractionCaps(void)
{
  rfbInteractionCapsMsg intr_caps;

  /* Read the counts of list items following */
  if (!ReadFromRFBServer((char *)&intr_caps, sz_rfbInteractionCapsMsg))
    return False;
  intr_caps.nServerMessageTypes = Swap16IfLE(intr_caps.nServerMessageTypes);
  intr_caps.nClientMessageTypes = Swap16IfLE(intr_caps.nClientMessageTypes);
  intr_caps.nEncodingTypes = Swap16IfLE(intr_caps.nEncodingTypes);

  /* Read the lists of server- and client-initiated messages */
  return (ReadCapabilityList(serverMsgCaps, intr_caps.nServerMessageTypes) &&
	  ReadCapabilityList(clientMsgCaps, intr_caps.nClientMessageTypes) &&
	  ReadCapabilityList(encodingCaps, intr_caps.nEncodingTypes));
}


/*
 * Read the list of rfbCapabilityInfo structures and enable corresponding
 * capabilities in the specified container. The count argument specifies how
 * many records to read from the socket.
 */

static Bool
ReadCapabilityList(CapsContainer *caps, int count)
{
  rfbCapabilityInfo msginfo;
  int i;

  for (i = 0; i < count; i++) {
    if (!ReadFromRFBServer((char *)&msginfo, sz_rfbCapabilityInfo))
      return False;
    msginfo.code = Swap32IfLE(msginfo.code);
    CapsEnable(caps, &msginfo);
  }

  return True;
}


/*
 * SetFormatAndEncodings.
 */

Bool
SetFormatAndEncodings()
{
  rfbSetPixelFormatMsg spf;
  char buf[sz_rfbSetEncodingsMsg + MAX_ENCODINGS * 4];
  rfbSetEncodingsMsg *se = (rfbSetEncodingsMsg *)buf;
  CARD32 *encs = (CARD32 *)(&buf[sz_rfbSetEncodingsMsg]);
  int len = 0;
  Bool requestCompressLevel = False;
  Bool requestQualityLevel = False;
  Bool requestSubsampLevel = False;
  Bool requestLastRectEncoding = False;

  spf.type = rfbSetPixelFormat;
  spf.format = myFormat;
  spf.format.redMax = Swap16IfLE(spf.format.redMax);
  spf.format.greenMax = Swap16IfLE(spf.format.greenMax);
  spf.format.blueMax = Swap16IfLE(spf.format.blueMax);

  if (!WriteExact(rfbsock, (char *)&spf, sz_rfbSetPixelFormatMsg))
    return False;

  se->type = rfbSetEncodings;
  se->nEncodings = 0;

  if (appData.encodingsString) {
    char *encStr = appData.encodingsString;
    int encStrLen;
    do {
      char *nextEncStr = strchr(encStr, ' ');
      if (nextEncStr) {
	encStrLen = nextEncStr - encStr;
	nextEncStr++;
      } else {
	encStrLen = strlen(encStr);
      }

      if (strncasecmp(encStr,"raw",encStrLen) == 0) {
	encs[se->nEncodings++] = Swap32IfLE(rfbEncodingRaw);
      } else if (strncasecmp(encStr,"copyrect",encStrLen) == 0) {
	encs[se->nEncodings++] = Swap32IfLE(rfbEncodingCopyRect);
      } else if (strncasecmp(encStr,"tight",encStrLen) == 0) {
	encs[se->nEncodings++] = Swap32IfLE(rfbEncodingTight);
	requestLastRectEncoding = True;
	if (appData.compressLevel >= 0 && appData.compressLevel <= 9)
	  requestCompressLevel = True;
	if (appData.enableJPEG)
	  requestQualityLevel = True;
	  requestSubsampLevel = True;
      } else if (strncasecmp(encStr,"hextile",encStrLen) == 0) {
	encs[se->nEncodings++] = Swap32IfLE(rfbEncodingHextile);
      } else {
	fprintf(stderr,"Unknown encoding '%.*s'\n",encStrLen,encStr);
      }

      encStr = nextEncStr;
    } while (encStr && se->nEncodings < MAX_ENCODINGS);

    if (se->nEncodings < MAX_ENCODINGS && requestCompressLevel) {
      encs[se->nEncodings++] = Swap32IfLE(appData.compressLevel +
					  rfbEncodingCompressLevel0);
    }

    if (se->nEncodings < MAX_ENCODINGS && requestQualityLevel) {
      int tightQualityLevel;
      if (appData.qualityLevel < 1 || appData.qualityLevel > 100)
        appData.qualityLevel = 95;
      tightQualityLevel = appData.qualityLevel / 10;
      if (tightQualityLevel > 9) tightQualityLevel = 9;
      encs[se->nEncodings++] = Swap32IfLE(appData.qualityLevel +
					  rfbJpegQualityLevel1 - 1);
      encs[se->nEncodings++] = Swap32IfLE(tightQualityLevel +
					  rfbEncodingQualityLevel0);
    }

    if (se->nEncodings < MAX_ENCODINGS && requestSubsampLevel) {
      if (appData.subsampLevel < 0 || appData.subsampLevel > TVNC_SAMPOPT - 1)
        appData.subsampLevel = TVNC_1X;
      encs[se->nEncodings++] = Swap32IfLE(appData.subsampLevel +
					  rfbJpegSubsamp1X);
    }

    if (appData.useRemoteCursor) {
      if (se->nEncodings < MAX_ENCODINGS)
	encs[se->nEncodings++] = Swap32IfLE(rfbEncodingXCursor);
      if (se->nEncodings < MAX_ENCODINGS)
	encs[se->nEncodings++] = Swap32IfLE(rfbEncodingRichCursor);
      if (se->nEncodings < MAX_ENCODINGS)
	encs[se->nEncodings++] = Swap32IfLE(rfbEncodingPointerPos);
    }

    if (se->nEncodings < MAX_ENCODINGS && requestLastRectEncoding) {
      encs[se->nEncodings++] = Swap32IfLE(rfbEncodingLastRect);
    }
  }
  else {
    if (SameMachine(rfbsock)) {
      if (!tunnelSpecified) {
	fprintf(stderr,"Same machine: preferring raw encoding\n");
	encs[se->nEncodings++] = Swap32IfLE(rfbEncodingRaw);
      } else {
	fprintf(stderr,"Tunneling active: preferring tight encoding\n");
      }
    }

    encs[se->nEncodings++] = Swap32IfLE(rfbEncodingCopyRect);
    encs[se->nEncodings++] = Swap32IfLE(rfbEncodingTight);
    encs[se->nEncodings++] = Swap32IfLE(rfbEncodingHextile);

    if (appData.compressLevel >= 0 && appData.compressLevel <= 9) {
      encs[se->nEncodings++] = Swap32IfLE(appData.compressLevel +
					  rfbEncodingCompressLevel0);
    } else if (!tunnelSpecified) {
      /* If -tunnel option was provided, we assume that server machine is
	 not in the local network so we use default compression level for
	 tight encoding instead of fast compression. Thus we are
	 requesting level 1 compression only if tunneling is not used. */
      encs[se->nEncodings++] = Swap32IfLE(rfbEncodingCompressLevel1);
    }

    if (appData.enableJPEG) {
      int tightQualityLevel;
      if (appData.qualityLevel < 1 || appData.qualityLevel > 100)
        appData.qualityLevel = 95;
      tightQualityLevel = appData.qualityLevel / 10;
      if (tightQualityLevel > 9) tightQualityLevel = 9;
      encs[se->nEncodings++] = Swap32IfLE(appData.qualityLevel +
					  rfbJpegQualityLevel1 - 1);
      encs[se->nEncodings++] = Swap32IfLE(tightQualityLevel +
					  rfbEncodingQualityLevel0);

      if (appData.subsampLevel >= 0 && appData.subsampLevel <= TVNC_SAMPOPT-1)
        encs[se->nEncodings++] = Swap32IfLE(appData.subsampLevel +  
					    rfbJpegSubsamp1X);
    }

    if (appData.useRemoteCursor) {
      encs[se->nEncodings++] = Swap32IfLE(rfbEncodingXCursor);
      encs[se->nEncodings++] = Swap32IfLE(rfbEncodingRichCursor);
      encs[se->nEncodings++] = Swap32IfLE(rfbEncodingPointerPos);
    }

    encs[se->nEncodings++] = Swap32IfLE(rfbEncodingLastRect);
  }

  len = sz_rfbSetEncodingsMsg + se->nEncodings * 4;

  se->nEncodings = Swap16IfLE(se->nEncodings);

  if (!WriteExact(rfbsock, buf, len)) return False;

  return True;
}


/*
 * SendIncrementalFramebufferUpdateRequest.
 */

Bool
SendIncrementalFramebufferUpdateRequest()
{
  return SendFramebufferUpdateRequest(0, 0, si.framebufferWidth,
				      si.framebufferHeight, True);
}


/*
 * SendFramebufferUpdateRequest.
 */

Bool
SendFramebufferUpdateRequest(int x, int y, int w, int h, Bool incremental)
{
  rfbFramebufferUpdateRequestMsg fur;

  fur.type = rfbFramebufferUpdateRequest;
  fur.incremental = incremental ? 1 : 0;
  fur.x = Swap16IfLE(x);
  fur.y = Swap16IfLE(y);
  fur.w = Swap16IfLE(w);
  fur.h = Swap16IfLE(h);

  if (!WriteExact(rfbsock, (char *)&fur, sz_rfbFramebufferUpdateRequestMsg))
    return False;

  return True;
}


/*
 * SendPointerEvent.
 */

Bool
SendPointerEvent(int x, int y, int buttonMask)
{
  rfbPointerEventMsg pe;

  pe.type = rfbPointerEvent;
  pe.buttonMask = buttonMask;
  if (x < 0) x = 0;
  if (y < 0) y = 0;

  if (!appData.useX11Cursor)
    SoftCursorMove(x, y);

  pe.x = Swap16IfLE(x);
  pe.y = Swap16IfLE(y);
  return WriteExact(rfbsock, (char *)&pe, sz_rfbPointerEventMsg);
}


/*
 * SendKeyEvent.
 */

Bool
SendKeyEvent(CARD32 key, Bool down)
{
  rfbKeyEventMsg ke;

  ke.type = rfbKeyEvent;
  ke.down = down ? 1 : 0;
  ke.key = Swap32IfLE(key);
  return WriteExact(rfbsock, (char *)&ke, sz_rfbKeyEventMsg);
}


/*
 * QualHigh
 */
void
QualHigh(Widget w, XEvent *e, String *s, Cardinal *c)
{
  appData.encodingsString="tight copyrect";
  appData.enableJPEG=True;
  appData.subsampLevel=TVNC_1X;
  appData.qualityLevel=95;
  UpdateQual();
}

/*
 * QualMed
 */
void
QualMed(Widget w, XEvent *e, String *s, Cardinal *c)
{
  appData.encodingsString="tight copyrect";
  appData.enableJPEG=True;
  appData.subsampLevel=TVNC_2X;
  appData.qualityLevel=80;
  UpdateQual();
}

/*
 * QualLow
 */
void
QualLow(Widget w, XEvent *e, String *s, Cardinal *c)
{
  appData.encodingsString="tight copyrect";
  appData.enableJPEG=True;
  appData.subsampLevel=TVNC_4X;
  appData.qualityLevel=30;
  UpdateQual();
}

/*
 * QualLossless
 */
void
QualLossless(Widget w, XEvent *e, String *s, Cardinal *c)
{
  appData.encodingsString="tight copyrect";
  appData.enableJPEG=False;
  appData.compressLevel=0;
  UpdateQual();
}

/*
 * QualLosslessWAN
 */
void
QualLosslessWAN(Widget w, XEvent *e, String *s, Cardinal *c)
{
  appData.encodingsString="tight copyrect";
  appData.enableJPEG=False;
  appData.compressLevel=1;
  UpdateQual();
}

/*
 * SendClientCutText.
 */

Bool
SendClientCutText(char *str, int len)
{
  rfbClientCutTextMsg cct;

  if (serverCutText)
    free(serverCutText);
  serverCutText = NULL;

  cct.type = rfbClientCutText;
  cct.length = Swap32IfLE(len);
  return  (WriteExact(rfbsock, (char *)&cct, sz_rfbClientCutTextMsg) &&
	   WriteExact(rfbsock, str, len));
}


/*
 * HandleRFBServerMessage.
 */

Bool
HandleRFBServerMessage()
{
  rfbServerToClientMsg msg;

  if (!ReadFromRFBServer((char *)&msg, 1))
    return False;

  switch (msg.type) {

  case rfbSetColourMapEntries:
  {
    int i;
    CARD16 rgb[3];
    XColor xc;

    if (!ReadFromRFBServer(((char *)&msg) + 1,
			   sz_rfbSetColourMapEntriesMsg - 1))
      return False;

    msg.scme.firstColour = Swap16IfLE(msg.scme.firstColour);
    msg.scme.nColours = Swap16IfLE(msg.scme.nColours);

    for (i = 0; i < msg.scme.nColours; i++) {
      if (!ReadFromRFBServer((char *)rgb, 6))
	return False;
      xc.pixel = msg.scme.firstColour + i;
      xc.red = Swap16IfLE(rgb[0]);
      xc.green = Swap16IfLE(rgb[1]);
      xc.blue = Swap16IfLE(rgb[2]);
      xc.flags = DoRed|DoGreen|DoBlue;
      XStoreColor(dpy, cmap, &xc);
    }

    break;
  }

  case rfbFramebufferUpdate:
  {
    rfbFramebufferUpdateRectHeader rect;
    int linesToRead;
    int bytesPerLine;
    int i;
    int usecs;
    XEvent ev;

    memset(&ev, 0, sizeof(ev));
    ev.xclient.type=ClientMessage;
    ev.xclient.window=XtWindow(desktop);
    ev.xclient.message_type=XA_INTEGER;
    ev.xclient.format=8;
    strcpy(ev.xclient.data.b, "SendRFBUpdate");
    XSendEvent(dpy, XtWindow(desktop), False, 0, &ev);

    if (!ReadFromRFBServer(((char *)&msg.fu) + 1,
			   sz_rfbFramebufferUpdateMsg - 1))
      return False;

    msg.fu.nRects = Swap16IfLE(msg.fu.nRects);

    if (appData.doubleBuffer)
	list = NULL;

    for (i = 0; i < msg.fu.nRects; i++) {
      if (!ReadFromRFBServer((char *)&rect, sz_rfbFramebufferUpdateRectHeader))
	return False;

      rect.encoding = Swap32IfLE(rect.encoding);
      if (rect.encoding == rfbEncodingLastRect) {
        while (appData.doubleBuffer && list != NULL) {
	  rfbFramebufferUpdateRectHeader* r1;
          node = list;
	  r1 = &node->region;

	  if (r1->encoding == rfbEncodingTight
	     || r1->encoding == rfbEncodingRaw) {
	     SoftCursorLockArea(r1->r.x, r1->r.y, r1->r.w, r1->r.h); 
	     if (node->isFill) {
	       XChangeGC(dpy, gc, GCForeground, &node->gcv);
	       XFillRectangle(dpy, desktopWin, gc,
			      r1->r.x, r1->r.y, r1->r.w, r1->r.h);

	     } else
	        CopyDataToScreen(NULL, r1->r.x, r1->r.y, r1->r.w, r1->r.h);
	   }

	   list = list->next;
	   free(node);
	}
	SoftCursorUnlockScreen(); 
	break;
      }

      rect.r.x = Swap16IfLE(rect.r.x);
      rect.r.y = Swap16IfLE(rect.r.y);
      rect.r.w = Swap16IfLE(rect.r.w);
      rect.r.h = Swap16IfLE(rect.r.h);

      if (rect.encoding == rfbEncodingXCursor ||
	  rect.encoding == rfbEncodingRichCursor) {
	if (!HandleCursorShape(rect.r.x, rect.r.y, rect.r.w, rect.r.h,
			      rect.encoding)) {
	  return False;
	}
	continue;
      }

      if (rect.encoding == rfbEncodingPointerPos) {
	if (!HandleCursorPos(rect.r.x, rect.r.y)) {
	  return False;
	}
	continue;
      }

      if ((rect.r.x + rect.r.w > si.framebufferWidth) ||
	  (rect.r.y + rect.r.h > si.framebufferHeight))
	{
	  fprintf(stderr,"Rect too large: %dx%d at (%d, %d)\n",
		  rect.r.w, rect.r.h, rect.r.x, rect.r.y);
	  return False;
	}

      if (rect.r.h * rect.r.w == 0) {
	fprintf(stderr,"Zero size rect - ignoring\n");
	continue;
      }

      /* If RichCursor encoding is used, we should prevent collisions
	 between framebuffer updates and cursor drawing operations. */
      SoftCursorLockArea(rect.r.x, rect.r.y, rect.r.w, rect.r.h);

      if (appData.doubleBuffer) {
        node = (UpdateList *)malloc(sizeof(UpdateList));
	node->next = NULL;
	node->isFill = 0;
	memcpy(&(node->region), &rect, sz_rfbFramebufferUpdateRectHeader);
	if (list == NULL)
	  tail = list = node;
	else {
	  tail->next = node;
	  tail = node;
	}
      }

      switch (rect.encoding) {

      case rfbEncodingRaw:

	bytesPerLine = rect.r.w * myFormat.bitsPerPixel / 8;
	linesToRead = BUFFER_SIZE / bytesPerLine;

	while (rect.r.h > 0) {
	  if (linesToRead > rect.r.h)
	    linesToRead = rect.r.h;

	  if (!ReadFromRFBServer(buffer,bytesPerLine * linesToRead))
	    return False;

	  CopyDataToScreen(buffer, rect.r.x, rect.r.y, rect.r.w,
			   linesToRead);

	  rect.r.h -= linesToRead;
	  rect.r.y += linesToRead;

	}
	break;

      case rfbEncodingCopyRect:
      {
	rfbCopyRect cr;

	if (!ReadFromRFBServer((char *)&cr, sz_rfbCopyRect))
	  return False;

	cr.srcX = Swap16IfLE(cr.srcX);
	cr.srcY = Swap16IfLE(cr.srcY);

	/* If RichCursor encoding is used, we should extend our
	   "cursor lock area" (previously set to destination
	   rectangle) to the source rectangle as well. */
	SoftCursorLockArea(cr.srcX, cr.srcY, rect.r.w, rect.r.h);

	if (appData.copyRectDelay != 0) {
	  XFillRectangle(dpy, desktopWin, srcGC, cr.srcX, cr.srcY,
			 rect.r.w, rect.r.h);
	  XFillRectangle(dpy, desktopWin, dstGC, rect.r.x, rect.r.y,
			 rect.r.w, rect.r.h);
	  XSync(dpy,False);
	  usleep(appData.copyRectDelay * 1000);
	  XFillRectangle(dpy, desktopWin, dstGC, rect.r.x, rect.r.y,
			 rect.r.w, rect.r.h);
	  XFillRectangle(dpy, desktopWin, srcGC, cr.srcX, cr.srcY,
			 rect.r.w, rect.r.h);
	}

	XCopyArea(dpy, desktopWin, desktopWin, gc, cr.srcX, cr.srcY,
		  rect.r.w, rect.r.h, rect.r.x, rect.r.y);

	break;
      }

      case rfbEncodingHextile:
      {
	switch (myFormat.bitsPerPixel) {
	case 8:
	  if (!HandleHextile8(rect.r.x,rect.r.y,rect.r.w,rect.r.h))
	    return False;
	  break;
	case 16:
	  if (!HandleHextile16(rect.r.x,rect.r.y,rect.r.w,rect.r.h))
	    return False;
	  break;
	case 32:
	  if (!HandleHextile32(rect.r.x,rect.r.y,rect.r.w,rect.r.h))
	    return False;
	  break;
	}
	break;
      }

      case rfbEncodingTight:
      {
	switch (myFormat.bitsPerPixel) {
	case 8:
	  if (!HandleTight8(rect.r.x,rect.r.y,rect.r.w,rect.r.h))
	    return False;
	  break;
	case 16:
	  if (!HandleTight16(rect.r.x,rect.r.y,rect.r.w,rect.r.h))
	    return False;
	  break;
	case 32:
	  if (!HandleTight32(rect.r.x,rect.r.y,rect.r.w,rect.r.h))
	    return False;
	  break;
	}
	break;
      }

      default:
	fprintf(stderr,"Unknown rect encoding %d\n",
		(int)rect.encoding);
	return False;
      }

      /* Now we may discard "soft cursor locks". */
      SoftCursorUnlockScreen();
    }

    if (appData.doubleBuffer) {
  	while (list != NULL) {
	  rfbFramebufferUpdateRectHeader* r1;
  	  node = list;
	  r1 = &node->region;

	  if (r1->encoding == rfbEncodingTight
	    || r1->encoding == rfbEncodingRaw) {
	    SoftCursorLockArea(r1->r.x, r1->r.y, r1->r.w, r1->r.h);
	    if (node->isFill) {
		XChangeGC(dpy, gc, GCForeground, &node->gcv);
		XFillRectangle(dpy, desktopWin, gc,
				r1->r.x, r1->r.y, r1->r.w, r1->r.h);

	    } else
		CopyDataToScreen(NULL, r1->r.x, r1->r.y, r1->r.w, r1->r.h);
           }

	   list = list->next;
	   free(node);
        }
	SoftCursorUnlockScreen();
    }

#ifdef MITSHM
    /* if using shared memory PutImage, make sure that the X server has
       updated its framebuffer before we reuse the shared memory.  This is
       mainly to avoid copyrect using invalid screen contents - not sure
       if we'd need it otherwise. */

    if (appData.useShm)
      XSync(dpy, False);
#endif

    break;
  }

  case rfbBell:
  {
    Window toplevelWin;

    XBell(dpy, 0);

    if (appData.raiseOnBeep) {
      toplevelWin = XtWindow(toplevel);
      XMapRaised(dpy, toplevelWin);
    }

    break;
  }

  case rfbServerCutText:
  {
    if (!ReadFromRFBServer(((char *)&msg) + 1,
			   sz_rfbServerCutTextMsg - 1))
      return False;

    msg.sct.length = Swap32IfLE(msg.sct.length);

    if (serverCutText)
      free(serverCutText);

    serverCutText = malloc(msg.sct.length+1);

    if (!ReadFromRFBServer(serverCutText, msg.sct.length))
      return False;

    serverCutText[msg.sct.length] = 0;

    newServerCutText = True;

    break;
  }

  default:
    fprintf(stderr,"Unknown message type %d from VNC server\n",msg.type);
    return False;
  }

  return True;
}


#define GET_PIXEL8(pix, ptr) ((pix) = *(ptr)++)

#define GET_PIXEL16(pix, ptr) (((CARD8*)&(pix))[0] = *(ptr)++, \
			       ((CARD8*)&(pix))[1] = *(ptr)++)

#define GET_PIXEL32(pix, ptr) (((CARD8*)&(pix))[0] = *(ptr)++, \
			       ((CARD8*)&(pix))[1] = *(ptr)++, \
			       ((CARD8*)&(pix))[2] = *(ptr)++, \
			       ((CARD8*)&(pix))[3] = *(ptr)++)

/* CONCAT2 concatenates its two arguments.  CONCAT2E does the same but also
   expands its arguments if they are macros */

#define CONCAT2(a,b) a##b
#define CONCAT2E(a,b) CONCAT2(a,b)

#define BPP 8
#include "hextile.c"
#include "tight.c"
#undef BPP
#define BPP 16
#include "hextile.c"
#include "tight.c"
#undef BPP
#define BPP 32
#include "hextile.c"
#include "tight.c"
#undef BPP

/*
 * Read the string describing the reason for a connection failure.
 */

static void
ReadConnFailedReason(void)
{
  CARD32 reasonLen;
  char *reason = NULL;

  if (ReadFromRFBServer((char *)&reasonLen, sizeof(reasonLen))) {
    reasonLen = Swap32IfLE(reasonLen);
    if ((reason = malloc(reasonLen)) != NULL &&
        ReadFromRFBServer(reason, reasonLen)) {
      fprintf(stderr,"VNC connection failed: %.*s\n", (int)reasonLen, reason);
      free(reason);
      return;
    }
  }

  fprintf(stderr, "VNC connection failed\n");

  if (reason != NULL)
    free(reason);
}

/*
 * PrintPixelFormat.
 */

void
PrintPixelFormat(format)
    rfbPixelFormat *format;
{
  if (format->bitsPerPixel == 1) {
    fprintf(stderr,"  Single bit per pixel.\n");
    fprintf(stderr,
	    "  %s significant bit in each byte is leftmost on the screen.\n",
	    (format->bigEndian ? "Most" : "Least"));
  } else {
    fprintf(stderr,"  %d bits per pixel.\n",format->bitsPerPixel);
    if (format->bitsPerPixel != 8) {
      fprintf(stderr,"  %s significant byte first in each pixel.\n",
	      (format->bigEndian ? "Most" : "Least"));
    }
    if (format->trueColour) {
      fprintf(stderr,"  True colour: max red %d green %d blue %d",
	      format->redMax, format->greenMax, format->blueMax);
      fprintf(stderr,", shift red %d green %d blue %d\n",
	      format->redShift, format->greenShift, format->blueShift);
    } else {
      fprintf(stderr,"  Colour map (not true colour).\n");
    }
  }
}

/*
 * Read an integer value encoded in 1..3 bytes. This function is used
 * by the Tight decoder.
 */

static long
ReadCompactLen (void)
{
  long len;
  CARD8 b;

  if (!ReadFromRFBServer((char *)&b, 1))
    return -1;
  len = (int)b & 0x7F;
  if (b & 0x80) {
    if (!ReadFromRFBServer((char *)&b, 1))
      return -1;
    len |= ((int)b & 0x7F) << 7;
    if (b & 0x80) {
      if (!ReadFromRFBServer((char *)&b, 1))
	return -1;
      len |= ((int)b & 0xFF) << 14;
    }
  }
  return len;
}
