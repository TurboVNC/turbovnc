/* -*-mode:java; c-basic-offset:2; indent-tabs-mode:nil -*- */
/*
Copyright (c) 2002-2018 ymnk, JCraft,Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright 
     notice, this list of conditions and the following disclaimer in 
     the documentation and/or other materials provided with the distribution.

  3. The names of the authors may not be used to endorse or promote products
     derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL JCRAFT,
INC. OR ANY CONTRIBUTORS TO THIS SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

package com.jcraft.jsch.jcraft;
import com.jcraft.jsch.*;
import java.util.zip.Deflater;
import java.util.zip.Inflater;
import java.util.zip.DataFormatException;

public class Compression implements com.jcraft.jsch.Compression {
  static private final int BUF_SIZE=4096;
  private final int buffer_margin=32+20; // AES256 + HMACSHA1
  private int type;
  private Inflater inflater=new Inflater();
  private Deflater deflater=new Deflater();
  private byte[] tmpbuf=new byte[BUF_SIZE];

  public void init(int type, int level){
    if(type==DEFLATER){
      deflater = new Deflater(level);
      this.type=DEFLATER;
    }
    else if(type==INFLATER){
      inflater = new Inflater();
      this.type=INFLATER;
    }
  }

  public byte[] compress(byte[] buf, int start, int[] len){
    int tmpbuflen = 0;
    deflater.setInput(buf, start, len[0] - start);

    while (true) {
      tmpbuflen += deflater.deflate(tmpbuf, tmpbuflen, tmpbuf.length - tmpbuflen, Deflater.SYNC_FLUSH);
      if (tmpbuflen < tmpbuf.length)
        break;
      byte[] foo = new byte[tmpbuf.length * 2];
      System.arraycopy(tmpbuf, 0, foo, 0, tmpbuf.length);
      tmpbuf = foo;
    }
    if (buf.length < start + tmpbuflen + buffer_margin) {
      byte[] foo = new byte[start + tmpbuflen + buffer_margin];
      System.arraycopy(buf, 0, foo, 0, buf.length);
      buf = foo;
    }
    System.arraycopy(tmpbuf, 0, buf, start, tmpbuflen);
    len[0] = start + tmpbuflen;
    return buf;
  }

  public byte[] uncompress(byte[] buffer, int start, int[] length){
    int tmpbuflen = 0;
    inflater.setInput(buffer, start, length[0]);

    try {
      while (true) {
        tmpbuflen += inflater.inflate(tmpbuf, tmpbuflen, tmpbuf.length - tmpbuflen);
        if (tmpbuflen < tmpbuf.length)
          break;
        byte[] foo = new byte[tmpbuf.length * 2];
        System.arraycopy(tmpbuf, 0, foo, 0, tmpbuf.length);
        tmpbuf = foo;
      }
    } catch (DataFormatException e) {
      System.err.println("uncompress: invalid zlib stream.");
      return null;
    }
    if (inflater.finished()) {
      System.err.println("uncompress: unexpected end of zlib stream.");
      return null;
    }
    if (buffer.length < start + tmpbuflen) {
      byte[] foo = new byte[start + tmpbuflen];
      System.arraycopy(buffer, 0, foo, 0, start);
      buffer = foo;
    }
    System.arraycopy(tmpbuf, 0, buffer, start, tmpbuflen);
    length[0] = tmpbuflen;
    return buffer;
  }
}
