find_package(Java REQUIRED)

# Allow the default Java compiler flags to be set using an environment variable
if(NOT DEFINED CMAKE_JAVA_COMPILE_FLAGS AND DEFINED ENV{JAVAFLAGS})
	set(DEFAULT_CMAKE_JAVA_COMPILE_FLAGS $ENV{JAVAFLAGS})
else()
	set(DEFAULT_CMAKE_JAVA_COMPILE_FLAGS
		"-Xlint:all,-serial,-cast,-unchecked,-rawtypes")
endif()
set(CMAKE_JAVA_COMPILE_FLAGS ${DEFAULT_CMAKE_JAVA_COMPILE_FLAGS} CACHE STRING
	"Java compiler flags (Default: ${DEFAULT_CMAKE_JAVA_COMPILE_FLAGS})")
set(CMAKE_JAVA_COMPILE_FLAGS "${CMAKE_JAVA_COMPILE_FLAGS} -J-Dfile.encoding=UTF8")
message(STATUS "CMAKE_JAVA_COMPILE_FLAGS = ${CMAKE_JAVA_COMPILE_FLAGS}")

set(CLASSPATH com/turbovnc/vncviewer)
set(SRCDIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BINDIR ${CMAKE_CURRENT_BINARY_DIR})

string(TIMESTAMP DEFAULT_JAVA_DATE "%Y-%m-%d")
set(JAVA_DATE ${DEFAULT_JAVA_DATE} CACHE STRING "Java date stamp")
mark_as_advanced(JAVA_DATE)
string(TIMESTAMP DEFAULT_JAVA_TIME "%H:%M:%S")
set(JAVA_TIME ${DEFAULT_JAVA_TIME} CACHE STRING "Java time stamp")
mark_as_advanced(JAVA_TIME)

set(JAVA_SOURCES "")
set(JAVA_CLASSES "")

# We have to explicitly specify some of the JSch classes because it dynamically
# loads them.

set(JSCH_CLASSNAMES
	CipherNone
	DHEC256
	DHEC384
	DHEC521
	DHG1
	DHG14
	DHGEX
	DHGEX256
	UserAuthGSSAPIWithMIC
	UserAuthKeyboardInteractive
	UserAuthNone
	UserAuthPassword
	UserAuthPublicKey
	jce/AES128CBC
	jce/AES128CTR
	jce/AES192CBC
	jce/AES192CTR
	jce/AES256CBC
	jce/AES256CTR
	jce/ARCFOUR
	jce/ARCFOUR128
	jce/ARCFOUR256
	jce/BlowfishCBC
	jce/DH
	jce/ECDH256
	jce/ECDH384
	jce/ECDH521
	jce/ECDHN
	jce/HMACMD5
	jce/HMACMD596
	jce/HMACSHA1
	jce/HMACSHA196
	jce/HMACSHA256
	jce/HMACSHA512
	jce/KeyPairGenDSA
	jce/KeyPairGenECDSA
	jce/KeyPairGenRSA
	jce/MD5
	jce/PBKDF
	jce/Random
	jce/SHA1
	jce/SHA256
	jce/SHA384
	jce/SHA512
	jce/SignatureDSA
	jce/SignatureECDSA256
	jce/SignatureECDSA384
	jce/SignatureECDSA521
	jce/SignatureECDSAN
	jce/SignatureRSA
	jce/SignatureRSAN
	jce/SignatureRSASHA256
	jce/SignatureRSASHA512
	jce/TripleDESCBC
	jce/TripleDESCTR
	jcraft/Compression
	jcraft/HMACMD5
	jcraft/HMACMD596
	jcraft/HMACSHA1
	jcraft/HMACSHA196
	jgss/GSSContextKrb5)

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
	${SRCDIR}/com/jcraft/jsch/agentproxy/*.java
	${SRCDIR}/com/jcraft/jsch/agentproxy/connector/*.java
	${SRCDIR}/com/jcraft/jsch/agentproxy/usocket/*.java
	${SRCDIR}/com/jcraft/jsch/jce/*.java
	${SRCDIR}/com/jcraft/jsch/jcraft/*.java
	${SRCDIR}/com/jcraft/jsch/jgss/*.java
	${SRCDIR}/com/jcraft/jzlib/*.java
	${SRCDIR}/org/spf4j/base/*.java
	${SRCDIR}/org/mindrot/jbcrypt/*.java)

string(REGEX REPLACE " " ";" CMAKE_JAVA_COMPILE_FLAGS
	"${CMAKE_JAVA_COMPILE_FLAGS}")
add_custom_command(OUTPUT ${JAVA_CLASSES}
	DEPENDS ${JAVA_SOURCES} ${DEPEND_SOURCES}
	COMMAND ${JAVA_COMPILE}
	ARGS ${CMAKE_JAVA_COMPILE_FLAGS} -sourcepath ${SRCDIR} -d ${BINDIR}
		${CLASSPATH}/VncViewer.java ${CLASSPATH}/ImageDrawTest.java ${JAVA_SOURCES}
	WORKING_DIRECTORY ${SRCDIR})

configure_file(${CLASSPATH}/timestamp.in ${CLASSPATH}/timestamp)

if(NOT "${SRCDIR}" STREQUAL "${BINDIR}")

foreach(icon toolbar.png turbovnc.png turbovnc-sm.png turbovnc-128.png
	insecure.png secure.png)
	add_custom_command(OUTPUT ${BINDIR}/${CLASSPATH}/${icon}
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${SRCDIR}/${CLASSPATH}/${icon} ${BINDIR}/${CLASSPATH}/${icon}
		DEPENDS ${SRCDIR}/${CLASSPATH}/${icon})
endforeach()

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
		${BINDIR}/${CLASSPATH}/turbovnc-128.png
		${BINDIR}/${CLASSPATH}/insecure.png
		${BINDIR}/${CLASSPATH}/secure.png
	COMMAND ${JAVA_ARCHIVE}
	ARGS cfm VncViewer.jar
		${SRCDIR}/${CLASSPATH}/MANIFEST.MF
		${CLASSPATH}/timestamp
		${CLASSPATH}/*.class
		com/turbovnc/rfb/*.class
		com/turbovnc/rdr/*.class
		com/turbovnc/network/*.class
		com/jcraft/jzlib/*.class
		com/jcraft/jsch/agentproxy/*.class
		com/jcraft/jsch/agentproxy/connector/*.class
		com/jcraft/jsch/agentproxy/usocket/*.class
		com/jcraft/jsch/jgss/*.class
		com/jcraft/jsch/jcraft/*.class
		com/jcraft/jsch/jce/*.class
		com/jcraft/jsch/*.class
		org/spf4j/base/*.class
		org/mindrot/jbcrypt/*.class
		${CLASSPATH}/toolbar.png
		${CLASSPATH}/turbovnc.png
		${CLASSPATH}/turbovnc-sm.png
		${CLASSPATH}/turbovnc-128.png
		${CLASSPATH}/insecure.png
		${CLASSPATH}/secure.png)

add_custom_target(java ALL DEPENDS VncViewer.jar)

if(CMAKE_INSTALL_PREFIX STREQUAL "${CMAKE_INSTALL_DEFAULT_PREFIX}" OR WIN32)
	set(CMAKE_INSTALL_DEFAULT_JAVADIR "<CMAKE_INSTALL_DATAROOTDIR>/java")
else()
	set(CMAKE_INSTALL_DEFAULT_JAVADIR "<CMAKE_INSTALL_DATAROOTDIR>/${CMAKE_PROJECT_NAME_LC}/classes")
endif()
GNUInstallDirs_set_install_dir(JAVADIR
	"The directory into which Java classes should be installed")
GNUInstallDirs_get_absolute_install_dir(CMAKE_INSTALL_FULL_JAVADIR
	CMAKE_INSTALL_JAVADIR)
set(CMAKE_INSTALL_JAVADIR ${CMAKE_INSTALL_JAVADIR} PARENT_SCOPE)
set(CMAKE_INSTALL_FULL_JAVADIR ${CMAKE_INSTALL_FULL_JAVADIR} PARENT_SCOPE)
mark_as_advanced(CLEAR CMAKE_INSTALL_JAVADIR)
report_directory(JAVADIR)

option(TVNC_INCLUDEJRE "Include a custom Java Runtime Environment (JRE) with the TurboVNC Viewer"
	FALSE)
boolean_number(TVNC_INCLUDEJRE)
boolean_number(TVNC_INCLUDEJRE PARENT_SCOPE)
report_option(TVNC_INCLUDEJRE "Custom JRE")

if(TVNC_INCLUDEJRE)
	execute_process(COMMAND ${Java_JAVA_EXECUTABLE} -version
		RESULT_VARIABLE RESULT OUTPUT_VARIABLE OUTPUT ERROR_VARIABLE OUTPUT
		OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE)
	if(NOT RESULT EQUAL 0)
		message(FATAL_ERROR "Could not determine Java version:\n${OUTPUT}")
	endif()
	string(TOLOWER ${OUTPUT} OUTPUT_LC)
	if(NOT OUTPUT_LC MATCHES ".*openjdk.*" OR Java_VERSION VERSION_LESS 11)
		message(FATAL_ERROR "OpenJDK 11 or later required with TVNC_INCLUDEJRE=1")
	endif()
	configure_file(cmake/BuildJRE.cmake.in cmake/BuildJRE.cmake @ONLY)

	add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/jrestamp
		${CMAKE_CURRENT_BINARY_DIR}/jre
		COMMAND ${CMAKE_COMMAND} -DJRE_OUTPUT_DIR=java/jre
			-P ${CMAKE_CURRENT_BINARY_DIR}/cmake/BuildJRE.cmake
		COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/jrestamp
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
		DEPENDS VncViewer.jar
		COMMENT "Generating Custom Java Runtime Environment (JRE)")
	add_custom_target(jrebuild ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/jrestamp java)
endif()

install(FILES ${BINDIR}/VncViewer.jar DESTINATION ${CMAKE_INSTALL_JAVADIR})
install(FILES ${SRCDIR}/${CLASSPATH}/README.md
	DESTINATION ${CMAKE_INSTALL_JAVADIR})
if(TVNC_INCLUDEJRE)
	install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/jre
		DESTINATION ${CMAKE_INSTALL_JAVADIR} USE_SOURCE_PERMISSIONS)
endif()
