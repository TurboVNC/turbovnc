/* Copyright (C) 2012, 2018, 2020 D. R. Commander.  All Rights Reserved.
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

import com.turbovnc.rdr.FdInStream;
import com.turbovnc.rdr.FdOutStream;
import com.turbovnc.rdr.*;
import com.turbovnc.rfb.LogWriter;

import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.net.UnknownHostException;
import java.nio.*;
import java.nio.channels.*;

public class TcpSocket extends Socket {

  // -=- Socket initialisation
  static boolean socketsInitialised = false;

  public static void initSockets() {
    if (socketsInitialised)
      return;
    socketsInitialised = true;
  }

  // -=- TcpSocket

  public TcpSocket(SocketDescriptor sock) {
    super(new FdInStream(sock), new FdOutStream(sock), true);
  }

  public TcpSocket(String host, int port) {
    SocketDescriptor sock = null;
    InetAddress addr = null;
    boolean result = false;

    // - Create a socket
    initSockets();

    try {
      addr = java.net.InetAddress.getByName(host);
    } catch (UnknownHostException e) {
      throw new WarningException("Could not resolve hostname: " +
                                 e.getMessage());
    }

    sock = new SocketDescriptor();

    /* Attempt to connect to the remote host */
    result = sock.connect(new InetSocketAddress(addr, port));

    if (!result && sock.isConnectionPending()) {
      while (!result)
        result = sock.finishConnect();
    }

    if (!result)
      throw new WarningException("Could not connect");

    // Disable Nagle's algorithm, to reduce latency
    enableNagles(sock, false);

    // Create the input and output streams
    instream = new FdInStream(sock);
    outstream = new FdOutStream(sock);
    ownStreams = true;
  }

  public int getMyPort() {
    SocketAddress address =
      ((SocketDescriptor)getFd()).socket().getLocalSocketAddress();
    return ((InetSocketAddress)address).getPort();
  }

  public String getPeerAddress() {
    InetAddress peer = ((SocketDescriptor)getFd()).socket().getInetAddress();
    if (peer != null)
      return peer.getHostAddress();
    return "";
  }

  public String getPeerName() {
    InetAddress peer = ((SocketDescriptor)getFd()).socket().getInetAddress();
    if (peer != null)
      return peer.getHostName();
    return "";
  }

  public int getPeerPort() {
    int port = ((SocketDescriptor)getFd()).socket().getPort();
    return port;
  }

  public String getPeerEndpoint() {
    String address = getPeerAddress();
    int port = getPeerPort();
    return address + "::" + port;
  }

  public boolean sameMachine() {
    SocketAddress peeraddr = ((SocketDescriptor)getFd()).getRemoteAddress();
    SocketAddress myaddr = ((SocketDescriptor)getFd()).getLocalAddress();
    return myaddr.equals(peeraddr);
  }

  public void shutdown() {
    super.shutdown();
    ((SocketDescriptor)getFd()).shutdown();
  }

  public void close() {
    ((SocketDescriptor)getFd()).close();
  }

  public static boolean enableNagles(SocketDescriptor sock, boolean enable) {
    try {
      sock.channel.socket().setTcpNoDelay(!enable);
    } catch (java.net.SocketException e) {
      vlog.error("Could not " + (enable ? "enable" : "disable") +
                 " Nagle's algorithm: " + e.getMessage());
      return false;
    }
    return true;
  }

  public boolean isConnected() {
    return ((SocketDescriptor)getFd()).isConnected();
  }

  public int getSockPort() {
    SocketAddress address =
      ((SocketDescriptor)getFd()).socket().getRemoteSocketAddress();
    return ((InetSocketAddress)address).getPort();
  }

  /* Tunnelling support. */
  public static int findFreeTcpPort() {
    java.net.ServerSocket sock;
    int port;
    try {
      sock = new java.net.ServerSocket(0);
      port = sock.getLocalPort();
      sock.close();
    } catch (java.io.IOException e) {
      throw new SystemException(e);
    }
    return port;
  }

  static LogWriter vlog = new LogWriter("TcpSocket");
}
