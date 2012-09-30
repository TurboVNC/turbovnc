/*
 *  Copyright (C) 2012 D. R. Commander.  All Rights Reserved.
 *  Copyright (C) 2009-2011 Pierre Ossman for Cendio AB.  All Rights Reserved.
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

#include <vncviewer.h>


Bool supportsCU = False;
Bool continuousUpdates = False;
Bool supportsFence = False;
Bool supportsSyncFence = False;
Bool pendingSyncFence = False;


Bool HandleFence(CARD32 flags, unsigned len, const char *data)
{
  if (flags & rfbFenceFlagRequest) {
    /* We handle everything synchronously, so we trivially honor these modes */
    flags = flags & (rfbFenceFlagBlockBefore | rfbFenceFlagBlockAfter);
    return SendFence(flags, len, data);
  }

  if (len == 0) {
    /* Initial probe */
    if (flags & rfbFenceFlagSyncNext) {
      supportsSyncFence = True;

      if (supportsCU) {
        continuousUpdates = True;
        return SendEnableContinuousUpdates(True, 0, 0, si.framebufferWidth,
                                           si.framebufferHeight);
      }
    }
  } else {
    /* Pixel format change */
    rfbPixelFormat pf;
    if (!ReadFromRFBServer((char *)&pf, sz_rfbPixelFormat))
      return False;
    fprintf(stderr, "Ignoring pixel format change request from server.\n");
  }

  return True;
}


Bool SendEnableContinuousUpdates(Bool enable, int x, int y, int w, int h)
{
  rfbEnableContinuousUpdatesMsg ecu;

  if (!supportsCU) {
    fprintf(stderr, "Attempted to send Continuous Updates message, but the server does not support\n"
                    "the extension.\n");
    return False;
  }

  ecu.type = rfbEnableContinuousUpdates;
  ecu.enable = enable ? 1 : 0;
  ecu.x = Swap16IfLE(x);
  ecu.y = Swap16IfLE(y);
  ecu.w = Swap16IfLE(w);
  ecu.h = Swap16IfLE(h);

  fprintf(stderr, "%s continuous updates\n",
          enable ? "Enabling" : "Disabling");

  return WriteExact(rfbsock, (char *)&ecu, sz_rfbEnableContinuousUpdatesMsg);
}


Bool SendFence(CARD32 flags, unsigned len, const char *data)
{
  rfbFenceMsg f;

  if (!supportsFence) {
    fprintf(stderr, "Attempted to send Fence message, but the server does not support the extension.\n");
    return False;
  }
  if (len > 64) {
    fprintf(stderr, "Fence payload of %d bytes is too large.\n", len);
    return False;
  }
  if ((flags & ~rfbFenceFlagsSupported) != 0) {
    fprintf(stderr, "Unknown fence flags.\n");
    return False;
  }

  f.type = rfbFence;
  f.flags = Swap32IfLE(flags);
  f.length = len;

  if (!WriteExact(rfbsock, (char *)&f, sz_rfbFenceMsg))
    return False;

  return WriteExact(rfbsock, (char *)data, len);
}
