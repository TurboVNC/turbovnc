# Microsoft Developer Studio Project File - Name="vncviewer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=vncviewer - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "vncviewer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "vncviewer.mak" CFG="vncviewer - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "vncviewer - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "vncviewer - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/vncviewer", KYAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "vncviewer - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /Ob0 /I "omnithread" /I ".." /I "..\turbojpeg" /I "..\..\..\vgl\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__NT__" /D "_WINSTATIC" /D "__WIN32__" /D "XMD_H" /YX"stdhdrs.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib wsock32.lib comctl32.lib htmlhelp.lib Release/turbojpeg.lib fbx.lib /nologo /subsystem:windows /machine:I386 /libpath:"..\..\..\vgl\windows\vnc\lib"
# Begin Special Build Tool
IntDir=.\Release
SOURCE="$(InputPath)"
PreLink_Cmds=link /lib /def:..\turbojpeg\turbojpeg.def /out:$(IntDir)\turbojpeg.lib
# End Special Build Tool

!ELSEIF  "$(CFG)" == "vncviewer - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "omnithread" /I ".." /I "..\turbojpeg" /I "..\..\..\vgl\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "__NT__" /D "_WINSTATIC" /D "__WIN32__" /D "XMD_H" /YX"stdhdrs.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib wsock32.lib comctl32.lib htmlhelp.lib Debug/turbojpeg.lib fbx.lib /nologo /subsystem:windows /map /debug /machine:I386 /nodefaultlib:"libcd" /pdbtype:sept /libpath:"..\..\..\vgl\windows\vnc\dbg\lib"
# Begin Special Build Tool
IntDir=.\Debug
SOURCE="$(InputPath)"
PreLink_Cmds=link /lib /def:..\turbojpeg\turbojpeg.def /out:$(IntDir)\turbojpeg.lib
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "vncviewer - Win32 Release"
# Name "vncviewer - Win32 Debug"
# Begin Group "Resources"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\cursor1.cur
# End Source File
# Begin Source File

SOURCE=.\res\filereload.ico
# End Source File
# Begin Source File

SOURCE=.\res\fileup.ico
# End Source File
# Begin Source File

SOURCE=.\res\idr_tray.ico
# End Source File
# Begin Source File

SOURCE=.\res\nocursor.cur
# End Source File
# Begin Source File

SOURCE=.\res\smalldot.cur
# End Source File
# Begin Source File

SOURCE=.\res\toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbar1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\turbovnc.bmp
# End Source File
# Begin Source File

SOURCE=.\res\turbovnc48.bmp
# End Source File
# Begin Source File

SOURCE=.\res\vnc.bmp
# End Source File
# Begin Source File

SOURCE=.\res\vnc32.BMP
# End Source File
# Begin Source File

SOURCE=.\res\vncviewer.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\AboutBox.cpp
# End Source File
# Begin Source File

SOURCE=.\AboutBox.h
# End Source File
# Begin Source File

SOURCE=.\AuthDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\AuthDialog.h
# End Source File
# Begin Source File

SOURCE=.\CapsContainer.cpp
# End Source File
# Begin Source File

SOURCE=.\CapsContainer.h
# End Source File
# Begin Source File

SOURCE=.\ClientConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientConnection.h
# End Source File
# Begin Source File

SOURCE=.\ClientConnectionClipboard.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientConnectionCopyRect.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientConnectionCursor.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientConnectionFile.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientConnectionFullScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientConnectionTight.cpp
# End Source File
# Begin Source File

SOURCE=.\ConnectingDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ConnectingDialog.h
# End Source File
# Begin Source File

SOURCE=.\COPYING.txt
# End Source File
# Begin Source File

SOURCE=.\d3des.c
# End Source File
# Begin Source File

SOURCE=.\d3des.h
# End Source File
# Begin Source File

SOURCE=.\Daemon.cpp
# End Source File
# Begin Source File

SOURCE=.\Daemon.h
# End Source File
# Begin Source File

SOURCE=.\Exception.cpp
# End Source File
# Begin Source File

SOURCE=.\Exception.h
# End Source File
# Begin Source File

SOURCE=.\FileTransfer.cpp
# End Source File
# Begin Source File

SOURCE=.\FileTransfer.h
# End Source File
# Begin Source File

SOURCE=.\FileTransferItemInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\FileTransferItemInfo.h
# End Source File
# Begin Source File

SOURCE=.\Flasher.cpp
# End Source File
# Begin Source File

SOURCE=.\Flasher.h
# End Source File
# Begin Source File

SOURCE=.\History.txt
# End Source File
# Begin Source File

SOURCE=.\HotKeys.cpp
# End Source File
# Begin Source File

SOURCE=.\HotKeys.h
# End Source File
# Begin Source File

SOURCE=.\KeyMap.cpp
# End Source File
# Begin Source File

SOURCE=.\KeyMap.h
# End Source File
# Begin Source File

SOURCE=.\keysymdef.h
# End Source File
# Begin Source File

SOURCE=.\LICENCE.txt
# End Source File
# Begin Source File

SOURCE=.\Log.cpp
# End Source File
# Begin Source File

SOURCE=.\Log.h
# End Source File
# Begin Source File

SOURCE=.\LoginAuthDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\LoginAuthDialog.h
# End Source File
# Begin Source File

SOURCE=.\res\resource.h
# End Source File
# Begin Source File

SOURCE=.\rfb.h
# End Source File
# Begin Source File

SOURCE=..\rfb\rfbproto.h
# End Source File
# Begin Source File

SOURCE=.\SessionDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\SessionDialog.h
# End Source File
# Begin Source File

SOURCE=.\stdhdrs.cpp

!IF  "$(CFG)" == "vncviewer - Win32 Release"

# ADD CPP /YX"stdhdrs.h"

!ELSEIF  "$(CFG)" == "vncviewer - Win32 Debug"

# ADD CPP /Yc"stdhdrs.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\stdhdrs.h
# End Source File
# Begin Source File

SOURCE=.\ToDo.txt
# End Source File
# Begin Source File

SOURCE=.\vncauth.c
# End Source File
# Begin Source File

SOURCE=.\vncauth.h
# End Source File
# Begin Source File

SOURCE=.\VNCHelp.cpp
# End Source File
# Begin Source File

SOURCE=.\VNCHelp.h
# End Source File
# Begin Source File

SOURCE=.\VNCOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\VNCOptions.h
# End Source File
# Begin Source File

SOURCE=.\vncviewer.cpp
# End Source File
# Begin Source File

SOURCE=.\vncviewer.h
# End Source File
# Begin Source File

SOURCE=.\res\vncviewer.rc
# End Source File
# Begin Source File

SOURCE=.\VNCviewerApp.cpp
# End Source File
# Begin Source File

SOURCE=.\VNCviewerApp.h
# End Source File
# Begin Source File

SOURCE=.\VNCviewerApp32.cpp
# End Source File
# Begin Source File

SOURCE=.\VNCviewerApp32.h
# End Source File
# End Target
# End Project
