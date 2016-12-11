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

if(CMAKE_C_COMPILER_ID MATCHES SunPro)
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

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(CMAKE_LIBRARY_PATH /usr/lib/${CPU_TYPE}-linux-gnu;/usr/lib${BITS};/usr/lib)
endif()

find_package(X11 REQUIRED)

message(STATUS "X11_X11_LIB = ${X11_X11_LIB}")

include_directories(include)

string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_UC)

set(EFFECTIVE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE_UC}}")
message(STATUS "Compiler flags = ${EFFECTIVE_C_FLAGS}")

set(EFFECTIVE_LD_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS_${CMAKE_BUILD_TYPE_UC}}")
message(STATUS "Linker flags = ${EFFECTIVE_LD_FLAGS}")


###############################################################################
# VIEWER BUILD
###############################################################################

if(TVNC_BUILDNATIVE)
	add_subdirectory(vncviewer)
endif()


###############################################################################
# SERVER BUILD
###############################################################################

if(TVNC_BUILDSERVER)

option(TVNC_NVCONTROL
	"Include fake NV-CONTROL extension in Xvnc (causes Xvnc to depend on the system's libX11 and libXext)"
	ON)

add_subdirectory(${CMAKE_SOURCE_DIR}/common ${CMAKE_BINARY_DIR}/common)
add_subdirectory(libvncauth)
if(TVNC_NVCONTROL)
	add_subdirectory(libXNVCtrl)
endif()
add_subdirectory(tvncconfig)
add_subdirectory(vncconnect)
add_subdirectory(vncpasswd)
add_subdirectory(Xvnc)

string(TOLOWER "${TVNC_USETLS}" USETLS)
if(USETLS AND NOT USETLS STREQUAL "none")
	set(USETLS 1)
else()
	set(USETLS 0)
endif()

configure_file(vncserver.in ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/vncserver @ONLY)
execute_process(COMMAND chmod +x vncserver
	WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

install(PROGRAMS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/vncserver
	DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/vncserver.man
	DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 RENAME vncserver.1)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/turbovncserver.conf
	DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/turbovncserver-security.conf
	DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR})

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	configure_file(vncserver.init.in vncserver.init @ONLY)
	install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/vncserver.init
		DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR}/init.d RENAME tvncserver)
	install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/tvncservers
		DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR}/sysconfig)
endif()

endif() # TVNC_BUILDSERVER
