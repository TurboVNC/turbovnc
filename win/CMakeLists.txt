enable_language(CXX)

string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_UC)

if(MSVC)
	# Use the static C library for all build types
	if(NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
		if(CMAKE_BUILD_TYPE_UC STREQUAL "DEBUG")
			set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDebug)
		elseif(MSVC_IDE)
			set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
		else()
			set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded)
		endif()
	endif()
	message(STATUS "CMAKE_MSVC_RUNTIME_LIBRARY = ${CMAKE_MSVC_RUNTIME_LIBRARY}")

	foreach(var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
		CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
		if(${var} MATCHES "/EHsc")
			string(REGEX REPLACE "/EHsc" "/EHs" ${var} "${${var}}")
		endif()
	endforeach()

	# Don't auto-generate manifests
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")

	# Avoid linker warning when doing Debug build if dependent libraries are
	# linked with the Release version of the static C library.
	set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:LIBCMT")

	add_definitions(-D__WIN32__ -U_MBCS -D_WINSOCK_DEPRECATED_NO_WARNINGS)
endif()

set(EFFECTIVE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE_UC}}")
message(STATUS "Compiler flags = ${EFFECTIVE_C_FLAGS}")

set(EFFECTIVE_LD_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS_${CMAKE_BUILD_TYPE_UC}}")
message(STATUS "Linker flags = ${EFFECTIVE_LD_FLAGS}")

add_subdirectory(vncviewer)

install(FILES ../LICENSE.txt TurboVNC.url ../ChangeLog.md
	DESTINATION .)
