add_definitions(-D_WINSTATIC -D_WIN32_WINNT=0x0500 -DNTDDI_VERSION=0x05000100)

include_directories(omnithread ${CMAKE_SOURCE_DIR}/common
	${CMAKE_SOURCE_DIR}/common/d3des ${CMAKE_CURRENT_BINARY_DIR}/res
	${CMAKE_SOURCE_DIR}/win/util)

if(BITS EQUAL 64)
	set(APPNAME "TurboVNC Viewer 64-bit v${VERSION} (${BUILD})")
else()
	set(APPNAME "TurboVNC Viewer v${VERSION} (${BUILD})")
endif()

configure_file(res/resdefs.h.in res/resdefs.h)

set(VNCVIEWER_SOURCES
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
	ClientConnectionGII.cpp
	ClientConnectionHextile.cpp
	ClientConnectionRaw.cpp
	ClientConnectionTight.cpp
	ClientConnectionTunnel.cpp
	ConnectingDialog.cpp
	${CMAKE_SOURCE_DIR}/common/d3des/d3des.c
	Daemon.cpp
	Exception.cpp
	FileTransfer.cpp
	FileTransferItemInfo.cpp
	HotKeys.cpp
	KeyMap.cpp
	keysym2ucs.c
	Log.cpp
	LoginAuthDialog.cpp
	LowLevelHook.cpp
	SessionDialog.cpp
	stdhdrs.cpp
	strsep.c
	vncauth.c
	VNCOptions.cpp
	vncviewer.cpp
	VNCHelp.cpp
	VNCviewerApp.cpp
	VNCviewerApp32.cpp
	omnithread/nt.cpp
	wintab/Utils.c
	res/vncviewer.rc)

add_executable(vncviewer WIN32 ${VNCVIEWER_SOURCES})
option(TVNC_WINCONSOLE
	"Build Win32 console version of the TurboVNC Viewer (useful for debugging)"
	1)
if(TVNC_WINCONSOLE)
	add_executable(cvncviewer ${VNCVIEWER_SOURCES})
endif()

target_link_libraries(vncviewer zlib ${TJPEG_LIBRARY} fbx ws2_32.lib winmm.lib
	htmlhelp.lib comctl32.lib imm32.lib)
if(TVNC_WINCONSOLE)
	target_link_libraries(cvncviewer zlib ${TJPEG_LIBRARY} fbx ws2_32.lib
		winmm.lib htmlhelp.lib comctl32.lib imm32.lib)
endif()

install(TARGETS vncviewer DESTINATION ${CMAKE_INSTALL_BINDIR})
if(TVNC_WINCONSOLE)
	install(TARGETS cvncviewer DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
