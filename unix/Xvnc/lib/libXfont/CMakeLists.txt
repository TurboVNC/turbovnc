include_directories(include stubs ../libfontenc/include)

add_definitions(-DHAVE_CONFIG_H)

if(APPLE)
	add_definitions(-DNO_WEAK_SYMBOLS)
endif()

add_subdirectory(FreeType)
add_subdirectory(bitmap)
add_subdirectory(builtins)
add_subdirectory(fc)
add_subdirectory(fontfile)
add_subdirectory(stubs)
add_subdirectory(util)

add_library(Xfont STATIC
	$<TARGET_OBJECTS:FreeType>
	$<TARGET_OBJECTS:bitmap>
	$<TARGET_OBJECTS:builtins>
	$<TARGET_OBJECTS:fc>
	$<TARGET_OBJECTS:fontfile>
	$<TARGET_OBJECTS:stubs>
	$<TARGET_OBJECTS:util>)
