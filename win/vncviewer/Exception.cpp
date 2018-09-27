//  Copyright (C) 2015-2016 D. R. Commander. All Rights Reserved.
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

#include "stdhdrs.h"
#include "Exception.h"
#include "safestr.h"


Exception::Exception(const char *info)
{
  assert(info != NULL);
  m_info = new char[strlen(info) + 1];
  STRNCPY(m_info, info, strlen(info) + 1);
}


Exception::~Exception()
{
  delete[] m_info;
}


void Exception::Report()
{
  assert(false);
}


// ---------------------------------------

QuietException::QuietException(const char *info) : Exception(info)
{
}


QuietException::~QuietException()
{
}


void QuietException::Report()
{
#ifdef _MSC_VER
  _RPT1(_CRT_WARN, "Warning : %s\n", m_info);
#endif
}


// ---------------------------------------

WarningException::WarningException(const char *info, bool close) :
  Exception(info)
{
  m_close = close;
}


WarningException::~WarningException()
{
}


void WarningException::Report()
{
#ifdef _MSC_VER
  _RPT1(_CRT_WARN, "Warning : %s\n", m_info);
#endif
  printf("WARNING: %s\n", m_info);
  MessageBox(NULL, m_info, "TurboVNC info",
             MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TOPMOST);
}


// ---------------------------------------

ErrorException::ErrorException(const char *info) : Exception(info)
{
}


ErrorException::~ErrorException()
{
}


void ErrorException::Report()
{
#ifdef _MSC_VER
  _RPT1(_CRT_WARN, "Warning : %s\n", m_info);
#endif
  printf("ERROR: %s\n", m_info);
  MessageBox(NULL, m_info, "TurboVNC info",
             MB_OK | MB_ICONSTOP | MB_SETFOREGROUND | MB_TOPMOST);
}


// ---------------------------------------

AuthException::AuthException(const char *info, bool close) : WarningException(info)
{
  m_close = close;
}


AuthException::~AuthException()
{
}


void AuthException::Report()
{
#ifdef _MSC_VER
  _RPT1(_CRT_WARN, "Warning : %s\n", m_info);
#endif
  printf("AUTHENTICATION FAILURE: %s\n", m_info);
  MessageBox(NULL, m_info, "TurboVNC authentication info",
             MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TOPMOST);
}
