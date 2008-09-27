/* $XConsortium: Wrap.h,v 1.3 94/02/06 20:09:49 rws Exp $ */
/*
 * header file for compatibility with something useful
 */

typedef unsigned char auth_cblock[8];	/* block size */

typedef struct auth_ks_struct { auth_cblock _; } auth_wrapper_schedule[16];

extern void _XdmcpWrapperToOddParity();

extern void _XdmcpAuthDoIt (
		 auth_cblock                input,
		 auth_cblock                output,
		 auth_wrapper_schedule      schedule,
		 int                        edflag
);

extern void _XdmcpAuthSetup (
		 auth_cblock                key,
		 auth_wrapper_schedule      schedule
);
