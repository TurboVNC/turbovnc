check_function_exists(strnlen HAVE_STRNLEN)
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
