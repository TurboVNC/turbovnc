project(winvnc)

add_definitions(-D_WINSTATIC -D_WIN32_WINNT=0x0500)
include_directories(omnithread zlib ${CMAKE_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR})

if(CMAKE_CL_64)
	set(APPNAME "TurboVNC Server 64-bit v${VERSION} (${BUILD})")
	set(WIN64 1)
else(CMAKE_CL_64)
	set(APPNAME "TurboVNC Server v${VERSION} (${BUILD})")
	set(WIN64 0)
endif(CMAKE_CL_64)

add_definitions(-DWITH_JAVA_VIEWER)

configure_file(resdefs.h.in resdefs.h)

add_library(VNCHooks SHARED
	VNCHooks/VNCHooks.cpp
	VNCHooks/VNCHooks.def
	VNCHooks/VNCHooks.rc)

target_link_libraries(VNCHooks
	user32.lib gdi32.lib advapi32.lib)

add_executable(winvnc WIN32
	AdministrationControls.cpp
	BuildTime.cpp
	d3des.c
	DynamicFn.cpp
	FileTransferItemInfo.cpp
	IncomingConnectionsControls.cpp
	InputHandlingControls.cpp
	Log.cpp
	MatchWindow.cpp
	MinMax.cpp
	ParseHost.c
	PollControls.cpp
	QuerySettingsControls.cpp
	RectList.cpp
	SharedDesktopArea.cpp
	stdhdrs.cpp
	translate.cpp
	TsSessions.cpp
	VideoDriver.cpp
	vncAbout.cpp
	vncAcceptDialog.cpp
	vncAcceptReverseDlg.cpp
	vncauth.c
	vncBuffer.cpp
	vncClient.cpp
	vncConnDialog.cpp
	vncDesktop.cpp
	vncEncoder.cpp
	vncEncodeCoRRE.cpp
	vncEncodeHexT.cpp
	vncEncodeRRE.cpp
	vncEncodeTight.cpp
	vncEncodeZlib.cpp
	vncEncodeZlibHex.cpp
	VNCHelp.cpp
	vncHTTPConnect.cpp
	vncInstHandler.cpp
	vncKeymap.cpp
	vncMenu.cpp
	vncProperties.cpp
	vncRegion.cpp
	vncServer.cpp
	vncService.cpp
	vncSockConnect.cpp
	vncTimedMsgBox.cpp
	VSocket.cpp
	WallpaperUtils.cpp
	WinVNC.cpp
	omnithread/nt.cpp
	WinVNC.rc)

target_link_libraries(winvnc zlib VNCHooks
	wsock32.lib gdi32.lib comdlg32.lib winmm.lib htmlhelp.lib comctl32.lib
	user32.lib advapi32.lib shell32.lib ole32.lib
	turbojpeg-static.lib)
