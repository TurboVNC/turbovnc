cmake_minimum_required(VERSION 2.8)

project(turbovnc-java Java)
set(VERSION 1.1.91)

find_package(Java)

set(DEFAULT_JAVACFLAGS "-source 1.5 -target 1.5 -Xlint:all,-serial,-cast,-unchecked")
set(JAVACFLAGS ${DEFAULT_JAVACFLAGS} CACHE STRING
  "Java compiler flags (Default: ${DEFAULT_JAVACFLAGS})")
message(STATUS "Java compiler flags = ${JAVACFLAGS}")

set(CLASSPATH com/turbovnc/vncviewer)
set(JAVA_SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}/${CLASSPATH})
set(JAVA_BINDIR ${CMAKE_CURRENT_BINARY_DIR}/${CLASSPATH})

if(MSVC) 	 
  if(NOT BUILD)
    execute_process(COMMAND "${CMAKE_SOURCE_DIR}/cmake/getdate.bat"
      OUTPUT_VARIABLE BUILD)    
  endif()
  execute_process(COMMAND "${CMAKE_SOURCE_DIR}/cmake/getdate.bat" "javadate" 	 
    OUTPUT_VARIABLE JAVA_DATE) 	 
  execute_process(COMMAND "${CMAKE_SOURCE_DIR}/cmake/getdate.bat" "javatime" 	 
	  OUTPUT_VARIABLE JAVA_TIME) 	 
else()
  if(NOT BUILD)
    execute_process(COMMAND "date" "+%Y%m%d" OUTPUT_VARIABLE BUILD)
  endif()
  execute_process(COMMAND "date" "+%b %d %Y" OUTPUT_VARIABLE JAVA_DATE)
  execute_process(COMMAND "date" "+%H:%M:%S" OUTPUT_VARIABLE JAVA_TIME)
endif()
string(REGEX REPLACE "\n" "" JAVA_DATE ${JAVA_DATE})
string(REGEX REPLACE "\n" "" JAVA_TIME ${JAVA_TIME})
string(REGEX REPLACE "\n" "" BUILD ${BUILD})

set(JAVA_CLASSNAMES
  CConn
  ClipboardDialog
  DesktopWindow
  Dialog
  F8Menu
  OptionsDialogCallback
  PasswdDialog
  PlatformPixelBuffer
  OptionsDialog
  ServerDialog
  UserPrefs
  VncViewer)

set(JAVA_SOURCES "")
set(JAVA_CLASSES_FULL "")
foreach(class ${JAVA_CLASSNAMES})
  set(JAVA_SOURCES ${JAVA_SOURCES} ${JAVA_SRCDIR}/${class}.java)
  set(JAVA_CLASSES_FULL ${JAVA_CLASSES_FULL}
    ${CMAKE_CURRENT_BINARY_DIR}/${CLASSPATH}/${class}.class)
endforeach()

file(GLOB DEPEND_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/com/turbovnc/rfb/*.java
  ${CMAKE_CURRENT_SOURCE_DIR}/com/turbovnc/rdr/*.java
  ${CMAKE_CURRENT_SOURCE_DIR}/com/turbovnc/network/*.java
  ${CMAKE_CURRENT_SOURCE_DIR}/com/jcraft/jzlib/*.java)

string(REGEX REPLACE " " ";" JAVACFLAGS "${JAVACFLAGS}")
add_custom_command(OUTPUT ${JAVA_CLASSES_FULL}
  DEPENDS ${JAVA_SOURCES} ${DEPEND_SOURCES}
  COMMAND ${JAVA_COMPILE}
  ARGS ${JAVACFLAGS} -cp ${CMAKE_CURRENT_SOURCE_DIR}
    -d ${CMAKE_CURRENT_BINARY_DIR} ${JAVA_SOURCES})

configure_file(${CLASSPATH}/timestamp.in ${CLASSPATH}/timestamp)

add_custom_command(OUTPUT ${JAVA_BINDIR}/turbovnc.png
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${JAVA_SRCDIR}/turbovnc.png ${JAVA_BINDIR}/turbovnc.png
  DEPENDS ${JAVA_SRCDIR}/turbovnc.png)

add_custom_command(OUTPUT ${JAVA_BINDIR}/turbovnc.ico
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${JAVA_SRCDIR}/turbovnc.ico ${JAVA_BINDIR}/turbovnc.ico
  DEPENDS ${JAVA_SRCDIR}/turbovnc.ico)

string(REGEX REPLACE "jar" "" Java_PATH ${Java_JAR_EXECUTABLE})
string(REGEX REPLACE ".exe" "" Java_PATH ${Java_PATH})

add_custom_command(OUTPUT VncViewer.jar
  DEPENDS ${JAVA_CLASSES_FULL}
    ${JAVA_SRCDIR}/MANIFEST.MF
    ${JAVA_BINDIR}/timestamp
    ${JAVA_BINDIR}/turbovnc.png
    ${JAVA_BINDIR}/turbovnc.ico
  COMMAND ${JAVA_ARCHIVE}
  ARGS cfm VncViewer.jar
    ${JAVA_SRCDIR}/MANIFEST.MF
    com/turbovnc/vncviewer/timestamp
    com/turbovnc/vncviewer/*.class
    com/turbovnc/rfb/*.class
    com/turbovnc/rdr/*.class
    com/turbovnc/network/*.class
    com/jcraft/jzlib/*.class
    com/turbovnc/vncviewer/turbovnc.png
    com/turbovnc/vncviewer/turbovnc.ico
  COMMAND ${CMAKE_COMMAND}
  ARGS -DJava_PATH=${Java_PATH}
    -DJAR_FILE=${CMAKE_CURRENT_BINARY_DIR}/VncViewer.jar
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/SignJar.cmake)

add_custom_target(java ALL DEPENDS VncViewer.jar)

if(NOT WIN32)
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/VncViewer.jar
    ${JAVA_SRCDIR}/README
    ${JAVA_SRCDIR}/index.vnc
    ${JAVA_SRCDIR}/favicon.ico
    DESTINATION vnc/classes)
endif()
