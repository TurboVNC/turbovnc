//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
//  Copyright (C) 2015, 2017 D. R. Commander. All Rights Reserved.
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

// This is an object and macros that provide general logging and debugging
// functionality.  The class can log to a file, to a new console, and/or to
// the debug console.  Every log object has a logging level (which can be
// changed.)  Only log requests whose level is >= the logging level will
// actually be logged, so the level can be thought of as an 'amount of detail'.
//
// Typical use:
//
//       Log vnclog;
//       vnclog.SetFile("myapp.log");
//       ...
//       vnclog.Print(2, "x = %d\n", x);
//

#ifndef LOG_H__
#define LOG_H__

#pragma once

#include <stdarg.h>


class Log
{
  public:
    // Logging mode flags:
    static const int ToDebug;
    static const int ToFile;
    static const int ToConsole;
    static const int UseStdio;

    // Create a new log object.
    // Parameters as follows:
    //    mode     - specifies where output should go, using combination
    //               of flags above.
    //    level    - the default level
    //    filename - if flag Log::ToFile is specified in the type,
    //               a filename must be specified here.
    //    append   - if logging to a file, whether or not to append to any
    //               existing log.
    Log(int mode = ToDebug, int level = 1, LPTSTR filename = NULL,
        bool append = false);

    inline void Print(int level, LPTSTR format, ...)
    {
      if (level >= 100) {
        if (level / 100 > m_giiLevel) return;
      } else if (level > m_level) return;
      va_list ap;
      va_start(ap, format);
      ReallyPrint(format, ap);
      va_end(ap);
    }

    // Change the log level
    void SetLevel(int level);

    // Change the logging mode
    void SetMode(int mode);

    // Change or set the logging filename.  This enables ToFile mode if
    // not already enabled.
    void SetFile(LPTSTR filename, bool append = false);

    virtual ~Log();

  private:
    void ReallyPrint(LPTSTR format, va_list ap);
    void CloseFile();
    bool m_tofile, m_todebug, m_toconsole, m_usestdio;
    int m_level, m_giiLevel;
    HANDLE hlogfile;
};

#endif  // LOG_H__
