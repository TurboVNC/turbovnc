/***************************************************************************/
/*                                                                         */
/*  ft2build.h                                                             */
/*                                                                         */
/*    FreeType 2 build and setup macros.                                   */
/*                                                                         */
/*  Copyright 1996-2001 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  Modified for XFree86.                                                  */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

/* $XFree86: xc/lib/font/FreeType/ft2build.h,v 1.3 2002/10/01 00:02:10 alanh Exp $ */

/* $XdotOrg: xc/lib/font/FreeType/module/ft2build.h,v 1.3 2005/07/03 07:00:58 daniels Exp $ */
  /*************************************************************************/
  /*                                                                       */
  /* This file corresponds to the default "ft2build.h" file for            */
  /* FreeType 2.  It uses the "freetype" include root.                     */
  /*                                                                       */
  /* Note that specific platforms might use a different configuration.     */
  /* See builds/unix/ft2unix.h for an example.                             */
  /*                                                                       */
  /*************************************************************************/

#ifndef __FT2_BUILD_GENERIC_H__
#define __FT2_BUILD_GENERIC_H__

# if defined (FONTMODULE)
#  include "ftheader.h"
# else
#include <freetype/config/ftheader.h>
# endif

#endif /* __FT2_BUILD_GENERIC_H__ */


/* END */
