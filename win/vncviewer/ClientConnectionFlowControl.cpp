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

#include "vncviewer.h"
#include "Exception.h"


void ClientConnection::HandleFence(CARD32 flags, unsigned len,
                                   const char *data)
{
  if (flags & rfbFenceFlagRequest) {
    /* We handle everything synchronously, so we trivially honor these modes */
    flags = flags & (rfbFenceFlagBlockBefore | rfbFenceFlagBlockAfter);
    SendFence(flags, len, data);
    return;
  }

  if (len == 0) {
    /* Initial probe */
    if (flags & rfbFenceFlagSyncNext) {
      supportsSyncFence = true;

      if (supportsCU) {
        continuousUpdates = true;
        SendEnableContinuousUpdates(true, 0, 0, m_si.framebufferWidth,
                                    m_si.framebufferHeight);
      }
    }
  } else {
    /* Pixel format change */
    rfbPixelFormat pf;
    ReadExact((char *)&pf, sz_rfbPixelFormat);
    vnclog.Print(0, "Ignoring pixel format change request from server.\n");
  }
}


void ClientConnection::SendEnableContinuousUpdates(bool enable, int x, int y,
                                                   int w, int h)
{
  rfbEnableContinuousUpdatesMsg ecu;

  if (!supportsCU)
    throw ErrorException("Attempted to send Continuous Updates message, but the server does not support the extension.");

  ecu.type = rfbEnableContinuousUpdates;
  ecu.enable = enable ? 1 : 0;
  ecu.x = Swap16IfLE(x);
  ecu.y = Swap16IfLE(y);
  ecu.w = Swap16IfLE(w);
  ecu.h = Swap16IfLE(h);

  vnclog.Print(0, "%s continuous updates\n",
               enable ? "Enabling" : "Disabling");

  WriteExact((char *)&ecu, sz_rfbEnableContinuousUpdatesMsg);
}


void ClientConnection::SendFence(CARD32 flags, unsigned len, const char *data)
{
  rfbFenceMsg f;

  if (!supportsFence)
    throw new ErrorException("Attempted to send Fence message, but the server does not support the extension.");
  if (len > 64)
    throw new ErrorException("Fence payload is too large.");
  if ((flags & ~rfbFenceFlagsSupported) != 0)
    throw new ErrorException("Unknown fence flags.");

  f.type = rfbFence;
  f.flags = Swap32IfLE(flags);
  f.length = len;

  // We lock the mutex here to prevent other messages, such as pointer events,
  // from being transmitted between the fence header and the data.
  omni_mutex_lock l(m_writeMutex);
  WriteExact((char *)&f, sz_rfbFenceMsg);
  WriteExact((char *)data, len);
}


void ClientConnection::ReadFence(void)
{
  rfbFenceMsg f;
  CARD32 flags;
  char data[64];

  ReadExact((char *)&f, sz_rfbFenceMsg);

  flags = Swap32IfLE(f.flags);

  if (f.length > 0)
    ReadExact(data, f.length);

  supportsFence = true;

  if (f.length > sizeof(data))
    vnclog.Print(0, "Ignoring fence.  Payload of %d bytes is too large.\n",
                 f.length);
  else
    HandleFence(flags, f.length, data);
}
