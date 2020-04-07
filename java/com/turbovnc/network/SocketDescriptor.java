/* Copyright (C) 2012, 2017-2018, 2020 D. R. Commander.  All Rights Reserved.
 * Copyright (C) 2012 Brian P. Hinz
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
package com.turbovnc.network;

import java.io.IOException;

import java.net.SocketAddress;
import java.nio.*;
import java.nio.channels.*;
import java.nio.channels.spi.SelectorProvider;

import com.turbovnc.rdr.*;
import com.turbovnc.rfb.Utils;

public class SocketDescriptor implements FileDescriptor {

  public SocketDescriptor() {
    defaultSelectorProvider();
    try {
      channel = SocketChannel.open();
      channel.configureBlocking(false);
      writeSelector = Selector.open();
      readSelector = Selector.open();
    } catch (IOException e) {
      throw new SystemException(e);
    }
    try {
      channel.register(writeSelector, SelectionKey.OP_WRITE);
      channel.register(readSelector, SelectionKey.OP_READ);
    } catch (ClosedChannelException e) {
      throw new SystemException(e);
    }
  }

  public void shutdown() {
    try {
      channel.socket().shutdownInput();
      channel.socket().shutdownOutput();
    } catch (IOException e) {
      throw new SystemException(e);
    }
  }

  public void close() {
    try {
      channel.close();
    } catch (IOException e) {
      throw new SystemException(e);
    }
  }

  private static SelectorProvider defaultSelectorProvider() {
    // kqueue() selector provider on OS X is not working, fall back to select()
    // for now
    if (Utils.isMac() && Utils.JAVA_VERSION < 9)
      System.setProperty("java.nio.channels.spi.SelectorProvider",
                         "sun.nio.ch.PollSelectorProvider");
    return SelectorProvider.provider();
  }

  public synchronized int read(byte[] buf, int bufPtr, int length) {
    int n;
    ByteBuffer b = ByteBuffer.allocate(length);
    try {
      n = channel.read(b);
    } catch (IOException e) {
      throw new WarningException("Read error: " + e.getMessage());
    }
    if (n <= 0)
      return (n == 0) ? -1 : 0;
    ((Buffer)b).flip();
    b.get(buf, bufPtr, n);
    ((Buffer)b).clear();
    return n;

  }

  public synchronized int write(byte[] buf, int bufPtr, int length) {
    int n;
    ByteBuffer b = ByteBuffer.allocate(length);
    b.put(buf, bufPtr, length);
    ((Buffer)b).flip();
    try {
      n = channel.write(b);
    } catch (IOException e) {
      throw new ErrorException("Write error: " + e.getMessage());
    }
    ((Buffer)b).clear();
    return n;
  }

  public synchronized int select(int interestOps, Integer timeout) {
    int n;
    Selector selector;
    if ((interestOps & SelectionKey.OP_READ) != 0) {
      selector = readSelector;
    } else {
      selector = writeSelector;
    }
    selector.selectedKeys().clear();
    try {
      if (timeout == null) {
        n = selector.select();
      } else {
        int tv = timeout.intValue();
        switch (tv) {
          case 0:
            n = selector.selectNow();
            break;
          default:
            n = selector.select((long)tv);
            break;
        }
      }
    } catch (IOException e) {
      throw new SystemException(e);
    }
    return n;
  }

  public int write(ByteBuffer buf) {
    int n = 0;
    try {
      n = channel.write(buf);
    } catch (IOException e) {
      throw new ErrorException("Write error: " + e.getMessage());
    }
    return n;
  }

  public long write(ByteBuffer[] buf, int offset, int length) {
    long n = 0;
    try {
      n = channel.write(buf, offset, length);
    } catch (IOException e) {
      throw new ErrorException("Write error: " + e.getMessage());
    }
    return n;
  }

  public int read(ByteBuffer buf) {
    int n = 0;
    try {
      n = channel.read(buf);
    } catch (IOException e) {
      throw new WarningException("Read error: " + e.getMessage());
    }
    return n;
  }

  public long read(ByteBuffer[] buf, int offset, int length) {
    long n = 0;
    try {
      n = channel.read(buf, offset, length);
    } catch (IOException e) {
      throw new WarningException("Read error: " + e.getMessage());
    }
    return n;
  }

  public java.net.Socket socket() {
    return channel.socket();
  }

  public SocketAddress getRemoteAddress() {
    if (isConnected())
      return channel.socket().getRemoteSocketAddress();
    return null;
  }

  public SocketAddress getLocalAddress() {
    if (isConnected())
      return channel.socket().getLocalSocketAddress();
    return null;
  }

  public boolean isConnectionPending() {
    return channel.isConnectionPending();
  }

  public boolean connect(SocketAddress remote) {
    try {
      return channel.connect(remote);
    } catch (IOException e) {
      throw new WarningException("Could not connect: " + e.getMessage());
    }
  }

  public boolean finishConnect() {
    try {
      return channel.finishConnect();
    } catch (IOException e) {
      throw new WarningException("Could not connect: " + e.getMessage());
    }
  }

  public boolean isConnected() {
    return channel.isConnected();
  }

  protected void setChannel(SocketChannel channel_) {
    try {
      if (channel != null)
        channel.close();
      if (readSelector != null)
        readSelector.close();
      if (writeSelector != null)
        writeSelector.close();
      channel = channel_;
      channel.configureBlocking(false);
      writeSelector = Selector.open();
      readSelector = Selector.open();
    } catch (IOException e) {
      throw new SystemException(e);
    }
    try {
      channel.register(writeSelector, SelectionKey.OP_WRITE);
      channel.register(readSelector, SelectionKey.OP_READ);
    } catch (ClosedChannelException e) {
      System.err.println(e.toString());
    }
  }

  protected SocketChannel channel;
  protected Selector writeSelector;
  protected Selector readSelector;
}
