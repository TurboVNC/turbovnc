#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "stubs.h"

#ifdef __SUNPRO_C
#pragma weak FatalError
#endif

weak void
FatalError(const char *f, ...)
{
}
