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

/***************************************************************/

#define JPEG_DCTSIZE2 64
#define JPEG_MAX_HUFF_TABLE 256

typedef struct {
  mlib_s32 length;
  mlib_u8  huffsize[JPEG_MAX_HUFF_TABLE];
  mlib_s32 huffcode[JPEG_MAX_HUFF_TABLE];
} jpeg_huff_encoder;

/***************************************************************/

/* JPEG luminance and chrominance quantization tables */

static mlib_u8 jpeg_first_bit_table[65536];
int jpeg_first_bit_table_init=0;

/***************************************************************/

mlib_status jpeg_EncoderHuffmanCreateTable(void    **hufftable,
                                           mlib_u8 *length,
                                           mlib_u8 *huffval) {
  mlib_s32 i, j, k, code;
  jpeg_huff_encoder *ht = malloc(sizeof(jpeg_huff_encoder));

  if (!ht) return MLIB_FAILURE;

  k = 0; code = 0;

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
  for(i = 1; i < 17; i++, code += code) {
    mlib_s32 l = length[i];

    if (l < 0 || l+k > 255) {
      free(ht);
      return MLIB_FAILURE;
    }
    for(j = 0; j < l; j++, k++) {
      ht->huffsize[huffval[k]]  = i;
      ht->huffcode[huffval[k]]  = code++;
    }
  }

  ht->length      = k;
  *hufftable      = ht;

  if(!jpeg_first_bit_table_init) {
    for(i = 0; i < 65536; i++) {
      if(i & 0x8000) jpeg_first_bit_table[i] = 16;
      else if(i & 0x4000) jpeg_first_bit_table[i] = 15;
      else if(i & 0x2000) jpeg_first_bit_table[i] = 14;
      else if(i & 0x1000) jpeg_first_bit_table[i] = 13;
      else if(i & 0x800) jpeg_first_bit_table[i] = 12;
      else if(i & 0x400) jpeg_first_bit_table[i] = 11;
      else if(i & 0x200) jpeg_first_bit_table[i] = 10;
      else if(i & 0x100) jpeg_first_bit_table[i] = 9;
      else if(i & 0x80) jpeg_first_bit_table[i] = 8;
      else if(i & 0x40) jpeg_first_bit_table[i] = 7;
      else if(i & 0x20) jpeg_first_bit_table[i] = 6;
      else if(i & 0x10) jpeg_first_bit_table[i] = 5;
      else if(i & 0x8) jpeg_first_bit_table[i] = 4;
      else if(i & 0x4) jpeg_first_bit_table[i] = 3;
      else if(i & 0x2) jpeg_first_bit_table[i] = 2;
      else if(i & 0x1) jpeg_first_bit_table[i] = 1;
      else jpeg_first_bit_table[i] = 0;
    }
    jpeg_first_bit_table_init = 1;
  }

  return MLIB_SUCCESS;
}

/***************************************************************/

void jpeg_EncoderHuffmanSetBuffer(jpeg_encoder  *encoder,
                                  mlib_u8       *buffer) {
  encoder->buffer     = buffer;
  encoder->position   = 0;
}

/***************************************************************/

mlib_status jpeg_EncoderHuffmanInit(jpeg_encoder *enc) {
  if (!enc) return MLIB_FAILURE;

  enc->buffer     = NULL;
  enc->position   = 0;
  enc->value      = 0;
  enc->bits       = 0;

  return MLIB_SUCCESS;
}

/***************************************************************/

#define DUMP_BITS_(code, size) {                                \
  bits += size;                                                 \
  value = (value << size) | code;                               \
  if (bits > 7)                                                 \
    while(bits > 7)                                             \
      if (0xFF == (buffer[position++] =  value >> (bits -= 8))) \
        buffer[position++] = 0;                                 \
 }

/***************************************************************/

#define DUMP_BITS(code, size) {                                 \
  bits += size;                                                 \
  value = (value << size) | code;                               \
  if (bits > 15) {                                              \
    if (0xFF == (buffer[position++] =  value >> (bits -= 8)))   \
      buffer[position++] = 0;                                   \
    if (0xFF == (buffer[position++] =  value >> (bits -= 8)))   \
      buffer[position++] = 0;                                   \
  }                                                             \
 }

/***************************************************************/

#define DUMP_SINGLE_VALUE(ht, codevalue) { \
  size = ht->huffsize[codevalue];          \
  code = ht->huffcode[codevalue];          \
                                           \
  DUMP_BITS(code, size)                    \
 }

/***************************************************************/

#define DUMP_VALUE(ht, codevalue, t, nbits) { \
  size = ht->huffsize[codevalue];             \
  code = ht->huffcode[codevalue];             \
  t &= ~(-1 << nbits);                        \
  DUMP_BITS(code, size)                       \
  DUMP_BITS(t, nbits)                         \
 }

/***************************************************************/

#define FIX_OF_PUTTING          \
  encoder->position = position; \
  encoder->value    = value;    \
  encoder->bits     = bits;     \

/***************************************************************/

mlib_status jpeg_EncoderHuffmanDumpBlock(jpeg_encoder  *encoder,
                                         mlib_s16      *coeffs,
                                         void          *huffdctable,
                                         void          *huffactable) {
  mlib_u8  *buffer  = encoder->buffer;
  mlib_s32 position = encoder->position;
  mlib_s32 value    = encoder->value;
  mlib_s32 bits     = encoder->bits;
  jpeg_huff_encoder *hdc = huffdctable;
  jpeg_huff_encoder *hac = huffactable;
  mlib_s32 t, t1, sflag, nbits, r, size, code;
  mlib_s32 code_0xf0 = hac->huffcode[0xf0], size_0xf0 = hac->huffsize[0xf0];

  t     = (t1 = coeffs[0]);
  sflag = t >> 31;
  t    -= ((t+t) & sflag);
  t1   += sflag;
  CALC_FIRST_BIT(nbits, t)
  DUMP_VALUE(hdc, nbits, t1, nbits)
  r     = 0;

#define innerloop(order) {  \
  t1  = *(mlib_u16*)((mlib_u8*)coeffs + order);  \
  if(t1 == 0) r++;  \
  else {  \
    t     = (mlib_s16)t1;  \
    sflag = t >> 31;  \
    t     = (t ^ sflag) - sflag;  \
    t1   += sflag;  \
    nbits = jpeg_first_bit_table[t];  \
    for(; r > 15; r -= 16) DUMP_BITS(code_0xf0, size_0xf0)  \
    sflag = (r << 4) + nbits;  \
    DUMP_VALUE(hac, sflag, t1, nbits)  \
    r = 0;  \
  }}

  innerloop(2*1);   innerloop(2*8);   innerloop(2*16);  innerloop(2*9);
  innerloop(2*2);   innerloop(2*3);   innerloop(2*10);  innerloop(2*17);
  innerloop(2*24);  innerloop(2*32);  innerloop(2*25);  innerloop(2*18);
  innerloop(2*11);  innerloop(2*4);   innerloop(2*5);   innerloop(2*12);
  innerloop(2*19);  innerloop(2*26);  innerloop(2*33);  innerloop(2*40);
  innerloop(2*48);  innerloop(2*41);  innerloop(2*34);  innerloop(2*27);
  innerloop(2*20);  innerloop(2*13);  innerloop(2*6);   innerloop(2*7);
  innerloop(2*14);  innerloop(2*21);  innerloop(2*28);  innerloop(2*35);
  innerloop(2*42);  innerloop(2*49);  innerloop(2*56);  innerloop(2*57);
  innerloop(2*50);  innerloop(2*43);  innerloop(2*36);  innerloop(2*29);
  innerloop(2*22);  innerloop(2*15);  innerloop(2*23);  innerloop(2*30);
  innerloop(2*37);  innerloop(2*44);  innerloop(2*51);  innerloop(2*58);
  innerloop(2*59);  innerloop(2*52);  innerloop(2*45);  innerloop(2*38);
  innerloop(2*31);  innerloop(2*39);  innerloop(2*46);  innerloop(2*53);
  innerloop(2*60);  innerloop(2*61);  innerloop(2*54);  innerloop(2*47);
  innerloop(2*55);  innerloop(2*62);  innerloop(2*63);

  if (r > 0) DUMP_SINGLE_VALUE(hac, 0x0)

  FIX_OF_PUTTING
  return MLIB_SUCCESS;
}

/***************************************************************/

mlib_status jpeg_EncoderHuffmanFlushBits(jpeg_encoder *encoder) {
  mlib_u8  *buffer  = encoder->buffer;
  mlib_s32 position = encoder->position;
  mlib_s32 value    = encoder->value;
  mlib_s32 bits     = encoder->bits;

  DUMP_BITS_(0x7F, 7)
  FIX_OF_PUTTING
  return MLIB_SUCCESS;
}

/***************************************************************/
