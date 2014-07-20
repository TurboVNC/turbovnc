#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "stubs.h"

#ifdef __SUNPRO_C
#pragma weak set_font_authorizations
#endif

weak int
set_font_authorizations(char **authorizations, int *authlen, ClientPtr client)
{
    return 0;
}
