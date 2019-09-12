if(APPLE)

find_package(JNI REQUIRED)

configure_file(vncviewer-mac.in ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/vncviewer
	@ONLY)
execute_process(COMMAND chmod +x vncviewer
	WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

include_directories(${JAVA_INCLUDE_PATH} ${JAVA_INCLUDE_PATH2}
	${TJPEG_INCLUDE_DIR})

set(HELPER_SOURCES cocoa.m turbovnchelper.m JRSwizzle.m)
add_library(turbovnchelper SHARED ${HELPER_SOURCES} keysym2ucs.c osx_to_qnum.c
	${CMAKE_SOURCE_DIR}/common/turbojpeg-jni/turbojpeg-jni.c usocket.c)
set_source_files_properties(${HELPER_SOURCES} PROPERTIES LANGUAGE C)
set_target_properties(turbovnchelper PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/java)
target_link_libraries(turbovnchelper "-framework Carbon -framework Cocoa"
	${TJPEG_LIBRARY})

if(CPU_TYPE STREQUAL "x86_64")
	# This is nasty, but for reasons unexplained, full-screen multi-screen
	# spanning doesn't work with the Mac TurboVNC Viewer app unless
	# JavaAppLauncher is built with an older OS X SDK.  Thus, we include a binary
	# of it in the source tree, and it can only be rebuilt by building the
	# "JavaAppLauncher" target.
	set(MACOS_PLATFORMDIR /Applications/Xcode46.app/Contents/Developer)
	add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/JavaAppLauncher
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/JavaAppLauncher.m
		COMMAND ${MACOS_PLATFORMDIR}/usr/bin/gcc
			-I${JAVA_INCLUDE_PATH} -I${JAVA_INCLUDE_PATH2}
			-isysroot ${MACOS_PLATFORMDIR}/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk
			-mmacosx-version-min=10.7 -framework Cocoa
			${CMAKE_CURRENT_SOURCE_DIR}/JavaAppLauncher.m
			-o ${CMAKE_CURRENT_SOURCE_DIR}/JavaAppLauncher)
	add_custom_target(JavaAppLauncher
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/JavaAppLauncher)
else()
	add_executable(JavaAppLauncher JavaAppLauncher.m)
	target_link_libraries(JavaAppLauncher "-framework Cocoa")
endif()

install(TARGETS turbovnchelper DESTINATION ${CMAKE_INSTALL_JAVADIR})

else() # APPLE

include_directories(${X11_INCLUDE_DIR})

configure_file(vncviewer.in ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/vncviewer @ONLY)
execute_process(COMMAND chmod +x vncviewer
	WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

find_package(JNI REQUIRED)

include_directories(${JAVA_INCLUDE_PATH} ${JAVA_INCLUDE_PATH2}
	${CMAKE_SOURCE_DIR}/common/rfb ${TJPEG_INCLUDE_DIR})
add_library(turbovnchelper SHARED turbovnchelper.c usocket.c
	${CMAKE_SOURCE_DIR}/common/turbojpeg-jni/turbojpeg-jni.c)
if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
	set_target_properties(turbovnchelper PROPERTIES LINK_FLAGS "-lc -z defs")
else()
	set_target_properties(turbovnchelper PROPERTIES LINK_FLAGS "-z defs")
endif()
set_target_properties(turbovnchelper PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/java)
if(NOT CMAKE_SYSTEM_NAME MATCHES "(OpenBSD|FreeBSD|NetBSD|DragonFly)")
	set(LIBDL dl)
endif()
target_link_libraries(turbovnchelper ${X11_LIBRARIES} ${X11_Xi_LIB} ${LIBDL}
	${TJPEG_LIBRARY})

install(TARGETS turbovnchelper DESTINATION ${CMAKE_INSTALL_JAVADIR})

install(FILES ${CMAKE_SOURCE_DIR}/java/com/turbovnc/vncviewer/turbovnc-128.png
	DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/icons/hicolor/128x128/apps
	RENAME turbovnc.png)
install(FILES ${CMAKE_SOURCE_DIR}/java/com/turbovnc/vncviewer/turbovnc.png
	DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/icons/hicolor/48x48/apps)
install(FILES ${CMAKE_SOURCE_DIR}/java/com/turbovnc/vncviewer/turbovnc-sm.png
	DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/icons/hicolor/16x16/apps
	RENAME turbovnc.png)

endif() # APPLE

install(PROGRAMS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/vncviewer
	DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES vncviewer.man DESTINATION ${CMAKE_INSTALL_MANDIR}/man1
	RENAME vncviewer.1)
