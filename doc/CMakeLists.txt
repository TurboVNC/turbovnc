file(GLOB SOURCE_FILES *.png *.rb [a-z]*.txt turbovnc.css)

file(GLOB DOC_FILES *.png LICENSE*.txt index.html turbovnc.css)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/stamp
	COMMAND deplate -c config.rb -m html-obfuscate-email turbovnc.txt
		-D _DOCVERSION=${DOCVERSION} -D _VERSION=${VERSION} -o index.html
	COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/stamp
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS ${SOURCE_FILES}
	COMMENT "Generating User's Guide"
)

add_custom_target(doc DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/stamp)

install(FILES ${DOC_FILES} DESTINATION ${CMAKE_INSTALL_DOCDIR})
