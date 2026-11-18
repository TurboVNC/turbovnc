if(BITS EQUAL 64)
	set(REQARCH amd64)
else()
	set(REQARCH x86)
endif()

configure_file(vncviewer.bat.in ${CMAKE_BINARY_DIR}/vncviewer.bat
	@ONLY)
configure_file(vncviewerw.bat.in ${CMAKE_BINARY_DIR}/vncviewerw.bat
	@ONLY)

install(PROGRAMS ${CMAKE_BINARY_DIR}/vncviewer.bat
	DESTINATION ${CMAKE_INSTALL_BINDIR})
install(PROGRAMS ${CMAKE_BINARY_DIR}/vncviewerw.bat
	DESTINATION ${CMAKE_INSTALL_BINDIR})

find_package(JNI REQUIRED)

add_definitions(-D_WINSTATIC)

include_directories(${JAVA_INCLUDE_PATH} ${JAVA_INCLUDE_PATH2} omnithread
	${TJPEG_INCLUDE_DIR})

add_library(turbovnchelper SHARED turbovnchelper.cpp LowLevelHook.cpp
	omnithread/nt.cpp ${CMAKE_SOURCE_DIR}/common/turbojpeg-jni/turbojpeg-jni.c)

set_target_properties(turbovnchelper PROPERTIES PREFIX ""
	LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/java
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/java)

target_link_libraries(turbovnchelper ${TJPEG_LIBRARY})

install(TARGETS turbovnchelper RUNTIME DESTINATION ${CMAKE_INSTALL_JAVADIR})
