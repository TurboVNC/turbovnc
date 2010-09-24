project(vncviewer)

add_definitions(-D_WINSTATIC -D_WIN32_WINNT=0x0400)
include_directories(omnithread zlib ${CMAKE_SOURCE_DIR})

add_subdirectory(zlib)

if(CMAKE_CL_64)
	set(APPNAME "TurboVNC Viewer 64-bit v${VERSION} (${BUILD})")
	set(WIN64DEF -DWIN64)
else(CMAKE_CL_64)
	set(APPNAME "TurboVNC Viewer v${VERSION} (${BUILD})")
endif(CMAKE_CL_64)

set(CMAKE_RC_COMPILE_OBJECT
	"<CMAKE_RC_COMPILER> -D__APPNAME=\\\"\"${APPNAME}\"\\\" ${WIN64DEF} -D__RCVERSION=${RCVERSION} -D__RCVERSIONSTR=\\\"${RCVERSION}\\0\\\" -l 0x409 -fo <OBJECT> <SOURCE>")

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
	wsock32.lib gdi32.lib comdlg32.lib winmm.lib htmlhelp.lib comctl32.lib
	user32.lib advapi32.lib shell32.lib
	turbojpeg-static.lib)
