/* Copyright (C) 2021 Steffen Kieß
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

import java.io.InputStream;
import java.io.OutputStream;

import com.turbovnc.rdr.*;

public class StreamSocket extends Socket {

  public StreamSocket(InputStream in, OutputStream out, boolean own) {
    descriptor = new StreamDescriptor(in, out);
    instream = new FdInStream(descriptor);
    outstream = new FdOutStream(descriptor);
    ownStreams = own;
  }

  public int getMyPort() {
    return -1;
  }

  public String getPeerAddress() {
    return "";
  }

  public String getPeerName() {
    return "";
  }

  public int getPeerPort() {
    return -1;
  }

  public String getPeerEndpoint() {
    return "";
  }

  public boolean sameMachine() {
    return false;
  }

  public void shutdown() {
    super.shutdown();
    descriptor.close();
  }

  public void close() {
    descriptor.close();
  }

  public boolean isConnected() {
    return true;
  }

  public int getSockPort() {
    return -1;
  }

  private StreamDescriptor descriptor;
}
