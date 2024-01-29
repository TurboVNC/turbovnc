if(CPU_TYPE STREQUAL "x86_64")
	set(REQARCH amd64)
elseif(CPU_TYPE STREQUAL "arm64")
	set(REQARCH aarch64)
else()
	set(REQARCH x86)
endif()

configure_file(vncviewer.bat.in ${CMAKE_BINARY_DIR}/vncviewer.bat
	@ONLY)
configure_file(vncviewerw.bat.in ${CMAKE_BINARY_DIR}/vncviewerw.bat
	@ONLY)

install(PROGRAMS ${CMAKE_BINARY_DIR}/vncviewer.bat
	DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
install(PROGRAMS ${CMAKE_BINARY_DIR}/vncviewerw.bat
	DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})

find_package(JNI REQUIRED)

add_definitions(-D_WINSTATIC)

include_directories(${JAVA_INCLUDE_PATH} ${JAVA_INCLUDE_PATH2} omnithread)
if(DEFINED TJPEG_INCLUDE_DIR)
	include_directories(${TJPEG_INCLUDE_DIR})
endif()

add_library(turbovnchelper SHARED turbovnchelper.cpp LowLevelHook.cpp
	omnithread/nt.cpp ${CMAKE_SOURCE_DIR}/common/turbojpeg-jni/turbojpeg-jni.c)

set_target_properties(turbovnchelper PROPERTIES PREFIX ""
	LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/java
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/java)

if(DEFINED TJPEG_LIBRARY)
	target_link_libraries(turbovnchelper ${TJPEG_LIBRARY})
elseif(TURBOJPEG_TARGET)
	target_link_libraries(turbovnchelper ${TURBOJPEG_TARGET})
endif()

install(TARGETS turbovnchelper RUNTIME
	DESTINATION ${CMAKE_INSTALL_FULL_JAVADIR})
