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
 * caps.c
 */

#include "vncviewer.h"

static int CapsIndex(CapsContainer *pcaps, CARD32 code);

/*
 * The constructor.
 */

CapsContainer *
CapsNewContainer(void)
{
  CapsContainer *pcaps;

  pcaps = malloc(sizeof(CapsContainer));
  if (pcaps != NULL) {
    pcaps->known_count = 0;
    pcaps->enabled_count = 0;
  }

  return pcaps;
}

/*
 * The destructor.
 */

void
CapsDeleteContainer(CapsContainer *pcaps)
{
  int i;

  for (i = 0; i < pcaps->known_count; i++) {
    if (pcaps->descriptions[i] != NULL)
      free(pcaps->descriptions[i]);
  }

  free(pcaps);
}

/*
 * Add information about a particular capability into the object. There are
 * two functions to perform this task. These functions overwrite capability
 * records with the same code.
 */

void
CapsAdd(CapsContainer *pcaps,
        CARD32 code, char *vendor, char *name, char *desc)
{
  /* Fill in an rfbCapabilityInfo structure and pass it to CapsAddInfo(). */
  rfbCapabilityInfo capinfo;
  capinfo.code = code;
  memcpy(capinfo.vendorSignature, vendor, sz_rfbCapabilityInfoVendor);
  memcpy(capinfo.nameSignature, name, sz_rfbCapabilityInfoName);
  CapsAddInfo(pcaps, &capinfo, desc);
}

void
CapsAddInfo(CapsContainer *pcaps,
            rfbCapabilityInfo *capinfo, char *desc)
{
  int i;
  char *desc_copy;

  i = CapsIndex(pcaps, capinfo->code);
  if (i == -1) {
    if (pcaps->known_count >= TIGHTVNC_MAX_CAPS) {
      return;                   /* container full */
    }
    i = pcaps->known_count++;
    pcaps->known_list[i] = capinfo->code;
    pcaps->descriptions[i] = NULL;
  }

  pcaps->known_info[i] = *capinfo;
  pcaps->enable_flags[i] = (char)False;
  if (pcaps->descriptions[i] != NULL) {
    free(pcaps->descriptions[i]);
  }

  desc_copy = NULL;
  if (desc != NULL) {
    desc_copy = strdup(desc);
  }
  pcaps->descriptions[i] = desc_copy;
}

/*
 * Check if a capability with the specified code was added earlier.
 */

static int
CapsIndex(CapsContainer *pcaps, CARD32 code)
{
  int i;

  for (i = 0; i < pcaps->known_count; i++) {
    if (pcaps->known_list[i] == code)
      return i;
  }

  return -1;
}

Bool
CapsIsKnown(CapsContainer *pcaps, CARD32 code)
{
  return (CapsIndex(pcaps, code) != -1);
}

/*
 * Fill in a rfbCapabilityInfo structure with contents corresponding to the
 * specified code. Returns True on success, False if the specified code is
 * not known.
 */

Bool
CapsGetInfo(CapsContainer *pcaps, CARD32 code, rfbCapabilityInfo *capinfo)
{
  int i;

  i = CapsIndex(pcaps, code);
  if (i != -1) {
    *capinfo = pcaps->known_info[i];
    return True;
  }

  return False;
}

/*
 * Get a description string for the specified capability code. Returns NULL
 * either if the code is not known, or if there is no description for this
 * capability.
 */

char *
CapsGetDescription(CapsContainer *pcaps, CARD32 code)
{
  int i;

  i = CapsIndex(pcaps, code);
  if (i != -1) {
    return pcaps->descriptions[i];
  }

  return NULL;
}

/*
 * Mark the specified capability as "enabled". This function checks "vendor"
 * and "name" signatures in the existing record and in the argument structure
 * and enables the capability only if both records are the same.
 */

Bool
CapsEnable(CapsContainer *pcaps, rfbCapabilityInfo *capinfo)
{
  int i;
  rfbCapabilityInfo *known;

  i = CapsIndex(pcaps, capinfo->code);
  if (i == -1)
    return False;

  known = &pcaps->known_info[i];
  if ( memcmp(known->vendorSignature, capinfo->vendorSignature,
              sz_rfbCapabilityInfoVendor) != 0 ||
       memcmp(known->nameSignature, capinfo->nameSignature,
              sz_rfbCapabilityInfoName) != 0 ) {
    pcaps->enable_flags[i] = (char)False;
    return False;
  }

  /* Cannot happen, but just in case. */
  if (pcaps->enabled_count >= TIGHTVNC_MAX_CAPS) {
    pcaps->enable_flags[i] = (char)False;
    return False;
  }

  pcaps->enable_flags[i] = (char)True;
  pcaps->enabled_list[pcaps->enabled_count++] = capinfo->code;
  return True;
}

/*
 * Check if the specified capability is known and enabled.
 */

Bool
CapsIsEnabled(CapsContainer *pcaps, CARD32 code)
{
  int i;

  i = CapsIndex(pcaps, code);
  if (i != -1) {
    return (pcaps->enable_flags[i] != (char)False);
  }

  return False;
}

/*
 * Return the number of enabled capabilities.
 */

int CapsNumEnabled(CapsContainer *pcaps)
{
  return pcaps->enabled_count;
}

/*
 * Return the capability code at the specified index.
 * If the index is not valid, return 0.
 */

CARD32
CapsGetByOrder(CapsContainer *pcaps, int idx)
{
  return (idx < pcaps->enabled_count) ? pcaps->enabled_list[idx] : 0;
}

