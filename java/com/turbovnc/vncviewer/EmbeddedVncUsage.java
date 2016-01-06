package com.turbovnc.vncviewer;

import java.awt.BorderLayout;

import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.SwingUtilities;

public class EmbeddedVncUsage {

  public static void main(String[] args) {
    SwingUtilities.invokeLater(new Runnable() {

      @Override
      public void run() {
        JFrame frame = new JFrame();
        JPanel vncPanel = new JPanel();
        new VncViewer(new String[] { "host:display", "toolbar=false" }, vncPanel).start();
        frame.getContentPane().add(vncPanel, BorderLayout.CENTER);
        frame.getContentPane().add(new JLabel("Custom Component"), BorderLayout.SOUTH);
        frame.setSize(600, 600);
        frame.setVisible(true);
      }
    });
  }
}
