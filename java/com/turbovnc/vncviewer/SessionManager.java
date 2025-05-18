/* Copyright (C) 2018, 2020-2025 D. R. Commander.  All Rights Reserved.
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

/*
 * SessionManager.java - TurboVNC Session Manager
 */

package com.turbovnc.vncviewer;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.*;
import javax.swing.*;

import com.turbovnc.rfb.*;
import com.turbovnc.rdr.*;
import com.turbovnc.network.*;

import com.jcraft.jsch.ChannelExec;

public final class SessionManager extends Tunnel {

  public static final int MAX_SESSIONS = 256;

  public static VncSession createSession(Params params) throws Exception {
    String host = Hostname.getHost(params.server.get());

    vlog.debug("Opening SSH connection to host " + host);
    VncViewer.noExceptionDialog =
      Utils.getBooleanProperty("turbovnc.sshkeytest", false);
    int sshPort = params.sshPort.isDefault() ? -1 : params.sshPort.get();
    // Via can be used as an alias for Jump if SessMgrAuto=1.  If
    // SessMgrAuto=0, then Via uses multi-level SSH tunneling for the Session
    // Manager's SSH connection, but it uses direct port forwarding for the
    // RFB/SSH connection.
    if (params.jump.get() != null)
      params.sshSession =
        createTunnelJSch(Hostname.getSSHUser(params.server.get()), host,
                         sshPort, Hostname.getSSHUser(params.jump.get()),
                         Hostname.getSSHHost(params.jump.get()),
                         Hostname.getSSHPort(params.jump.get()), params);
    else
      params.sshSession =
        createTunnelJSch(Hostname.getSSHUser(params.server.get()), host,
                         sshPort, Hostname.getSSHUser(params.via.get()),
                         Hostname.getSSHHost(params.via.get()), sshPort,
                         params);
    if (Utils.getBooleanProperty("turbovnc.sshkeytest", false)) {
      System.out.println("SSH SUCCEEDED");
      System.exit(0);
    }

    boolean firstTime = true;
    while (true) {
      VncSession[] sessions = getSessions(params, host);

      if (Utils.getBooleanProperty("turbovnc.autotest", false)) {
        int autotestSession =
          Utils.getIntProperty("turbovnc.autotestsession");
        if (autotestSession >= 0) {
          if (sessions == null || autotestSession >= sessions.length)
            throw new ErrorException("turbovnc.autotestsession is out of range");
          if (params.sessMgrAuto.get())
            generateOTP(params, host, sessions[autotestSession], true,
                        params.viewOnly.get());
          return sessions[autotestSession];
        }
      }

      if ((sessions == null || sessions.length <= 0) && firstTime) {
        return startSession(params, host);
      } else {
        SessionManagerDialog dlg = new SessionManagerDialog(sessions, host);
        dlg.initDialog();
        boolean ret = dlg.showDialog();
        if (!ret) return null;
        else if (dlg.getConnectSession() != null) {
          if (dlg.getConnectSession().display.equals("NEW"))
            return startSession(params, host);
          else {
            if (params.sessMgrAuto.get())
              generateOTP(params, host, dlg.getConnectSession(), true,
                          dlg.getViewOnly());
            return dlg.getConnectSession();
          }
        } else if (dlg.getKillSession() != null) {
          killSession(params, host, dlg.getKillSession());
        } else if (dlg.getNewOTPSession() != null) {
          String otp = generateOTP(params, host, dlg.getNewOTPSession(), false,
                                   dlg.getViewOnly());
          String msg = "New " +
                       (dlg.getViewOnly() ? "view-only " : "full control ") +
                       "one-time password:\n\n" + otp + "\n\n" +
                       "Click OK to copy OTP to clipboard.";
          int option =
            JOptionPane.showConfirmDialog(dlg.getJDialog(), msg,
                                          "New OTP for " + host +
                                            dlg.getNewOTPSession().display,
                                          JOptionPane.OK_CANCEL_OPTION,
                                          JOptionPane.INFORMATION_MESSAGE);
          if (option == JOptionPane.OK_OPTION)
            ClipboardDialog.setClipboard(otp);
        }
      }
      firstTime = false;
    }
  }

  private static VncSession[] getSessions(Params params, String host)
                                          throws Exception {
    ChannelExec channelExec =
      (ChannelExec)params.sshSession.openChannel("exec");

    String dir = params.serverDir.get();
    if (dir == null)
      dir = System.getProperty("turbovnc.serverdir");
    if (dir == null)
      dir = System.getenv("TVNC_SERVERDIR");

    String command = (dir != null ? dir : "") + "/bin/vncserver -sessionlist";
    channelExec.setCommand("bash -c \"set -o pipefail; " + command +
                           " | sed \'s/^/[TURBOVNC] /g\'\"");
    InputStream stdout = channelExec.getInputStream();
    InputStream stderr = channelExec.getErrStream();
    channelExec.connect();

    BufferedReader br = new BufferedReader(new InputStreamReader(stdout));
    String result, error = null;
    VncSession[] sessions = null;
    while ((result = br.readLine()) != null) {
      if (!result.startsWith("[TURBOVNC] ")) continue;
      result = result.replace("[TURBOVNC] ", "");
      if (error == null && result.length() > 0) error = result;
      String[] splitResult = result.split("\t");
      if (splitResult.length >= 2) {
        int numSessions = Integer.parseInt(splitResult[0]);
        int numFields = Integer.parseInt(splitResult[1]);
        if (numSessions > 0 && numFields > 0 &&
            splitResult.length == numSessions * numFields + 2) {
          sessions = new VncSession[numSessions];
          int sessionIndex = 0;
          for (int index = 2; index < splitResult.length; index += numFields)
            sessions[sessionIndex++] =
              new VncSession(splitResult[index], null);
          if (numFields > 3) {
            sessionIndex = 0;
            for (int index = 5; index < splitResult.length;
                 index += numFields) {
              if (!splitResult[index].equals("@NONE"))
                sessions[sessionIndex].udsPath = splitResult[index];
              sessionIndex++;
            }
          }
          if (numFields > 4) {
            sessionIndex = 0;
            for (int index = 6; index < splitResult.length;
                 index += numFields) {
              int temp = Integer.parseInt(splitResult[index]);
              if (temp >= 0)
                sessions[sessionIndex].sessionLimit = temp;
              sessionIndex++;
            }
          }
        }
      }
      break;
    }

    br = new BufferedReader(new InputStreamReader(stderr));
    int nLines = 0;
    while ((result = br.readLine()) != null && nLines < 20) {
      if (error == null) error = result;
      if (nLines == 0) {
        vlog.debug("===============================================================================");
        vlog.debug("SERVER WARNINGS/NOTIFICATIONS:");
      }
      vlog.debug(result);
      nLines++;
    }
    if (nLines > 0)
      vlog.debug("===============================================================================");

    channelExec.disconnect();

    if (channelExec.getExitStatus() == 127) {
      throw new ErrorException("Could not execute\n    " + command + "\n" +
                               "on host " + host + ".\n" +
                               "Is the TurboVNC Server installed in " + dir +
                               " ?");
    } else if (channelExec.getExitStatus() != 0) {
      throw new ErrorException("Could not execute\n    " + command + "\n" +
                               "on host " + host +
                               (error != null ? ":\n    " + error : ""));
    }

    StringBuilder sb = null;
    if (sessions != null) {
      sb = new StringBuilder();
      for (VncSession s : sessions)
        sb.append(s.display + " ");
    }
    vlog.debug("Available sessions: " + (sb != null ? sb.toString() : "None"));

    return sessions;
  }

  private static VncSession startSession(Params params, String host)
                                         throws Exception {
    vlog.debug("Starting new TurboVNC session on host " + host);

    ChannelExec channelExec =
      (ChannelExec)params.sshSession.openChannel("exec");

    String dir = params.serverDir.get();
    if (dir == null)
      dir = System.getProperty("turbovnc.serverdir");
    if (dir == null)
      dir = System.getenv("TVNC_SERVERDIR");

    String args = params.serverArgs.get();
    if (args == null)
      args = System.getProperty("turbovnc.serverargs");
    if (args == null)
      args = System.getenv("TVNC_SERVERARGS");

    String command = (dir != null ? dir : "") +
                     "/bin/vncserver -sessionstart" +
                     (params.sessMgrAuto.get() ? " -securitytypes otp" : "") +
                     (args != null ? " " + args : "");
    channelExec.setCommand("bash -c \"set -o pipefail; " + command +
                           " | sed \'s/^/[TURBOVNC] /g\'\"");
    InputStream stdout = channelExec.getInputStream();
    InputStream stderr = channelExec.getErrStream();
    channelExec.connect();

    BufferedReader br = new BufferedReader(new InputStreamReader(stdout));
    String result, error = null;
    VncSession[] sessions = null;
    while ((result = br.readLine()) != null) {
      if (!result.startsWith("[TURBOVNC] ")) continue;
      result = result.replace("[TURBOVNC] ", "");
      if (error == null && result.length() > 0 &&
          !result.startsWith("TurboVNC Server (Xvnc)"))
        error = result;
      String[] splitResult = result.split("\t");
      if (splitResult.length >= 2) {
        int numSessions = Integer.parseInt(splitResult[0]);
        int numFields = Integer.parseInt(splitResult[1]);
        if (numSessions > 0 && numFields > 0 &&
            splitResult.length == numSessions * numFields + 2) {
          sessions = new VncSession[numSessions];
          int sessionIndex = 0;
          for (int index = 2; index < splitResult.length; index += numFields)
            sessions[sessionIndex++] =
              new VncSession(splitResult[index], null);
          if (numFields > 3) {
            sessionIndex = 0;
            for (int index = 5; index < splitResult.length;
                 index += numFields) {
              if (!splitResult[index].equals("@NONE"))
                sessions[sessionIndex].udsPath = splitResult[index];
              sessionIndex++;
            }
          }
        }
      }
      break;
    }

    br = new BufferedReader(new InputStreamReader(stderr));
    int nLines = 0;
    while ((result = br.readLine()) != null && nLines < 20) {
      if (error == null) error = result;
      if (nLines == 0) {
        vlog.debug("===============================================================================");
        vlog.debug("SERVER WARNINGS/NOTIFICATIONS:");
      }
      vlog.debug(result);
      nLines++;
    }
    if (nLines > 0)
      vlog.debug("===============================================================================");

    channelExec.disconnect();

    if (channelExec.getExitStatus() == 127) {
      throw new ErrorException("Could not execute\n    " + command + "\n" +
                               "on host " + host + ".\n" +
                               "Is the TurboVNC Server installed in " + dir +
                               " ?");
    } else if (channelExec.getExitStatus() != 0) {
      throw new ErrorException("Could not execute\n    " + command + "\n" +
                               "on host " + host +
                               (error != null ? ":\n    " + error : ""));
    }

    if (sessions == null)
      throw new ErrorException("Could not parse TurboVNC Server output");

    if (params.sessMgrAuto.get())
      generateOTP(params, host, sessions[0], true, false);

    return sessions[0];
  }

  @SuppressWarnings("checkstyle:EmptyBlock")
  private static String generateOTP(Params params, String host,
                                    VncSession session, boolean setPassword,
                                    boolean viewOnly) throws Exception {
    vlog.debug("Generating one-time password for session " + host +
               session.display);

    if (!Utils.getBooleanProperty("turbovnc.autotest", false))
      VncViewer.noExceptionDialog = true;

    ChannelExec channelExec =
      (ChannelExec)params.sshSession.openChannel("exec");

    String dir = params.serverDir.get();
    if (dir == null)
      dir = System.getProperty("turbovnc.serverdir");
    if (dir == null)
      dir = System.getenv("TVNC_SERVERDIR");

    String command = (dir != null ? dir : "") + "/bin/vncpasswd -o -display " +
                     session.display;
    if (viewOnly) command += " -v";
    channelExec.setCommand("bash -c \"set -o pipefail; " + command +
                           " 2>&1 | sed \'s/^/[TURBOVNC] /g\'\"");
    InputStream stdout = channelExec.getInputStream();
    InputStream stderr = channelExec.getErrStream();
    channelExec.connect();

    BufferedReader br = new BufferedReader(new InputStreamReader(stdout));
    String result = null, error = null, line;
    int nLines = 0;
    while ((line = br.readLine()) != null && nLines < 20) {
      if (!line.startsWith("[TURBOVNC] ")) continue;
      line = line.replace("[TURBOVNC] ", "");
      if (result == null && line.length() > 0) result = line;
      if (viewOnly && result.matches("Full control one-time password:.*"))
        result = null;
      if (error == null && line.length() > 0) error = line;
      if (nLines == 0) {
        vlog.debug("===============================================================================");
        vlog.debug("SERVER WARNINGS/NOTIFICATIONS:");
      }
      vlog.debug(line);
      nLines++;
    }
    if (nLines > 0)
      vlog.debug("===============================================================================");

    // No idea why this is necessary, but we sometimes get an incorrect exit
    // status from Solaris 11 hosts without it.
    br = new BufferedReader(new InputStreamReader(stderr));
    while ((line = br.readLine()) != null) { }

    if (result != null) {
      result = result.replaceAll("\\s", "");
      result = result.replaceAll("^.*:", "");
      if (setPassword) params.password.set(result);
    }

    channelExec.disconnect();

    VncViewer.noExceptionDialog = false;

    if (channelExec.getExitStatus() == 127) {
      throw new ErrorException("Could not execute\n    " + command + "\n" +
                               "on host " + host + ".\n" +
                               "Is the TurboVNC Server installed in " + dir +
                               " ?");
    } else if (channelExec.getExitStatus() != 0) {
      throw new ErrorException("Could not execute\n    " + command + "\n" +
                               "on host " + host +
                               (error != null ? ":\n    " + error : ""));
    }

    return result;
  }

  private static void killSession(Params params, String host,
                                  VncSession session) throws Exception {
    vlog.debug("Killing TurboVNC session " + host + session.display);

    if (!Utils.getBooleanProperty("turbovnc.autotest", false))
      VncViewer.noExceptionDialog = true;

    ChannelExec channelExec =
      (ChannelExec)params.sshSession.openChannel("exec");

    String dir = params.serverDir.get();
    if (dir == null)
      dir = System.getProperty("turbovnc.serverdir");
    if (dir == null)
      dir = System.getenv("TVNC_SERVERDIR");

    String command = (dir != null ? dir : "") + "/bin/vncserver -kill " +
                     session.display;
    channelExec.setCommand("bash -c \"set -o pipefail; " + command +
                           " | sed \'s/^/[TURBOVNC] /g\'\"");
    InputStream stdout = channelExec.getInputStream();
    InputStream stderr = channelExec.getErrStream();
    channelExec.connect();

    BufferedReader br = new BufferedReader(new InputStreamReader(stdout));
    String result, error = null;
    while ((result = br.readLine()) != null) {
      if (!result.startsWith("[TURBOVNC] ")) continue;
      result = result.replace("[TURBOVNC] ", "");
      if (error == null && result.length() > 0) error = result;
      break;
    }

    br = new BufferedReader(new InputStreamReader(stderr));
    int nLines = 0;
    while ((result = br.readLine()) != null && nLines < 20) {
      if (error == null && result.length() > 0)
        error = result;
      if (nLines == 0) {
        vlog.debug("===============================================================================");
        vlog.debug("SERVER WARNINGS/NOTIFICATIONS:");
      }
      vlog.debug(result);
      nLines++;
    }
    if (nLines > 0)
      vlog.debug("===============================================================================");

    channelExec.disconnect();

    VncViewer.noExceptionDialog = false;

    if (channelExec.getExitStatus() == 127) {
      throw new ErrorException("Could not execute\n    " + command + "\n" +
                               "on host " + host + ".\n" +
                               "Is the TurboVNC Server installed in " + dir +
                               " ?");
    } else if (channelExec.getExitStatus() != 0) {
      throw new ErrorException("Could not execute\n    " + command + "\n" +
                               "on host " + host +
                               (error != null ? ":\n    " + error : ""));
    }
  }

  private SessionManager() {}
  static LogWriter vlog = new LogWriter("SessionManager");
}
