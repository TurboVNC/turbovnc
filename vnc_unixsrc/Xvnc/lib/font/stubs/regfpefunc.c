/* $XFree86: xc/lib/font/stubs/regfpefunc.c,v 1.1 1999/01/11 05:13:20 dawes Exp $ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "stubs.h"

int 
RegisterFPEFunctions(NameCheckFunc name_func, 
		     InitFpeFunc init_func, 
		     FreeFpeFunc free_func, 
		     ResetFpeFunc reset_func, 
		     OpenFontFunc open_func, 
		     CloseFontFunc close_func, 
		     ListFontsFunc list_func, 
		     StartLfwiFunc start_lfwi_func, 
		     NextLfwiFunc next_lfwi_func, 
		     WakeupFpeFunc wakeup_func, 
		     ClientDiedFunc client_died, 
		     LoadGlyphsFunc load_glyphs, 
		     StartLaFunc start_list_alias_func, 
		     NextLaFunc next_list_alias_func, 
		     SetPathFunc set_path_func)
{
    return 0;
}

/* end of file */
