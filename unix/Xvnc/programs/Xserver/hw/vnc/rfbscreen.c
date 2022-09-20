/* Copyright (C) 2017, 2019 D. R. Commander.  All Rights Reserved.
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

#include "rfb.h"
#include <stdlib.h>
#include <string.h>


struct xorg_list rfbScreens;


void rfbAddScreen(struct xorg_list *list, rfbScreenInfo *screen)
{
  if (!list || !screen) return;

  xorg_list_append(&screen->entry, list);
}


void rfbClipScreens(struct xorg_list *list, int w, int h)
{
  rfbScreenInfo *screen, *tmp;

  xorg_list_for_each_entry_safe(screen, tmp, list, entry) {
    if (screen->s.x >= w || screen->s.y >= h) {
      rfbRemoveScreen(screen);
      continue;
    }
    if (screen->s.x + screen->s.w > w)
      screen->s.w = w - screen->s.x;
    if (screen->s.y + screen->s.h > h)
      screen->s.h = h - screen->s.y;
  }
}


static rfbScreenInfo *rfbDupeScreen(rfbScreenInfo *screen)
{
  rfbScreenInfo *newScreen;

  if (!screen) return NULL;

  newScreen = (rfbScreenInfo *)rfbAlloc0(sizeof(rfbScreenInfo));
  memcpy(newScreen, screen, sizeof(rfbScreenInfo));

  return newScreen;
}


void rfbDupeScreens(struct xorg_list *newList, struct xorg_list *list)
{
  rfbScreenInfo *screen, *newScreen;

  if (!list || !newList) return;

  xorg_list_init(newList);
  xorg_list_for_each_entry(screen, list, entry) {
    newScreen = rfbDupeScreen(screen);
    rfbAddScreen(newList, newScreen);
  }
}


rfbScreenInfo *rfbFindScreen(struct xorg_list *list, CARD16 x, CARD16 y,
                             CARD16 w, CARD16 h)
{
  rfbScreenInfo *screen;

  if (!list) return NULL;

  xorg_list_for_each_entry(screen, list, entry) {
    if (screen->s.x == x && screen->s.y == y && screen->s.w == w &&
        screen->s.h == h && screen->idAssigned)
      return screen;
  }

  return NULL;
}


rfbScreenInfo *rfbFindScreenID(struct xorg_list *list, CARD32 id)
{
  rfbScreenInfo *screen;

  if (!list) return NULL;

  xorg_list_for_each_entry(screen, list, entry) {
    if (screen->s.id == id)
      return screen;
  }

  return NULL;
}


rfbScreenInfo *rfbNewScreen(CARD32 id, CARD16 x, CARD16 y, CARD16 w, CARD16 h,
                            CARD32 flags)
{
  rfbScreenInfo *screen = (rfbScreenInfo *)rfbAlloc0(sizeof(rfbScreenInfo));

  screen->s.id = id;
  screen->s.x = x;
  screen->s.y = y;
  screen->s.w = w;
  screen->s.h = h;
  screen->s.flags = flags;

  return screen;
}


void rfbRemoveScreen(rfbScreenInfo *screen)
{
  if (!screen) return;

  xorg_list_del(&screen->entry);
  free(screen);
}


void rfbRemoveScreens(struct xorg_list *list)
{
  rfbScreenInfo *screen, *tmp;

  if (!list) return;

  xorg_list_for_each_entry_safe(screen, tmp, list, entry)
    rfbRemoveScreen(screen);
}
