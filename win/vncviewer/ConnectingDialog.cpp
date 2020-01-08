//  Copyright (C) 2010, 2016 D. R. Commander. All Rights Reserved.
//  Copyright (C) 2003 Constantin Kaplinsky. All Rights Reserved.
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
#include "ConnectingDialog.h"


class ConnDialogThread : public omni_thread
{
  public:
    void Init(HINSTANCE hInst, const char *vnchost = NULL);
    virtual ~ConnDialogThread();
    virtual void *run_undetached(void *);
    void SetStatus(const char *msg);
    void Close();

  private:
    static LRESULT CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                    LPARAM lParam);
    HINSTANCE m_hInst;
    char *m_vnchost;

    // NOTE: These two fields change during thread execution.
    bool m_started;
    HWND m_hwnd;
};


void ConnDialogThread::Init(HINSTANCE hInst, const char *vnchost)
{
  m_hInst = hInst;
  if (vnchost != NULL)
    m_vnchost = _strdup(vnchost);
  else
    m_vnchost = NULL;

  m_started = false;
  m_hwnd = NULL;

  // Start execution of the new thread
  start_undetached();

  // Wait for the dialog box to be displayed
  while (!m_started)
    sleep(0, 50000000);
}


ConnDialogThread::~ConnDialogThread()
{
  free(m_vnchost);
}


void *ConnDialogThread::run_undetached(void *)
{
  DialogBoxParam(m_hInst, MAKEINTRESOURCE(IDD_CONNECTING_DIALOG), NULL,
                 (DLGPROC)DlgProc, (LPARAM)this);
  return NULL;
}


void ConnDialogThread::SetStatus(const char *msg)
{
  if (m_hwnd != NULL) {
    char buf[256];
    SPRINTF(buf, "Status: %.240s.", msg);
    SetDlgItemText(m_hwnd, IDC_STATUS_STATIC, buf);
  }
}


void ConnDialogThread::Close()
{
  if (m_hwnd != NULL)
    PostMessage(m_hwnd, WM_CLOSE, 0, 0);
  try {
    void *p;
    join(&p);
  } catch (omni_thread_invalid) {}
}


LRESULT CALLBACK ConnDialogThread::DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                           LPARAM lParam)
{
  ConnDialogThread *_this =
    (ConnDialogThread *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

  switch (uMsg) {
    case WM_INITDIALOG:
      SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
      _this = (ConnDialogThread *)lParam;
      _this->m_hwnd = hwnd;
      if (_this->m_vnchost != NULL) {
        char buf[256];
        if (_this->m_vnchost[0] != '\0')
          SPRINTF(buf, "Connecting to %.200s ...", _this->m_vnchost);
        else
          SPRINTF(buf, "Accepting reverse connection...");
        SetDlgItemText(hwnd, IDC_CONNECTING_STATIC, buf);
      }
      SetForegroundWindow(hwnd);
      _this->m_started = true;
      return TRUE;

    case WM_COMMAND:
      switch (LOWORD(wParam)) {
        case IDCLOSE:
          _this->m_hwnd = NULL;
          EndDialog(hwnd, IDCLOSE);
          return TRUE;
      }
      break;

    case WM_CLOSE:
      _this->m_hwnd = NULL;
      EndDialog(hwnd, IDCLOSE);
      return TRUE;
  }

  return FALSE;
}


ConnectingDialog::ConnectingDialog(HINSTANCE hInst, const char *vnchost)
{
  m_thread = new ConnDialogThread;
  m_thread->Init(hInst, vnchost);
}


ConnectingDialog::~ConnectingDialog()
{
  m_thread->Close();
}


void ConnectingDialog::SetStatus(const char *msg)
{
  m_thread->SetStatus(msg);
}
