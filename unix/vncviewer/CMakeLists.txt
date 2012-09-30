include_directories(${X11_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/common/rfb .)

add_definitions(-D__VERSION="${VERSION}" -D__BUILD="${BUILD}")
add_definitions(-D__COPYRIGHT_YEAR="${COPYRIGHT_YEAR}")
add_definitions(-D__COPYRIGHT="${COPYRIGHT}" -D__URLTEXT="${URLTEXT}")

add_executable(vncviewer
argsresources.c
caps.c
color.c
cursor.c
desktop.c
dialogs.c
flowcontrol.c
fullscreen.c
listen.c
misc.c
popup.c
rfbproto.c
selection.c
shm.c
sockets.c
tunnel.c
vncviewer.c
caps.h
vncviewer.h)

target_link_libraries(vncviewer ${X11_Xmu_LIB} ${X11_Xaw_LIB} ${X11_Xt_LIB}
	${X11_LIBRARIES} ${X11_Xcursor_LIB} vncauth zlib pthread ${TJPEG_LIBRARY})

install(TARGETS vncviewer DESTINATION ${TVNC_BINDIR})

install(FILES vncviewer.man DESTINATION ${TVNC_MANDIR}/man1 RENAME vncviewer.1)

add_custom_target(res
	COMMAND awk -f ./genres.awk ./argsresources.c >Tvncviewer
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS argsresources.c)
