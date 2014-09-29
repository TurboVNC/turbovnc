if(BITS EQUAL 64)
	set(REQARCH amd64)
else()
	set(REQARCH x86)
endif()

configure_file(vncviewer-java.bat.in vncviewer-java.bat @ONLY)
configure_file(vncviewer-javaw.bat.in vncviewer-javaw.bat @ONLY)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/vncviewer-java.bat
	DESTINATION ${TVNC_BINDIR})
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/vncviewer-javaw.bat
	DESTINATION ${TVNC_BINDIR})

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

file(COPY ${TJPEG_JNILIBRARY} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# We have to just assume that the DLL contains the JNI functions, because there
# is no straightforward way to check for that using CMake.

message(STATUS "TJPEG_JNILIBRARY = ${TJPEG_JNILIBRARY}")

install(PROGRAMS ${TJPEG_JNILIBRARY} DESTINATION ${TVNC_JAVADIR}
	RENAME turbojpeg.dll)
