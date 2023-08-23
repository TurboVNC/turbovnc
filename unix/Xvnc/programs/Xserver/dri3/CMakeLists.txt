include_directories(../miext/sync ../randr ../render)

add_definitions(-DHAVE_XORG_CONFIG_H)

add_library(dri3 STATIC
	dri3.c
	dri3_request.c
	dri3_screen.c)

target_link_libraries(dri3 PkgConfig::DRM)
