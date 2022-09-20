/* Copyright (C) 2012, 2018 D. R. Commander.  All Rights Reserved.
 * Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

package com.turbovnc.rfb;

public abstract class Decoder {

  public abstract void readRect(Rect r, CMsgHandler handler);

  public void reset() {}

  public void close() {}

  public static boolean supported(int encoding) {
    return (encoding == RFB.ENCODING_RAW ||
            encoding == RFB.ENCODING_HEXTILE ||
            encoding == RFB.ENCODING_TIGHT ||
            encoding == RFB.ENCODING_ZRLE);
  }

  public static Decoder createDecoder(int encoding, CMsgReader reader) {
    switch (encoding) {
      case RFB.ENCODING_RAW:      return new RawDecoder(reader);
      case RFB.ENCODING_HEXTILE:  return new HextileDecoder(reader);
      case RFB.ENCODING_TIGHT:    return new TightDecoder(reader);
      case RFB.ENCODING_ZRLE:     return new ZRLEDecoder(reader);
    }
    return null;
  }
}
