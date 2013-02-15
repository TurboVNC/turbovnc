add_definitions(-D_WINSTATIC -D_WIN32_WINNT=0x0500 -DNTDDI_VERSION=0x05000100)

include_directories(omnithread ${CMAKE_SOURCE_DIR}/common
	${CMAKE_SOURCE_DIR}/common/d3des ${CMAKE_CURRENT_BINARY_DIR}/res)

if(BITS EQUAL 64)
	set(APPNAME "TurboVNC Viewer 64-bit v${VERSION} (${BUILD})")
else()
	set(APPNAME "TurboVNC Viewer v${VERSION} (${BUILD})")
endif()

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
	ClientConnectionFlowControl.cpp
	ClientConnectionFullScreen.cpp
	ClientConnectionHextile.cpp
	ClientConnectionRaw.cpp
	ClientConnectionTight.cpp
	ClientConnectionTunnel.cpp
	ConnectingDialog.cpp
	${CMAKE_SOURCE_DIR}/common/d3des/d3des.c
	Daemon.cpp
	Exception.cpp
	fbx.c
	FileTransfer.cpp
	FileTransferItemInfo.cpp
	HotKeys.cpp
	KeyMap.cpp
	Log.cpp
	LoginAuthDialog.cpp
	LowLevelHook.cpp
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
	ws2_32.lib winmm.lib htmlhelp.lib comctl32.lib ${TJPEG_LIBRARY})

install(TARGETS vncviewer DESTINATION ${TVNC_BINDIR})
