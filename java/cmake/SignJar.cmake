if(NOT Java_PATH)
  message(FATAL_ERROR "Java_PATH must be defined")
endif()

if(NOT JAR_FILE)
  message(FATAL_ERROR "JAR_FILE must be defined")
endif()

message(STATUS "Signing ${JAR_FILE}")

set(KEYTOOL "${Java_PATH}/keytool")
set(JARSIGNER "${Java_PATH}/jarsigner")

file(REMOVE turbovnc.keystore)
execute_process(COMMAND
  ${KEYTOOL} -genkey -alias TurboVNC -keystore turbovnc.keystore -keyalg RSA
    -storepass turbovnc -keypass turbovnc -validity 7300
    -dname "CN=TurboVNC, OU=Software Development, O=The VirtualGL Project, L=Austin, S=Texas, C=US"
  RESULT_VARIABLE RESULT OUTPUT_VARIABLE OUTPUT ERROR_VARIABLE ERROR)
if(NOT RESULT EQUAL 0)
  message(FATAL_ERROR "${KEYTOOL} failed:\n${ERROR}")
endif()
execute_process(COMMAND
  ${JARSIGNER} -keystore turbovnc.keystore
    -storepass turbovnc -keypass turbovnc ${JAR_FILE} TurboVNC
  RESULT_VARIABLE RESULT OUTPUT_VARIABLE OUTPUT ERROR_VARIABLE ERROR)
if(NOT RESULT EQUAL 0)
  message(FATAL_ERROR "${JARSIGNER} failed:\n${ERROR}")
endif()
file(REMOVE turbovnc.keystore)
