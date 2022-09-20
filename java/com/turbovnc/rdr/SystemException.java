/* Copyright (C) 2012, 2020 D. R. Commander.  All Rights Reserved.
 * Copyright (C) 2012 Brian P. Hinz
 * Copyright (C) 2010 TigerVNC Team
 * Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright (C) 2004 Red Hat Inc.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

/* This is used solely to pass Java exceptions up to the main viewer class,
   so we don't have to declare them to be thrown everywhere. */

package com.turbovnc.rdr;

public class SystemException extends RuntimeException {
  public SystemException(Throwable e) {
    super(e);
  }

  public static final void checkException(Exception e) {
    Throwable cause = e.getCause();
    if (cause instanceof ErrorException)
      throw (ErrorException)cause;
    else if (cause instanceof WarningException)
      throw (WarningException)cause;
    else if (cause instanceof SystemException)
      throw (SystemException)cause;
    else if (e instanceof ErrorException)
      throw (ErrorException)e;
    else if (e instanceof WarningException)
      throw (WarningException)e;
    else if (e instanceof SystemException)
      throw (SystemException)e;
  }
}
