#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "stubs.h"

#ifdef __SUNPRO_C
#pragma weak GetNewFontClientID
#endif

weak Font
GetNewFontClientID(void)
{
    return (Font)0;
}
