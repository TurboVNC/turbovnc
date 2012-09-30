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

// VNCHelp.h: interface for the VNCHelp class.

#ifndef VNCHELP_H__
#define VNCHELP_H__

#pragma once


class VNCHelp
{
  public:
    VNCHelp();
    void VNCHelp::Popup(LPARAM lParam);
    BOOL VNCHelp::TranslateMsg(MSG *pmsg);
    virtual ~VNCHelp();

  private:
    DWORD m_dwCookie;
};

#endif 
