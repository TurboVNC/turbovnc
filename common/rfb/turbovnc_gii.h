/* Copyright (C) 2015, 2021 D. R. Commander.  All Rights Reserved.
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

/* The TurboVNC implementation of the GII extension uses the productID field
   in rfbGIIDeviceCreateMsg to send the X Input device type to the server.
   This is not required by the GII spec and is subject to change. */

#define rfbGIIDevTypeNone   0
#define rfbGIIDevTypeCursor 1
#define rfbGIIDevTypeStylus 2
#define rfbGIIDevTypeEraser 3
#define rfbGIIDevTypeTouch  4
#define rfbGIIDevTypePad    5

/* The TurboVNC implementation of the GII extension uses fake valuators to send
   X Input multitouch events to the server.  This is not required by the GII
   spec and is subject to change. */

#define rfbGIITouchBegin    0
#define rfbGIITouchUpdate   1
#define rfbGIITouchEnd      2
/* These event types signal to the server that pointer emulation should be
   enabled. */
#define rfbGIITouchBeginEP  3
#define rfbGIITouchUpdateEP 4
#define rfbGIITouchEndEP    5
