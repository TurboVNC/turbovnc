//  Copyright (C) 2004 Landmark Graphics Corporation. All Rights Reserved.
//  Copyright (C) 2005-2008 Sun Microsystems, Inc. All Rights Reserved.
//  Copyright (C) 2000 Constantin Kaplinsky. All Rights Reserved.
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


// vncEncodeTight

// This file implements the vncEncoder-derived vncEncodeTight class.
// This class overrides some vncEncoder functions to produce a bitmap
// to Tight encoder. Tight is much more efficient than RAW format on
// most screen data and usually 2..10 times as efficient as hextile.
// It's also more efficient than Zlib encoding in most cases.
// But note that tight compression may use more CPU time on the server.
// However, over slower (128kbps or less) connections, the reduction
// in data transmitted usually outweighs the extra latency added
// while the server CPU performs the compression algorithms.

#include "vncEncodeTight.h"

// Compression level stuff. The following array contains various
// encoder parameters for each of 10 compression levels (0..9).
// Last three parameters correspond to JPEG quality levels (0..9).
//
// NOTE: m_conf[9].maxRectSize should be >= m_conf[i].maxRectSize,
// where i in [0..8]. RequiredBuffSize() method depends on this.

TIGHT_CONF vncEncodeTight::m_conf[2] = {
	{ 65536, 2048,   6, 0, 0, 0,  4 },
#if 0
	{  2048,  128,   6, 1, 1, 1,  8 },
	{  6144,  256,   8, 3, 3, 2, 24 },
	{ 10240, 1024,  12, 5, 5, 3, 32 },
	{ 16384, 2048,  12, 6, 6, 4, 32 },
	{ 32768, 2048,  12, 7, 7, 5, 32 },
	{ 65536, 2048,  16, 7, 7, 6, 48 },
	{ 65536, 2048,  16, 8, 8, 7, 64 },
	{ 65536, 2048,  32, 9, 9, 8, 64 },
#endif
	{ 65536, 2048,  32, 1, 1, 1, 96 }
};

static const int subsampLevel2subsamp[4] = {
    TJ_444, TJ_411, TJ_422, TJ_GRAYSCALE
};

vncEncodeTight::vncEncodeTight()
{
	m_buffer = NULL;
	m_bufflen = 0;

	m_hdrBuffer = new BYTE [sz_rfbFramebufferUpdateRectHeader + 8 + 256*4];
	m_prevRowBuf = NULL;

	for (int i = 0; i < 4; i++)
		m_zsActive[i] = false;

	tjhnd=NULL;
}

vncEncodeTight::~vncEncodeTight()
{
	if (m_buffer != NULL) {
		delete[] m_buffer;
		m_buffer = NULL;
	}

	delete[] m_hdrBuffer;

	for (int i = 0; i < 4; i++) {
		if (m_zsActive[i])
			deflateEnd(&m_zsStruct[i]);
		m_zsActive[i] = false;
	}

	if(tjhnd) tjDestroy(tjhnd);
}

void
vncEncodeTight::Init()
{
	vncEncoder::Init();
}

/*****************************************************************************
 *
 * Routines to implement Tight Encoding.
 *
 */

UINT
vncEncodeTight::RequiredBuffSize(UINT width, UINT height)
{
	// FIXME: Use actual compression level instead of 9?
	int result = m_conf[1].maxRectSize * (m_remoteformat.bitsPerPixel / 8);
	result += result / 100 + 16;
	if(result<(int)TJBUFSIZE(width, height)) result=TJBUFSIZE(width, height);
	return result;
}

UINT
vncEncodeTight::NumCodedRects(RECT &rect)
{
	const int w = rect.right - rect.left;
	const int h = rect.bottom - rect.top;

	// No matter how many rectangles we will send if LastRect markers
	// are used to terminate rectangle stream.
	if (m_use_lastrect && w * h >= MIN_SPLIT_RECT_SIZE) {
		return 0;
	}

	const int maxRectSize = m_conf[compressLevel].maxRectSize;
	const int maxRectWidth = m_conf[compressLevel].maxRectWidth;

	if (w > maxRectWidth || w * h > maxRectSize) {
		const int subrectMaxWidth = (w > maxRectWidth) ? maxRectWidth : w;
		const int subrectMaxHeight = maxRectSize / subrectMaxWidth;
		return (((w - 1) / maxRectWidth + 1) *
				((h - 1) / subrectMaxHeight + 1));
	} else {
		return 1;
	}
}

UINT
vncEncodeTight::EncodeRect(BYTE *source, VSocket *outConn, BYTE *dest,
						   const RECT &rect, int offx, int offy)
{
	int x = rect.left, y = rect.top;
	int w = rect.right - x, h = rect.bottom - y;
	offsetx = offx;
	offsety = offy;

	compressLevel = m_compresslevel > 0 ? 1 : 0;
	if (m_qualitylevel != -1)
	{
		compressLevel = 1;
		m_conf[compressLevel].idxZlibLevel = 1;
		m_conf[compressLevel].monoZlibLevel = 1;
		m_conf[compressLevel].rawZlibLevel = 1;
	}
	else
	{
		m_conf[compressLevel].idxZlibLevel = m_compresslevel;
		m_conf[compressLevel].monoZlibLevel = m_compresslevel;
		m_conf[compressLevel].rawZlibLevel = m_compresslevel;
	}

	const int maxRectSize = m_conf[m_compresslevel].maxRectSize;
	const int rawDataSize = maxRectSize * (m_remoteformat.bitsPerPixel / 8);

	if (m_bufflen < rawDataSize) {
		if (m_buffer != NULL)
			delete [] m_buffer;

		m_buffer = new BYTE [rawDataSize+1];
		if (m_buffer == NULL)
			return vncEncoder::EncodeRect(source, dest, rect, offsetx, offsety);

		m_bufflen = rawDataSize;
	}

	if ( m_remoteformat.depth == 24 && m_remoteformat.redMax == 0xFF &&
		 m_remoteformat.greenMax == 0xFF && m_remoteformat.blueMax == 0xFF ) {
		m_usePixelFormat24 = true;
	} else {
		m_usePixelFormat24 = false;
	}

	if (!m_use_lastrect || w * h < MIN_SPLIT_RECT_SIZE)
		return EncodeRectSimple(source, outConn, dest, rect);

	// Calculate maximum number of rows in one non-solid rectangle.

	int nMaxRows;
	{
		int maxRectSize = m_conf[compressLevel].maxRectSize;
		int maxRectWidth = m_conf[compressLevel].maxRectWidth;
		int nMaxWidth = (w > maxRectWidth) ? maxRectWidth : w;
		nMaxRows = maxRectSize / nMaxWidth;
	}

	// Try to find large solid-color areas and send them separately.

	CARD32 colorValue;
	int x_best, y_best, w_best, h_best;
	int dx, dy, dw, dh;

	for (dy = y; dy < y + h; dy += MAX_SPLIT_TILE_SIZE) {

		// If a rectangle becomes too large, send its upper part now.

		if (dy - y >= nMaxRows) {
			RECT upperRect;
			SetRect(&upperRect, x, y, x + w, y + nMaxRows);

			int size = EncodeRectSimple(source, outConn, dest, upperRect);
			outConn->SendQueued((char *)dest, size);

			y += nMaxRows;
			h -= nMaxRows;
		}

		dh = (dy + MAX_SPLIT_TILE_SIZE <= y + h) ?
			MAX_SPLIT_TILE_SIZE : (y + h - dy);

		for (dx = x; dx < x + w; dx += MAX_SPLIT_TILE_SIZE) {

			dw = (dx + MAX_SPLIT_TILE_SIZE <= x + w) ?
				MAX_SPLIT_TILE_SIZE : (x + w - dx);

			if (CheckSolidTile(source, dx, dy, dw, dh, &colorValue, FALSE)) {

				if(m_subsamplevel == TVNC_GRAY && m_qualitylevel != -1) {
					CARD32 r=(colorValue>>16)&0xFF;
					CARD32 g=(colorValue>>8)&0xFF;
					CARD32 b=(colorValue)&0xFF;
					double y=(0.257*(double)r)+(0.504*(double)g)+(0.098*(double)b)+16.;
					colorValue=(int)y+(((int)y)<<8)+(((int)y)<<16);
				}

				// Get dimensions of solid-color area.

				FindBestSolidArea(source, dx, dy, w - (dx - x), h - (dy - y),
								  colorValue, &w_best, &h_best);

				// Make sure a solid rectangle is large enough
				// (or the whole rectangle is of the same color).

				if ( w_best * h_best != w * h &&
					 w_best * h_best < MIN_SOLID_SUBRECT_SIZE )
					continue;

				// Try to extend solid rectangle to maximum size.

				x_best = dx; y_best = dy;
				ExtendSolidArea(source, x, y, w, h, colorValue,
								&x_best, &y_best, &w_best, &h_best);

				// Compute dimensions of surrounding rectangles.

				RECT rects[4];
				SetRect(&rects[0],
						x, y, x + w, y_best);
				SetRect(&rects[1],
						x, y_best, x_best, y_best + h_best);
				SetRect(&rects[2],
						x_best + w_best, y_best, x + w, y_best + h_best);
				SetRect(&rects[3],
						x, y_best + h_best, x + w, y + h);

				// Send solid-color area and surrounding rectangles.

				for (int i = 0; i < 4; i++) {
					if (i == 2) {
						RECT onePixel;
						SetRect(&onePixel,
								x_best, y_best, x_best + 1, y_best + 1);
						Translate(source, m_buffer, onePixel);

						SendTightHeader(x_best, y_best, w_best, h_best);
						int size = SendSolidRect(dest);

						outConn->SendQueued((char *)m_hdrBuffer, m_hdrBufferBytes);
						outConn->SendQueued((char *)dest, size);
						encodedSize += (m_hdrBufferBytes + size -
										sz_rfbFramebufferUpdateRectHeader);
						transmittedSize += (m_hdrBufferBytes + size);
					}
					if ( rects[i].left == rects[i].right ||
						 rects[i].top  == rects[i].bottom ) {
						continue;
					}
					int size = EncodeRect(source, outConn, dest, rects[i], offsetx, offsety);
					outConn->SendQueued((char *)dest, size);
				}

				// Return after all recursive calls done (0 == data sent).

				return 0;
			}

		}

	}

	// No suitable solid-color rectangles found.

	return EncodeRectSimple(source, outConn, dest, rect);
}

void
vncEncodeTight::FindBestSolidArea(BYTE *source, int x, int y, int w, int h,
								  CARD32 colorValue, int *w_ptr, int *h_ptr)
{
	int dx, dy, dw, dh;
	int w_prev;
	int w_best = 0, h_best = 0;

	w_prev = w;

	for (dy = y; dy < y + h; dy += MAX_SPLIT_TILE_SIZE) {

		dh = (dy + MAX_SPLIT_TILE_SIZE <= y + h) ?
			MAX_SPLIT_TILE_SIZE : (y + h - dy);
		dw = (w_prev > MAX_SPLIT_TILE_SIZE) ?
			MAX_SPLIT_TILE_SIZE : w_prev;

		if (!CheckSolidTile(source, x, dy, dw, dh, &colorValue, TRUE))
			break;

		for (dx = x + dw; dx < x + w_prev;) {
			dw = (dx + MAX_SPLIT_TILE_SIZE <= x + w_prev) ?
				MAX_SPLIT_TILE_SIZE : (x + w_prev - dx);
			if (!CheckSolidTile(source, dx, dy, dw, dh, &colorValue, TRUE))
				break;
			dx += dw;
		}

		w_prev = dx - x;
		if (w_prev * (dy + dh - y) > w_best * h_best) {
			w_best = w_prev;
			h_best = dy + dh - y;
		}
	}

	*w_ptr = w_best;
	*h_ptr = h_best;
}

void vncEncodeTight::ExtendSolidArea(BYTE *source, int x, int y, int w, int h,
									 CARD32 colorValue,
									 int *x_ptr, int *y_ptr,
									 int *w_ptr, int *h_ptr)
{
	int cx, cy;

	// Try to extend the area upwards.
	for ( cy = *y_ptr - 1;
		  cy >= y && CheckSolidTile(source, *x_ptr, cy, *w_ptr, 1,
									&colorValue, TRUE);
		  cy-- );
	*h_ptr += *y_ptr - (cy + 1);
	*y_ptr = cy + 1;

	// ... downwards.
	for ( cy = *y_ptr + *h_ptr;
		  cy < y + h && CheckSolidTile(source, *x_ptr, cy, *w_ptr, 1,
									   &colorValue, TRUE);
		  cy++ );
	*h_ptr += cy - (*y_ptr + *h_ptr);

	// ... to the left.
	for ( cx = *x_ptr - 1;
		  cx >= x && CheckSolidTile(source, cx, *y_ptr, 1, *h_ptr,
									&colorValue, TRUE);
		  cx-- );
	*w_ptr += *x_ptr - (cx + 1);
	*x_ptr = cx + 1;

	// ... to the right.
	for ( cx = *x_ptr + *w_ptr;
		  cx < x + w && CheckSolidTile(source, cx, *y_ptr, 1, *h_ptr,
									   &colorValue, TRUE);
		  cx++ );
	*w_ptr += cx - (*x_ptr + *w_ptr);
}

bool
vncEncodeTight::CheckSolidTile(BYTE *source, int x, int y, int w, int h,
							   CARD32 *colorPtr, bool needSameColor)
{
	switch(m_localformat.bitsPerPixel) {
	case 32:
		return CheckSolidTile32(source, x, y, w, h, colorPtr, needSameColor);
	case 16:
		return CheckSolidTile16(source, x, y, w, h, colorPtr, needSameColor);
	default:
		return CheckSolidTile8(source, x, y, w, h, colorPtr, needSameColor);
	}
}

#define DEFINE_CHECK_SOLID_FUNCTION(bpp)									  \
																			  \
bool 																		  \
vncEncodeTight::CheckSolidTile##bpp(BYTE *source, int x, int y, int w, int h, \
									CARD32 *colorPtr, bool needSameColor)	  \
{																			  \
	CARD##bpp *fbptr;														  \
	CARD##bpp colorValue;													  \
	int dx, dy; 															  \
																			  \
	fbptr = (CARD##bpp *)													  \
		&source[y * m_bytesPerRow + x * (bpp/8)];							  \
																			  \
	colorValue = *fbptr;													  \
	if (needSameColor && (CARD32)colorValue != *colorPtr)					  \
		return false;														  \
																			  \
	for (dy = 0; dy < h; dy++) {											  \
		for (dx = 0; dx < w; dx++) {										  \
			if (colorValue != fbptr[dx])									  \
				return false;												  \
		}																	  \
		fbptr = (CARD##bpp *)((BYTE *)fbptr + m_bytesPerRow);				  \
	}																		  \
																			  \
	*colorPtr = (CARD32)colorValue; 										  \
	return true;															  \
}

DEFINE_CHECK_SOLID_FUNCTION(8)
DEFINE_CHECK_SOLID_FUNCTION(16)
DEFINE_CHECK_SOLID_FUNCTION(32)

UINT
vncEncodeTight::EncodeRectSimple(BYTE *source, VSocket *outConn, BYTE *dest,
								 const RECT &rect)
{
	const int x = rect.left, y = rect.top;
	const int w = rect.right - x, h = rect.bottom - y;

	const int maxRectSize = m_conf[compressLevel].maxRectSize;
	const int maxRectWidth = m_conf[compressLevel].maxRectWidth;

	int partialSize = 0;

	if (w > maxRectWidth || w * h > maxRectSize) {
		const int subrectMaxWidth = (w > maxRectWidth) ? maxRectWidth : w;
		const int subrectMaxHeight = maxRectSize / subrectMaxWidth;
		int dx, dy, rw, rh;

		for (dy = 0; dy < h; dy += subrectMaxHeight) {
			for (dx = 0; dx < w; dx += maxRectWidth) {
				rw = (dx + maxRectWidth < w) ? maxRectWidth : w - dx;
				rh = (dy + subrectMaxHeight < h) ? subrectMaxHeight : h - dy;

				partialSize = EncodeSubrect(source, outConn, dest,
											x+dx, y+dy, rw, rh);
				if (dy + subrectMaxHeight < h || dx + maxRectWidth < w) {
					outConn->SendQueued((char *)dest, partialSize);
				}
			}
		}
	} else {
		partialSize = EncodeSubrect(source, outConn, dest, x, y, w, h);
	}

	return partialSize;
}

UINT
vncEncodeTight::EncodeSubrect(BYTE *source, VSocket *outConn, BYTE *dest,
							  int x, int y, int w, int h)
{
	SendTightHeader(x, y, w, h);

	RECT r;
	r.left = x; r.top = y;
	r.right = x + w; r.bottom = y + h;

	int encDataSize;
	if (m_subsamplevel == TVNC_GRAY && m_qualitylevel != -1)
		encDataSize = SendJpegRect(source, dest, x, y, w, h, m_qualitylevel);
	else {

	m_paletteMaxColors = w * h / m_conf[compressLevel].idxMaxColorsDivisor;
	if(m_qualitylevel != -1)
		m_paletteMaxColors = 24;
	if ( m_paletteMaxColors < 2 &&
		 w * h >= m_conf[compressLevel].monoMinRectSize ) {
		m_paletteMaxColors = 2;
	}

	if (m_remoteformat.bitsPerPixel == m_localformat.bitsPerPixel &&
		m_remoteformat.redMax == m_localformat.redMax &&
		m_remoteformat.greenMax == m_localformat.greenMax && 
		m_remoteformat.blueMax == m_localformat.blueMax &&
		m_remoteformat.bitsPerPixel >= 16)
	{
		/* This is so we can avoid translating the pixels when compressing
		   with JPEG, since it is unnecessary */

		BYTE *fbptr = (source + (m_bytesPerRow * y)
		              + (x * (m_localformat.bitsPerPixel / 8)));

		switch (m_remoteformat.bitsPerPixel)
		{
			case 16:
				FastFillPalette16((CARD16 *)fbptr, w, m_bytesPerRow/2, h);
				break;
			default:
				FastFillPalette32((CARD32 *)fbptr, w, m_bytesPerRow/4, h);
		}

		if(m_paletteNumColors != 0 || m_qualitylevel == -1)
			Translate(source, m_buffer, r);
	}
	else
	{
		Translate(source, m_buffer, r);

		switch (m_remoteformat.bitsPerPixel)
		{
			case 8:
				FillPalette8(w * h);
				break;
			case 16:
				FillPalette16(w * h);
				break;
			default:
				FillPalette32(w * h);
		}
	}

	switch (m_paletteNumColors) {
	case 0:
		// Truecolor image
		if (m_qualitylevel != -1)
			encDataSize = SendJpegRect(source, dest, x, y, w, h, m_qualitylevel);
		else
			encDataSize = SendFullColorRect(dest, w, h);
		break;
	case 1:
		// Solid rectangle
		encDataSize = SendSolidRect(dest);
		break;
	case 2:
		// Two-color rectangle
		encDataSize = SendMonoRect(dest, w, h);
		break;
	default:
		// Up to 256 different colors
		encDataSize = SendIndexedRect(dest, w, h);
	}
	} // if(m_subsamplevel == TVNC_GRAY && m_qualitylevel != -1)

	if (encDataSize < 0)
		return vncEncoder::EncodeRect(source, dest, r, 0, 0);

	outConn->SendQueued((char *)m_hdrBuffer, m_hdrBufferBytes);

	encodedSize += m_hdrBufferBytes - sz_rfbFramebufferUpdateRectHeader + encDataSize;
	transmittedSize += m_hdrBufferBytes + encDataSize;

	return encDataSize;
}

void
vncEncodeTight::SendTightHeader(int x, int y, int w, int h)
{
	rfbFramebufferUpdateRectHeader rect;

	rect.r.x = Swap16IfLE(x - offsetx);
	rect.r.y = Swap16IfLE(y - offsety);
	rect.r.w = Swap16IfLE(w);
	rect.r.h = Swap16IfLE(h);
	rect.encoding = Swap32IfLE(rfbEncodingTight);

	dataSize += w * h * (m_remoteformat.bitsPerPixel / 8);
	rectangleOverhead += sz_rfbFramebufferUpdateRectHeader;

	memcpy(m_hdrBuffer, (BYTE *)&rect, sz_rfbFramebufferUpdateRectHeader);
	m_hdrBufferBytes = sz_rfbFramebufferUpdateRectHeader;
}

//
// Subencoding implementations.
//

int
vncEncodeTight::SendSolidRect(BYTE *dest)
{
	int len;

	if (m_usePixelFormat24) {
		Pack24(m_buffer, 1);
		len = 3;
	} else
		len = m_remoteformat.bitsPerPixel / 8;

	m_hdrBuffer[m_hdrBufferBytes++] = rfbTightFill << 4;
	memcpy (dest, m_buffer, len);

	return len;
}

int
vncEncodeTight::SendMonoRect(BYTE *dest, int w, int h)
{
	const int streamId = 1;
	int paletteLen, dataLen;
	CARD8 paletteBuf[8];

	// Prepare tight encoding header.
	dataLen = (w + 7) / 8;
	dataLen *= h;

	if (m_conf[compressLevel].monoZlibLevel == 0)
		m_hdrBuffer[m_hdrBufferBytes++] = (char)((rfbTightNoZlib | rfbTightExplicitFilter) << 4);
	else
		m_hdrBuffer[m_hdrBufferBytes++] = (streamId | rfbTightExplicitFilter) << 4;
	m_hdrBuffer[m_hdrBufferBytes++] = rfbTightFilterPalette;
	m_hdrBuffer[m_hdrBufferBytes++] = 1;

	// Prepare palette, convert image.
	switch (m_remoteformat.bitsPerPixel) {
	case 32:
		EncodeMonoRect32((CARD8 *)m_buffer, w, h);

		((CARD32 *)paletteBuf)[0] = m_monoBackground;
		((CARD32 *)paletteBuf)[1] = m_monoForeground;

		if (m_usePixelFormat24) {
			Pack24(paletteBuf, 2);
			paletteLen = 6;
		} else
			paletteLen = 8;

		memcpy(&m_hdrBuffer[m_hdrBufferBytes], paletteBuf, paletteLen);
		m_hdrBufferBytes += paletteLen;
		break;

	case 16:
		EncodeMonoRect16((CARD8 *)m_buffer, w, h);

		((CARD16 *)paletteBuf)[0] = (CARD16)m_monoBackground;
		((CARD16 *)paletteBuf)[1] = (CARD16)m_monoForeground;

		memcpy(&m_hdrBuffer[m_hdrBufferBytes], paletteBuf, 4);
		m_hdrBufferBytes += 4;
		break;

	default:
		EncodeMonoRect8((CARD8 *)m_buffer, w, h);

		m_hdrBuffer[m_hdrBufferBytes++] = (BYTE)m_monoBackground;
		m_hdrBuffer[m_hdrBufferBytes++] = (BYTE)m_monoForeground;
	}

	return CompressData(dest, streamId, dataLen,
						m_conf[compressLevel].monoZlibLevel,
						Z_DEFAULT_STRATEGY);
}

int
vncEncodeTight::SendIndexedRect(BYTE *dest, int w, int h)
{
	const int streamId = 2;
	int i, entryLen;
	CARD8 paletteBuf[256*4];

	// Prepare tight encoding header.
	if (m_conf[compressLevel].idxZlibLevel == 0)
		m_hdrBuffer[m_hdrBufferBytes++] = (char)((rfbTightNoZlib | rfbTightExplicitFilter) << 4);
	else
		m_hdrBuffer[m_hdrBufferBytes++] = (streamId | rfbTightExplicitFilter) << 4;
	m_hdrBuffer[m_hdrBufferBytes++] = rfbTightFilterPalette;
	m_hdrBuffer[m_hdrBufferBytes++] = (BYTE)(m_paletteNumColors - 1);

	// Prepare palette, convert image.
	switch (m_remoteformat.bitsPerPixel) {
	case 32:
		EncodeIndexedRect32((CARD8 *)m_buffer, w * h);

		for (i = 0; i < m_paletteNumColors; i++) {
			((CARD32 *)paletteBuf)[i] =
				m_palette.entry[i].listNode->rgb;
		}
		if (m_usePixelFormat24) {
			Pack24(paletteBuf, m_paletteNumColors);
			entryLen = 3;
		} else
			entryLen = 4;

		memcpy(&m_hdrBuffer[m_hdrBufferBytes], paletteBuf,
			   m_paletteNumColors * entryLen);
		m_hdrBufferBytes += m_paletteNumColors * entryLen;
		break;

	case 16:
		EncodeIndexedRect16((CARD8 *)m_buffer, w * h);

		for (i = 0; i < m_paletteNumColors; i++) {
			((CARD16 *)paletteBuf)[i] =
				(CARD16)m_palette.entry[i].listNode->rgb;
		}

		memcpy(&m_hdrBuffer[m_hdrBufferBytes], paletteBuf,
			   m_paletteNumColors * 2);
		m_hdrBufferBytes += m_paletteNumColors * 2;
		break;

	default:
		return -1;				// Should never happen.
	}

	return CompressData(dest, streamId, w * h,
						m_conf[compressLevel].idxZlibLevel,
						Z_DEFAULT_STRATEGY);
}

int
vncEncodeTight::SendFullColorRect(BYTE *dest, int w, int h)
{
	const int streamId = 0;
	int len;

	if (m_conf[compressLevel].rawZlibLevel == 0)
		m_hdrBuffer[m_hdrBufferBytes++] = (char)(rfbTightNoZlib << 4);
	else
		m_hdrBuffer[m_hdrBufferBytes++] = 0x00;  /* stream id = 0, no flushing, no filter */

	if (m_usePixelFormat24) {
		Pack24(m_buffer, w * h);
		len = 3;
	} else
		len = m_remoteformat.bitsPerPixel / 8;

	return CompressData(dest, streamId, w * h * len,
						m_conf[compressLevel].rawZlibLevel,
						Z_DEFAULT_STRATEGY);
}

int
vncEncodeTight::CompressData(BYTE *dest, int streamId, int dataLen,
							 int zlibLevel, int zlibStrategy)
{
	if (dataLen < TIGHT_MIN_TO_COMPRESS) {
		memcpy(dest, m_buffer, dataLen);
		return dataLen;
	}

	if (zlibLevel == 0) {
		memcpy(dest, m_buffer, dataLen);
		return SendCompressedData(dataLen);
	}

	z_streamp pz = &m_zsStruct[streamId];

	// Initialize compression stream if needed.
	if (!m_zsActive[streamId]) {
		pz->zalloc = Z_NULL;
		pz->zfree = Z_NULL;
		pz->opaque = Z_NULL;

		vnclog.Print(LL_INTINFO,
					 VNCLOG("calling deflateInit2 with zlib level:%d\n"),
					 zlibLevel);
		int err = deflateInit2 (pz, zlibLevel, Z_DEFLATED, MAX_WBITS,
								MAX_MEM_LEVEL, zlibStrategy);
		if (err != Z_OK) {
			vnclog.Print(LL_INTINFO,
						 VNCLOG("deflateInit2 returned error:%d:%s\n"),
						 err, pz->msg);
			return -1;
		}

		m_zsActive[streamId] = true;
		m_zsLevel[streamId] = zlibLevel;
	}

	int outBufferSize = dataLen + dataLen / 100 + 16;

	// Prepare buffer pointers.
	pz->next_in = (Bytef *)m_buffer;
	pz->avail_in = dataLen;
	pz->next_out = (Bytef *)dest;
	pz->avail_out = outBufferSize;

	// Change compression parameters if needed.
	if (zlibLevel != m_zsLevel[streamId]) {
		vnclog.Print(LL_INTINFO,
					 VNCLOG("calling deflateParams with zlib level:%d\n"),
					 zlibLevel);
		int err = deflateParams (pz, zlibLevel, zlibStrategy);
		if (err != Z_OK) {
			vnclog.Print(LL_INTINFO,
						 VNCLOG("deflateParams returned error:%d:%s\n"),
						 err, pz->msg);
			return -1;
		}
		m_zsLevel[streamId] = zlibLevel;
	}

	// Actual compression.
	if ( deflate (pz, Z_SYNC_FLUSH) != Z_OK ||
		 pz->avail_in != 0 || pz->avail_out == 0 ) {
		vnclog.Print(LL_INTINFO, VNCLOG("deflate() call failed.\n"));
		return -1;
	}

	return SendCompressedData(outBufferSize - pz->avail_out);
}

int
vncEncodeTight::SendCompressedData(int compressedLen)
{
	// Prepare compressed data size for sending.
	m_hdrBuffer[m_hdrBufferBytes++] = compressedLen & 0x7F;
	if (compressedLen > 0x7F) {
		m_hdrBuffer[m_hdrBufferBytes-1] |= 0x80;
		m_hdrBuffer[m_hdrBufferBytes++] = compressedLen >> 7 & 0x7F;
		if (compressedLen > 0x3FFF) {
			m_hdrBuffer[m_hdrBufferBytes-1] |= 0x80;
			m_hdrBuffer[m_hdrBufferBytes++] = compressedLen >> 14 & 0xFF;
		}
	}
	return compressedLen;
}

void
vncEncodeTight::FillPalette8(int count)
{
	CARD8 *data = (CARD8 *)m_buffer;
	CARD8 c0, c1;
	int i, n0, n1;

	m_paletteNumColors = 0;

	c0 = data[0];
	for (i = 1; i < count && data[i] == c0; i++);
	if (i == count) {
		m_paletteNumColors = 1;
		return; 				// Solid rectangle
	}

	if (m_paletteMaxColors < 2)
		return;

	n0 = i;
	c1 = data[i];
	n1 = 0;
	for (i++; i < count; i++) {
		if (data[i] == c0) {
			n0++;
		} else if (data[i] == c1) {
			n1++;
		} else
			break;
	}
	if (i == count) {
		if (n0 > n1) {
			m_monoBackground = (CARD32)c0;
			m_monoForeground = (CARD32)c1;
		} else {
			m_monoBackground = (CARD32)c1;
			m_monoForeground = (CARD32)c0;
		}
		m_paletteNumColors = 2;   // Two colors
	}
}

#define DEFINE_FILL_PALETTE_FUNCTION(bpp)									  \
																			  \
void																		  \
vncEncodeTight::FillPalette##bpp(int count) 								  \
{																			  \
	CARD##bpp *data = (CARD##bpp *)m_buffer;								  \
	CARD##bpp c0, c1, ci;													  \
	int i, n0, n1, ni;														  \
																			  \
	c0 = data[0];															  \
	for (i = 1; i < count && data[i] == c0; i++);							  \
	if (i >= count) {														  \
		m_paletteNumColors = 1; /* Solid rectangle */						  \
		return; 															  \
	}																		  \
																			  \
	if (m_paletteMaxColors < 2) {											  \
		m_paletteNumColors = 0; /* Full-color format preferred */			  \
		return; 															  \
	}																		  \
																			  \
	n0 = i; 																  \
	c1 = data[i];															  \
	n1 = 0; 																  \
	for (i++; i < count; i++) { 											  \
		ci = data[i];														  \
		if (ci == c0) { 													  \
			n0++;															  \
		} else if (ci == c1) {												  \
			n1++;															  \
		} else																  \
			break;															  \
	}																		  \
	if (i >= count) {														  \
		if (n0 > n1) {														  \
			m_monoBackground = (CARD32)c0;									  \
			m_monoForeground = (CARD32)c1;									  \
		} else {															  \
			m_monoBackground = (CARD32)c1;									  \
			m_monoForeground = (CARD32)c0;									  \
		}																	  \
		m_paletteNumColors = 2; /* Two colors */							  \
		return; 															  \
	}																		  \
																			  \
	PaletteReset(); 														  \
	PaletteInsert (c0, (CARD32)n0, bpp);									  \
	PaletteInsert (c1, (CARD32)n1, bpp);									  \
																			  \
	ni = 1; 																  \
	for (i++; i < count; i++) { 											  \
		if (data[i] == ci) {												  \
			ni++;															  \
		} else {															  \
			if (!PaletteInsert (ci, (CARD32)ni, bpp))						  \
				return; 													  \
			ci = data[i];													  \
			ni = 1; 														  \
		}																	  \
	}																		  \
	PaletteInsert (ci, (CARD32)ni, bpp);									  \
}

DEFINE_FILL_PALETTE_FUNCTION(16)
DEFINE_FILL_PALETTE_FUNCTION(32)


#define DEFINE_FAST_FILL_PALETTE_FUNCTION(bpp)                          \
                                                                        \
void                                                                    \
vncEncodeTight::FastFillPalette##bpp                                    \
  (CARD##bpp *data, int w, int pitch, int h)                            \
{                                                                       \
    CARD##bpp c0, c1, ci, mask, c0t, c1t, cit;                          \
    int i, j, i2, j2, n0, n1, ni;                                       \
                                                                        \
    if (m_transfunc != rfbTranslateNone) {                              \
        mask = m_localformat.redMax << m_localformat.redShift;          \
        mask |= m_localformat.greenMax << m_localformat.greenShift;     \
        mask |= m_localformat.blueMax << m_localformat.blueShift;       \
    } else mask = ~0;                                                   \
                                                                        \
    c0 = data[0] & mask;                                                \
    for (j = 0; j < h; j++) {                                           \
        for (i = 0; i < w; i++) {                                       \
            if ((data[j * pitch + i] & mask) != c0)                     \
                goto done;                                              \
        }                                                               \
    }                                                                   \
    done:                                                               \
    if (j >= h) {                                                       \
        m_paletteNumColors = 1;   /* Solid rectangle */                 \
        return;                                                         \
    }                                                                   \
    if (m_paletteMaxColors < 2) {                                       \
        m_paletteNumColors = 0;   /* Full-color encoding preferred */   \
        return;                                                         \
    }                                                                   \
                                                                        \
    n0 = j * w + i;                                                     \
    c1 = data[j * pitch + i] & mask;                                    \
    n1 = 0;                                                             \
    i++;  if (i >= w) {i = 0;  j++;}                                    \
    for (j2 = j; j2 < h; j2++) {                                        \
        for (i2 = i; i2 < w; i2++) {                                    \
            ci = data[j2 * pitch + i2] & mask;                          \
            if (ci == c0) {                                             \
                n0++;                                                   \
            } else if (ci == c1) {                                      \
                n1++;                                                   \
            } else                                                      \
                goto done2;                                             \
        }                                                               \
        i = 0;                                                          \
    }                                                                   \
    done2:                                                              \
    RECT rect1 = {0, 0, 1, 1};                                          \
    Translate((BYTE *)&c0, (BYTE *)&c0t, rect1);                        \
    Translate((BYTE *)&c1, (BYTE *)&c1t, rect1);                        \
    if (j2 >= h) {                                                      \
        if (n0 > n1) {                                                  \
            m_monoBackground = (CARD32)c0t;                             \
            m_monoForeground = (CARD32)c1t;                             \
        } else {                                                        \
            m_monoBackground = (CARD32)c1t;                             \
            m_monoForeground = (CARD32)c0t;                             \
        }                                                               \
        m_paletteNumColors = 2;   /* Two colors */                      \
        return;                                                         \
    }                                                                   \
                                                                        \
    PaletteReset();                                                     \
    PaletteInsert (c0t, (CARD32)n0, bpp);                               \
    PaletteInsert (c1t, (CARD32)n1, bpp);                               \
                                                                        \
    ni = 1;                                                             \
    i2++;  if (i2 >= w) {i2 = 0;  j2++;}                                \
    for (j = j2; j < h; j++) {                                          \
        for (i = i2; i < w; i++) {                                      \
            if ((data[j * pitch + i] & mask) == ci) {                   \
                ni++;                                                   \
            } else {                                                    \
                Translate((BYTE *)&ci, (BYTE *)&cit, rect1);            \
                if (!PaletteInsert (cit, (CARD32)ni, bpp))              \
                    return;                                             \
                ci = data[j * pitch + i] & mask;                        \
                ni = 1;                                                 \
            }                                                           \
        }                                                               \
        i2 = 0;                                                         \
    }                                                                   \
                                                                        \
    Translate((BYTE *)&ci, (BYTE *)&cit, rect1);                        \
    PaletteInsert (cit, (CARD32)ni, bpp);                               \
}

DEFINE_FAST_FILL_PALETTE_FUNCTION(16)
DEFINE_FAST_FILL_PALETTE_FUNCTION(32)


//
// Functions to operate with palette structures.
//

#define HASH_FUNC16(rgb) ((int)(((rgb >> 8) + rgb) & 0xFF))
#define HASH_FUNC32(rgb) ((int)(((rgb >> 16) + (rgb >> 8)) & 0xFF))

void
vncEncodeTight::PaletteReset(void)
{
	m_paletteNumColors = 0;
	memset(m_palette.hash, 0, 256 * sizeof(COLOR_LIST *));
}

int
vncEncodeTight::PaletteInsert(CARD32 rgb, int numPixels, int bpp)
{
	COLOR_LIST *pnode;
	COLOR_LIST *prev_pnode = NULL;
	int hash_key, idx, new_idx, count;

	hash_key = (bpp == 16) ? HASH_FUNC16(rgb) : HASH_FUNC32(rgb);

	pnode = m_palette.hash[hash_key];

	while (pnode != NULL) {
		if (pnode->rgb == rgb) {
			// Such palette entry already exists.
			new_idx = idx = pnode->idx;
			count = m_palette.entry[idx].numPixels + numPixels;
			if (new_idx && m_palette.entry[new_idx-1].numPixels < count) {
				do {
					m_palette.entry[new_idx] = m_palette.entry[new_idx-1];
					m_palette.entry[new_idx].listNode->idx = new_idx;
					new_idx--;
				}
				while (new_idx &&
					   m_palette.entry[new_idx-1].numPixels < count);
				m_palette.entry[new_idx].listNode = pnode;
				pnode->idx = new_idx;
			}
			m_palette.entry[new_idx].numPixels = count;
			return m_paletteNumColors;
		}
		prev_pnode = pnode;
		pnode = pnode->next;
	}

	// Check if palette is full.
	if ( m_paletteNumColors == 256 ||
		 m_paletteNumColors == m_paletteMaxColors ) {
		m_paletteNumColors = 0;
		return 0;
	}

	// Move palette entries with lesser pixel counts.
	for ( idx = m_paletteNumColors;
		  idx > 0 && m_palette.entry[idx-1].numPixels < numPixels;
		  idx-- ) {
		m_palette.entry[idx] = m_palette.entry[idx-1];
		m_palette.entry[idx].listNode->idx = idx;
	}

	// Add new palette entry into the freed slot.
	pnode = &m_palette.list[m_paletteNumColors];
	if (prev_pnode != NULL) {
		prev_pnode->next = pnode;
	} else {
		m_palette.hash[hash_key] = pnode;
	}
	pnode->next = NULL;
	pnode->idx = idx;
	pnode->rgb = rgb;
	m_palette.entry[idx].listNode = pnode;
	m_palette.entry[idx].numPixels = numPixels;

	return (++m_paletteNumColors);
}


//
// Converting 32-bit color samples into 24-bit colors.
// Should be called only when redMax, greenMax and blueMax are 255.
// Color components assumed to be byte-aligned.
//

void
vncEncodeTight::Pack24(BYTE *buf, int count)
{
	CARD32 *buf32;
	CARD32 pix;
	int r_shift, g_shift, b_shift;

	buf32 = (CARD32 *)buf;

	if (!m_localformat.bigEndian == !m_remoteformat.bigEndian) {
		r_shift = m_remoteformat.redShift;
		g_shift = m_remoteformat.greenShift;
		b_shift = m_remoteformat.blueShift;
	} else {
		r_shift = 24 - m_remoteformat.redShift;
		g_shift = 24 - m_remoteformat.greenShift;
		b_shift = 24 - m_remoteformat.blueShift;
	}

	while (count--) {
		pix = *buf32++;
		*buf++ = (char)(pix >> r_shift);
		*buf++ = (char)(pix >> g_shift);
		*buf++ = (char)(pix >> b_shift);
	}
}


//
// Converting truecolor samples into palette indices.
//

#define DEFINE_IDX_ENCODE_FUNCTION(bpp) 									  \
																			  \
void																		  \
vncEncodeTight::EncodeIndexedRect##bpp(BYTE *buf, int count)				  \
{																			  \
	COLOR_LIST *pnode;														  \
	CARD##bpp *src; 														  \
	CARD##bpp rgb;															  \
	int rep = 0;															  \
																			  \
	src = (CARD##bpp *) buf;												  \
																			  \
	while (count--) {														  \
		rgb = *src++;														  \
		while (count && *src == rgb) {										  \
			rep++, src++, count--;											  \
		}																	  \
		pnode = m_palette.hash[HASH_FUNC##bpp(rgb)];						  \
		while (pnode != NULL) { 											  \
			if ((CARD##bpp)pnode->rgb == rgb) { 							  \
				*buf++ = (CARD8)pnode->idx; 								  \
				while (rep) {												  \
					*buf++ = (CARD8)pnode->idx; 							  \
					rep--;													  \
				}															  \
				break;														  \
			}																  \
			pnode = pnode->next;											  \
		}																	  \
	}																		  \
}

DEFINE_IDX_ENCODE_FUNCTION(16)
DEFINE_IDX_ENCODE_FUNCTION(32)

#define DEFINE_MONO_ENCODE_FUNCTION(bpp)									  \
																			  \
void																		  \
vncEncodeTight::EncodeMonoRect##bpp(BYTE *buf, int w, int h)				  \
{																			  \
	CARD##bpp *ptr; 														  \
	CARD##bpp bg;															  \
	unsigned int value, mask;												  \
	int aligned_width;														  \
	int x, y, bg_bits;														  \
																			  \
	ptr = (CARD##bpp *) buf;												  \
	bg = (CARD##bpp) m_monoBackground;										  \
	aligned_width = w - w % 8;												  \
																			  \
	for (y = 0; y < h; y++) {												  \
		for (x = 0; x < aligned_width; x += 8) {							  \
			for (bg_bits = 0; bg_bits < 8; bg_bits++) { 					  \
				if (*ptr++ != bg)											  \
					break;													  \
			}																  \
			if (bg_bits == 8) { 											  \
				*buf++ = 0; 												  \
				continue;													  \
			}																  \
			mask = 0x80 >> bg_bits; 										  \
			value = mask;													  \
			for (bg_bits++; bg_bits < 8; bg_bits++) {						  \
				mask >>= 1; 												  \
				if (*ptr++ != bg) { 										  \
					value |= mask;											  \
				}															  \
			}																  \
			*buf++ = (CARD8)value;											  \
		}																	  \
																			  \
		mask = 0x80;														  \
		value = 0;															  \
		if (x >= w) 														  \
			continue;														  \
																			  \
		for (; x < w; x++) {												  \
			if (*ptr++ != bg) { 											  \
				value |= mask;												  \
			}																  \
			mask >>= 1; 													  \
		}																	  \
		*buf++ = (CARD8)value;												  \
	}																		  \
}

DEFINE_MONO_ENCODE_FUNCTION(8)
DEFINE_MONO_ENCODE_FUNCTION(16)
DEFINE_MONO_ENCODE_FUNCTION(32)


//
// JPEG compression stuff.
//

int
vncEncodeTight::SendJpegRect(BYTE *src, BYTE *dst, int x, int y, int w, int h, int quality)
{
	BYTE *srcbuf;
	int ps=m_localformat.bitsPerPixel/8;
	unsigned long jpegDstDataLen;
	int i, flags=0, srcbufalloc=0, pitch=m_bytesPerRow;

	if (ps < 2) return SendFullColorRect(dst, w, h);

	if(!tjhnd) {
		if((tjhnd=tjInitCompress())==NULL) {
			vnclog.Print(LL_INTERR, VNCLOG("JPEG Error: %s\n"), tjGetErrorStr());
			return 0;
		}
	}
	if(ps >= 3)
	{
		srcbuf=&src[y*m_bytesPerRow + x*ps];
		flags=m_localformat.bigEndian?0:TJ_BGR;
	}
	else
	{
		srcbuf=new byte[w * h * 3];
		srcbufalloc=1;
		for(i = 0; i < h; i++)
			PrepareRowForJpeg16(&srcbuf[i*w*3],
				(CARD16 *)&src[(y+i)*m_bytesPerRow + x*ps], w);
		ps=3;  pitch=w*ps;
	}
	if(tjCompress(tjhnd, (unsigned char *)srcbuf, w, pitch,
		h, ps, (unsigned char *)dst, &jpegDstDataLen,
		subsampLevel2subsamp[m_subsamplevel], quality, flags)==-1) {
		vnclog.Print(LL_INTERR, VNCLOG("JPEG Error: %s\n"), tjGetErrorStr());
		return 0;
	}


	m_hdrBuffer[m_hdrBufferBytes++] = rfbTightJpeg << 4;

	if(srcbufalloc) delete [] srcbuf;

	return SendCompressedData(jpegDstDataLen);
}

void
vncEncodeTight::PrepareRowForJpeg16(BYTE *dst, CARD16 *src, int count)
{
	bool endianMismatch =
		(!m_localformat.bigEndian != !m_remoteformat.bigEndian);

	int r_shift = m_localformat.redShift;
	int g_shift = m_localformat.greenShift;
	int b_shift = m_localformat.blueShift;
	int r_max = m_localformat.redMax;
	int g_max = m_localformat.greenMax;
	int b_max = m_localformat.blueMax;

	CARD16 pix;
	while (count--) {
		pix = *src++;
		if (endianMismatch) {
			pix = Swap16(pix);
		}
		*dst++ = (BYTE)((pix >> r_shift & r_max) * 255 / r_max);
		*dst++ = (BYTE)((pix >> g_shift & g_max) * 255 / g_max);
		*dst++ = (BYTE)((pix >> b_shift & b_max) * 255 / b_max);
	}
}
