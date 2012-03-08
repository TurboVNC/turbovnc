add_definitions(-D_WINSTATIC -D_WIN32_WINNT=0x0400)
include_directories(omnithread zlib ${CMAKE_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/res)

if(CMAKE_CL_64)
	set(APPNAME "TurboVNC Viewer 64-bit v${VERSION} (${BUILD})")
else(CMAKE_CL_64)
	set(APPNAME "TurboVNC Viewer v${VERSION} (${BUILD})")
endif(CMAKE_CL_64)

configure_file(res/resdefs.h.in res/resdefs.h)

add_executable(vncviewer WIN32
	AboutBox.cpp
	BuildTime.cpp
	CapsContainer.cpp
	ClientConnection.cpp
	ClientConnectionClipboard.cpp
	ClientConnectionCopyRect.cpp
	ClientConnectionCursor.cpp
	ClientConnectionFile.cpp
	ClientConnectionFullScreen.cpp
	ClientConnectionHextile.cpp
	ClientConnectionRaw.cpp
	ClientConnectionTight.cpp
	ConnectingDialog.cpp
	d3des.c
	Daemon.cpp
	Exception.cpp
	fbx.c
	FileTransfer.cpp
	FileTransferItemInfo.cpp
	HotKeys.cpp
	KeyMap.cpp
	Log.cpp
	LoginAuthDialog.cpp
	SessionDialog.cpp
	stdhdrs.cpp
	vncauth.c
	VNCOptions.cpp
	vncviewer.cpp
	VNCHelp.cpp
	VNCviewerApp.cpp
	VNCviewerApp32.cpp
	omnithread/nt.cpp
	res/vncviewer.rc)

target_link_libraries(vncviewer zlib
	wsock32.lib winmm.lib htmlhelp.lib comctl32.lib ${TJPEG_LIBRARY})

install(TARGETS vncviewer DESTINATION .)
