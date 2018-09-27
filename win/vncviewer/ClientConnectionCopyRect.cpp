//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
//  Copyright (C) 2015 D. R. Commander. All Rights Reserved.
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


void ClientConnection::ReadCopyRect(rfbFramebufferUpdateRectHeader *pfburh)
{
  rfbCopyRect cr;
  double tBlitStart;

  ReadExact((char *)&cr, sz_rfbCopyRect);
  cr.srcX = Swap16IfLE(cr.srcX);
  cr.srcY = Swap16IfLE(cr.srcY);

  // If *Cursor encoding is used, we should extend our "cursor lock area"
  // (previously set to destination rectangle) to the source rect as well.
  SoftCursorLockArea(cr.srcX, cr.srcY, pfburh->r.w, pfburh->r.h);

  omni_mutex_lock l(m_bitmapdcMutex);
  ObjectSelector b(m_hBitmapDC, m_hBitmap);
  PaletteSelector p(m_hBitmapDC, m_hPalette);

  if (m_opts.m_benchFile) tBlitStart = getTime();

  if (!BitBlt(m_hBitmapDC, pfburh->r.x, pfburh->r.y, pfburh->r.w, pfburh->r.h,
              m_hBitmapDC, cr.srcX, cr.srcY, SRCCOPY))
    vnclog.Print(0, "Error in blit in ClientConnection::CopyRect\n");

  if (m_opts.m_benchFile) tBlit += getTime() - tBlitStart;
}
