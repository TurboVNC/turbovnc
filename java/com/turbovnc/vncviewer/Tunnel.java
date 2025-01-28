/* Copyright (C) 2012-2015, 2017-2018, 2020-2025 D. R. Commander.
 *                                               All Rights Reserved.
 * Copyright (C) 2021 Steffen Kieß
 * Copyright (C) 2012, 2016 Brian P. Hinz.  All Rights Reserved.
 * Copyright (C) 2000 Const Kaplinsky.  All Rights Reserved.
 * Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
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
 * Tunnel.java - SSH tunneling support
 */

package com.turbovnc.vncviewer;

import java.io.*;
import java.util.*;

import com.turbovnc.rfb.*;
import com.turbovnc.rdr.*;
import com.turbovnc.network.*;

import com.jcraft.jsch.agentproxy.*;
import com.jcraft.jsch.agentproxy.connector.*;
import com.jcraft.jsch.agentproxy.usocket.*;
import com.jcraft.jsch.Identity;
import com.jcraft.jsch.*;

public class Tunnel {

  public static void createTunnel(Params params) throws Exception {
    int vncPort;

    if (params.server.get() != null &&
        Hostname.getColonPos(params.server.get()) < 0 && params.port.get() > 0)
      vncPort = params.port.get();
    else
      vncPort = Hostname.getPort(params.server.get());

    String pattern = params.extSSHTemplate.get();
    if (pattern == null) {
      if (params.via.get() != null) {
        pattern = System.getProperty("turbovnc.via");
        if (pattern == null)
          pattern = System.getenv("VNC_VIA_CMD");
      } else {
        pattern = System.getProperty("turbovnc.tunnel");
        if (pattern == null)
          pattern = System.getenv("VNC_TUNNEL_CMD");
      }
    }

    if (params.udsPath != null &&
        (pattern == null || pattern.indexOf("%L") < 0)) {
      // Connect to Unix domain socket using stdio-based forwarding
      params.stdioSocket = createTunnelExtUDS(pattern, params);
    } else {
      int localPort = TcpSocket.findFreeTcpPort();
      if (localPort == 0)
        throw new ErrorException("Could not obtain free TCP port");

      if (params.extSSH.get() || (pattern != null && pattern.length() > 0))
        createTunnelExt(vncPort, localPort, pattern, params);
      else {
        String vncHost, gatewayStr;
        int sshPort = params.sshPort.isDefault() ? -1 : params.sshPort.get();

        if (params.jump.get() != null && !params.tunnel.get()) {
          gatewayStr = Hostname.getHost(params.server.get());
          if (params.sshSession == null) {
            vncHost = gatewayStr;
            vlog.debug("Opening SSH connection to host " + vncHost);
            params.sshSession =
              createTunnelJSch(Hostname.getSSHUser(params.server.get()),
                               vncHost, sshPort,
                               Hostname.getSSHUser(params.jump.get()),
                               Hostname.getSSHHost(params.jump.get()),
                               Hostname.getSSHPort(params.jump.get()), params);
          }
          vncHost = "localhost";
        } else {
          String gatewaySSHUser, gatewayHost;

          boolean tunnel = params.tunnel.get() ||
                           (params.sessMgrActive && params.sessMgrAuto.get());

          if (tunnel) {
            gatewaySSHUser = Hostname.getSSHUser(params.server.get());
            gatewayHost = Hostname.getHost(params.server.get());
            vncHost = "localhost";
          } else {
            gatewaySSHUser = Hostname.getSSHUser(params.via.get());
            gatewayHost = Hostname.getHost(params.via.get());
            vncHost = Hostname.getHost(params.server.get());
            // If the Session Manager is active and SessMgrAuto=0, then Via
            // uses multi-level SSH tunneling for the Session Manager's SSH
            // connection, but it uses direct port forwarding for the RFB/SSH
            // connection.  Thus, close the Session Manager's multi-level SSH
            // connection so a new single-level SSH connection can be created
            // below.
            if (params.sessMgrActive && params.sshSession != null) {
              params.sshSession.disconnect();
              params.sshSession = null;
            }
          }

          if (params.sshSession == null) {
            vlog.debug("Opening SSH tunnel through gateway " + gatewayHost);
            params.sshSession = createTunnelJSch(gatewaySSHUser, gatewayHost,
                                                 sshPort, null, null, -1,
                                                 params);
          }
          vncHost = vncHost.replaceAll("[\\[\\]]", "");
          gatewayStr = gatewayHost;
        }
        vlog.debug("Forwarding local port " + localPort + " to " + vncHost +
                   "::" + vncPort + " (relative to " + gatewayStr + ")");
        params.sshSession.setPortForwardingL(localPort, vncHost, vncPort);
      }
      params.server.set("localhost::" + localPort);
    }
    params.sshTunnelActive = true;
  }

  /* Create a tunnel using the built-in JSch SSH client */

  private static class JumpProxy implements Proxy {
    Session jumpSSHSession;
    ChannelDirectTCPIP channel;
    PipedOutputStream osToRemote;
    PipedInputStream isFromRemote;

    JumpProxy(Session jumpSSHSession_) {
      jumpSSHSession = jumpSSHSession_;
    }

    public void connect(SocketFactory socket_factory, String host, int port,
                        int timeout) throws Exception {
      channel = (ChannelDirectTCPIP)jumpSSHSession.openChannel("direct-tcpip");
      channel.setHost(host);
      channel.setPort(port);
      PipedInputStream isToRemote = new PipedInputStream();
      osToRemote = new PipedOutputStream(isToRemote);
      isFromRemote = new PipedInputStream();
      PipedOutputStream osFromRemote = new PipedOutputStream(isFromRemote);
      channel.setInputStream(isToRemote);
      channel.setOutputStream(osFromRemote);
      channel.connect();
    }

    public InputStream getInputStream() {
      return isFromRemote;
    }

    public OutputStream getOutputStream() {
      return osToRemote;
    }

    public java.net.Socket getSocket() {
      return null;
    }

    public void close() {
      if (channel != null)
        channel.disconnect();
      channel = null;
      isFromRemote = null;
      osToRemote = null;
      jumpSSHSession.disconnect();
    }
  }

  protected static Session createTunnelJSch(String user, String host, int port,
                                            String jumpUser, String jumpHost,
                                            int jumpPort, Params params)
                                            throws Exception {
    JSch jsch = new JSch();
    JSch.setLogger(LOGGER);
    String homeDir = new String("");
    try {
      homeDir = System.getProperty("user.home");
    } catch (Exception e) {
      System.err.println("Cannot access user.home system property");
    }

    // NOTE: JSch does not support all ciphers.  User may be prompted to accept
    //       the authenticity of the host key even if the key is in the
    //       known_hosts file.

    File knownHosts = new File(homeDir + "/.ssh/known_hosts");
    jsch.setKnownHosts(knownHosts.getAbsolutePath());

    if (Helper.isAvailable()) {
      Connector connector = null;
      try {
        if (Utils.isWindows())
          connector = new PageantConnector();
        else
          connector = new SSHAgentConnector(new JNIUSocketFactory());
        if (connector != null) {
          IdentityRepository repo = new RemoteIdentityRepository(connector);
          vlog.sshdebug("SSH private keys offered by agent:");
          Iterator<Identity> iter = repo.getIdentities().iterator();
          while (iter.hasNext()) {
            Identity id = iter.next();
            vlog.sshdebug("  " + id.getName());
            if (id.getFingerPrint() != null)
              vlog.sshdebug("    Fingerprint: " + id.getFingerPrint());
          }
          LocalIdentityRepository localRepo =
            (LocalIdentityRepository)jsch.getIdentityRepository();
          localRepo.copyFrom(repo);
        }
      } catch (Exception e) {
        if (Utils.isWindows())
          vlog.debug("Could not contact Pageant:\n        " + e.getMessage());
        else
          vlog.debug("Could not contact ssh-agent:\n        " +
                     e.getMessage());
      }
    }

    ArrayList<File> privateKeys = new ArrayList<File>();
    String sshKeyFile = params.sshKeyFile.get();
    String sshKey = params.sshKey.get();
    boolean useDefaultPrivateKeyFiles = false;
    if (sshKey != null) {
      String sshKeyPass = params.sshKeyPass.get();
      byte[] keyPass = null, key;
      if (sshKeyPass != null)
        keyPass = sshKeyPass.getBytes();
      sshKey = sshKey.replaceAll("\\\\n", "\n");
      key = sshKey.getBytes();
      jsch.addIdentity("TurboVNC", key, null, keyPass);
    } else if (sshKeyFile != null) {
      File f = new File(sshKeyFile);
      if (!f.exists() || !f.canRead())
        throw new ErrorException("Cannot access private SSH key file " +
                                 sshKeyFile);
      privateKeys.add(f);
    } else {
      useDefaultPrivateKeyFiles = true;
    }

    // username and passphrase will be given via UserInfo interface.
    File sshConfigFile = new File(params.sshConfig.get());
    if (sshConfigFile.exists() && sshConfigFile.canRead()) {
      ConfigRepository repo =
        OpenSSHConfig.parseFile(sshConfigFile.getAbsolutePath());
      jsch.setConfigRepository(repo);
      vlog.debug("Read OpenSSH config file " + params.sshConfig.get());
      String repoUser = repo.getConfig(host).getUser();
      if (repoUser != null && user == null)
        user = repoUser;
      int repoPort = repo.getConfig(host).getPort();
      if (repoPort != -1 && port == -1)
        port = repoPort;
      String[] identityFiles = repo.getConfig(host).getValues("IdentityFile");
      if (identityFiles != null) {
        for (String file : identityFiles) {
          if (file != null && !file.isEmpty())
            useDefaultPrivateKeyFiles = false;
        }
      }

      String repoJumpHost = repo.getConfig(host).getValue("ProxyJump");
      if (repoJumpHost != null && jumpHost == null) {
        jumpHost = repoJumpHost.replaceAll("\\s", "");
        jumpUser = Hostname.getSSHUser(jumpHost);
        jumpPort = Hostname.getSSHPort(jumpHost);
        jumpHost = Hostname.getSSHHost(jumpHost);
      }
    } else {
      if (params.sshConfig.isDefault()) {
        vlog.debug("Could not parse SSH config file " +
                   params.sshConfig.get());
      } else {
        vlog.info("Could not parse SSH config file " + params.sshConfig.get());
      }
    }

    if (user == null) {
      user = (String)System.getProperties().get("user.name");
      if (params.localUsernameLC.get())
        user = user.toLowerCase();
    }

    if (useDefaultPrivateKeyFiles) {
      privateKeys.add(new File(homeDir + "/.ssh/id_rsa"));
      privateKeys.add(new File(homeDir + "/.ssh/id_ecdsa"));
      privateKeys.add(new File(homeDir + "/.ssh/id_ed25519"));
      privateKeys.add(new File(homeDir + "/.ssh/id_dsa"));
    }

    for (Iterator<File> i = privateKeys.iterator(); i.hasNext();) {
      File privateKey = i.next();
      try {
        if (privateKey.exists() && privateKey.canRead()) {
          if (params.sshKeyPass.get() != null)
            jsch.addIdentity(privateKey.getAbsolutePath(),
                             params.sshKeyPass.get());
          else
            jsch.addIdentity(privateKey.getAbsolutePath());
        }
      } catch (Exception e) {
        throw new ErrorException("Could not use SSH private key " +
                                 privateKey.getAbsolutePath() + ":\n" +
                                 e.getMessage());
      }
    }

    Session sshSession = null, jumpSSHSession = null;

    if (jumpHost != null) {
      vlog.debug("Opening SSH connection to jump host " + jumpHost);
      jumpSSHSession = createTunnelJSch(jumpUser, jumpHost, jumpPort, null,
                                        null, -1, params);
    }
    if (port == -1) port = params.sshPort.getDefault();
    sshSession = jsch.getSession(user, host, port);
    if (jumpSSHSession != null) {
      JumpProxy proxy = new JumpProxy(jumpSSHSession);
      sshSession.setProxy(proxy);
    }

    vlog.sshdebug("Attempting to use the following SSH private keys:");
    Iterator<Identity> iter =
      sshSession.getIdentityRepository().getIdentities().iterator();
    while (iter.hasNext()) {
      Identity id = iter.next();
      vlog.sshdebug("  " + id.getName());
      if (id.getFingerPrint() != null)
        vlog.sshdebug("    Fingerprint: " + id.getFingerPrint());
    }

    // OpenSSHConfig doesn't recognize StrictHostKeyChecking
    if (sshSession.getConfig("StrictHostKeyChecking") == null)
      sshSession.setConfig("StrictHostKeyChecking", "ask");
    sshSession.setConfig("MaxAuthTries", "3");
    String auth = System.getProperty("turbovnc.sshauth");
    if (auth != null)
      sshSession.setConfig("PreferredAuthentications", auth);
    PasswdDialog dlg = new PasswdDialog(new String("SSH Authentication"),
                                        true, user, false, true, -1);
    if (!Utils.getBooleanProperty("turbovnc.sshkeytest", false))
      sshSession.setUserInfo(dlg);
    sshSession.connect();

    return sshSession;
  }

  /* Create a tunnel using an external SSH client. */

  public static final String DEFAULT_TUNNEL_CMD =
    "%S -f -L %L:localhost:%R %H sleep 20";
  public static final String DEFAULT_JUMP_CMD =
    "%S -J %G -f -L %L:localhost:%R %H sleep 20";
  public static final String DEFAULT_VIA_CMD =
    "%S -f -L %L:%H:%R %G sleep 20";

  private static void createTunnelExt(int vncPort, int localPort,
                                      String pattern, Params params)
                                      throws Exception {
    if (pattern == null || pattern.length() < 1)
      pattern =
        (params.tunnel.get() ? DEFAULT_TUNNEL_CMD :
                               (params.jump.get() != null ? DEFAULT_JUMP_CMD :
                                                            DEFAULT_VIA_CMD));

    String command = fillCmdPattern(pattern, vncPort, params.udsPath,
                                    localPort, params);

    vlog.debug("SSH command line: " + command);
    List<String> args = ArgumentTokenizer.tokenize(command);
    ProcessBuilder pb = new ProcessBuilder(args);
    pb.redirectInput(ProcessBuilder.Redirect.INHERIT);
    pb.redirectOutput(ProcessBuilder.Redirect.INHERIT);
    pb.redirectError(ProcessBuilder.Redirect.INHERIT);
    Process p = pb.start();
    if (p == null || p.waitFor() != 0)
      throw new ErrorException("External SSH error");
  }

  public static final String DEFAULT_TUNNEL_CMD_UDS =
    "%S -- %H exec socat stdio unix-connect:%R";
  public static final String DEFAULT_JUMP_CMD_UDS =
    "%S -J %G -- %H exec socat stdio unix-connect:%R";

  private static Socket createTunnelExtUDS(String pattern, Params params)
                                           throws Exception {
    if (pattern == null || pattern.length() < 1)
      pattern = (params.tunnel.get() ? DEFAULT_TUNNEL_CMD_UDS :
                                       DEFAULT_JUMP_CMD_UDS);

    // Escape the Unix domain socket path twice, since it will be interpreted
    // once by ArgumentTokenizer.tokenize() and again by the remote shell.
    String udsPath = escapeUDSPath(params.udsPath, true);
    udsPath = escapeUDSPath(udsPath, false);

    String command = fillCmdPattern(pattern, -1, udsPath, -1, params);

    vlog.debug("SSH command line (stdio): " + command);
    List<String> args = ArgumentTokenizer.tokenize(command);
    ProcessBuilder pb = new ProcessBuilder(args);
    pb.redirectError(ProcessBuilder.Redirect.INHERIT);
    Process p = pb.start();
    if (p == null)
      throw new ErrorException("External SSH error");
    return new StreamSocket(p.getInputStream(), p.getOutputStream(), true);
  }

  protected static Socket connectUDSDirect(String udsPath) {
    udsPath = expandUDSPathLocal(udsPath);

    vlog.debug("Connecting to Unix domain socket: " + udsPath);
    try {
      ProcessBuilder pb =
        new ProcessBuilder("socat", "stdio",
                           "unix-connect:\"" + udsPath + "\"");
      pb.redirectError(ProcessBuilder.Redirect.INHERIT);
      Process p = pb.start();
      if (p == null)
        throw new ErrorException("socat error");
      return new StreamSocket(p.getInputStream(), p.getOutputStream(), true);
    } catch (Exception e) {
      throw new ErrorException("Could not start socat:\n" + e.getMessage());
    }
  }

  private static String fillCmdPattern(String pattern, int vncPort,
                                       String udsPath, int localPort,
                                       Params params) {
    int i, j;
    boolean hFound = false, gFound = false, rFound = false, lFound = false;
    String command = "";

    for (i = 0; i < pattern.length(); i++) {
      if (pattern.charAt(i) == '%') {
        switch (pattern.charAt(++i)) {
          case 'S':
            command += params.extSSHCommand.get();
            continue;
          case 'H':
            String vncSSHUser = Hostname.getSSHUser(params.server.get());
            String vncHost = Hostname.getHost(params.server.get());
            if (vncSSHUser != null)
              command += vncSSHUser + "@" + vncHost;
            else
              command += vncHost;
            hFound = true;
            continue;
          case 'G':
            command += (params.jump.get() != null ? params.jump.get() :
                                                    params.via.get());
            gFound = true;
            continue;
          case 'R':
            if (udsPath != null)
              command += udsPath;
            else
              command += vncPort;
            rFound = true;
            continue;
          case 'L':
            command += localPort;
            lFound = true;
            continue;
        }
      }
      command += pattern.charAt(i);
    }

    if (!hFound || !rFound || (!lFound && udsPath == null))
      throw new ErrorException("%H, %R or %L absent in tunneling command template.");

    if ((params.jump.get() != null || params.via.get() != null) && !gFound)
      throw new ErrorException("%G pattern absent in tunneling command template.");

    return command;
  }

  // Escape a remote Unix domain socket path so that it can survive being
  // parsed by a POSIX shell on the host or by ArgumentTokenizer.tokenize().
  // Expand ~ to the remote home directory, %h to the remote host name, %i to
  // the remote (numeric) user ID, and %u to the remote username.
  private static String escapeUDSPath(String udsPath, boolean expandTokens) {
    String result = "'";

    for (int i = 0; i < udsPath.length(); i++) {
      if (udsPath.charAt(i) == '\'') {
        // Replace ' with '"'"'
        result += "'\"'\"'";
        continue;
      }
      if (expandTokens) {
        if (i == 0 && udsPath.charAt(i) == '~') {
          result += "'\"$HOME\"'";
          continue;
        } else if (udsPath.charAt(i) == '%') {
          switch (udsPath.charAt(++i)) {
            case '%':
              break;
            case 'h':
              result += "'\"$(uname -n)\"'";
              continue;
            case 'i':
              result += "'\"$(id -u)\"'";
              continue;
            case 'u':
              result += "'\"$(id -u -n)\"'";
              continue;
            default:
              throw new ErrorException("Invalid % sequence (%" +
                                       udsPath.charAt(i) +
                                       ") in Unix domain socket path");
          }
        }
      }
      result += udsPath.charAt(i);
    }
    result += "'";

    return result;
  }

  // Expand a local (client-side) Unix domain socket path similarly to
  // escapeUDSPath()
  private static String expandUDSPathLocal(String udsPath) {
    String result = "";

    for (int i = 0; i < udsPath.length(); i++) {
      if (i == 0 && udsPath.charAt(i) == '~') {
        result += System.getProperty("user.home");
        continue;
      } else if (udsPath.charAt(i) == '%') {
        switch (udsPath.charAt(++i)) {
          case '%':
            break;
          case 'h':
            try {
              ProcessBuilder pb = new ProcessBuilder("uname", "-n");
              pb.redirectInput(ProcessBuilder.Redirect.INHERIT);
              pb.redirectError(ProcessBuilder.Redirect.INHERIT);
              Process p = pb.start();
              if (p == null)
                throw new ErrorException("error calling 'uname -n'");
              String id = new BufferedReader(
                new InputStreamReader(p.getInputStream())).readLine();
              p.getOutputStream().close();
              p.waitFor();
              result += id;
            } catch (Exception e) {
              throw new ErrorException("Could run 'uname -n':\n" +
                                       e.getMessage());
            }
            continue;
          case 'i':
            try {
              ProcessBuilder pb = new ProcessBuilder("id", "-u");
              pb.redirectInput(ProcessBuilder.Redirect.INHERIT);
              pb.redirectError(ProcessBuilder.Redirect.INHERIT);
              Process p = pb.start();
              if (p == null)
                throw new ErrorException("error calling 'id -u'");
              String id = new BufferedReader(
                new InputStreamReader(p.getInputStream())).readLine();
              p.getOutputStream().close();
              p.waitFor();
              result += id;
            } catch (Exception e) {
              throw new ErrorException("Could run 'id -u':\n" +
                                       e.getMessage());
            }
            continue;
          case 'u':
            result += System.getProperty("user.name");
            continue;
          default:
            throw new ErrorException("Invalid % sequence (%" +
                                     udsPath.charAt(i) +
                                     ") in Unix domain socket path");
        }
      }
      result += udsPath.charAt(i);
    }

    return result;
  }

  // JSch logging interface
  private static final Logger LOGGER = new Logger() {
    public boolean isEnabled(int level) {
      switch (level) {
        case Logger.DEBUG:
        case Logger.INFO:
        case Logger.WARN:
        case Logger.ERROR:
        case Logger.FATAL:
          return true;
        default:
          return false;
      }
    }

    public void log(int level, String message) {
      switch (level) {
        case Logger.DEBUG:
        case Logger.INFO:
          vlogSSH.sshdebug(message);
          return;
        case Logger.WARN:
          vlogSSH.status(message);
          return;
        case Logger.ERROR:
          vlogSSH.error(message);
          return;
        case Logger.FATAL:
          throw new ErrorException("SSH: " + message);
      }
    }
  };

  protected Tunnel() {}
  static LogWriter vlog = new LogWriter("Tunnel");
  static LogWriter vlogSSH = new LogWriter("SSH");
}
