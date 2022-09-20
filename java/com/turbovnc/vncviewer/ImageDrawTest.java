/* Copyright (C) 2012, 2015, 2018, 2020 D. R. Commander.  All Rights Reserved.
 * Copyright (C) 2011-2012 Brian P. Hinz
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

/* Low-level benchmark for our Java image drawing routines */

package com.turbovnc.vncviewer;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;

import com.turbovnc.rfb.*;

public class ImageDrawTest extends JFrame {

  static final PixelFormat VERY_LOW_COLOR_PF =
    new PixelFormat(8, 3, false, true, 1, 1, 1, 2, 1, 0);
  static final PixelFormat LOW_COLOR_PF =
    new PixelFormat(8, 6, false, true, 3, 3, 3, 4, 2, 0);
  static final PixelFormat MEDIUM_COLOR_PF =
    new PixelFormat(8, 8, false, true, 7, 7, 3, 5, 2, 0);
  static final PixelFormat MEDIUMHIGH_COLOR_PF =
    new PixelFormat(16, 15, false, true, 31, 31, 31, 10, 5, 0);
  static final PixelFormat HIGH_COLOR_PF =
    new PixelFormat(16, 16, false, true, 31, 63, 31, 11, 5, 0);

  public static final int DEFAULT_WIDTH = 1240;
  public static final int DEFAULT_HEIGHT = 900;

  public class MyPanel extends JPanel {

    public MyPanel(int w, int h, int colors) {
      swingDB = Utils.getBooleanProperty("turbovnc.swingdb", false);
      setOpaque(!swingDB);
      GraphicsEnvironment ge =
        GraphicsEnvironment.getLocalGraphicsEnvironment();
      GraphicsDevice gd = ge.getDefaultScreenDevice();
      GraphicsConfiguration gc = gd.getDefaultConfiguration();
      BufferCapabilities bufCaps = gc.getBufferCapabilities();
      ImageCapabilities imgCaps = gc.getImageCapabilities();
      if (bufCaps.isPageFlipping() || bufCaps.isMultiBufferAvailable() ||
          imgCaps.isAccelerated()) {
        System.out.println("Graphics device supports HW acceleration.");
      } else {
        System.out.println("Graphics device does not support HW acceleration.");
      }

      width = w;  height = h;
      im1 = new BIPixelBuffer(width, height, null, null);
      if (colors == 8)
        im1.setPF(VERY_LOW_COLOR_PF);
      else if (colors == 64)
        im1.setPF(LOW_COLOR_PF);
      else if (colors == 256)
        im1.setPF(MEDIUM_COLOR_PF);
      else if (colors == 32768)
        im1.setPF(MEDIUMHIGH_COLOR_PF);
      else if (colors == 65536)
        im1.setPF(HIGH_COLOR_PF);
      im2 = new BIPixelBuffer(width, height, null, null);
      if (colors == 8)
        im2.setPF(VERY_LOW_COLOR_PF);
      else if (colors == 64)
        im2.setPF(LOW_COLOR_PF);
      else if (colors == 256)
        im2.setPF(MEDIUM_COLOR_PF);
      else if (colors == 32768)
        im2.setPF(MEDIUMHIGH_COLOR_PF);
      else if (colors == 65536)
        im2.setPF(HIGH_COLOR_PF);

      System.out.println("Pixel format: " + im1.getPF().print());
    }

    public Dimension getPreferredSize() {
      return new Dimension(width, height);
    }

    public void initImage(BIPixelBuffer im, int w, int h, int offset) {
      int i, j;
      PixelFormat pf = im.getPF();
      int[] stride = new int[]{ w };
      Object pixels = im.getRawPixelsRW(stride);

      if (pf.is888() && pixels instanceof int[]) {
        int[] buf = (int[])pixels;
        for (j = 0; j < h; j++) {
          for (i = 0; i < w; i++) {
            buf[j * stride[0] + i] = 0xff << 24;
            buf[j * stride[0] + i] |= ((i + offset) % 256) << pf.redShift;
            buf[j * stride[0] + i] |= ((j + offset) % 256) << pf.greenShift;
            buf[j * stride[0] + i] |= ((i + j + offset) % 256) << pf.blueShift;
          }
        }
      } else {
        if (rgbBuf == null)
          rgbBuf = new byte[w * h * 3];
        for (j = 0; j < h; j++) {
          for (i = 0; i < w; i++) {
            rgbBuf[(j * w + i) * 3] = (byte)((i + offset) % 256);
            rgbBuf[(j * w + i) * 3 + 1] = (byte)((j + offset) % 256);
            rgbBuf[(j * w + i) * 3 + 2] = (byte)((i + j + offset) % 256);
          }
        }
        pf.bufferFromRGB(pixels, 0, 0, stride[0], rgbBuf, w, h);
      }
    }

    public void paintComponent(Graphics g) {
      Graphics2D g2 = (Graphics2D)g;
      if (!swingDB &&
          RepaintManager.currentManager(this).isDoubleBufferingEnabled())
        super.paintComponent(g);
      Rectangle r = g.getClipBounds();
      g2.drawImage((iter % 2 == 0 ? im2.getImage() : im1.getImage()), r.x,
                   r.y, r.x + r.width, r.y + r.height, r.x, r.y, r.x + r.width,
                   r.y + r.height, null);
      g2.dispose();
      if (!swingDB)
        RepaintManager.currentManager(this).setDoubleBufferingEnabled(true);
    }

    public void display() {
      double t0, t1;
      if (width != getWidth() || height != getHeight()) {
        im1.resize(getWidth(), getHeight());
        im2.resize(getWidth(), getHeight());
        width = getWidth();  height = getHeight();
      }
      System.out.format("Window size: %d x %d\n", width, height);

      initImage(im1, width, height, 0);
      initImage(im2, width, height, 1);
      t0 = Utils.getTime();
      while (true) {
        if (!swingDB)
          RepaintManager.currentManager(this).setDoubleBufferingEnabled(false);
        paintImmediately(0, 0, width, height);
        iter++;
        t1 = Utils.getTime();
        if (t1 - t0 > 5.0) {
          System.out.format("%f Mpixels/sec\n",
            (double)iter * (double)(width * height) / (1000000. * (t1 - t0)));
          t0 = t1;
          iter = 0;
        }
      }
    }

    BIPixelBuffer im1, im2;
    byte[] rgbBuf;
    int preferredWidth, preferredHeight, width, height, iter;
    boolean swingDB;
  }

  public ImageDrawTest(int width, int height, int colors) {
    setTitle("Image drawing benchmark");
    setSize(width, height);
    MyPanel panel = new MyPanel(width, height, colors);
    add(panel);
    pack();
    setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
    setVisible(true);
    setResizable(false);
    panel.display();
  }

  static {
    VncViewer.setBlitterDefaults();
  }

  public static void main(String[] arg) {
    int colors = -1, width = DEFAULT_WIDTH, height = DEFAULT_HEIGHT;

    for (int i = 0; i < arg.length; i++) {
      if (arg[i].toLowerCase().startsWith("-c") && i < arg.length - 1) {
        int temp = -1;
        try {
          temp = Integer.parseInt(arg[++i]);
        } catch (NumberFormatException e) {}
        switch (temp) {
          case 8:  case 64:  case 256:  case 32768:  case 65536:
            colors = temp;
        }
      }
      if (arg[i].toLowerCase().startsWith("-w") && i < arg.length - 1) {
        int temp = -1;
        try {
          temp = Integer.parseInt(arg[++i]);
        } catch (NumberFormatException e) {}
        if (temp > 0)
          width = temp;
      }
      if (arg[i].toLowerCase().startsWith("-h") && i < arg.length - 1) {
        int temp = -1;
        try {
          temp = Integer.parseInt(arg[++i]);
        } catch (NumberFormatException e) {}
        if (temp > 0)
          height = temp;
      }
    }
    new ImageDrawTest(width, height, colors);
  }
}
