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

import java.io.IOException;

import java.nio.channels.*;

import java.io.InputStream;
import java.io.OutputStream;

import com.turbovnc.rdr.*;

// A FileDescriptor implementation which reads from an InputStream and writes
// into an OutputStream.
//
// Because it is not possible to find out whether a read or write from an
// InputStream / OutputStream will be blocking, reading and writing will be done
// in background threads.
public class StreamDescriptor implements FileDescriptor {

  public StreamDescriptor(InputStream in, OutputStream out) {
    inputStream = in;
    outputStream = out;

    inThread = new Thread() {
      public void run() {
        inThreadRun();
      }
    };
    inThread.start();

    outThread = new Thread() {
      public void run() {
        outThreadRun();
      }
    };
    outThread.start();
  }

  public void close() {
    try {
      inputStream.close();
      outputStream.close();
    } catch (java.io.IOException e) {
      throw new SystemException(e);
    }
  }

  private void inThreadRun() {
    while (true) {
      synchronized (this) {
        // Wait until data has been removed from input buffer
        while (inPos != inLen) {
          try {
            wait();
          } catch (InterruptedException e) {
            throw new SystemException(e);
          }
        }
        inLen = inPos = 0;
      }

      int n;
      try {
        n = inputStream.read(inBuffer, 0, inBuffer.length);
      } catch (IOException e) {
        synchronized (this) {
          error = e;
          notifyAll();
          return;
        }
      }

      synchronized (this) {
        if (n < 0) { // EOF
          inLen = -1;
          notifyAll();
          return;
        } else {
          inLen = n;
          notifyAll();
        }
      }
    }
  }

  public synchronized int read(byte[] buf, int bufPtr, int length) {
    if (error != null)
      throw new ErrorException("Connection encountered an error: "
                               + error.getMessage());

    if (length == 0)
      return 0;

    if (inPos == inLen)
      throw new ErrorException("Attempted blocking read operation");

    if (inLen == -1)
      return 0; // EOF

    int bytesToRead = Math.min(length, inLen - inPos);
    System.arraycopy(inBuffer, inPos, buf, bufPtr, bytesToRead);
    inPos += bytesToRead;
    StreamDescriptor.this.notifyAll();

    return bytesToRead;
  }

  private void outThreadRun() {
    while (true) {
      synchronized (this) {
        // Wait until there is data in the output buffer
        while (outLen == 0) {
          try {
            wait();
          } catch (InterruptedException e) {
            throw new SystemException(e);
          }
        }
      }

      try {
        if (outLen == -1) { // EOF
          outputStream.close();
          return;
        }

        outputStream.write(outBuffer, 0, outLen);
        outputStream.flush();
      } catch (IOException e) {
        synchronized (this) {
          error = e;
          notifyAll();
          return;
        }
      }

      synchronized (this) {
        outLen = 0;
        notifyAll();
      }
    }
  }

  public synchronized int write(byte[] buf, int bufPtr, int length) {
    if (error != null)
      throw new ErrorException("Connection encountered an error: "
                               + error.getMessage());

    if (length == 0)
      return 0;

    if (outLen == -1)
      throw new ErrorException("Attempted write operation after sending EOF");

    if (outLen != 0)
      throw new ErrorException("Attempted blocking write operation");

    int bytesToWrite = Math.min(length, outBuffer.length);
    System.arraycopy(buf, bufPtr, outBuffer, 0, bytesToWrite);
    outLen = bytesToWrite;
    StreamDescriptor.this.notifyAll();

    return bytesToWrite;
  }

  public synchronized int select(int interestOps, Integer timeout) {
    if ((interestOps & ~(SelectionKey.OP_READ | SelectionKey.OP_WRITE)) != 0)
      throw new ErrorException("Unexpected selection key");

    while (true) {
      if (error != null)
        return 1;
      if ((interestOps & SelectionKey.OP_READ) != 0 && inPos != inLen)
        return 1;
      if ((interestOps & SelectionKey.OP_WRITE) != 0 && outLen == 0)
        return 1;

      try {
        if (timeout == null)
          wait();
        else if (timeout == 0)
          return 0;
        else
          // Note: This might wait multiple times for timeout if the thread is
          // woken up for other reasons.
          wait(timeout);
      } catch (InterruptedException e) {
        throw new SystemException(e);
      }
    }
  }

  private InputStream inputStream;
  private OutputStream outputStream;

  private Thread inThread;
  private byte[] inBuffer = new byte[65536];
  private int inPos = 0;
  private int inLen = 0;

  private Thread outThread;
  private byte[] outBuffer = new byte[65536];
  private int outLen = 0;

  private IOException error = null;
}
