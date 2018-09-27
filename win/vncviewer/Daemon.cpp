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


// Daemon.cpp: implementation of the Daemon class.

#include "stdhdrs.h"
#include "vncviewer.h"
#include "Daemon.h"
#include "Exception.h"
#include "ClientConnection.h"
#include "AboutBox.h"

#define DAEMON_CLASS_NAME "VNCviewer Daemon"


Daemon::Daemon(int port, bool ipv6)
{
  // Create a dummy window
  WNDCLASSEX wndclass;

  wndclass.cbSize        = sizeof(wndclass);
  wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
  wndclass.lpfnWndProc   = Daemon::WndProc;
  wndclass.cbClsExtra    = 0;
  wndclass.cbWndExtra    = 0;
  wndclass.hInstance     = pApp->m_instance;
  wndclass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
  wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
  wndclass.lpszMenuName  = (const char *)NULL;
  wndclass.lpszClassName = DAEMON_CLASS_NAME;
  wndclass.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

  RegisterClassEx(&wndclass);

  m_hwnd = CreateWindow(DAEMON_CLASS_NAME, DAEMON_CLASS_NAME,
                        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                        200, 200, NULL, NULL, pApp->m_instance, NULL);

  // record which client created this window
  SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)this);

  // Load a popup menu
  m_hmenu = LoadMenu(pApp->m_instance, MAKEINTRESOURCE(IDR_TRAYMENU));

  // Create a listening socket
  struct sockaddr_storage addr;
  struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)&addr;
  struct sockaddr_in *addr4 = (struct sockaddr_in *)&addr;
  int addrlen;

  memset(&addr, 0, sizeof(addr));
  int family = ipv6 ? AF_INET6 : AF_INET;
  if (family == AF_INET6) {
    addr6->sin6_family = family;
    addr6->sin6_port = htons(port);
    addr6->sin6_addr = in6addr_any;
    addrlen = sizeof(struct sockaddr_in6);
  } else {
    addr4->sin_family = family;
    addr4->sin_port = htons(port);
    addr4->sin_addr.s_addr = INADDR_ANY;
    addrlen = sizeof(struct sockaddr_in);
  }

  m_sock = socket(family, SOCK_STREAM, 0);
  if (!m_sock)
    throw WarningException("Error creating Daemon socket");

  int one = 1, res = 0;
  // res = setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (const char *) &one, sizeof(one));
  // if (res == SOCKET_ERROR)  {
  //   closesocket(m_sock);
  //   m_sock = 0;
  //   throw WarningException("Error setting Daemon socket options");
  // }

  res = bind(m_sock, (struct sockaddr *)&addr, addrlen);
  if (res == SOCKET_ERROR) {
    closesocket(m_sock);
    m_sock = 0;
    throw WarningException("Error binding Daemon socket");
  }

  res = listen(m_sock, 5);
  if (res == SOCKET_ERROR) {
    closesocket(m_sock);
    m_sock = 0;
    throw WarningException("Error when Daemon listens");
  }

  // Send a message to the specified window whenever an incoming connection
  // occurs
  WSAAsyncSelect(m_sock, m_hwnd, WM_SOCKEVENT, FD_ACCEPT);

  // Create the tray icon
  AddTrayIcon();

  // A timer checks that the tray icon is intact
  m_timer = SetTimer(m_hwnd, IDT_TRAYTIMER, 15000, NULL);
}


void Daemon::AddTrayIcon()
{
  vnclog.Print(4, "Adding tray icon\n");
  SendTrayMsg(NIM_ADD);
}


void Daemon::CheckTrayIcon()
{
  vnclog.Print(8, "Checking tray icon\n");
  if (!SendTrayMsg(NIM_MODIFY)) {
    vnclog.Print(4, "Tray icon not there - reinstalling\n");
    AddTrayIcon();
  }
}


void Daemon::RemoveTrayIcon()
{
  vnclog.Print(4, "Deleting tray icon\n");
  SendTrayMsg(NIM_DELETE);
}


bool Daemon::SendTrayMsg(DWORD msg)
{
  m_nid.hWnd = m_hwnd;
  m_nid.cbSize = sizeof(m_nid);
  m_nid.uID = IDR_TRAY;  // never changes after construction
  m_nid.hIcon = LoadIcon(pApp->m_instance, MAKEINTRESOURCE(IDR_TRAY));
  m_nid.uFlags = NIF_ICON | NIF_MESSAGE;
  m_nid.uCallbackMessage = WM_TRAYNOTIFY;
  m_nid.szTip[0] = '\0';
  // Use resource string as tooltip, if there is one
  if (LoadString(pApp->m_instance, IDR_TRAY, m_nid.szTip, sizeof(m_nid.szTip)))
    m_nid.uFlags |= NIF_TIP;
  return (bool)(Shell_NotifyIcon(msg, &m_nid) != 0);
}


LRESULT CALLBACK Daemon::WndProc(HWND hwnd, UINT iMsg, WPARAM wParam,
                                 LPARAM lParam)
{
  // This is a static method, so we don't know which instantiation we're
  // dealing with.  We have stored a pseudo-this in the window user data,
  // though.
  Daemon *_this = (Daemon *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

  switch (iMsg) {

    case WM_CREATE:
      return 0;

    case WM_SOCKEVENT:
    {
      assert(HIWORD(lParam) == 0);
      // A new socket created by accept() might send messages to
      // this procedure.  We can ignore them.
      if (wParam != _this->m_sock)
        return 0;

      switch (lParam) {
        case FD_ACCEPT:
        {
          SOCKET hNewSock;
          hNewSock = accept(_this->m_sock, NULL, NULL);
          WSAAsyncSelect(hNewSock, hwnd, 0, 0);
          unsigned long nbarg = 0;
          ioctlsocket(hNewSock, FIONBIO, &nbarg);
          pApp->NewConnection(hNewSock);
          break;
        }
        case FD_READ:
        {
          unsigned long numbytes;
          ioctlsocket(_this->m_sock, FIONREAD, &numbytes);
          recv(_this->m_sock, _this->netbuf, numbytes, 0);
          break;
        }
        case FD_CLOSE:
          vnclog.Print(5, "Daemon connection closed\n");
          DestroyWindow(hwnd);
          break;
      }
      return 0;
    }

    case WM_COMMAND:
      switch (LOWORD(wParam)) {
        case ID_NEWCONN:
          pApp->NewConnection();
          break;
        case IDC_OPTIONBUTTON:
          pApp->m_options.DoDialog();
          pApp->m_options.SaveOpt(".listen", KEY_VNCVIEWER_HISTORY);
          break;
        case ID_CLOSEDAEMON:
          PostQuitMessage(0);
          break;
        case IDD_APP_ABOUT:
          ShowAboutBox();
          break;
      }
      return 0;

    case WM_TRAYNOTIFY:
    {
      HMENU hSubMenu = GetSubMenu(_this->m_hmenu, 0);
      if (lParam == WM_LBUTTONDBLCLK) {
        // double-click: execute first menu item
        SendMessage(_this->m_nid.hWnd, WM_COMMAND, GetMenuItemID(hSubMenu, 0),
                    0);
      } else if (lParam == WM_RBUTTONUP) {
        if (hSubMenu == NULL) {
          vnclog.Print(2, "No systray submenu\n");
          return 0;
        }
        // Make first menu item the default (bold font)
        SetMenuDefaultItem(hSubMenu, 0, TRUE);

        // Display the menu at the current mouse location
        POINT mouse;
        GetCursorPos(&mouse);
        SetForegroundWindow(_this->m_nid.hWnd);
        TrackPopupMenu(hSubMenu, 0, mouse.x, mouse.y, 0, _this->m_nid.hWnd,
                       NULL);
      }
      return 0;
    }

    case WM_TIMER:
      _this->CheckTrayIcon();
      return 0;

    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
  }

  return DefWindowProc(hwnd, iMsg, wParam, lParam);
}


Daemon::~Daemon()
{
  KillTimer(m_hwnd, m_timer);
  RemoveTrayIcon();
  DestroyMenu(m_hmenu);
  shutdown(m_sock, SD_BOTH);
  closesocket(m_sock);
}
