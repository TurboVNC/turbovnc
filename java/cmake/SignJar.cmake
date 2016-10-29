if(NOT Java_PATH)
  message(FATAL_ERROR "Java_PATH must be defined")
endif()

if(NOT JAR_FILE)
  message(FATAL_ERROR "JAR_FILE must be defined")
endif()

set(KEYTOOL "${Java_PATH}/keytool")
set(JARSIGNER "${Java_PATH}/jarsigner")

if(JAVA_KEY_ALIAS)
  if((NOT JAVA_KEYSTORE_PASS) OR (NOT JAVA_KEY_PASS))
    message(FATAL_ERROR "When JAVA_KEY_ALIAS is specified, JAVA_KEYSTORE_PASS and JAVA_KEY_PASS must also be specified.")
  endif()
  if(JAVA_KEYSTORE)
    message(STATUS "Signing ${JAR_FILE} using key ${JAVA_KEY_ALIAS} in keystore ${JAVA_KEYSTORE}")
  else()
    message(STATUS "Signing ${JAR_FILE} using key ${JAVA_KEY_ALIAS} in default keystore")
  endif()
  set(JAVA_SELF_SIGNED 0)
else()
  message(STATUS "Signing ${JAR_FILE} using self-signed certificate")
  file(REMOVE ${JAR_FILE}.keystore)
  execute_process(COMMAND
    ${KEYTOOL} -genkey -alias TurboVNC -keystore ${JAR_FILE}.keystore
      -keyalg RSA -storepass turbovnc -keypass turbovnc -validity 7300
      -dname "CN=TurboVNC, OU=Software Development, O=The VirtualGL Project, L=Austin, S=Texas, C=US"
    RESULT_VARIABLE RESULT OUTPUT_VARIABLE OUTPUT ERROR_VARIABLE ERROR)
  if(NOT RESULT EQUAL 0)
    message(FATAL_ERROR "${KEYTOOL} failed:\n${ERROR}")
  endif()
  set(JAVA_KEYSTORE "${JAR_FILE}.keystore")
  set(JAVA_KEYSTORE_PASS "turbovnc")
  set(JAVA_KEY_PASS "turbovnc")
  set(JAVA_KEY_ALIAS "TurboVNC")
  set(JAVA_SELF_SIGNED 1)
endif()

if(JAVA_KEYSTORE)
  set(ARGS -keystore ${JAVA_KEYSTORE})
endif()
set(ARGS ${ARGS} -storetype ${JAVA_KEYSTORE_TYPE})

if(${JAVA_KEYSTORE_PASS} MATCHES "^env:")
  string(REGEX REPLACE "^env:(.*)$" "\\1" JAVA_KEYSTORE_PASS
    "${JAVA_KEYSTORE_PASS}")
  set(ARGS ${ARGS} -storepass:env ${JAVA_KEYSTORE_PASS})
elseif("${JAVA_KEYSTORE_PASS}" MATCHES "^file:")
  string(REGEX REPLACE "^file:(.*)$" "\\1" JAVA_KEYSTORE_PASS
    "${JAVA_KEYSTORE_PASS}")
  set(ARGS ${ARGS} -storepass:file ${JAVA_KEYSTORE_PASS})
else()
  set(ARGS ${ARGS} -storepass ${JAVA_KEYSTORE_PASS})
endif()

if(${JAVA_KEY_PASS} MATCHES "^env:")
  string(REGEX REPLACE "^env:(.*)$" "\\1" JAVA_KEY_PASS
    "${JAVA_KEY_PASS}")
  set(ARGS ${ARGS} -keypass:env ${JAVA_KEY_PASS})
elseif("${JAVA_KEY_PASS}" MATCHES "^file:")
  string(REGEX REPLACE "^file:(.*)$" "\\1" JAVA_KEY_PASS
    "${JAVA_KEY_PASS}")
  set(ARGS ${ARGS} -keypass:file ${JAVA_KEY_PASS})
else()
  set(ARGS ${ARGS} -keypass ${JAVA_KEY_PASS})
endif()

if(NOT JAVA_SELF_SIGNED AND JAVA_TSA_URL)
  message(STATUS "Using timestamp authority ${JAVA_TSA_URL}")
  set(ARGS ${ARGS} -tsa ${JAVA_TSA_URL})
endif()

execute_process(COMMAND ${JARSIGNER} ${ARGS} ${JAR_FILE} ${JAVA_KEY_ALIAS}
  RESULT_VARIABLE RESULT OUTPUT_VARIABLE OUTPUT ERROR_VARIABLE ERROR)

if(NOT RESULT EQUAL 0)
  if(OUTPUT)
    set(ERROR_MESSAGE ${OUTPUT})
  endif()
  if(ERROR)
    set(ERROR_MESSAGE "${ERROR_MESSAGE}\n${ERROR}")
  endif()
  message(FATAL_ERROR "${JARSIGNER} failed:\n${ERROR_MESSAGE}")
endif()

if(NOT JAVA_SELF_SIGNED)
  execute_process(COMMAND ${JARSIGNER} -verify ${JAR_FILE}
    OUTPUT_VARIABLE OUTPUT)
  message(STATUS ${OUTPUT})
  execute_process(COMMAND ${JARSIGNER} -verify -verbose -certs ${JAR_FILE}
    OUTPUT_VARIABLE OUTPUT)
  string(REGEX MATCH "\\[entry was signed on" OUTPUT ${OUTPUT})
  if(OUTPUT)
    message(STATUS "JAR timestamp verified.")
  else()
    message(WARNING "JAR timestamp verification failed.")
  endif()
endif()

if(EXISTS ${JAR_FILE}.keystore)
  file(REMOVE ${JAR_FILE}.keystore)
endif()
