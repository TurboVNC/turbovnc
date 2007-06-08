/* Copyright (C)2007 Sun Microsystems, Inc.
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

#ifndef JPEG_IN_H
#define JPEG_IN_H

#include <mlib_image.h>
#include <mlib_video.h>

typedef struct {
  mlib_u8  *buffer;      /* pointer to user data                     */
  mlib_s32 n;            /* length of buffer                         */
  mlib_s32 position;     /* position of first unread byte            */
  mlib_s32 value;
  mlib_s32 bits;         /* bits left in value                       */
  mlib_s32 done;         /* number of processed elements in block    */
} jpeg_decoder;

typedef struct {
  mlib_u8  *buffer;      /* pointer to user data                     */
  mlib_s32 position;     /* position of next byte to write           */
  mlib_s32 value;        /* already created value                    */
  mlib_s32 bits;         /* bits left in value                       */
} jpeg_encoder;

/***************************************************************/

#define CALC_FIRST_BIT(nbits, t)                       \
  nbits = jpeg_first_bit_table[t&255];                 \
  if (t > 255) nbits = jpeg_first_bit_table[t>>8] + 8;

#endif	/* JPEG_IN_H */
