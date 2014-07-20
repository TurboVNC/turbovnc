copy_X_header(Xdmcp.h X11)

include(CheckFunctionExists)
check_function_exists(srand48 HAVE_SRAND48)
if(HAVE_SRAND48)
	add_definitions(-DHAVE_SRAND48)
endif()
check_function_exists(lrand48 HAVE_LRAND48)
if(HAVE_LRAND48)
	add_definitions(-DHAVE_LRAND48)
endif()

add_definitions(-DHASXDMAUTH)

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
