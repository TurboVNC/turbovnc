//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
//  Copyright (C) 2015, 2017 D. R. Commander. All Rights Reserved.
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

#include "stdhdrs.h"
#include "vncviewer.h"
#include "VNCviewerApp.h"
#include "Exception.h"


VNCviewerApp *pApp;
extern bool console;


VNCviewerApp::VNCviewerApp(HINSTANCE hInstance, LPTSTR szCmdLine)
{
  pApp = this;
  m_instance = hInstance;

  // Read the command line
  m_options.SetFromCommandLine(szCmdLine);

  // Logging info
  vnclog.SetLevel(m_options.m_logLevel);
  if (console)
    vnclog.SetMode(Log::ToConsole | Log::UseStdio);
  if (m_options.m_logToConsole) {
    if (console)
      vnclog.SetMode(Log::ToConsole | Log::UseStdio | Log::ToDebug);
    else
      vnclog.SetMode(Log::ToConsole | Log::ToDebug);
  }
  if (m_options.m_logToFile) {
    vnclog.SetFile(m_options.m_logFilename);
  }

  // Clear connection list
  for (int i = 0; i < MAX_CONNECTIONS; i++)
    m_clilist[i] = NULL;

  // Initialise winsock
  WORD wVersionRequested = MAKEWORD(2, 0);
  WSADATA wsaData;
  if (WSAStartup(wVersionRequested, &wsaData) != 0) {
    MessageBox(NULL, "Error initialising sockets library", "VNC info",
               MB_OK | MB_ICONSTOP);
    PostQuitMessage(1);
  }
  vnclog.Print(3, "Started and Winsock (v %d) initialised\n",
               wsaData.wVersion);
}


// The list of clients should fill up from the start and have NULLs
// afterwards.  If the first entry is a NULL, the list is empty.

void VNCviewerApp::RegisterConnection(ClientConnection *pConn)
{
  omni_mutex_lock l(m_clilistMutex);
  int i;
  for (i = 0; i < MAX_CONNECTIONS; i++) {
    if (m_clilist[i] == NULL) {
      m_clilist[i] = pConn;
      vnclog.Print(4, "Registered connection with app\n");
      return;
    }
  }
  // If we've reached this point, something is wrong
  vnclog.Print(-1, "Client list overflow!\n");
  MessageBox(NULL, "Client list overflow!", "VNC error",
             MB_OK | MB_ICONSTOP);
  PostQuitMessage(1);

}


void VNCviewerApp::DeregisterConnection(ClientConnection *pConn)
{
  omni_mutex_lock l(m_clilistMutex);
  int i;
  for (i = 0; i < MAX_CONNECTIONS; i++) {
    if (m_clilist[i] == pConn) {
      // shuffle everything above downwards
      for (int j = i; m_clilist[j] && j < MAX_CONNECTIONS-1 ; j++)
        m_clilist[j] = m_clilist[j + 1];
      m_clilist[MAX_CONNECTIONS-1] = NULL;
      vnclog.Print(4, "Deregistered connection from app\n");

      // No clients left?  Then we should finish, unless we're in
      // listen mode
      if ((m_clilist[0] == NULL) && (!pApp->m_options.m_listening)){
        PostQuitMessage(0);}

      return;
    }
  }
  // If we've reached this point, something is wrong
  vnclog.Print(-1, "Client not found for deregistering!\n");
  PostQuitMessage(1);
}


VNCviewerApp::~VNCviewerApp()
{
  WSACleanup();

  vnclog.Print(2, "VNC viewer closing down\n");
}
