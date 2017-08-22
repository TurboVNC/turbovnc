include_directories(include src/stubs ../libfontenc/include)

add_definitions(-DHAVE_CONFIG_H -DHAVE_READLINK)

if(APPLE)
	add_definitions(-DNO_WEAK_SYMBOLS)
endif()

add_subdirectory(src/FreeType)
add_subdirectory(src/bitmap)
add_subdirectory(src/builtins)
add_subdirectory(src/fc)
add_subdirectory(src/fontfile)
add_subdirectory(src/stubs)
add_subdirectory(src/util)

add_library(Xfont STATIC
	$<TARGET_OBJECTS:FreeType>
	$<TARGET_OBJECTS:bitmap>
	$<TARGET_OBJECTS:builtins>
	$<TARGET_OBJECTS:fc>
	$<TARGET_OBJECTS:fontfile>
	$<TARGET_OBJECTS:stubs>
	$<TARGET_OBJECTS:util>)
