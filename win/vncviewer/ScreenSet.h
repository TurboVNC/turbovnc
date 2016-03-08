/* Copyright 2009 Pierre Ossman for Cendio AB
 * Copyright (C)2015-2016 D. R. Commander.  All Rights Reserved.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 * USA.
 */

// Management class for the RFB virtual screens

#ifndef SCREENSET_H__
#define SCREENSET_H__

#include <stdio.h>
#include <string.h>

#include <list>
#include <set>
#include "safestr.h"

// Screen
//
// Represents a single RFB virtual screen, which includes
// coordinates, an id and flags.

struct Screen {
  Screen(void) : id(0), flags(0) {};
  Screen(unsigned int id_, int x_, int y_, int w_, int h_,
         unsigned int flags_) : id(id_), flags(flags_) {
    SetRect(&dimensions, x_, y_, x_ + w_, y_ + h_);
  };

  inline bool operator==(const Screen& r) const {
    if (id != r.id)
      return false;
    if (!EqualRect(&dimensions, &r.dimensions))
      return false;
    if (flags != r.flags)
      return false;
    return true;
  }

  unsigned int id;
  RECT dimensions;
  unsigned int flags;
};

// ScreenSet
//
// Represents a complete screen configuration, excluding framebuffer
// dimensions.

struct ScreenSet {
  ScreenSet(void) {};

  typedef std::list<Screen>::iterator iterator;
  typedef std::list<Screen>::const_iterator const_iterator;

  inline iterator begin(void) { return screens.begin(); };
  inline const_iterator begin(void) const { return screens.begin(); };
  inline iterator end(void) { return screens.end(); };
  inline const_iterator end(void) const { return screens.end(); };

  inline int num_screens(void) const { return (int)screens.size(); };

  inline void add_screen(const Screen screen) { screens.push_back(screen); };
  inline void remove_screen(unsigned int id) {
    std::list<Screen>::iterator iter, nextiter;
    for (iter = screens.begin(); iter != screens.end(); iter = nextiter) {
      nextiter = iter; nextiter++;
      if (iter->id == id)
          screens.erase(iter);
    }
  }

  inline bool validate(int fb_width, int fb_height) const {
    std::list<Screen>::const_iterator iter;
    std::set<unsigned int> seen_ids;

    if (screens.empty())
      return false;
    if (num_screens() > 255)
      return false;

    for (iter = screens.begin(); iter != screens.end(); ++iter) {
      if (IsRectEmpty(&iter->dimensions))
        return false;
      if (iter->dimensions.left < 0 || iter->dimensions.top < 0 ||
          iter->dimensions.right > fb_width ||
          iter->dimensions.bottom > fb_height)
        return false;
      if (seen_ids.find(iter->id) != seen_ids.end())
        return false;
      seen_ids.insert(iter->id);
    }

    return true;
  };

  inline void print(char* str, size_t len) const {
    char buffer[128];
    std::list<Screen>::const_iterator iter;
    SPRINTF(buffer, "%d screen(s)\n", num_screens());
    str[0] = '\0';
    STRNCAT(str, buffer, len);
    for (iter = screens.begin(); iter != screens.end(); ++iter) {
      SPRINTF(buffer,
               "    %10d (0x%08x): %dx%d+%d+%d (flags 0x%08x)\n",
               (int)iter->id, (unsigned)iter->id,
               iter->dimensions.right - iter->dimensions.left,
               iter->dimensions.bottom - iter->dimensions.top,
               iter->dimensions.left, iter->dimensions.top,
               (unsigned)iter->flags);
      STRNCAT(str, buffer, len);
    }
  };

  // FIXME: List order shouldn't matter
  inline bool operator==(const ScreenSet& r) const {
    return screens == r.screens;
  }

  inline bool operator!=(const ScreenSet& r) const {
    return screens != r.screens;
  }

  std::list<Screen> screens;
};

#endif
