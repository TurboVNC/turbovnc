include_directories(include)

disable_compiler_warnings()

if(APPLE)
	set(DEFAULT_FONT_ENCODINGS_DIRECTORY "/opt/X11/share/fonts/encodings")
else()
	set(DEFAULT_FONT_ENCODINGS_DIRECTORY "/usr/share/X11/fonts/encodings")
endif()
set(FONT_ENCODINGS_DIRECTORY ${DEFAULT_FONT_ENCODINGS_DIRECTORY} CACHE PATH
	"Font encodings directory (default: ${DEFAULT_FONT_ENCODINGS_DIRECTORY})")
message(STATUS "FONT_ENCODINGS_DIRECTORY = ${FONT_ENCODINGS_DIRECTORY}")
add_definitions(-DFONT_ENCODINGS_DIRECTORY=\"${FONT_ENCODINGS_DIRECTORY}\")

add_library(fontenc STATIC
	src/encparse.c
	src/fontenc.c)
