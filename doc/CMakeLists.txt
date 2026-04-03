file(GLOB SOURCE_FILES *.png *.rb [a-z]*.txt turbovnc.css)

file(GLOB DOC_FILES *.png LICENSE*.txt index.html turbovnc.css
	viewer_parameters.txt viewer_advanced_parameters.txt )

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/stamp
	COMMAND deplate -c config.rb -m html-obfuscate-email turbovnc.txt
		-D _DOCVERSION=${DOCVERSION} -D _VERSION=${VERSION} -o index.html
	COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/stamp
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS ${SOURCE_FILES}
	COMMENT "Generating User's Guide"
)

add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/viewer_parameters.txt
	COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/vncviewer -?
		>viewer_parameters.txt || true
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS java
	COMMENT "Generating TurboVNC Viewer parameter list"
)

add_custom_command(OUTPUT
	${CMAKE_CURRENT_SOURCE_DIR}/viewer_advanced_parameters.txt
	COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/vncviewer -??
		>viewer_advanced_parameters.txt || true
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS java
	COMMENT "Generating TurboVNC Viewer advanced parameter list"
)

add_custom_target(doc DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/stamp
	${CMAKE_CURRENT_SOURCE_DIR}/viewer_parameters.txt
	${CMAKE_CURRENT_SOURCE_DIR}/viewer_advanced_parameters.txt)

install(FILES ${DOC_FILES} DESTINATION ${CMAKE_INSTALL_FULL_DOCDIR})
