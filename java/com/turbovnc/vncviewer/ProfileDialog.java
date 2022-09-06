/* Copyright (C) 2013-2014, 2018, 2020 D. R. Commander.  All Rights Reserved.
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
import javax.swing.*;

import com.turbovnc.rfb.Utils;

class ProfileDialog extends Dialog {

  ProfileDialog(CConn cc_) {
    super(false);
    cc = cc_;

    panel = new JPanel(new GridBagLayout());

    JLabel recvHeading = new JLabel("Recv");
    Font font = recvHeading.getFont();
    Font boldFont = new Font(font.getFontName(), Font.BOLD, font.getSize());
    recvHeading.setFont(boldFont);
    JLabel decodeHeading = new JLabel("Decode");
    font = decodeHeading.getFont();
    boldFont = new Font(font.getFontName(), Font.BOLD, font.getSize());
    decodeHeading.setFont(boldFont);
    JLabel blitHeading = new JLabel("Blit");
    font = blitHeading.getFont();
    boldFont = new Font(font.getFontName(), Font.BOLD, font.getSize());
    blitHeading.setFont(boldFont);
    JLabel totalHeading = new JLabel("Total");
    font = totalHeading.getFont();
    boldFont = new Font(font.getFontName(), Font.BOLD, font.getSize());
    totalHeading.setFont(boldFont);

    JLabel upsHeading = new JLabel("Updates/sec:");
    font = upsHeading.getFont();
    boldFont = new Font(font.getFontName(), Font.BOLD, font.getSize());
    upsHeading.setFont(boldFont);
    upsVal = new JLabel("000.000");

    JLabel tpHeading = new JLabel("Throughput (Mbits/sec):");
    font = tpHeading.getFont();
    boldFont = new Font(font.getFontName(), Font.BOLD, font.getSize());
    tpHeading.setFont(boldFont);
    tpVal = new JLabel("000.000");

    JLabel tpuHeading = new JLabel("Time/update (ms):");
    font = tpuHeading.getFont();
    boldFont = new Font(font.getFontName(), Font.BOLD, font.getSize());
    tpuHeading.setFont(boldFont);
    tpuRecvVal = new JLabel("000.000");
    tpuDecodeVal = new JLabel("000.000");
    tpuBlitVal = new JLabel("000.000");
    tpuTotalVal = new JLabel("000.000");

    JLabel mpHeading = new JLabel("Mpixels:");
    font = mpHeading.getFont();
    boldFont = new Font(font.getFontName(), Font.BOLD, font.getSize());
    mpHeading.setFont(boldFont);
    mpDecodeVal = new JLabel("000.000");
    mpBlitVal = new JLabel("000.000");

    JLabel mpsHeading = new JLabel("Mpixels/sec:");
    font = mpsHeading.getFont();
    boldFont = new Font(font.getFontName(), Font.BOLD, font.getSize());
    mpsHeading.setFont(boldFont);
    mpsDecodeVal = new JLabel("000.000");
    mpsBlitVal = new JLabel("000.000");
    mpsTotalVal = new JLabel("000.000");

    JLabel rectHeading = new JLabel("Rectangles:");
    font = rectHeading.getFont();
    boldFont = new Font(font.getFontName(), Font.BOLD, font.getSize());
    rectHeading.setFont(boldFont);
    rectDecodeVal = new JLabel("0000000");
    rectBlitVal = new JLabel("0000000");

    JLabel pprHeading = new JLabel("Pixels/rect:");
    font = pprHeading.getFont();
    boldFont = new Font(font.getFontName(), Font.BOLD, font.getSize());
    pprHeading.setFont(boldFont);
    pprDecodeVal = new JLabel("0000000");
    pprBlitVal = new JLabel("0000000");

    JLabel rpuHeading = new JLabel("Rects/update:");
    font = rpuHeading.getFont();
    boldFont = new Font(font.getFontName(), Font.BOLD, font.getSize());
    rpuHeading.setFont(boldFont);
    rpuDecodeVal = new JLabel("0000000");

    Dialog.addGBComponent(recvHeading, panel,
                          1, 0, 1, 1, 0, 0, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 8, 2, 8));
    Dialog.addGBComponent(decodeHeading, panel,
                          2, 0, 1, 1, 0, 0, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.WEST,
                          new Insets(2, 8, 2, 8));
    Dialog.addGBComponent(blitHeading, panel,
                          3, 0, 1, 1, 0, 0, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.WEST,
                          new Insets(2, 8, 2, 8));
    Dialog.addGBComponent(totalHeading, panel,
                          4, 0, 1, 1, 0, 0, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.WEST,
                          new Insets(2, 8, 2, 8));

    Dialog.addGBComponent(upsHeading, panel,
                          0, 1, 1, 1, 0, 0, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 8, 2, 8));
    Dialog.addGBComponent(upsVal, panel,
                          4, 1, 1, 1, 0, 0, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.WEST,
                          new Insets(2, 8, 2, 8));

    Dialog.addGBComponent(tpHeading, panel,
                          0, 2, 1, 1, 0, 0, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 8, 2, 8));
    Dialog.addGBComponent(tpVal, panel,
                          4, 2, 1, 1, 0, 0, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.WEST,
                          new Insets(2, 8, 2, 8));

    Dialog.addGBComponent(tpuHeading, panel,
                          0, 3, 1, 1, 0, 0, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 8, 2, 8));
    Dialog.addGBComponent(tpuRecvVal, panel,
                          1, 3, 1, 1, 0, 0, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 8, 2, 8));
    Dialog.addGBComponent(tpuDecodeVal, panel,
                          2, 3, 1, 1, 0, 0, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 8, 2, 8));
    Dialog.addGBComponent(tpuBlitVal, panel,
                          3, 3, 1, 1, 0, 0, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 8, 2, 8));
    Dialog.addGBComponent(tpuTotalVal, panel,
                          4, 3, 1, 1, 0, 0, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 8, 2, 8));

    Dialog.addGBComponent(mpHeading, panel,
                          0, 4, 1, 1, 0, 0, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 8, 2, 8));
    Dialog.addGBComponent(mpDecodeVal, panel,
                          2, 4, 1, 1, 0, 0, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 8, 2, 8));
    Dialog.addGBComponent(mpBlitVal, panel,
                          3, 4, 1, 1, 0, 0, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 8, 2, 8));

    Dialog.addGBComponent(mpsHeading, panel,
                          0, 5, 1, 1, 0, 0, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 8, 2, 8));
    Dialog.addGBComponent(mpsDecodeVal, panel,
                          2, 5, 1, 1, 0, 0, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 8, 2, 8));
    Dialog.addGBComponent(mpsBlitVal, panel,
                          3, 5, 1, 1, 0, 0, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 8, 2, 8));
    Dialog.addGBComponent(mpsTotalVal, panel,
                          4, 5, 1, 1, 0, 0, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 8, 2, 8));

    Dialog.addGBComponent(rectHeading, panel,
                          0, 6, 1, 1, 0, 0, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 8, 2, 8));
    Dialog.addGBComponent(rectDecodeVal, panel,
                          2, 6, 1, 1, 0, 0, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 8, 2, 8));
    Dialog.addGBComponent(rectBlitVal, panel,
                          3, 6, 1, 1, 0, 0, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 8, 2, 8));

    Dialog.addGBComponent(pprHeading, panel,
                          0, 7, 1, 1, 0, 0, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 8, 2, 8));
    Dialog.addGBComponent(pprDecodeVal, panel,
                          2, 7, 1, 1, 0, 0, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 8, 2, 8));
    Dialog.addGBComponent(pprBlitVal, panel,
                          3, 7, 1, 1, 0, 0, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 8, 2, 8));

    Dialog.addGBComponent(rpuHeading, panel,
                          0, 8, 1, 1, 0, 0, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 8, 2, 8));
    Dialog.addGBComponent(rpuDecodeVal, panel,
                          2, 8, 1, 1, 0, 0, 0, 0,
                          GridBagConstraints.NONE,
                          GridBagConstraints.LINE_START,
                          new Insets(2, 8, 2, 8));

    panel.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
  }

  protected void populateDialog(JDialog dlg) {
    dlg.setTitle("TurboVNC profiling information");
    dlg.setResizable(false);
    dlg.getContentPane().add(panel);
    dlg.pack();

    dlg.addWindowListener(new WindowAdapter() {
      public void windowClosing(WindowEvent e) {
        endDialog();
        cc.toggleProfile();
      }
    });

    ActionListener actionListener = new ActionListener() {
      public void actionPerformed(ActionEvent actionEvent) {
        endDialog();
        cc.toggleProfile();
      }
    };
    KeyStroke ks = KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0);
    dlg.getRootPane().registerKeyboardAction(actionListener, ks,
      JComponent.WHEN_IN_FOCUSED_WINDOW);
    ks = KeyStroke.getKeyStroke(KeyEvent.VK_P,
      KeyEvent.CTRL_DOWN_MASK | KeyEvent.ALT_DOWN_MASK |
      KeyEvent.SHIFT_DOWN_MASK);
    dlg.getRootPane().registerKeyboardAction(actionListener, ks,
      JComponent.WHEN_IN_FOCUSED_WINDOW);
    // macOS always treats the RAlt key as an AltGr key.  For reasons that
    // aren't clearly understood, after RAlt is pressed for the first time,
    // the modifier mask will always contain KeyEvent.ALT_GRAPH_DOWN_MASK
    // and KeyEvent.ALT_DOWN_MASK whenever LAlt is pressed, so we need to
    // register a separate keyboard action to handle that situation.
    if (Utils.isMac()) {
      ks = KeyStroke.getKeyStroke(KeyEvent.VK_P,
        KeyEvent.CTRL_DOWN_MASK | KeyEvent.ALT_GRAPH_DOWN_MASK |
        KeyEvent.SHIFT_DOWN_MASK | KeyEvent.ALT_DOWN_MASK);
      dlg.getRootPane().registerKeyboardAction(actionListener, ks,
        JComponent.WHEN_IN_FOCUSED_WINDOW);
    }
    if (Utils.isMac() && (Utils.JAVA_VERSION < 9 || Utils.JAVA_VERSION > 11)) {
      int appleKey = VncViewer.getMenuShortcutKeyMask();
      ks = KeyStroke.getKeyStroke(KeyEvent.VK_P, appleKey);
      dlg.getRootPane().registerKeyboardAction(actionListener, ks,
        JComponent.WHEN_IN_FOCUSED_WINDOW);
    }
  }

  public boolean isVisible() {
    JDialog dlg = getJDialog();
    return (dlg != null && dlg.isVisible());
  }

  CConn cc;
  JPanel panel;
  JLabel upsVal, tpVal;
  JLabel tpuRecvVal, tpuDecodeVal, tpuBlitVal, tpuTotalVal;
  JLabel mpDecodeVal, mpBlitVal, mpsDecodeVal, mpsBlitVal, mpsTotalVal;
  JLabel rectDecodeVal, rectBlitVal, pprDecodeVal, pprBlitVal;
  JLabel rpuDecodeVal;
}
