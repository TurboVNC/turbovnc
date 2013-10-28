/*
 *  Copyright (C) 2010 University Corporation for Atmospheric Research.
                       All Rights Reserved.
 *  Copyright (C) 2009-2013 D. R. Commander.  All Rights Reserved.
 *  Copyright (C) 2009-2011 Pierre Ossman for Cendio AB.  All Rights Reserved.
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

#ifndef _POSIX_PTHREAD_SEMANTICS
#define _POSIX_PTHREAD_SEMANTICS
#endif

#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <vncviewer.h>
#include <vncauth.h>
#include <zlib.h>
#include <pthread.h>
#include "turbojpeg.h"


/* OS X 10.4 doesn't define this */
#ifdef __APPLE__
#ifndef _SC_NPROCESSORS_CONF
#define _SC_NPROCESSORS_CONF 1
#endif
#endif

#ifndef min
 #define min(a, b) ((a) < (b) ? (a) : (b))
#endif


static void InitCapabilities(void);
static Bool SetupTunneling(void);
static int ReadSecurityType(void);
static int SelectSecurityType(void);
static Bool PerformAuthenticationTight(void);
static Bool AuthenticateVNC(void);
static Bool AuthenticateUnixLogin(void);
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
Bool encodingChange = False;


/*
 * Profiling stuff
 */

BOOL rfbProfile = FALSE;
static double tUpdate = 0., tStart = -1., tElapsed;
double tRecv = 0., tDecode = 0., tBlit = 0.;
unsigned long long decodePixels = 0, blitPixels = 0, recvBytes = 0;
unsigned long decodeRect = 0, blitRect = 0, updates = 0;

double gettime(void)
{
  struct timeval __tv;
  gettimeofday(&__tv, (struct timezone *)NULL);
  return((double)__tv.tv_sec + (double)__tv.tv_usec * 0.000001);
}


/* For double buffering */
UpdateList *list, *node, *tail;

int endianTest = 1;

static int protocolMinorVersion;
static Bool tightVncProtocol = False;
static CapsContainer *tunnelCaps;    /* known tunneling/encryption methods */
static CapsContainer *authCaps;      /* known authentication schemes       */
static CapsContainer *serverMsgCaps; /* known non-standard server messages */
static CapsContainer *clientMsgCaps; /* known non-standard client messages */
static CapsContainer *encodingCaps;  /* known encodings besides Raw        */


/* Note that the CoRRE encoding uses this buffer and assumes it is big enough
   to hold 255 * 255 * 32 bits = 260100 bytes.  640 * 480 = 307200 bytes.
   Hextile also assumes it is big enough to hold 16 * 16 * 32 bits.
   Tight encoding assumes BUFFER_SIZE is at least 16384 bytes. */

#define BUFFER_SIZE (640 * 480)
static char buffer[BUFFER_SIZE];


/*
 * Variables for the Tight encoding implementation.
 */

/* Four independent zlib compression streams */
z_stream zlibStream[4];
Bool zlibStreamActive[4] = {
  False, False, False, False
};

/* Filter stuff.  Should be initialized by the filter initialization code. */
static Bool cutZeros;
static CARD8 tightPrevRow[2048 * 3 * sizeof(CARD16)];


/* Multi-threading stuff */
static Bool threadInit = False;
static int nt;

typedef struct _threadparam
{
  int id, x, y, w, h, compressedLen;
  char *compressedData, *uncompressedData, *buffer;
  pthread_mutex_t ready, done;
  tjhandle tjhnd;
  Bool (*filterFn)(struct _threadparam *, int, int, int, int);
  Bool (*decompFn)(struct _threadparam *, int, int, int, int);
  Bool status, deadyet;
  z_streamp zs;
  int rectColors;
  char tightPalette[256 * 4];
  int rects;
} threadparam;

static threadparam tparam[TVNC_MAXTHREADS];
static pthread_t thnd[TVNC_MAXTHREADS] = {0, 0, 0, 0, 0, 0, 0, 0};
static int curthread = 0;

static int nthreads(void)
{
  char *mtenv = getenv("TVNC_MT");
  char *ntenv = getenv("TVNC_NTHREADS");
  int np = sysconf(_SC_NPROCESSORS_CONF), nt = 0;
  if (!mtenv || strlen(mtenv) < 1 || strcmp(mtenv, "1"))
    return 1;
  if (np == -1) np = 1;
  np = min(np, TVNC_MAXTHREADS);
  if (ntenv && strlen(ntenv) > 0) nt = atoi(ntenv);
  if (nt >= 1 && nt <= np) return nt;
  else return np;
}

static void *TightThreadFunc(void *param)
{
  threadparam *t = (threadparam *)param;
  while (!t->deadyet) {
    pthread_mutex_lock(&t->ready);
    if (t->deadyet) break;
    t->status = t->decompFn(t, t->x, t->y, t->w, t->h);
    if (t->status == True) t->rects++;
    pthread_mutex_unlock(&t->done);
  }
  return NULL;
}

static void InitThreads(void)
{
  int err = 0, i;
  if (threadInit) return;

  nt = nthreads();

  if (!appData.doubleBuffer || appData.rawDelay != 0) {
    if (nt != 1) {
      fprintf(stderr, "Multi-threading does not work with single buffering or raw delay options\n");
      nt = 1;
    }
  }

  if (nt > 1)
    fprintf(stderr, "Using %d thread%s for Tight decoding\n", nt,
      nt == 1 ? "" : "s");
  memset(tparam, 0, sizeof(threadparam) * TVNC_MAXTHREADS);

  for (i = 0; i < nt; i++) {
    tparam[i].id = i;
    tparam[i].status = True;
  }
  if (nt > 1) {
    for (i = 1; i < nt; i++) {
      pthread_mutex_init(&tparam[i].ready, NULL);
      pthread_mutex_lock(&tparam[i].ready);
      pthread_mutex_init(&tparam[i].done, NULL);
      if ((err = pthread_create(&thnd[i], NULL, TightThreadFunc,
        &tparam[i])) != 0) {
        fprintf(stderr, "Could not start thread %d: %s\n", i + 1,
          strerror(err == -1 ? errno : err));
        return;
      }
    }
  }
  threadInit = True;
}


void ShutdownThreads(void)
{
  int i;
  if (nt > 1) {
    for (i = 1; i < nt; i++) {
      if (thnd[i]) {
        tparam[i].deadyet = True;
        pthread_mutex_unlock(&tparam[i].ready);
        pthread_join(thnd[i], NULL);
      }
    }
  }
  for (i = 0; i < nt; i++) {
    if (tparam[i].compressedData) free(tparam[i].compressedData);
    if (tparam[i].uncompressedData) free(tparam[i].uncompressedData);
    if (tparam[i].buffer) free(tparam[i].buffer);
    memset(&tparam[i], 0, sizeof(threadparam));
  }
  threadInit = False;
}


static void NewNode(int x, int y, int w, int h, int encoding)
{
  if (appData.doubleBuffer) {
    node = (UpdateList *)malloc(sizeof(UpdateList));
    memset(node, 0, sizeof(UpdateList));
    node->region.r.x = x;
    node->region.r.y = y;
    node->region.r.w = w;
    node->region.r.h = h;
    node->region.encoding = encoding;
    if (list == NULL)
      tail = list = node;
    else {
      tail->next = node;
      tail = node;
    }
  }
}


static void FillRectangle(XGCValues *gcv, int x, int y, int w, int h)
{
  if (appData.doubleBuffer) {
    node->isFill = 1;
    memcpy(&node->gcv, gcv, sizeof(XGCValues));
  } else {
    double tBlitStart = 0.0;
    if (rfbProfile || benchFile) tBlitStart = gettime();
    XChangeGC(dpy, gc, GCForeground, gcv);
    XFillRectangle(dpy, desktopWin, gc, x, y, w, h);
    blitPixels += w * h;
    blitRect++;
    if (rfbProfile || benchFile) tBlit += gettime() - tBlitStart;
  }
}


static void CopyRectangle(int src_x, int src_y, int w, int h, int dest_x,
                          int dest_y)
{
  double tBlitStart = 0.0;
  if (!appData.doubleBuffer && (rfbProfile || benchFile))
    tBlitStart = gettime();
  if (appData.copyRectDelay != 0) {
    XFillRectangle(dpy, desktopWin, srcGC, src_x, src_y, w, h);
    XFillRectangle(dpy, desktopWin, dstGC, dest_x, dest_y, w, h);
    XSync(dpy, False);
    usleep(appData.copyRectDelay * 1000);
    XFillRectangle(dpy, desktopWin, dstGC, dest_x, dest_y, w, h);
    XFillRectangle(dpy, desktopWin, srcGC, src_x, src_y, w, h);
  }
  XCopyArea(dpy, desktopWin, desktopWin, gc, src_x, src_y, w, h, dest_x,
            dest_y);
  blitPixels += w * h;
  blitRect++;
  if (!appData.doubleBuffer && (rfbProfile || benchFile))
    tBlit += gettime() - tBlitStart;
}


static void InitCapabilities(void)
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
          "Standard VNC authentication");
  CapsAdd(authCaps, rfbAuthUnixLogin, rfbTightVncVendor, sig_rfbAuthUnixLogin,
          "Unix login authentication");

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
  if (appData.useRichCursor)
    CapsAdd(encodingCaps, rfbEncodingRichCursor, rfbTightVncVendor,
            sig_rfbEncodingRichCursor, "Rich-color cursor shape update");
  CapsAdd(encodingCaps, rfbEncodingPointerPos, rfbTightVncVendor,
          sig_rfbEncodingPointerPos, "Pointer position update");
  CapsAdd(encodingCaps, rfbEncodingLastRect, rfbTightVncVendor,
          sig_rfbEncodingLastRect, "LastRect protocol extension");
  CapsAdd(encodingCaps, rfbEncodingFineQualityLevel0, rfbTurboVncVendor,
          sig_rfbEncodingFineQualityLevel0, "TurboJPEG fine-grained quality level");
  CapsAdd(encodingCaps, rfbEncodingSubsamp1X, rfbTurboVncVendor,
          sig_rfbEncodingSubsamp1X, "TurboJPEG subsampling level");
}


Bool ConnectToRFBServer(const char *hostname, int port)
{
  rfbsock = ConnectToTcpAddr(hostname, port);

  if (rfbsock < 0) {
    fprintf(stderr, "Unable to connect to VNC server\n");
    return False;
  }

  return SetNonBlocking(rfbsock);
}


Bool ReadServerInitMessage(void)
{
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

  fprintf(stderr, "Desktop name \"%s\"\n", desktopName);

  fprintf(stderr, "VNC server default format:\n");
  PrintPixelFormat(&si.format);

  if (benchFile) benchFileStart = ftell(benchFile);

  return True;
}


Bool InitialiseRFBConnection(void)
{
  rfbProtocolVersionMsg pv;
  int server_major, server_minor;
  rfbClientInitMsg ci;
  int secType;
  char *env = NULL;

  /* If the connection is immediately closed, don't report anything so
     that pmw's monitor can make test connections */

  if (listenSpecified)
    errorMessageOnReadFailure = False;

  if (!ReadFromRFBServer(pv, sz_rfbProtocolVersionMsg))
    return False;

  errorMessageOnReadFailure = True;

  pv[sz_rfbProtocolVersionMsg] = 0;

  if (sscanf(pv, rfbProtocolVersionFormat,
             &server_major, &server_minor) != 2) {
    fprintf(stderr, "Not a valid VNC server\n");
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

  InitCapabilities();

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

  if (!ReadServerInitMessage())
    return False;

  if (tightVncProtocol) {
    /* Read interaction capabilities (protocol 3.7t, 3.8t) */
    if (!ReadInteractionCaps())
      return False;
  }

  if ((env = getenv("TVNC_PROFILE")) != NULL && !strcmp(env, "1"))
    rfbProfile = TRUE;

  return True;
}


/*
 * Read security type from the server (protocol 3.3)
 */

static int ReadSecurityType(void)
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

static int SelectSecurityType(void)
{
  CARD8 nSecTypes;
  CARD8 knownSecTypes[] = {rfbSecTypeNone, rfbSecTypeVncAuth};
  int nKnownSecTypes = sizeof(knownSecTypes);
  CARD8 *secTypes;
  CARD8 secType = rfbSecTypeInvalid;
  int i, j;

  /* Read the list of security types. */
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

static Bool SetupTunneling(void)
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
  CARD32 a;
  int i;

  /* In protocols 3.7t/3.8t, the server informs us about supported
     authentication schemes.  Here we read this information. */

  if (!ReadFromRFBServer((char *)&caps, sz_rfbAuthenticationCapsMsg))
    return False;

  caps.nAuthTypes = Swap32IfLE(caps.nAuthTypes);

  /* Special case - empty capability list stands for no authentication. */
  if (!caps.nAuthTypes)
    return AuthenticateNone();

  if (!ReadCapabilityList(authCaps, caps.nAuthTypes))
    return False;

  authScheme = 0;
  if (!appData.noUnixLogin && (appData.userLogin != NULL)) {
    /* Prefer Unix Login over other types */
    for (i = 0; i < CapsNumEnabled(authCaps); i++) {
      if (CapsGetByOrder(authCaps, i) == rfbAuthUnixLogin) {
        authScheme = rfbAuthUnixLogin;
        break;
      }
    }
  }

  if (authScheme == 0) {
    /* Try server's preferred authentication scheme. */
    for (i = 0; (authScheme == 0) && (i < CapsNumEnabled(authCaps)); i++) {
      a = CapsGetByOrder(authCaps, i);
      switch (a) {
        case rfbAuthVNC:
        case rfbAuthNone:
          authScheme = a;
          break;

        case rfbAuthUnixLogin:
          if (!appData.noUnixLogin)
            authScheme = a;
          break;

        default:
          /* unknown scheme - cannot use it */
          continue;
      }
    }
  }

  if (authScheme == 0) {
    fprintf(stderr, "No suitable authentication schemes offered by server\n");
    return False;
  }

  authScheme = Swap32IfLE(authScheme);
  if (!WriteExact(rfbsock, (char *)&authScheme, sizeof(authScheme)))
    return False;

  authScheme = Swap32IfLE(authScheme); /* convert it back */
  switch (authScheme) {
    case rfbAuthNone:
      return AuthenticateNone();

    case rfbAuthVNC:
      return AuthenticateVNC();

    case rfbAuthUnixLogin:
      return AuthenticateUnixLogin();

    default:                      /* should never happen */
      fprintf(stderr, "Internal error: Invalid authentication scheme\n");
      return False;
  }
}


/*
 * Null authentication.
 */

static Bool AuthenticateNone(void)
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

extern char encryptedPassword[9];
extern Bool encryptedPasswordSet;

static Bool AuthenticateVNC(void)
{
  CARD8 challenge[CHALLENGESIZE];
  char *passwd;
  char  buffer[64];
  char* cstatus;
  int   len;

  fprintf(stderr, "Performing standard VNC authentication\n");

  if (!ReadFromRFBServer((char *)challenge, CHALLENGESIZE))
    return False;

  if (encryptedPasswordSet) {
    passwd = buffer;
    if (!vncDecryptPasswd(encryptedPassword, passwd)) {
      fprintf(stderr, "Password stored in connection info file is invalid.\n");
      return False;
    }
  } else if (appData.passwordFile) {
    passwd = vncDecryptPasswdFromFile(appData.passwordFile);
    if (!passwd) {
      fprintf(stderr, "Cannot read valid password from file \"%s\"\n",
              appData.passwordFile);
      return False;
    }
  } else if (appData.autoPass) {
    passwd = buffer;
    cstatus = fgets(buffer, sizeof(buffer), stdin);
    if (cstatus == NULL)
       buffer[0] = '\0';
    else {
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
 * Unix Logon authentication.
 */

static Bool AuthenticateUnixLogin(void)
{
  char* user;
  char* passwd;
  char  buf[64], buf2[256];
  char  buffer[256];
  CARD32 userLen;
  CARD32 pwdLen;
  CARD32 t;
  struct passwd pwbuf;
  struct passwd* pw;
  BOOL curUser = FALSE;

  fprintf(stderr, "Performing Unix Login VNC authentication\n");
  user = passwd = NULL;
  if ((appData.userLogin != NULL) && (strlen(appData.userLogin) > 0))
    user = appData.userLogin;

  if (user && appData.autoPass) {
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
      buffer[0] = '\0';
      DoUserPwdDialog(&user, &passwd);
    } else {
      passwd = buffer;
      int len = strlen(buffer);
      if (len > 0 && buffer[len - 1] == '\n')
        buffer[len - 1] = '\0';
    }
  } else if (appData.passwordDialog) {
    DoUserPwdDialog(&user, &passwd);
  } else {
    if (user == NULL) {
      buf2[0] = 0;
      if (getpwuid_r(getuid(), &pwbuf, buf2, sizeof(buf2), &pw) == 0 &&
          pwbuf.pw_name && strlen(pwbuf.pw_name) > 0)
        curUser = TRUE;
      if (curUser)
        fprintf(stdout, "User (%s): ", pwbuf.pw_name);
      else
        fprintf(stdout, "User: ");
      fflush(stdout);
      if (fgets(buf, sizeof(buf), stdin) == NULL) {
        if (curUser)
          strncpy(buf, pwbuf.pw_name, 63);
        else {
          fprintf(stderr, "Reading user name failed\n");
          return False;
        }
      }

      userLen = strlen(buf);
      if (userLen > 0) {
        if (buf[userLen - 1] == '\n')
          buf[--userLen] = '\0';
      }
      if (userLen == 0 && curUser) {
        strncpy(buf, pwbuf.pw_name, 63);
        userLen = strlen(buf);
      }

      user = buf;
    }

    if (strlen(user) > 0)
      passwd = getpass("Password: ");
  }

  if (!user || ((userLen = strlen(user)) == 0)) {
    fprintf(stderr, "Reading user name failed\n");
    return False;
  }

  if (!passwd || (pwdLen = strlen(passwd)) == 0) {
    fprintf(stderr, "Reading password failed\n");
    return False;
  }

  t = Swap32IfLE(userLen);
  if (!WriteExact(rfbsock, (char *)&t, sizeof(t)))
    return False;

  t = Swap32IfLE(pwdLen);
  if (!WriteExact(rfbsock, (char *)&t, sizeof(t)))
    return False;

  if (!WriteExact(rfbsock, (char *)user, userLen))
    return False;

  if (!WriteExact(rfbsock, (char *)passwd, pwdLen))
    return False;

  memset(buffer, '\0', sizeof(buffer));

  while (*passwd == '\0')
    *passwd++ = '\0';

  return ReadAuthenticationResult();
}


/*
 * Read and report the result of authentication.
 */

static Bool ReadAuthenticationResult(void)
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
      if (protocolMinorVersion >= 8)
        ReadConnFailedReason();
      else
        fprintf(stderr, "Authentication failure\n");
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

static Bool ReadInteractionCaps(void)
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
 * capabilities in the specified container.  The count argument specifies how
 * many records to read from the socket.
 */

static Bool ReadCapabilityList(CapsContainer *caps, int count)
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


static Bool SetFormatAndEncodings()
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

      if (strncasecmp(encStr, "raw", encStrLen) == 0) {
        encs[se->nEncodings++] = Swap32IfLE(rfbEncodingRaw);
      } else if (strncasecmp(encStr, "copyrect", encStrLen) == 0) {
        encs[se->nEncodings++] = Swap32IfLE(rfbEncodingCopyRect);
      } else if (strncasecmp(encStr, "tight", encStrLen) == 0) {
        encs[se->nEncodings++] = Swap32IfLE(rfbEncodingTight);
        requestLastRectEncoding = True;
        if (appData.compressLevel >= 0 && appData.compressLevel <= 9)
          requestCompressLevel = True;
        if (appData.enableJPEG)
          requestQualityLevel = True;
          requestSubsampLevel = True;
      } else if (strncasecmp(encStr, "hextile", encStrLen) == 0) {
        encs[se->nEncodings++] = Swap32IfLE(rfbEncodingHextile);
      } else {
        fprintf(stderr, "Unknown encoding '%.*s'\n", encStrLen, encStr);
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
      encs[se->nEncodings++] = Swap32IfLE(tightQualityLevel +
                                          rfbEncodingQualityLevel0);
      encs[se->nEncodings++] = Swap32IfLE(appData.qualityLevel +
                                          rfbEncodingFineQualityLevel0);
    }

    if (se->nEncodings < MAX_ENCODINGS && requestSubsampLevel) {
      if (appData.subsampLevel < 0 || appData.subsampLevel > TVNC_SAMPOPT - 1)
        appData.subsampLevel = TVNC_1X;
      encs[se->nEncodings++] = Swap32IfLE(appData.subsampLevel +
                                          rfbEncodingSubsamp1X);
    }

    if (appData.cursorShape) {
      if (se->nEncodings < MAX_ENCODINGS && appData.useRichCursor)
        encs[se->nEncodings++] = Swap32IfLE(rfbEncodingRichCursor);
      if (se->nEncodings < MAX_ENCODINGS)
        encs[se->nEncodings++] = Swap32IfLE(rfbEncodingXCursor);
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
        fprintf(stderr, "Same machine: preferring raw encoding\n");
        encs[se->nEncodings++] = Swap32IfLE(rfbEncodingRaw);
      } else {
        fprintf(stderr, "Tunneling active: preferring tight encoding\n");
      }
    }

    encs[se->nEncodings++] = Swap32IfLE(rfbEncodingCopyRect);
    encs[se->nEncodings++] = Swap32IfLE(rfbEncodingTight);
    encs[se->nEncodings++] = Swap32IfLE(rfbEncodingHextile);

    if (appData.compressLevel >= 0 && appData.compressLevel <= 9) {
      encs[se->nEncodings++] = Swap32IfLE(appData.compressLevel +
                                          rfbEncodingCompressLevel0);
    } else if (!tunnelSpecified) {
      /* If -tunnel option was provided, we assume that the server machine is
         not in the local network, so we use the default compression level for
         Tight encoding instead of fast compression.  Thus, we are
         requesting level 1 compression only if tunneling is not used. */
      encs[se->nEncodings++] = Swap32IfLE(rfbEncodingCompressLevel1);
    }

    if (appData.enableJPEG) {
      int tightQualityLevel;
      if (appData.qualityLevel < 1 || appData.qualityLevel > 100)
        appData.qualityLevel = 95;
      tightQualityLevel = appData.qualityLevel / 10;
      if (tightQualityLevel > 9) tightQualityLevel = 9;
      encs[se->nEncodings++] = Swap32IfLE(tightQualityLevel +
                                          rfbEncodingQualityLevel0);
      encs[se->nEncodings++] = Swap32IfLE(appData.qualityLevel +
                                          rfbEncodingFineQualityLevel0);

      if (appData.subsampLevel >= 0 && appData.subsampLevel <= TVNC_SAMPOPT-1)
        encs[se->nEncodings++] = Swap32IfLE(appData.subsampLevel +
                                            rfbEncodingSubsamp1X);
    }

    if (appData.cursorShape) {
      if (appData.useRichCursor)
        encs[se->nEncodings++] = Swap32IfLE(rfbEncodingRichCursor);
      encs[se->nEncodings++] = Swap32IfLE(rfbEncodingXCursor);
      encs[se->nEncodings++] = Swap32IfLE(rfbEncodingPointerPos);
    }

    encs[se->nEncodings++] = Swap32IfLE(rfbEncodingLastRect);
  }

  if (appData.continuousUpdates) {
    encs[se->nEncodings++] = Swap32IfLE(rfbEncodingContinuousUpdates);
    encs[se->nEncodings++] = Swap32IfLE(rfbEncodingFence);
  }

  len = sz_rfbSetEncodingsMsg + se->nEncodings * 4;

  se->nEncodings = Swap16IfLE(se->nEncodings);

  if (!WriteExact(rfbsock, buf, len)) return False;

  return True;
}


Bool SendIncrementalFramebufferUpdateRequest()
{
  return SendFramebufferUpdateRequest(0, 0, si.framebufferWidth,
                                      si.framebufferHeight, True);
}


Bool SendFramebufferUpdateRequest(int x, int y, int w, int h, Bool incremental)
{
  rfbFramebufferUpdateRequestMsg fur;

  if (encodingChange) {
    SetFormatAndEncodings();
    encodingChange = False;
  }
  if (incremental && continuousUpdates)
    return True;

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


Bool SendPointerEvent(int x, int y, int buttonMask)
{
  rfbPointerEventMsg pe;

  pe.type = rfbPointerEvent;
  pe.buttonMask = buttonMask;
  if (x < 0) x = 0;
  if (y < 0) y = 0;

  pe.x = Swap16IfLE(x);
  pe.y = Swap16IfLE(y);
  return WriteExact(rfbsock, (char *)&pe, sz_rfbPointerEventMsg);
}


Bool SendKeyEvent(CARD32 key, Bool down)
{
  rfbKeyEventMsg ke;

  ke.type = rfbKeyEvent;
  ke.down = down ? 1 : 0;
  ke.key = Swap32IfLE(key);
  return WriteExact(rfbsock, (char *)&ke, sz_rfbKeyEventMsg);
}


/*
 * ToggleViewOnly() is an action that toggles in and out of view-only mode.
 */

void ToggleViewOnly(Widget w, XEvent *ev, String *params, Cardinal *num_params)
{
  appData.viewOnly = !appData.viewOnly;
}


/*
 * QualHigh() is an action that enables the Tight + Perceptually Lossless JPEG
 * encoding method
 */

void QualHigh(Widget w, XEvent *e, String *s, Cardinal *c)
{
  appData.encodingsString = "tight copyrect";
  appData.enableJPEG = True;
  appData.subsampLevel = TVNC_1X;
  appData.qualityLevel = 95;
  appData.compressLevel = 1;
  UpdateQual();
}


/*
 * QualMed() is an action that enables the Tight + Medium Quality JPEG
 * encoding method
 */

void QualMed(Widget w, XEvent *e, String *s, Cardinal *c)
{
  appData.encodingsString = "tight copyrect";
  appData.enableJPEG = True;
  appData.subsampLevel = TVNC_2X;
  appData.qualityLevel = 80;
  appData.compressLevel = 1;
  UpdateQual();
}


/*
 * QualLow() is an action that enables the Tight + Low Quality JPEG
 * encoding method
 */

void QualLow(Widget w, XEvent *e, String *s, Cardinal *c)
{
  appData.encodingsString = "tight copyrect";
  appData.enableJPEG = True;
  appData.subsampLevel = TVNC_4X;
  appData.qualityLevel = 30;
  appData.compressLevel = 1;
  UpdateQual();
}


/*
 * QualLossless() is an action that enables the Lossless Tight encoding method
 */

void QualLossless(Widget w, XEvent *e, String *s, Cardinal *c)
{
  appData.encodingsString = "tight copyrect";
  appData.enableJPEG = False;
  appData.compressLevel = 0;
  UpdateQual();
}


/*
 * QualLosslessWAN() is an action that enables the Lossless Tight + Zlib
 * encoding method
 */

void QualLosslessWAN(Widget w, XEvent *e, String *s, Cardinal *c)
{
  appData.encodingsString = "tight copyrect";
  appData.enableJPEG = False;
  appData.compressLevel = 1;
  UpdateQual();
}


Bool SendClientCutText(char *str, int len)
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


Bool HandleRFBServerMessage()
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
        xc.flags = DoRed | DoGreen | DoBlue;
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
      XEvent ev;
      double tDecodeStart = 0., tBlitStart = 0., tUpdateStart = 0.,
        tRecvOld = 0., tBlitOld = 0.;
      static Bool firstUpdate = True;

      if (rfbProfile) {
        tUpdateStart = gettime();
        if (tStart < 0.) tStart = tUpdateStart;
      }

      memset(&ev, 0, sizeof(ev));
      ev.xclient.type = ClientMessage;
      ev.xclient.window = XtWindow(desktop);
      ev.xclient.message_type = XA_INTEGER;
      ev.xclient.format = 8;
      strcpy(ev.xclient.data.b, "SendRFBUpdate");
      XSendEvent(dpy, XtWindow(desktop), False, 0, &ev);

      if (!ReadFromRFBServer(((char *)&msg.fu) + 1,
                             sz_rfbFramebufferUpdateMsg - 1))
        return False;

      msg.fu.nRects = Swap16IfLE(msg.fu.nRects);

      if (appData.doubleBuffer)
        list = NULL;

      if (!threadInit) {
        InitThreads();
        if (!threadInit) return False;
      }

      updates++;

      for (i = 0; i < msg.fu.nRects; i++) {
        if (!ReadFromRFBServer((char *)&rect, sz_rfbFramebufferUpdateRectHeader))
          return False;

        rect.encoding = Swap32IfLE(rect.encoding);
        if (rect.encoding == rfbEncodingLastRect) {
          for (i = 1; i < nt; i++) {
            pthread_mutex_lock(&tparam[i].done);
            pthread_mutex_unlock(&tparam[i].done);
          }
          if (rfbProfile || benchFile) tBlitStart = gettime();
          while (appData.doubleBuffer && list != NULL) {
            rfbFramebufferUpdateRectHeader* r1;
            node = list;
            r1 = &node->region;

            if (r1->encoding == rfbEncodingTight ||
                r1->encoding == rfbEncodingRaw ||
                r1->encoding == rfbEncodingHextile ||
                r1->encoding == rfbEncodingCopyRect) {
              if (node->isFill) {
                XChangeGC(dpy, gc, GCForeground, &node->gcv);
                XFillRectangle(dpy, desktopWin, gc,
                               r1->r.x, r1->r.y, r1->r.w, r1->r.h);
                blitPixels += r1->r.w * r1->r.h;
                blitRect++;
              } else if (node->isCopyRect) {
                CopyRectangle(node->crx, node->cry, r1->r.w, r1->r.h, r1->r.x,
                              r1->r.y);
              } else
                CopyImageToScreen(r1->r.x, r1->r.y, r1->r.w, r1->r.h);
            }

            list = list->next;
            free(node);
          }
          if (rfbProfile || benchFile) tBlit += gettime() - tBlitStart;
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
          if (!HandleCursorPos(rect.r.x, rect.r.y))
            return False;
          continue;
        }

        if ((rect.r.x + rect.r.w > si.framebufferWidth) ||
            (rect.r.y + rect.r.h > si.framebufferHeight)) {
          fprintf(stderr, "Rect too large: %dx%d at (%d, %d)\n",
                  rect.r.w, rect.r.h, rect.r.x, rect.r.y);
          return False;
        }

        if (rect.r.h * rect.r.w == 0)
          fprintf(stderr, "WARNING: Zero size rect\n");

        if (rfbProfile || benchFile) {
          tDecodeStart = gettime();
          tRecvOld = tRecv;
          tBlitOld = tBlit;
        }

        decodePixels += (double)rect.r.w * (double)rect.r.h;
        decodeRect++;

        switch (rect.encoding) {

          case rfbEncodingRaw:
            SetLastEncoding(rect.encoding);
            NewNode(rect.r.x, rect.r.y, rect.r.w, rect.r.h, rect.encoding);

            bytesPerLine = rect.r.w * myFormat.bitsPerPixel / 8;
            linesToRead = BUFFER_SIZE / bytesPerLine;

            while (rect.r.h > 0) {
              if (linesToRead > rect.r.h)
                linesToRead = rect.r.h;

              if (!ReadFromRFBServer(buffer, bytesPerLine * linesToRead))
                return False;

              CopyDataToImage(buffer, rect.r.x, rect.r.y, rect.r.w,
                              linesToRead);

              rect.r.h -= linesToRead;
              rect.r.y += linesToRead;

            }
            break;

          case rfbEncodingCopyRect:
          {
            rfbCopyRect cr;

            NewNode(rect.r.x, rect.r.y, rect.r.w, rect.r.h, rect.encoding);
            if (!ReadFromRFBServer((char *)&cr, sz_rfbCopyRect))
              return False;

            cr.srcX = Swap16IfLE(cr.srcX);
            cr.srcY = Swap16IfLE(cr.srcY);

            if (appData.doubleBuffer) {
              node->isCopyRect = True;
              node->crx = cr.srcX;
              node->cry = cr.srcY;
            } else
              CopyRectangle(cr.srcX, cr.srcY, rect.r.w, rect.r.h, rect.r.x,
                            rect.r.y);

            break;
          }

          case rfbEncodingHextile:
          {
            SetLastEncoding(rect.encoding);
            switch (myFormat.bitsPerPixel) {
            case 8:
              if (!HandleHextile8(rect.r.x, rect.r.y, rect.r.w, rect.r.h))
                return False;
              break;
            case 16:
              if (!HandleHextile16(rect.r.x, rect.r.y, rect.r.w, rect.r.h))
                return False;
              break;
            case 32:
              if (!HandleHextile32(rect.r.x, rect.r.y, rect.r.w, rect.r.h))
                return False;
              break;
            }
            break;
          }

          case rfbEncodingTight:
          {
            SetLastEncoding(rect.encoding);
            NewNode(rect.r.x, rect.r.y, rect.r.w, rect.r.h, rect.encoding);

            switch (myFormat.bitsPerPixel) {
            case 8:
              if (!HandleTight8(rect.r.x, rect.r.y, rect.r.w, rect.r.h))
                return False;
              break;
            case 16:
              if (!HandleTight16(rect.r.x, rect.r.y, rect.r.w, rect.r.h))
                return False;
              break;
            case 32:
              if (!HandleTight32(rect.r.x, rect.r.y, rect.r.w, rect.r.h))
                return False;
              break;
            }
            break;
          }

          default:
            fprintf(stderr, "Unknown rect encoding %d\n",
                    (int)rect.encoding);
            return False;
        }

        if (rfbProfile || benchFile)
          tDecode += gettime() - tDecodeStart - (tRecv - tRecvOld) -
                     (tBlit - tBlitOld);
      }

      for (i = 1; i < nt; i++) {
        pthread_mutex_lock(&tparam[i].done);
        pthread_mutex_unlock(&tparam[i].done);
      }
      for (i = 1; i < nt; i++) {
        if (tparam[i].status == False) return False;
      }

      if (appData.doubleBuffer) {
        if (rfbProfile || benchFile) tBlitStart = gettime();
        while (list != NULL) {
          rfbFramebufferUpdateRectHeader* r1;
          node = list;
          r1 = &node->region;

          if (r1->encoding == rfbEncodingTight ||
              r1->encoding == rfbEncodingRaw ||
              r1->encoding == rfbEncodingHextile ||
              r1->encoding == rfbEncodingCopyRect) {
            if (node->isFill) {
              XChangeGC(dpy, gc, GCForeground, &node->gcv);
              XFillRectangle(dpy, desktopWin, gc,
                             r1->r.x, r1->r.y, r1->r.w, r1->r.h);
              blitPixels += r1->r.w * r1->r.h;
              blitRect++;
            } else if (node->isCopyRect) {
              CopyRectangle(node->crx, node->cry, r1->r.w, r1->r.h, r1->r.x,
                            r1->r.y);
            } else
              CopyImageToScreen(r1->r.x, r1->r.y, r1->r.w, r1->r.h);
          }

          list = list->next;
          free(node);
        }
        if (rfbProfile || benchFile) tBlit += gettime() - tBlitStart;
      }

#ifdef MITSHM
      /* if using shared memory PutImage, make sure that the X server has
         updated its framebuffer before we reuse the shared memory.  This is
         mainly to avoid CopyRect using invalid screen contents - not sure
         if we'd need it otherwise. */

      if (appData.useShm) {
        if (rfbProfile || benchFile) tBlitStart = gettime();
        XSync(dpy, False);
        if (rfbProfile || benchFile) tBlit += gettime() - tBlitStart;
      }
#endif

      if (firstUpdate) {
        /* We need fences in order to make extra update requests and continuous
           updates "safe".  See HandleFence() for the next step. */
        if (supportsFence)
          SendFence(rfbFenceFlagRequest | rfbFenceFlagSyncNext, 0, NULL);

        firstUpdate = False;
      }

      if (rfbProfile && !benchFile) {
        tUpdate += gettime() - tUpdateStart;
        tElapsed = gettime() - tStart;

        if (tElapsed > 5.) {
          printf("-------------------------------------------------------------------------------\n");
          printf("Total:   %.3f updates/sec,  %.3f Mpixels/sec,  %.3f Mbits/sec\n",
                 (double)updates / tElapsed,
                 (double)decodePixels / 1000000. / tElapsed,
                 (double)recvBytes / 125000. / tElapsed);
          printf("         %lu rect,  %.0f pixels/rect,  %.0f rect/update\n",
                 decodeRect, (double)decodePixels / (double)decodeRect,
                 (double)decodeRect / (double)updates);
          printf("Decode:  %.3f Mpixels/sec      Blit:  %.3f Mpixels/sec\n",
                 (double)decodePixels / 1000000. / tDecode,
                 (double)blitPixels / 1000000. / tBlit);
          printf("Time/update:  Recv = %.3f ms,  Decode = %.3f ms,  Blit = %.3f ms\n",
                 tRecv / (double)updates * 1000.,
                 tDecode / (double)updates * 1000.,
                 tBlit / (double)updates * 1000.);
          printf("              Total = %.3f ms  +  Overhead = %.3f ms\n",
                 tUpdate / (double)updates * 1000.,
                 (tElapsed - tUpdate) / (double)updates * 1000.);
          tUpdate = tRecv = tDecode = tBlit = 0.;
          decodePixels = blitPixels = recvBytes = 0;
          updates = decodeRect = 0;
          tStart = gettime();
        }
      }
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

      serverCutText = malloc(msg.sct.length + 1);

      if (!ReadFromRFBServer(serverCutText, msg.sct.length))
        return False;

      serverCutText[msg.sct.length] = 0;

      newServerCutText = True;

      break;
    }

    case rfbFence:
    {
      CARD32 flags;
      char data[64];

      if (!ReadFromRFBServer(((char *)&msg.f) + 1, sz_rfbFenceMsg - 1))
        return False;

      flags = Swap32IfLE(msg.f.flags);

      if (msg.f.length > 0 && !ReadFromRFBServer(data, msg.f.length))
        return False;

      supportsFence = True;

      if (msg.f.length > sizeof(data))
        fprintf(stderr, "Ignoring fence.  Payload of %d bytes is too large.\n",
                msg.f.length);
      else
        if (!HandleFence(flags, msg.f.length, data))
          return False;

      break;
    }

    case rfbEndOfContinuousUpdates:
      supportsCU = True;
      break;

    default:
      fprintf(stderr, "Unknown message type %d from VNC server\n", msg.type);
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

#define CONCAT2(a, b) a##b
#define CONCAT2E(a, b) CONCAT2(a, b)

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

static void ReadConnFailedReason(void)
{
  CARD32 reasonLen;
  char *reason = NULL;

  if (ReadFromRFBServer((char *)&reasonLen, sizeof(reasonLen))) {
    reasonLen = Swap32IfLE(reasonLen);
    if ((reason = malloc(reasonLen)) != NULL &&
        ReadFromRFBServer(reason, reasonLen)) {
      fprintf(stderr, "VNC connection failed: %.*s\n", (int)reasonLen, reason);
      free(reason);
      return;
    }
  }

  fprintf(stderr, "VNC connection failed\n");

  if (reason != NULL)
    free(reason);
}


void PrintPixelFormat(rfbPixelFormat *format)
{
  if (format->bitsPerPixel == 1) {
    fprintf(stderr, "  Single bit per pixel.\n");
    fprintf(stderr,
            "  %s significant bit in each byte is leftmost on the screen.\n",
            (format->bigEndian ? "Most" : "Least"));
  } else {
    fprintf(stderr, "  %d bits per pixel.\n", format->bitsPerPixel);
    if (format->bitsPerPixel != 8) {
      fprintf(stderr, "  %s significant byte first in each pixel.\n",
              (format->bigEndian ? "Most" : "Least"));
    }
    if (format->trueColour) {
      fprintf(stderr, "  True color: max red %d green %d blue %d",
              format->redMax, format->greenMax, format->blueMax);
      fprintf(stderr, ", shift red %d green %d blue %d\n",
              format->redShift, format->greenShift, format->blueShift);
    } else {
      fprintf(stderr, "  Color map (not true color).\n");
    }
  }
}


/*
 * The length of compressed data is transmitted as 1-3 bytes, in which each
 * byte contains 7 bits of actual data, and the 8th bit is set to indicate
 * that an additional byte should be read.
 */

static long ReadCompactLen (void)
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
