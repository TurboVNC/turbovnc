if(DEFINED TJPEG_INCLUDE_DIR OR DEFINED TJPEG_LIBRARY)

	unset(TURBOJPEG_VERSION CACHE)
	unset(TURBOJPEG_EXACT CACHE)
	unset(TURBOJPEG_PATH CACHE)
	unset(TURBOJPEG_STATIC CACHE)
	unset(libjpeg-turbo_DIR CACHE)

	if(TJPEG_INCLUDE_DIR)
		set(TJPEG_INCLUDE_DIR_HINTS ${TJPEG_INCLUDE_DIR})
		unset(TJPEG_INCLUDE_DIR)
		unset(TJPEG_INCLUDE_DIR CACHE)
	endif()

	find_path(TJPEG_INCLUDE_DIR turbojpeg.h DOC "TurboJPEG include directory"
		HINTS ${TJPEG_INCLUDE_DIR_HINTS})
	if(TJPEG_INCLUDE_DIR STREQUAL "TJPEG_INCLUDE_DIR-NOTFOUND")
		unset(TURBOJPEG_WORKS CACHE)
		set(TJPEG_INCLUDE_DIR ${TJPEG_INCLUDE_DIR_HINTS} CACHE PATH
			"TurboJPEG include directory" FORCE)
		message(FATAL_ERROR "Could not find turbojpeg.h in ${TJPEG_INCLUDE_DIR_HINTS}.  If it is installed in a different place, then set TJPEG_INCLUDE_DIR accordingly.")
	else()
		message(STATUS "TJPEG_INCLUDE_DIR = ${TJPEG_INCLUDE_DIR}")
	endif()

	set(TJPEG_LIBRARY "" CACHE STRING
		"Path to TurboJPEG library or flags necessary to link with it")

	include(CheckCSourceCompiles)
	if(MSVC)
		set(CMAKE_REQUIRED_DEFINITIONS -MT)
	endif()
	set(CMAKE_REQUIRED_INCLUDES ${TJPEG_INCLUDE_DIR})
	set(CMAKE_REQUIRED_LIBRARIES ${TJPEG_LIBRARY})
	check_c_source_compiles("#include <turbojpeg.h>\n#include <stddef.h>\nint main(void) { tjhandle h = tjInitCompress();  return h != NULL ? 0 : 1; }" TURBOJPEG_WORKS)
	unset(CMAKE_REQUIRED_LIBRARIES)
	unset(CMAKE_REQUIRED_INCLUDES)
	if(MSVC)
		unset(CMAKE_REQUIRED_DEFINITIONS)
	endif()
	if(NOT TURBOJPEG_WORKS)
		unset(TURBOJPEG_WORKS CACHE)
		if(NOT TJPEG_LIBRARY)
			unset(TJPEG_LIBRARY CACHE)
		endif()
		message(FATAL_ERROR "Could not link with TurboJPEG library ${TJPEG_LIBRARY}.  If it is installed in a different place, then set TJPEG_LIBRARY accordingly.")
	endif()

	message(STATUS "TJPEG_LIBRARY = ${TJPEG_LIBRARY}")

	return()

endif()

unset(TURBOJPEG_WORKS CACHE)

# Use the CMake package config file in libjpeg-turbo v2.1 or later to find the
# TurboJPEG API library and header.

set(TURBOJPEG_VERSION "1.1.90" CACHE STRING
	"Minimum version of libjpeg-turbo containing the desired TurboJPEG API functionality")
option(TURBOJPEG_EXACT
	"Require the exact version of libjpeg-turbo specified in TURBOJPEG_VERSION"
	FALSE)
boolean_number(TURBOJPEG_EXACT)
set(TURBOJPEG_PATH "" CACHE PATH
	"Search only this path for libjpeg-turbo")
option(TURBOJPEG_STATIC
	"Link with the static TurboJPEG API library rather than the shared TurboJPEG API library"
	FALSE)
boolean_number(TURBOJPEG_STATIC)

set(EXACT "")
if(TURBOJPEG_EXACT AND TURBOJPEG_VERSION)
	set(EXACT EXACT)
endif()

if(TURBOJPEG_PATH)
	find_package(libjpeg-turbo ${TURBOJPEG_VERSION} ${EXACT} REQUIRED
		PATHS ${TURBOJPEG_PATH} NO_DEFAULT_PATH)
else()
	find_package(libjpeg-turbo ${TURBOJPEG_VERSION} ${EXACT} REQUIRED)
endif()

unset(EXACT)

set(TURBOJPEG_TARGET libjpeg-turbo::turbojpeg)
if(TURBOJPEG_STATIC)
	set(TURBOJPEG_TARGET libjpeg-turbo::turbojpeg-static)
endif()
if(NOT TARGET ${TURBOJPEG_TARGET})
	message(FATAL_ERROR "Target ${TURBOJPEG_TARGET} does not exist in libjpeg-turbo package")
endif()
foreach(buildtype RELEASE RELWITHDEBINFO NONE MINSIZEREL DEBUG)
	get_target_property(TURBOJPEG_LIB ${TURBOJPEG_TARGET}
		IMPORTED_LOCATION_${buildtype})
	if(NOT TURBOJPEG_LIB STREQUAL "TURBOJPEG_LIB-NOTFOUND")
		break()
	endif()
endforeach()
if(TURBOJPEG_LIB STREQUAL "TURBOJPEG_LIB-NOTFOUND")
	set(TURBOJPEG_LIB "Could not determine library location")
endif()
message(STATUS "Found TurboJPEG: ${TURBOJPEG_LIB} (found version \"${libjpeg-turbo_VERSION}\")")
unset(TURBOJPEG_LIB)
