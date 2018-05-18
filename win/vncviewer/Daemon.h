//  Copyright (C) 2010, 2012 D. R. Commander. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
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


#ifndef DAEMON_H__
#define DAEMON_H__

#pragma once

#include "stdhdrs.h"

class Daemon
{
  public:
    Daemon(int port, bool ipv6);
    virtual ~Daemon();
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam,
                                    LPARAM lParam);
  protected:
    void AddTrayIcon();
    void CheckTrayIcon();
    void RemoveTrayIcon();
    bool SendTrayMsg(DWORD msg);
    SOCKET m_sock;
    HWND m_hwnd;
    HMENU m_hmenu;
    UINT_PTR m_timer;
    NOTIFYICONDATA m_nid;
    char netbuf[1024];
};

#endif  // DAEMON_H__
