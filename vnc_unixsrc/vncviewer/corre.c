/*
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
 * corre.c - handle CoRRE encoding.
 *
 * This file shouldn't be compiled directly.  It is included multiple times by
 * rfbproto.c, each time with a different definition of the macro BPP.  For
 * each value of BPP, this file defines a function which handles a CoRRE
 * encoded rectangle with BPP bits per pixel.
 */

#define HandleCoRREBPP CONCAT2E(HandleCoRRE,BPP)
#define CARDBPP CONCAT2E(CARD,BPP)

static Bool
HandleCoRREBPP (int rx, int ry, int rw, int rh)
{
    rfbRREHeader hdr;
    XGCValues gcv;
    int i;
    CARDBPP pix;
    CARD8 *ptr;
    int x, y, w, h;

    if (!ReadFromRFBServer((char *)&hdr, sz_rfbRREHeader))
	return False;

    hdr.nSubrects = Swap32IfLE(hdr.nSubrects);

    if (!ReadFromRFBServer((char *)&pix, sizeof(pix)))
	return False;

#if (BPP == 8)
    gcv.foreground = (appData.useBGR233 ? BGR233ToPixel[pix] : pix);
#else
    gcv.foreground = pix;
#endif

    XChangeGC(dpy, gc, GCForeground, &gcv);
    XFillRectangle(dpy, desktopWin, gc, rx, ry, rw, rh);

    if (!ReadFromRFBServer(buffer, hdr.nSubrects * (4 + (BPP / 8))))
	return False;

    ptr = (CARD8 *)buffer;

    for (i = 0; i < hdr.nSubrects; i++) {
	pix = *(CARDBPP *)ptr;
	ptr += BPP/8;
	x = *ptr++;
	y = *ptr++;
	w = *ptr++;
	h = *ptr++;

#if (BPP == 8)
	gcv.foreground = (appData.useBGR233 ? BGR233ToPixel[pix] : pix);
#else
	gcv.foreground = pix;
#endif

	XChangeGC(dpy, gc, GCForeground, &gcv);
	XFillRectangle(dpy, desktopWin, gc, rx + x, ry + y, w, h);
    }

    return True;
}
