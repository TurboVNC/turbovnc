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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
// TightVNC distribution homepage on the Web: http://www.tightvnc.com/
//
// If the source code for the VNC system is not available from the place 
// whence you received this file, check http://www.uk.research.att.com/vnc or 
// contact the authors on vnc@uk.research.att.com for information on obtaining it.
//
// CopyRect Encoding
//
// The bits of the ClientConnection object to do with CopyRect.

#include "stdhdrs.h"
#include "vncviewer.h"
#include "ClientConnection.h"

void ClientConnection::ReadCopyRect(rfbFramebufferUpdateRectHeader *pfburh) {
	rfbCopyRect cr;
	ReadExact((char *) &cr, sz_rfbCopyRect);
	cr.srcX = Swap16IfLE(cr.srcX); 
	cr.srcY = Swap16IfLE(cr.srcY);

	// If *Cursor encoding is used, we should extend our "cursor lock area"
	// (previously set to destination rectangle) to the source rect as well.
	SoftCursorLockArea(cr.srcX, cr.srcY, pfburh->r.w, pfburh->r.h);

	omni_mutex_lock l(m_bitmapdcMutex);									  
	ObjectSelector b(m_hBitmapDC, m_hBitmap);							  
	PaletteSelector p(m_hBitmapDC, m_hPalette);							  

	if (!BitBlt(
		m_hBitmapDC,pfburh->r.x, pfburh->r.y, pfburh->r.w, pfburh->r.h,
		m_hBitmapDC, cr.srcX, cr.srcY, SRCCOPY)) {
		vnclog.Print(0, _T("Error in blit in ClientConnection::CopyRect\n"));
	}
}
