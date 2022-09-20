/*
 * stats.c
 */

/* Copyright (C) 2014 D. R. Commander.  All Rights Reserved.
 * Copyright (C) 2002 Constantin Kaplinsky.  All Rights Reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include "rfb.h"

static char *encNames[MAX_ENCODINGS] = {
  "Raw", "CopyRect", "RRE", "[Encoding 3]", "CoRRE", "Hextile", "Zlib",
  "Tight", "[Encoding 8]", "[Encoding 9]", "[Encoding 10]", "[Encoding 11]",
  "[Encoding 12]", "[Encoding 13]", "[Encoding 14]", "[Encoding 15]", "ZRLE",
  "ZYWRLE"
};


void rfbResetStats(rfbClientPtr cl)
{
  int i;

  for (i = 0; i < MAX_ENCODINGS; i++) {
    cl->rfbBytesSent[i] = 0;
    cl->rfbRectanglesSent[i] = 0;
  }
  cl->rfbLastRectMarkersSent = 0;
  cl->rfbLastRectBytesSent = 0;
  cl->rfbCursorShapeBytesSent = 0;
  cl->rfbCursorShapeUpdatesSent = 0;
  cl->rfbCursorPosBytesSent = 0;
  cl->rfbCursorPosUpdatesSent = 0;
  cl->rfbFramebufferUpdateMessagesSent = 0;
  cl->rfbRawBytesEquivalent = 0;
  cl->rfbKeyEventsRcvd = 0;
  cl->rfbPointerEventsRcvd = 0;
}


void rfbPrintStats(rfbClientPtr cl)
{
  int i;
  int totalRectanglesSent = 0;
  long long totalBytesSent = 0;

  rfbLog("Statistics:\n");

  if ((cl->rfbKeyEventsRcvd != 0) || (cl->rfbPointerEventsRcvd != 0))
    rfbLog("  key events received %d, pointer events %d\n",
           cl->rfbKeyEventsRcvd, cl->rfbPointerEventsRcvd);

  for (i = 0; i < MAX_ENCODINGS; i++) {
    totalRectanglesSent += cl->rfbRectanglesSent[i];
    totalBytesSent += cl->rfbBytesSent[i];
  }
  totalRectanglesSent += (cl->rfbCursorShapeUpdatesSent +
                          cl->rfbCursorPosUpdatesSent +
                          cl->rfbLastRectMarkersSent);
  totalBytesSent += (cl->rfbCursorShapeBytesSent +
                     cl->rfbCursorPosBytesSent +
                     cl->rfbLastRectBytesSent);

  rfbLog("  framebuffer updates %d, rectangles %d, bytes %d\n",
         cl->rfbFramebufferUpdateMessagesSent, totalRectanglesSent,
         totalBytesSent);

  if (cl->rfbLastRectMarkersSent != 0)
    rfbLog("    LastRect markers %d, bytes %d\n", cl->rfbLastRectMarkersSent,
           cl->rfbLastRectBytesSent);

  if (cl->rfbCursorShapeUpdatesSent != 0)
    rfbLog("    cursor shape updates %d, bytes %d\n",
           cl->rfbCursorShapeUpdatesSent, cl->rfbCursorShapeBytesSent);

  if (cl->rfbCursorPosUpdatesSent != 0)
    rfbLog("    cursor position updates %d, bytes %d\n",
           cl->rfbCursorPosUpdatesSent, cl->rfbCursorPosBytesSent);

  for (i = 0; i < MAX_ENCODINGS; i++) {
    if (cl->rfbRectanglesSent[i] != 0)
      rfbLog("    %s rectangles %d, bytes %d\n", encNames[i],
             cl->rfbRectanglesSent[i], cl->rfbBytesSent[i]);
  }

  if ((totalBytesSent - cl->rfbBytesSent[rfbEncodingCopyRect]) != 0) {
    rfbLog("  raw equivalent %f Mbytes, compression ratio %f\n",
           (double)cl->rfbRawBytesEquivalent / 1000000.,
           (double)cl->rfbRawBytesEquivalent /
           (double)(totalBytesSent - cl->rfbBytesSent[rfbEncodingCopyRect] -
                    cl->rfbCursorShapeBytesSent - cl->rfbCursorPosBytesSent -
                    cl->rfbLastRectBytesSent));
  }
}
