//  Copyright (C) 2010, 2012, 2015 D. R. Commander. All Rights Reserved.
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


static LRESULT CALLBACK AboutDlgProc(HWND hwnd, UINT iMsg, WPARAM wParam,
                                     LPARAM lParam)
{
  switch (iMsg) {
    case WM_INITDIALOG:
      SetDlgItemText(hwnd, IDC_BUILDTIME, g_buildTime);
      CenterWindow(hwnd);
      return TRUE;
    case WM_CLOSE:
      EndDialog(hwnd, TRUE);
      return TRUE;
    case WM_COMMAND:
      if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        EndDialog(hwnd, TRUE);
  }
  return FALSE;
}


void ShowAboutBox()
{
  INT_PTR res = DialogBox(pApp->m_instance,
                          MAKEINTRESOURCE(IDD_APP_ABOUT), NULL,
                          (DLGPROC)AboutDlgProc);
}


static LRESULT CALLBACK HelpDlgProc(HWND hwnd, UINT iMsg, WPARAM wParam,
                                    LPARAM lParam)
{
  switch (iMsg) {
    case WM_INITDIALOG:
    {
      char buf[16384];
      buf[0] = 0;
      for (int i = 0; i < IDS_NUMHELP; i++) {
        if (strlen(buf) < 16383)
          LoadString(pApp->m_instance, IDS_HELP + i, &buf[strlen(buf)],
                     sizeof(buf) - (int)strlen(buf));
      }
      SetDlgItemText(hwnd, IDC_EDIT_HELP, buf);
      SetWindowText(hwnd, (LPTSTR)lParam);
      CenterWindow(hwnd);
      return TRUE;
    }
  case WM_CLOSE:
    EndDialog(hwnd, TRUE);
    return TRUE;
  case WM_COMMAND:
    if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
      EndDialog(hwnd, TRUE);
  }
  return FALSE;
}


void ShowHelpBox(LPTSTR title)
{
  INT_PTR res = DialogBoxParam(pApp->m_instance,
                               MAKEINTRESOURCE(IDD_HELP), NULL,
                               (DLGPROC)HelpDlgProc, (LPARAM)title);
}
