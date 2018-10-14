//  Copyright (C) 2013, 2016-2018 D. R. Commander. All Rights Reserved.
//  Copyright (C) 2000 Const Kaplinsky.  All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
//
//  This file is part of the VNC system.
//
//  The VNC system is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
//  USA.

#include "vncviewer.h"

#include "ClientConnection.h"
#include "Exception.h"


#define DEFAULT_SSH_CMD "ssh.exe"
#define DEFAULT_TUNNEL_CMD  \
  (DEFAULT_SSH_CMD " -f -L %L:localhost:%R %H sleep 20")
#define DEFAULT_VIA_CMD  \
  (DEFAULT_SSH_CMD "  -f -L %L:%H:%R %G sleep 20")


static char *getCmdPattern(bool tunnelOption)
{
  char *pattern;

#pragma warning(disable: 4996)
  pattern = getenv(tunnelOption ? "VNC_TUNNEL_CMD" : "VNC_VIA_CMD");
#pragma warning(default: 4996)
  if (pattern == NULL)
    pattern = tunnelOption ? DEFAULT_TUNNEL_CMD : DEFAULT_VIA_CMD;

  return pattern;
}


// FindFreeTcpPort() tries to find an unused TCP port.  Returns 0 on failure.

static int FindFreeTcpPort(void)
{
  SOCKET sock;
  struct sockaddr_in addr;
  socklen_t n;

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    throw WarningException("Could not create socket");

  addr.sin_port = 0;
  if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    closesocket(sock);
    throw WarningException("Could not find free TCP port");
  }

  n = sizeof(addr);
  if (getsockname(sock, (struct sockaddr *)&addr, &n) < 0) {
    closesocket(sock);
    throw WarningException("Could not get port number");
  }

  closesocket(sock);
  return ntohs(addr.sin_port);
}


// NOTE: result points to a 1024-byte buffer

static void fillCmdPattern(char *result, char *pattern, char *gatewayHost,
                           char *remoteHost, char *remotePort, char *localPort,
                           bool tunnelOption)
{
  int i, j;
  bool H_found = false, G_found = false, R_found = false, L_found = false;

  for (i = 0, j = 0; pattern[i] && j < 1023; i++, j++) {
    if (pattern[i] == '%') {
      switch (pattern[++i]) {
        case 'H':
          STRNCPY(&result[j], remoteHost, 1024 - j);
          j += (int)strlen(remoteHost) - 1;
          H_found = true;
          continue;
        case 'G':
          STRNCPY(&result[j], gatewayHost, 1024 - j);
          j += (int)strlen(gatewayHost) - 1;
          G_found = true;
          continue;
        case 'R':
          STRNCPY(&result[j], remotePort, 1024 - j);
          j += (int)strlen(remotePort) - 1;
          R_found = true;
          continue;
        case 'L':
          STRNCPY(&result[j], localPort, 1024 - j);
          j += (int)strlen(localPort) - 1;
          L_found = true;
          continue;
        case '\0':
          i--;
          continue;
      }
    }
    result[j] = pattern[i];
  }

  if (pattern[i])
    throw WarningException("Tunneling command is too long.");

  if (!H_found || !R_found || !L_found)
    throw WarningException("%H, %R, or %L absent in tunneling command.");

  if (!tunnelOption && !G_found)
    throw WarningException("%G pattern absent in tunneling command.");

  result[j] = '\0';
}


void ClientConnection::SetupSSHTunnel(void)
{
  char *pattern;
  char cmd[1024];
  int localPort;
  char localPortStr[8];
  char remotePortStr[8];
  bool tunnelOption = strlen(m_opts.m_gatewayHost) == 0;

  pattern = getCmdPattern(tunnelOption);
  if (!pattern)
    throw WarningException("Invalid SSH command pattern");

  localPort = FindFreeTcpPort();
  if (localPort == 0)
    throw WarningException("Could not find free TCP port");

  SPRINTF(localPortStr, "%d", localPort);
  SPRINTF(remotePortStr, "%d", m_port);

  fillCmdPattern(cmd, pattern, m_opts.m_gatewayHost, m_host, remotePortStr,
                 localPortStr, tunnelOption);

  m_port = localPort;
  SPRINTF(m_host, "localhost");

  if (system(cmd) != 0)
    throw WarningException("Could not start SSH client to create tunnel");
}
