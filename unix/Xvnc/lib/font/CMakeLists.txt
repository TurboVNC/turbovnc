include_directories(include ../../include/fonts)

add_subdirectory(bitmap)
add_subdirectory(fontfile)
add_subdirectory(fc)
add_subdirectory(Speedo)
add_subdirectory(Type1)
add_subdirectory(FreeType)
add_subdirectory(util)

add_library(Xfont STATIC
	$<TARGET_OBJECTS:bitmap>
	$<TARGET_OBJECTS:fontfile>
	$<TARGET_OBJECTS:fc>
	$<TARGET_OBJECTS:Speedo>
	$<TARGET_OBJECTS:Type1>
	$<TARGET_OBJECTS:FreeType>
	$<TARGET_OBJECTS:util>)
