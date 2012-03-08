/* $XFree86: xc/lib/font/include/fontmod.h,v 1.3 1998/12/13 05:32:33 dawes Exp $ */

#ifndef _FONTMOD_H_
#define _FONTMOD_H_

typedef void (*InitFont)(void);

typedef struct {
    InitFont	initFunc;
    char *	name;
    pointer	module;
} FontModule;

extern FontModule *FontModuleList;

#endif /* _FONTMOD_H_ */
