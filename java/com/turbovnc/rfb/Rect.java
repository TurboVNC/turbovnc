/* Copyright (C) 2017-2018 D. R. Commander.  All Rights Reserved.
 * Copyright (C) 2011 Brian P. Hinz
 * Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
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

package com.turbovnc.rfb;

public class Rect {

  // Rect
  //
  // Represents a rectangular region defined by its top-left (tl)
  // and bottom-right (br) Points.
  // Rects may be compared for equality, checked to determine whether
  // or not they are empty, cleared (made empty), or intersected with
  // one another.  The bounding rectangle of two existing Rects
  // may be calculated, as may the area of a Rect.
  // Rects may also be translated, in the same way as Points, by
  // an offset specified in a Point structure.

  public Rect() {
    tl = new Point(0, 0);
    br = new Point(0, 0);
  }

  public Rect(Point tl_, Point br_) {
    tl = new Point(tl_.x, tl_.y);
    br = new Point(br_.x, br_.y);
  }

  public Rect(int x1, int y1, int x2, int y2) {
    tl = new Point(x1, y1);
    br = new Point(x2, y2);
  }

  public final void setXYWH(int x, int y, int w, int h) {
    tl.x = x;  tl.y = y;  br.x = x + w;  br.y = y + h;
  }

  public final boolean equals(Rect r) {
    return r.tl.equals(tl) && r.br.equals(br);
  }

  public final boolean isEmpty() {
    return (tl.x >= br.x) || (tl.y >= br.y);
  }

  public final void clear() { tl = new Point();  br = new Point(); }

  public final boolean enclosedBy(Rect r) {
    return (tl.x >= r.tl.x) && (tl.y >= r.tl.y) &&
           (br.x <= r.br.x) && (br.y <= r.br.y);
  }

  public final boolean overlaps(Rect r) {
    return tl.x < r.br.x && tl.y < r.br.y && br.x > r.tl.x && br.y > r.tl.y;
  }

  public final Rect intersection(Rect r) {
    if (!overlaps(r))
      return new Rect(0, 0, 0, 0);
    return new Rect(Math.max(tl.x, r.tl.x), Math.max(tl.y, r.tl.y),
                    Math.min(br.x, r.br.x), Math.min(br.y, r.br.y));
  }

  public final int area() {
    int area = (br.x - tl.x) * (br.y - tl.y);
    if (area > 0)
      return area;
    return 0;
  }

  public final Point dimensions() { return new Point(width(), height()); }

  public final int width() { return br.x - tl.x; }

  public final int height() { return br.y - tl.y; }

  public final boolean contains(Point p) {
    return (tl.x <= p.x) && (tl.y <= p.y) && (br.x > p.x) && (br.y > p.y);
  }

  // CHECKSTYLE VisibilityModifier:OFF
  public Point tl;
  public Point br;
  // CHECKSTYLE VisibilityModifier:ON
}
