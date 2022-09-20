/*
 * sprite.h
 *
 * software-sprite/sprite drawing interface spec - based on misprite
 */

/* Copyright (C) 2014 D. R. Commander.  All Rights Reserved.
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

/*

Copyright 1989, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.
*/

extern Bool rfbSpriteInitialize(ScreenPtr, miPointerScreenFuncPtr);
extern void rfbSpriteGetCursorPos(ScreenPtr, int *, int *);
extern CursorPtr rfbSpriteGetCursorPtr(ScreenPtr);
extern void rfbSpriteRemoveCursorAllDev(ScreenPtr pScreen);
extern void rfbSpriteRestoreCursorAllDev(ScreenPtr pScreen);

extern Bool rfbDCRealizeCursor(ScreenPtr pScreen, CursorPtr pCursor);
extern Bool rfbDCUnrealizeCursor(ScreenPtr pScreen, CursorPtr pCursor);
extern Bool rfbDCPutUpCursor(DeviceIntPtr pDev, ScreenPtr pScreen,
                             CursorPtr pCursor, int x, int y,
                             unsigned long source, unsigned long mask);
extern Bool rfbDCSaveUnderCursor(DeviceIntPtr pDev, ScreenPtr pScreen,
                                 int x, int y, int w, int h);
extern Bool rfbDCRestoreUnderCursor(DeviceIntPtr pDev, ScreenPtr pScreen,
                                    int x, int y, int w, int h);
extern Bool rfbDCDeviceInitialize(DeviceIntPtr pDev, ScreenPtr pScreen);
extern void rfbDCDeviceCleanup(DeviceIntPtr pDev, ScreenPtr pScreen);
