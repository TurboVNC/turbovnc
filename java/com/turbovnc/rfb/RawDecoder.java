/* Copyright (C) 2015 D. R. Commander.  All Rights Reserved.
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

public class RawDecoder extends Decoder {

  public RawDecoder(CMsgReader reader_) { reader = reader_; }

  public void readRect(Rect r, CMsgHandler handler) {
    int[] stride = { r.width() };
    Object buf = handler.getRawPixelsRW(stride);

    reader.getInStream().readPixels(buf, stride[0], r, (reader.bpp() / 8),
                                    handler.cp.pf().bigEndian);
    handler.releaseRawPixels(r);
  }

  CMsgReader reader;
  static LogWriter vlog = new LogWriter("RawDecoder");
}
