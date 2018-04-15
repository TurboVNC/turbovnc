if(FOUND_GLIBC)
	set(CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
endif()
check_symbol_exists(strnlen string.h HAVE_STRNLEN)
if(FOUND_GLIBC)
	set(CMAKE_REQUIRED_DEFINITIONS)
endif()
if(NOT HAVE_STRNLEN)
	set(STRNLEN_SOURCE strnlen.c)
endif()

add_library(bitmap OBJECT
	bdfread.c
	bdfutils.c
	bitmap.c
	bitmapfunc.c
	bitmaputil.c
	bitscale.c
	fontink.c
	pcfread.c
	pcfwrite.c
	snfread.c
	${STRNLEN_SOURCE})
