/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
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

// Abstract base class for any kind of network stream/socket

package com.turbovnc.network;

import java.nio.channels.*;

public abstract class SocketListener {

  public SocketListener() {}

  // shutdown() stops the socket from accepting further connections
  public abstract void shutdown();

  // accept() returns a new Socket object if there is a connection
  // attempt in progress AND if the connection passes the filter
  // if one is installed.  Otherwise, returns 0.
  public abstract Socket accept();

  // setFilter() applies the specified filter to all new connections
  //public void setFilter(ConnectionFilter* f) {filter = f;}
  public FileDescriptor getFd() { return fd; }

  protected FileDescriptor fd;
  //protected ConnectionFilter* filter;

}
