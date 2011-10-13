/* $XFree86: xc/lib/font/stubs/initfshdl.c,v 1.1 1999/01/11 05:13:20 dawes Exp $ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "stubs.h"

int
init_fs_handlers(FontPathElementPtr fpe,
                 BlockHandlerProcPtr block_handler)
{
  return Successful;
}

/* end of file */
