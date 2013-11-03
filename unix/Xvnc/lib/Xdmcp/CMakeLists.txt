copy_X_header(Xdmcp.h X11)

add_definitions(-DTCPCONN -DUNIXCONN -DHASXDMAUTH)

add_library(Xdmcp STATIC
	A8Eq.c
	AA8.c
	AA16.c
	AA32.c
	AofA8.c
	CA8.c
	DA8.c
	DA16.c
	DA32.c
	DAofA8.c
	Fill.c
	Flush.c
	RA8.c
	RA16.c
	RA32.c
	RAofA8.c
	RC8.c
	RC16.c
	RC32.c
	RHead.c
	RR.c
	RaA8.c
	RaA16.c
	RaA32.c
	RaAoA8.c
	WA8.c
	WA16.c
	WA32.c
	WAofA8.c
	WC8.c
	WC16.c
	WC32.c
	Whead.c
	Alloc.c
	CmpKey.c
	DecKey.c
	GenKey.c
	IncKey.c
	Wrap.c
	Unwrap.c
	Wraphelp.c)
