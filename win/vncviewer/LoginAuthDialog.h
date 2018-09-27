//  Copyright (C) 2010 D. R. Commander. All Rights Reserved.
//  Copyright (C) 2003-2006 Constantin Kaplinsky. All Rights Reserved.
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


#ifndef LOGINAUTHDIALOG_H__
#define LOGINAUTHDIALOG_H__

#pragma once


class LoginAuthDialog
{
  public:
    char m_username[256];
    char m_passwd[256];

    LoginAuthDialog(char *vnchost, char *title = NULL, char *username = NULL);
    virtual ~LoginAuthDialog();
    INT_PTR DoDialog();

    static BOOL CALLBACK DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,
                                 LPARAM lParam);
  private:
    char m_title[256];
    char *m_vnchost;
    bool m_username_disabled;
};

#endif  // LOGINAUTHDIALOG_H__
