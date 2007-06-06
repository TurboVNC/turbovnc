/*
 *  Copyright (C) 2000 Const Kaplinsky.  All Rights Reserved.
 *  Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
 *
 *  This is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this software; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 *  USA.
 */

/*
 * tunnel.c - tunneling support (e.g. for using standalone SSH installation)
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <vncviewer.h>

/* True if there was -tunnel or -via option in the command line. */
Bool tunnelSpecified = False;

/* True if it was -tunnel, not -via option. */
static Bool tunnelOption = False;

/* "Hostname:display" pair in the command line will be substituted
   by this fake argument when tunneling is used. */
static char lastArgv[32];


static void processTunnelArgs(char **remoteHost,
			      int *remotePort, int localPort,
			      int *pargc, char **argv, int tunnelArgIndex);
static void processViaArgs(char **gatewayHost, char **remoteHost,
			   int *remotePort, int localPort,
			   int *pargc, char **argv, int tunnelArgIndex);
static char *getCmdPattern(void);
static Bool fillCmdPattern(char *result, char *pattern,
			   char *gatewayHost, char *remoteHost,
			   char *remotePort, char *localPort);
static Bool runCommand(char *cmd);


Bool
createTunnel(int *pargc, char **argv, int tunnelArgIndex)
{
  char *pattern;
  char cmd[1024];
  int localPort, remotePort;
  char localPortStr[8];
  char remotePortStr[8];
  char *gatewayHost = "";
  char *remoteHost = "localhost";

  tunnelSpecified = True;
  if (strcmp(argv[tunnelArgIndex], "-tunnel") == 0)
    tunnelOption = True;

  pattern = getCmdPattern();
  if (!pattern)
    return False;

  localPort = FindFreeTcpPort();
  if (localPort == 0)
    return False;

  if (tunnelOption) {
    processTunnelArgs(&remoteHost, &remotePort, localPort,
		      pargc, argv, tunnelArgIndex);
  } else {
    processViaArgs(&gatewayHost, &remoteHost, &remotePort, localPort,
		   pargc, argv, tunnelArgIndex);
  }

  sprintf(localPortStr, "%d", localPort);
  sprintf(remotePortStr, "%d", remotePort);

  if (!fillCmdPattern(cmd, pattern, gatewayHost, remoteHost,
		      remotePortStr, localPortStr))
    return False;

  if (!runCommand(cmd))
    return False;

  return True;
}

static void
processTunnelArgs(char **remoteHost, int *remotePort, int localPort,
		  int *pargc, char **argv, int tunnelArgIndex)
{
  char *pdisplay;
  int port;

  if (tunnelArgIndex >= *pargc - 1)
    usage();

  pdisplay = strchr(argv[*pargc - 1], ':');
  if (pdisplay == NULL || pdisplay == argv[*pargc - 1])
    usage();

  *pdisplay++ = '\0';
  if (strspn(pdisplay, "-0123456789") != strlen(pdisplay))
    usage();

  *remotePort = atoi(pdisplay);
  if (*remotePort < 100)
    *remotePort += SERVER_PORT_OFFSET;

  sprintf(lastArgv, "localhost::%d", localPort);

  *remoteHost = argv[*pargc - 1];
  argv[*pargc - 1] = lastArgv;

  removeArgs(pargc, argv, tunnelArgIndex, 1);
}

static void
processViaArgs(char **gatewayHost, char **remoteHost,
	       int *remotePort, int localPort,
	       int *pargc, char **argv, int tunnelArgIndex)
{
  char *colonPos;
  int len, portOffset;
  int disp;

  if (tunnelArgIndex >= *pargc - 2)
    usage();

  colonPos = strchr(argv[*pargc - 1], ':');
  if (colonPos == NULL) {
    /* No colon -- use default port number */
    *remotePort = SERVER_PORT_OFFSET;
  } else {
    *colonPos++ = '\0';
    len = strlen(colonPos);
    portOffset = SERVER_PORT_OFFSET;
    if (colonPos[0] == ':') {
      /* Two colons -- interpret as a port number */
      colonPos++;
      len--;
      portOffset = 0;
    }
    if (!len || strspn(colonPos, "-0123456789") != len) {
      usage();
    }
    disp = atoi(colonPos);
    if (portOffset != 0 && disp >= 100)
      portOffset = 0;
    *remotePort = disp + portOffset;
  }

  sprintf(lastArgv, "localhost::%d", localPort);

  *gatewayHost = argv[tunnelArgIndex + 1];

  if (argv[*pargc - 1][0] != '\0')
    *remoteHost = argv[*pargc - 1];

  argv[*pargc - 1] = lastArgv;

  removeArgs(pargc, argv, tunnelArgIndex, 2);
}

static char *
getCmdPattern(void)
{
  struct stat st;
  char *pattern;

  pattern = getenv((tunnelOption) ? "VNC_TUNNEL_CMD" : "VNC_VIA_CMD");
  if (pattern == NULL) {
    if ( stat(DEFAULT_SSH_CMD, &st) != 0 ||
	 !(S_ISREG(st.st_mode) || S_ISLNK(st.st_mode)) ) {
      fprintf(stderr, "%s: Cannot establish SSH tunnel: missing %s binary.\n",
	      programName, DEFAULT_SSH_CMD);
      return NULL;
    }
    pattern = (tunnelOption) ? DEFAULT_TUNNEL_CMD : DEFAULT_VIA_CMD;
  }

  return pattern;
}

/* Note: in fillCmdPattern() result points to a 1024-byte buffer */

static Bool
fillCmdPattern(char *result, char *pattern,
	       char *gatewayHost, char *remoteHost,
	       char *remotePort, char *localPort)
{
  int i, j;
  Bool H_found = False, G_found = False, R_found = False, L_found = False;

  for (i=0, j=0; pattern[i] && j<1023; i++, j++) {
    if (pattern[i] == '%') {
      switch (pattern[++i]) {
      case 'H':
	strncpy(&result[j], remoteHost, 1024 - j);
	j += strlen(remoteHost) - 1;
	H_found = True;
	continue;
      case 'G':
	strncpy(&result[j], gatewayHost, 1024 - j);
	j += strlen(gatewayHost) - 1;
	G_found = True;
	continue;
      case 'R':
	strncpy(&result[j], remotePort, 1024 - j);
	j += strlen(remotePort) - 1;
	R_found = True;
	continue;
      case 'L':
	strncpy(&result[j], localPort, 1024 - j);
	j += strlen(localPort) - 1;
	L_found = True;
	continue;
      case '\0':
	i--;
	continue;
      }
    }
    result[j] = pattern[i];
  }

  if (pattern[i]) {
    fprintf(stderr, "%s: Tunneling command is too long.\n", programName);
    return False;
  }

  if (!H_found || !R_found || !L_found) {
    fprintf(stderr, "%s: %%H, %%R or %%L absent in tunneling command.\n",
	    programName);
    return False;
  }
  if (!tunnelOption && !G_found) {
    fprintf(stderr, "%s: %%G pattern absent in tunneling command.\n",
	    programName);
    return False;
  }

  result[j] = '\0';
  return True;
}

static Bool
runCommand(char *cmd)
{
  if (system(cmd) != 0) {
    fprintf(stderr, "%s: Tunneling command failed: %s.\n",
	    programName, cmd);
    return False;
  }
  return True;
}

