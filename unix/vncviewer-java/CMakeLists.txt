include(CheckCSourceCompiles)

configure_file(vncviewer-java.in vncviewer-java @ONLY)
execute_process(COMMAND chmod +x vncviewer-java
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/vncviewer-java
	DESTINATION ${TVNC_BINDIR})

install(FILES vncviewer-java.man DESTINATION ${TVNC_MANDIR}/man1
	RENAME vncviewer-java.1)

if(BITS EQUAL 64)
	set(DEFAULT_TJPEG_JNILIBRARY /opt/libjpeg-turbo/lib64/libturbojpeg.so)
else()
	set(DEFAULT_TJPEG_JNILIBRARY /opt/libjpeg-turbo/lib32/libturbojpeg.so)
endif()

set(TJPEG_JNILIBRARY ${DEFAULT_TJPEG_JNILIBRARY} CACHE FILEPATH
	"Shared library containing TurboJPEG JNI functions (default: ${DEFAULT_TJPEG_JNILIBRARY})")

get_filename_component(TJPEG_JNILIBRARY_RP ${TJPEG_JNILIBRARY} REALPATH)

if(NOT EXISTS ${TJPEG_JNILIBRARY_RP})
	message(FATAL_ERROR "Could not find TurboJPEG JNI library at ${TJPEG_JNILIBRARY}.  Set TJPEG_JNILIBRARY to the full pathname of libturbojpeg.so or install the libjpeg-turbo SDK.")
endif()

set(CMAKE_REQUIRED_LIBRARIES ${TJPEG_JNILIBRARY})
check_c_source_compiles("int main(void) { Java_org_libjpegturbo_turbojpeg_TJ_bufSize(0, 0, 1, 1, 0); return 0; }" TURBOJPEG_JNI_WORKS)
set(CMAKE_REQUIRED_LIBRARIES)
if(NOT TURBOJPEG_JNI_WORKS)
	message(FATAL_ERROR "Could not link with TurboJPEG JNI library ${TJPEG_JNILIBRARY}.  This may be because it was not built with JNI support.")
endif()

message(STATUS "TJPEG_JNILIBRARY = ${TJPEG_JNILIBRARY}")

install(PROGRAMS ${TJPEG_JNILIBRARY_RP} DESTINATION ${TVNC_JAVADIR}
	RENAME libturbojpeg.so)
