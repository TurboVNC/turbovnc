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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
// TightVNC distribution homepage on the Web: http://www.tightvnc.com/
//
// If the source code for the VNC system is not available from the place 
// whence you received this file, check http://www.uk.research.att.com/vnc or contact
// the authors on vnc@uk.research.att.com for information on obtaining it.


#include "stdhdrs.h"
#include "Exception.h"

Exception::Exception(const char *info)
{
	assert(info != NULL);
	m_info = new char[strlen(info)+1];
	strcpy(m_info, info);
}

Exception::~Exception()
{
	delete [] m_info;
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

WarningException::WarningException(const char *info, bool close) : Exception(info)
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
	MessageBox(NULL, m_info, "TightVNC info", MB_OK| MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TOPMOST);
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
	MessageBox(NULL, m_info, "TightVNC info", MB_OK | MB_ICONSTOP | MB_SETFOREGROUND | MB_TOPMOST);
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
	MessageBox(NULL, m_info, "TightVNC authentication info", MB_OK| MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TOPMOST);
}
