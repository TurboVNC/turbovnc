/*
 *  Copyright (C) 2005-2006 Sun Microsystems, Inc.  All Rights Reserved.
 *  Copyright (C) 2004 Landmark Graphics Corporation.  All Rights Reserved.
 *  Copyright (C) 2000, 2001 Const Kaplinsky.  All Rights Reserved.
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
 * tight.c - handle ``tight'' encoding.
 *
 * This file shouldn't be compiled directly. It is included multiple
 * times by rfbproto.c, each time with a different definition of the
 * macro BPP. For each value of BPP, this file defines a function
 * which handles a tight-encoded rectangle with BPP bits per pixel.
 *
 */

#define TIGHT_MIN_TO_COMPRESS 12

#define CARDBPP CONCAT2E(CARD,BPP)
#define filterPtrBPP CONCAT2E(filterPtr,BPP)

#define HandleTightBPP CONCAT2E(HandleTight,BPP)
#define InitFilterCopyBPP CONCAT2E(InitFilterCopy,BPP)
#define InitFilterPaletteBPP CONCAT2E(InitFilterPalette,BPP)
#define InitFilterGradientBPP CONCAT2E(InitFilterGradient,BPP)
#define FilterCopyBPP CONCAT2E(FilterCopy,BPP)
#define FilterPaletteBPP CONCAT2E(FilterPalette,BPP)
#define FilterGradientBPP CONCAT2E(FilterGradient,BPP)

#if BPP != 8
#define DecompressJpegRectBPP CONCAT2E(DecompressJpegRect,BPP)
#endif

#ifndef RGB_TO_PIXEL

#define RGB_TO_PIXEL(bpp,r,g,b)						\
  (((CARD##bpp)(r) & myFormat.redMax) << myFormat.redShift |		\
   ((CARD##bpp)(g) & myFormat.greenMax) << myFormat.greenShift |	\
   ((CARD##bpp)(b) & myFormat.blueMax) << myFormat.blueShift)

#define RGB24_TO_PIXEL(bpp,r,g,b)                                       \
   ((((CARD##bpp)(r) & 0xFF) * myFormat.redMax + 127) / 255             \
    << myFormat.redShift |                                              \
    (((CARD##bpp)(g) & 0xFF) * myFormat.greenMax + 127) / 255           \
    << myFormat.greenShift |                                            \
    (((CARD##bpp)(b) & 0xFF) * myFormat.blueMax + 127) / 255            \
    << myFormat.blueShift)

#define RGB24_TO_PIXEL32(r,g,b)						\
  (((CARD32)(r) & 0xFF) << myFormat.redShift |				\
   ((CARD32)(g) & 0xFF) << myFormat.greenShift |			\
   ((CARD32)(b) & 0xFF) << myFormat.blueShift)

#endif

extern XImage *image;

/* Type declarations */

typedef void (*filterPtrBPP)(int, int, int);

/* Prototypes */

static int InitFilterCopyBPP (int rw, int rh);
static int InitFilterPaletteBPP (int rw, int rh);
static int InitFilterGradientBPP (int rw, int rh);
static void FilterCopyBPP (int srcx, int srcy, int numRows);
static void FilterPaletteBPP (int srcx, int srcy, int numRows);
static void FilterGradientBPP (int srcx, int srcy, int numRows);

static Bool DecompressJpegRectBPP(int x, int y, int w, int h);

/* Definitions */

static Bool
HandleTightBPP (int rx, int ry, int rw, int rh)
{
  CARDBPP fill_colour;
  XGCValues gcv;
  CARD8 comp_ctl;
  CARD8 filter_id;
  filterPtrBPP filterFn;
  z_streamp zs;
  int err, stream_id, compressedLen, bitsPixel;
  int bufferSize, rowSize, numRows;
  Bool readUncompressed = False;
  CARDBPP *rawData;

  if (!ReadFromRFBServer((char *)&comp_ctl, 1))
    return False;

  /* Flush zlib streams if we are told by the server to do so. */
  for (stream_id = 0; stream_id < 4; stream_id++) {
    if ((comp_ctl & 1) && zlibStreamActive[stream_id]) {
      if (inflateEnd (&zlibStream[stream_id]) != Z_OK &&
	  zlibStream[stream_id].msg != NULL)
	fprintf(stderr, "inflateEnd: %s\n", zlibStream[stream_id].msg);
      zlibStreamActive[stream_id] = False;
    }
    comp_ctl >>= 1;
  }

  if ((comp_ctl & rfbTightNoZlib) == rfbTightNoZlib) {
     comp_ctl &= ~(rfbTightNoZlib);
     readUncompressed = True;
  }

  /* Handle solid rectangles. */
  if (comp_ctl == rfbTightFill) {
#if BPP == 32
    if (myFormat.depth == 24 && myFormat.redMax == 0xFF &&
	myFormat.greenMax == 0xFF && myFormat.blueMax == 0xFF) {
      if (!ReadFromRFBServer(buffer, 3))
	return False;
      fill_colour = RGB24_TO_PIXEL32(buffer[0], buffer[1], buffer[2]);
    } else {
      if (!ReadFromRFBServer((char*)&fill_colour, sizeof(fill_colour)))
	return False;
    }
#else
    if (!ReadFromRFBServer((char*)&fill_colour, sizeof(fill_colour)))
	return False;
#endif

#if (BPP == 8)
    gcv.foreground = (appData.useBGR233) ?
      BGR233ToPixel[fill_colour] : fill_colour;
#else
    gcv.foreground = fill_colour;
#endif

    FillRectangle(&gcv, rx, ry, rw, rh);
    return True;
  }

#if BPP == 8
  if (comp_ctl == rfbTightJpeg) {
    fprintf(stderr, "Tight encoding: JPEG is not supported in 8 bpp mode.\n");
    return False;
  }
#else
  if (comp_ctl == rfbTightJpeg) {
    return DecompressJpegRectBPP(rx, ry, rw, rh);
  }
#endif

  /* Quit on unsupported subencoding value. */
  if (comp_ctl > rfbTightMaxSubencoding) {
    fprintf(stderr, "Tight encoding: bad subencoding value received.\n");
    return False;
  }

  /*
   * Here primary compression mode handling begins.
   * Data was processed with optional filter + zlib compression.
   */

  /* First, we should identify a filter to use. */
  if ((comp_ctl & rfbTightExplicitFilter) != 0) {
    if (!ReadFromRFBServer((char*)&filter_id, 1))
      return False;

    switch (filter_id) {
    case rfbTightFilterCopy:
      filterFn = FilterCopyBPP;
      bitsPixel = InitFilterCopyBPP(rw, rh);
      break;
    case rfbTightFilterPalette:
      filterFn = FilterPaletteBPP;
      bitsPixel = InitFilterPaletteBPP(rw, rh);
      break;
    case rfbTightFilterGradient:
      filterFn = FilterGradientBPP;
      bitsPixel = InitFilterGradientBPP(rw, rh);
      break;
    default:
      fprintf(stderr, "Tight encoding: unknown filter code received.\n");
      return False;
    }
  } else {
    filterFn = FilterCopyBPP;
    bitsPixel = InitFilterCopyBPP(rw, rh);
  }
  if (bitsPixel == 0) {
    fprintf(stderr, "Tight encoding: error receiving palette.\n");
    return False;
  }

  /* Determine if the data should be decompressed or just copied. */
  rowSize = (rw * bitsPixel + 7) / 8;
  bufferSize = -1;
  if (rh * rowSize < TIGHT_MIN_TO_COMPRESS)
    bufferSize = rh * rowSize;
  else if (readUncompressed) {
    bufferSize = (int)ReadCompactLen();
  }
  if (bufferSize != -1) {
    uncompressedData = (char *)realloc(uncompressedData, bufferSize);
    if (!uncompressedData) {
      fprintf(stderr, "Memory allocation error\n");
      return False;
    }
    if (!ReadFromRFBServer(uncompressedData, bufferSize))
      return False;
    filterFn(rx, ry, rh);
    if (appData.useBGR233) CopyDataToImage(buffer, rx, ry, rw, rh);
    if (!appData.doubleBuffer) CopyImageToScreen(rx, ry, rw, rh);

    return True;
  }

  /* Read the length (1..3 bytes) of compressed data following. */
  compressedLen = (int)ReadCompactLen();
  if (compressedLen <= 0) {
    fprintf(stderr, "Incorrect data received from the server.\n");
    return False;
  }

  /* Now let's initialize compression stream if needed. */
  stream_id = comp_ctl & 0x03;
  zs = &zlibStream[stream_id];
  if (!zlibStreamActive[stream_id]) {
    zs->zalloc = Z_NULL;
    zs->zfree = Z_NULL;
    zs->opaque = Z_NULL;
    err = inflateInit(zs);
    if (err != Z_OK) {
      if (zs->msg != NULL)
	fprintf(stderr, "InflateInit error: %s.\n", zs->msg);
      return False;
    }
    zlibStreamActive[stream_id] = True;
  }

  /* Read, decode and draw actual pixel data in a loop. */

  compressedData = (char *)realloc(compressedData, compressedLen);
  if (!compressedData) {
    fprintf(stderr, "Memory allocation error\n");
    return False;
  }
  uncompressedData = (char *)realloc(uncompressedData, rh * rowSize);
  if (!uncompressedData) {
    fprintf(stderr, "Memory allocation error\n");
    return False;
  }

  if (!ReadFromRFBServer(compressedData, compressedLen))
    return False;
  zs->next_in = (Bytef *)compressedData;
  zs->avail_in = compressedLen;
  zs->next_out = (Bytef *)uncompressedData;
  zs->avail_out = rh * rowSize;

  err = inflate(zs, Z_SYNC_FLUSH);
  if (err != Z_OK && err != Z_STREAM_END) {
    if (zs->msg != NULL) {
      fprintf(stderr, "Inflate error: %s.\n", zs->msg);
    } else {
      fprintf(stderr, "Inflate error: %d.\n", err);
    }
    return False;
  }

  filterFn(rx, ry, rh);
  if (appData.useBGR233) CopyDataToImage(buffer, rx, ry, rw, rh);
  if (!appData.doubleBuffer) CopyImageToScreen(rx, ry, rw, rh);

  return True;
}

/*----------------------------------------------------------------------------
 *
 * Filter stuff.
 *
 */

/*
   The following variables are defined in rfbproto.c:
     static Bool cutZeros;
     static int rectWidth, rectColors;
     static CARD8 tightPalette[256*4];
     static CARD8 tightPrevRow[2048*3*sizeof(CARD16)];
*/

static int
InitFilterCopyBPP (int rw, int rh)
{
  rectWidth = rw;

#if BPP == 32
  if (myFormat.depth == 24 && myFormat.redMax == 0xFF &&
      myFormat.greenMax == 0xFF && myFormat.blueMax == 0xFF) {
    cutZeros = True;
    return 24;
  } else {
    cutZeros = False;
  }
#endif

  return BPP;
}

static void
FilterCopyBPP (int srcx, int srcy, int numRows)
{
  CARDBPP *dst = (CARDBPP *)&image->data[srcy * image->bytes_per_line
                                         + srcx * image->bits_per_pixel/8];
  int dstw = image->bytes_per_line / (image->bits_per_pixel / 8);
  int y;
#if BPP == 32
  int x;
#endif

  if (appData.useBGR233) {
    dst = (CARDBPP *)buffer;
    dstw = rectWidth;
  }

#if BPP == 32
  if (cutZeros) {
    for (y = 0; y < numRows; y++) {
      for (x = 0; x < rectWidth; x++) {
	dst[y*dstw+x] =
	  RGB24_TO_PIXEL32(uncompressedData[(y*rectWidth+x)*3],
			   uncompressedData[(y*rectWidth+x)*3+1],
			   uncompressedData[(y*rectWidth+x)*3+2]);
      }
    }
    return;
  }
#endif

  for (y = 0; y < numRows; y++)
    memcpy (&dst[y*dstw], &uncompressedData[y*rectWidth], rectWidth * (BPP / 8));
}

static int
InitFilterGradientBPP (int rw, int rh)
{
  int bits;

  bits = InitFilterCopyBPP(rw, rh);
  if (cutZeros)
    memset(tightPrevRow, 0, rw * 3);
  else
    memset(tightPrevRow, 0, rw * 3 * sizeof(CARD16));

  return bits;
}

#if BPP == 32

static void
FilterGradient24 (int srcx, int srcy, int numRows)
{
  CARDBPP *dst = (CARDBPP *)&image->data[srcy * image->bytes_per_line
                                         + srcx * image->bits_per_pixel/8];
  int dstw = image->bytes_per_line / (image->bits_per_pixel / 8);
  int x, y, c;
  CARD8 thisRow[2048*3];
  CARD8 pix[3];
  int est[3];

  if (appData.useBGR233) {
    dst = (CARDBPP *)buffer;
    dstw = rectWidth;
  }

  for (y = 0; y < numRows; y++) {

    /* First pixel in a row */
    for (c = 0; c < 3; c++) {
      pix[c] = tightPrevRow[c] + uncompressedData[y*rectWidth*3+c];
      thisRow[c] = pix[c];
    }
    dst[y*dstw] = RGB24_TO_PIXEL32(pix[0], pix[1], pix[2]);

    /* Remaining pixels of a row */
    for (x = 1; x < rectWidth; x++) {
      for (c = 0; c < 3; c++) {
	est[c] = (int)tightPrevRow[x*3+c] + (int)pix[c] -
		 (int)tightPrevRow[(x-1)*3+c];
	if (est[c] > 0xFF) {
	  est[c] = 0xFF;
	} else if (est[c] < 0x00) {
	  est[c] = 0x00;
	}
	pix[c] = (CARD8)est[c] + buffer[(y*rectWidth+x)*3+c];
	thisRow[x*3+c] = pix[c];
      }
      dst[y*dstw+x] = RGB24_TO_PIXEL32(pix[0], pix[1], pix[2]);
    }

    memcpy(tightPrevRow, thisRow, rectWidth * 3);
  }
}

#endif

static void
FilterGradientBPP (int srcx, int srcy, int numRows)
{
  int x, y, c;
  CARDBPP *dst = (CARDBPP *)&image->data[srcy * image->bytes_per_line
                                         + srcx * image->bits_per_pixel/8];
  int dstw = image->bytes_per_line / (image->bits_per_pixel / 8);
  CARDBPP *src = (CARDBPP *)uncompressedData;
  CARD16 *thatRow = (CARD16 *)tightPrevRow;
  CARD16 thisRow[2048*3];
  CARD16 pix[3];
  CARD16 max[3];
  int shift[3];
  int est[3];

  if (appData.useBGR233) {
    dst = (CARDBPP *)buffer;
    dstw = rectWidth;
  }

#if BPP == 32
  if (cutZeros) {
    FilterGradient24(srcx, srcy, numRows);
    return;
  }
#endif

  max[0] = myFormat.redMax;
  max[1] = myFormat.greenMax;
  max[2] = myFormat.blueMax;

  shift[0] = myFormat.redShift;
  shift[1] = myFormat.greenShift;
  shift[2] = myFormat.blueShift;

  for (y = 0; y < numRows; y++) {

    /* First pixel in a row */
    for (c = 0; c < 3; c++) {
      pix[c] = (CARD16)((src[y*rectWidth] >> shift[c]) + thatRow[c] & max[c]);
      thisRow[c] = pix[c];
    }
    dst[y*dstw] = RGB_TO_PIXEL(BPP, pix[0], pix[1], pix[2]);

    /* Remaining pixels of a row */
    for (x = 1; x < rectWidth; x++) {
      for (c = 0; c < 3; c++) {
	est[c] = (int)thatRow[x*3+c] + (int)pix[c] - (int)thatRow[(x-1)*3+c];
	if (est[c] > (int)max[c]) {
	  est[c] = (int)max[c];
	} else if (est[c] < 0) {
	  est[c] = 0;
	}
	pix[c] = (CARD16)((src[y*rectWidth+x] >> shift[c]) + est[c] & max[c]);
	thisRow[x*3+c] = pix[c];
      }
      dst[y*dstw+x] = RGB_TO_PIXEL(BPP, pix[0], pix[1], pix[2]);
    }
    memcpy(thatRow, thisRow, rectWidth * 3 * sizeof(CARD16));
  }
}

static int
InitFilterPaletteBPP (int rw, int rh)
{
  int i;
  CARD8 numColors;
  CARDBPP *palette = (CARDBPP *)tightPalette;

  rectWidth = rw;

  if (!ReadFromRFBServer((char*)&numColors, 1))
    return 0;

  rectColors = (int)numColors;
  if (++rectColors < 2)
    return 0;

#if BPP == 32
  if (myFormat.depth == 24 && myFormat.redMax == 0xFF &&
      myFormat.greenMax == 0xFF && myFormat.blueMax == 0xFF) {
    if (!ReadFromRFBServer((char*)&tightPalette, rectColors * 3))
      return 0;
    for (i = rectColors - 1; i >= 0; i--) {
      palette[i] = RGB24_TO_PIXEL32(tightPalette[i*3],
				    tightPalette[i*3+1],
				    tightPalette[i*3+2]);
    }
    return (rectColors == 2) ? 1 : 8;
  }
#endif

  if (!ReadFromRFBServer((char*)&tightPalette, rectColors * (BPP / 8)))
    return 0;

  return (rectColors == 2) ? 1 : 8;
}

static void
FilterPaletteBPP (int srcx, int srcy, int numRows)
{
  int x, y, b, w;
  CARDBPP *dst = (CARDBPP *)&image->data[srcy * image->bytes_per_line
                                         + srcx * image->bits_per_pixel/8];
  int dstw = image->bytes_per_line / (image->bits_per_pixel / 8);
  CARD8 *src = (CARD8 *)uncompressedData;
  CARDBPP *palette = (CARDBPP *)tightPalette;

  if (appData.useBGR233) {
    dst = (CARDBPP *)buffer;
    dstw = rectWidth;
  }

  if (rectColors == 2) {
    w = (rectWidth + 7) / 8;
    for (y = 0; y < numRows; y++) {
      for (x = 0; x < rectWidth / 8; x++) {
	for (b = 7; b >= 0; b--)
	  dst[y*dstw+x*8+7-b] = palette[src[y*w+x] >> b & 1];
      }
      for (b = 7; b >= 8 - rectWidth % 8; b--) {
	dst[y*dstw+x*8+7-b] = palette[src[y*w+x] >> b & 1];
      }
    }
  } else {
    for (y = 0; y < numRows; y++)
      for (x = 0; x < rectWidth; x++)
	dst[y*dstw+x] = palette[(int)src[y*rectWidth+x]];
  }
}

#if BPP != 8

/*----------------------------------------------------------------------------
 *
 * JPEG decompression.
 *
 */

/*
   The following variables are defined in rfbproto.c:
     static Bool jpegError;
     static struct jpeg_source_mgr jpegSrcManager;
     static JOCTET *jpegBufferPtr;
     static size_t *jpegBufferLen;
*/

static Bool
DecompressJpegRectBPP(int x, int y, int w, int h)
{
  int compressedLen;
  char *dstptr;
  int ps, flags=0;

  compressedLen = (int)ReadCompactLen();
  if (compressedLen <= 0) {
    fprintf(stderr, "Incorrect data received from the server.\n");
    return False;
  }

  compressedData = (char *)realloc(compressedData, compressedLen);
  if (compressedData == NULL) {
    fprintf(stderr, "Memory allocation error.\n");
    return False;
  }

  if (!ReadFromRFBServer(compressedData, compressedLen)) {
    return False;
  }

  if(!tjhnd) {
    if((tjhnd=tjInitDecompress())==NULL) {
      fprintf(stderr, "TurboJPEG error: %s\n", tjGetErrorStr());
      return False;
    }
  }     

  ps=image->bits_per_pixel/8;
  if(myFormat.bigEndian && ps==4) flags|=TJ_ALPHAFIRST;
  if(myFormat.redShift==16 && myFormat.blueShift==0)
    flags|=TJ_BGR;
  if(myFormat.bigEndian) flags^=TJ_BGR;

  dstptr=&image->data[image->bytes_per_line*y+x*ps];
  if(tjDecompress(tjhnd, (unsigned char *)compressedData, (unsigned long)compressedLen,
    (unsigned char *)dstptr, w, image->bytes_per_line, h, ps, flags)==-1) {
    fprintf(stderr, "TurboJPEG error: %s\n", tjGetErrorStr());
    return False;
  }

  if (!appData.doubleBuffer)
    CopyImageToScreen(x, y, w, h);

  return True;
}

#endif

