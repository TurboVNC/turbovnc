/* $XdotOrg: xc/lib/Xdmcp/Wrap.h,v 1.6 2005/07/03 07:00:56 daniels Exp $ */
/* $Xorg: Wrap.h,v 1.3 2000/08/17 19:45:50 cpqbld Exp $ */
/*
 * header file for compatibility with something useful
 */

/* $XFree86: xc/lib/Xdmcp/Wrap.h,v 1.3 2001/01/17 19:42:44 dawes Exp $ */

typedef unsigned char auth_cblock[8];	/* block size */

typedef struct auth_ks_struct { auth_cblock _; } auth_wrapper_schedule[16];

extern void _XdmcpWrapperToOddParity (unsigned char *in, unsigned char *out);

#ifdef HASXDMAUTH
extern void _XdmcpAuthSetup (auth_cblock key, auth_wrapper_schedule schedule);
extern void _XdmcpAuthDoIt (auth_cblock input, auth_cblock output,
	auth_wrapper_schedule schedule, int edflag);
#endif
