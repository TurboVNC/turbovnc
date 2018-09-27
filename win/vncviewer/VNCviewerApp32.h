//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
//  Copyright (C) 2012 D. R. Commander. All Rights Reserved.
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

#include "VNCviewerApp.h"
#include "Daemon.h"
#include <list>


class VNCviewerApp32 : public VNCviewerApp
{
  public:

    VNCviewerApp32(HINSTANCE hInstance, PSTR szCmdLine);
    void ListenMode();
    int NewConnection();
    int NewConnection(char *host, int port);
    int NewConnection(SOCKET sock);
    Daemon *m_pdaemon;
    ~VNCviewerApp32();

  private:

    // Set up registry for program's sounds
    void RegisterSounds();

  // The list of modeless dialogs is maintained for proper message dispatching
  public:

    // Functions to operate on the m_dialogs list
    void AddModelessDialog(HWND hwnd)
    {
      omni_mutex_lock l(m_dialogsMutex);  m_dialogs.push_back(hwnd);
    }

    void RemoveModelessDialog(HWND hwnd)
    {
      omni_mutex_lock l(m_dialogsMutex);  m_dialogs.remove(hwnd);
    }

    bool ProcessDialogMessage(MSG *pmsg);

  private:

    // List of open modeless dialogs
    std::list<HWND> m_dialogs;
    omni_mutex m_dialogsMutex;

};
