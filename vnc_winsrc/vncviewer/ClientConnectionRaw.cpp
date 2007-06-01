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

// Raw Encoding
//
// The bits of the ClientConnection object to do with Raw.

#include "stdhdrs.h"
#include "vncviewer.h"
#include "ClientConnection.h"

void ClientConnection::ReadRawRect(rfbFramebufferUpdateRectHeader *pfburh) {

	UINT numpixels = pfburh->r.w * pfburh->r.h;
    // this assumes at least one byte per pixel. Naughty.
	UINT numbytes = numpixels * m_minPixelBytes;
	// Read in the whole thing
    CheckBufferSize(numbytes);
	ReadExact(m_netbuf, numbytes);

	SETUP_COLOR_SHORTCUTS;

	{
		// No other threads can use bitmap DC
		omni_mutex_lock l(m_bitmapdcMutex);
		ObjectSelector b(m_hBitmapDC, m_hBitmap);							  \
		PaletteSelector p(m_hBitmapDC, m_hPalette);							  \

		if((m_myFormat.bitsPerPixel==24 || m_myFormat.bitsPerPixel==32)
			&& rm==0xFF && gm==0xFF && bm==0xFF && (rs%8)==0 && (gs%8)==0
			&& (bs%8)==0) {

			// Fast path
			int srcpf=-1, srcps=m_myFormat.bitsPerPixel/8;
			if(rs==0 && gs==8 && bs==16) {
				if(srcps==3) srcpf=FBX_RGB;
				else if(srcps==4) srcpf=FBX_RGBA;
			}
			else if(rs==16 && gs==8 && bs==0) {
				if(srcps==3) srcpf=FBX_BGR;
				else if(srcps==4) srcpf=FBX_BGRA;
			}
			else if(rs==8 && gs==16 && bs==24 && srcps==4) srcpf=FBX_ARGB;
			else if(rs==24 && gs==16 && bs==8 && srcps==4) srcpf=FBX_ABGR;
			if(srcpf<0) {
				vnclog.Print(0, _T("Invalid pixel format: rs=%d gs=%d bs=%d rm=%d gm=%d bm=%d\n"),
					rs, gs, bs, rm, gm, bm);
				return;
			}

			int i, j;
			int srcstride=srcps*pfburh->r.w, dststride=fb.pitch;
			char *srcptr=m_netbuf,
				*dstptr=&fb.bits[pfburh->r.y*fb.pitch+pfburh->r.x*fbx_ps[fb.format]];
			char *srcptr2, *dstptr2;
			int srcbgr=fbx_bgr[srcpf], dstbgr=fbx_bgr[fb.format],
				dstps=fbx_ps[fb.format], srcaf=fbx_alphafirst[fb.format],
				dstaf=fbx_alphafirst[fb.format];

			if(srcbgr==dstbgr && srcps==dstps)
			{
				int wps=pfburh->r.w*srcps;
				if(srcaf) srcptr++;  if(dstaf) dstptr++;
				if(srcaf || dstaf)  wps--;
				for(i=0; i<pfburh->r.h; i++, srcptr+=srcstride, dstptr+=dststride)
				{
					memcpy(dstptr, srcptr, wps);
				}
			}
			else
			{
				if(srcaf) srcptr++;  if(dstaf) dstptr++;
				if(srcbgr==dstbgr)
				{
 					for(i=0; i<pfburh->r.h; i++, srcptr+=srcstride, dstptr+=dststride)
					{
						for(j=0, srcptr2=srcptr, dstptr2=dstptr; j<pfburh->r.w; j++,
							srcptr2+=srcps, dstptr2+=fbx_ps[fb.format])
						{
							memcpy(dstptr2, srcptr2, 3);
					 	}
					}
			 	}
				else
				{
					for(i=0; i<pfburh->r.h; i++, srcptr+=srcstride, dstptr+=dststride)
					{
						for(j=0, srcptr2=srcptr, dstptr2=dstptr; j<pfburh->r.w; j++,
							srcptr2+=srcps, dstptr2+=fbx_ps[fb.format])
						{
							dstptr2[2]=srcptr2[0];
							dstptr2[1]=srcptr2[1];
							dstptr2[0]=srcptr2[2];
					 	}
					}
				}
			}
			return;
		}

		// This big switch is untidy but fast
		switch (m_myFormat.bitsPerPixel) {
		case 8:
			SETPIXELS(m_netbuf, 8, pfburh->r.x, pfburh->r.y, pfburh->r.w, pfburh->r.h)
				break;
		case 16:
			SETPIXELS(m_netbuf, 16, pfburh->r.x, pfburh->r.y, pfburh->r.w, pfburh->r.h)
				break;
		case 24:
		case 32:
			SETPIXELS(m_netbuf, 32, pfburh->r.x, pfburh->r.y, pfburh->r.w, pfburh->r.h)            
				break;
		default:
			vnclog.Print(0, _T("Invalid number of bits per pixel: %d\n"), m_myFormat.bitsPerPixel);
			return;
		}
		
	}
}

