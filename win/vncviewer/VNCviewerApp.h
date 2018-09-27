//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation. All Rights Reserved.
//  Copyright (C) 2012, 2015 D. R. Commander. All Rights Reserved.
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

#ifndef VNCVIEWERAPP_H__
#define VNCVIEWERAPP_H__

#pragma once


// The state of the application as a whole is contained in the app object
class VNCviewerApp;


// 128 connections ought to be enough for anybody
#define MAX_CONNECTIONS    (128)

#define MAX_AUTH_RETRIES   (3)


#include "ClientConnection.h"


class VNCviewerApp
{
  public:
    VNCviewerApp(HINSTANCE hInstance, LPTSTR szCmdLine);
    virtual void ListenMode() = 0;
    virtual int NewConnection() = 0;
    virtual int NewConnection(char *host, int port) = 0;
    virtual int NewConnection(SOCKET sock) = 0;

    ~VNCviewerApp();

    // This should be used by Connections to register and deregister
    // themselves.  When the last connection is deregistered, the app
    // will close unless in listen mode.
    void RegisterConnection(ClientConnection *pConn);
    void DeregisterConnection(ClientConnection *pConn);

    VNCOptions m_options;
    HINSTANCE m_instance;
    bool m_wacom;

  private:
    ClientConnection *m_clilist[MAX_CONNECTIONS];
    omni_mutex m_clilistMutex;

  protected:
    // Benchmark stuff
    double tDecode, tBlit, tRead;
    unsigned long long decodePixels, blitPixels;
    unsigned long decodeRect, blits, updates;
    friend ClientConnection;
};

#endif  // VNCVIEWERAPP_H__
