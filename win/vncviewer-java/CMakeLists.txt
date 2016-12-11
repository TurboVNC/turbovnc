if(BITS EQUAL 64)
	set(REQARCH amd64)
else()
	set(REQARCH x86)
endif()

configure_file(vncviewer-java.bat.in ${CMAKE_BINARY_DIR}/vncviewer-java.bat
	@ONLY)
configure_file(vncviewer-javaw.bat.in ${CMAKE_BINARY_DIR}/vncviewer-javaw.bat
	@ONLY)

install(PROGRAMS ${CMAKE_BINARY_DIR}/vncviewer-java.bat
	DESTINATION ${CMAKE_INSTALL_BINDIR})
install(PROGRAMS ${CMAKE_BINARY_DIR}/vncviewer-javaw.bat
	DESTINATION ${CMAKE_INSTALL_BINDIR})

if(BITS EQUAL 64)
	set(DEFAULT_TJPEG_JNILIBRARY "c:\\libjpeg-turbo64\\bin\\turbojpeg.dll")
else()
	set(DEFAULT_TJPEG_JNILIBRARY "c:\\libjpeg-turbo\\bin\\turbojpeg.dll")
endif()

set(TJPEG_JNILIBRARY ${DEFAULT_TJPEG_JNILIBRARY} CACHE FILEPATH
	"Shared library containing TurboJPEG JNI functions (default: ${DEFAULT_TJPEG_JNILIBRARY})")

if(NOT EXISTS ${TJPEG_JNILIBRARY})
	message(FATAL_ERROR "Could not find TurboJPEG JNI library at ${TJPEG_JNILIBRARY}.  Set TJPEG_JNILIBRARY to the full pathname of turbojpeg.dll or install the libjpeg-turbo SDK.")
endif()

file(COPY ${TJPEG_JNILIBRARY} DESTINATION ${CMAKE_BINARY_DIR}/java)

# We have to just assume that the DLL contains the JNI functions, because there
# is no straightforward way to check for that using CMake.

message(STATUS "TJPEG_JNILIBRARY = ${TJPEG_JNILIBRARY}")

install(PROGRAMS ${TJPEG_JNILIBRARY} DESTINATION ${CMAKE_INSTALL_JAVADIR}
	RENAME turbojpeg.dll)

if(TVNC_BUILDNATIVE)

find_package(JNI REQUIRED)

add_definitions(-D_WINSTATIC)

include_directories(${JAVA_INCLUDE_PATH} ${JAVA_INCLUDE_PATH2}
	${CMAKE_SOURCE_DIR}/win/vncviewer/omnithread)

add_library(turbovnchelper SHARED turbovnchelper.cpp LowLevelHook.cpp
	../vncviewer/omnithread/nt.cpp)

set_target_properties(turbovnchelper PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/java
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/java)

target_link_libraries(turbovnchelper)

install(TARGETS turbovnchelper DESTINATION ${CMAKE_INSTALL_JAVADIR})

endif()
