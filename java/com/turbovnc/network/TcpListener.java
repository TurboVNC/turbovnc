/* Copyright (C) 2012-2013, 2018 D. R. Commander.  All Rights Reserved.
 * Copyright (C) 2012 Brian P. Hinz
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

package com.turbovnc.network;

import java.io.IOException;
import java.nio.*;
import java.nio.channels.*;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.UnknownHostException;
import java.util.Set;
import java.util.Iterator;

import com.turbovnc.rdr.ErrorException;
import com.turbovnc.rdr.WarningException;

public class TcpListener  {

  public TcpListener(String listenaddr, int port, boolean localhostOnly,
                     boolean close_) {
    closeFd = close_;

    TcpSocket.initSockets();
    try {
      channel = ServerSocketChannel.open();
      channel.configureBlocking(false);
    } catch (IOException e) {
      throw new ErrorException("Could not create listening socket: " +
                               e.getMessage());
    }

    // - Bind it to the desired port
    InetAddress addr = null;

    try {
      if (localhostOnly) {
        addr = InetAddress.getByName(null);
      } else if (listenaddr != null) {
        addr = java.net.InetAddress.getByName(listenaddr);
      } else {
        addr = InetAddress.getByName("0.0.0.0");
      }
    } catch (UnknownHostException e) {
      throw new ErrorException(e.getMessage());
    }

    try {
      channel.socket().bind(new InetSocketAddress(addr, port));
    } catch (IOException e) {
      throw new WarningException("Could not bind listening socket: " +
                                 e.getMessage());
    }

    // - Set it to be a listening socket
    try {
      selector = Selector.open();
      channel.register(selector, SelectionKey.OP_ACCEPT);
    } catch (IOException e) {
      throw new ErrorException("Could not enable listen mode for socket: " +
                               e.getMessage());
    }
  }

  public TcpListener(String listenaddr, int port) {
    this(listenaddr, port, false, true);
  }

  public void shutdown() {
    try {
      channel.close();
    } catch (IOException e) {
      throw new ErrorException("Could not close listener: " +
                               e.getMessage());
    }
  }

  public TcpSocket accept() {
    SocketChannel newSock = null;

    // Accept an incoming connection
    try {
      if (selector.select(0) > 0) {
        Set<SelectionKey> keys = selector.selectedKeys();
        Iterator<SelectionKey> iter = keys.iterator();
        while (iter.hasNext()) {
          SelectionKey key = (SelectionKey)iter.next();
          iter.remove();
          if (key.isAcceptable()) {
            newSock = channel.accept();
            break;
          }
        }
        keys.clear();
        if (newSock == null)
          return null;
      }
    } catch (IOException e) {
      throw new ErrorException("Could not accept new connection: " +
                               e.getMessage());
    }

    if (newSock == null)
      return null;

    // Disable Nagle's algorithm, to reduce latency
    try {
      newSock.socket().setTcpNoDelay(true);
    } catch (java.net.SocketException e) {
      throw new ErrorException("Could not disable Nagle's algorithm: " +
                               e.getMessage());
    }

    // Create the socket object & check connection is allowed
    SocketDescriptor fd = null;
    fd = new SocketDescriptor();
    fd.setChannel(newSock);
    TcpSocket s = new TcpSocket(fd);

    return s;
  }

  private boolean closeFd;
  private ServerSocketChannel channel;
  private Selector selector;
}
