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

set(JAVA_KEYSTORE "" CACHE FILEPATH "URL or pathname of the Java keystore (Default: ~/.keystore)")
set(DEFAULT_JAVA_KEYSTORE_TYPE "jks")
set(JAVA_KEYSTORE_TYPE ${DEFAULT_JAVA_KEYSTORE_TYPE} CACHE STRING "Java keystore type (Default: ${DEFAULT_JAVA_KEYSTORE_TYPE})")
set(JAVA_KEYSTORE_PASS "" CACHE STRING "Java keystore password (plain-text.)  Add a prefix of env: to specify an environment variable from which to read the password, or a prefix of file: to specify a file from which to read the password.")
set(JAVA_KEY_ALIAS "" CACHE STRING "Alias for the signing certificate's entry in the Java keystore.  Leave this blank to sign the JAR file using a self-signed certificate.")
set(JAVA_KEY_PASS "" CACHE STRING "Password for the signing certificate's entry in the Java keystore (plain-text.)  Add a prefix of env: to specify an environment variable from which to read the password, or a prefix of file: to specify a file from which to read the password.")
set(JAVA_TSA_URL "" CACHE STRING "URL of Timestamp Authority (TSA)")
set(JAVA_TSA_ALG "" CACHE STRING "Timestamp Authority (TSA) message digest algorithm")

string(TIMESTAMP DEFAULT_JAVA_DATE "%Y-%m-%d")
set(JAVA_DATE ${DEFAULT_JAVA_DATE} CACHE STRING "Java date stamp")
mark_as_advanced(JAVA_DATE)
string(TIMESTAMP DEFAULT_JAVA_TIME "%H:%M:%S")
set(JAVA_TIME ${DEFAULT_JAVA_TIME} CACHE STRING "Java time stamp")
mark_as_advanced(JAVA_TIME)

if(WIN32)
	if(BITS EQUAL 64)
		set(DEFAULT_TJPEG_JAR c:\\libjpeg-turbo64\\classes\\turbojpeg.jar)
	else()
		set(DEFAULT_TJPEG_JAR c:\\libjpeg-turbo\\classes\\turbojpeg.jar)
	endif()
else()
	set(DEFAULT_TJPEG_JAR /opt/libjpeg-turbo/classes/turbojpeg.jar)
endif()

set(TJPEG_JAR ${DEFAULT_TJPEG_JAR} CACHE FILEPATH
	"JAR file containing TurboJPEG Java classes (default: ${DEFAULT_TJPEG_JAR})")

if(NOT EXISTS ${TJPEG_JAR})
	message(FATAL_ERROR "Could not find TurboJPEG JAR file at ${TJPEG_JAR}.  Set TJPEG_JAR to the full pathname of turbojpeg.jar or install the libjpeg-turbo SDK.")
endif()

if(APPLE)
	set(DEFAULT_TJPEG_JNILIBRARY /opt/libjpeg-turbo/lib/libturbojpeg.dylib)
	set(TJPEG_JNILIBRARY ${DEFAULT_TJPEG_JNILIBRARY} CACHE FILEPATH
		"Shared library containing TurboJPEG JNI functions (default: ${DEFAULT_TJPEG_JNILIBRARY})")
endif()

if(UNIX AND TVNC_BUILDSERVER)
	set(DEFAULT_TVNC_INCLUDEJNIJARS 0)
	option(TVNC_INCLUDEJNIJARS "Include TurboJPEG JNI JARs for common platforms when installing/packaging TurboVNC, sign the JARs using the same certificate as VncViewer.jar, and make them available through Java Web Start when using the built-in HTTP server."
		${DEFAULT_TVNC_INCLUDEJNIJARS})
	boolean_number(TVNC_INCLUDEJNIJARS PARENT_SCOPE)
endif()

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
	jce/SignatureECDSA
	jce/SignatureRSA
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
	${SRCDIR}/com/jcraft/jsch/jce/*.java
	${SRCDIR}/com/jcraft/jsch/jcraft/*.java
	${SRCDIR}/com/jcraft/jsch/jgss/*.java
	${SRCDIR}/com/jcraft/jzlib/*.java
	${SRCDIR}/org/spf4j/base/*.java)

string(REGEX REPLACE " " ";" CMAKE_JAVA_COMPILE_FLAGS
	"${CMAKE_JAVA_COMPILE_FLAGS}")
add_custom_command(OUTPUT ${JAVA_CLASSES}
	DEPENDS ${JAVA_SOURCES} ${DEPEND_SOURCES}
	COMMAND ${JAVA_COMPILE}
	ARGS ${CMAKE_JAVA_COMPILE_FLAGS} -cp ${TJPEG_JAR} -sourcepath ${SRCDIR}
		-d ${BINDIR} ${CLASSPATH}/VncViewer.java ${CLASSPATH}/ImageDrawTest.java
		${JAVA_SOURCES}
	WORKING_DIRECTORY ${SRCDIR})

configure_file(${CLASSPATH}/timestamp.in ${CLASSPATH}/timestamp)

if(NOT "${SRCDIR}" STREQUAL "${BINDIR}")

foreach(icon toolbar.png turbovnc.png turbovnc-sm.png turbovnc-128.png)
	add_custom_command(OUTPUT ${BINDIR}/${CLASSPATH}/${icon}
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${SRCDIR}/${CLASSPATH}/${icon} ${BINDIR}/${CLASSPATH}/${icon}
		DEPENDS ${SRCDIR}/${CLASSPATH}/${icon})
endforeach()

endif()

if(TVNC_BUILDSERVER)

set(TVNCJNIOPENCOMMENT "<?comment")
set(TVNCJNICLOSECOMMENT "?>")
if(TVNC_INCLUDEJNIJARS)
	set(TVNCJNIOPENCOMMENT "")
	set(TVNCJNICLOSECOMMENT "")
endif()
configure_file(${SRCDIR}/${CLASSPATH}/VncViewer.jnlp.in
	${BINDIR}/VncViewer.jnlp)

add_custom_target(favicon.ico ALL
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${SRCDIR}/${CLASSPATH}/favicon.ico ${BINDIR}/favicon.ico
	DEPENDS ${SRCDIR}/${CLASSPATH}/favicon.ico)

endif()

string(REGEX REPLACE "jar" "" Java_PATH ${Java_JAR_EXECUTABLE})
string(REGEX REPLACE ".exe" "" Java_PATH ${Java_PATH})

if(NOT JAVA_KEY_ALIAS)
	set(KEYTOOL "${Java_PATH}/keytool")
	add_custom_command(OUTPUT turbovnc.keystore
		COMMAND ${KEYTOOL}
		ARGS -genkey -alias TurboVNC -keystore turbovnc.keystore -keyalg RSA
			-storepass turbovnc -keypass turbovnc -validity 7300
			-dname "CN=TurboVNC, OU=Software Development, O=The VirtualGL Project, L=Austin, S=Texas, C=US")
	set(JAVA_KEYSTORE "turbovnc.keystore")
	set(JAVA_KEYSTORE_TYPE "jks")
	set(JAVA_KEYSTORE_PASS "turbovnc")
	set(JAVA_KEY_ALIAS "TurboVNC")
	set(JAVA_KEY_PASS "turbovnc")
	set(JAVA_TSA_URL "")
	set(JAVA_TSA_ALG "")
	add_custom_target(keystore DEPENDS turbovnc.keystore)
	set(JAVA_KEYSTORE_DEPENDS keystore)
endif()

add_custom_command(OUTPUT VncViewer.jar
	DEPENDS ${JAVA_CLASSES}
		${SRCDIR}/${CLASSPATH}/MANIFEST.MF
		${BINDIR}/${CLASSPATH}/timestamp
		${BINDIR}/${CLASSPATH}/toolbar.png
		${BINDIR}/${CLASSPATH}/turbovnc.png
		${BINDIR}/${CLASSPATH}/turbovnc-sm.png
		${BINDIR}/${CLASSPATH}/turbovnc-128.png
		${TJPEG_JAR}
		${JAVA_KEYSTORE_DEPENDS}
	COMMAND ${JAVA_ARCHIVE}
	ARGS xf ${TJPEG_JAR}
		org/libjpegturbo/turbojpeg
	COMMAND ${JAVA_ARCHIVE}
	ARGS cfm VncViewer-unsigned.jar
		${SRCDIR}/${CLASSPATH}/MANIFEST.MF
		${CLASSPATH}/timestamp
		${CLASSPATH}/*.class
		com/turbovnc/rfb/*.class
		com/turbovnc/rdr/*.class
		com/turbovnc/network/*.class
		com/jcraft/jzlib/*.class
		com/jcraft/jsch/jgss/*.class
		com/jcraft/jsch/jcraft/*.class
		com/jcraft/jsch/jce/*.class
		com/jcraft/jsch/*.class
		org/spf4j/base/*.class
		org/libjpegturbo/turbojpeg/*.class
		${CLASSPATH}/toolbar.png
		${CLASSPATH}/turbovnc.png
		${CLASSPATH}/turbovnc-sm.png
		${CLASSPATH}/turbovnc-128.png
	COMMAND ${CMAKE_COMMAND}
	ARGS -DJava_PATH=${Java_PATH} -DJAR_FILE=${BINDIR}/VncViewer-unsigned.jar
		-DJAVA_KEYSTORE=${JAVA_KEYSTORE}
		-DJAVA_KEYSTORE_TYPE=${JAVA_KEYSTORE_TYPE}
		-DJAVA_KEYSTORE_PASS=${JAVA_KEYSTORE_PASS}
		-DJAVA_KEY_ALIAS=${JAVA_KEY_ALIAS}
		-DJAVA_KEY_PASS=${JAVA_KEY_PASS}
		-DJAVA_TSA_URL=${JAVA_TSA_URL}
		-DJAVA_TSA_ALG=${JAVA_TSA_ALG}
		-P ${SRCDIR}/cmake/SignJar.cmake
	COMMAND ${CMAKE_COMMAND}
	ARGS -E copy_if_different VncViewer-unsigned.jar VncViewer.jar
	COMMAND ${CMAKE_COMMAND}
	ARGS -E remove VncViewer-unsigned.jar)

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

if(TVNC_BUILDSERVER AND TVNC_INCLUDEJNIJARS)
	set(DEFAULT_TJPEG_JNIJARPATH /opt/libjpeg-turbo-jni)
	set(TJPEG_JNIJARPATH ${DEFAULT_TJPEG_JNIJARPATH} CACHE PATH
		"Directory containing TurboJPEG JNI JARs (default: ${DEFAULT_TJPEG_JNIJARPATH})")
	set(JNI_JAR_FILES ljtlinux32.jar ljtlinux64.jar ljtosx.jar ljtwin32.jar
		ljtwin64.jar)
	foreach(jarfile ${JNI_JAR_FILES})
		add_custom_command(OUTPUT ${jarfile}
			DEPENDS ${JAVA_KEYSTORE_DEPENDS}
			COMMAND ${CMAKE_COMMAND}
			ARGS -E copy_if_different ${TJPEG_JNIJARPATH}/${jarfile} ${jarfile}
			COMMAND ${CMAKE_COMMAND}
			ARGS -DJava_PATH=${Java_PATH} -DJAR_FILE=${BINDIR}/${jarfile}
				-DJAVA_KEYSTORE=${JAVA_KEYSTORE}
				-DJAVA_KEYSTORE_TYPE=${JAVA_KEYSTORE_TYPE}
				-DJAVA_KEYSTORE_PASS=${JAVA_KEYSTORE_PASS}
				-DJAVA_KEY_ALIAS=${JAVA_KEY_ALIAS}
				-DJAVA_KEY_PASS=${JAVA_KEY_PASS}
				-DJAVA_TSA_URL=${JAVA_TSA_URL}
				-DJAVA_TSA_ALG=${JAVA_TSA_ALG}
				-P ${SRCDIR}/cmake/SignJar.cmake)
		install(FILES ${BINDIR}/${jarfile} DESTINATION ${CMAKE_INSTALL_JAVADIR})
	endforeach()
	add_custom_target(jnijars ALL DEPENDS ${JNI_JAR_FILES})
endif()

install(FILES ${BINDIR}/VncViewer.jar DESTINATION ${CMAKE_INSTALL_JAVADIR})
install(FILES ${SRCDIR}/${CLASSPATH}/README.md
	DESTINATION ${CMAKE_INSTALL_JAVADIR})
if(TVNC_BUILDSERVER)
	install(FILES ${BINDIR}/VncViewer.jnlp DESTINATION ${CMAKE_INSTALL_JAVADIR})
	install(FILES ${SRCDIR}/${CLASSPATH}/favicon.ico
		DESTINATION ${CMAKE_INSTALL_JAVADIR})
endif()
