# Microsoft Developer Studio Generated NMAKE File, Based on WinVNC.dsp
!IF "$(CFG)" == ""
CFG=WinVNC - Win32 Release
!MESSAGE No configuration specified. Defaulting to WinVNC - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "WinVNC - Win32 Release" && "$(CFG)" != "WinVNC - Win32 Debug" && "$(CFG)" != "WinVNC - Win32 Profile" && "$(CFG)" != "WinVNC - Win32 HorizonLive"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "WinVNC - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\WinVNC.exe"

!ELSE 

ALL : "zlib - Win32 Release" "omnithread - Win32 Release" "VNCHooks - Win32 Release" "$(OUTDIR)\WinVNC.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"VNCHooks - Win32 ReleaseCLEAN" "omnithread - Win32 ReleaseCLEAN" "zlib - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\AdministrationControls.obj"
	-@erase "$(INTDIR)\d3des.obj"
	-@erase "$(INTDIR)\FileTransferItemInfo.obj"
	-@erase "$(INTDIR)\IncomingConnectionsControls.obj"
	-@erase "$(INTDIR)\InputHandlingControls.obj"
	-@erase "$(INTDIR)\Log.obj"
	-@erase "$(INTDIR)\MatchWindow.obj"
	-@erase "$(INTDIR)\MinMax.obj"
	-@erase "$(INTDIR)\PollControls.obj"
	-@erase "$(INTDIR)\QuerySettingsControls.obj"
	-@erase "$(INTDIR)\RectList.obj"
	-@erase "$(INTDIR)\SharedDesktopArea.obj"
	-@erase "$(INTDIR)\stdhdrs.obj"
	-@erase "$(INTDIR)\translate.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\VideoDriver.obj"
	-@erase "$(INTDIR)\vncAbout.obj"
	-@erase "$(INTDIR)\vncAcceptDialog.obj"
	-@erase "$(INTDIR)\vncAcceptReverseDlg.obj"
	-@erase "$(INTDIR)\vncauth.obj"
	-@erase "$(INTDIR)\vncBuffer.obj"
	-@erase "$(INTDIR)\vncClient.obj"
	-@erase "$(INTDIR)\vncConnDialog.obj"
	-@erase "$(INTDIR)\vncDesktop.obj"
	-@erase "$(INTDIR)\vncEncoder.obj"
	-@erase "$(INTDIR)\vncEncodeTight.obj"
	-@erase "$(INTDIR)\VNCHelp.obj"
	-@erase "$(INTDIR)\vncHTTPConnect.obj"
	-@erase "$(INTDIR)\vncInstHandler.obj"
	-@erase "$(INTDIR)\vncKeymap.obj"
	-@erase "$(INTDIR)\vncMenu.obj"
	-@erase "$(INTDIR)\vncProperties.obj"
	-@erase "$(INTDIR)\vncRegion.obj"
	-@erase "$(INTDIR)\vncServer.obj"
	-@erase "$(INTDIR)\vncService.obj"
	-@erase "$(INTDIR)\vncSockConnect.obj"
	-@erase "$(INTDIR)\vncTimedMsgBox.obj"
	-@erase "$(INTDIR)\VSocket.obj"
	-@erase "$(INTDIR)\WallpaperUtils.obj"
	-@erase "$(INTDIR)\WinVNC.obj"
	-@erase "$(INTDIR)\WinVNC.res"
	-@erase "$(OUTDIR)\WinVNC.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "./omnithread" /I "./zlib" /I ".." /I "../../../vgl/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "__WIN32__" /D "__NT__" /D "__x86__" /D "_WINSTATIC" /D "NCORBA" /Fp"$(INTDIR)\WinVNC.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\WinVNC.res" /d "NDEBUG" /d "WITH_JAVA_VIEWER" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\WinVNC.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comctl32.lib htmlhelp.lib turbojpeg.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\WinVNC.pdb" /machine:I386 /nodefaultlib:"LIBC" /out:"$(OUTDIR)\WinVNC.exe" /libpath:"../../../vgl/windows/vnc/lib" 
LINK32_OBJS= \
	"$(INTDIR)\AdministrationControls.obj" \
	"$(INTDIR)\d3des.obj" \
	"$(INTDIR)\FileTransferItemInfo.obj" \
	"$(INTDIR)\IncomingConnectionsControls.obj" \
	"$(INTDIR)\InputHandlingControls.obj" \
	"$(INTDIR)\Log.obj" \
	"$(INTDIR)\MatchWindow.obj" \
	"$(INTDIR)\MinMax.obj" \
	"$(INTDIR)\PollControls.obj" \
	"$(INTDIR)\QuerySettingsControls.obj" \
	"$(INTDIR)\RectList.obj" \
	"$(INTDIR)\SharedDesktopArea.obj" \
	"$(INTDIR)\stdhdrs.obj" \
	"$(INTDIR)\translate.obj" \
	"$(INTDIR)\VideoDriver.obj" \
	"$(INTDIR)\vncAbout.obj" \
	"$(INTDIR)\vncAcceptDialog.obj" \
	"$(INTDIR)\vncAcceptReverseDlg.obj" \
	"$(INTDIR)\vncauth.obj" \
	"$(INTDIR)\vncBuffer.obj" \
	"$(INTDIR)\vncClient.obj" \
	"$(INTDIR)\vncConnDialog.obj" \
	"$(INTDIR)\vncDesktop.obj" \
	"$(INTDIR)\vncEncoder.obj" \
	"$(INTDIR)\vncEncodeTight.obj" \
	"$(INTDIR)\VNCHelp.obj" \
	"$(INTDIR)\vncHTTPConnect.obj" \
	"$(INTDIR)\vncInstHandler.obj" \
	"$(INTDIR)\vncKeymap.obj" \
	"$(INTDIR)\vncMenu.obj" \
	"$(INTDIR)\vncProperties.obj" \
	"$(INTDIR)\vncRegion.obj" \
	"$(INTDIR)\vncServer.obj" \
	"$(INTDIR)\vncService.obj" \
	"$(INTDIR)\vncSockConnect.obj" \
	"$(INTDIR)\vncTimedMsgBox.obj" \
	"$(INTDIR)\VSocket.obj" \
	"$(INTDIR)\WallpaperUtils.obj" \
	"$(INTDIR)\WinVNC.obj" \
	"$(INTDIR)\WinVNC.res" \
	"$(OUTDIR)\VNCHooks.lib" \
	"$(OUTDIR)\omnithread.lib" \
	"$(OUTDIR)\zlib.lib"

"$(OUTDIR)\WinVNC.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\WinVNC.exe"

!ELSE 

ALL : "zlib - Win32 Debug" "omnithread - Win32 Debug" "VNCHooks - Win32 Debug" "$(OUTDIR)\WinVNC.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"VNCHooks - Win32 DebugCLEAN" "omnithread - Win32 DebugCLEAN" "zlib - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\AdministrationControls.obj"
	-@erase "$(INTDIR)\d3des.obj"
	-@erase "$(INTDIR)\FileTransferItemInfo.obj"
	-@erase "$(INTDIR)\IncomingConnectionsControls.obj"
	-@erase "$(INTDIR)\InputHandlingControls.obj"
	-@erase "$(INTDIR)\Log.obj"
	-@erase "$(INTDIR)\MatchWindow.obj"
	-@erase "$(INTDIR)\MinMax.obj"
	-@erase "$(INTDIR)\PollControls.obj"
	-@erase "$(INTDIR)\QuerySettingsControls.obj"
	-@erase "$(INTDIR)\RectList.obj"
	-@erase "$(INTDIR)\SharedDesktopArea.obj"
	-@erase "$(INTDIR)\stdhdrs.obj"
	-@erase "$(INTDIR)\translate.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\VideoDriver.obj"
	-@erase "$(INTDIR)\vncAbout.obj"
	-@erase "$(INTDIR)\vncAcceptDialog.obj"
	-@erase "$(INTDIR)\vncAcceptReverseDlg.obj"
	-@erase "$(INTDIR)\vncauth.obj"
	-@erase "$(INTDIR)\vncBuffer.obj"
	-@erase "$(INTDIR)\vncClient.obj"
	-@erase "$(INTDIR)\vncConnDialog.obj"
	-@erase "$(INTDIR)\vncDesktop.obj"
	-@erase "$(INTDIR)\vncEncoder.obj"
	-@erase "$(INTDIR)\vncEncodeTight.obj"
	-@erase "$(INTDIR)\VNCHelp.obj"
	-@erase "$(INTDIR)\vncHTTPConnect.obj"
	-@erase "$(INTDIR)\vncInstHandler.obj"
	-@erase "$(INTDIR)\vncKeymap.obj"
	-@erase "$(INTDIR)\vncMenu.obj"
	-@erase "$(INTDIR)\vncProperties.obj"
	-@erase "$(INTDIR)\vncRegion.obj"
	-@erase "$(INTDIR)\vncServer.obj"
	-@erase "$(INTDIR)\vncService.obj"
	-@erase "$(INTDIR)\vncSockConnect.obj"
	-@erase "$(INTDIR)\vncTimedMsgBox.obj"
	-@erase "$(INTDIR)\VSocket.obj"
	-@erase "$(INTDIR)\WallpaperUtils.obj"
	-@erase "$(INTDIR)\WinVNC.obj"
	-@erase "$(INTDIR)\WinVNC.res"
	-@erase "$(OUTDIR)\WinVNC.exe"
	-@erase "$(OUTDIR)\WinVNC.ilk"
	-@erase "$(OUTDIR)\WinVNC.map"
	-@erase "$(OUTDIR)\WinVNC.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "./omnithread" /I "./zlib" /I ".." /I "../../../vgl/include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "__WIN32__" /D "__NT__" /D "_WINSTATIC" /D "__x86__" /D "NCORBA" /Fp"$(INTDIR)\WinVNC.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\WinVNC.res" /d "_DEBUG" /d "WITH_JAVA_VIEWER" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\WinVNC.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comctl32.lib htmlhelp.lib turbojpeg.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\WinVNC.pdb" /map:"$(INTDIR)\WinVNC.map" /debug /machine:I386 /nodefaultlib:"LIBCD" /out:"$(OUTDIR)\WinVNC.exe" /pdbtype:sept /libpath:"../../../vgl/windows/vnc/dbg/lib" 
LINK32_OBJS= \
	"$(INTDIR)\AdministrationControls.obj" \
	"$(INTDIR)\d3des.obj" \
	"$(INTDIR)\FileTransferItemInfo.obj" \
	"$(INTDIR)\IncomingConnectionsControls.obj" \
	"$(INTDIR)\InputHandlingControls.obj" \
	"$(INTDIR)\Log.obj" \
	"$(INTDIR)\MatchWindow.obj" \
	"$(INTDIR)\MinMax.obj" \
	"$(INTDIR)\PollControls.obj" \
	"$(INTDIR)\QuerySettingsControls.obj" \
	"$(INTDIR)\RectList.obj" \
	"$(INTDIR)\SharedDesktopArea.obj" \
	"$(INTDIR)\stdhdrs.obj" \
	"$(INTDIR)\translate.obj" \
	"$(INTDIR)\VideoDriver.obj" \
	"$(INTDIR)\vncAbout.obj" \
	"$(INTDIR)\vncAcceptDialog.obj" \
	"$(INTDIR)\vncAcceptReverseDlg.obj" \
	"$(INTDIR)\vncauth.obj" \
	"$(INTDIR)\vncBuffer.obj" \
	"$(INTDIR)\vncClient.obj" \
	"$(INTDIR)\vncConnDialog.obj" \
	"$(INTDIR)\vncDesktop.obj" \
	"$(INTDIR)\vncEncoder.obj" \
	"$(INTDIR)\vncEncodeTight.obj" \
	"$(INTDIR)\VNCHelp.obj" \
	"$(INTDIR)\vncHTTPConnect.obj" \
	"$(INTDIR)\vncInstHandler.obj" \
	"$(INTDIR)\vncKeymap.obj" \
	"$(INTDIR)\vncMenu.obj" \
	"$(INTDIR)\vncProperties.obj" \
	"$(INTDIR)\vncRegion.obj" \
	"$(INTDIR)\vncServer.obj" \
	"$(INTDIR)\vncService.obj" \
	"$(INTDIR)\vncSockConnect.obj" \
	"$(INTDIR)\vncTimedMsgBox.obj" \
	"$(INTDIR)\VSocket.obj" \
	"$(INTDIR)\WallpaperUtils.obj" \
	"$(INTDIR)\WinVNC.obj" \
	"$(INTDIR)\WinVNC.res" \
	"$(OUTDIR)\VNCHooks.lib" \
	"$(OUTDIR)\omnithread.lib" \
	"$(OUTDIR)\zlib.lib"

"$(OUTDIR)\WinVNC.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"

OUTDIR=.\Profile
INTDIR=.\Profile
# Begin Custom Macros
OutDir=.\Profile
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\WinVNC.exe"

!ELSE 

ALL : "zlib - Win32 Profile" "omnithread - Win32 Profile" "VNCHooks - Win32 Profile" "$(OUTDIR)\WinVNC.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"VNCHooks - Win32 ProfileCLEAN" "omnithread - Win32 ProfileCLEAN" "zlib - Win32 ProfileCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\AdministrationControls.obj"
	-@erase "$(INTDIR)\d3des.obj"
	-@erase "$(INTDIR)\FileTransferItemInfo.obj"
	-@erase "$(INTDIR)\IncomingConnectionsControls.obj"
	-@erase "$(INTDIR)\InputHandlingControls.obj"
	-@erase "$(INTDIR)\Log.obj"
	-@erase "$(INTDIR)\MatchWindow.obj"
	-@erase "$(INTDIR)\MinMax.obj"
	-@erase "$(INTDIR)\PollControls.obj"
	-@erase "$(INTDIR)\QuerySettingsControls.obj"
	-@erase "$(INTDIR)\RectList.obj"
	-@erase "$(INTDIR)\SharedDesktopArea.obj"
	-@erase "$(INTDIR)\stdhdrs.obj"
	-@erase "$(INTDIR)\translate.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\VideoDriver.obj"
	-@erase "$(INTDIR)\vncAbout.obj"
	-@erase "$(INTDIR)\vncAcceptDialog.obj"
	-@erase "$(INTDIR)\vncAcceptReverseDlg.obj"
	-@erase "$(INTDIR)\vncauth.obj"
	-@erase "$(INTDIR)\vncBuffer.obj"
	-@erase "$(INTDIR)\vncClient.obj"
	-@erase "$(INTDIR)\vncConnDialog.obj"
	-@erase "$(INTDIR)\vncDesktop.obj"
	-@erase "$(INTDIR)\vncEncoder.obj"
	-@erase "$(INTDIR)\vncEncodeTight.obj"
	-@erase "$(INTDIR)\VNCHelp.obj"
	-@erase "$(INTDIR)\vncHTTPConnect.obj"
	-@erase "$(INTDIR)\vncInstHandler.obj"
	-@erase "$(INTDIR)\vncKeymap.obj"
	-@erase "$(INTDIR)\vncMenu.obj"
	-@erase "$(INTDIR)\vncProperties.obj"
	-@erase "$(INTDIR)\vncRegion.obj"
	-@erase "$(INTDIR)\vncServer.obj"
	-@erase "$(INTDIR)\vncService.obj"
	-@erase "$(INTDIR)\vncSockConnect.obj"
	-@erase "$(INTDIR)\vncTimedMsgBox.obj"
	-@erase "$(INTDIR)\VSocket.obj"
	-@erase "$(INTDIR)\WallpaperUtils.obj"
	-@erase "$(INTDIR)\WinVNC.obj"
	-@erase "$(INTDIR)\WinVNC.res"
	-@erase "$(OUTDIR)\WinVNC.exe"
	-@erase "$(OUTDIR)\WinVNC.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "./omnithread" /I "./zlib" /I ".." /I "../../../vgl/include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "__WIN32__" /D "__NT__" /D "_WINSTATIC" /D "__x86__" /D "NCORBA" /Fp"$(INTDIR)\WinVNC.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\WinVNC.res" /d "_DEBUG" /d "WITH_JAVA_VIEWER" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\WinVNC.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comctl32.lib htmlhelp.lib turbojpeg.lib /nologo /subsystem:windows /profile /map:"$(INTDIR)\WinVNC.map" /debug /machine:I386 /out:"$(OUTDIR)\WinVNC.exe" /libpath:"../../../vgl/windows/vnc/dbg/lib" 
LINK32_OBJS= \
	"$(INTDIR)\AdministrationControls.obj" \
	"$(INTDIR)\d3des.obj" \
	"$(INTDIR)\FileTransferItemInfo.obj" \
	"$(INTDIR)\IncomingConnectionsControls.obj" \
	"$(INTDIR)\InputHandlingControls.obj" \
	"$(INTDIR)\Log.obj" \
	"$(INTDIR)\MatchWindow.obj" \
	"$(INTDIR)\MinMax.obj" \
	"$(INTDIR)\PollControls.obj" \
	"$(INTDIR)\QuerySettingsControls.obj" \
	"$(INTDIR)\RectList.obj" \
	"$(INTDIR)\SharedDesktopArea.obj" \
	"$(INTDIR)\stdhdrs.obj" \
	"$(INTDIR)\translate.obj" \
	"$(INTDIR)\VideoDriver.obj" \
	"$(INTDIR)\vncAbout.obj" \
	"$(INTDIR)\vncAcceptDialog.obj" \
	"$(INTDIR)\vncAcceptReverseDlg.obj" \
	"$(INTDIR)\vncauth.obj" \
	"$(INTDIR)\vncBuffer.obj" \
	"$(INTDIR)\vncClient.obj" \
	"$(INTDIR)\vncConnDialog.obj" \
	"$(INTDIR)\vncDesktop.obj" \
	"$(INTDIR)\vncEncoder.obj" \
	"$(INTDIR)\vncEncodeTight.obj" \
	"$(INTDIR)\VNCHelp.obj" \
	"$(INTDIR)\vncHTTPConnect.obj" \
	"$(INTDIR)\vncInstHandler.obj" \
	"$(INTDIR)\vncKeymap.obj" \
	"$(INTDIR)\vncMenu.obj" \
	"$(INTDIR)\vncProperties.obj" \
	"$(INTDIR)\vncRegion.obj" \
	"$(INTDIR)\vncServer.obj" \
	"$(INTDIR)\vncService.obj" \
	"$(INTDIR)\vncSockConnect.obj" \
	"$(INTDIR)\vncTimedMsgBox.obj" \
	"$(INTDIR)\VSocket.obj" \
	"$(INTDIR)\WallpaperUtils.obj" \
	"$(INTDIR)\WinVNC.obj" \
	"$(INTDIR)\WinVNC.res" \
	"$(OUTDIR)\VNCHooks.lib" \
	"$(OUTDIR)\omnithread.lib" \
	"$(OUTDIR)\zlib.lib"

"$(OUTDIR)\WinVNC.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "WinVNC - Win32 HorizonLive"

OUTDIR=.\HorizonLive
INTDIR=.\HorizonLive
# Begin Custom Macros
OutDir=.\HorizonLive
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\AppShare.exe"

!ELSE 

ALL : "zlib - Win32 HorizonLive" "omnithread - Win32 HorizonLive" "VNCHooks - Win32 HorizonLive" "$(OUTDIR)\AppShare.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"VNCHooks - Win32 HorizonLiveCLEAN" "omnithread - Win32 HorizonLiveCLEAN" "zlib - Win32 HorizonLiveCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\AdministrationControls.obj"
	-@erase "$(INTDIR)\d3des.obj"
	-@erase "$(INTDIR)\FileTransferItemInfo.obj"
	-@erase "$(INTDIR)\IncomingConnectionsControls.obj"
	-@erase "$(INTDIR)\InputHandlingControls.obj"
	-@erase "$(INTDIR)\Log.obj"
	-@erase "$(INTDIR)\MatchWindow.obj"
	-@erase "$(INTDIR)\MinMax.obj"
	-@erase "$(INTDIR)\PollControls.obj"
	-@erase "$(INTDIR)\QuerySettingsControls.obj"
	-@erase "$(INTDIR)\RectList.obj"
	-@erase "$(INTDIR)\SharedDesktopArea.obj"
	-@erase "$(INTDIR)\stdhdrs.obj"
	-@erase "$(INTDIR)\translate.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\VideoDriver.obj"
	-@erase "$(INTDIR)\vncAbout.obj"
	-@erase "$(INTDIR)\vncAcceptDialog.obj"
	-@erase "$(INTDIR)\vncAcceptReverseDlg.obj"
	-@erase "$(INTDIR)\vncauth.obj"
	-@erase "$(INTDIR)\vncBuffer.obj"
	-@erase "$(INTDIR)\vncClient.obj"
	-@erase "$(INTDIR)\vncConnDialog.obj"
	-@erase "$(INTDIR)\vncDesktop.obj"
	-@erase "$(INTDIR)\vncEncoder.obj"
	-@erase "$(INTDIR)\vncEncodeTight.obj"
	-@erase "$(INTDIR)\VNCHelp.obj"
	-@erase "$(INTDIR)\vncHTTPConnect.obj"
	-@erase "$(INTDIR)\vncInstHandler.obj"
	-@erase "$(INTDIR)\vncKeymap.obj"
	-@erase "$(INTDIR)\vncMenu.obj"
	-@erase "$(INTDIR)\vncProperties.obj"
	-@erase "$(INTDIR)\vncRegion.obj"
	-@erase "$(INTDIR)\vncServer.obj"
	-@erase "$(INTDIR)\vncService.obj"
	-@erase "$(INTDIR)\vncSockConnect.obj"
	-@erase "$(INTDIR)\vncTimedMsgBox.obj"
	-@erase "$(INTDIR)\VSocket.obj"
	-@erase "$(INTDIR)\WallpaperUtils.obj"
	-@erase "$(INTDIR)\WinVNC.obj"
	-@erase "$(INTDIR)\WinVNC.res"
	-@erase "$(OUTDIR)\AppShare.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "./omnithread" /I "./zlib" /I ".." /I "../../../vgl/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "__WIN32__" /D "__NT__" /D "__x86__" /D "_WINSTATIC" /D "NCORBA" /D "XMD_H" /D "HORIZONLIVE" /Fp"$(INTDIR)\WinVNC.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\WinVNC.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\WinVNC.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comctl32.lib htmlhelp.lib turbojpeg.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\AppShare.pdb" /machine:I386 /nodefaultlib:"LIBC" /out:"$(OUTDIR)\AppShare.exe" /libpath:"../../../vgl/windows/vnc/lib" 
LINK32_OBJS= \
	"$(INTDIR)\AdministrationControls.obj" \
	"$(INTDIR)\d3des.obj" \
	"$(INTDIR)\FileTransferItemInfo.obj" \
	"$(INTDIR)\IncomingConnectionsControls.obj" \
	"$(INTDIR)\InputHandlingControls.obj" \
	"$(INTDIR)\Log.obj" \
	"$(INTDIR)\MatchWindow.obj" \
	"$(INTDIR)\MinMax.obj" \
	"$(INTDIR)\PollControls.obj" \
	"$(INTDIR)\QuerySettingsControls.obj" \
	"$(INTDIR)\RectList.obj" \
	"$(INTDIR)\SharedDesktopArea.obj" \
	"$(INTDIR)\stdhdrs.obj" \
	"$(INTDIR)\translate.obj" \
	"$(INTDIR)\VideoDriver.obj" \
	"$(INTDIR)\vncAbout.obj" \
	"$(INTDIR)\vncAcceptDialog.obj" \
	"$(INTDIR)\vncAcceptReverseDlg.obj" \
	"$(INTDIR)\vncauth.obj" \
	"$(INTDIR)\vncBuffer.obj" \
	"$(INTDIR)\vncClient.obj" \
	"$(INTDIR)\vncConnDialog.obj" \
	"$(INTDIR)\vncDesktop.obj" \
	"$(INTDIR)\vncEncoder.obj" \
	"$(INTDIR)\vncEncodeTight.obj" \
	"$(INTDIR)\VNCHelp.obj" \
	"$(INTDIR)\vncHTTPConnect.obj" \
	"$(INTDIR)\vncInstHandler.obj" \
	"$(INTDIR)\vncKeymap.obj" \
	"$(INTDIR)\vncMenu.obj" \
	"$(INTDIR)\vncProperties.obj" \
	"$(INTDIR)\vncRegion.obj" \
	"$(INTDIR)\vncServer.obj" \
	"$(INTDIR)\vncService.obj" \
	"$(INTDIR)\vncSockConnect.obj" \
	"$(INTDIR)\vncTimedMsgBox.obj" \
	"$(INTDIR)\VSocket.obj" \
	"$(INTDIR)\WallpaperUtils.obj" \
	"$(INTDIR)\WinVNC.obj" \
	"$(INTDIR)\WinVNC.res" \
	"$(OUTDIR)\VNCHooks.lib" \
	"$(OUTDIR)\omnithread.lib" \
	"$(OUTDIR)\zlib.lib"

"$(OUTDIR)\AppShare.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("WinVNC.dep")
!INCLUDE "WinVNC.dep"
!ELSE 
!MESSAGE Warning: cannot find "WinVNC.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "WinVNC - Win32 Release" || "$(CFG)" == "WinVNC - Win32 Debug" || "$(CFG)" == "WinVNC - Win32 Profile" || "$(CFG)" == "WinVNC - Win32 HorizonLive"
SOURCE=.\AdministrationControls.cpp

"$(INTDIR)\AdministrationControls.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\d3des.c

"$(INTDIR)\d3des.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\FileTransferItemInfo.cpp

"$(INTDIR)\FileTransferItemInfo.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\IncomingConnectionsControls.cpp

"$(INTDIR)\IncomingConnectionsControls.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\InputHandlingControls.cpp

"$(INTDIR)\InputHandlingControls.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Log.cpp

"$(INTDIR)\Log.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\MatchWindow.cpp

"$(INTDIR)\MatchWindow.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\MinMax.cpp

"$(INTDIR)\MinMax.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\PollControls.cpp

"$(INTDIR)\PollControls.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\QuerySettingsControls.cpp

"$(INTDIR)\QuerySettingsControls.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RectList.cpp

"$(INTDIR)\RectList.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\SharedDesktopArea.cpp

"$(INTDIR)\SharedDesktopArea.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\stdhdrs.cpp

"$(INTDIR)\stdhdrs.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tableinitcmtemplate.cpp
SOURCE=.\tableinittctemplate.cpp
SOURCE=.\tabletranstemplate.cpp
SOURCE=.\translate.cpp

"$(INTDIR)\translate.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\VideoDriver.cpp

"$(INTDIR)\VideoDriver.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vncAbout.cpp

"$(INTDIR)\vncAbout.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vncAcceptDialog.cpp

"$(INTDIR)\vncAcceptDialog.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vncAcceptReverseDlg.cpp

"$(INTDIR)\vncAcceptReverseDlg.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vncauth.c

"$(INTDIR)\vncauth.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vncBuffer.cpp

"$(INTDIR)\vncBuffer.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vncClient.cpp

"$(INTDIR)\vncClient.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vncConnDialog.cpp

"$(INTDIR)\vncConnDialog.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vncDesktop.cpp

"$(INTDIR)\vncDesktop.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vncEncoder.cpp

"$(INTDIR)\vncEncoder.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vncEncodeTight.cpp

"$(INTDIR)\vncEncodeTight.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\VNCHelp.cpp

"$(INTDIR)\VNCHelp.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vncHTTPConnect.cpp

"$(INTDIR)\vncHTTPConnect.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vncInstHandler.cpp

"$(INTDIR)\vncInstHandler.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vncKeymap.cpp

"$(INTDIR)\vncKeymap.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vncMenu.cpp

"$(INTDIR)\vncMenu.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vncProperties.cpp

"$(INTDIR)\vncProperties.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vncRegion.cpp

"$(INTDIR)\vncRegion.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vncServer.cpp

"$(INTDIR)\vncServer.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vncService.cpp

"$(INTDIR)\vncService.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vncSockConnect.cpp

"$(INTDIR)\vncSockConnect.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vncTimedMsgBox.cpp

"$(INTDIR)\vncTimedMsgBox.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\VSocket.cpp

"$(INTDIR)\VSocket.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\WallpaperUtils.cpp

"$(INTDIR)\WallpaperUtils.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\WinVNC.cpp

"$(INTDIR)\WinVNC.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\WinVNC.rc

"$(INTDIR)\WinVNC.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


!IF  "$(CFG)" == "WinVNC - Win32 Release"

"VNCHooks - Win32 Release" : 
   cd ".\VNCHooks"
   $(MAKE) /$(MAKEFLAGS) /F .\VNCHooks.mak CFG="VNCHooks - Win32 Release" 
   cd ".."

"VNCHooks - Win32 ReleaseCLEAN" : 
   cd ".\VNCHooks"
   $(MAKE) /$(MAKEFLAGS) /F .\VNCHooks.mak CFG="VNCHooks - Win32 Release" RECURSE=1 CLEAN 
   cd ".."

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"

"VNCHooks - Win32 Debug" : 
   cd ".\VNCHooks"
   $(MAKE) /$(MAKEFLAGS) /F .\VNCHooks.mak CFG="VNCHooks - Win32 Debug" 
   cd ".."

"VNCHooks - Win32 DebugCLEAN" : 
   cd ".\VNCHooks"
   $(MAKE) /$(MAKEFLAGS) /F .\VNCHooks.mak CFG="VNCHooks - Win32 Debug" RECURSE=1 CLEAN 
   cd ".."

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"

"VNCHooks - Win32 Profile" : 
   cd ".\VNCHooks"
   $(MAKE) /$(MAKEFLAGS) /F .\VNCHooks.mak CFG="VNCHooks - Win32 Profile" 
   cd ".."

"VNCHooks - Win32 ProfileCLEAN" : 
   cd ".\VNCHooks"
   $(MAKE) /$(MAKEFLAGS) /F .\VNCHooks.mak CFG="VNCHooks - Win32 Profile" RECURSE=1 CLEAN 
   cd ".."

!ELSEIF  "$(CFG)" == "WinVNC - Win32 HorizonLive"

"VNCHooks - Win32 HorizonLive" : 
   cd ".\VNCHooks"
   $(MAKE) /$(MAKEFLAGS) /F .\VNCHooks.mak CFG="VNCHooks - Win32 HorizonLive" 
   cd ".."

"VNCHooks - Win32 HorizonLiveCLEAN" : 
   cd ".\VNCHooks"
   $(MAKE) /$(MAKEFLAGS) /F .\VNCHooks.mak CFG="VNCHooks - Win32 HorizonLive" RECURSE=1 CLEAN 
   cd ".."

!ENDIF 

!IF  "$(CFG)" == "WinVNC - Win32 Release"

"omnithread - Win32 Release" : 
   cd ".\omnithread"
   $(MAKE) /$(MAKEFLAGS) /F .\omnithread.mak CFG="omnithread - Win32 Release" 
   cd ".."

"omnithread - Win32 ReleaseCLEAN" : 
   cd ".\omnithread"
   $(MAKE) /$(MAKEFLAGS) /F .\omnithread.mak CFG="omnithread - Win32 Release" RECURSE=1 CLEAN 
   cd ".."

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"

"omnithread - Win32 Debug" : 
   cd ".\omnithread"
   $(MAKE) /$(MAKEFLAGS) /F .\omnithread.mak CFG="omnithread - Win32 Debug" 
   cd ".."

"omnithread - Win32 DebugCLEAN" : 
   cd ".\omnithread"
   $(MAKE) /$(MAKEFLAGS) /F .\omnithread.mak CFG="omnithread - Win32 Debug" RECURSE=1 CLEAN 
   cd ".."

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"

"omnithread - Win32 Profile" : 
   cd ".\omnithread"
   $(MAKE) /$(MAKEFLAGS) /F .\omnithread.mak CFG="omnithread - Win32 Profile" 
   cd ".."

"omnithread - Win32 ProfileCLEAN" : 
   cd ".\omnithread"
   $(MAKE) /$(MAKEFLAGS) /F .\omnithread.mak CFG="omnithread - Win32 Profile" RECURSE=1 CLEAN 
   cd ".."

!ELSEIF  "$(CFG)" == "WinVNC - Win32 HorizonLive"

"omnithread - Win32 HorizonLive" : 
   cd ".\omnithread"
   $(MAKE) /$(MAKEFLAGS) /F .\omnithread.mak CFG="omnithread - Win32 HorizonLive" 
   cd ".."

"omnithread - Win32 HorizonLiveCLEAN" : 
   cd ".\omnithread"
   $(MAKE) /$(MAKEFLAGS) /F .\omnithread.mak CFG="omnithread - Win32 HorizonLive" RECURSE=1 CLEAN 
   cd ".."

!ENDIF 

!IF  "$(CFG)" == "WinVNC - Win32 Release"

"zlib - Win32 Release" : 
   cd ".\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 Release" 
   cd ".."

"zlib - Win32 ReleaseCLEAN" : 
   cd ".\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 Release" RECURSE=1 CLEAN 
   cd ".."

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"

"zlib - Win32 Debug" : 
   cd ".\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 Debug" 
   cd ".."

"zlib - Win32 DebugCLEAN" : 
   cd ".\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 Debug" RECURSE=1 CLEAN 
   cd ".."

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"

"zlib - Win32 Profile" : 
   cd ".\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 Profile" 
   cd ".."

"zlib - Win32 ProfileCLEAN" : 
   cd ".\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 Profile" RECURSE=1 CLEAN 
   cd ".."

!ELSEIF  "$(CFG)" == "WinVNC - Win32 HorizonLive"

"zlib - Win32 HorizonLive" : 
   cd ".\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 HorizonLive" 
   cd ".."

"zlib - Win32 HorizonLiveCLEAN" : 
   cd ".\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 HorizonLive" RECURSE=1 CLEAN 
   cd ".."

!ENDIF 


!ENDIF 

