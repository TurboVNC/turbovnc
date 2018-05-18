include_directories(../Xi ../composite ../damageext ../mi ../miext/sync
	../randr ../render ../xfixes)

add_definitions(-DRGB_DB=\"/usr/share/X11/rgb\"
	-DCOMPILEDDEFAULTFONTPATH=\"/usr/share/X11/fonts/misc/,/usr/share/X11/fonts/Type1/,/usr/share/X11/fonts/75dpi/,/usr/share/X11/fonts/100dpi/\")

if(BITS EQUAL 64)
	set(DEFAULT_SERVER_MISC_CONFIG_PATH "/usr/lib64/xorg")
else()
	set(DEFAULT_SERVER_MISC_CONFIG_PATH "/usr/lib/xorg")
endif()
set(SERVER_MISC_CONFIG_PATH ${DEFAULT_SERVER_MISC_CONFIG_PATH} CACHE PATH
	"Path to miscellaneous server config files (default: ${DEFAULT_SERVER_MISC_CONFIG_PATH})")
message(STATUS "SERVER_MISC_CONFIG_PATH = ${SERVER_MISC_CONFIG_PATH}")
add_definitions(-DSERVER_MISC_CONFIG_PATH=\"${SERVER_MISC_CONFIG_PATH}\")

disable_compiler_warnings()
handle_type_puns(
	colormap.c
	cursor.c
	dispatch.c
	dixfonts.c
	dixutils.c
	events.c
	privates.c
	registry.c
	swaprep.c
	swapreq.c
	window.c)

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
	ffs.c
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
