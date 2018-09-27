//  Copyright (C) 2010, 2016 D. R. Commander. All Rights Reserved.
//  Copyright (C) 2003-2006 Constantin Kaplinsky. All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation. All Rights Reserved.
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

// Implementation of the dialog box for authentication with a username/password
// pair.

#include "stdhdrs.h"
#include "vncviewer.h"
#include "LoginAuthDialog.h"
#include "Exception.h"
#include "safestr.h"


LoginAuthDialog::LoginAuthDialog(char *vnchost, char *title, char *username)
{
  if (title != NULL)
    STRCPY(m_title, title);
  else
    m_title[0] = '\0';

  m_username_disabled = (username == NULL);

  if (username == NULL || username[0] == '\0')
    m_username[0] = '\0';
  else
    STRCPY(m_username, username);

  m_passwd[0] = '\0';
  m_vnchost = (vnchost != NULL) ? vnchost : "[unknown]";
}


LoginAuthDialog::~LoginAuthDialog()
{
}


INT_PTR LoginAuthDialog::DoDialog()
{
  return DialogBoxParam(pApp->m_instance,
                        MAKEINTRESOURCE(IDD_LOGIN_AUTH_DIALOG), NULL,
                        (DLGPROC)DlgProc, (LPARAM)this);
}


BOOL CALLBACK LoginAuthDialog::DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                       LPARAM lParam)
{
  // This is a static method, so we don't know which instantiation we're
  // dealing with.  But we can get a pseudo-this from the parameter to
  // WM_INITDIALOG, which we thereafter store with the window and retrieve
  // as follows:
  LoginAuthDialog *_this =
    (LoginAuthDialog *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

  switch (uMsg) {

    case WM_INITDIALOG:
      SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
      _this = (LoginAuthDialog *)lParam;
      if (_this->m_title[0] != '\0')
        SetWindowText(hwnd, _this->m_title);
      SetDlgItemText(hwnd, IDC_VNCHOST, _this->m_vnchost);
      CenterWindow(hwnd);
      if (_this->m_username[0] != '\0') {
        SetDlgItemText(hwnd, IDC_LOGIN_EDIT, _this->m_username);
        SetFocus(GetDlgItem(hwnd, IDC_PASSWD_EDIT));
        return FALSE;
      }
      if (_this->m_username_disabled)
        EnableWindow(GetDlgItem(hwnd, IDC_LOGIN_EDIT), FALSE);
      return TRUE;

    case WM_COMMAND:
      switch (LOWORD(wParam)) {
        case IDOK:
          GetDlgItemText(hwnd, IDC_LOGIN_EDIT, _this->m_username, 256);
          GetDlgItemText(hwnd, IDC_PASSWD_EDIT, _this->m_passwd, 256);
          EndDialog(hwnd, TRUE);
          return TRUE;
        case IDCANCEL:
          EndDialog(hwnd, FALSE);
          throw QuietException("User canceled authentication.");
          return TRUE;
      }
      break;

    case WM_DESTROY:
      EndDialog(hwnd, FALSE);
      return TRUE;

  }
  return 0;
}
