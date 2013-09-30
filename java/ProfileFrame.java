/* Copyright (C) 2013 D. R. Commander.  All Rights Reserved.
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

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

class ProfileFrame extends Frame
  implements ActionListener, KeyListener, WindowListener {

  public JLabel upsVal, tpVal;
  public JLabel mpDecodeVal, mpBlitVal, mpsTotalVal;
  public JLabel rectDecodeVal, rectBlitVal, pprDecodeVal, pprBlitVal;
  public JLabel rpuDecodeVal;
  VncViewer viewer;
  Button closeButton;

  public ProfileFrame(VncViewer v) {
    super("TurboVNC profiling information");

    viewer = v;
    setResizable(false);

    GridBagLayout gbl = new GridBagLayout();
    setLayout(gbl);

    JLabel decodeHeading = new JLabel("Decode");
    Font font = decodeHeading.getFont();
    Font boldFont = new Font(font.getFontName(), Font.BOLD, font.getSize());
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

    addGBComponent(decodeHeading, gbl,
                   1, 0, 1, 1, 0, 0, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.WEST,
                   new Insets(12, 8, 2, 8));
    addGBComponent(blitHeading, gbl,
                   2, 0, 1, 1, 0, 0, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.WEST,
                   new Insets(12, 8, 2, 8));
    addGBComponent(totalHeading, gbl,
                   3, 0, 1, 1, 0, 0, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.WEST,
                   new Insets(12, 8, 2, 18));

    addGBComponent(upsHeading, gbl,
                   0, 1, 1, 1, 0, 0, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 18, 2, 8));
    addGBComponent(upsVal, gbl,
                   3, 1, 1, 1, 0, 0, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.WEST,
                   new Insets(2, 8, 2, 18));

    addGBComponent(tpHeading, gbl,
                   0, 2, 1, 1, 0, 0, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 18, 2, 8));
    addGBComponent(tpVal, gbl,
                   3, 2, 1, 1, 0, 0, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.WEST,
                   new Insets(2, 8, 2, 8));

    addGBComponent(mpHeading, gbl,
                   0, 3, 1, 1, 0, 0, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 18, 2, 8));
    addGBComponent(mpDecodeVal, gbl,
                   1, 3, 1, 1, 0, 0, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 8, 2, 8));
    addGBComponent(mpBlitVal, gbl,
                   2, 3, 1, 1, 0, 0, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 8, 2, 8));

    addGBComponent(mpsHeading, gbl,
                   0, 4, 1, 1, 0, 0, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 18, 2, 8));
    addGBComponent(mpsTotalVal, gbl,
                   3, 4, 1, 1, 0, 0, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 8, 2, 18));

    addGBComponent(rectHeading, gbl,
                   0, 5, 1, 1, 0, 0, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 18, 2, 8));
    addGBComponent(rectDecodeVal, gbl,
                   1, 5, 1, 1, 0, 0, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 8, 2, 8));
    addGBComponent(rectBlitVal, gbl,
                   2, 5, 1, 1, 0, 0, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 8, 2, 8));

    addGBComponent(pprHeading, gbl,
                   0, 6, 1, 1, 0, 0, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 18, 2, 8));
    addGBComponent(pprDecodeVal, gbl,
                   1, 6, 1, 1, 0, 0, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 8, 2, 8));
    addGBComponent(pprBlitVal, gbl,
                   2, 6, 1, 1, 0, 0, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 8, 2, 8));

    addGBComponent(rpuHeading, gbl,
                   0, 7, 1, 1, 0, 0, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 18, 12, 8));
    addGBComponent(rpuDecodeVal, gbl,
                   1, 7, 1, 1, 0, 0, 0, 0,
                   GridBagConstraints.NONE,
                   GridBagConstraints.LINE_START,
                   new Insets(2, 8, 12, 8));

    pack();
    addKeyListener(this);
    addWindowListener(this);
  }

  public void addGBComponent(Component c, GridBagLayout gbl,
                             int gx, int gy,
                             int gw, int gh,
                             int gipx, int gipy,
                             double gwx, double gwy,
                             int fill, int anchor,
                             Insets insets) {
      GridBagConstraints gbc = new GridBagConstraints();
      gbc.anchor = anchor;
      gbc.fill = fill;
      gbc.gridx = gx;
      gbc.gridy = gy;
      gbc.gridwidth = gw;
      gbc.gridheight = gh;
      gbc.insets = insets;
      gbc.ipadx = gipx;
      gbc.ipady = gipy;
      gbc.weightx = gwx;
      gbc.weighty = gwy;
      gbl.setConstraints(c, gbc);
      add(c);
  }

  public void actionPerformed(ActionEvent evt) {
    if (evt.getSource() == closeButton)
      setVisible(false);
  }

  public void keyPressed(KeyEvent evt) {
    if (evt.getKeyCode() == KeyEvent.VK_ESCAPE) {
      setVisible(false);
    }
  }

  public void keyReleased(KeyEvent evt) {
  }

  public void keyTyped(KeyEvent evt) {
  }

  public void windowClosing(WindowEvent evt) {
    setVisible(false);
  }

  public void windowActivated(WindowEvent evt) {}
  public void windowClosed(WindowEvent evt) {}
  public void windowDeactivated(WindowEvent evt) {}
  public void windowDeiconified(WindowEvent evt) {}
  public void windowIconified(WindowEvent evt) {}
  public void windowOpened(WindowEvent evt) {}

}
