/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright (C) 2006 Constantin Kaplinsky.  All Rights Reserved.
 * Copyright (C) 2009 Paul Donohue.  All Rights Reserved.
 * Copyright (C) 2010, 2012-2013, 2015 D. R. Commander.  All Rights Reserved.
 * Copyright (C) 2011-2013 Brian P. Hinz
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

//
// DesktopWindow is an AWT Canvas representing a VNC desktop.
//
// Methods on DesktopWindow are called from both the GUI thread and the thread
// which processes incoming RFB messages ("the RFB thread").  This means we
// need to be careful with synchronization here.
//

package com.turbovnc.vncviewer;
import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;
import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.Transferable;
import java.awt.datatransfer.Clipboard;
import javax.swing.*;

import com.turbovnc.rfb.*;
import com.turbovnc.rfb.Cursor;
import com.turbovnc.rfb.Point;

class DesktopWindow extends JPanel implements Runnable, MouseListener,
  MouseMotionListener, MouseWheelListener, KeyListener {

  static final double getTime() {
    return (double)System.nanoTime() / 1.0e9;
  }

  ////////////////////////////////////////////////////////////////////
  // The following methods are all called from the RFB thread

  public DesktopWindow(int width, int height, PixelFormat serverPF,
                       CConn cc_) {
    cc = cc_;
    setSize(width, height);
    swingDB = VncViewer.getBooleanProperty("turbovnc.swingdb", false);
    setOpaque(!swingDB);
    GraphicsEnvironment ge =
      GraphicsEnvironment.getLocalGraphicsEnvironment();
    GraphicsDevice gd = ge.getDefaultScreenDevice();
    GraphicsConfiguration gc = gd.getDefaultConfiguration();
    BufferCapabilities bufCaps = gc.getBufferCapabilities();
    ImageCapabilities imgCaps = gc.getImageCapabilities();
    if (bufCaps.isPageFlipping() || bufCaps.isMultiBufferAvailable() ||
        imgCaps.isAccelerated()) {
      vlog.debug("GraphicsDevice supports HW acceleration.");
    } else {
      vlog.debug("GraphicsDevice does not support HW acceleration.");
    }
    im = new BIPixelBuffer(width, height, cc, this);

    cursor = new Cursor();
    cursorBacking = new ManagedPixelBuffer();
    Dimension bestSize = tk.getBestCursorSize(16, 16);
    BufferedImage cursorImage =
      new BufferedImage(bestSize.width, bestSize.height,
                        BufferedImage.TYPE_INT_ARGB);
    noCursor = tk.createCustomCursor(cursorImage, new java.awt.Point(0, 0),
                                     "noCursor");
    cursorImage.flush();
    if (!cc.opts.cursorShape && !bestSize.equals(new Dimension(0, 0)))
      setCursor(noCursor);
    addMouseListener(this);
    addMouseWheelListener(this);
    addMouseMotionListener(this);
    addKeyListener(this);
    addFocusListener(new FocusAdapter() {
      public void focusGained(FocusEvent e) {
        if (cc.viewer.benchFile == null) checkClipboard();
      }
      public void focusLost(FocusEvent e) {
        cc.releaseModifiers();
      }
    });
    setFocusTraversalKeysEnabled(false);
    setFocusable(true);
  }

  public int width() {
    return getWidth();
  }

  public int height() {
    return getHeight();
  }

  public final PixelFormat getPF() { return im.getPF(); }

  public void setViewport(Viewport viewport) {
    viewport.setChild(this);
  }

  // Methods called from the RFB thread - these need to be synchronized
  // wherever they access data shared with the GUI thread.

  public void setCursor(int w, int h, Point hotspot,
                        int[] data, byte[] mask) {
    if (!cc.opts.cursorShape)
      return;

    hideLocalCursor();

    if (hotspot == null)
      hotspot = new Point(0, 0);

    cursor.hotspot = hotspot;

    cursor.setSize(w, h);
    PixelFormat cursorPF = getPF();
    cursorPF.alphaPreMultiplied = false;
    cursor.setPF(cursorPF);

    cursorBacking.setSize(cursor.width(), cursor.height());
    cursorBacking.setPF(cursorPF);

    cursor.data = (Object)(new int[cursor.width() * cursor.height()]);
    cursor.mask = new byte[cursor.maskLen()];

    int maskBytesPerRow = (w + 7) / 8;
    for (int y = 0; y < h; y++) {
      for (int x = 0; x < w; x++) {
        int byte_ = y * maskBytesPerRow + x / 8;
        int bit = 7 - x % 8;
        if ((mask[byte_] & (1 << bit)) > 0) {
          ((int[])cursor.data)[y * cursor.width() + x] = (0xff << 24) |
            (cursor.cm.getRed(data[y * w + x]) << 16) |
            (cursor.cm.getGreen(data[y * w + x]) << 8) |
            (cursor.cm.getBlue(data[y * w + x]));
        }
      }
      System.arraycopy(mask, y * maskBytesPerRow, cursor.mask,
        y * ((cursor.width() + 7) / 8), maskBytesPerRow);
    }

    MemoryImageSource cursorSrc =
      new MemoryImageSource(cursor.width(), cursor.height(),
                            ColorModel.getRGBdefault(), (int[])cursor.data, 0,
                            cursor.width());
    int cw = (int)Math.floor((float)cursor.width() * scaleWidthRatio);
    int ch = (int)Math.floor((float)cursor.height() * scaleHeightRatio);
    Dimension bestSize = tk.getBestCursorSize(cw, ch);
    Image srcImage = tk.createImage(cursorSrc);
    BufferedImage cursorImage =
      new BufferedImage(bestSize.width, bestSize.height,
                        BufferedImage.TYPE_INT_ARGB);
    Graphics2D g2 = cursorImage.createGraphics();
    g2.setRenderingHint(RenderingHints.KEY_RENDERING,
                        RenderingHints.VALUE_RENDER_SPEED);
    g2.drawImage(srcImage, 0, 0, (int)Math.min(cw, bestSize.width),
                 (int)Math.min(ch, bestSize.height), 0, 0, cursor.width(),
                 cursor.height(), null);
    g2.dispose();
    srcImage.flush();

    int x = (int)Math.floor((float)hotspot.x * scaleWidthRatio);
    int y = (int)Math.floor((float)hotspot.y * scaleHeightRatio);
    x = (int)Math.min(x, Math.max(bestSize.width - 1, 0));
    y = (int)Math.min(y, Math.max(bestSize.height - 1, 0));
    java.awt.Point hs = new java.awt.Point(x, y);
    if (!bestSize.equals(new Dimension(0, 0)))
      softCursor = tk.createCustomCursor(cursorImage, hs, "softCursor");
    cursorImage.flush();

    if (softCursor != null) {
      setCursor(softCursor);
      cursorAvailable = true;
      return;
    }

    if (!cursorAvailable) {
      cursorAvailable = true;
    }

    showLocalCursor();
    return;
  }

  public void setServerPF(PixelFormat pf) {
    im.setPF(pf);
  }

  public PixelFormat getPreferredPF() {
    return im.getNativePF();
  }

  // setColourMapEntries() changes some of the entries in the colourmap.
  // Unfortunately these messages are often sent one at a time, so we delay the
  // settings taking effect unless the whole colourmap has changed.  This is
  // because getting java to recalculate its internal translation table and
  // redraw the screen is expensive.

  public synchronized void setColourMapEntries(int firstColour, int nColours,
                                               int[] rgbs) {
    im.setColourMapEntries(firstColour, nColours, rgbs);
    if (nColours <= 256) {
      im.updateColourMap();
    } else {
      if (setColourMapEntriesTimerThread == null) {
        setColourMapEntriesTimerThread = new Thread(this);
        setColourMapEntriesTimerThread.start();
      }
    }
  }

  // Update the actual window with the changed parts of the framebuffer.
  public void updateWindow() {
    double tBlitStart = getTime();
    Rect r = damage;
    cc.blitPixels += r.width() * r.height();
    if (!r.isEmpty()) {
      if (cc.cp.width != scaledWidth || cc.cp.height != scaledHeight) {
        int x = (int)Math.floor(r.tl.x * scaleWidthRatio);
        if (cc.viewport.dx > 0)
          x += cc.viewport.dx;
        int y = (int)Math.floor(r.tl.y * scaleHeightRatio);
        if (cc.viewport.dy > 0)
          y += cc.viewport.dy;
        // Need one extra pixel to account for rounding.
        int width = (int)Math.ceil(r.width() * scaleWidthRatio) + 1;
        int height = (int)Math.ceil(r.height() * scaleHeightRatio) + 1;
        if (x + width > scaledWidth + cc.viewport.dx)
          width = scaledWidth + cc.viewport.dx - x;
        if (y + height > scaledHeight + cc.viewport.dy)
          height = scaledHeight + cc.viewport.dy - y;
        // We don't actually need Java 2D to double-buffer the viewport,
        // because we're taking care of that ourselves.  This improves
        // performance on a lot of systems and allows the viewer to achieve
        // optimal performance under X11 without requiring MIT-SHM pixmaps.
        if (!swingDB)
          RepaintManager.currentManager(this).setDoubleBufferingEnabled(false);
        paintImmediately(x, y, width, height);
      } else {
        int x = (cc.viewport.dx > 0) ? cc.viewport.dx + r.tl.x : r.tl.x;
        int y = (cc.viewport.dy > 0) ? cc.viewport.dy + r.tl.y : r.tl.y;
        if (!swingDB)
          RepaintManager.currentManager(this).setDoubleBufferingEnabled(false);
        paintImmediately(x, y, r.width(), r.height());
      }
      damage.clear();
    }
    cc.tBlit += getTime() - tBlitStart;
    cc.blits += 1;
  }

  // resize() is called when the desktop has changed size
  public void resize() {
    int w = cc.cp.width;
    int h = cc.cp.height;
    hideLocalCursor();
    setSize(w, h);
    im.resize(w, h);
  }

  public final void fillRect(int x, int y, int w, int h, int pix) {
    if (overlapsCursor(x, y, w, h)) hideLocalCursor();
    im.fillRect(x, y, w, h, pix);
    damageRect(x, y, w, h);
    if (softCursor == null)
      showLocalCursor();
  }

  public final void imageRect(int x, int y, int w, int h,
                              Object pix) {
    if (overlapsCursor(x, y, w, h)) hideLocalCursor();
    im.imageRect(x, y, w, h, pix);
    damageRect(x, y, w, h);
    if (softCursor == null)
      showLocalCursor();
  }

  public final void copyRect(int x, int y, int w, int h,
                             int srcX, int srcY) {
    if (overlapsCursor(x, y, w, h) || overlapsCursor(srcX, srcY, w, h))
      hideLocalCursor();
    im.copyRect(x, y, w, h, srcX, srcY);
    damageRect(x, y, w, h);
  }

  public final Object getRawPixelsRW(int[] stride) {
    return im.getRawPixelsRW(stride);
  }

  public final void releaseRawPixels(Rect r) {
    damageRect(r.tl.x, r.tl.y, r.width(), r.height());
  }

  // mutex MUST be held when overlapsCursor() is called
  final boolean overlapsCursor(int x, int y, int w, int h) {
    return (x < cursorBackingX + cursorBacking.width() &&
            y < cursorBackingY + cursorBacking.height() &&
            x + w > cursorBackingX && y + h > cursorBackingY);
  }


  ////////////////////////////////////////////////////////////////////
  // The following methods are all called from the GUI thread

  void resetLocalCursor() {
    hideLocalCursor();
    cursorAvailable = false;
  }

  //
  // Callback methods to determine geometry of our Component.
  //

  public Dimension getPreferredSize() {
    return new Dimension(scaledWidth, scaledHeight);
  }

  public Dimension getMinimumSize() {
    return new Dimension(scaledWidth, scaledHeight);
  }

  public Dimension getMaximumSize() {
    return new Dimension(scaledWidth, scaledHeight);
  }

  public void setScaledSize() {
    if (cc.opts.scalingFactor != Options.SCALE_AUTO &&
        cc.opts.scalingFactor != Options.SCALE_FIXEDRATIO) {
      scaledWidth = (int)Math.floor((float)cc.cp.width *
                                    (float)cc.opts.scalingFactor / 100.0);
      scaledHeight = (int)Math.floor((float)cc.cp.height *
                                     (float)cc.opts.scalingFactor / 100.0);
    } else {
      if (cc.viewport == null) {
        scaledWidth = cc.cp.width;
        scaledHeight = cc.cp.height;
      } else {
        Dimension vpSize = cc.viewport.getSize();
        Insets vpInsets = cc.viewport.getInsets();
        int availableHeight = vpSize.height - vpInsets.top - vpInsets.bottom;
        if (cc.viewport.tb.isVisible())
          availableHeight -= cc.viewport.tb.getHeight();
        Dimension availableSize =
          new Dimension(vpSize.width - vpInsets.left - vpInsets.right,
                        availableHeight);
        if (availableSize.width == 0 || availableSize.height == 0)
          availableSize = new Dimension(cc.cp.width, cc.cp.height);
        if (cc.opts.scalingFactor == Options.SCALE_FIXEDRATIO) {
          float widthRatio = (float)availableSize.width / (float)cc.cp.width;
          float heightRatio = (float)availableSize.height / (float)cc.cp.height;
          float ratio = Math.min(widthRatio, heightRatio);
          scaledWidth = (int)Math.floor(cc.cp.width * ratio);
          scaledHeight = (int)Math.floor(cc.cp.height * ratio);
        } else {
          scaledWidth = availableSize.width;
          scaledHeight = availableSize.height;
        }
      }
    }
    scaleWidthRatio = (float)scaledWidth / (float)cc.cp.width;
    scaleHeightRatio = (float)scaledHeight / (float)cc.cp.height;
  }

  public void paintComponent(Graphics g) {
    Graphics2D g2 = (Graphics2D) g;
    if (repaintBackground && !swingDB) {
      super.paintComponent(g);
      repaintBackground = false;
    }
    if (cc.viewport.dx > 0 || cc.viewport.dy > 0)
      g2.translate(cc.viewport.dx, cc.viewport.dy);
    if (cc.cp.width != scaledWidth || cc.cp.height != scaledHeight) {
      g2.setRenderingHint(RenderingHints.KEY_INTERPOLATION,
                          RenderingHints.VALUE_INTERPOLATION_BILINEAR);
      g2.drawImage(im.getImage(), 0, 0, scaledWidth, scaledHeight, null);
    } else {
      g2.drawImage(im.getImage(), 0, 0, null);
    }
    g2.dispose();
    if (!swingDB)
      RepaintManager.currentManager(this).setDoubleBufferingEnabled(true);
  }

  String oldContents = "";

  public synchronized void checkClipboard() {
    SecurityManager sm = System.getSecurityManager();
    try {
      if (sm != null) sm.checkPermission(new AWTPermission("accessClipboard"));
      Clipboard cb = Toolkit.getDefaultToolkit().getSystemClipboard();
      if (cb != null && cc.opts.sendClipboard) {
        Transferable t = cb.getContents(null);
        if ((t != null) && t.isDataFlavorSupported(DataFlavor.stringFlavor)) {
          try {
            String newContents = (String)t.getTransferData(DataFlavor.stringFlavor);
            if (newContents != null && !newContents.equals(oldContents)) {
              cc.writeClientCutText(newContents, newContents.length());
              oldContents = newContents;
              cc.clipboardDialog.setContents(newContents);
            }
          } catch(Exception e) {
            vlog.error("Error getting clipboard data:");
            vlog.error("  " + e.toString());
          }
        }
      }
    } catch(SecurityException e) {
      System.err.println("Cannot access the system clipboard");
    }
  }

  // Mouse-Motion callback function
  private void mouseMotionCB(MouseEvent e) {
    if (!cc.opts.viewOnly &&
        e.getX() >= cc.viewport.dx &&
        e.getY() >= cc.viewport.dy &&
        e.getX() <= cc.viewport.dx + scaledWidth &&
        e.getY() <= cc.viewport.dy + scaledHeight)
      cc.writePointerEvent(e);
    // - If local cursor rendering is enabled then use it
    if (cursorAvailable) {
      // - Render the cursor!
      if (e.getX() != cursorPosX || e.getY() != cursorPosY) {
        hideLocalCursor();
        if (e.getX() >= 0 && e.getX() < im.width() &&
            e.getY() >= 0 && e.getY() < im.height()) {
          cursorPosX = e.getX();
          cursorPosY = e.getY();
          if (softCursor == null)
            showLocalCursor();
        }
      }
    }
    lastX = e.getX();
    lastY = e.getY();
  }
  public void mouseDragged(MouseEvent e) { mouseMotionCB(e); }
  public void mouseMoved(MouseEvent e) { mouseMotionCB(e); }

  // Mouse callback function
  private void mouseCB(MouseEvent e) {
    if (!cc.opts.viewOnly && (e.getID() == MouseEvent.MOUSE_RELEASED ||
        (e.getX() >= cc.viewport.dx &&
         e.getY() >= cc.viewport.dy &&
         e.getX() <= cc.viewport.dx + scaledWidth &&
         e.getY() <= cc.viewport.dy + scaledHeight)))
      cc.writePointerEvent(e);
    lastX = e.getX();
    lastY = e.getY();
  }
  public void mouseReleased(MouseEvent e) { mouseCB(e); }
  public void mousePressed(MouseEvent e) { mouseCB(e); }
  public void mouseClicked(MouseEvent e) {}
  public void mouseEntered(MouseEvent e) {}
  public void mouseExited(MouseEvent e) {}

  // MouseWheel callback function
  private void mouseWheelCB(MouseWheelEvent e) {
    if (!cc.opts.viewOnly)
      cc.writeWheelEvent(e);
  }

  public void mouseWheelMoved(MouseWheelEvent e) {
    mouseWheelCB(e);
  }

  // Handle the key-typed event.
  public void keyTyped(KeyEvent e) {}

  // Handle the key-released event.
  public void keyReleased(KeyEvent e) {
    if (!cc.opts.viewOnly)
      cc.writeKeyEvent(e);
  }

  // Handle the key-pressed event.
  public void keyPressed(KeyEvent e) {
    if (e.getKeyCode() == MenuKey.getMenuKeyCode()) {
      int sx = (scaleWidthRatio == 1.00) ?
        lastX : (int)Math.floor(lastX * scaleWidthRatio);
      int sy = (scaleHeightRatio == 1.00) ?
        lastY : (int)Math.floor(lastY * scaleHeightRatio);
      java.awt.Point ev = new java.awt.Point(lastX, lastY);
      ev.translate(sx - lastX, sy - lastY);
      cc.showMenu((int)ev.getX(), (int)ev.getY());
      e.consume();
      return;
    }
    int ctrlAltShiftMask = Event.SHIFT_MASK | Event.CTRL_MASK | Event.ALT_MASK;
    if ((e.getModifiers() & ctrlAltShiftMask) == ctrlAltShiftMask) {
      switch (e.getKeyCode()) {
        case KeyEvent.VK_F:
          cc.toggleFullScreen();
          return;
        case KeyEvent.VK_I:
          cc.showInfo();
          return;
        case KeyEvent.VK_L:
          cc.losslessRefresh();
          return;
        case KeyEvent.VK_N:
          VncViewer.newViewer(cc.viewer);
          return;
        case KeyEvent.VK_O:
          cc.options.showDialog(cc.viewport);
          return;
        case KeyEvent.VK_P:
          if (cc.profileDialog.isVisible())
            cc.profileDialog.endDialog();
          else
            cc.profileDialog.showDialog(cc.viewport);
          cc.toggleProfile();
          return;
        case KeyEvent.VK_R:
          cc.refresh();
          return;
        case KeyEvent.VK_T:
          cc.toggleToolbar();
          return;
        case KeyEvent.VK_Z:
          cc.sizeWindow();
          return;
        case KeyEvent.VK_LEFT:
        case KeyEvent.VK_RIGHT:
        case KeyEvent.VK_UP:
        case KeyEvent.VK_DOWN:
        case KeyEvent.VK_PAGE_UP:
        case KeyEvent.VK_PAGE_DOWN:
        case KeyEvent.VK_HOME:
        case KeyEvent.VK_END:
          return;
      }
    }
    if ((e.getModifiers() & Event.META_MASK) == Event.META_MASK) {
      switch (e.getKeyCode()) {
        case KeyEvent.VK_COMMA:
        case KeyEvent.VK_N:
        case KeyEvent.VK_W:
        case KeyEvent.VK_I:
        case KeyEvent.VK_R:
        case KeyEvent.VK_L:
        case KeyEvent.VK_F:
        case KeyEvent.VK_Z:
        case KeyEvent.VK_T:
          return;
      }
    }
    if ((e.getModifiers() & Event.ALT_MASK) == Event.ALT_MASK &&
        e.getKeyCode() == KeyEvent.VK_ENTER &&
        VncViewer.fsAltEnter.getValue()) {
      cc.toggleFullScreen();
      return;
    }
    if (!cc.opts.viewOnly)
      cc.writeKeyEvent(e);
  }

  ////////////////////////////////////////////////////////////////////
  // The following methods are called from both RFB and GUI threads

  // Note that mutex MUST be held when hideLocalCursor() and showLocalCursor()
  // are called.

  private synchronized void hideLocalCursor() {
    if (!cc.opts.cursorShape)
      setCursor(noCursor);
    // - Blit the cursor backing store over the cursor
    if (cursorVisible) {
      cursorVisible = false;
      im.imageRect(cursorBackingX, cursorBackingY, cursorBacking.width(),
                   cursorBacking.height(), cursorBacking.data);
    }
  }

  private synchronized void showLocalCursor() {
    if (cursorAvailable && !cursorVisible) {
      if (!im.getPF().equal(cursor.getPF()) ||
          cursor.width() == 0 || cursor.height() == 0) {
        vlog.debug("attempting to render invalid local cursor");
        cursorAvailable = false;
        return;
      }
      cursorVisible = true;
      if (softCursor != null) return;

      int cursorLeft = cursor.hotspot.x;
      int cursorTop = cursor.hotspot.y;
      int cursorRight = cursorLeft + cursor.width();
      int cursorBottom = cursorTop + cursor.height();

      int x = (cursorLeft >= 0 ? cursorLeft : 0);
      int y = (cursorTop >= 0 ? cursorTop : 0);
      int w = ((cursorRight < im.width() ? cursorRight : im.width()) - x);
      int h = ((cursorBottom < im.height() ? cursorBottom : im.height()) - y);

      cursorBackingX = x;
      cursorBackingY = y;
      cursorBacking.setSize(w, h);

      for (int j = 0; j < h; j++)
        System.arraycopy(im.data, (y + j) * im.width() + x,
                         cursorBacking.data, j * w, w);

      im.maskRect(cursorLeft, cursorTop, cursor.width(), cursor.height(),
                  (int[])cursor.data, cursor.mask);
    }
  }

  void damageRect(int x, int y, int w, int h) {
    if (damage.isEmpty()) {
      damage.setXYWH(x, y, w, h);
    } else if (x >= 0 && y >= 0 && w > 0 && h > 0) {
      int x1 = Math.min(damage.tl.x, x);
      int y1 = Math.min(damage.tl.y, y);
      int x2 = Math.max(damage.br.x, x + w);
      int y2 = Math.max(damage.br.y, y + h);
      damage.setXYWH(x1, y1, x2 - x1, y2 - y1);
    }
  }

  // run() is executed by the setColourMapEntriesTimerThread - it sleeps for
  // 100ms before actually updating the colourmap.
  public synchronized void run() {
    try {
      Thread.sleep(100);
    } catch(InterruptedException e) {}
    im.updateColourMap();
    setColourMapEntriesTimerThread = null;
  }

  // access to cc by different threads is specified in CConn
  CConn cc;

  // access to the following must be synchronized:
  PlatformPixelBuffer im;
  Thread setColourMapEntriesTimerThread;

  Cursor cursor;
  boolean cursorVisible;     // Is cursor currently rendered?
  boolean cursorAvailable;   // Is cursor available for rendering?
  int cursorPosX, cursorPosY;
  ManagedPixelBuffer cursorBacking;
  int cursorBackingX, cursorBackingY;
  java.awt.Cursor softCursor, noCursor;
  static Toolkit tk = Toolkit.getDefaultToolkit();
  boolean swingDB, repaintBackground;

  public int scaledWidth = 0, scaledHeight = 0;
  float scaleWidthRatio, scaleHeightRatio;

  // the following are only ever accessed by the GUI thread:
  int lastX, lastY;
  Rect damage = new Rect();

  static LogWriter vlog = new LogWriter("DesktopWindow");
}
