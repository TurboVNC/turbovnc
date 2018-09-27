//  Copyright (C) 2010, 2012, 2016 D. R. Commander. All Rights Reserved.
//  Copyright (C) 2003 TightVNC Development Team. All Rights Reserved.
//
//  VNC is free software; you can redistribute it and/or modify
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
#include "HotKeys.h"


HotKeys::HotKeys()
{
  m_hwnd = 0;
  Init(false, false);
}


void HotKeys::Init(bool FSAltEnter, bool zoom)
{
  const int MAX_ACCELS = 20;
  ACCEL accel[MAX_ACCELS];
  int i = 0;

  accel[i].fVirt = FVIRTKEY | FALT | FCONTROL | FSHIFT | FNOINVERT;
  accel[i].key = 0x4f;  // "O"
  accel[i++].cmd = IDC_OPTIONBUTTON;

  accel[i].fVirt = FVIRTKEY | FALT | FCONTROL | FSHIFT | FNOINVERT;
  accel[i].key = 0x49;  // "I"
  accel[i++].cmd = ID_CONN_ABOUT;

  accel[i].fVirt = FVIRTKEY | FALT | FCONTROL | FSHIFT | FNOINVERT;
  accel[i].key = 0x46;  // "F"
  accel[i++].cmd = ID_FULLSCREEN_NODIALOG;

  accel[i].fVirt = FVIRTKEY | FALT | FCONTROL | FSHIFT | FNOINVERT;
  accel[i].key = 0x47;  // "G"
  accel[i++].cmd = ID_TOGGLE_GRAB;

  accel[i].fVirt = FVIRTKEY | FALT | FCONTROL | FSHIFT | FNOINVERT;
  accel[i].key = 0x52;  // "R"
  accel[i++].cmd = ID_REQUEST_REFRESH;

  accel[i].fVirt = FVIRTKEY | FALT | FCONTROL | FSHIFT | FNOINVERT;
  accel[i].key = 0x4C;  // "L"
  accel[i++].cmd = ID_REQUEST_LOSSLESS_REFRESH;

  accel[i].fVirt = FVIRTKEY | FALT | FCONTROL | FSHIFT | FNOINVERT;
  accel[i].key = 0x4e;  // "N"
  accel[i++].cmd = ID_NEWCONN;

  accel[i].fVirt = FVIRTKEY | FALT | FCONTROL | FSHIFT | FNOINVERT;
  accel[i].key = 0x53;  // "S"
  accel[i++].cmd = ID_CONN_SAVE_AS;

  accel[i].fVirt = FVIRTKEY | FALT | FCONTROL | FSHIFT | FNOINVERT;
  accel[i].key = 0x54;  // "T"
  accel[i++].cmd = ID_TOOLBAR;

  accel[i].fVirt = FVIRTKEY | FALT | FCONTROL | FSHIFT | FNOINVERT;
  accel[i].key = 0x56;  // "V"
  accel[i++].cmd = ID_TOGGLE_VIEWONLY;

  accel[i].fVirt = FVIRTKEY | FALT | FCONTROL | FSHIFT | FNOINVERT;
  accel[i].key = 0x43;  // "C"
  accel[i++].cmd = ID_TOGGLE_CLIPBOARD;

  accel[i].fVirt = FVIRTKEY | FALT | FCONTROL | FSHIFT | FNOINVERT;
  accel[i].key = 0x45;  // "E"
  accel[i++].cmd = IDD_FILETRANSFER;

  accel[i].fVirt = FVIRTKEY | FALT | FCONTROL | FSHIFT | FNOINVERT;
  accel[i].key = 0x5A;  // "Z"
  accel[i++].cmd = ID_DEFAULT_WINDOW_SIZE;

  if (zoom) {
    accel[i].fVirt = FVIRTKEY | FALT | FCONTROL | FSHIFT | FNOINVERT;
    accel[i].key = VK_OEM_PLUS;
    accel[i++].cmd = ID_ZOOM_IN;

    accel[i].fVirt = FVIRTKEY | FALT | FCONTROL | FSHIFT | FNOINVERT;
    accel[i].key = VK_OEM_MINUS;
    accel[i++].cmd = ID_ZOOM_OUT;

    accel[i].fVirt = FVIRTKEY | FALT | FCONTROL | FSHIFT | FNOINVERT;
    accel[i].key = 0x30;  // "0"
    accel[i++].cmd = ID_ZOOM_100;
  }

  if (FSAltEnter) {
    accel[i].fVirt = FVIRTKEY | FALT | FNOINVERT;
    accel[i].key = VK_RETURN;  // Enter
    accel[i++].cmd = ID_FULLSCREEN_NODIALOG;
  }

  int numKeys = i;
  assert(numKeys <= MAX_ACCELS);

  Destroy();
  m_hAccel = CreateAcceleratorTable((LPACCEL)accel, numKeys);
}


bool HotKeys::TranslateAccel(MSG *pmsg)
{
  return TranslateAccelerator(m_hwnd, m_hAccel, pmsg) != 0;
}


void HotKeys::Destroy(void)
{
  if (m_hAccel) {
    DestroyAcceleratorTable(m_hAccel);
    m_hAccel = 0;
  }
}


HotKeys::~HotKeys()
{
  Destroy();
}
