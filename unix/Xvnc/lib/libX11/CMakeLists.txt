configure_file(include/X11/XlibConf.h.in include/X11/XlibConf.h)

set(HEADERS
	include/X11/Xlib.h
	${CMAKE_CURRENT_BINARY_DIR}/include/X11/XlibConf.h
	include/X11/Xlibint.h
	include/X11/Xlib-xcb.h
	include/X11/Xutil.h)

foreach(h ${HEADERS})
	copy_X_header(${h} X11)
endforeach()

include_directories(include/X11)

handle_type_puns()

add_library(X11xcb STATIC
	src/Xlibint.c
	src/globals.c
	src/x11_xcb.c
	src/xcb_io.c)
