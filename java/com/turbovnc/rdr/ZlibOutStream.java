/* Copyright (C) 2022 D. R. Commander.  All Rights Reserved.
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

package com.turbovnc.rdr;

import com.jcraft.jzlib.*;

import com.turbovnc.rfb.LogWriter;

public class ZlibOutStream extends OutStream {

  static final int DEFAULT_BUF_SIZE = 16384;

  public ZlibOutStream(OutStream os, int bufSize_, int compressLevel)
  {
    underlying = os;
    compressionLevel = newLevel = compressLevel;
    bufSize = (bufSize_ != 0 ? bufSize_ : DEFAULT_BUF_SIZE);

    zs = new ZStream();
    if (zs.deflateInit(compressLevel) != JZlib.Z_OK) {
      zs = null;
      throw new ErrorException("ZlibOutStream: deflateInit failed");
    }

    b = new byte[bufSize];
    ptr = start = 0;
    end = bufSize;
  }

  public ZlibOutStream() { this(null, 0, JZlib.Z_DEFAULT_COMPRESSION); }

  public void close() {
    b = null;
    zs.deflateEnd();
  }

  public void setUnderlying(OutStream os)
  {
    underlying = os;
  }

  public void setCompressionLevel(int level)
  {
    if (level < -1 || level > 9)
      level = JZlib.Z_DEFAULT_COMPRESSION;

    newLevel = level;
  }

  public int length() {
    return offset + ptr - start;
  }

  public void flush()
  {
    checkCompressionLevel();

    zs.next_in = b;
    zs.next_in_index = start;
    zs.avail_in = ptr - start;

    // Force out everything from the zlib encoder
    deflate(JZlib.Z_SYNC_FLUSH);

    offset += ptr - start;
    ptr = start;
  }

  public int overrun(int itemSize, int nItems)
  {
    if (itemSize > bufSize)
      throw new ErrorException("ZlibOutStream overrun: max itemSize exceeded");

    checkCompressionLevel();

    while ((end - ptr) < itemSize) {
      zs.next_in = b;
      zs.next_in_index = start;
      zs.avail_in = ptr - start;

      deflate(JZlib.Z_NO_FLUSH);

      // output buffer not full

      if (zs.avail_in == 0) {
        offset += ptr - start;
        ptr = start;
      } else {
        // but didn't consume all the data?  try shifting what's left to the
        // start of the buffer.
        vlog.info("z out buf not full, but in data not consumed");
        System.arraycopy(b, zs.next_in_index, b, start,
                         ptr - zs.next_in_index);
        offset += zs.next_in_index - start;
        ptr -= zs.next_in_index - start;
      }
    }

    int nAvail = (end - ptr) / itemSize;
    if (nAvail < nItems)
      return nAvail;

    return nItems;
  }

  void deflate(int flush)
  {
    int rc;

    if (underlying == null)
      throw new ErrorException("ZlibOutStream: underlying OutStream has not been set");

    if ((flush == JZlib.Z_NO_FLUSH) && (zs.avail_in == 0))
      return;

    do {
      underlying.check(1);
      zs.next_out = underlying.getbuf();
      zs.next_out_index = underlying.getptr();
      zs.avail_out = underlying.getend() - underlying.getptr();

      rc = zs.deflate(flush);
      if (rc < 0) {
        // zlib returns an error if you try to flush something twice
        if ((rc == JZlib.Z_BUF_ERROR) && (flush != JZlib.Z_NO_FLUSH))
          break;

        throw new ErrorException("ZlibOutStream: deflate failed");
      }

      underlying.setptr(zs.next_out_index);
    } while (zs.avail_out == 0);
  }

  public void checkCompressionLevel()
  {
    int rc;

    if (newLevel != compressionLevel) {
      // zlib performs an implicit flush when the parameters are changed, but
      // the flush it performs does not force out all of the data.  Since
      // multiple flushes cannot be performed, we cannot force out the data
      // after the parameters are changed.  Hence we must explicitly trigger a
      // proper flush here.
      zs.deflate(JZlib.Z_SYNC_FLUSH);

      rc = zs.deflateParams(newLevel, JZlib.Z_DEFAULT_STRATEGY);
      if (rc < 0) {
        // The implicit flush can result in this error, caused by the
        // explicit flush we triggered above.  It should be safe to ignore,
        // however, as the first flush should have left things in a stable
        // state.
        if (rc != JZlib.Z_BUF_ERROR)
          throw new ErrorException("ZlibOutStream: deflateParams failed");
      }

      compressionLevel = newLevel;
    }
  }

  private OutStream underlying;
  private int compressionLevel;
  private int newLevel;
  private int bufSize;
  private int offset;
  private com.jcraft.jzlib.ZStream zs;
  private int start;

  static LogWriter vlog = new LogWriter("ZlibOutStream");
}
