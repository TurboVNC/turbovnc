/*
 *  Copyright (C) 2003 Constantin Kaplinsky.  All Rights Reserved.
 *
 *  This is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this software; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 *  USA.
 */

/*
 * caps.h
 */

#ifndef _VNC_CAPSCONTAINER
#define _VNC_CAPSCONTAINER

/* FIXME: Don't limit the number of capabilities. */
#define TIGHTVNC_MAX_CAPS  64

typedef struct _CapsContainer
{
  int known_count;
  CARD32 known_list[TIGHTVNC_MAX_CAPS];
  rfbCapabilityInfo known_info[TIGHTVNC_MAX_CAPS];
  char *descriptions[TIGHTVNC_MAX_CAPS];
  char enable_flags[TIGHTVNC_MAX_CAPS];

  /* These are redundant, but improve the performance. */
  int enabled_count;
  CARD32 enabled_list[TIGHTVNC_MAX_CAPS];

} CapsContainer;

CapsContainer *CapsNewContainer(void);
void CapsDeleteContainer(CapsContainer *pcaps);

void CapsAdd(CapsContainer *pcaps,
             CARD32 code, char *vendor, char *name, char *desc);
void CapsAddInfo(CapsContainer *pcaps,
                 rfbCapabilityInfo *capinfo, char *desc);

Bool CapsIsKnown(CapsContainer *pcaps, CARD32 code);
Bool CapsGetInfo(CapsContainer *pcaps, CARD32 code,
                 rfbCapabilityInfo *capinfo);
char *CapsGetDescription(CapsContainer *pcaps, CARD32 code);

Bool CapsEnable(CapsContainer *pcaps, rfbCapabilityInfo *capinfo);
Bool CapsIsEnabled(CapsContainer *pcaps, CARD32 code);
int CapsNumEnabled(CapsContainer *pcaps);
CARD32 CapsGetByOrder(CapsContainer *pcaps, int idx);

#endif /* _VNC_CAPSCONTAINER */

