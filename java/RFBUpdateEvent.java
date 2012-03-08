//
//  Copyright (C) 2006 Sun Microsystems, Inc.  All Rights Reserved.
//
//  This is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this software; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
//  USA.
//

import java.awt.*;
import java.awt.event.*;

class RFBUpdateEvent extends AWTEvent {

  public static final int RFBUPDATE_FIRST = AWTEvent.RESERVED_ID_MAX + 1;
  public static final int RFBUPDATE_LAST = RFBUPDATE_FIRST;

  public RFBUpdateEvent (Component source, int id) {
    super(source, id);
  }

  public String paramString() {
    String s = "SendRFBUpdate";
    return s;
  }

}
