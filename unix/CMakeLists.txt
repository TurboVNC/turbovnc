set(GCC34 0)
if(CMAKE_COMPILER_IS_GNUCC)
	# Use the maximum optimization level for release builds
	foreach(var CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_RELWITHDEBINFO)
		if(${var} MATCHES "-O2")
			string(REGEX REPLACE "-O2" "-O3" ${var} "${${var}}")
		endif()
	endforeach()
	execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
		OUTPUT_VARIABLE GCC_VERSION)

	# NOTE: Setting GCC34=1 enables MMX code in Xorg.  We can't enable it on
	# Solaris because the version of GCC that ships with Solaris 10 doesn't put
	# mmintrin.h in a sane place.
	if((GCC_VERSION VERSION_GREATER 3.4 OR GCC_VERSION VERSION_EQUAL 3.4)
		AND NOT CMAKE_SYSTEM_NAME STREQUAL "SunOS")
		set(GCC34 1)
	endif()
endif()

if("${CMAKE_C_COMPILER_ID}" MATCHES SunPro)
	# Use the maximum optimization level for release builds
	foreach(var CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_RELWITHDEBINFO)
		if(${var} MATCHES "-xO3")
			string(REGEX REPLACE "-xO3" "-xO5" ${var} "${${var}}")
		endif()
		if(${var} MATCHES "-xO2")
			string(REGEX REPLACE "-xO2" "-xO5" ${var} "${${var}}")
		endif()
	endforeach()
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	set(CMAKE_LIBRARY_PATH /usr/lib/${CPU_TYPE}-linux-gnu;/usr/lib${BITS};/usr/lib)
endif()

find_package(X11 REQUIRED)

macro(lib_check libname)
	if(NOT X11_${libname}_LIB)
		message(FATAL_ERROR "lib${libname} not found.  Try setting X11_${libname}_LIB to the location of lib${libname}.so.")
	endif()
endmacro()

lib_check(Xt)
lib_check(Xcursor)
if(NOT X11_Xaw_LIB)
	find_library(X11_Xaw_LIB Xaw7 "")
	if(NOT X11_Xaw_LIB)
		find_library(X11_Xaw_LIB Xaw "")
		lib_check(Xaw)
	endif()
endif()
if(NOT X11_Xmu_LIB)
	find_library(X11_Xmu_LIB Xmu "")
	lib_check(Xmu)
endif()

message(STATUS "X11_X11_LIB = ${X11_X11_LIB}")
message(STATUS "X11_Xmu_LIB = ${X11_Xmu_LIB}")
message(STATUS "X11_Xt_LIB = ${X11_Xt_LIB}")
message(STATUS "X11_Xaw_LIB = ${X11_Xaw_LIB}")
message(STATUS "X11_Xcursor_LIB = ${X11_Xcursor_LIB}")

# MIT-SHM should be available on all modern Un*x O/S's
add_definitions("-DMITSHM")

set(DEFAULT_TVNC_MANDIR ${CMAKE_INSTALL_PREFIX}/man)
set(TVNC_MANDIR ${DEFAULT_TVNC_MANDIR} CACHE PATH
	"Directory into which man pages should be installed (default: ${DEFAULT_TVNC_MANDIR})")
message(STATUS "TVNC_MANDIR = ${TVNC_MANDIR}")

include_directories(include ${CMAKE_SOURCE_DIR}/common/zlib)


#
# Viewer build
#

add_subdirectory(${CMAKE_SOURCE_DIR}/common ${CMAKE_BINARY_DIR}/common)
add_subdirectory(libvncauth)
add_subdirectory(vncviewer)


#
# Server build
#

if(TVNC_BUILDSERVER)

add_subdirectory(tvncconfig)
add_subdirectory(vncconnect)
add_subdirectory(vncpasswd)

string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_UC)

set(EFFECTIVE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE_UC}}")

message(STATUS "Compiler flags = ${EFFECTIVE_C_FLAGS}")
message(STATUS "Linker flags = ${CMAKE_EXE_LINKER_FLAGS}")

set(DEFAULT_TVNC_CONFDIR ${CMAKE_INSTALL_PREFIX}/etc)
set(TVNC_CONFDIR ${DEFAULT_TVNC_CONFDIR} CACHE PATH
	"Directory into which configuration files should be installed (default: ${DEFAULT_TVNC_CONFDIR})")
message(STATUS "TVNC_CONFDIR = ${TVNC_CONFDIR}")

configure_file(vncserver.in vncserver @ONLY)
execute_process(COMMAND chmod +x vncserver
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
configure_file(Xvnc/config/cf/tvnc_version.h.in Xvnc/config/cf/tvnc_version.h)
configure_file(Xvnc-configure.in Xvnc-configure)
configure_file(vncserver.init.in vncserver.init @ONLY)

if(NOT "${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
	add_custom_target(xserver-syncsource
		COMMAND rsync -avC ${CMAKE_CURRENT_SOURCE_DIR}/Xvnc
			${CMAKE_CURRENT_BINARY_DIR}/)
else()
	add_custom_target(xserver-syncsource)
endif()

add_custom_target(xserver
	COMMAND ${CMAKE_COMMAND} -E remove -f config/imake/imake config/imake/*.o
		config/makedepend/makedepend config/makedepend/*.o config/util/lndir
		config/util/*.o
	COMMAND sh ${CMAKE_CURRENT_BINARY_DIR}/Xvnc-configure
	COMMAND ${CMAKE_BUILD_TOOL} CC=${CMAKE_C_COMPILER}
		CDEBUGFLAGS=${EFFECTIVE_C_FLAGS}
		EXTRA_LDOPTIONS=${CMAKE_EXE_LINKER_FLAGS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Xvnc
	DEPENDS vncauth zlib xserver-syncsource
	SOURCES Xvnc-configure Xvnc/config/cf/tvnc_version.h)

add_custom_target(xserver-rebuild
	COMMAND ${CMAKE_BUILD_TOOL} CC=${CMAKE_C_COMPILER}
		CDEBUGFLAGS=${EFFECTIVE_C_FLAGS}
		EXTRA_LDOPTIONS=${CMAKE_EXE_LINKER_FLAGS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Xvnc
	DEPENDS vncauth zlib xserver-syncsource)

add_custom_target(xserver-clean
	COMMAND ${CMAKE_BUILD_TOOL} CC=${CMAKE_C_COMPILER}
		CDEBUGFLAGS=${EFFECTIVE_C_FLAGS}
		EXTRA_LDOPTIONS=${CMAKE_EXE_LINKER_FLAGS} clean
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Xvnc
	DEPENDS xserver-syncsource)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/vncserver
	DESTINATION ${TVNC_BINDIR})
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/Xvnc/programs/Xserver/Xvnc
	DESTINATION ${TVNC_BINDIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/vncserver.man
	DESTINATION ${TVNC_MANDIR}/man1 RENAME vncserver.1)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/Xvnc/programs/Xserver/Xvnc.man
	DESTINATION ${TVNC_MANDIR}/man1 RENAME Xvnc.1)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/Xvnc/programs/Xserver/Xserver.man
	DESTINATION ${TVNC_MANDIR}/man1 RENAME Xserver.1)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/turbovncserver.conf
	DESTINATION ${TVNC_CONFDIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/turbovncserver-auth.conf
	DESTINATION ${TVNC_CONFDIR})
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/vncserver.init
	DESTINATION ${TVNC_CONFDIR}/init.d RENAME tvncserver)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/tvncservers
	DESTINATION ${TVNC_CONFDIR}/sysconfig)

endif() # TVNC_BUILDSERVER
