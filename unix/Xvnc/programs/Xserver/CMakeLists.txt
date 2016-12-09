include_directories(include Xext ../../lib/pixman
	${CMAKE_CURRENT_BINARY_DIR}/include)

# Enable extensions
add_definitions(-DCOMPOSITE -DDAMAGE -DDPMSExtension -DMITSHM -DRANDR -DRENDER
	-DRES -DSCREENSAVER -DSHAPE -DXACE -DXCSECURITY -DXFIXES -DXF86BIGFONT
	-DXINPUT -DXTEST -DXV)

if(TVNC_NVCONTROL)
add_definitions(-DNVCONTROL)
endif()

add_definitions(-DHAVE_DIX_CONFIG_H -DCLIENTIDS -DHAS_SHM -DPIXPRIV)

if(BITS EQUAL 64)
	add_definitions(-D_XSERVER64)
endif()

set(VENDOR_NAME "The X.Org Foundation")
set(VENDOR_RELEASE "11204000")
configure_file(include/version-config.h.in include/version-config.h)

include(CheckLibraryExists)
check_library_exists(m cbrt "" HAVE_CBRT)
include(CheckIncludeFiles)
check_include_files(dlfcn.h HAVE_DLFCN_H)
check_include_files(ieeefp.h HAVE_IEEEFP_H)
include(CheckFunctionExists)
check_function_exists(ffs HAVE_FFS)
check_function_exists(getdtablesize HAVE_GETDTABLESIZE)
check_function_exists(getifaddrs HAVE_GETIFADDRS)
check_function_exists(getpeereid HAVE_GETPEEREID)
check_function_exists(getpeerucred HAVE_GETPEERUCRED)
check_function_exists(getzoneid HAVE_GETZONEID)
check_function_exists(shmctl64 HAVE_SHMCTL64)
check_function_exists(strcasecmp HAVE_STRCASECMP)
check_function_exists(strcasestr HAVE_STRCASESTR)
check_function_exists(strncasecmp HAVE_STRNCASECMP)
check_function_exists(strlcat HAVE_STRLCAT)
check_function_exists(strlcpy HAVE_STRLCPY)
check_function_exists(strndup HAVE_STRNDUP)
check_function_exists(asprintf HAVE_ASPRINTF)
check_function_exists(vasprintf HAVE_VASPRINTF)

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
	set(DEFAULT_XKB_DFLT_RULES "xorg")
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
add_subdirectory(mi)
add_subdirectory(miext)
add_subdirectory(os)
add_subdirectory(randr)
add_subdirectory(render)
add_subdirectory(xfixes)
add_subdirectory(xkb)
add_subdirectory(hw/vnc)

add_executable(Xvnc dummy.c)
set(PAM_LIB "")
if(TVNC_USEPAM)
	set(PAM_LIB pam)
endif()
set(EXTRA_LIB "")
if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
	set(EXTRA_LIB "-lsocket -lnsl")
endif()
if(TVNC_NVCONTROL)
	set(EXTRA_LIB ${EXTRA_LIB} XNVCtrl)
endif()
target_link_libraries(Xvnc main dix vnc fb Xi composite mi damage damageext
	randr render os Xext-server sync xfixes xkb Xau Xdmcp Xfont fontenc freetype2
	pixman sha1 ${TJPEG_LIBRARY} zlib bzip2 vncauth m pthread ${PAM_LIB}
	${EXTRA_LIB})

install(TARGETS Xvnc DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/Xserver.man
	DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 RENAME Xserver.1)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/Xvnc.man
	DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 RENAME Xvnc.1)
