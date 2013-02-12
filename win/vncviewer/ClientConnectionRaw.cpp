//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
//
//  This file is part of the VNC system.
//
//  The VNC system is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
//  USA.

#include "stdhdrs.h"
#include "vncviewer.h"
#include "ClientConnection.h"


void ClientConnection::ReadRawRect(rfbFramebufferUpdateRectHeader *pfburh)
{
  UINT numpixels = pfburh->r.w * pfburh->r.h;
  // this assumes at least one byte per pixel.  Naughty.
  UINT numbytes = numpixels * m_minPixelBytes;
  // Read in the whole thing
  CheckBufferSize(numbytes);
  ReadExact(m_netbuf, numbytes);

  SETUP_COLOR_SHORTCUTS;
  InitSetPixels();

  {
    // No other threads can use bitmap DC
    omni_mutex_lock l(m_bitmapdcMutex);
    ObjectSelector b(m_hBitmapDC, m_hBitmap);
    PaletteSelector p(m_hBitmapDC, m_hPalette);

    // This big switch is untidy but fast
    switch (m_myFormat.bitsPerPixel) {
      case 8:
        SETPIXELS(m_netbuf, 8, pfburh->r.x, pfburh->r.y, pfburh->r.w,
                  pfburh->r.h)
        break;
      case 16:
        SETPIXELS(m_netbuf, 16, pfburh->r.x, pfburh->r.y, pfburh->r.w,
                  pfburh->r.h)
        break;
      case 24:
      case 32:
        SETPIXELS(m_netbuf, 32, pfburh->r.x, pfburh->r.y, pfburh->r.w,
                  pfburh->r.h)
        break;
      default:
        vnclog.Print(0, "Invalid number of bits per pixel: %d\n",
                     m_myFormat.bitsPerPixel);
        return;
    }
  }
}
