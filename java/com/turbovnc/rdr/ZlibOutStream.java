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

import java.util.zip.Deflater;

import com.turbovnc.rfb.LogWriter;

public class ZlibOutStream extends OutStream {

  static final int DEFAULT_BUF_SIZE = 16384;

  public ZlibOutStream(OutStream os, int bufSize_, int compressLevel)
  {
    underlying = os;
    bufSize = (bufSize_ != 0 ? bufSize_ : DEFAULT_BUF_SIZE);

    compressionLevel = newLevel = compressLevel;
    deflater = new Deflater(compressLevel);

    b = new byte[bufSize];
    ptr = start = 0;
    end = bufSize;
  }

  public ZlibOutStream() { this(null, 0, Deflater.DEFAULT_COMPRESSION); }

  public void close() {
    b = null;
    deflater.end();
  }

  public void setUnderlying(OutStream os)
  {
    underlying = os;
  }

  public void setCompressionLevel(int level)
  {
    if (level < Deflater.NO_COMPRESSION || level > Deflater.BEST_COMPRESSION)
      level = Deflater.DEFAULT_COMPRESSION;

    newLevel = level;
  }

  public int length() {
    return offset + ptr - start;
  }

  public void flush()
  {
    checkCompressionLevel();

    if (ptr - start > 0)
      deflater.setInput(b, start, ptr - start);

    // Force out everything from the zlib encoder
    deflate(Deflater.SYNC_FLUSH);

    offset = deflater.getTotalIn();
    ptr = start;
  }

  public int overrun(int itemSize, int nItems)
  {
    if (itemSize > bufSize)
      throw new ErrorException("ZlibOutStream overrun: max itemSize exceeded");

    checkCompressionLevel();

    while ((end - ptr) < itemSize) {

      deflater.setInput(b, start, ptr - start);
      deflate(Deflater.NO_FLUSH);

      // output buffer not full
      int consumed = deflater.getTotalIn() - offset;

      if (consumed == ptr - start) {
        ptr = start;
      } else {
        vlog.info("z out buf not full, but in data not consumed");
        System.arraycopy(b, start + consumed, b, start, ptr - start - consumed);
        ptr -= consumed;
      }
      offset += consumed;
    }

    int nAvail = (end - ptr) / itemSize;
    if (nAvail < nItems)
      return nAvail;

    return nItems;
  }

  void deflate(int flush)
  {
    int n, outbufptr, outbuflen;

    if (underlying == null)
      throw new ErrorException("ZlibOutStream: underlying OutStream has not been set");

    do {
      // According to zlib manual, the avail_out should be greater than 6 when
      // the flush marker begins. This is to avoid repeated flush markers if
      // deflate() needs to be called again when avail_out == 0.
      //
      // Therefore, we make sure that there is enough space in the output buffer.
      underlying.check(16);

      outbufptr = underlying.getptr();
      outbuflen = underlying.getend() - outbufptr;

      n = deflater.deflate(underlying.getbuf(), outbufptr, outbuflen, flush);
      underlying.setptr(outbufptr + n);
    } while (n == outbuflen);
  }

  public void checkCompressionLevel()
  {
    if (newLevel != compressionLevel) {
      deflater.setLevel(newLevel);

      // The Deflater.setLevel() method does not immediately invoke
      // deflateParam() to adjust the compression level. Instead, it sets a
      // flag to signal that the deflater’s parameters should be changed.
      //
      // The deflateParam() function in zlib will be called during subsequent
      // invocations of Deflater.deflate() until it returns Z_OK, indicating
      // the parameter update is successful.
      //
      // To ensure that the compression level change is fully applied, we need
      // to call deflate() here. This ensures that deflate(Deflater.SYNC_FLUSH)
      // operates correctly after the compression level adjustment.
      deflate(Deflater.NO_FLUSH);
      compressionLevel = newLevel;
    }
  }

  private OutStream underlying;
  private int compressionLevel;
  private int newLevel;
  private int bufSize;
  private int offset;
  private Deflater deflater;
  private int start;

  static LogWriter vlog = new LogWriter("ZlibOutStream");
}
