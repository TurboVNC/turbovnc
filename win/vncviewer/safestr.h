//  Copyright (C) 2012, 2016 D. R. Commander. All Rights Reserved.
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


// _snprintf() doesn't always terminate the string, so we use _snprintf_s()
// instead and define a macro that works like the Unix version.
#define snprintf(str, n, format, ...)  \
  _snprintf_s(str, n, _TRUNCATE, format, __VA_ARGS__)


// Safe, truncating version of sprintf() for static destination buffers
#define SPRINTF(str, format, ...)  \
  snprintf(str, _countof(str), format, __VA_ARGS__)


// Safe, truncating version of strncat()
#define STRNCAT(dst, src, n)  \
  strncat_s(dst, n, src, _TRUNCATE)


// Safe, truncating version of strcat() for static destination buffers
#define STRCAT(dst, src)  \
  STRNCAT(dst, src, _countof(dst))


// Safe, truncating version of strncpy()
#define STRNCPY(dst, src, n)  \
  strncpy_s(dst, n, src, _TRUNCATE)


// Safe, truncating version of strcpy() for static destination buffers.
#define STRCPY(dst, src)  \
  STRNCPY(dst, src, _countof(dst))


extern "C" char *strsep(char **stringp, const char *delim);
