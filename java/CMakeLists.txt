cmake_minimum_required(VERSION 2.8)

if(UNIX)
	set(CMAKE_INSTALL_PREFIX /opt/TurboVNC CACHE PATH
		"Install path prefix, prepended onto install directories.")
endif()

project(turbovnc-java Java)
set(VERSION 1.2.1)
set(COPYRIGHT_YEAR "2000-2013")
set(COPYRIGHT "The VirtualGL Project and many others (see README.txt)")
set(URLTEXT "Visit http://www.virtualgl.org for more information on TurboVNC")

find_package(Java)

set(DEFAULT_JAVACFLAGS "-source 1.5 -target 1.5 -Xlint:all,-serial,-cast,-unchecked,-rawtypes")
set(JAVACFLAGS ${DEFAULT_JAVACFLAGS} CACHE STRING
	"Java compiler flags (Default: ${DEFAULT_JAVACFLAGS})")
message(STATUS "Java compiler flags = ${JAVACFLAGS}")

set(DEFAULT_TVNC_JAVADIR ${CMAKE_INSTALL_PREFIX}/java)
set(TVNC_JAVADIR ${DEFAULT_TVNC_JAVADIR} CACHE PATH
	"Directory into which Java viewer should be installed (default: ${DEFAULT_TVNC_JAVADIR})")
message(STATUS "TVNC_JAVADIR = ${TVNC_JAVADIR}")

set(CLASSPATH com/turbovnc/vncviewer)
set(SRCDIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BINDIR ${CMAKE_CURRENT_BINARY_DIR})

if(MSVC) 	 
	if(NOT BUILD)
		execute_process(COMMAND "${SRCDIR}/cmake/getdate.bat"
			OUTPUT_VARIABLE BUILD)
	endif()
	execute_process(COMMAND "${SRCDIR}/cmake/getdate.bat" "javadate" 	 
		OUTPUT_VARIABLE JAVA_DATE) 	 
	execute_process(COMMAND "${SRCDIR}/cmake/getdate.bat" "javatime" 	 
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
	set(DEFAULT_TJPEG_JAR c:\\libjpeg-turbo\\classes\\turbojpeg.jar)
else()
	set(DEFAULT_TJPEG_JAR /opt/libjpeg-turbo/classes/turbojpeg.jar)
endif()

set(TJPEG_JAR ${DEFAULT_TJPEG_JAR} CACHE PATH
	"JAR file containing TurboJPEG Java classes (default: ${DEFAULT_TJPEG_JAR})")

if(NOT EXISTS ${TJPEG_JAR})
	message(FATAL_ERROR "Could not find TurboJPEG JAR file at ${TJPEG_JAR}.  Set TJPEG_JAR to the full pathname of turbojpeg.jar or install the libjpeg-turbo SDK.")
endif()

if(APPLE)
	set(DEFAULT_TJPEG_JNILIBRARY /opt/libjpeg-turbo/lib/libturbojpeg.dylib)
	set(TJPEG_JNILIBRARY ${DEFAULT_TJPEG_JNILIBRARY} CACHE PATH
		"Shared library containing TurboJPEG JNI functions (default: ${DEFAULT_TJPEG_JNILIBRARY})")
endif()

set(JAVA_SOURCES "")
set(JAVA_CLASSES "")

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

foreach(class ${JSCH_CLASSNAMES})
	set(JAVA_SOURCES ${JAVA_SOURCES} com/jcraft/jsch/${class}.java)
	set(JAVA_CLASSES ${JAVA_CLASSES}
		${BINDIR}/com/jcraft/jsch/${class}.class)
endforeach()

file(GLOB DEPEND_SOURCES
	${SRCDIR}/${CLASSPATH}/*.java
	${SRCDIR}/com/turbovnc/rfb/*.java
	${SRCDIR}/com/turbovnc/rdr/*.java
	${SRCDIR}/com/turbovnc/network/*.java
	${SRCDIR}/com/jcraft/jsch/*.java
	${SRCDIR}/com/jcraft/jsch/jce/*.java
	${SRCDIR}/com/jcraft/jsch/jcraft/*.java
	${SRCDIR}/com/jcraft/jsch/jgss/*.java
	${SRCDIR}/com/jcraft/jzlib/*.java)

string(REGEX REPLACE " " ";" JAVACFLAGS "${JAVACFLAGS}")
add_custom_command(OUTPUT ${JAVA_CLASSES}
	DEPENDS ${JAVA_SOURCES} ${DEPEND_SOURCES}
	COMMAND ${JAVA_COMPILE}
	ARGS ${JAVACFLAGS} -cp ${TJPEG_JAR} -sourcepath ${SRCDIR} -d ${BINDIR}
		${CLASSPATH}/VncViewer.java ${JAVA_SOURCES}
	COMMAND ${JAVA_COMPILE}
	ARGS ${JAVACFLAGS} -cp ${TJPEG_JAR} -sourcepath ${SRCDIR} -d ${BINDIR}
		${CLASSPATH}/ImageDrawTest.java ${JAVA_SOURCES}
	WORKING_DIRECTORY ${SRCDIR})

configure_file(${CLASSPATH}/timestamp.in ${CLASSPATH}/timestamp)

if(NOT "${SRCDIR}" STREQUAL "${BINDIR}")

add_custom_command(OUTPUT ${BINDIR}/${CLASSPATH}/toolbar.png
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${SRCDIR}/${CLASSPATH}/toolbar.png ${BINDIR}/${CLASSPATH}/toolbar.png
	DEPENDS ${SRCDIR}/${CLASSPATH}/toolbar.png)

add_custom_command(OUTPUT ${BINDIR}/${CLASSPATH}/turbovnc.png
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${SRCDIR}/${CLASSPATH}/turbovnc.png ${BINDIR}/${CLASSPATH}/turbovnc.png
	DEPENDS ${SRCDIR}/${CLASSPATH}/turbovnc.png)

add_custom_command(OUTPUT ${BINDIR}/${CLASSPATH}/turbovnc-sm.png
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${SRCDIR}/${CLASSPATH}/turbovnc-sm.png
		${BINDIR}/${CLASSPATH}/turbovnc-sm.png
	DEPENDS ${SRCDIR}/${CLASSPATH}/turbovnc-sm.png)

endif()

string(REGEX REPLACE "jar" "" Java_PATH ${Java_JAR_EXECUTABLE})
string(REGEX REPLACE ".exe" "" Java_PATH ${Java_PATH})

add_custom_command(OUTPUT VncViewer.jar
	DEPENDS ${JAVA_CLASSES}
		${SRCDIR}/${CLASSPATH}/MANIFEST.MF
		${BINDIR}/${CLASSPATH}/timestamp
		${BINDIR}/${CLASSPATH}/toolbar.png
		${BINDIR}/${CLASSPATH}/turbovnc.png
		${BINDIR}/${CLASSPATH}/turbovnc-sm.png
	COMMAND ${JAVA_ARCHIVE}
	ARGS xf ${TJPEG_JAR}
		org/libjpegturbo/turbojpeg/TJ.class
		org/libjpegturbo/turbojpeg/TJDecompressor.class
		org/libjpegturbo/turbojpeg/TJLoader.class
	COMMAND ${JAVA_ARCHIVE}
	ARGS cfm VncViewer.jar
		${SRCDIR}/${CLASSPATH}/MANIFEST.MF
		${CLASSPATH}/timestamp
		${CLASSPATH}/*.class
		com/turbovnc/rfb/*.class
		com/turbovnc/rdr/*.class
		com/turbovnc/network/*.class
		com/jcraft/jzlib/*.class
		com/jcraft/jsch/jcraft/*.class
		com/jcraft/jsch/jce/*.class
		com/jcraft/jsch/*.class
		org/libjpegturbo/turbojpeg/*.class
		${CLASSPATH}/toolbar.png
		${CLASSPATH}/turbovnc.png
		${CLASSPATH}/turbovnc-sm.png
	COMMAND ${CMAKE_COMMAND}
	ARGS -DJava_PATH=${Java_PATH} -DJAR_FILE=${BINDIR}/VncViewer.jar
		-P ${SRCDIR}/cmake/SignJar.cmake)

add_custom_target(java ALL DEPENDS VncViewer.jar)

install(FILES ${BINDIR}/VncViewer.jar DESTINATION ${TVNC_JAVADIR})
install(FILES ${SRCDIR}/${CLASSPATH}/README.txt DESTINATION ${TVNC_JAVADIR})
if(TVNC_BUILDSERVER)
	install(FILES ${SRCDIR}/${CLASSPATH}/index.vnc DESTINATION ${TVNC_JAVADIR})
	install(FILES ${SRCDIR}/${CLASSPATH}/favicon.ico DESTINATION ${TVNC_JAVADIR})
endif()
