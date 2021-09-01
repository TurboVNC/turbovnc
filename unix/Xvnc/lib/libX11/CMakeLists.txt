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

add_definitions(-DHAVE_SYS_SELECT_H)
include(CheckIncludeFiles)
check_include_files(sys/ioctl.h HAVE_SYS_IOCTL_H)
if(HAVE_SYS_IOCTL_H)
	add_definitions(-DHAVE_SYS_IOCTL_H)
endif()
check_include_files(sys/filio.h HAVE_SYS_FILIO_H)
if(HAVE_SYS_FILIO_H)
	add_definitions(-DHAVE_SYS_FILIO_H)
endif()

disable_compiler_warnings()

add_library(X11xcb STATIC
	src/XlibInt.c
	src/globals.c
	src/reallocarray.c
	src/x11_xcb.c
	src/xcb_io.c)
