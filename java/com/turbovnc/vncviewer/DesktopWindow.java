/* Copyright (C) 2010, 2012-2013, 2015-2018, 2020-2023 D. R. Commander.
 *                                                     All Rights Reserved.
 * Copyright (C) 2011-2013 Brian P. Hinz
 * Copyright (C) 2009 Paul Donohue.  All Rights Reserved.
 * Copyright (C) 2006 Constantin Kaplinsky.  All Rights Reserved.
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

// Methods in DesktopWindow are called from both the Swing Event Dispatch
// Thread (EDT) and the thread that processes incoming RFB messages ("the RFB
// thread").  This means that we need to be careful with synchronization here.

package com.turbovnc.vncviewer;

import java.awt.*;
import java.awt.event.*;
import java.awt.font.TextHitInfo;
import java.awt.im.InputMethodRequests;
import java.awt.image.*;
import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.Transferable;
import java.awt.datatransfer.Clipboard;
import java.io.*;
import java.lang.reflect.*;
import java.nio.*;
import java.text.AttributedCharacterIterator;
import java.text.AttributedString;
import javax.imageio.ImageIO;
import javax.swing.*;

import com.turbovnc.rdr.*;
import com.turbovnc.rfb.*;
import com.turbovnc.rfb.Cursor;
import com.turbovnc.rfb.Point;

class DesktopWindow extends JPanel implements Runnable, MouseListener,
  MouseMotionListener, MouseWheelListener, KeyListener, InputMethodRequests {

  // RFB thread
  DesktopWindow(int width, int height, PixelFormat serverPF, CConn cc_) {
    cc = cc_;
    setSize(width, height);
    setBackground(Color.BLACK);
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
    if (!cc.params.cursorShape.get() && !bestSize.equals(new Dimension(0, 0)))
      setCursor(noCursor);
    addMouseListener(this);
    addMouseWheelListener(this);
    addMouseMotionListener(this);
    addKeyListener(this);

    addFocusListener(new FocusAdapter() {
      // On Windows, Swing redirects keyboard focus to the default JMenu (or
      // the system menu, if the component has no JMenu) when LAlt is pressed
      // and subsequently released.  When that happens, subsequent keystrokes
      // are consumed by the system menu and not sent to the server until Esc
      // or LAlt is pressed to dismiss the menu.  In order to bypass that
      // behavior, we install a custom key event dispatcher into the keyboard
      // focus manager whenever the component gains focus.
      private final KeyEventDispatcher keyEventDispatcher =
        new KeyEventDispatcher() {
        public boolean dispatchKeyEvent(KeyEvent e) {
          if (e.getKeyCode() == 18 &&
              e.getKeyLocation() == KeyEvent.KEY_LOCATION_LEFT) {
            if (e.getID() == KeyEvent.KEY_PRESSED)
              cc.desktop.keyPressed(e);
            else if (e.getID() == KeyEvent.KEY_RELEASED)
              cc.desktop.keyReleased(e);
            return true;
          }
          return false;
        }
      };

      public void focusGained(FocusEvent e) {
        KeyboardFocusManager kfm =
          KeyboardFocusManager.getCurrentKeyboardFocusManager();
        if (Utils.isWindows()) kfm.addKeyEventDispatcher(keyEventDispatcher);
        if (cc.viewer.benchFile == null) checkClipboard();
        cc.pushLEDState();
      }

      public void focusLost(FocusEvent e) {
        KeyboardFocusManager kfm =
          KeyboardFocusManager.getCurrentKeyboardFocusManager();
        if (Utils.isWindows())
          kfm.removeKeyEventDispatcher(keyEventDispatcher);
        cc.releasePressedKeys();
      }
    });

    setFocusTraversalKeysEnabled(false);
    setFocusable(true);
    enableInputMethods(false);
  }

  // RFB thread
  public int width() {
    return getWidth();
  }

  public int height() {
    return getHeight();
  }

  public final PixelFormat getPF() { return im.getPF(); }

  // EDT
  public void setViewport(Viewport viewport) {
    viewport.setChild(this);
  }

  // RFB thread
  public void setCursor(int w, int h, Point hotspot,
                        int[] data, byte[] mask) {
    if (!cc.params.cursorShape.get() || cc.params.localCursor.get())
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
        int _byte = y * maskBytesPerRow + x / 8;
        int bit = 7 - x % 8;
        if ((mask[_byte] & (1 << bit)) > 0) {
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

  // RFB thread
  public void setServerPF(PixelFormat pf) {
    im.setPF(pf);
  }

  public PixelFormat getPreferredPF() {
    return im.getNativePF();
  }

  // RFB thread: setColourMapEntries() changes some of the entries in the
  // colourmap.  Unfortunately these messages are often sent one at a time, so
  // we prevent the settings from taking effect until the whole colourmap has
  // changed.  This is because making Java recalculate its internal translation
  // table and redraw the screen is expensive.

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

  // RFB thread: Update the actual window with the changed parts of the
  // framebuffer.
  public void updateWindow() {
    double tBlitStart = Utils.getTime();
    Rect r = damage;
    cc.blitPixels += r.width() * r.height();
    if (!r.isEmpty()) {
      if (cc.cp.width != scaledWidth || cc.cp.height != scaledHeight) {
        int x = (int)Math.floor(r.tl.x * scaleWidthRatio);
        int y = (int)Math.floor(r.tl.y * scaleHeightRatio);
        // Need one extra pixel to account for rounding.
        int width = (int)Math.ceil(r.width() * scaleWidthRatio) + 1;
        int height = (int)Math.ceil(r.height() * scaleHeightRatio) + 1;
        if (cc.viewport != null) {
          if (cc.viewport.dx > 0)
            x += cc.viewport.dx;
          if (cc.viewport.dy > 0)
            y += cc.viewport.dy;
          if (x + width > scaledWidth + cc.viewport.dx)
            width = scaledWidth + cc.viewport.dx - x;
          if (y + height > scaledHeight + cc.viewport.dy)
            height = scaledHeight + cc.viewport.dy - y;
        }
        // We don't actually need Java 2D to double-buffer the viewport,
        // because we're taking care of that ourselves.  This improves
        // performance on a lot of systems and allows the viewer to achieve
        // optimal performance under X11 without requiring MIT-SHM pixmaps.
        Window activeWindow =
          javax.swing.FocusManager.getCurrentManager().getActiveWindow();
        if (!swingDB &&
            (activeWindow == null || activeWindow instanceof Viewport ||
             (activeWindow instanceof JDialog &&
              ((JDialog)activeWindow).getTitle().equals(
                "TurboVNC profiling information"))))
          RepaintManager.currentManager(this).setDoubleBufferingEnabled(false);
        if (cc.viewer.benchFile != null)
          paintImmediately(x, y, width, height);
        else
          repaint(x, y, width, height);
      } else {
        int x = r.tl.x;
        int y = r.tl.y;
        if (cc.viewport != null) {
          if (cc.viewport.dx > 0)
            x += cc.viewport.dx;
          if (cc.viewport.dy > 0)
            y += cc.viewport.dy;
        }
        Window activeWindow =
          javax.swing.FocusManager.getCurrentManager().getActiveWindow();
        if (!swingDB &&
            (activeWindow == null || activeWindow instanceof Viewport ||
             (activeWindow instanceof JDialog &&
              ((JDialog)activeWindow).getTitle().equals(
                "TurboVNC profiling information"))))
          RepaintManager.currentManager(this).setDoubleBufferingEnabled(false);
        if (cc.viewer.benchFile != null)
          paintImmediately(x, y, r.width(), r.height());
        else
          repaint(x, y, r.width(), r.height());
      }
      damage.clear();
    }
    cc.tBlit += Utils.getTime() - tBlitStart;
    cc.blits += 1;
  }

  // resize() is called when the desktop has changed size.  See
  // CConn.resizeFramebuffer().
  public void resize() {
    int w = cc.cp.width;
    int h = cc.cp.height;
    hideLocalCursor();
    setSize(w, h);
    im.resize(w, h);
  }

  // RFB thread
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

  // mutex MUST be held when overlapsCursor() is called.
  final boolean overlapsCursor(int x, int y, int w, int h) {
    return (x < cursorBackingX + cursorBacking.width() &&
            y < cursorBackingY + cursorBacking.height() &&
            x + w > cursorBackingX && y + h > cursorBackingY);
  }


  ////////////////////////////////////////////////////////////////////
  // The following methods are all called from the EDT.

  void resetLocalCursor() {
    hideLocalCursor();
    cursorAvailable = false;
  }

  // Callback methods to determine the geometry of our Component.

  public Dimension getPreferredSize() {
    return new Dimension(scaledWidth, scaledHeight);
  }

  public Dimension getMinimumSize() {
    return new Dimension(scaledWidth, scaledHeight);
  }

  public Dimension getMaximumSize() {
    return new Dimension(scaledWidth, scaledHeight);
  }

  // Mostly called from the EDT, except for a couple of instances in
  // CConn.resizeFramebuffer().
  public void setScaledSize() {
    if (cc.params.scale.get() != ScaleParameter.AUTO &&
        cc.params.scale.get() != ScaleParameter.FIXEDRATIO) {
      scaledWidth = (int)Math.floor((float)cc.cp.width *
                                    (float)cc.params.scale.get() / 100.0);
      scaledHeight = (int)Math.floor((float)cc.cp.height *
                                     (float)cc.params.scale.get() / 100.0);
    } else {
      if (cc.viewport == null) {
        scaledWidth = cc.cp.width;
        scaledHeight = cc.cp.height;
      } else {
        Dimension availableSize = cc.viewport.getAvailableSize();
        if (availableSize.width == 0 || availableSize.height == 0) {
          availableSize.width = cc.cp.width;
          availableSize.height = cc.cp.height;
        }
        if (cc.params.scale.get() == ScaleParameter.FIXEDRATIO) {
          float widthRatio = (float)availableSize.width / (float)cc.cp.width;
          float heightRatio =
            (float)availableSize.height / (float)cc.cp.height;
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

  // EDT
  public void paintComponent(Graphics g) {
    Graphics2D g2 = (Graphics2D)g;
    if (!swingDB &&
        RepaintManager.currentManager(this).isDoubleBufferingEnabled())
      // If double buffering is enabled, then this must be a system-triggered
      // repaint, so we need to repaint all of the parent components.
      super.paintComponent(g);
    if (cc.viewport != null && (cc.viewport.dx > 0 || cc.viewport.dy > 0))
      g2.translate(cc.viewport.dx, cc.viewport.dy);
    Object scalingAlg = RenderingHints.VALUE_INTERPOLATION_BILINEAR;
    double displayScalingFactor = g2.getTransform().getScaleX();
    // If the remote desktop image will not be scaled, then the scaling
    // algorithm will only be used to support high DPI scaling on Windows.  In
    // that case, we emulate the behavior of Windows by default, using nearest-
    // neighbor interpolation with integral display scaling factors and
    // bilinear interpolation with fractional display scaling factors.
    if (Utils.isWindows() && (displayScalingFactor % 1.0) == 0.0 &&
        cc.cp.width == scaledWidth && cc.cp.height == scaledHeight)
      scalingAlg = RenderingHints.VALUE_INTERPOLATION_NEAREST_NEIGHBOR;
    String alg = System.getProperty("turbovnc.scalingalg");
    if (alg != null) {
      if (alg.equalsIgnoreCase("bicubic"))
        scalingAlg = RenderingHints.VALUE_INTERPOLATION_BICUBIC;
      else if (alg.equalsIgnoreCase("bilinear"))
        scalingAlg = RenderingHints.VALUE_INTERPOLATION_BILINEAR;
      else if (alg.equalsIgnoreCase("nearestneighbor"))
        scalingAlg = RenderingHints.VALUE_INTERPOLATION_NEAREST_NEIGHBOR;
    }
    if (Utils.isWindows() && displayScalingFactor != 1.0)
      g2.setRenderingHint(RenderingHints.KEY_INTERPOLATION, scalingAlg);
    if (cc.cp.width != scaledWidth || cc.cp.height != scaledHeight) {
      g2.setRenderingHint(RenderingHints.KEY_INTERPOLATION, scalingAlg);
      g2.drawImage(im.getImage(), 0, 0, scaledWidth, scaledHeight, null);
    } else {
      Rectangle r = g.getClipBounds();
      g2.drawImage(im.getImage(), r.x, r.y, r.x + r.width, r.y + r.height,
                   r.x, r.y, r.x + r.width, r.y + r.height, null);
    }
    g2.dispose();
    if (!swingDB)
      RepaintManager.currentManager(this).setDoubleBufferingEnabled(true);
  }

  // EDT
  public synchronized void checkClipboard() {
    try {
      Clipboard cb = null;
      if (Utils.getBooleanProperty("turbovnc.primary", true))
        cb = Toolkit.getDefaultToolkit().getSystemSelection();
      if (cb == null)
        cb = Toolkit.getDefaultToolkit().getSystemClipboard();
      if (cb != null && cc.params.sendClipboard.get()) {
        Transferable t = cb.getContents(null);
        if (t == null || !t.isDataFlavorSupported(DataFlavor.stringFlavor))
          return;
        BufferedReader br =
          new BufferedReader(DataFlavor.stringFlavor.getReaderForText(t));
        CharBuffer cbuf = CharBuffer.allocate(cc.params.maxClipboard.get());
        br.read(cbuf);
        ((Buffer)cbuf).flip();
        String newContents = cbuf.toString();
        if (!cc.clipboardDialog.compareContentsTo(newContents)) {
          cc.clipboardDialog.setContents(newContents);
          vlog.debug("Local clipboard changed.  Notifying server.");
          cc.announceClipboard(true);
        }
        br.close();
        System.gc();
      }
    } catch (Exception e) {
      vlog.error("Error getting clipboard data:");
      vlog.error("  " + e.getMessage());
    }
  }

  // EDT: Mouse motion callback function
  private void mouseMotionCB(MouseEvent e) {
    int x = (cc.viewport == null) ? 0 : cc.viewport.dx;
    int y = (cc.viewport == null) ? 0 : cc.viewport.dy;
    if (!cc.params.viewOnly.get() &&
        (!Utils.osEID() ||
         !cc.viewport.processExtInputEventHelper(cc.viewport.motionType)) &&
        e.getX() >= x &&
        e.getY() >= y &&
        e.getX() <= x + scaledWidth &&
        e.getY() <= y + scaledHeight)
      cc.writePointerEvent(e);
    // If local cursor rendering is enabled, then use it.
    if (cursorAvailable) {
      // Render the cursor
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

  // EDT: Mouse callback function
  private void mouseCB(MouseEvent e, int type) {
    int x = (cc.viewport == null) ? 0 : cc.viewport.dx;
    int y = (cc.viewport == null) ? 0 : cc.viewport.dy;
    if (!cc.params.viewOnly.get() &&
        (!Utils.osEID() || !cc.viewport.processExtInputEventHelper(type)) &&
        (e.getID() == MouseEvent.MOUSE_RELEASED ||
         (e.getX() >= x &&
          e.getY() >= y &&
          e.getX() <= x + scaledWidth &&
          e.getY() <= y + scaledHeight)))
      cc.writePointerEvent(e);
    lastX = e.getX();
    lastY = e.getY();
  }
  public void mouseReleased(MouseEvent e) {
    mouseCB(e, cc.viewport.buttonReleaseType);
  }
  public void mousePressed(MouseEvent e) {
    if (Utils.isMac()) {
      try {
        Class appClass;
        Object obj;

        if (Utils.JAVA_VERSION >= 9) {
          appClass = Desktop.class;
          obj = Desktop.getDesktop();
        } else {
          appClass = Class.forName("com.apple.eawt.Application");
          Method getApplication = appClass.getMethod("getApplication",
                                                     (Class[])null);
          obj = getApplication.invoke(appClass);
        }

        Method requestForeground =
          appClass.getMethod("requestForeground", boolean.class);
        requestForeground.invoke(obj, false);
      } catch (Exception ex) {
        vlog.error("Could not bring window to foreground:");
        vlog.error("  " + ex.getMessage());
      }
    }
    if (cc.viewer.benchFile == null) checkClipboard();
    mouseCB(e, cc.viewport.buttonPressType);
  }
  public void mouseClicked(MouseEvent e) {}
  public void mouseEntered(MouseEvent e) {}
  public void mouseExited(MouseEvent e) {}

  // EDT: Mouse wheel callback function
  private void mouseWheelCB(MouseWheelEvent e) {
    if (!cc.params.viewOnly.get() &&
        (!Utils.osEID() ||
         !cc.viewport.processExtInputEventHelper(cc.viewport.motionType)))
      cc.writeWheelEvent(e);
  }

  public void mouseWheelMoved(MouseWheelEvent e) {
    mouseWheelCB(e);
  }

  // EDT: Handle the key typed event.
  public void keyTyped(KeyEvent e) {}

  // EDT: Handle the key released event.
  public void keyReleased(KeyEvent e) {
    if (!cc.params.viewOnly.get())
      cc.writeKeyEvent(e);
  }

  // EDT: Handle the key pressed event.
  public void keyPressed(KeyEvent e) {
    if (e.getKeyCode() == cc.params.menuKey.getVKeyCode() &&
        e.getModifiersEx() == 0) {
      cc.showMenu();
      e.consume();
      return;
    }
    int ctrlAltShiftMask = InputEvent.SHIFT_DOWN_MASK |
                           InputEvent.CTRL_DOWN_MASK |
                           InputEvent.ALT_DOWN_MASK;
    if ((e.getModifiersEx() & ctrlAltShiftMask) == ctrlAltShiftMask &&
        (!Utils.isWindows() || !e.isAltGraphDown())) {
      switch (e.getKeyCode()) {
        case KeyEvent.VK_F:
          cc.toggleFullScreen();
          return;
        case KeyEvent.VK_G:
          if (cc.viewport != null) {
            cc.viewport.grabKeyboardHelper(!cc.isGrabSelected());
            cc.selectGrab(!cc.isGrabSelected());
          }
          return;
        case KeyEvent.VK_I:
          cc.showInfo();
          return;
        case KeyEvent.VK_L:
          cc.losslessRefresh();
          return;
        case KeyEvent.VK_M:
          cc.screenshot();
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
        case KeyEvent.VK_V:
          cc.toggleViewOnly();
          return;
        case KeyEvent.VK_X:
          VncViewer.tileWindows();
          return;
        case KeyEvent.VK_Z:
          cc.sizeWindow();
          return;
        case KeyEvent.VK_9:
        case KeyEvent.VK_NUMPAD9:
        case KeyEvent.VK_ADD:
          if (cc.params.desktopSize.getMode() != DesktopSize.AUTO &&
              cc.params.scale.get() != ScaleParameter.AUTO &&
              cc.params.scale.get() != ScaleParameter.FIXEDRATIO) {
            cc.zoomIn();
            return;
          }
          break;
        case KeyEvent.VK_8:
        case KeyEvent.VK_NUMPAD8:
        case KeyEvent.VK_SUBTRACT:
          if (cc.params.desktopSize.getMode() != DesktopSize.AUTO &&
              cc.params.scale.get() != ScaleParameter.AUTO &&
              cc.params.scale.get() != ScaleParameter.FIXEDRATIO) {
            cc.zoomOut();
            return;
          }
          break;
        case KeyEvent.VK_0:
        case KeyEvent.VK_NUMPAD0:
          if (cc.params.desktopSize.getMode() != DesktopSize.AUTO &&
              cc.params.scale.get() != ScaleParameter.AUTO &&
              cc.params.scale.get() != ScaleParameter.FIXEDRATIO) {
            cc.zoom100();
            return;
          }
          break;
        case KeyEvent.VK_LEFT:
        case KeyEvent.VK_RIGHT:
        case KeyEvent.VK_UP:
        case KeyEvent.VK_DOWN:
        case KeyEvent.VK_PAGE_UP:
        case KeyEvent.VK_PAGE_DOWN:
        case KeyEvent.VK_HOME:
        case KeyEvent.VK_END:
          if (cc.viewport != null &&
              (cc.viewport.sp.getHorizontalScrollBar().isVisible() ||
               cc.viewport.sp.getVerticalScrollBar().isVisible()) &&
              !VncViewer.isKeyboardGrabbed(cc.viewport))
            return;
      }
    }
    if (cc.params.macHotkeys.get() &&
        (e.getModifiersEx() & InputEvent.META_DOWN_MASK) ==
         InputEvent.META_DOWN_MASK) {
      switch (e.getKeyCode()) {
        case KeyEvent.VK_P:
          if (Utils.JAVA_VERSION >= 9 && Utils.JAVA_VERSION <= 11)
            e.consume();
          return;
        case KeyEvent.VK_COMMA:
        case KeyEvent.VK_BACK_QUOTE:
        case KeyEvent.VK_H:
        case KeyEvent.VK_Q:
        case KeyEvent.VK_N:
        case KeyEvent.VK_W:
        case KeyEvent.VK_I:
        case KeyEvent.VK_R:
        case KeyEvent.VK_L:
        case KeyEvent.VK_M:
        case KeyEvent.VK_F:
        case KeyEvent.VK_Z:
        case KeyEvent.VK_X:
        case KeyEvent.VK_T:
          return;
        case KeyEvent.VK_9:
        case KeyEvent.VK_8:
        case KeyEvent.VK_0:
          if (cc.params.desktopSize.getMode() != DesktopSize.AUTO &&
              cc.params.scale.get() != ScaleParameter.AUTO &&
              cc.params.scale.get() != ScaleParameter.FIXEDRATIO)
            return;
      }
    }
    if ((e.getModifiersEx() & InputEvent.ALT_DOWN_MASK) ==
        InputEvent.ALT_DOWN_MASK && e.getKeyCode() == KeyEvent.VK_ENTER &&
        cc.params.fsAltEnter.get()) {
      cc.toggleFullScreen();
      return;
    }
    if (!cc.params.viewOnly.get()) {
      // For some reason, Java on Windows doesn't update the lock key state
      // until another key is pressed.
      if (Utils.isWindows())
        cc.pushLEDState();
      cc.writeKeyEvent(e);
    }
  }

  // EDT: Save screenshot
  public void screenshot(File file) {
    int index = file.getName().lastIndexOf('.');
    String formatName =
      (index > 0 ? file.getName().substring(index + 1) : "");
    BufferedImage fbImage = (BufferedImage)im.getImage();
    int width = fbImage.getWidth(), height = fbImage.getHeight();
    BufferedImage rgbImage =
      new BufferedImage(width, height, BufferedImage.TYPE_INT_RGB);
    rgbImage.createGraphics().drawImage(fbImage, 0, 0, width, height, null);
    try {
      if (!ImageIO.write(rgbImage, formatName, file)) {
        WarningException we = new WarningException(
          "Could not save remote desktop image:\nImage format not supported");
        cc.viewer.reportException(we);
      }
    } catch (IOException e) {
      WarningException we = new WarningException(
        "Could not save remote desktop image:\n" + e.toString());
      cc.viewer.reportException(we);
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  // The following methods are called from both the EDT and the RFB thread.

  // Note that mutex MUST be held when hideLocalCursor() and showLocalCursor()
  // are called.

  private synchronized void hideLocalCursor() {
    if (cc.params.localCursor.get())
      return;
    if (!cc.params.cursorShape.get())
      setCursor(noCursor);
    // Blit the cursor backing store over the cursor.
    if (cursorVisible) {
      cursorVisible = false;
      im.imageRect(cursorBackingX, cursorBackingY, cursorBacking.width(),
                   cursorBacking.height(), cursorBacking.data);
    }
  }

  private synchronized void showLocalCursor() {
    if (cc.params.localCursor.get())
      return;
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

  // RFB thread
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

  // run() is executed by the setColourMapEntriesTimerThread.  It sleeps for
  // 100ms before actually updating the colourmap.
  public synchronized void run() {
    try {
      Thread.sleep(100);
    } catch (InterruptedException e) {}
    im.updateColourMap();
    setColourMapEntriesTimerThread = null;
  }

  // These methods, mostly borrowed from https://github.com/brackeen/Scared,
  // ensure that key input continues to work on Mac platforms even if
  // ApplePressAndHoldEnabled is true in the user defaults.
  //
  // Copyright (c) 1998-2012, David Brackeen
  // All rights reserved.
  //
  // Redistribution and use in source and binary forms, with or without
  // modification, are permitted provided that the following conditions are
  // met:
  //
  //  * Redistributions of source code must retain the above copyright notice,
  //    this list of conditions and the following disclaimer.
  //  * Redistributions in binary form must reproduce the above copyright
  //    notice, this list of conditions and the following disclaimer in the
  //    documentation and/or other materials provided with the distribution.
  //  * Neither the name of David Brackeen nor the names of its contributors
  //    may be used to endorse or promote products derived from this software
  //    without specific prior written permission.
  //
  // THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
  // IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  // THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  // PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
  // CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  // EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  // PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
  // PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  // LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  // NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  // SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  @Override
  public AttributedCharacterIterator cancelLatestCommittedText(
                          AttributedCharacterIterator.Attribute[] attributes) {
    return null;
  }

  @Override
  public AttributedCharacterIterator getCommittedText(int beginIndex,
            int endIndex, AttributedCharacterIterator.Attribute[] attributes) {
    return null;
  }

  @Override
  public int getCommittedTextLength() {
    return 0;
  }

  @Override
  public InputMethodRequests getInputMethodRequests() {
    if (Utils.isMac())
      return this;
    return null;
  }

  @Override
  public int getInsertPositionOffset() {
    return 0;
  }

  @Override
  public TextHitInfo getLocationOffset(int x, int y) {
    return null;
  }

  @Override
  public AttributedCharacterIterator getSelectedText(
                          AttributedCharacterIterator.Attribute[] attributes) {
    return (new AttributedString("")).getIterator();
  }

  @Override
  public Rectangle getTextLocation(TextHitInfo textHitInfo) {
    return new Rectangle(-32768, -32768, 0, 0);
  }

  // End borrowed code

  CConn cc;

  // Access to the following must be synchronized:
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
  boolean swingDB;

  int scaledWidth = 0, scaledHeight = 0;
  float scaleWidthRatio, scaleHeightRatio;

  int lastX, lastY;  // EDT only
  Rect damage = new Rect();

  static LogWriter vlog = new LogWriter("DesktopWindow");
}
