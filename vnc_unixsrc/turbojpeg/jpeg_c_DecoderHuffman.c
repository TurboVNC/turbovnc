/* Copyright (C)2007 Sun Microsystems, Inc.
 *
 * This library is free software and may be redistributed and/or modified under
 * the terms of the wxWindows Library License, Version 3.1 or (at your option)
 * any later version.  The full license is in the LICENSE.txt file included
 * with this distribution.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * wxWindows Library License for more details.
 */

/*
 * FUNCTIONS
 * SYNOPSIS
 * ARGUMENT
 */

#include "jpeg_IN.h"
#include <mlib_algebra.h>

/***************************************************************/

#define JPEG_MAX_HUFF_TABLE 256
#define JPEG_LOOKUP_TABLE 256

typedef struct {
  mlib_s32 length;
  mlib_u8  huffsize[JPEG_MAX_HUFF_TABLE];
  mlib_u8  huffvalue[JPEG_MAX_HUFF_TABLE];
  mlib_s32 huffcode[JPEG_MAX_HUFF_TABLE];
  mlib_s32 lookup[JPEG_LOOKUP_TABLE];
  mlib_s32 maxcode[18];
  mlib_s32 valoffset[18];
  mlib_s32 hufflengths[18];
} jpeg_huff_table;

/***************************************************************/

mlib_status jpeg_DecoderHuffmanCreateTable(void    **hufftable,
                                               mlib_u8 *length,
                                               mlib_u8 *huffval) {
  mlib_s32 i, j, k, code, oldmaxcode;
  mlib_s32 array_k[17];
  jpeg_huff_table *ht = malloc(sizeof(jpeg_huff_table));

  if (!ht) return MLIB_FAILURE;
  k = 0; code = 0; oldmaxcode = 0;
  ht->hufflengths[0] = 0;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
  for(i = 1; i < 17; i++, code += code) {
    mlib_s32 l = length[i];

    ht->hufflengths[i] = l;
    if (l < 0 || l+k > 255) {
      free(ht);
      return MLIB_FAILURE;
    }
    ht->valoffset[i] = k - code;

    for(j = 0; j < l; j++, k++) {
      ht->huffvalue[k] = huffval[k];
      ht->huffsize[k]  = i;
      ht->huffcode[k]  = code++;
    }

    array_k[i] = k;
    ht->maxcode[i] = code;
    if (l)
    if (code >= 1 << i) break;
  }

  for(j = 0; j < JPEG_LOOKUP_TABLE; j++) ht->lookup[j] = 0;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
  for(i = 1; i < 17; i++) {
    mlib_s32 l = length[i];
    code = ht->maxcode[i];
    k = array_k[i];

    if (l) {
      if (code >= 1 << i)  return MLIB_FAILURE;
      else
      if (i < 9) {
        mlib_s32 shift = 8 - i;
        mlib_s32 mincode = (code - l) << shift;
        mlib_s32 maxcode = code << shift;

        maxcode = (maxcode > 0xFF) ? 0xFF : maxcode;
        for(j = mincode; j <= maxcode; j++) {
          mlib_s32 indx = (j >> shift) - (mincode >> shift) + (k - l);
          ht->lookup[j] = (ht->huffsize[indx] << 8) | ht->huffvalue[indx];
        }
        oldmaxcode = maxcode;
      } else {
        mlib_s32 shift = i - 8;
        mlib_s32 mincode = (code - l) >> shift;
        mlib_s32 maxcode = (code - 1) >> shift;
        mlib_s32 start;

        maxcode = (maxcode > 0xFF) ? 0xFF : maxcode;
        start = (mincode > oldmaxcode) ? mincode : oldmaxcode;
        for(j = start; j <= maxcode; j++) {
          mlib_s32 indx = (j << shift) - code + k;
          ht->lookup[j] = (ht->huffsize[indx] << 8) | ht->huffvalue[indx];
        }
        oldmaxcode = maxcode + 1;
      }
    }
  }

  ht->maxcode[17]   = 0x20000;
  ht->valoffset[17] = -0x20000;
  ht->length      = k;
  *hufftable      = ht;

  return MLIB_SUCCESS;
}

/***************************************************************/

void jpeg_DecoderHuffmanGetTable(void     *hufftable,
                                 mlib_u8  *hufflengths,
                                 mlib_u8  *huffvalues) {
  jpeg_huff_table *ht = hufftable;
  mlib_s32 i, l = 0;

  for(i = 0; i < 17; i++) l += (hufflengths[i] = ht->hufflengths[i]);
  for(i = 0; i < l; i++) huffvalues[i] = ht->huffvalue[i];
}

/***************************************************************/

void jpeg_DecoderHuffmanSetBuffer(jpeg_decoder  *decoder,
                                  mlib_u8       *buffer,
                                  mlib_s32      n) {
  decoder->buffer   = buffer;
  decoder->n        = n;
  decoder->position = 0;
}

/***************************************************************/

mlib_status jpeg_DecoderHuffmanInit(jpeg_decoder *dec) {

  if (!dec) return MLIB_FAILURE;

  dec->buffer   = NULL;
  dec->n        = 0;
  dec->position = 0;
  dec->value    = 0;
  dec->bits     = 0;
  dec->done     = 0;

  return MLIB_SUCCESS;
}

/***************************************************************/

#define HUFF_EXTEND(x,s)  ((x) + ((((x) - (1<<((s)-1))) >> 31) & (((-1)<<(s)) + 1)))

#define RETURN(status) return MLIB_##status;

/***************************************************************/

#define FIX_OF_GETTING() {      \
  decoder->position = position; \
  decoder->value    = value;    \
  decoder->bits     = bits;     \
}

/***************************************************************/

#define ADD_BYTE  {                                     \
  mlib_s32 val0 = buffer[position++];                   \
  mlib_s32 val1 = buffer[position];                     \
                                                        \
  bits += 8;                                            \
  value = (value << 8) | (val0);                        \
  if (val0 == 0xFF) {                                   \
    position++;                                         \
    if (val1 != 0) {                                    \
      position   -= 2;                                  \
      value      &= ~0xFF;                              \
    }                                                   \
  }                                                     \
}

/***************************************************************/

#define ENSURE_SHORT  if (bits < 16) { ADD_BYTE ADD_BYTE }

/***************************************************************/

#define GET_SYMBOL(symbol, size, ht) {       \
  ENSURE_SHORT                               \
  symbol = value >> (bits - 8);              \
  /* to avoid crash on buggy bit streams */  \
  symbol = symbol & 0xFF;                    \
  symbol = ht->lookup[symbol];               \
  size = (mlib_u32)symbol >> 8;              \
  bits -= size;                              \
  symbol = symbol & 0xFF;                    \
  if (size > 8) {                            \
    symbol = value >> bits;                  \
    while (symbol >= ht->maxcode[size])      \
      symbol = value >> (--bits), ++size;    \
    symbol += ht->valoffset[size];           \
    symbol = ht->huffvalue[symbol];          \
  }                                          \
  value &= ~(-1 << bits);                    \
}

/***************************************************************/

#define GET_BITS(rest, size) {               \
  ENSURE_SHORT                               \
  bits -= size;                              \
  rest = value >> bits;                      \
  value &= ~(-1 << bits);                    \
}

/***************************************************************/

void jpeg_DecoderHuffmanDrawData(jpeg_decoder  *decoder,
                                 mlib_u8       *data,
                                 mlib_s32      stride,
                                 mlib_s16      *coeffs,
                                 mlib_s16      *dc,
                                 void          *huffdctable,
                                 void          *huffactable,
                                 mlib_s16      qtable[64]) {
  {
    jpeg_huff_table *hdc = huffdctable;
    jpeg_huff_table *hac = huffactable;
    mlib_u8  *buffer  = decoder->buffer;
    mlib_s32 position = decoder->position;
    mlib_u32 value    = decoder->value;
    mlib_s32 bits     = decoder->bits;
    mlib_s32 done     = 1;
    mlib_s32 symbol, l, r, i, j, o, quarterflag = 0;

    if (position >= decoder->n) {	/* premature end of data segment */
      mlib_u8 *dp, *dl;
      dp = dl = data;
      for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
      	  *dp++ = 128;	/* YUV = RGB = [128, 128, 128] */
	}
	dl += stride;
	dp = dl;
      }
      return;
    }

    GET_SYMBOL(symbol, l, hdc)
    GET_BITS(r, symbol)
    coeffs[0] = 1024 + qtable[0] * (*dc += HUFF_EXTEND(r, symbol));

  #ifdef __SUNPRO_C
  #pragma pipeloop(0)
  #endif /* __SUNPRO_C */
    while (done < 64) {
      GET_SYMBOL(symbol, l, hac)
      r       = symbol >> 4;
      symbol &= 15;
      if (symbol) {
        done += r;
        l    += symbol;
        GET_BITS(r, symbol)
        quarterflag |= (o = jpeg_natural_order[done]);
        coeffs[o] = qtable[o] * HUFF_EXTEND(r, symbol);
        done++;
      } else done += ((15 - r) << 6) + 16;
    }

    FIX_OF_GETTING()

    if (quarterflag & 0x24) {
      mlib_d64  *dcoeffs = (mlib_d64*)coeffs;
      mlib_VideoIDCT8x8_U8_S16(data, coeffs, stride);
      for(i = 0; i < 16; i++) dcoeffs[i] = 0.;
    } else if (quarterflag) {
      mlib_d64  *dcoeffs = (mlib_d64*)coeffs;
      mlib_VideoIDCT8x8_U8_S16_Q1(data, coeffs, stride);
      dcoeffs[0] = 0.;
      dcoeffs[2] = 0.;
      dcoeffs[4] = 0.;
      dcoeffs[6] = 0.;
    } else {
      mlib_VideoIDCT8x8_U8_S16_DC(data, coeffs, stride);
      coeffs[0] = 0;
    }
  }
}

/***************************************************************/
