/* $Xorg: Wrap.h,v 1.3 2000/08/17 19:45:50 cpqbld Exp $ */
/*
 * header file for compatibility with something useful
 */

/* $XFree86: xc/lib/Xdmcp/Wrap.h,v 1.4 2003/12/19 02:05:38 dawes Exp $ */

typedef unsigned char auth_cblock[8];	/* block size */

typedef struct auth_ks_struct { auth_cblock _; } auth_wrapper_schedule[16];

extern void _XdmcpWrapperToOddParity (unsigned char *in, unsigned char *out);

#ifdef HASXDMAUTH
/* They're really void but that would mean changing Wraphelp.c as well
   which isnt in version control. */
extern void _XdmcpAuthSetup (auth_cblock key, auth_wrapper_schedule schedule);
extern void _XdmcpAuthDoIt (auth_cblock input, auth_cblock output,
	auth_wrapper_schedule schedule, int edflag);
#endif
