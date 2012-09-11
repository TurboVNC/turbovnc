/* Copyright (C) 2012 D. R. Commander.  All Rights Reserved.
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

import java.io.*;
import com.turbovnc.network.FileDescriptor;
import com.turbovnc.rfb.Exception;

public class FileInStream extends FileInputStream implements FileDescriptor {

  static final double getTime() {
    return (double)System.nanoTime() / 1.0e9;
  }

  public FileInStream(String fileName) throws FileNotFoundException {
    super(fileName);
  }

  public double getReadTime() { return tRead; }
  public void resetReadTime() { tRead = 0.0; }

  public void reset() {
    try {
      super.getChannel().position(0);
    } catch(IOException e) {}
  }

  public int read(byte[] buf, int bufPtr, int length) throws Exception {
    int retval = 0;
    try {
      double tStart = getTime();
      retval = super.read(buf, bufPtr, length);
      tRead += getTime() - tStart;
    } catch(IOException e) {
      throw new Exception(e.getMessage());
    }
    return retval;
  }

  public int write(byte[] buf, int bufPtr, int length) { return 0; }

  public int select(int interestOps, Integer timeout) throws Exception {
    try {
      return available();
    } catch(IOException e) {
      throw new Exception(e.getMessage());
    }
  }

  public void close() throws IOException {
    super.close();
  }

  double tRead;
};
