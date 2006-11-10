/* Copyright (C)2004 Landmark Graphics
 * Copyright (C)2005 Sun Microsystems, Inc.
 *
 * This library is free software and may be redistributed and/or modified under
 * the terms of the wxWindows Library License, Version 3 or (at your option)
 * any later version.  The full license is in the LICENSE.txt file included
 * with this distribution.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * wxWindows Library License for more details.
 */

// This implements a JPEG compressor/decompressor using the Sun mediaLib

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mlib.h>
#include "turbojpeg.h"

static const char *lasterror="No error";
static const int _mcuw[NUMSUBOPT]={8, 16, 16};
static const int _mcuh[NUMSUBOPT]={8, 8, 16};

#define checkhandle(h) jpgstruct *jpg=(jpgstruct *)h; \
	if(!jpg) {lasterror="Invalid handle";  return -1;}

#define _throw(c) {lasterror=c;  goto bailout;}
#define _mlib(a) {mlib_status __err;  if((__err=(a))!=MLIB_SUCCESS) _throw("MLIB failure in "#a"()");}
#define _mlibn(a) {if(!(a)) _throw("MLIB failure in "#a"()");}
#define _catch(a) {if((a)==-1) goto bailout;}

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

#define write_byte(j, b) {if(j->bytesleft<=0) _throw("Not enough space in buffer");  \
	*j->jpgptr=b;  j->jpgptr++;  j->bytesprocessed++;  j->bytesleft--;}
#define write_word(j, w) {write_byte(j, (w>>8)&0xff);  write_byte(j, w&0xff);}

#define read_byte(j, b) {if(j->bytesleft<=0) _throw("Unexpected end of image");  \
	b=*j->jpgptr;  j->jpgptr++;  j->bytesleft--;  j->bytesprocessed++;}
#define read_word(j, w) {mlib_u8 __b;  read_byte(j, __b);  w=(__b<<8);  \
	read_byte(j, __b);  w|=(__b&0xff);}

typedef struct
{
	unsigned int ehufco[256];	/* code for each symbol */
  char ehufsi[256];		/* length of code for each symbol */
  /* If no code has been allocated for a symbol S, ehufsi[S] contains 0 */
} c_derived_tbl;

#define HUFF_LOOKAHEAD	8	/* # of bits of lookahead */

typedef struct
{
	/* Basic tables: (element [0] of each array is unused) */
	int maxcode[18];		/* largest code of length k (-1 if none) */
	/* (maxcode[17] is a sentinel to ensure jpeg_huff_decode terminates) */
	int valoffset[18];		/* huffval[] offset for codes of length k */
	/* valoffset[k] = huffval[] index of 1st symbol of code length k, less
	 * the smallest code of length k; so given a code of length k, the
	 * corresponding symbol is huffval[code + valoffset[k]]
	 */

	/* Lookahead tables: indexed by the next HUFF_LOOKAHEAD bits of
	 * the input data stream.  If the next Huffman code is no more
	 * than HUFF_LOOKAHEAD bits long, we can obtain its length and
	 * the corresponding symbol directly from these tables.
	 */
	int look_nbits[1<<HUFF_LOOKAHEAD]; /* # bits, or 0 if too long */
	mlib_u8 look_sym[1<<HUFF_LOOKAHEAD]; /* symbol, or unused */
	mlib_u8 huffval[256];		/* The symbols, in order of incr code length */
} d_derived_tbl;

typedef struct _jpgstruct
{
	mlib_d64 chromqtable[64], lumqtable[64];
	mlib_d64 _mcubuf[384/4];

	mlib_u8 *bmpbuf, *bmpptr, *jpgbuf, *jpgptr;
	int width, height, pitch, ps, subsamp, qual, flags;
	unsigned long bytesprocessed, bytesleft;

	int huffbits, huffbuf, unread_marker, insufficient_data;
	c_derived_tbl *e_dclumtable, *e_aclumtable, *e_dcchromtable, *e_acchromtable;
	d_derived_tbl *d_dclumtable, *d_aclumtable, *d_dcchromtable, *d_acchromtable;
	mlib_s16 *mcubuf;
	int initc, initd, isvis;
} jpgstruct;


//////////////////////////////////////////////////////////////////////////////
//    COMPRESSOR
//////////////////////////////////////////////////////////////////////////////

// Default quantization tables per JPEG spec

static const mlib_s16 lumqtable[64]=
{
  16,  11,  10,  16,  24,  40,  51,  61,
  12,  12,  14,  19,  26,  58,  60,  55,
  14,  13,  16,  24,  40,  57,  69,  56,
  14,  17,  22,  29,  51,  87,  80,  62,
  18,  22,  37,  56,  68, 109, 103,  77,
  24,  35,  55,  64,  81, 104, 113,  92,
  49,  64,  78,  87, 103, 121, 120, 101,
  72,  92,  95,  98, 112, 100, 103,  99
};

static const mlib_s16 chromqtable[64]=
{
  17,  18,  24,  47,  99,  99,  99,  99,
  18,  21,  26,  66,  99,  99,  99,  99,
  24,  26,  56,  99,  99,  99,  99,  99,
  47,  66,  99,  99,  99,  99,  99,  99,
  99,  99,  99,  99,  99,  99,  99,  99,
  99,  99,  99,  99,  99,  99,  99,  99,
  99,  99,  99,  99,  99,  99,  99,  99,
  99,  99,  99,  99,  99,  99,  99,  99
};

// Huffman tables per JPEG spec
static const mlib_u8 dclumbits[16]=
{
	0, 1, 5, 1, 1, 1, 1, 1,
	1, 0, 0, 0, 0, 0, 0, 0
};
static const mlib_u8 dclumvals[]=
{
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
};

static const mlib_u8 dcchrombits[16]=
{
	0, 3, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 0, 0, 0, 0, 0
};
static const mlib_u8 dcchromvals[]=
{
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
};

static const mlib_u8 aclumbits[16]=
{
	0, 2, 1, 3, 3, 2, 4, 3,
	5, 5, 4, 4, 0, 0, 1, 0x7d
};
static const mlib_u8 aclumvals[]=
{
	0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
	0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
	0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
	0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
	0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
	0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
	0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
	0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
	0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
	0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
	0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
	0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
	0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
	0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
	0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
	0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
	0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
	0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
	0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
	0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
	0xf9, 0xfa
};

static const mlib_u8 acchrombits[16]=
{
	0, 2, 1, 2, 4, 4, 3, 4,
	7, 5, 4, 4, 0, 1, 2, 0x77
};
static const mlib_u8 acchromvals[]=
{
	0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
	0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
	0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
	0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
	0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
	0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
	0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
	0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
	0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
	0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
	0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
	0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
	0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
	0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
	0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
	0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
	0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
	0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
	0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
	0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
	0xf9, 0xfa
};

#include "turbojpeg-mlibhuff.c"

static mlib_u8 *e_preconvertline(mlib_u8 *srcbuf, mlib_u8 *linebuf, int *ps, int flags,
	int srcw, int dstw)
{
	// Convert BGR to ABGR
	if(*ps==3 && flags&TJ_BGR)
	{
		mlib_VideoColorBGRint_to_ABGRint((mlib_u32 *)linebuf, srcbuf, NULL,
			0, srcw, 1, dstw*4, srcw*(*ps), 0);
		*ps=4;
		return linebuf;
	}
	// Convert BGRA to ABGR
	else if(*ps==4 && flags&TJ_BGR && !(flags&TJ_ALPHAFIRST))
	{
		mlib_VideoColorBGRAint_to_ARGBint((mlib_u32 *)linebuf, (mlib_u32 *)srcbuf,
			srcw, 1, dstw*4, srcw*(*ps));
		return linebuf;
	}
	// Convert RGBA to ABGR
	else if(*ps==4 && !(flags&TJ_BGR) && !(flags&TJ_ALPHAFIRST))
	{
		mlib_VideoColorRGBAint_to_ABGRint((mlib_u32 *)linebuf, (mlib_u32 *)srcbuf,
			srcw, 1, dstw*4, srcw*(*ps));
		return linebuf;
	}
	else return srcbuf;
}

static mlib_u8 *e_extendline(mlib_u8 *srcbuf, mlib_u8 *linebuf, int oldw, int neww, int ps)
{
	int i;
	if(((long)srcbuf&7L)!=0L) {mlib_VectorCopy_U8(linebuf, srcbuf, oldw*ps);  srcbuf=linebuf;}
	if(oldw==neww) return srcbuf;
	if(srcbuf!=linebuf) mlib_VectorCopy_U8(linebuf, srcbuf, oldw*ps);
	for(i=oldw; i<neww; i++)
		mlib_VectorCopy_U8(&linebuf[i*ps], &linebuf[(oldw-1)*ps], ps);
	return linebuf;
}

static int e_mcu_color_convert(jpgstruct *jpg, mlib_u8 *ybuf, int yw, mlib_u8 *cbbuf,
	mlib_u8 *crbuf, int cw, int startline, mlib_u8 *linebuf, mlib_u8 *linebuf2)
{
	int rgbstride=jpg->pitch;
	mlib_status (DLLCALL *ccfct)(mlib_u8 *, mlib_u8 *, mlib_u8 *,
		const mlib_u8 *, mlib_s32)=NULL;
	mlib_status (DLLCALL *ccfct420)(mlib_u8 *, mlib_u8 *, mlib_u8 *, mlib_u8 *,
		const mlib_u8 *, const mlib_u8 *, mlib_s32)=NULL;
	int mcuh=_mcuh[jpg->subsamp], h, j;

	h=mcuh;
	if(startline+mcuh>jpg->height) h=jpg->height-startline;

	switch(jpg->subsamp)
	{
		case TJ_411:
			if(jpg->flags&TJ_BGR && jpg->flags&TJ_ALPHAFIRST && jpg->ps==4) // ABGR
				ccfct420=mlib_VideoColorABGR2JFIFYCC420;
			else if(!(jpg->flags&TJ_BGR) && !(jpg->flags&TJ_ALPHAFIRST) && jpg->ps==4) // RGBA
				ccfct420=mlib_VideoColorABGR2JFIFYCC420;
			else if(!(jpg->flags&TJ_BGR) && jpg->flags&TJ_ALPHAFIRST && jpg->ps==4) // ARGB
				ccfct420=mlib_VideoColorARGB2JFIFYCC420;
			else if(jpg->flags&TJ_BGR && !(jpg->flags&TJ_ALPHAFIRST) && jpg->ps==4) // BGRA
				ccfct420=mlib_VideoColorARGB2JFIFYCC420;
			else if(!(jpg->flags&TJ_BGR) && jpg->ps==3) ccfct420=mlib_VideoColorRGB2JFIFYCC420; // RGB
			else if(jpg->flags&TJ_BGR && jpg->ps==3) ccfct420=mlib_VideoColorABGR2JFIFYCC420; //BGR
			break;
		case TJ_422:
			if(jpg->flags&TJ_BGR && jpg->flags&TJ_ALPHAFIRST && jpg->ps==4) // ABGR
				ccfct=mlib_VideoColorABGR2JFIFYCC422;
			else if(!(jpg->flags&TJ_BGR) && !(jpg->flags&TJ_ALPHAFIRST) && jpg->ps==4) // RGBA
				ccfct=mlib_VideoColorABGR2JFIFYCC422;
			else if(!(jpg->flags&TJ_BGR) && jpg->flags&TJ_ALPHAFIRST && jpg->ps==4) // ARGB
				ccfct=mlib_VideoColorARGB2JFIFYCC422;
			else if(jpg->flags&TJ_BGR && !(jpg->flags&TJ_ALPHAFIRST) && jpg->ps==4) // BGRA
				ccfct=mlib_VideoColorARGB2JFIFYCC422;
			else if(!(jpg->flags&TJ_BGR) && jpg->ps==3) ccfct=mlib_VideoColorRGB2JFIFYCC422; // RGB
			else if(jpg->flags&TJ_BGR && jpg->ps==3) ccfct=mlib_VideoColorABGR2JFIFYCC422; // BGR
			break;
		case TJ_444:
			if(jpg->flags&TJ_BGR && jpg->flags&TJ_ALPHAFIRST && jpg->ps==4) // ABGR
				ccfct=mlib_VideoColorABGR2JFIFYCC444;
			else if(!(jpg->flags&TJ_BGR) && !(jpg->flags&TJ_ALPHAFIRST) && jpg->ps==4) // RGBA
				ccfct=mlib_VideoColorABGR2JFIFYCC444;
			else if(!(jpg->flags&TJ_BGR) && jpg->flags&TJ_ALPHAFIRST && jpg->ps==4) // ARGB
				ccfct=mlib_VideoColorARGB2JFIFYCC444;
			else if(jpg->flags&TJ_BGR && !(jpg->flags&TJ_ALPHAFIRST) && jpg->ps==4) // BGRA
				ccfct=mlib_VideoColorARGB2JFIFYCC444;
			else if(!(jpg->flags&TJ_BGR) && jpg->ps==3) ccfct=mlib_VideoColorRGB2JFIFYCC444; // RGB
			else if(jpg->flags&TJ_BGR && jpg->ps==3) ccfct=mlib_VideoColorABGR2JFIFYCC444; // BGR
			break;
		default:
			_throw("Invalid argument to mcu_color_convert()");
	}

	// Do color conversion
	rgbstride=jpg->pitch;
	if(jpg->flags&TJ_BOTTOMUP) rgbstride=-jpg->pitch;

	if(jpg->subsamp==TJ_411)
	{
		mlib_u8 *y=ybuf, *cb=cbbuf, *cr=crbuf, *tmpptr, *tmpptr2;
		for(j=0; j<h; j+=2, jpg->bmpptr+=rgbstride*2, y+=yw*2, cb+=cw, cr+=cw)
		{
			int ps=jpg->ps;
			tmpptr=e_preconvertline(jpg->bmpptr, linebuf, &ps, jpg->flags, jpg->width, yw);
			tmpptr=e_extendline(tmpptr, linebuf, jpg->width, yw, ps);
			if(j<h-1)
			{
				ps=jpg->ps;
				tmpptr2=e_preconvertline(jpg->bmpptr+rgbstride, linebuf2, &ps, jpg->flags, jpg->width, yw);
				tmpptr2=e_extendline(tmpptr2, linebuf2, jpg->width, yw, ps);
			} else tmpptr2=tmpptr;
			_mlib(ccfct420(y, y+yw, cb, cr, tmpptr, tmpptr2, yw));
		}
	}
	else
	{
		mlib_u8 *y=ybuf, *cb=cbbuf, *cr=crbuf, *tmpptr;
		for(j=0; j<h; j++, jpg->bmpptr+=rgbstride, y+=yw, cb+=cw, cr+=cw)
		{
			int ps=jpg->ps;
			tmpptr=e_preconvertline(jpg->bmpptr, linebuf, &ps, jpg->flags, jpg->width, yw);
			tmpptr=e_extendline(tmpptr, linebuf, jpg->width, yw, ps);
			_mlib(ccfct(y, cb, cr, tmpptr, yw));
		}
	}

	// Extend to a full MCU row (if necessary)
	if(mcuh>h)
	{
		for(j=h; j<mcuh; j++)
		{
			mlib_VectorCopy_U8(&ybuf[j*yw], &ybuf[(h-1)*yw], yw);
		}
	}
	if(8>(h-1)*8/mcuh+1)
	{
		for(j=(h-1)*8/mcuh+1; j<8; j++)
		{
			mlib_VectorCopy_U8(&cbbuf[j*cw], &cbbuf[((h-1)*8/mcuh)*cw], cw);
			mlib_VectorCopy_U8(&crbuf[j*cw], &crbuf[((h-1)*8/mcuh)*cw], cw);
		}
	}

	return 0;

	bailout:
	return -1;
}

static void QuantFwdRawTableInit(mlib_s16 *rawqtable, int quality)
{
	int scale=quality, temp;  int i;
	if(scale<=0) scale=1;  if(scale>100) scale=100;
	if(scale<50) scale=5000/scale;  else scale=200-scale*2;

	for(i=0; i<64; i++)
	{
		temp=((int)rawqtable[i]*scale+50)/100;
		if(temp<=0) temp=1;
		if(temp>255) temp=255;
		rawqtable[i]=(mlib_s16)temp;
  }
}

mlib_status my_VideoQuantizeInit_S16(mlib_d64 dqtable[64],
	const mlib_s16 iqtable[64])
{
	mlib_s16 *qtable=(mlib_s16 *)dqtable;
	mlib_s32 i;
	#pragma pipeloop(0)
	for(i=0; i<64; i++)
	{
		mlib_s32 tmp=(mlib_s32)(32768.0/iqtable[i]+0.5);
		qtable[i]=tmp-(tmp>>15);
	}
	return (MLIB_SUCCESS);
}

static int encode_jpeg_init(jpgstruct *jpg)
{
	mlib_s16 rawqtable[64];  int i, nval, len;
	int mcuw=_mcuw[jpg->subsamp], mcuh=_mcuh[jpg->subsamp];

	jpg->bytesprocessed=0;
	jpg->huffbits = jpg->huffbuf = jpg->unread_marker = jpg->insufficient_data = 0;

	if(jpg->flags&TJ_BOTTOMUP) jpg->bmpptr=&jpg->bmpbuf[jpg->pitch*(jpg->height-1)];
	else jpg->bmpptr=jpg->bmpbuf;
	jpg->jpgptr=jpg->jpgbuf;

	write_byte(jpg, 0xff);  write_byte(jpg, 0xd8);  // Start of image marker

	// JFIF header
	write_byte(jpg, 0xff);  write_byte(jpg, 0xe0);  // JFIF marker
	write_word(jpg, 16);  // JFIF header length
	write_byte(jpg, 'J');  write_byte(jpg, 'F');
	write_byte(jpg, 'I');  write_byte(jpg, 'F');  write_byte(jpg, 0);
	write_word(jpg, 0x0101);  // JFIF version
	write_byte(jpg, 0);  // JFIF density units
	write_word(jpg, 1);  // JFIF X density
	write_word(jpg, 1);  // JFIF Y density
	write_byte(jpg, 0);  // thumbnail width
	write_byte(jpg, 0);  // thumbnail height

	// Generate and write quant. tables
	memcpy(rawqtable, lumqtable, 64*sizeof(mlib_s16));
	QuantFwdRawTableInit(rawqtable, jpg->qual);
	if(jpg->isvis) {_mlib(my_VideoQuantizeInit_S16(jpg->lumqtable, rawqtable));}
	else {_mlib(mlib_VideoQuantizeInit_S16(jpg->lumqtable, rawqtable));}

	write_byte(jpg, 0xff);  write_byte(jpg, 0xdb);  // DQT marker
	write_word(jpg, 67);  // DQT length
	write_byte(jpg, 0);  // Index for luminance
	for(i=0; i<64; i++) write_byte(jpg, (mlib_u8)rawqtable[jpeg_natural_order[i]]);

	memcpy(rawqtable, chromqtable, 64*sizeof(mlib_s16));
	QuantFwdRawTableInit(rawqtable, jpg->qual);
	if(jpg->isvis) {_mlib(my_VideoQuantizeInit_S16(jpg->chromqtable, rawqtable));}
	else {_mlib(mlib_VideoQuantizeInit_S16(jpg->chromqtable, rawqtable));}

	write_byte(jpg, 0xff);  write_byte(jpg, 0xdb);  // DQT marker
	write_word(jpg, 67);  // DQT length
	write_byte(jpg, 1);  // Index for chrominance
	for(i=0; i<64; i++) write_byte(jpg, (mlib_u8)rawqtable[jpeg_natural_order[i]]);

	// Write default Huffman tables
	write_byte(jpg, 0xff);  write_byte(jpg, 0xc4);  // DHT marker
	nval=0;  for(i=0; i<16; i++) nval+=dclumbits[i];
	len=19+nval;
	write_word(jpg, len);  // DHT length
	write_byte(jpg, 0x00);  // Huffman class
	for(i=0; i<16; i++) write_byte(jpg, dclumbits[i]);
	for(i=0; i<nval; i++) write_byte(jpg, dclumvals[i]);

	write_byte(jpg, 0xff);  write_byte(jpg, 0xc4);  // DHT marker
	nval=0;  for(i=0; i<16; i++) nval+=aclumbits[i];
	len=19+nval;
	write_word(jpg, len);  // DHT length
	write_byte(jpg, 0x10);  // Huffman class
	for(i=0; i<16; i++) write_byte(jpg, aclumbits[i]);
	for(i=0; i<nval; i++) write_byte(jpg, aclumvals[i]);

	write_byte(jpg, 0xff);  write_byte(jpg, 0xc4);  // DHT marker
	nval=0;  for(i=0; i<16; i++) nval+=dcchrombits[i];
	len=19+nval;
	write_word(jpg, len);  // DHT length
	write_byte(jpg, 0x01);  // Huffman class
	for(i=0; i<16; i++) write_byte(jpg, dcchrombits[i]);
	for(i=0; i<nval; i++) write_byte(jpg, dcchromvals[i]);

	write_byte(jpg, 0xff);  write_byte(jpg, 0xc4);  // DHT marker
	nval=0;  for(i=0; i<16; i++) nval+=acchrombits[i];
	len=19+nval;
	write_word(jpg, len);  // DHT length
	write_byte(jpg, 0x11);  // Huffman class
	for(i=0; i<16; i++) write_byte(jpg, acchrombits[i]);
	for(i=0; i<nval; i++) write_byte(jpg, acchromvals[i]);

	// Initialize Huffman tables
	_catch(jpeg_make_c_derived_tbl(dclumbits, dclumvals, 1, jpg->e_dclumtable));
	_catch(jpeg_make_c_derived_tbl(aclumbits, aclumvals, 0, jpg->e_aclumtable));
	_catch(jpeg_make_c_derived_tbl(dcchrombits, dcchromvals, 1, jpg->e_dcchromtable));
	_catch(jpeg_make_c_derived_tbl(acchrombits, acchromvals, 0, jpg->e_acchromtable));

	// Write Start Of Frame
	write_byte(jpg, 0xff);  write_byte(jpg, 0xc0);  // SOF marker
	write_word(jpg, 17);  // SOF length
	write_byte(jpg, 8);  // precision
	write_word(jpg, jpg->height);
	write_word(jpg, jpg->width);
	write_byte(jpg, 3);  // Number of components

	write_byte(jpg, 1);  // Y Component ID
	write_byte(jpg, ((mcuw/8)<<4)+(mcuh/8));  // Horiz. and Vert. sampling factors
	write_byte(jpg, 0);  // Quantization table selector
	for(i=2; i<=3; i++)
	{
		write_byte(jpg, i);  // Component ID
		write_byte(jpg, 0x11);  // Horiz. and Vert. sampling factors
		write_byte(jpg, 1);  // Quantization table selector
	}

	// Write Start of Scan
	write_byte(jpg, 0xff);  write_byte(jpg, 0xda);  // SOS marker
	write_word(jpg, 12);  // SOS length
	write_byte(jpg, 3);  // Number of components
	for(i=1; i<=3; i++)
	{
		write_byte(jpg, i);  // Component ID
		write_byte(jpg, i==1? 0 : 0x11);  // Huffman table selector
	}
	write_byte(jpg, 0);  // Spectral start
	write_byte(jpg, 63);  // Spectral end
	write_byte(jpg, 0);  // Successive approximation (N/A unless progressive)

	return 0;

	bailout:
	return -1;
}

static int encode_jpeg(jpgstruct *jpg)
{
	int i, j, k;
	int mcuw=_mcuw[jpg->subsamp], mcuh=_mcuh[jpg->subsamp];
	int lastdc[3]={0, 0, 0};
	int x,y;
	mlib_u8 *ybuf=NULL, *cbbuf, *crbuf, *linebuf=NULL;
	int yw=(jpg->width+mcuw-1)&(~(mcuw-1));
	int cw=yw*8/mcuw;

	_catch(encode_jpeg_init(jpg));

	_mlibn(ybuf=(mlib_u8 *)mlib_malloc(yw*mcuh + cw*8*2));
	_mlibn(linebuf=(mlib_u8 *)mlib_malloc(yw*4*2));
	cbbuf=&ybuf[yw*mcuh];  crbuf=&ybuf[yw*mcuh+cw*8];

	for(j=0; j<jpg->height; j+=mcuh)
	{
		_catch(e_mcu_color_convert(jpg, ybuf, yw, cbbuf, crbuf, cw, j, linebuf, &linebuf[yw*4]));

		for(i=0; i<yw; i+=mcuw)
		{
			k=0;    // luminance blocks
			for(y=0; y<mcuh; y+=8)

				for(x=0; x<mcuw; x+=8)
				{
					_mlib(mlib_VideoDCT8x8_S16_U8(&jpg->mcubuf[k], &ybuf[i+y*yw+x], yw));
					jpg->mcubuf[k]-=1024;
					_mlib(mlib_VideoQuantize_S16(&jpg->mcubuf[k], jpg->lumqtable));
					_catch( encode_one_block(jpg, &jpg->mcubuf[k], &lastdc[0],
						jpg->e_dclumtable, jpg->e_aclumtable) );
					k+=64;
				}

			// Cb block
			_mlib(mlib_VideoDCT8x8_S16_U8(&jpg->mcubuf[k], &cbbuf[i*8/mcuw], cw));
			jpg->mcubuf[k]-=1024;
			_mlib(mlib_VideoQuantize_S16(&jpg->mcubuf[k], jpg->chromqtable));
			_catch( encode_one_block(jpg, &jpg->mcubuf[k], &lastdc[1],
				jpg->e_dcchromtable, jpg->e_acchromtable) );
			k+=64;

			// Cr block
			_mlib(mlib_VideoDCT8x8_S16_U8(&jpg->mcubuf[k], &crbuf[i*8/mcuw], cw));
			jpg->mcubuf[k]-=1024;
			_mlib(mlib_VideoQuantize_S16(&jpg->mcubuf[k], jpg->chromqtable));
			_catch( encode_one_block(jpg, &jpg->mcubuf[k], &lastdc[2],
				jpg->e_dcchromtable, jpg->e_acchromtable) );

		} // xmcus
	} // ymcus

	// Flush Huffman state
	_catch( flush_bits(jpg) );

	write_byte(jpg, 0xff);  write_byte(jpg, 0xd9);  // EOI marker

	if(ybuf) mlib_free(ybuf);
	if(linebuf) mlib_free(linebuf);
	return 0;

	bailout:
	if(ybuf) mlib_free(ybuf);
	if(linebuf) mlib_free(linebuf);
	return -1;
}

DLLEXPORT tjhandle DLLCALL tjInitCompress(void)
{
	jpgstruct *jpg=NULL;  char *v=NULL;

	if((jpg=(jpgstruct *)mlib_malloc(sizeof(jpgstruct)))==NULL)
		_throw("Memory allocation failure");
	memset(jpg, 0, sizeof(jpgstruct));

	jpg->mcubuf=(mlib_s16 *)jpg->_mcubuf;

	if((jpg->e_dclumtable=(c_derived_tbl *)mlib_malloc(sizeof(c_derived_tbl)))==NULL
	|| (jpg->e_aclumtable=(c_derived_tbl *)mlib_malloc(sizeof(c_derived_tbl)))==NULL
	|| (jpg->e_dcchromtable=(c_derived_tbl *)mlib_malloc(sizeof(c_derived_tbl)))==NULL
	|| (jpg->e_acchromtable=(c_derived_tbl *)mlib_malloc(sizeof(c_derived_tbl)))==NULL)
		_throw("Memory allocation failure");

	if((v=mlib_version())!=NULL)
	{
		char *ptr=NULL;
		if((ptr=strrchr(v, ':'))!=NULL && strlen(ptr)>1)
		{
			ptr++;
			if(strlen(ptr)>6 && !strncmp(ptr, "v8plus", 6) && ptr[6]>='a'
				&& ptr[6]<='z') jpg->isvis=1;
			else if(strlen(ptr)>2 && !strncmp(ptr, "v9", 2) && ptr[2]>='a'
				&& ptr[2]<='z') jpg->isvis=1;
		}
	}

	jpg->initc=1;
	return (tjhandle)jpg;

	bailout:
	if(jpg) {jpg->initc=1;  tjDestroy(jpg);}
	return NULL;
}

DLLEXPORT unsigned long DLLCALL TJBUFSIZE(int width, int height)
{
	// This allows enough room in case the image doesn't compress
	return ((width+15)&(~15)) * ((height+15)&(~15)) * 3 + 2048;
}

DLLEXPORT int DLLCALL tjCompress(tjhandle h,
	unsigned char *srcbuf, int width, int pitch, int height, int ps,
	unsigned char *dstbuf, unsigned long *size,
	int jpegsub, int qual, int flags)
{
	checkhandle(h);

	if(srcbuf==NULL || width<=0 || height<=0
		|| dstbuf==NULL || size==NULL
		|| jpegsub<0 || jpegsub>=NUMSUBOPT || qual<0 || qual>100)
		_throw("Invalid argument in tjCompress()");
	if(qual<1) qual=1;
	if(!jpg->initc) _throw("Instance has not been initialized for compression");

	if(ps!=3 && ps!=4) _throw("This JPEG codec supports only 24-bit or 32-bit true color");

	if(pitch==0) pitch=width*ps;

	jpg->bmpbuf=srcbuf;

	jpg->jpgbuf=dstbuf;
	jpg->width=width;  jpg->height=height;  jpg->pitch=pitch;
	jpg->ps=ps;  jpg->subsamp=jpegsub;  jpg->qual=qual;  jpg->flags=flags;
	jpg->bytesleft=TJBUFSIZE(width, height);

	_catch(encode_jpeg(jpg));

	*size=jpg->bytesprocessed;
	return 0;

	bailout:
	return -1;
}


//////////////////////////////////////////////////////////////////////////////
//   DECOMPRESSOR
//////////////////////////////////////////////////////////////////////////////

static int find_marker(jpgstruct *jpg, unsigned char *marker)
{
	unsigned char b;
	while(1)
	{
		do {read_byte(jpg, b);} while(b!=0xff);
		read_byte(jpg, b);
		if(b!=0 && b!=0xff) {*marker=b;  return 0;}
		else {jpg->jpgptr--;  jpg->bytesleft++;  jpg->bytesprocessed--;}
	}

	bailout:
	return -1;
}

#define check_byte(j, b) {unsigned char __b;  read_byte(j, __b);  \
	if(__b!=(b)) _throw("JPEG bitstream error");}

static int d_postconvertline(mlib_u8 *linebuf, mlib_u8 *dstbuf, jpgstruct *jpg)
{
	// Convert RGB to BGR
	if(jpg->flags&TJ_BGR && jpg->ps==3)
	{
		_mlib(mlib_VectorReverseByteOrder(dstbuf, linebuf, jpg->width, 3));
	}
	// Convert RGB to ABGR
	else if(jpg->flags&TJ_BGR && jpg->flags&TJ_ALPHAFIRST && jpg->ps==4)
		mlib_VideoColorRGBint_to_ABGRint((mlib_u32 *)dstbuf, linebuf, NULL, 0, jpg->width, 1,
			jpg->pitch, jpg->width*3, 0);
	// Convert RGB to ARGB
	else if(!(jpg->flags&TJ_BGR) && jpg->flags&TJ_ALPHAFIRST && jpg->ps==4)
		mlib_VideoColorRGBint_to_ARGBint((mlib_u32 *)dstbuf, linebuf, NULL, 0, jpg->width, 1,
			jpg->pitch, jpg->width*3, 0);
	// Convert RGB to BGRA
	else if(jpg->flags&TJ_BGR && !(jpg->flags&TJ_ALPHAFIRST) && jpg->ps==4)
	{
		mlib_VideoColorRGBint_to_ARGBint((mlib_u32 *)dstbuf, linebuf, NULL, 0, jpg->width, 1,
			jpg->pitch, jpg->width*3, 0);
		_mlib(mlib_VectorReverseByteOrder_Inp(dstbuf, jpg->width, 4));
	}
	// Convert RGB to RGBA
	else if(!(jpg->flags&TJ_BGR) && !(jpg->flags&TJ_ALPHAFIRST) && jpg->ps==4)
	{
		mlib_VideoColorRGBint_to_ABGRint((mlib_u32 *)dstbuf, linebuf, NULL, 0, jpg->width, 1,
			jpg->pitch, jpg->width*3, 0);
		_mlib(mlib_VectorReverseByteOrder_Inp(dstbuf, jpg->width, 4));
	}
	return 0;

	bailout:
	return -1;
}

static int d_mcu_color_convert(jpgstruct *jpg, mlib_u8 *ybuf, int yw, mlib_u8 *cbbuf,
	mlib_u8 *crbuf, int cw, int startline, mlib_u8 *linebuf, mlib_u8 *linebuf2)
{
	int rgbstride=jpg->pitch;
	mlib_status (DLLCALL *ccfct)(mlib_u8 *, const mlib_u8 *, const mlib_u8 *,
		const mlib_u8 *, mlib_s32)=NULL;
	mlib_status (DLLCALL *ccfct420)(mlib_u8 *, mlib_u8 *, const mlib_u8 *,
		const mlib_u8 *, const mlib_u8 *, const mlib_u8 *, mlib_s32)=NULL;
	int mcuh=_mcuh[jpg->subsamp], h, j, convreq=0;

	h=mcuh;
	if(startline+mcuh>jpg->height) h=jpg->height-startline;

	switch(jpg->subsamp)
	{
		case TJ_411:
			ccfct420=mlib_VideoColorJFIFYCC2RGB420_Nearest;
			if(jpg->flags&TJ_BGR || jpg->ps!=3) convreq=1;
			break;
		case TJ_422:
			ccfct=mlib_VideoColorJFIFYCC2RGB422_Nearest;
			if(jpg->flags&TJ_BGR || jpg->ps!=3) convreq=1;
			break;
		case TJ_444:
			ccfct=mlib_VideoColorJFIFYCC2RGB444;
			if(!(jpg->flags&TJ_BGR) && jpg->flags&TJ_ALPHAFIRST && jpg->ps==4)  // ARGB
				ccfct=mlib_VideoColorJFIFYCC2ARGB444;
			else if(jpg->flags&TJ_BGR && jpg->flags&TJ_ALPHAFIRST && jpg->ps==4)  // ABGR
				ccfct=mlib_VideoColorJFIFYCC2ABGR444;
			else if(jpg->flags&TJ_BGR || jpg->ps!=3) convreq=1;
			break;
		default:
			_throw("Invalid argument to d_mcu_color_convert()");
	}

	// Do color conversion
	rgbstride=jpg->pitch;
	if(jpg->flags&TJ_BOTTOMUP) rgbstride=-jpg->pitch;

	if(jpg->subsamp==TJ_411)
	{
		mlib_u8 *y=ybuf, *cb=cbbuf, *cr=crbuf, *tmpptr, *tmpptr2;
		for(j=0; j<h; j+=2, jpg->bmpptr+=rgbstride*2, y+=yw*2, cb+=cw, cr+=cw)
		{
			tmpptr=jpg->bmpptr;  tmpptr2=jpg->bmpptr+rgbstride;
			if(convreq || ((long)tmpptr&7L)!=0L) tmpptr=linebuf;
			if(convreq || ((long)tmpptr2&7L)!=0L || j>=h-1) tmpptr2=linebuf2;
			_mlib(ccfct420(tmpptr, tmpptr2, y, y+yw, cb, cr, jpg->width));
			if(tmpptr!=jpg->bmpptr)
			{
				if(convreq) {_catch(d_postconvertline(tmpptr, jpg->bmpptr, jpg));}
				else {_mlib(mlib_VectorCopy_U8(jpg->bmpptr, tmpptr, jpg->width*jpg->ps));}
			}
			if(j<h-1 && tmpptr2!=jpg->bmpptr+rgbstride)
			{
				if(convreq) {_catch(d_postconvertline(tmpptr2, jpg->bmpptr+rgbstride, jpg));}
				else {_mlib(mlib_VectorCopy_U8(jpg->bmpptr+rgbstride, tmpptr2, jpg->width*jpg->ps));}
			}
		}
	}
	else
	{
		mlib_u8 *y=ybuf, *cb=cbbuf, *cr=crbuf, *tmpptr;
		for(j=0; j<h; j++, jpg->bmpptr+=rgbstride, y+=yw, cb+=cw, cr+=cw)
		{
			tmpptr=jpg->bmpptr;
			if(convreq || ((long)tmpptr&7L)!=0L) tmpptr=linebuf;
			_mlib(ccfct(tmpptr, y, cb, cr, jpg->width));
			if(tmpptr!=jpg->bmpptr)
			{
				if(convreq) {_catch(d_postconvertline(tmpptr, jpg->bmpptr, jpg));}
				else {_mlib(mlib_VectorCopy_U8(jpg->bmpptr, tmpptr, jpg->width*jpg->ps));}
			}
		}
	}

	return 0;

	bailout:
	return -1;
}

static int decode_jpeg_init(jpgstruct *jpg)
{
	mlib_s16 rawqtable[64];  mlib_u8 rawhuffbits[16], rawhuffvalues[256];
	int i;  unsigned char tempbyte, tempbyte2, marker;  unsigned short tempword, length;
	int markerread=0;  unsigned char compid[3];

	jpg->bytesprocessed=0;
	jpg->huffbits = jpg->huffbuf = jpg->unread_marker = jpg->insufficient_data = 0;
	if(jpg->flags&TJ_BOTTOMUP) jpg->bmpptr=&jpg->bmpbuf[jpg->pitch*(jpg->height-1)];
	else jpg->bmpptr=jpg->bmpbuf;
	jpg->jpgptr=jpg->jpgbuf;

	check_byte(jpg, 0xff);  check_byte(jpg, 0xd8);  // SOI

	while(1)
	{
		_catch(find_marker(jpg, &marker));

		switch(marker)
		{
			case 0xe0:  // JFIF
				read_word(jpg, length);
				if(length<8) _throw("JPEG bitstream error");
				check_byte(jpg, 'J');  check_byte(jpg, 'F');
				check_byte(jpg, 'I');  check_byte(jpg, 'F');  check_byte(jpg, 0);
				for(i=7; i<length; i++) read_byte(jpg, tempbyte) // We don't care about the rest
				markerread+=1;
				break;
			case 0xdb:  // DQT
			{
				int dqtbytecount;
				read_word(jpg, length);
				if(length<67 || length>(64+1)*2+2) _throw("JPEG bitstream error");
				dqtbytecount=2;
				while(dqtbytecount<length)
				{
					read_byte(jpg, tempbyte);  // Quant. table index
					dqtbytecount++;
					for(i=0; i<64; i++)
					{
						read_byte(jpg, tempbyte2);
						rawqtable[jpeg_natural_order[i]]=(mlib_s16)tempbyte2;
					}
					dqtbytecount+=64;

					if(tempbyte==0)
					{
						_mlib(mlib_VideoDeQuantizeInit_S16(jpg->lumqtable, rawqtable));
						markerread+=2;
					}
					else if(tempbyte==1)
					{
						_mlib(mlib_VideoDeQuantizeInit_S16(jpg->chromqtable, rawqtable));
						markerread+=4;
					}
				}
				break;
			}
			case 0xc4:  // DHT
			{
				int nval, dhtbytecount;
				read_word(jpg, length);
				if(length<19 || length>(17+256)*4+2) _throw("JPEG bitstream error");
				dhtbytecount=2;
				while(dhtbytecount<length)
				{
					read_byte(jpg, tempbyte);  // Huffman class
					dhtbytecount++;
					memset(rawhuffbits, 0, 16);
					memset(rawhuffvalues, 0, 256);
					nval=0;
					for(i=0; i<16; i++) {read_byte(jpg, rawhuffbits[i]);  nval+=rawhuffbits[i];}
					dhtbytecount+=16;
					if(nval>256) _throw("JPEG bitstream error");
					for(i=0; i<nval; i++) read_byte(jpg, rawhuffvalues[i]);
					dhtbytecount+=nval;
					if(tempbyte==0x00)  // DC luminance
					{
						_catch(jpeg_make_d_derived_tbl(rawhuffbits, rawhuffvalues, 1,
							jpg->d_dclumtable));
						markerread+=8;
					}
					else if(tempbyte==0x10)  // AC luminance
					{
						_catch(jpeg_make_d_derived_tbl(rawhuffbits, rawhuffvalues, 0,
							jpg->d_aclumtable));
						markerread+=16;
					}
					else if(tempbyte==0x01)  // DC chrominance
					{
						_catch(jpeg_make_d_derived_tbl(rawhuffbits, rawhuffvalues, 1,
							jpg->d_dcchromtable));
						markerread+=32;
					}
					else if(tempbyte==0x11)  // AC chrominance
					{
						_catch(jpeg_make_d_derived_tbl(rawhuffbits, rawhuffvalues, 0,
							jpg->d_acchromtable));
						markerread+=64;
					}
				}
				break;
			}
			case 0xc0:  // SOF
				read_word(jpg, length);
				if(length<11) _throw("JPEG bitstream error");
				read_byte(jpg, tempbyte);  // precision
				if(tempbyte!=8) _throw("Only 8-bit-per-component JPEGs are supported");
				read_word(jpg, tempword);  // height
				if(!jpg->height) jpg->height=tempword;
				if(tempword!=jpg->height) _throw("Height mismatch between JPEG and bitmap");
				read_word(jpg, tempword);  // width
				if(!jpg->width) jpg->width=tempword;
				if(tempword!=jpg->width) _throw("Width mismatch between JPEG and bitmap");
				read_byte(jpg, tempbyte);  // Number of components
				if(tempbyte!=3 || length<17) _throw("Only YCbCr JPEG's are supported");
				read_byte(jpg, compid[0]);  // Component ID
				read_byte(jpg, tempbyte);  // Horiz. and Vert. sampling factors
				if(tempbyte==0x11) jpg->subsamp=TJ_444;
				else if(tempbyte==0x21) jpg->subsamp=TJ_422;
				else if(tempbyte==0x22) jpg->subsamp=TJ_411;
				else _throw("Unsupported subsampling type");
				check_byte(jpg, 0);  // Luminance
				for(i=1; i<3; i++)
				{
					read_byte(jpg, compid[i]);  // Component ID
					check_byte(jpg, 0x11);  // Sampling factors
					check_byte(jpg, 1);  // Chrominance
				}
				markerread+=128;
				break;
			case 0xda:  // SOS
				if(markerread!=255) _throw("JPEG bitstream error");
				read_word(jpg, length);
				if(length<12) _throw("JPEG bitstream error");
				check_byte(jpg, 3);  // Number of components
				for(i=0; i<3; i++)
				{
					check_byte(jpg, compid[i])
					check_byte(jpg, i==0? 0 : 0x11);  // Huffman table selector
				}
				for(i=0; i<3; i++) read_byte(jpg, tempbyte);
				goto done;
		}
	}
	done:
	return 0;

	bailout:
	return -1;
}

static int decode_jpeg(jpgstruct *jpg)
{
	int i, j, k, mcuw, mcuh, x, y;
	int lastdc[3]={0, 0, 0};
	mlib_u8 *ybuf=NULL, *cbbuf, *crbuf, *linebuf=NULL;
	int yw, cw;

	_catch(decode_jpeg_init(jpg));

	mcuw=_mcuw[jpg->subsamp];  mcuh=_mcuh[jpg->subsamp];
	yw=(jpg->width+mcuw-1)&(~(mcuw-1));
	cw=yw*8/mcuw;

	_mlibn(ybuf=(mlib_u8 *)mlib_malloc(yw*mcuh + cw*8*2));
	_mlibn(linebuf=(mlib_u8 *)mlib_malloc(yw*4*2));
	cbbuf=&ybuf[yw*mcuh];  crbuf=&ybuf[yw*mcuh+cw*8];

	for(j=0; j<jpg->height; j+=mcuh)
	{
		
		for(i=0; i<yw; i+=mcuw)
		{
			k=0;    // luminance blocks
			for(y=0; y<mcuh; y+=8)
				for(x=0; x<mcuw; x+=8)
				{
					_catch( decode_one_block(jpg, &jpg->mcubuf[k], &lastdc[0],
						jpg->d_dclumtable, jpg->d_aclumtable) );
					_mlib(mlib_VideoDeQuantize_S16(&jpg->mcubuf[k], jpg->lumqtable));
					jpg->mcubuf[k]+=1024;
					_mlib(mlib_VideoIDCT8x8_U8_S16(&ybuf[i+y*yw+x], &jpg->mcubuf[k], yw));
					k+=64;
				}

			// Cb block
			_catch( decode_one_block(jpg, &jpg->mcubuf[k], &lastdc[1],
				jpg->d_dcchromtable, jpg->d_acchromtable) );
			_mlib(mlib_VideoDeQuantize_S16(&jpg->mcubuf[k], jpg->chromqtable));
			jpg->mcubuf[k]+=1024;
			_mlib(mlib_VideoIDCT8x8_U8_S16(&cbbuf[i*8/mcuw], &jpg->mcubuf[k], cw));
			k+=64;

			// Cr block
			_catch( decode_one_block(jpg, &jpg->mcubuf[k], &lastdc[2],
				jpg->d_dcchromtable, jpg->d_acchromtable) );
			_mlib(mlib_VideoDeQuantize_S16(&jpg->mcubuf[k], jpg->chromqtable));
			jpg->mcubuf[k]+=1024;
			_mlib(mlib_VideoIDCT8x8_U8_S16(&crbuf[i*8/mcuw], &jpg->mcubuf[k], cw));
		}
		_catch(d_mcu_color_convert(jpg, ybuf, yw, cbbuf, crbuf, cw, j, linebuf, &linebuf[yw*4]));
	}

	if(ybuf) mlib_free(ybuf);
	if(linebuf) mlib_free(linebuf);
	return 0;

	bailout:
	if(ybuf) mlib_free(ybuf);
	if(linebuf) mlib_free(linebuf);
	return -1;
}

DLLEXPORT tjhandle DLLCALL tjInitDecompress(void)
{
	jpgstruct *jpg=NULL;

	if((jpg=(jpgstruct *)mlib_malloc(sizeof(jpgstruct)))==NULL)
		_throw("Memory allocation failure");
	memset(jpg, 0, sizeof(jpgstruct));

	jpg->mcubuf=(mlib_s16 *)jpg->_mcubuf;

	if((jpg->d_dclumtable=(d_derived_tbl *)mlib_malloc(sizeof(d_derived_tbl)))==NULL
	|| (jpg->d_aclumtable=(d_derived_tbl *)mlib_malloc(sizeof(d_derived_tbl)))==NULL
	|| (jpg->d_dcchromtable=(d_derived_tbl *)mlib_malloc(sizeof(d_derived_tbl)))==NULL
	|| (jpg->d_acchromtable=(d_derived_tbl *)mlib_malloc(sizeof(d_derived_tbl)))==NULL)
		_throw("Memory allocation failure");

	jpg->initd=1;
	return (tjhandle)jpg;

	bailout:
	if(jpg) {jpg->initd=1;  tjDestroy(jpg);}
	return NULL;
}

DLLEXPORT int DLLCALL tjDecompressHeader(tjhandle h,
	unsigned char *srcbuf, unsigned long size,
	int *width, int *height)
{
	checkhandle(h);

	if(srcbuf==NULL || size<=0 || width==NULL || height==NULL)
		_throw("Invalid argument in tjDecompressHeader()");
	if(!jpg->initd) _throw("Instance has not been initialized for decompression");

	jpg->jpgbuf=srcbuf;
	jpg->bytesleft=size;

	jpg->width=jpg->height=0;

	_catch(decode_jpeg_init(jpg));
	*width=jpg->width;  *height=jpg->height;

	if(*width<1 || *height<1) _throw("Invalid data returned in header");
	return 0;

	bailout:
	return -1;
}

DLLEXPORT int DLLCALL tjDecompress(tjhandle h,
	unsigned char *srcbuf, unsigned long size,
	unsigned char *dstbuf, int width, int pitch, int height, int ps,
	int flags)
{
	checkhandle(h);

	if(srcbuf==NULL || size<=0
		|| dstbuf==NULL || width<=0 || height<=0)
		_throw("Invalid argument in tjDecompress()");
	if(!jpg->initd) _throw("Instance has not been initialized for decompression");

	if(ps!=3 && ps!=4) _throw("This JPEG codec supports only 24-bit or 32-bit true color");

	if(pitch==0) pitch=width*ps;

	jpg->bmpbuf=dstbuf;

	jpg->jpgbuf=srcbuf;
	jpg->width=width;  jpg->height=height;  jpg->pitch=pitch;
	jpg->ps=ps;  jpg->flags=flags;
	jpg->bytesleft=size;

	_catch(decode_jpeg(jpg));

	return 0;

	bailout:
	return -1;
}


// General

DLLEXPORT char* DLLCALL tjGetErrorStr(void)
{
	return (char *)lasterror;
}

DLLEXPORT int DLLCALL tjDestroy(tjhandle h)
{
	checkhandle(h);

	if(jpg->initc)
	{
		if(jpg->e_dclumtable) mlib_free(jpg->e_dclumtable);
		if(jpg->e_aclumtable) mlib_free(jpg->e_aclumtable);
		if(jpg->e_dcchromtable) mlib_free(jpg->e_dcchromtable);
		if(jpg->e_acchromtable) mlib_free(jpg->e_acchromtable);
	}

	if(jpg->initd)
	{
		if(jpg->d_dclumtable) mlib_free(jpg->d_dclumtable);
		if(jpg->d_aclumtable) mlib_free(jpg->d_aclumtable);
		if(jpg->d_dcchromtable) mlib_free(jpg->d_dcchromtable);
		if(jpg->d_acchromtable) mlib_free(jpg->d_acchromtable);
	}

	free(jpg);
	return 0;
}
