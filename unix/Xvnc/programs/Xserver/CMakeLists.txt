include_directories(Xext include miext/damage present ../../lib/pixman/pixman
	${CMAKE_CURRENT_BINARY_DIR}/include)

# Enable extensions
add_definitions(-DCOMPOSITE -DDAMAGE -DDPMSExtension -DMITSHM -DPANORAMIX
	-DPRESENT -DRANDR -DRENDER -DRES -DSCREENSAVER -DSHAPE -DXACE -DXCSECURITY
	-DXFIXES -DXF86BIGFONT -DXINPUT -DXRECORD -DXTEST -DXV -DTURBOVNC)

if(TVNC_NVCONTROL)
	add_definitions(-DNVCONTROL)
endif()
if(TVNC_GLX)
	add_definitions(-DGLXEXT)
endif()

add_definitions(-DHAVE_DIX_CONFIG_H -DCLIENTIDS -DHAS_SHM -DHAVE_XSHMFENCE)

if(BITS EQUAL 64)
	add_definitions(-D_XSERVER64)
endif()

set(VENDOR_NAME "The X.Org Foundation")
set(VENDOR_RELEASE "12014000")
configure_file(include/version-config.h.in include/version-config.h)

set(CMAKE_REQUIRED_LIBRARIES m)
check_symbol_exists(cbrt math.h HAVE_CBRT)
set(CMAKE_REQUIRED_LIBRARIES)
check_include_files(dlfcn.h HAVE_DLFCN_H)
if(FOUND_GLIBC)
	set(CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
endif()
check_symbol_exists(getifaddrs ifaddrs.h HAVE_GETIFADDRS)
check_symbol_exists(getpeereid unistd.h HAVE_GETPEEREID)
check_symbol_exists(getpeerucred ucred.h HAVE_GETPEERUCRED)
check_symbol_exists(getresuid unistd.h HAVE_GETRESUID)
check_symbol_exists(getzoneid zone.h HAVE_GETZONEID)
check_symbol_exists(mkostemp stdlib.h HAVE_MKOSTEMP)
check_symbol_exists(issetugid unistd.h HAVE_ISSETUGID)
check_symbol_exists(shmctl64 sys/ipc_impl.h HAVE_SHMCTL64)
check_symbol_exists(strcasestr string.h HAVE_STRCASESTR)
check_symbol_exists(strncasecmp strings.h HAVE_STRNCASECMP)
check_symbol_exists(strndup string.h HAVE_STRNDUP)
check_symbol_exists(timingsafe_memcmp string.h HAVE_TIMINGSAFE_MEMCMP)
check_symbol_exists(asprintf stdio.h HAVE_ASPRINTF)
check_symbol_exists(vasprintf stdio.h HAVE_VASPRINTF)
check_symbol_exists(setitimer sys/time.h HAVE_SETITIMER)
check_symbol_exists(poll poll.h HAVE_POLL)
check_symbol_exists(epoll_create1 sys/epoll.h HAVE_EPOLL_CREATE1)
foreach(typeof typeof __typeof__)
	check_c_source_compiles("int main(void) { int value = 0;  ${typeof}(value) value2 = value;  return value2; }"
		TYPEOF_WORKS)
	if(TYPEOF_WORKS)
		set(HAVE_TYPEOF 1)
		set(TYPEOF ${typeof})
		break()
	endif()
endforeach()
set(CMAKE_REQUIRED_LIBRARIES rt)
check_symbol_exists(clock_gettime time.h HAVE_CLOCK_GETTIME)
if(HAVE_CLOCK_GETTIME)
	set(MONOTONIC_CLOCK_SOURCE "\n
	  #include <time.h>\n
	  int main(void) {\n
	    struct timespec tp;\n
	    return (clock_gettime(CLOCK_MONOTONIC, &tp) != 0);\n
	  }")
	check_c_source_compiles("${MONOTONIC_CLOCK_SOURCE}" HAVE_MONOTONIC_CLOCK)
	if(HAVE_MONOTONIC_CLOCK)
		add_definitions(-DMONOTONIC_CLOCK)
	endif()
endif()
set(CMAKE_REQUIRED_LIBRARIES)
if(FOUND_GLIBC)
	set(CMAKE_REQUIRED_DEFINITIONS)
endif()

configure_file(include/dix-config.h.in include/dix-config.h)

if(APPLE)
	set(DEFAULT_XKB_BASE_DIRECTORY "/opt/X11/share/X11/xkb")
else()
	set(DEFAULT_XKB_BASE_DIRECTORY "/usr/share/X11/xkb")
endif()
set(XKB_BASE_DIRECTORY ${DEFAULT_XKB_BASE_DIRECTORY} CACHE PATH
	"Path to XKB data (default: ${DEFAULT_XKB_BASE_DIRECTORY})")
message(STATUS "XKB_BASE_DIRECTORY = ${XKB_BASE_DIRECTORY}")

if(APPLE)
	set(DEFAULT_XKB_BIN_DIRECTORY /opt/X11/bin)
else()
	set(DEFAULT_XKB_BIN_DIRECTORY "")
endif()
set(XKB_BIN_DIRECTORY ${DEFAULT_XKB_BIN_DIRECTORY} CACHE PATH
	"Directory containing xkbcomp program (default: ${DEFAULT_XKB_BIN_DIRECTORY})")
message(STATUS "XKB_BIN_DIRECTORY = ${XKB_BIN_DIRECTORY}")

if(APPLE)
	set(DEFAULT_XKB_DFLT_RULES "base")
else()
	set(DEFAULT_XKB_DFLT_RULES "evdev")
endif()
set(XKB_DFLT_RULES ${DEFAULT_XKB_DFLT_RULES} CACHE STRING
	"Keyboard ruleset (default: ${DEFAULT_XKB_DFLT_RULES})")
message(STATUS "XKB_DFLT_RULES = ${XKB_DFLT_RULES}")

set(XKB_DFLT_MODEL "pc105" CACHE STRING "Keyboard model (default: pc105)")
message(STATUS "XKB_DFLT_MODEL = ${XKB_DFLT_MODEL}")

set(XKB_DFLT_LAYOUT "us" CACHE STRING "Keyboard layout (default: us)")
message(STATUS "XKB_DFLT_LAYOUT = ${XKB_DFLT_LAYOUT}")

set(XKB_DFLT_VARIANT "" CACHE STRING "Keyboard variant (default: (none))")
message(STATUS "XKB_DFLT_VARIANT = ${XKB_DFLT_VARIANT}")

set(XKB_DFLT_OPTIONS "" CACHE STRING
	"Keyboard layout options (default: (none))")
message(STATUS "XKB_DFLT_OPTIONS = ${XKB_DFLT_OPTIONS}")

configure_file(include/xkb-config.h.in include/xkb-config.h)

add_subdirectory(Xext)
add_subdirectory(Xi)
add_subdirectory(composite)
add_subdirectory(damageext)
add_subdirectory(dix)
add_subdirectory(fb)
if(TVNC_GLX)
	add_subdirectory(glx)
endif()
add_subdirectory(mi)
add_subdirectory(miext)
add_subdirectory(os)
add_subdirectory(present)
add_subdirectory(randr)
add_subdirectory(record)
add_subdirectory(render)
add_subdirectory(xfixes)
add_subdirectory(xkb)
add_subdirectory(hw/vnc)

add_executable(Xvnc dix/stubmain.c)
set(PAM_LIB "")
if(TVNC_USEPAM)
	if(NOT APPLE)
		if(NOT PAM_LIBRARY)
			set(PAM_LIBRARY pam)
		endif()
		set(CMAKE_REQUIRED_LIBRARIES ${PAM_LIBRARY})
		check_function_exists(pam_start HAVE_PAM_START)
		unset(CMAKE_REQUIRED_LIBRARIES)
		if(NOT HAVE_PAM_START)
			message(FATAL_ERROR "Could not find PAM library")
		endif()
	endif()
	set(PAM_LIB ${PAM_LIBRARY})
endif()
set(EXTRA_LIB "")
if(TVNC_NVCONTROL)
	set(EXTRA_LIB ${EXTRA_LIB} XNVCtrl)
endif()
if(TVNC_GLX)
	if(TVNC_SYSTEMX11)
		set(EXTRA_LIB ${EXTRA_LIB} glx-server ${OPENGL_gl_LIBRARY})
	else()
		set(EXTRA_LIB ${EXTRA_LIB} glx-server glx glapi-server xcbglx X11xcb
			${X11_LIBRARIES})
	endif()
endif()
if(HAVE_MONOTONIC_CLOCK)
	set(EXTRA_LIB ${EXTRA_LIB} rt)
endif()
target_link_libraries(Xvnc dix mi vnc fb Xi composite mi damage damageext randr
	record render os present Xext-server sync xfixes xkb ${X11_Xau_LIB}
	${X11_Xdmcp_LIB} ${X11_Xfont2_LDFLAGS} ${X11_Fontenc_LDFLAGS}
	${FREETYPE_LIBRARIES} ${X11_Pixman_LDFLAGS} sha1 ${TJPEG_LIBRARY}
	${ZLIB_LIBRARIES} ${BZIP2_LIBRARIES} vncauth m pthread ${PAM_LIB}
	${EXTRA_LIB})
if(NOT APPLE AND NOT
	CMAKE_SYSTEM_NAME MATCHES "(OpenBSD|FreeBSD|NetBSD|DragonFly)")
	target_link_libraries(Xvnc dl)
endif()

install(TARGETS Xvnc DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/Xserver.man
	DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 RENAME Xserver.1)
configure_file(Xvnc.man.in Xvnc.man @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Xvnc.man
	DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 RENAME Xvnc.1)
