find_package(Java REQUIRED)
include(UseJava)

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

string(TIMESTAMP DEFAULT_JAVA_DATE "%Y-%m-%d")
set(JAVA_DATE ${DEFAULT_JAVA_DATE} CACHE STRING "Java date stamp")
mark_as_advanced(JAVA_DATE)
string(TIMESTAMP DEFAULT_JAVA_TIME "%H:%M:%S")
set(JAVA_TIME ${DEFAULT_JAVA_TIME} CACHE STRING "Java time stamp")
mark_as_advanced(JAVA_TIME)

# We have to explicitly specify some of the JSch classes because it dynamically
# loads them.

set(JSCH_SOURCES
	com/jcraft/jsch/CipherNone.java
	com/jcraft/jsch/DHEC256.java
	com/jcraft/jsch/DHEC384.java
	com/jcraft/jsch/DHEC521.java
	com/jcraft/jsch/DHG1.java
	com/jcraft/jsch/DHG14.java
	com/jcraft/jsch/DHGEX.java
	com/jcraft/jsch/DHGEX256.java
	com/jcraft/jsch/UserAuthGSSAPIWithMIC.java
	com/jcraft/jsch/UserAuthKeyboardInteractive.java
	com/jcraft/jsch/UserAuthNone.java
	com/jcraft/jsch/UserAuthPassword.java
	com/jcraft/jsch/UserAuthPublicKey.java
	com/jcraft/jsch/jce/AES128CBC.java
	com/jcraft/jsch/jce/AES128CTR.java
	com/jcraft/jsch/jce/AES192CBC.java
	com/jcraft/jsch/jce/AES192CTR.java
	com/jcraft/jsch/jce/AES256CBC.java
	com/jcraft/jsch/jce/AES256CTR.java
	com/jcraft/jsch/jce/ARCFOUR.java
	com/jcraft/jsch/jce/ARCFOUR128.java
	com/jcraft/jsch/jce/ARCFOUR256.java
	com/jcraft/jsch/jce/BlowfishCBC.java
	com/jcraft/jsch/jce/DH.java
	com/jcraft/jsch/jce/ECDH256.java
	com/jcraft/jsch/jce/ECDH384.java
	com/jcraft/jsch/jce/ECDH521.java
	com/jcraft/jsch/jce/ECDHN.java
	com/jcraft/jsch/jce/HMACMD5.java
	com/jcraft/jsch/jce/HMACMD596.java
	com/jcraft/jsch/jce/HMACSHA1.java
	com/jcraft/jsch/jce/HMACSHA196.java
	com/jcraft/jsch/jce/HMACSHA256.java
	com/jcraft/jsch/jce/HMACSHA512.java
	com/jcraft/jsch/jce/KeyPairGenDSA.java
	com/jcraft/jsch/jce/KeyPairGenECDSA.java
	com/jcraft/jsch/jce/KeyPairGenRSA.java
	com/jcraft/jsch/jce/MD5.java
	com/jcraft/jsch/jce/PBKDF.java
	com/jcraft/jsch/jce/Random.java
	com/jcraft/jsch/jce/SHA1.java
	com/jcraft/jsch/jce/SHA256.java
	com/jcraft/jsch/jce/SHA384.java
	com/jcraft/jsch/jce/SHA512.java
	com/jcraft/jsch/jce/SignatureDSA.java
	com/jcraft/jsch/jce/SignatureECDSA256.java
	com/jcraft/jsch/jce/SignatureECDSA384.java
	com/jcraft/jsch/jce/SignatureECDSA521.java
	com/jcraft/jsch/jce/SignatureECDSAN.java
	com/jcraft/jsch/jce/SignatureRSA.java
	com/jcraft/jsch/jce/SignatureRSAN.java
	com/jcraft/jsch/jce/SignatureRSASHA256.java
	com/jcraft/jsch/jce/SignatureRSASHA512.java
	com/jcraft/jsch/jce/TripleDESCBC.java
	com/jcraft/jsch/jce/TripleDESCTR.java
	com/jcraft/jsch/jcraft/Compression.java
	com/jcraft/jsch/jcraft/HMACMD5.java
	com/jcraft/jsch/jcraft/HMACMD596.java
	com/jcraft/jsch/jcraft/HMACSHA1.java
	com/jcraft/jsch/jcraft/HMACSHA196.java
	com/jcraft/jsch/jgss/GSSContextKrb5.java)

string(REGEX REPLACE " " ";" CMAKE_JAVA_COMPILE_FLAGS
	"${CMAKE_JAVA_COMPILE_FLAGS}")

configure_file(${CLASSPATH}/timestamp.in ${CLASSPATH}/timestamp)

cmake_minimum_required(VERSION 3.21)
add_jar(VncViewer
	${CLASSPATH}/VncViewer.java
	${CLASSPATH}/ImageDrawTest.java
	${JSCH_SOURCES}
	MANIFEST ${CLASSPATH}/MANIFEST.MF
	RESOURCES
	NAMESPACE ${CLASSPATH} ${CMAKE_BINARY_DIR}/java/${CLASSPATH}/timestamp
	NAMESPACE ${CLASSPATH} ${CLASSPATH}/toolbar.png
	NAMESPACE ${CLASSPATH} ${CLASSPATH}/turbovnc.png
	NAMESPACE ${CLASSPATH} ${CLASSPATH}/turbovnc-sm.png
	NAMESPACE ${CLASSPATH} ${CLASSPATH}/turbovnc-128.png
	NAMESPACE ${CLASSPATH} ${CLASSPATH}/insecure.png
	NAMESPACE ${CLASSPATH} ${CLASSPATH}/secure.png)

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
		DEPENDS VncViewer
		COMMENT "Generating Custom Java Runtime Environment (JRE)")
	add_custom_target(jrebuild ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/jrestamp
		VncViewer)
endif()

install_jar(VncViewer DESTINATION ${CMAKE_INSTALL_JAVADIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${CLASSPATH}/README.md
	DESTINATION ${CMAKE_INSTALL_JAVADIR})
if(TVNC_INCLUDEJRE)
	install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/jre
		DESTINATION ${CMAKE_INSTALL_JAVADIR} USE_SOURCE_PERMISSIONS)
endif()
