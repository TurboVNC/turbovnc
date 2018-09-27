//  Copyright (C) 2010, 2016 D. R. Commander. All Rights Reserved.
//  Copyright (C) 2005-2006 Sun Microsystems, Inc. All Rights Reserved.
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

#include "stdhdrs.h"
#include "vncviewer.h"
#include "SessionDialog.h"
#include "Exception.h"
#include "Htmlhelp.h"


SessionDialog::SessionDialog(VNCOptions *pOpt, ClientConnection *cc)
{
  m_pOpt = pOpt;
  m_cc = cc;
  DWORD dispos;

  RegCreateKeyEx(HKEY_CURRENT_USER, KEY_VNCVIEWER_HISTORY, 0, NULL,
                 REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &m_hRegKey,
                 &dispos);
}


SessionDialog::~SessionDialog()
{
  RegCloseKey(m_hRegKey);
}


// It's exceedingly unlikely, but possible, that if two modal dialogs were
// closed at the same time, the static variables used for transfer between
// the window procedure and this method could overwrite each other.

INT_PTR SessionDialog::DoDialog()
{
  return DialogBoxParam(pApp->m_instance, MAKEINTRESOURCE(IDD_SESSION_DLG),
                        NULL, (DLGPROC)SessDlgProc, (LPARAM)this);
}


BOOL CALLBACK SessionDialog::SessDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                         LPARAM lParam)
{
  // This is a static method, so we don't know which instantiation we're
  // dealing with, but we can get a pseudo-this from the parameter to
  // WM_INITDIALOG, which we thereafter store with the window and retrieve
  // as follows:
  SessionDialog *_this =
    (SessionDialog *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

  int i;
  char tmphost[256];
  char buffer[256];
  HWND hListMode = GetDlgItem(hwnd, IDC_LIST_MODE);
  HWND hcombo = GetDlgItem(hwnd, IDC_HOSTNAME_EDIT);

  switch (uMsg) {
    case WM_INITDIALOG:
    {
      SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
      SessionDialog *_this = (SessionDialog *)lParam;
      CenterWindow(hwnd);
      _this->m_cc->m_hSess = hwnd;

      // Load connection history into the combo box
      const int maxEntries = pApp->m_options.m_historyLimit;
      int listIndex = 0;
      for (i = 0; i < maxEntries; i++) {
        char keyName[256];
        _itoa_s(i, keyName, _countof(keyName), 10);
        char buf[256];
        int dwbuflen = 255;
        if (RegQueryValueEx(_this->m_hRegKey, keyName, NULL, NULL, (LPBYTE)buf,
                            (LPDWORD)&dwbuflen) == ERROR_SUCCESS) {
          buf[255] = '\0';
          if (buf[0] != 0)
            SendMessage(hcombo, CB_INSERTSTRING, (WPARAM)listIndex++,
                        (LPARAM)buf);
        }
      }
      if (_this->m_pOpt->m_display[0] == '\0') {
        SendMessage(hcombo, CB_SETCURSEL, 0, 0);
        LRESULT r = SendMessage(hcombo, CB_GETLBTEXTLEN, 0, 0);
        if (r > 1 && r <= 256) {
          r = SendMessage(hcombo, CB_GETLBTEXT, 0, (LPARAM)buffer);
          if (r > 1)
            _this->m_pOpt->LoadOpt(buffer, KEY_VNCVIEWER_HISTORY);
        }
      } else {
        SetDlgItemText(hwnd, IDC_HOSTNAME_EDIT, _this->m_pOpt->m_display);
      }

      UpdateConnectButton(hwnd);
      SetFocus(hcombo);
      return TRUE;
    }
    case WM_HELP:
      help.Popup(lParam);
      return 0;
    case WM_ACTIVATE:
    case WM_ACTIVATEAPP:
      if ((pApp->m_options.m_listening) ||
          (FindWindow("VNCviewer Daemon", 0) != NULL))
        EnableWindow(hListMode, FALSE);
      if ((!pApp->m_options.m_listening) &&
          (FindWindow("VNCviewer Daemon", 0) == NULL))
        EnableWindow(hListMode, TRUE);
      return 0;
    case WM_COMMAND:
      switch (LOWORD(wParam)) {
        case IDC_HOSTNAME_EDIT:
          switch (HIWORD(wParam)) {
            case CBN_SELENDOK:
            {
              int a = (int)SendMessage(hcombo, CB_GETCURSEL, 0, 0L);
              SendMessage(hcombo, CB_GETLBTEXT, a, (LPARAM)(int FAR *)buffer);
              _this->m_pOpt->LoadOpt(buffer, KEY_VNCVIEWER_HISTORY);
              EnableConnectButton(hwnd, TRUE);
              SetFocus(hcombo);
            }
            break;
            case CBN_EDITCHANGE:
              UpdateConnectButton(hwnd);
              break;
          }
          return TRUE;
        case IDC_LOAD:
        {
          char buf[80];
          buf[0] = '\0';
          if (_this->m_cc->LoadConnection(buf, true) != -1) {
            FormatDisplay(_this->m_cc->m_port, _this->m_pOpt->m_display,
                          _countof(_this->m_pOpt->m_display),
                          _this->m_cc->m_host);
            SetDlgItemText(hwnd, IDC_HOSTNAME_EDIT, _this->m_pOpt->m_display);
          }
          UpdateConnectButton(hwnd);
          SetFocus(hcombo);
          return TRUE;
        }
        case IDC_LIST_MODE:
          pApp->m_options.LoadOpt(".listen", KEY_VNCVIEWER_HISTORY);
          pApp->m_options.m_listening = true;
          pApp->ListenMode();
          _this->m_pOpt->CloseDialog();
          EndDialog(hwnd, FALSE);
          return TRUE;
        case IDC_OK:
          char display[256];
          GetDlgItemText(hwnd, IDC_HOSTNAME_EDIT, display, 256);
          if (strlen(display) == 0)
            return TRUE;
          if (!ParseDisplay(display, _countof(display), tmphost,
                            _countof(tmphost), &_this->m_cc->m_port)) {
            MessageBox(NULL, "Invalid VNC server specified.\n\r"
                       "Server should be of the form host:display.",
                       "Connection setup", MB_OK | MB_ICONEXCLAMATION);
            return TRUE;
          } else {
            STRCPY(_this->m_cc->m_host, tmphost);
            STRCPY(_this->m_pOpt->m_display, display);
          }

          _this->m_pOpt->CloseDialog();
          EndDialog(hwnd, TRUE);
          return TRUE;
        case IDCANCEL:
          _this->m_pOpt->CloseDialog();
          EndDialog(hwnd, FALSE);
          return TRUE;
        case IDC_OPTIONBUTTON:
        {
          if (_this->m_pOpt->RaiseDialog())
            return TRUE;  // Options dialog already shown
          HWND hOptionButton = GetDlgItem(hwnd, IDC_OPTIONBUTTON);
          _this->m_pOpt->DoDialog();
          GetDlgItemText(hwnd, IDC_HOSTNAME_EDIT,
                         _this->m_pOpt->m_display, 256);
          SendMessage(hcombo, CB_RESETCONTENT, 0, 0);
          int dwbuflen = 255;
          char valname[256];
          char buf[256];
          int maxEntries = pApp->m_options.m_historyLimit;
          for (i = 0; i < maxEntries; i++) {
            _itoa_s(i, valname, _countof(valname), 10);
            dwbuflen = 255;
            if (RegQueryValueEx(_this->m_hRegKey, (LPTSTR)valname, NULL, NULL,
                                (LPBYTE)buf,
                                (LPDWORD)&dwbuflen) != ERROR_SUCCESS)
              break;
            SendMessage(hcombo, CB_INSERTSTRING, (WPARAM)i,
                        (LPARAM)(int FAR *)buf);
          }
          SetDlgItemText(hwnd, IDC_HOSTNAME_EDIT, _this->m_pOpt->m_display);
          SetFocus(hOptionButton);
          return TRUE;
        }
      }
    case WM_DESTROY:
      EndDialog(hwnd, FALSE);
      return TRUE;
  }
  return 0;
}


void SessionDialog::EnableConnectButton(HWND hDialog, BOOL bEnable)
{
  HWND hConnectButton = GetDlgItem(hDialog, IDC_OK);
  EnableWindow(hConnectButton, bEnable);
}


void SessionDialog::UpdateConnectButton(HWND hDialog)
{
  HWND hHostComboBox = GetDlgItem(hDialog, IDC_HOSTNAME_EDIT);
  BOOL hostNotEmpty = (GetWindowTextLength(hHostComboBox) != 0);
  EnableConnectButton(hDialog, hostNotEmpty);
}
