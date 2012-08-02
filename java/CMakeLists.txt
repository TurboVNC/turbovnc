cmake_minimum_required(VERSION 2.8)

project(turbovnc-java Java)
set(VERSION 1.1.80)

find_package(Java)

set(DEFAULT_JAVACFLAGS "-source 1.5 -target 1.5 -Xlint:all,-serial,-cast,-unchecked")
set(JAVACFLAGS ${DEFAULT_JAVACFLAGS} CACHE STRING
  "Java compiler flags (Default: ${DEFAULT_JAVACFLAGS})")
message(STATUS "Java compiler flags = ${JAVACFLAGS}")

set(DEFAULT_TVNC_JAVADIR ${CMAKE_INSTALL_PREFIX}/java)
set(TVNC_JAVADIR ${DEFAULT_TVNC_JAVADIR} CACHE PATH
	"Directory into which Java viewer should be installed (default: ${DEFAULT_TVNC_JAVADIR})")
message(STATUS "TVNC_JAVADIR = ${TVNC_JAVADIR}")

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

if(WIN32)
  set(DEFAULT_TURBOJPEG_JAR c:\\libjpeg-turbo\\classes\\turbojpeg.jar)
else()
  set(DEFAULT_TURBOJPEG_JAR /opt/libjpeg-turbo/classes/turbojpeg.jar)
endif()

set(TURBOJPEG_JAR ${DEFAULT_TURBOJPEG_JAR} CACHE PATH
  "JAR file containing TurboJPEG Java classes (default: ${DEFAULT_TURBOJPEG_JAR})")

if(NOT EXISTS ${TURBOJPEG_JAR})
  message(FATAL_ERROR "Could not find TurboJPEG JAR file at ${TURBOJPEG_JAR}.  Set TURBOJPEG_JAR to the full pathname of turbojpeg.jar or install the libjpeg-turbo SDK.")
endif()

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

set(JSCH_CLASSNAMES
  DH
  DHG1
  DHG14
  DHGEX
  JSch
  Session
  UserAuth
  UserAuthKeyboardInteractive
  UserAuthPassword
  UserAuthPublicKey
  UserAuthNone
  jce/AES128CBC
  jce/AES192CTR
  jce/ARCFOUR128
  jce/BlowfishCBC
  jce/HMACMD5
  jce/KeyPairGenDSA
  jce/Random
  jce/SignatureRSA
  jce/AES128CTR
  jce/AES256CBC
  jce/ARCFOUR256
  jce/DH
  jce/HMACSHA196
  jce/KeyPairGenRSA
  jce/SHA1
  jce/TripleDESCBC
  jce/AES192CBC
  jce/AES256CTR
  jce/ARCFOUR
  jce/HMACMD596
  jce/HMACSHA1
  jce/MD5
  jce/SignatureDSA
  jce/TripleDESCTR
  jcraft/Compression
  jcraft/HMAC
  jcraft/HMACMD596
  jcraft/HMACMD5
  jcraft/HMACSHA196
  jcraft/HMACSHA1)
set(JSCH_SRCDIR ${CMAKE_CURRENT_SOURCE_DIR}/com/jcraft/jsch)
foreach(class ${JSCH_CLASSNAMES})
  set(JAVA_SOURCES ${JAVA_SOURCES} ${JSCH_SRCDIR}/${class}.java)
  set(JAVA_CLASSES_FULL ${JAVA_CLASSES_FULL}
    ${CMAKE_CURRENT_BINARY_DIR}/com/jcraft/jsch/${class}.class)
endforeach()

file(GLOB DEPEND_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/com/turbovnc/rfb/*.java
  ${CMAKE_CURRENT_SOURCE_DIR}/com/turbovnc/rdr/*.java
  ${CMAKE_CURRENT_SOURCE_DIR}/com/turbovnc/network/*.java
  ${CMAKE_CURRENT_SOURCE_DIR}/com/jcraft/jsch/*.java
  ${CMAKE_CURRENT_SOURCE_DIR}/com/jcraft/jsch/jcraft/*.java
  ${CMAKE_CURRENT_SOURCE_DIR}/com/jcraft/jsch/jgss/*.java
  ${CMAKE_CURRENT_SOURCE_DIR}/com/jcraft/jzlib/*.java)

string(REGEX REPLACE " " ";" JAVACFLAGS "${JAVACFLAGS}")
add_custom_command(OUTPUT ${JAVA_CLASSES_FULL}
  DEPENDS ${JAVA_SOURCES} ${DEPEND_SOURCES}
  COMMAND ${JAVA_COMPILE}
  ARGS ${JAVACFLAGS} -cp ${TURBOJPEG_JAR}
    -sourcepath ${CMAKE_CURRENT_SOURCE_DIR}
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
  ARGS xf ${TURBOJPEG_JAR}
    org/libjpegturbo/turbojpeg/TJ.class
    org/libjpegturbo/turbojpeg/TJDecompressor.class
    org/libjpegturbo/turbojpeg/TJLoader.class
  COMMAND ${JAVA_ARCHIVE}
  ARGS cfm VncViewer.jar
    ${JAVA_SRCDIR}/MANIFEST.MF
    com/turbovnc/vncviewer/timestamp
    com/turbovnc/vncviewer/*.class
    com/turbovnc/rfb/*.class
    com/turbovnc/rdr/*.class
    com/turbovnc/network/*.class
    com/jcraft/jzlib/*.class
    com/jcraft/jsch/jcraft/*.class
    com/jcraft/jsch/jce/*.class
    com/jcraft/jsch/*.class
    org/libjpegturbo/turbojpeg/*.class
    com/turbovnc/vncviewer/turbovnc.png
    com/turbovnc/vncviewer/turbovnc.ico
  COMMAND ${CMAKE_COMMAND}
  ARGS -DJava_PATH=${Java_PATH}
    -DJAR_FILE=${CMAKE_CURRENT_BINARY_DIR}/VncViewer.jar
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/SignJar.cmake)

add_custom_target(java ALL DEPENDS VncViewer.jar)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/VncViewer.jar
	DESTINATION ${TVNC_JAVADIR})
if(NOT WIN32)
	install(FILES ${JAVA_SRCDIR}/README DESTINATION ${TVNC_JAVADIR})
endif()
if(TVNC_BUILDSERVER)
  install(FILES ${JAVA_SRCDIR}/index.vnc DESTINATION ${TVNC_JAVADIR})
endif()
