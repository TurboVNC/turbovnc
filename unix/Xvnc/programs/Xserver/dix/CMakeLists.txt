add_definitions(-DRGB_DB=\"/usr/share/X11/rgb\"
	-DVENDOR_STRING=\"${CMAKE_PROJECT_NAME}\"
	-DVENDOR_RELEASE=${XVENDORRELEASE}
	-DCOMPILEDDEFAULTFONTPATH=\"/usr/share/X11/fonts/misc/,/usr/share/X11/fonts/Type1/,/usr/share/X11/fonts/75dpi/,/usr/share/X11/fonts/100dpi/\")

add_library(dix STATIC
	atom.c
	colormap.c
	cursor.c
	devices.c
	dispatch.c
	dixutils.c
	events.c
	extension.c
	gc.c
	globals.c
	glyphcurs.c
	grabs.c
	main.c
	property.c
	resource.c
	swaprep.c
	swapreq.c
	tables.c
	window.c
	initatoms.c
	dixfonts.c
	privates.c
	pixmap.c)

add_library(xpstubs STATIC
	xpstubs.c)
