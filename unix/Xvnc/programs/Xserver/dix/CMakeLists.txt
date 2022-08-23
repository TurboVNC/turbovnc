include_directories(../Xi ../composite ../damageext ../mi ../miext/sync
	../randr ../render ../xfixes)

set(DEFAULT_XORG_FONT_PATH
	"/usr/share/X11/fonts/misc/,/usr/share/X11/fonts/Type1/,/usr/share/X11/fonts/75dpi/,/usr/share/X11/fonts/100dpi/")
set(XORG_FONT_PATH ${DEFAULT_XORG_FONT_PATH} CACHE STRING
	"X.org font path (default: ${DEFAULT_XORG_FONT_PATH})")
message(STATUS "XORG_FONT_PATH = ${XORG_FONT_PATH}")
add_definitions(-DCOMPILEDDEFAULTFONTPATH=\"${XORG_FONT_PATH}\")

if(BITS EQUAL 64)
	set(DEFAULT_XORG_REGISTRY_PATH "/usr/lib64/xorg")
else()
	set(DEFAULT_XORG_REGISTRY_PATH "/usr/lib/xorg")
endif()
set(XORG_REGISTRY_PATH ${DEFAULT_XORG_REGISTRY_PATH} CACHE PATH
	"X.org registry path, which contains protocol.txt (default: ${DEFAULT_XORG_REGISTRY_PATH})")
message(STATUS "XORG_REGISTRY_PATH = ${XORG_REGISTRY_PATH}")
add_definitions(-DSERVER_MISC_CONFIG_PATH=\"${XORG_REGISTRY_PATH}\")

disable_compiler_warnings()

add_library(dix STATIC
	atom.c
	colormap.c
	cursor.c
	devices.c
	dispatch.c
	dixfonts.c
	dixutils.c
	enterleave.c
	eventconvert.c
	events.c
	extension.c
	gc.c
	getevents.c
	globals.c
	glyphcurs.c
	grabs.c
	initatoms.c
	inpututils.c
	main.c
	pixmap.c
	privates.c
	property.c
	ptrveloc.c
	region.c
	registry.c
	resource.c
	selection.c
	swaprep.c
	swapreq.c
	tables.c
	touch.c
	window.c)
