//  Copyright (C) 2000 Tridia Corporation. All Rights Reserved.
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
// whence you received this file, check http://www.uk.research.att.com/vnc or contact
// the authors on vnc@uk.research.att.com for information on obtaining it.

// ZLIB Encoding
//
// The bits of the ClientConnection object to do with zlib.

#include "stdhdrs.h"
#include "vncviewer.h"
#include "ClientConnection.h"
#include "zlib\zlib.h"

void ClientConnection::ReadZlibRect(rfbFramebufferUpdateRectHeader *pfburh) {

	UINT numpixels = pfburh->r.w * pfburh->r.h;
    // this assumes at least one byte per pixel. Naughty.
	UINT numRawBytes = numpixels * m_minPixelBytes;
	UINT numCompBytes;
	int inflateResult;

	rfbZlibHeader hdr;

	// Read in the rfbZlibHeader
	ReadExact((char *)&hdr, sz_rfbZlibHeader);

	numCompBytes = Swap32IfLE(hdr.nBytes);

	// Read in the compressed data
    CheckBufferSize(numCompBytes);
	ReadExact(m_netbuf, numCompBytes);

	// Verify enough buffer space for screen update.
	CheckZlibBufferSize(numRawBytes);

	m_decompStream.next_in = (unsigned char *)m_netbuf;
	m_decompStream.avail_in = numCompBytes;
	m_decompStream.next_out = m_zlibbuf;
	m_decompStream.avail_out = numRawBytes;
	m_decompStream.data_type = Z_BINARY;
		
	// Insure the inflator is initialized
	if ( m_decompStreamInited == false ) {
		m_decompStream.total_in = 0;
		m_decompStream.total_out = 0;
		m_decompStream.zalloc = Z_NULL;
		m_decompStream.zfree = Z_NULL;
		m_decompStream.opaque = Z_NULL;

		inflateResult = inflateInit( &m_decompStream );
		if ( inflateResult != Z_OK ) {
			vnclog.Print(0, _T("zlib inflateInit error: %d\n"), inflateResult);
			return;
		}
		m_decompStreamInited = true;
	}

	// Decompress screen data
	inflateResult = inflate( &m_decompStream, Z_SYNC_FLUSH );
	if ( inflateResult < 0 ) {
		vnclog.Print(0, _T("zlib inflate error: %d\n"), inflateResult);
		return;
	}

	SETUP_COLOR_SHORTCUTS;

	{
		// No other threads can use bitmap DC
		omni_mutex_lock l(m_bitmapdcMutex);
		ObjectSelector b(m_hBitmapDC, m_hBitmap);							  \
		PaletteSelector p(m_hBitmapDC, m_hPalette);							  \

		// This big switch is untidy but fast
		switch (m_myFormat.bitsPerPixel) {
		case 8:
			SETPIXELS(m_zlibbuf, 8, pfburh->r.x, pfburh->r.y, pfburh->r.w, pfburh->r.h)
				break;
		case 16:
			SETPIXELS(m_zlibbuf, 16, pfburh->r.x, pfburh->r.y, pfburh->r.w, pfburh->r.h)
				break;
		case 24:
		case 32:
			SETPIXELS(m_zlibbuf, 32, pfburh->r.x, pfburh->r.y, pfburh->r.w, pfburh->r.h)            
				break;
		default:
			vnclog.Print(0, _T("Invalid number of bits per pixel: %d\n"), m_myFormat.bitsPerPixel);
			return;
		}
		
	}
}

