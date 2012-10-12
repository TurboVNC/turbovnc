/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright (C) 2011-2012 Brian P. Hinz
 * Copyright (C) 2012 D. R. Commander.  All Rights Reserved.
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

package com.turbovnc.vncviewer;

import java.awt.*;
import java.awt.event.*;
import java.awt.GraphicsConfiguration;
import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.awt.Window;
import javax.swing.*;

import com.turbovnc.rdr.*;
import com.turbovnc.rfb.*;
import com.turbovnc.rfb.Cursor;

public class Viewport extends JFrame
{
  public Viewport(CConn cc_) {
    cc = cc_;
    updateTitle();
    setFocusable(false);
    setFocusTraversalKeysEnabled(false);
    UIManager.getDefaults().put("ScrollPane.ancestorInputMap", 
      new UIDefaults.LazyInputMap(new Object[]{}));
    sp = new JScrollPane();
    sp.setBorder(BorderFactory.createEmptyBorder(0,0,0,0));
    InputMap im = sp.getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW);
    int ctrlAltShiftMask = Event.SHIFT_MASK | Event.CTRL_MASK | Event.ALT_MASK;
    if (im != null) {
      im.put(KeyStroke.getKeyStroke(KeyEvent.VK_UP, ctrlAltShiftMask),
             "unitScrollUp");
      im.put(KeyStroke.getKeyStroke(KeyEvent.VK_DOWN, ctrlAltShiftMask),
             "unitScrollDown");
      im.put(KeyStroke.getKeyStroke(KeyEvent.VK_LEFT, ctrlAltShiftMask),
             "unitScrollLeft");
      im.put(KeyStroke.getKeyStroke(KeyEvent.VK_RIGHT, ctrlAltShiftMask),
             "unitScrollRight");
    }
    tb = new Toolbar(cc);
    add(tb, BorderLayout.PAGE_START);
    showToolbar(cc.showToolbar);
    getContentPane().add(sp);
    if (cc.viewer.os.startsWith("mac os x")) {
      setJMenuBar(new MacMenuBar(cc));
      // NOTE: not sure why this is necessary, but the toolbar appears with a
      // black background otherwise.
      tb.setBackground(new Color(240, 240, 240, 255));
    }
    addWindowFocusListener(new WindowAdapter() {
      public void windowGainedFocus(WindowEvent e) {
        sp.getViewport().getView().requestFocusInWindow();
      }
    });
    addWindowListener(new WindowAdapter() {
      public void windowClosing(WindowEvent e) {
        cc.close();
      }
    });
    addComponentListener(new ComponentAdapter() {
      public void componentResized(ComponentEvent e) {
        if (cc.opts.scalingFactor == Options.SCALE_AUTO ||
            cc.opts.scalingFactor == Options.SCALE_FIXEDRATIO) {
          if ((sp.getSize().width != cc.desktop.scaledWidth) ||
              (sp.getSize().height != cc.desktop.scaledHeight)) {
            cc.desktop.setScaledSize();
            sp.setHorizontalScrollBarPolicy(ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
            sp.setVerticalScrollBarPolicy(ScrollPaneConstants.VERTICAL_SCROLLBAR_NEVER);
            sp.validate();
            if (getExtendedState() != JFrame.MAXIMIZED_BOTH &&
                !cc.opts.fullScreen) {
              sp.setSize(new Dimension(cc.desktop.scaledWidth,
                                       cc.desktop.scaledHeight));
              int w = cc.desktop.scaledWidth + getInsets().left + getInsets().right;
              int h = cc.desktop.scaledHeight + getInsets().top + getInsets().bottom;
              if (tb.isVisible())
                h += tb.getHeight();
              if (cc.opts.scalingFactor == Options.SCALE_FIXEDRATIO)
                setSize(w, h);
            }
            if (cc.desktop.cursor != null) {
              Cursor cursor = cc.desktop.cursor;
              cc.setCursor(cursor.width(),cursor.height(),cursor.hotspot, 
                           (int[])cursor.data, cursor.mask);
            }
          }
        } else {
          sp.setHorizontalScrollBarPolicy(ScrollPaneConstants.HORIZONTAL_SCROLLBAR_AS_NEEDED);
          sp.setVerticalScrollBarPolicy(ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED);
          if (sp.getSize().width >= cc.desktop.scaledWidth)
            sp.setHorizontalScrollBarPolicy(ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
          if (sp.getSize().height >= cc.desktop.scaledHeight)
            sp.setVerticalScrollBarPolicy(ScrollPaneConstants.VERTICAL_SCROLLBAR_NEVER);
          sp.validate();
        }
        if ((sp.getSize().width > cc.desktop.scaledWidth) ||
            (sp.getSize().height > cc.desktop.scaledHeight)) {
          dx = (sp.getSize().width <= cc.desktop.scaledWidth) ? 0 :
            (int)Math.floor((sp.getSize().width - cc.desktop.scaledWidth) / 2);
          dy = (sp.getSize().height <= cc.desktop.scaledHeight) ? 0 :
            (int)Math.floor((sp.getSize().height - cc.desktop.scaledHeight) / 2);
        } else {
          dx = dy = 0;
        }
        repaint();
      }
    });
  }

  public void setChild(DesktopWindow child) {
    sp.getViewport().setView(child);
  }

  public void setGeometry(int x, int y, int w, int h, boolean pack) {
    if (pack) {
      pack();
    } else {
      setSize(w, h);
    }
    setLocation(x, y);
    setBackground(Color.BLACK);
  }

  public void showToolbar(boolean show) {
    tb.setVisible(show && !cc.opts.fullScreen);
  }

  public void updateTitle() {
    int enc = cc.lastServerEncoding;
    if (enc < 0) enc = cc.currentEncoding;
    if (enc == Encodings.encodingTight) {
      if (cc.opts.allowJpeg) {
        String subsampStr[] = { "1X", "4X", "2X", "Gray" };
        setTitle(cc.cp.name() + " [Tight + JPEG " +
                 subsampStr[cc.opts.subsampling] + " Q" + cc.opts.quality +
                 (cc.opts.compressLevel > 1 ? " + CL " + cc.opts.compressLevel : "") +
                 "]");
      } else {
        setTitle(cc.cp.name() + " [Lossless Tight" +
                 (cc.opts.compressLevel == 1 ? " + Zlib" : "") +
                 (cc.opts.compressLevel > 1 ? " + CL " + cc.opts.compressLevel : "") +
                 "]");
      }
    } else {
      setTitle(cc.cp.name() + " [" + Encodings.encodingName(enc) + "]");
    }
  }


  CConn cc;
  JScrollPane sp;
  public Toolbar tb;
  public int dx, dy = 0;
  static LogWriter vlog = new LogWriter("Viewport");
}

