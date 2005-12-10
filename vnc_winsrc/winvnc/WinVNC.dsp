# Microsoft Developer Studio Project File - Name="WinVNC" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=WinVNC - Win32 Profile
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "WinVNC.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "WinVNC.mak" CFG="WinVNC - Win32 Profile"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "WinVNC - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "WinVNC - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "WinVNC - Win32 Profile" (based on "Win32 (x86) Application")
!MESSAGE "WinVNC - Win32 HorizonLive" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/WinVNC", IABAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "WinVNC - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "./omnithread" /I "./zlib" /I ".." /I "../turbojpeg" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "__WIN32__" /D "__NT__" /D "__x86__" /D "_WINSTATIC" /D "NCORBA" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG" /d "WITH_JAVA_VIEWER"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comctl32.lib htmlhelp.lib Release/turbojpeg.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"LIBC"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
IntDir=.\Release
SOURCE="$(InputPath)"
PreLink_Cmds=link /lib /def:..\turbojpeg\turbojpeg.def /out:$(IntDir)\turbojpeg.lib
# End Special Build Tool

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "./omnithread" /I "./zlib" /I ".." /I "../turbojpeg" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "__WIN32__" /D "__NT__" /D "_WINSTATIC" /D "__x86__" /D "NCORBA" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG" /d "WITH_JAVA_VIEWER"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comctl32.lib htmlhelp.lib Debug/turbojpeg.lib /nologo /subsystem:windows /map /debug /machine:I386 /nodefaultlib:"LIBCD" /pdbtype:sept
# SUBTRACT LINK32 /incremental:no
# Begin Special Build Tool
IntDir=.\Debug
SOURCE="$(InputPath)"
PreLink_Cmds=link /lib /def:..\turbojpeg\turbojpeg.def /out:$(IntDir)\turbojpeg.lib
# End Special Build Tool

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinVNC__"
# PROP BASE Intermediate_Dir "WinVNC__"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Profile"
# PROP Intermediate_Dir "Profile"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "O:\release\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "__WIN32__" /D "__NT__" /D "__x86__" /D "_CORBA" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "./omnithread" /I "./zlib" /I ".." /I "../turbojpeg" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "__WIN32__" /D "__NT__" /D "_WINSTATIC" /D "__x86__" /D "NCORBA" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG" /d "WITH_JAVA_VIEWER"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wsock32.lib omniORB2_rtd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"o:\release\lib\x86_nt_3.5"
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comctl32.lib htmlhelp.lib Profile/turbojpeg.lib /nologo /subsystem:windows /profile /map /debug /machine:I386
# Begin Special Build Tool
IntDir=.\Profile
SOURCE="$(InputPath)"
PreLink_Cmds=link /lib /def:..\turbojpeg\turbojpeg.def /out:$(IntDir)\turbojpeg.lib
# End Special Build Tool

!ELSEIF  "$(CFG)" == "WinVNC - Win32 HorizonLive"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinVNC___Win32_HorizonLive"
# PROP BASE Intermediate_Dir "WinVNC___Win32_HorizonLive"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "HorizonLive"
# PROP Intermediate_Dir "HorizonLive"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "./omnithread" /I "./zlib" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "__WIN32__" /D "__NT__" /D "__x86__" /D "_WINSTATIC" /D "NCORBA" /D "ZLIB_DLL" /D "XMD_H" /Fr /YX /FD /c
# SUBTRACT BASE CPP /X
# ADD CPP /nologo /MT /W3 /GX /O2 /I "./omnithread" /I "./zlib" /I ".." /I "../turbojpeg" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "__WIN32__" /D "__NT__" /D "__x86__" /D "_WINSTATIC" /D "NCORBA" /D "XMD_H" /D "HORIZONLIVE" /YX /FD /c
# SUBTRACT CPP /X /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"LIBC"
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comctl32.lib htmlhelp.lib HorizonLive/turbojpeg.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"LIBC" /out:"HorizonLive/AppShare.exe"
# SUBTRACT LINK32 /pdb:none /nodefaultlib
# Begin Special Build Tool
IntDir=.\HorizonLive
SOURCE="$(InputPath)"
PreLink_Cmds=link /lib /def:..\turbojpeg\turbojpeg.def /out:$(IntDir)\turbojpeg.lib
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "WinVNC - Win32 Release"
# Name "WinVNC - Win32 Debug"
# Name "WinVNC - Win32 Profile"
# Name "WinVNC - Win32 HorizonLive"
# Begin Group "Source"

# PROP Default_Filter ".cpp, .c"
# Begin Source File

SOURCE=.\AdministrationControls.cpp
# End Source File
# Begin Source File

SOURCE=.\d3des.c
# End Source File
# Begin Source File

SOURCE=.\FileTransferItemInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\IncomingConnectionsControls.cpp
# End Source File
# Begin Source File

SOURCE=.\InputHandlingControls.cpp
# End Source File
# Begin Source File

SOURCE=.\Log.cpp
# End Source File
# Begin Source File

SOURCE=.\MatchWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\MinMax.cpp
# End Source File
# Begin Source File

SOURCE=.\PollControls.cpp
# End Source File
# Begin Source File

SOURCE=.\QuerySettingsControls.cpp
# End Source File
# Begin Source File

SOURCE=.\RectList.cpp
# End Source File
# Begin Source File

SOURCE=.\SharedDesktopArea.cpp
# End Source File
# Begin Source File

SOURCE=.\stdhdrs.cpp
# End Source File
# Begin Source File

SOURCE=.\tableinitcmtemplate.cpp

!IF  "$(CFG)" == "WinVNC - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "WinVNC - Win32 HorizonLive"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\tableinittctemplate.cpp

!IF  "$(CFG)" == "WinVNC - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "WinVNC - Win32 HorizonLive"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\tabletranstemplate.cpp

!IF  "$(CFG)" == "WinVNC - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "WinVNC - Win32 HorizonLive"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\translate.cpp
# End Source File
# Begin Source File

SOURCE=.\VideoDriver.cpp
# End Source File
# Begin Source File

SOURCE=.\vncAbout.cpp
# End Source File
# Begin Source File

SOURCE=.\vncAcceptDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\vncAcceptReverseDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\vncAcceptReverseDlg.h
# End Source File
# Begin Source File

SOURCE=.\vncauth.c
# End Source File
# Begin Source File

SOURCE=.\vncBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\vncClient.cpp
# End Source File
# Begin Source File

SOURCE=.\vncConnDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\vncDesktop.cpp
# End Source File
# Begin Source File

SOURCE=.\vncEncoder.cpp
# End Source File
# Begin Source File

SOURCE=.\vncEncodeTight.cpp
# End Source File
# Begin Source File

SOURCE=.\VNCHelp.cpp
# End Source File
# Begin Source File

SOURCE=.\vncHTTPConnect.cpp
# End Source File
# Begin Source File

SOURCE=.\vncInstHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\vncKeymap.cpp
# End Source File
# Begin Source File

SOURCE=.\vncMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\vncProperties.cpp
# End Source File
# Begin Source File

SOURCE=.\vncRegion.cpp
# End Source File
# Begin Source File

SOURCE=.\vncServer.cpp
# End Source File
# Begin Source File

SOURCE=.\vncService.cpp
# End Source File
# Begin Source File

SOURCE=.\vncSockConnect.cpp
# End Source File
# Begin Source File

SOURCE=.\vncTimedMsgBox.cpp
# End Source File
# Begin Source File

SOURCE=.\VSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\WallpaperUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\WinVNC.cpp
# End Source File
# Begin Source File

SOURCE=.\WinVNC.rc
# End Source File
# End Group
# Begin Group "Headers"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=.\AdministrationControls.h
# End Source File
# Begin Source File

SOURCE=.\d3des.h
# End Source File
# Begin Source File

SOURCE=.\FileTransferItemInfo.h
# End Source File
# Begin Source File

SOURCE=.\IncomingConnectionsControls.h
# End Source File
# Begin Source File

SOURCE=.\InputHandlingControls.h
# End Source File
# Begin Source File

SOURCE=.\keysymdef.h
# End Source File
# Begin Source File

SOURCE=.\Log.h
# End Source File
# Begin Source File

SOURCE=.\MatchWindow.h
# End Source File
# Begin Source File

SOURCE=.\MinMax.h
# End Source File
# Begin Source File

SOURCE=.\PollControls.h
# End Source File
# Begin Source File

SOURCE=.\QuerySettingsControls.h
# End Source File
# Begin Source File

SOURCE=.\RectList.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\rfb.h
# End Source File
# Begin Source File

SOURCE=..\rfb\rfbproto.h
# End Source File
# Begin Source File

SOURCE=.\SharedDesktopArea.h
# End Source File
# Begin Source File

SOURCE=.\stdhdrs.h
# End Source File
# Begin Source File

SOURCE=.\translate.h
# End Source File
# Begin Source File

SOURCE=..\turbojpeg\turbojpeg.h
# End Source File
# Begin Source File

SOURCE=.\VideoDriver.h
# End Source File
# Begin Source File

SOURCE=.\vncAbout.h
# End Source File
# Begin Source File

SOURCE=.\vncAcceptDialog.h
# End Source File
# Begin Source File

SOURCE=.\vncauth.h
# End Source File
# Begin Source File

SOURCE=.\vncBuffer.h
# End Source File
# Begin Source File

SOURCE=.\vncClient.h
# End Source File
# Begin Source File

SOURCE=.\vncConnDialog.h
# End Source File
# Begin Source File

SOURCE=.\vncCorbaConnect.h
# End Source File
# Begin Source File

SOURCE=.\vncDesktop.h
# End Source File
# Begin Source File

SOURCE=.\vncEncodeCoRRE.h

!IF  "$(CFG)" == "WinVNC - Win32 Release"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "WinVNC - Win32 HorizonLive"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vncEncodeHexT.h
# End Source File
# Begin Source File

SOURCE=.\vncEncoder.h
# End Source File
# Begin Source File

SOURCE=.\vncEncodeRRE.h
# End Source File
# Begin Source File

SOURCE=.\vncEncodeTight.h
# End Source File
# Begin Source File

SOURCE=.\vncEncodeZlib.h
# End Source File
# Begin Source File

SOURCE=.\vncEncodeZlibHex.h
# End Source File
# Begin Source File

SOURCE=.\VNCHelp.h
# End Source File
# Begin Source File

SOURCE=.\vncHTTPConnect.h
# End Source File
# Begin Source File

SOURCE=.\vncInstHandler.h
# End Source File
# Begin Source File

SOURCE=.\vncKeymap.h
# End Source File
# Begin Source File

SOURCE=.\vncMenu.h
# End Source File
# Begin Source File

SOURCE=.\vncPasswd.h
# End Source File
# Begin Source File

SOURCE=.\vncProperties.h
# End Source File
# Begin Source File

SOURCE=.\vncRegion.h
# End Source File
# Begin Source File

SOURCE=.\vncServer.h
# End Source File
# Begin Source File

SOURCE=.\vncService.h
# End Source File
# Begin Source File

SOURCE=.\vncSockConnect.h
# End Source File
# Begin Source File

SOURCE=.\vncTimedMsgBox.h
# End Source File
# Begin Source File

SOURCE=.\VSocket.h
# End Source File
# Begin Source File

SOURCE=.\VTypes.h
# End Source File
# Begin Source File

SOURCE=.\WallpaperUtils.h
# End Source File
# Begin Source File

SOURCE=.\WinVNC.h
# End Source File
# End Group
# Begin Group "Resources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\cursor1.cur
# End Source File
# Begin Source File

SOURCE=.\res\ico00001.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00002.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00003.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00004.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\tightvnc.bmp
# End Source File
# Begin Source File

SOURCE=.\res\WinVNC.ico
# End Source File
# End Group
# Begin Group "Java Classes"

# PROP Default_Filter ".class, .jar"
# Begin Source File

SOURCE=.\res\AuthPanel.class
# End Source File
# Begin Source File

SOURCE=.\res\AuthUnixLoginPanel.class
# End Source File
# Begin Source File

SOURCE=.\res\ButtonPanel.class
# End Source File
# Begin Source File

SOURCE=.\res\CapabilityInfo.class
# End Source File
# Begin Source File

SOURCE=.\res\CapsContainer.class
# End Source File
# Begin Source File

SOURCE=.\res\ClipboardFrame.class
# End Source File
# Begin Source File

SOURCE=.\res\DesCipher.class
# End Source File
# Begin Source File

SOURCE=.\res\OptionsFrame.class
# End Source File
# Begin Source File

SOURCE=.\res\RecordingFrame.class
# End Source File
# Begin Source File

SOURCE=.\res\ReloginPanel.class
# End Source File
# Begin Source File

SOURCE=.\res\RfbProto.class
# End Source File
# Begin Source File

SOURCE=.\res\SessionRecorder.class
# End Source File
# Begin Source File

SOURCE=.\res\SocketFactory.class
# End Source File
# Begin Source File

SOURCE=.\res\VncCanvas.class
# End Source File
# Begin Source File

SOURCE=.\res\VncViewer.class
# End Source File
# Begin Source File

SOURCE=.\res\VncViewer.jar
# End Source File
# End Group
# Begin Source File

SOURCE=.\res\bitmap3.bmp
# End Source File
# Begin Source File

SOURCE=.\BUILDING.txt
# End Source File
# Begin Source File

SOURCE=.\History.txt
# End Source File
# End Target
# End Project
