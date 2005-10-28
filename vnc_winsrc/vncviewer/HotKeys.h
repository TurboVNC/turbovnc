// Copyright (C) 2003 TightVNC Development Team. All Rights Reserved.
//
//  TightVNC is free software; you can redistribute it and/or modify
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
// TightVNC homepage on the Web: http://www.tightvnc.com/

// HotKeys.h: interface for the HotKeys class.

#ifndef HOTKEYS_H__
#define HOTKEYS_H__

#pragma once

class HotKeys
{
	HWND m_hwnd;
	HACCEL m_hAccel;
public:
	HotKeys();
	void SetWindow(HWND hwnd) { m_hwnd = hwnd; }
	bool TranslateAccel(MSG *pmsg);
	virtual ~HotKeys();	
};

#endif 
