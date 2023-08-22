if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
	# Use the maximum optimization level for release builds
	foreach(var CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_RELWITHDEBINFO)
		if(${var} MATCHES "-O2")
			string(REGEX REPLACE "-O2" "-O3" ${var} "${${var}}")
		endif()
	endforeach()
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
	if(CPUTYPE STREQUAL "arm")
		set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH};/usr/lib/arm-linux-gnueabihf)
	endif()
endif()

if(NOT APPLE OR TVNC_BUILDSERVER)
	find_package(X11 REQUIRED)
	message(STATUS "X11_X11_LIB = ${X11_X11_LIB}")
endif()

string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_UC)

set(EFFECTIVE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE_UC}}")
message(STATUS "Compiler flags = ${EFFECTIVE_C_FLAGS}")

set(EFFECTIVE_LD_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS_${CMAKE_BUILD_TYPE_UC}}")
message(STATUS "Linker flags = ${EFFECTIVE_LD_FLAGS}")


###############################################################################
# VIEWER BUILD
###############################################################################

if(TVNC_BUILDVIEWER)
	add_subdirectory(vncviewer)
endif()


###############################################################################
# SERVER BUILD
###############################################################################

if(TVNC_BUILDSERVER)

set(DEFAULT_TVNC_BUILDWEBSERVER 1)
option(TVNC_BUILDWEBSERVER "Build web server for noVNC"
	${DEFAULT_TVNC_BUILDWEBSERVER})
boolean_number(TVNC_BUILDWEBSERVER)
boolean_number(TVNC_BUILDWEBSERVER PARENT_SCOPE)
report_option(TVNC_BUILDWEBSERVER "Web server for noVNC")

include_directories(include)

option(TVNC_NVCONTROL
	"Include fake NV-CONTROL extension in Xvnc (causes Xvnc to depend on the system's libX11 and libXext)"
	ON)

set(ELIGIBLE_SYSTEMLIBS "bzip2 and FreeType")
if((CPU_TYPE STREQUAL "x86_64" OR CPU_TYPE STREQUAL "i386") AND
	NOT CMAKE_C_COMPILER_ID MATCHES SunPro)
	set(ELIGIBLE_SYSTEMLIBS "zlib, bzip2, and FreeType")
endif()
option(TVNC_SYSTEMLIBS
	"Build the TurboVNC Server against the system-supplied versions of ${ELIGIBLE_SYSTEMLIBS} rather than the in-tree versions"
	OFF)
boolean_number(TVNC_SYSTEMLIBS)
report_option(TVNC_SYSTEMLIBS "System ${ELIGIBLE_SYSTEMLIBS}")

option(TVNC_STATIC_XORG_PATHS
	"Force vncserver and Xvnc to use the XKB base directory, XKB binary directory, X.org DRI driver path, X.org font path, and X.org registry path specified in the XKB_BASE_DIRECTORY, XKB_BIN_DIRECTORY, XORG_DRI_DRIVER_PATH, XORG_FONT_PATH, and XORG_REGISTRY_PATH CMake variables rather than detecting the appropriate paths at run time"
	OFF)
boolean_number(TVNC_STATIC_XORG_PATHS)
report_option(TVNC_STATIC_XORG_PATHS "Static X.org paths")
if(TVNC_STATIC_XORG_PATHS)
	add_definitions(-DTURBOVNC_STATIC_XORG_PATHS)
endif()

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
if(TVNC_BUILDWEBSERVER)
	if(CMAKE_VERSION VERSION_GREATER 3.12 OR
		CMAKE_VERSION VERSION_EQUAL 3.12)
		find_package(Python 3.0 REQUIRED)
		if(CMAKE_SYSTEM_NAME MATCHES "(OpenBSD|FreeBSD|NetBSD|DragonFly)")
			set(PYTHON_EXECUTABLE ${Python_EXECUTABLE})
		else()
			string(REGEX REPLACE "\\.[0-9]+$" "" PYTHON_EXECUTABLE
				${Python_EXECUTABLE})
			set(PYTHON_EXECUTABLE ${PYTHON_EXECUTABLE} PARENT_SCOPE)
		endif()
		message(STATUS "PYTHON_EXECUTABLE = ${PYTHON_EXECUTABLE}")
		set(PYTHON_VERSION ${Python_VERSION} PARENT_SCOPE)
	else()
		find_package(PythonInterp 2.6 REQUIRED)
		set(PYTHON_VERSION ${PYTHON_VERSION_STRING} PARENT_SCOPE)
	endif()
	configure_file(webserver.in ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/webserver
		@ONLY)
	execute_process(COMMAND chmod +x webserver
		WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
	install(PROGRAMS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/webserver
		DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
configure_file(vncserver.man.in vncserver.man @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/vncserver.man
	DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 RENAME vncserver.1)
configure_file(turbovncserver.conf.in turbovncserver.conf @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/turbovncserver.conf
	DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/turbovncserver-security.conf
	DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/xstartup.turbovnc
	DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
execute_process(COMMAND chmod +x xstartup.turbovnc
	WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/xstartup.turbovnc
	DESTINATION ${CMAKE_INSTALL_BINDIR})

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	option(TVNC_BUILDINITSCRIPT "Build TurboVNC Server init script" 1)
	boolean_number(TVNC_BUILDINITSCRIPT)
	boolean_number(TVNC_BUILDINITSCRIPT PARENT_SCOPE)
	report_option(TVNC_BUILDINITSCRIPT "TurboVNC Server init script")

	if(TVNC_BUILDINITSCRIPT)
		configure_file(vncserver.init.in vncserver.init @ONLY)
		install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/vncserver.init
			DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR}/init.d RENAME tvncserver)
		install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/tvncservers
			DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR}/sysconfig)
	endif()
endif()

endif() # TVNC_BUILDSERVER
