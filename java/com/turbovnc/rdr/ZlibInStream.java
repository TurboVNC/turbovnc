/* Copyright (C) 2012, 2018, 2025 D. R. Commander.  All Rights Reserved.
 * Copyright (C) 2011 Brian P. Hinz
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

//
// A ZlibInStream reads from a zlib.io.InputStream
//

package com.turbovnc.rdr;

import com.jcraft.jsch.jzlib.*;

public class ZlibInStream extends InStream {

  static final int DEFAULT_BUF_SIZE = 16384;

  public ZlibInStream(int bufSize_) {
    bufSize = bufSize_;
    b = new byte[bufSize];
    bytesIn = offset = 0;
    zs = new ZStream();
    zs.next_in = null;
    zs.next_in_index = 0;
    zs.avail_in = 0;
    if (zs.inflateInit() != JZlib.Z_OK) {
      zs = null;
      throw new ErrorException("ZlibInStream: inflateInit failed");
    }
    ptr = end = start = 0;
  }

  public ZlibInStream() { this(DEFAULT_BUF_SIZE); }

  public void close() {
    b = null;
    zs.inflateEnd();
  }

  public void setUnderlying(InStream is, int bytesIn_) {
    underlying = is;
    bytesIn = bytesIn_;
    ptr = end = start;
  }

  public int pos() {
    return offset + ptr - start;
  }

  public void reset() {
    ptr = end = start;
    if (underlying == null) return;

    while (bytesIn > 0) {
      decompress(true);
      end = start;  // throw away any data
    }
    underlying = null;
  }

  protected int overrun(int itemSize, int nItems, boolean wait) {
    if (itemSize > bufSize)
      throw new ErrorException("ZlibInStream overrun: max itemSize exceeded");
    if (underlying == null)
      throw new ErrorException("ZlibInStream overrun: no underlying stream");

    if (end - ptr != 0)
      System.arraycopy(b, ptr, b, start, end - ptr);

    offset += ptr - start;
    end -= ptr - start;
    ptr = start;

    while (end - ptr < itemSize) {
      if (!decompress(wait))
        return 0;
    }

    if (itemSize * nItems > end - ptr)
      nItems = (end - ptr) / itemSize;

    return nItems;
  }

  // decompress() calls the decompressor once.  Note that this won't
  // necessarily generate any output data - it may just consume some input
  // data.  Returns false if wait is false and we would block on the underlying
  // stream.

  private boolean decompress(boolean wait) {
    if (zs.inflateFinished())
      throw new ErrorException("ZlibInStream: unexpected end of zlib stream");

    zs.next_out = b;
    zs.next_out_index = end;
    zs.avail_out = start + bufSize - end;

    int n = underlying.check(1, 1, wait);
    if (n == 0) return false;
    zs.next_in = underlying.getbuf();
    zs.next_in_index = underlying.getptr();
    zs.avail_in = underlying.getend() - underlying.getptr();
    if (zs.avail_in > bytesIn)
      zs.avail_in = bytesIn;

    int rc = zs.inflate(JZlib.Z_SYNC_FLUSH);
    if (rc != JZlib.Z_OK && rc != JZlib.Z_STREAM_END) {
      throw new ErrorException("ZlibInStream: inflate failed");
    }

    bytesIn -= zs.next_in_index - underlying.getptr();
    end = zs.next_out_index;
    underlying.setptr(zs.next_in_index);
    return true;
  }

  private InStream underlying;
  private int bufSize;
  private int offset;
  private com.jcraft.jsch.jzlib.ZStream zs;
  private int bytesIn;
  private int start;
}
