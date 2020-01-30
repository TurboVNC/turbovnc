copy_X_header(include/X11/Xdmcp.h X11)

check_symbol_exists(arc4random_buf stdlib.h HAVE_ARC4RANDOM_BUF)
if(HAVE_ARC4RANDOM_BUF)
	add_definitions(-DHAVE_ARC4RANDOM_BUF)
endif()
check_symbol_exists(srand48 stdlib.h HAVE_SRAND48)
if(HAVE_SRAND48)
	add_definitions(-DHAVE_SRAND48)
endif()
check_symbol_exists(lrand48 stdlib.h HAVE_LRAND48)
if(HAVE_LRAND48)
	add_definitions(-DHAVE_LRAND48)
endif()

add_definitions(-DHASXDMAUTH)

disable_compiler_warnings()

add_library(Xdmcp STATIC
	Array.c
	Fill.c
	Flush.c
	Key.c
	Read.c
	Unwrap.c
	Wrap.c
	Wraphelp.c
	Write.c)
