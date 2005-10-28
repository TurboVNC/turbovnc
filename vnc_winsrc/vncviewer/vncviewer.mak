# Microsoft Developer Studio Generated NMAKE File, Based on vncviewer.dsp
!IF "$(CFG)" == ""
CFG=vncviewer - Win32 Release
!MESSAGE No configuration specified. Defaulting to vncviewer - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "vncviewer - Win32 Release" && "$(CFG)" != "vncviewer - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "vncviewer - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\vncviewer.exe"

!ELSE 

ALL : "zlib - Win32 Release" "omnithread - Win32 Release" "$(OUTDIR)\vncviewer.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"omnithread - Win32 ReleaseCLEAN" "zlib - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\AboutBox.obj"
	-@erase "$(INTDIR)\AuthDialog.obj"
	-@erase "$(INTDIR)\CapsContainer.obj"
	-@erase "$(INTDIR)\ClientConnection.obj"
	-@erase "$(INTDIR)\ClientConnectionClipboard.obj"
	-@erase "$(INTDIR)\ClientConnectionCopyRect.obj"
	-@erase "$(INTDIR)\ClientConnectionCursor.obj"
	-@erase "$(INTDIR)\ClientConnectionFile.obj"
	-@erase "$(INTDIR)\ClientConnectionFullScreen.obj"
	-@erase "$(INTDIR)\ClientConnectionTight.obj"
	-@erase "$(INTDIR)\ConnectingDialog.obj"
	-@erase "$(INTDIR)\d3des.obj"
	-@erase "$(INTDIR)\Daemon.obj"
	-@erase "$(INTDIR)\Exception.obj"
	-@erase "$(INTDIR)\FileTransfer.obj"
	-@erase "$(INTDIR)\FileTransferItemInfo.obj"
	-@erase "$(INTDIR)\Flasher.obj"
	-@erase "$(INTDIR)\HotKeys.obj"
	-@erase "$(INTDIR)\KeyMap.obj"
	-@erase "$(INTDIR)\Log.obj"
	-@erase "$(INTDIR)\LoginAuthDialog.obj"
	-@erase "$(INTDIR)\SessionDialog.obj"
	-@erase "$(INTDIR)\stdhdrs.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vncauth.obj"
	-@erase "$(INTDIR)\VNCHelp.obj"
	-@erase "$(INTDIR)\VNCOptions.obj"
	-@erase "$(INTDIR)\vncviewer.obj"
	-@erase "$(INTDIR)\vncviewer.res"
	-@erase "$(INTDIR)\VNCviewerApp.obj"
	-@erase "$(INTDIR)\VNCviewerApp32.obj"
	-@erase "$(OUTDIR)\vncviewer.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /Ob0 /I "omnithread" /I ".." /I "..\..\..\vgl\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__NT__" /D "_WINSTATIC" /D "__WIN32__" /D "XMD_H" /Fp"$(INTDIR)\vncviewer.pch" /YX"stdhdrs.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\vncviewer.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\vncviewer.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib wsock32.lib comctl32.lib htmlhelp.lib turbojpeg.lib fbx.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\vncviewer.pdb" /machine:I386 /out:"$(OUTDIR)\vncviewer.exe" /libpath:"..\..\..\vgl\windows\vnc\lib" 
LINK32_OBJS= \
	"$(INTDIR)\AboutBox.obj" \
	"$(INTDIR)\AuthDialog.obj" \
	"$(INTDIR)\CapsContainer.obj" \
	"$(INTDIR)\ClientConnection.obj" \
	"$(INTDIR)\ClientConnectionClipboard.obj" \
	"$(INTDIR)\ClientConnectionCopyRect.obj" \
	"$(INTDIR)\ClientConnectionCursor.obj" \
	"$(INTDIR)\ClientConnectionFile.obj" \
	"$(INTDIR)\ClientConnectionFullScreen.obj" \
	"$(INTDIR)\ClientConnectionTight.obj" \
	"$(INTDIR)\ConnectingDialog.obj" \
	"$(INTDIR)\d3des.obj" \
	"$(INTDIR)\Daemon.obj" \
	"$(INTDIR)\Exception.obj" \
	"$(INTDIR)\FileTransfer.obj" \
	"$(INTDIR)\FileTransferItemInfo.obj" \
	"$(INTDIR)\Flasher.obj" \
	"$(INTDIR)\HotKeys.obj" \
	"$(INTDIR)\KeyMap.obj" \
	"$(INTDIR)\Log.obj" \
	"$(INTDIR)\LoginAuthDialog.obj" \
	"$(INTDIR)\SessionDialog.obj" \
	"$(INTDIR)\stdhdrs.obj" \
	"$(INTDIR)\vncauth.obj" \
	"$(INTDIR)\VNCHelp.obj" \
	"$(INTDIR)\VNCOptions.obj" \
	"$(INTDIR)\vncviewer.obj" \
	"$(INTDIR)\VNCviewerApp.obj" \
	"$(INTDIR)\VNCviewerApp32.obj" \
	"$(INTDIR)\vncviewer.res" \
	"$(OUTDIR)\omnithread.lib" \
	"$(OUTDIR)\zlib.lib"

"$(OUTDIR)\vncviewer.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "vncviewer - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\vncviewer.exe" "$(OUTDIR)\vncviewer.pch"

!ELSE 

ALL : "zlib - Win32 Debug" "omnithread - Win32 Debug" "$(OUTDIR)\vncviewer.exe" "$(OUTDIR)\vncviewer.pch"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"omnithread - Win32 DebugCLEAN" "zlib - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\AboutBox.obj"
	-@erase "$(INTDIR)\AuthDialog.obj"
	-@erase "$(INTDIR)\CapsContainer.obj"
	-@erase "$(INTDIR)\ClientConnection.obj"
	-@erase "$(INTDIR)\ClientConnectionClipboard.obj"
	-@erase "$(INTDIR)\ClientConnectionCopyRect.obj"
	-@erase "$(INTDIR)\ClientConnectionCursor.obj"
	-@erase "$(INTDIR)\ClientConnectionFile.obj"
	-@erase "$(INTDIR)\ClientConnectionFullScreen.obj"
	-@erase "$(INTDIR)\ClientConnectionTight.obj"
	-@erase "$(INTDIR)\ConnectingDialog.obj"
	-@erase "$(INTDIR)\d3des.obj"
	-@erase "$(INTDIR)\Daemon.obj"
	-@erase "$(INTDIR)\Exception.obj"
	-@erase "$(INTDIR)\FileTransfer.obj"
	-@erase "$(INTDIR)\FileTransferItemInfo.obj"
	-@erase "$(INTDIR)\Flasher.obj"
	-@erase "$(INTDIR)\HotKeys.obj"
	-@erase "$(INTDIR)\KeyMap.obj"
	-@erase "$(INTDIR)\Log.obj"
	-@erase "$(INTDIR)\LoginAuthDialog.obj"
	-@erase "$(INTDIR)\SessionDialog.obj"
	-@erase "$(INTDIR)\stdhdrs.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vncauth.obj"
	-@erase "$(INTDIR)\VNCHelp.obj"
	-@erase "$(INTDIR)\VNCOptions.obj"
	-@erase "$(INTDIR)\vncviewer.obj"
	-@erase "$(INTDIR)\vncviewer.pch"
	-@erase "$(INTDIR)\vncviewer.res"
	-@erase "$(INTDIR)\VNCviewerApp.obj"
	-@erase "$(INTDIR)\VNCviewerApp32.obj"
	-@erase "$(OUTDIR)\vncviewer.exe"
	-@erase "$(OUTDIR)\vncviewer.ilk"
	-@erase "$(OUTDIR)\vncviewer.map"
	-@erase "$(OUTDIR)\vncviewer.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "omnithread" /I ".." /I "..\..\..\vgl\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "__NT__" /D "_WINSTATIC" /D "__WIN32__" /D "XMD_H" /Fp"$(INTDIR)\vncviewer.pch" /YX"stdhdrs.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\vncviewer.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\vncviewer.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib wsock32.lib comctl32.lib htmlhelp.lib turbojpeg.lib fbx.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\vncviewer.pdb" /map:"$(INTDIR)\vncviewer.map" /debug /machine:I386 /nodefaultlib:"libcd" /out:"$(OUTDIR)\vncviewer.exe" /pdbtype:sept /libpath:"..\..\..\vgl\windows\vnc\dbg\lib" 
LINK32_OBJS= \
	"$(INTDIR)\AboutBox.obj" \
	"$(INTDIR)\AuthDialog.obj" \
	"$(INTDIR)\CapsContainer.obj" \
	"$(INTDIR)\ClientConnection.obj" \
	"$(INTDIR)\ClientConnectionClipboard.obj" \
	"$(INTDIR)\ClientConnectionCopyRect.obj" \
	"$(INTDIR)\ClientConnectionCursor.obj" \
	"$(INTDIR)\ClientConnectionFile.obj" \
	"$(INTDIR)\ClientConnectionFullScreen.obj" \
	"$(INTDIR)\ClientConnectionTight.obj" \
	"$(INTDIR)\ConnectingDialog.obj" \
	"$(INTDIR)\d3des.obj" \
	"$(INTDIR)\Daemon.obj" \
	"$(INTDIR)\Exception.obj" \
	"$(INTDIR)\FileTransfer.obj" \
	"$(INTDIR)\FileTransferItemInfo.obj" \
	"$(INTDIR)\Flasher.obj" \
	"$(INTDIR)\HotKeys.obj" \
	"$(INTDIR)\KeyMap.obj" \
	"$(INTDIR)\Log.obj" \
	"$(INTDIR)\LoginAuthDialog.obj" \
	"$(INTDIR)\SessionDialog.obj" \
	"$(INTDIR)\stdhdrs.obj" \
	"$(INTDIR)\vncauth.obj" \
	"$(INTDIR)\VNCHelp.obj" \
	"$(INTDIR)\VNCOptions.obj" \
	"$(INTDIR)\vncviewer.obj" \
	"$(INTDIR)\VNCviewerApp.obj" \
	"$(INTDIR)\VNCviewerApp32.obj" \
	"$(INTDIR)\vncviewer.res" \
	"$(OUTDIR)\omnithread.lib" \
	"$(OUTDIR)\zlib.lib"

"$(OUTDIR)\vncviewer.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

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


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("vncviewer.dep")
!INCLUDE "vncviewer.dep"
!ELSE 
!MESSAGE Warning: cannot find "vncviewer.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "vncviewer - Win32 Release" || "$(CFG)" == "vncviewer - Win32 Debug"

!IF  "$(CFG)" == "vncviewer - Win32 Release"

"omnithread - Win32 Release" : 
   cd ".\omnithread"
   $(MAKE) /$(MAKEFLAGS) /F .\omnithread.mak CFG="omnithread - Win32 Release" 
   cd ".."

"omnithread - Win32 ReleaseCLEAN" : 
   cd ".\omnithread"
   $(MAKE) /$(MAKEFLAGS) /F .\omnithread.mak CFG="omnithread - Win32 Release" RECURSE=1 CLEAN 
   cd ".."

!ELSEIF  "$(CFG)" == "vncviewer - Win32 Debug"

"omnithread - Win32 Debug" : 
   cd ".\omnithread"
   $(MAKE) /$(MAKEFLAGS) /F .\omnithread.mak CFG="omnithread - Win32 Debug" 
   cd ".."

"omnithread - Win32 DebugCLEAN" : 
   cd ".\omnithread"
   $(MAKE) /$(MAKEFLAGS) /F .\omnithread.mak CFG="omnithread - Win32 Debug" RECURSE=1 CLEAN 
   cd ".."

!ENDIF 

!IF  "$(CFG)" == "vncviewer - Win32 Release"

"zlib - Win32 Release" : 
   cd ".\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 Release" 
   cd ".."

"zlib - Win32 ReleaseCLEAN" : 
   cd ".\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 Release" RECURSE=1 CLEAN 
   cd ".."

!ELSEIF  "$(CFG)" == "vncviewer - Win32 Debug"

"zlib - Win32 Debug" : 
   cd ".\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 Debug" 
   cd ".."

"zlib - Win32 DebugCLEAN" : 
   cd ".\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 Debug" RECURSE=1 CLEAN 
   cd ".."

!ENDIF 

!IF  "$(CFG)" == "vncviewer - Win32 Release"

!ELSEIF  "$(CFG)" == "vncviewer - Win32 Debug"

!ENDIF 

SOURCE=.\AboutBox.cpp

"$(INTDIR)\AboutBox.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\AuthDialog.cpp

"$(INTDIR)\AuthDialog.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\CapsContainer.cpp

"$(INTDIR)\CapsContainer.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ClientConnection.cpp

"$(INTDIR)\ClientConnection.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ClientConnectionClipboard.cpp

"$(INTDIR)\ClientConnectionClipboard.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ClientConnectionCopyRect.cpp

"$(INTDIR)\ClientConnectionCopyRect.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ClientConnectionCursor.cpp

"$(INTDIR)\ClientConnectionCursor.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ClientConnectionFile.cpp

"$(INTDIR)\ClientConnectionFile.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ClientConnectionFullScreen.cpp

"$(INTDIR)\ClientConnectionFullScreen.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ClientConnectionTight.cpp

"$(INTDIR)\ClientConnectionTight.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ConnectingDialog.cpp

"$(INTDIR)\ConnectingDialog.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\d3des.c

"$(INTDIR)\d3des.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Daemon.cpp

"$(INTDIR)\Daemon.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Exception.cpp

"$(INTDIR)\Exception.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\FileTransfer.cpp

"$(INTDIR)\FileTransfer.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\FileTransferItemInfo.cpp

"$(INTDIR)\FileTransferItemInfo.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Flasher.cpp

"$(INTDIR)\Flasher.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\HotKeys.cpp

"$(INTDIR)\HotKeys.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\KeyMap.cpp

"$(INTDIR)\KeyMap.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Log.cpp

"$(INTDIR)\Log.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\LoginAuthDialog.cpp

"$(INTDIR)\LoginAuthDialog.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\SessionDialog.cpp

"$(INTDIR)\SessionDialog.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\stdhdrs.cpp

!IF  "$(CFG)" == "vncviewer - Win32 Release"

CPP_SWITCHES=/nologo /MT /W3 /GX /O2 /Ob0 /I "omnithread" /I ".." /I "..\..\..\vgl\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__NT__" /D "_WINSTATIC" /D "__WIN32__" /D "XMD_H" /Fp"$(INTDIR)\vncviewer.pch" /YX"stdhdrs.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\stdhdrs.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "vncviewer - Win32 Debug"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "omnithread" /I ".." /I "..\..\..\vgl\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "__NT__" /D "_WINSTATIC" /D "__WIN32__" /D "XMD_H" /Fp"$(INTDIR)\vncviewer.pch" /Yc"stdhdrs.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\stdhdrs.obj"	"$(INTDIR)\vncviewer.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\vncauth.c

"$(INTDIR)\vncauth.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\VNCHelp.cpp

"$(INTDIR)\VNCHelp.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\VNCOptions.cpp

"$(INTDIR)\VNCOptions.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vncviewer.cpp

"$(INTDIR)\vncviewer.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\res\vncviewer.rc

!IF  "$(CFG)" == "vncviewer - Win32 Release"


"$(INTDIR)\vncviewer.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\vncviewer.res" /i "res" /d "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "vncviewer - Win32 Debug"


"$(INTDIR)\vncviewer.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\vncviewer.res" /i "res" /d "_DEBUG" $(SOURCE)


!ENDIF 

SOURCE=.\VNCviewerApp.cpp

"$(INTDIR)\VNCviewerApp.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\VNCviewerApp32.cpp

"$(INTDIR)\VNCviewerApp32.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

