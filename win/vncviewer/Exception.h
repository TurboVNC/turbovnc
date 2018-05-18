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

#ifndef EXCEPTION_H__
#define EXCEPTION_H__

#pragma once


class Exception
{
  public:
    Exception(const char *info = NULL);
    virtual void Report();
    virtual ~Exception();
    char *m_info;
};


// This indicates that the catcher should close the connection quietly.
// Report() will display a TRACE message

class QuietException : public Exception
{
  public:
    QuietException(const char *info = NULL);
    virtual void Report();
    virtual ~QuietException();
};


// This indicates a not-necessarily-fatal situation about which the user should
// be informed.  In cases of ambiguity, the 'close' parameter can be used to
// specify whether or not the connection is closed as a result.  In general, it
// will be.  Report() will display a message box.

class WarningException : public Exception
{
  public:
    WarningException(const char *info = NULL, bool close = true);
    virtual void Report();
    virtual ~WarningException();
    bool m_close;
};


// This indicates a fatal condition.  Report() will display an important
// message box.  Connection will definitely be closed.

class ErrorException : public Exception
{
  public:
    ErrorException(const char *info = NULL);
    virtual void Report();
    virtual ~ErrorException();
};


// This indicates a not-necessarily-fatal situation about which the user should
// be informed.  In cases of ambiguity, the 'close' parameter can be used to
// specify whether or not the connection is closed as a result.  In general, it
// will be.  Report() will display a message box.
class AuthException : public WarningException
{
  public:
    AuthException(const char *info = NULL, bool close = true);
    virtual void Report();
    virtual ~AuthException();
};

#endif  // EXCEPTION_H__
