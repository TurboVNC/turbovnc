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

/*
 * rfbproto.c - functions to deal with client side of RFB protocol.
 */

#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <vncviewer.h>
#include <vncauth.h>
#include <zlib.h>
#include "hpjpeg.h"

static Bool HandleTight16(int rx, int ry, int rw, int rh);
static Bool HandleTight32(int rx, int ry, int rw, int rh);

static long ReadCompactLen (void);

int rfbsock;
char *desktopName;
rfbPixelFormat myFormat;
rfbServerInitMsg si;
char *serverCutText = NULL;
Bool newServerCutText = False;
hpjhandle hpjhnd=NULL;

/* For double buffering */
UpdateList *list, *node, *tail;

int endianTest = 1;


/* Note that the CoRRE encoding uses this buffer and assumes it is big enough
   to hold 255 * 255 * 32 bits -> 260100 bytes.  640*480 = 307200 bytes.
   Hextile also assumes it is big enough to hold 16 * 16 * 32 bits.
   Tight encoding assumes BUFFER_SIZE is at least 16384 bytes. */

#define BUFFER_SIZE (640*480)
static char buffer[BUFFER_SIZE];


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
InitialiseRFBConnection()
{
  rfbProtocolVersionMsg pv;
  int major,minor;
  CARD32 authScheme, reasonLen, authResult;
  char *reason;
  CARD8 challenge[CHALLENGESIZE];
  char *passwd;
  int i;
  rfbClientInitMsg ci;

  /* if the connection is immediately closed, don't report anything, so
       that pmw's monitor can make test connections */

  if (listenSpecified)
    errorMessageOnReadFailure = False;

  if (!ReadFromRFBServer(pv, sz_rfbProtocolVersionMsg)) return False;

  errorMessageOnReadFailure = True;

  pv[sz_rfbProtocolVersionMsg] = 0;

  if (sscanf(pv,rfbProtocolVersionFormat,&major,&minor) != 2) {
    fprintf(stderr,"Not a valid VNC server\n");
    return False;
  }

  fprintf(stderr,"VNC server supports protocol version %d.%d (viewer %d.%d)\n",
	  major, minor, rfbProtocolMajorVersion, rfbProtocolMinorVersion);

  major = rfbProtocolMajorVersion;
  minor = rfbProtocolMinorVersion;

  sprintf(pv,rfbProtocolVersionFormat,major,minor);

  if (!WriteExact(rfbsock, pv, sz_rfbProtocolVersionMsg)) return False;

  if (!ReadFromRFBServer((char *)&authScheme, 4)) return False;

  authScheme = Swap32IfLE(authScheme);

  switch (authScheme) {

  case rfbConnFailed:
    if (!ReadFromRFBServer((char *)&reasonLen, 4)) return False;
    reasonLen = Swap32IfLE(reasonLen);

    reason = malloc(reasonLen);

    if (!ReadFromRFBServer(reason, reasonLen)) return False;

    fprintf(stderr,"VNC connection failed: %.*s\n",(int)reasonLen, reason);
    return False;

  case rfbNoAuth:
    fprintf(stderr,"No authentication needed\n");
    break;

  case rfbVncAuth:
    if (!ReadFromRFBServer((char *)challenge, CHALLENGESIZE)) return False;

    if (appData.passwordFile) {
      passwd = vncDecryptPasswdFromFile(appData.passwordFile);
      if (!passwd) {
	fprintf(stderr,"Cannot read valid password from file \"%s\"\n",
		appData.passwordFile);
	return False;
      }
    } else if (appData.passwordDialog) {
      passwd = DoPasswordDialog();
    } else {
      passwd = getpass("Password: ");
    }

    if ((!passwd) || (strlen(passwd) == 0)) {
      fprintf(stderr,"Reading password failed\n");
      return False;
    }
    if (strlen(passwd) > 8) {
      passwd[8] = '\0';
    }

    vncEncryptBytes(challenge, passwd);

	/* Lose the password from memory */
    for (i = strlen(passwd); i >= 0; i--) {
      passwd[i] = '\0';
    }

    if (!WriteExact(rfbsock, (char *)challenge, CHALLENGESIZE)) return False;

    if (!ReadFromRFBServer((char *)&authResult, 4)) return False;

    authResult = Swap32IfLE(authResult);

    switch (authResult) {
    case rfbVncAuthOK:
      fprintf(stderr,"VNC authentication succeeded\n");
      break;
    case rfbVncAuthFailed:
      fprintf(stderr,"VNC authentication failed\n");
      return False;
    case rfbVncAuthTooMany:
      fprintf(stderr,"VNC authentication failed - too many tries\n");
      return False;
    default:
      fprintf(stderr,"Unknown VNC authentication result: %d\n",
	      (int)authResult);
      return False;
    }
    break;

  default:
    fprintf(stderr,"Unknown authentication scheme from VNC server: %d\n",
	    (int)authScheme);
    return False;
  }

  ci.shared = (appData.shareDesktop ? 1 : 0);

  if (!WriteExact(rfbsock, (char *)&ci, sz_rfbClientInitMsg)) return False;

  if (!ReadFromRFBServer((char *)&si, sz_rfbServerInitMsg)) return False;

  si.framebufferWidth = Swap16IfLE(si.framebufferWidth);
  si.framebufferHeight = Swap16IfLE(si.framebufferHeight);
  si.format.redMax = Swap16IfLE(si.format.redMax);
  si.format.greenMax = Swap16IfLE(si.format.greenMax);
  si.format.blueMax = Swap16IfLE(si.format.blueMax);
  si.nameLength = Swap32IfLE(si.nameLength);

  desktopName = malloc(si.nameLength + 1);
  if (!desktopName) {
    fprintf(stderr, "Error allocating memory for desktop name, %lu bytes\n",
            (unsigned long)si.nameLength);
    return False;
  }

  if (!ReadFromRFBServer(desktopName, si.nameLength)) return False;

  desktopName[si.nameLength] = 0;

  fprintf(stderr,"Desktop name \"%s\"\n",desktopName);

  fprintf(stderr,"Connected to VNC server, using protocol version %d.%d\n",
	  rfbProtocolMajorVersion, rfbProtocolMinorVersion);

  fprintf(stderr,"VNC server default format:\n");
  PrintPixelFormat(&si.format);

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

      if (strncasecmp(encStr,"copyrect",encStrLen) == 0) {
	encs[se->nEncodings++] = Swap32IfLE(rfbEncodingCopyRect);
      } else if (strncasecmp(encStr,"tight",encStrLen) == 0) {
	encs[se->nEncodings++] = Swap32IfLE(rfbEncodingTight);
	requestLastRectEncoding = True;
      } else {
	fprintf(stderr,"Unknown encoding '%.*s'\n",encStrLen,encStr);
      }

      encStr = nextEncStr;
    } while (encStr && se->nEncodings < MAX_ENCODINGS);

    if (se->nEncodings < MAX_ENCODINGS) {
      if (appData.compressLevel < 0 || appData.compressLevel > 2)
        appData.compressLevel = 0;
      encs[se->nEncodings++] = Swap32IfLE(appData.compressLevel +
					  rfbJpegSubsamp444);
    }

    if (se->nEncodings < MAX_ENCODINGS) {
      if (appData.qualityLevel < 1 || appData.qualityLevel > 100)
        appData.qualityLevel = 95;
      encs[se->nEncodings++] = Swap32IfLE(appData.qualityLevel +
					  rfbJpegQualityLevel1 - 1);
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
	fprintf(stderr,"Same machine: preferring tight encoding\n");
	encs[se->nEncodings++] = Swap32IfLE(rfbEncodingTight);
      } else {
	fprintf(stderr,"Tunneling active: preferring tight encoding\n");
      }
    }

    encs[se->nEncodings++] = Swap32IfLE(rfbEncodingCopyRect);
    encs[se->nEncodings++] = Swap32IfLE(rfbEncodingTight);
//    encs[se->nEncodings++] = Swap32IfLE(rfbEncodingHextile);
//    encs[se->nEncodings++] = Swap32IfLE(rfbEncodingZlib);
//    encs[se->nEncodings++] = Swap32IfLE(rfbEncodingCoRRE);
//    encs[se->nEncodings++] = Swap32IfLE(rfbEncodingRRE);

    if (appData.compressLevel >= 0 && appData.compressLevel <= 2) {
      encs[se->nEncodings++] = Swap32IfLE(appData.compressLevel +
					  rfbJpegSubsamp444);
    }

      if (appData.qualityLevel < 1 || appData.qualityLevel > 100)
	appData.qualityLevel = 95;
      encs[se->nEncodings++] = Swap32IfLE(appData.qualityLevel +
					  rfbJpegQualityLevel1 - 1);

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
          node = list;

	  rfbFramebufferUpdateRectHeader* r1 = &node->region;

	  if (r1->encoding == rfbEncodingTight) {
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

      case rfbEncodingTight:
      {
	switch (myFormat.bitsPerPixel) {
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
  	  node = list;
	  rfbFramebufferUpdateRectHeader* r1 = &node->region;

	  if (r1->encoding == rfbEncodingTight) {
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

    if (!SendIncrementalFramebufferUpdateRequest())
      return False;

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

#define BPP 16
#include "tight.c"
#undef BPP
#define BPP 32
#include "tight.c"
#undef BPP


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
