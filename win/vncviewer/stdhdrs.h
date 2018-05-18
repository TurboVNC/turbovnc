//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
//  Copyright (C) 2010, 2012 D. R. Commander. All Rights Reserved.
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


#define VC_EXTRALEAN

// These two lines are needed to get the mouse wheel macros.
#define WINVER 0x0500
#define _WIN32_WINDOWS 0x0410

#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <process.h>
#include <assert.h>
#ifndef __MINGW32__
#include <crtdbg.h>
#endif
#include <locale.h>
#include <time.h>
#include <tchar.h>
#include <windows.h>
#include <io.h>

#include "rfb.h"

extern const char *g_buildTime;
