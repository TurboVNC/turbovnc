/* -*-mode:java; c-basic-offset:2; indent-tabs-mode:nil -*- */
/*
Copyright (c) 2011 ymnk, JCraft,Inc. All rights reserved.
Copyright (c) 2018, 2021 D. R. Commander. All rights reserved.

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

package com.jcraft.jsch.agentproxy.usocket;

import com.jcraft.jsch.agentproxy.AgentProxyException;
import com.jcraft.jsch.agentproxy.USocketFactory;

import java.io.IOException;
import java.util.Arrays;
import java.util.List;

public class JNIUSocketFactory implements USocketFactory {

  public JNIUSocketFactory() throws AgentProxyException {
  }

  public class MySocket extends Socket {
    private int sock;

    public int readFull(byte[] buf, int s, int len) throws IOException {
      byte[] _buf = buf;
      int _len = len;
      int _s = s;

      while(_len > 0){
        if(_s != 0){
          _buf = new byte[_len];
        }
        int i = readSocket(sock, _buf, _len);
        if(i <= 0)
          return -1;
        if(_s != 0)
          System.arraycopy(_buf, 0, buf, _s, i);
        _s += i;
        _len -= i;
      }
      return len;
    }

    public void write(byte[] buf, int s, int len) throws IOException {
      byte[] _buf = buf;
      int _len = len;
      if(s != 0){
        _buf = new byte[len];
        System.arraycopy(buf, s, _buf, 0, len);
      }
      writeSocket(sock, _buf, len);
    }

    MySocket(int sock) throws IOException {
      this.sock = sock;
    }

    public void close() throws IOException {
      closeSocket(sock);
    }
  }

  public Socket open(String path) throws IOException {
    if (!com.turbovnc.rfb.Helper.isAvailable())
      return null;

    int sock = openSocket(path);

    return new MySocket(sock);
  }

  static final native int openSocket(String path);
  static final native int readSocket(int fd, byte[] buf, int len);
  static final native void writeSocket(int fd, byte[] buf, int len);
  static final native void closeSocket(int fd);
}
