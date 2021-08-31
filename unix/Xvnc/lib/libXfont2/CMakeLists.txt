set(HEADERS
	include/X11/fonts/libxfont2.h)

foreach(h ${HEADERS})
	copy_X_header(${h} X11/fonts)
endforeach()

include_directories(include src/stubs ../libfontenc/include .)

check_include_files(stdint.h HAVE_STDINT_H)
if(HAVE_STDINT_H)
	add_definitions(-DHAVE_STDINT_H)
endif()

add_definitions(-DHAVE_CONFIG_H -DHAVE_READLINK -UUSE_POLL)

disable_compiler_warnings()

add_subdirectory(src/FreeType)
add_subdirectory(src/bitmap)
add_subdirectory(src/builtins)
add_subdirectory(src/fc)
add_subdirectory(src/fontfile)
add_subdirectory(src/stubs)
add_subdirectory(src/util)

add_library(Xfont2 STATIC
	$<TARGET_OBJECTS:FreeType>
	$<TARGET_OBJECTS:bitmap>
	$<TARGET_OBJECTS:builtins>
	$<TARGET_OBJECTS:fc>
	$<TARGET_OBJECTS:fontfile>
	$<TARGET_OBJECTS:stubs>
	$<TARGET_OBJECTS:util>)
