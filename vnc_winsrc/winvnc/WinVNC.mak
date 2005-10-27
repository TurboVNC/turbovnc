# Microsoft Developer Studio Generated NMAKE File, Based on WinVNC.dsp
!IF "$(CFG)" == ""
CFG=WinVNC - Win32 No_CORBA
!MESSAGE No configuration specified. Defaulting to WinVNC - Win32 No_CORBA.
!ENDIF 

!IF "$(CFG)" != "WinVNC - Win32 Release" && "$(CFG)" != "WinVNC - Win32 Debug" && "$(CFG)" != "WinVNC - Win32 Purify" && "$(CFG)" != "WinVNC - Win32 No_CORBA" && "$(CFG)" != "WinVNC - Win32 Profile" && "$(CFG)" != "WinVNC - Win32 Alpha No_CORBA" && "$(CFG)" != "WinVNC - Win32 Alpha Debug No_CORBA"
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
!MESSAGE "WinVNC - Win32 Purify" (based on "Win32 (x86) Application")
!MESSAGE "WinVNC - Win32 No_CORBA" (based on "Win32 (x86) Application")
!MESSAGE "WinVNC - Win32 Profile" (based on "Win32 (x86) Application")
!MESSAGE "WinVNC - Win32 Alpha No_CORBA" (based on "Win32 (ALPHA) Application")
!MESSAGE "WinVNC - Win32 Alpha Debug No_CORBA" (based on "Win32 (ALPHA) Application")
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
	-@erase "$(INTDIR)\d3des.obj"
	-@erase "$(INTDIR)\Log.obj"
	-@erase "$(INTDIR)\MinMax.obj"
	-@erase "$(INTDIR)\RectList.obj"
	-@erase "$(INTDIR)\stdhdrs.obj"
	-@erase "$(INTDIR)\translate.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vncAbout.obj"
	-@erase "$(INTDIR)\vncAcceptDialog.obj"
	-@erase "$(INTDIR)\vncAdvancedProperties.obj"
	-@erase "$(INTDIR)\vncauth.obj"
	-@erase "$(INTDIR)\vncBuffer.obj"
	-@erase "$(INTDIR)\vncClient.obj"
	-@erase "$(INTDIR)\vncConnDialog.obj"
	-@erase "$(INTDIR)\vncDesktop.obj"
	-@erase "$(INTDIR)\vncEncoder.obj"
	-@erase "$(INTDIR)\vncEncodeTight.obj"
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
	-@erase "$(INTDIR)\WinVNC.obj"
	-@erase "$(INTDIR)\WinVNC.res"
	-@erase "$(OUTDIR)\WinVNC.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "./omnithread" /I "\\shallot\omni\release\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "__WIN32__" /D "__NT__" /D "__x86__" /D "_CORBA" /Fp"$(INTDIR)\WinVNC.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\WinVNC.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\WinVNC.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib omniORB270_rt.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\WinVNC.pdb" /machine:I386 /out:"$(OUTDIR)\WinVNC.exe" /libpath:"\\shallot\omni\release\lib\x86_nt_3.5" /libpath:"\\shallot\omni\release\lib\x86_nt_4.0" 
LINK32_OBJS= \
	"$(INTDIR)\d3des.obj" \
	"$(INTDIR)\Log.obj" \
	"$(INTDIR)\MinMax.obj" \
	"$(INTDIR)\RectList.obj" \
	"$(INTDIR)\stdhdrs.obj" \
	"$(INTDIR)\translate.obj" \
	"$(INTDIR)\vncAbout.obj" \
	"$(INTDIR)\vncAcceptDialog.obj" \
	"$(INTDIR)\vncAdvancedProperties.obj" \
	"$(INTDIR)\vncauth.obj" \
	"$(INTDIR)\vncBuffer.obj" \
	"$(INTDIR)\vncClient.obj" \
	"$(INTDIR)\vncConnDialog.obj" \
	"$(INTDIR)\vncDesktop.obj" \
	"$(INTDIR)\vncEncoder.obj" \
	"$(INTDIR)\vncEncodeTight.obj" \
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
	"$(INTDIR)\WinVNC.obj" \
	"$(INTDIR)\WinVNC.res" \
	"$(OUTDIR)\VNCHooks.lib" \
	"$(OUTDIR)\omnithread_rt.lib" \
	".\zlib\Release\zlib.lib"

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
	-@erase "$(INTDIR)\d3des.obj"
	-@erase "$(INTDIR)\Log.obj"
	-@erase "$(INTDIR)\MinMax.obj"
	-@erase "$(INTDIR)\RectList.obj"
	-@erase "$(INTDIR)\stdhdrs.obj"
	-@erase "$(INTDIR)\translate.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vncAbout.obj"
	-@erase "$(INTDIR)\vncAcceptDialog.obj"
	-@erase "$(INTDIR)\vncAdvancedProperties.obj"
	-@erase "$(INTDIR)\vncauth.obj"
	-@erase "$(INTDIR)\vncBuffer.obj"
	-@erase "$(INTDIR)\vncClient.obj"
	-@erase "$(INTDIR)\vncConnDialog.obj"
	-@erase "$(INTDIR)\vncDesktop.obj"
	-@erase "$(INTDIR)\vncEncoder.obj"
	-@erase "$(INTDIR)\vncEncodeTight.obj"
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
	-@erase "$(INTDIR)\WinVNC.obj"
	-@erase "$(INTDIR)\WinVNC.res"
	-@erase "$(OUTDIR)\WinVNC.exe"
	-@erase "$(OUTDIR)\WinVNC.map"
	-@erase "$(OUTDIR)\WinVNC.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "./omnithread" /I "\\shallot\omni\release\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "__WIN32__" /D "__NT__" /D "__x86__" /D "NCORBA" /D "ZLIB_DLL" /Fp"$(INTDIR)\WinVNC.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\WinVNC.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\WinVNC.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\WinVNC.pdb" /map:"$(INTDIR)\WinVNC.map" /debug /machine:I386 /out:"$(OUTDIR)\WinVNC.exe" /pdbtype:sept /libpath:"\\shallot\omni\release\lib\x86_nt_3.5" 
LINK32_OBJS= \
	"$(INTDIR)\d3des.obj" \
	"$(INTDIR)\Log.obj" \
	"$(INTDIR)\MinMax.obj" \
	"$(INTDIR)\RectList.obj" \
	"$(INTDIR)\stdhdrs.obj" \
	"$(INTDIR)\translate.obj" \
	"$(INTDIR)\vncAbout.obj" \
	"$(INTDIR)\vncAcceptDialog.obj" \
	"$(INTDIR)\vncAdvancedProperties.obj" \
	"$(INTDIR)\vncauth.obj" \
	"$(INTDIR)\vncBuffer.obj" \
	"$(INTDIR)\vncClient.obj" \
	"$(INTDIR)\vncConnDialog.obj" \
	"$(INTDIR)\vncDesktop.obj" \
	"$(INTDIR)\vncEncoder.obj" \
	"$(INTDIR)\vncEncodeTight.obj" \
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
	"$(INTDIR)\WinVNC.obj" \
	"$(INTDIR)\WinVNC.res" \
	"$(OUTDIR)\VNCHooks.lib" \
	"$(OUTDIR)\omnithread_rtd.lib" \
	"$(OUTDIR)\zlib.lib"

"$(OUTDIR)\WinVNC.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"

OUTDIR=.\Purify
INTDIR=.\Purify
# Begin Custom Macros
OutDir=.\Purify
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\WinVNC.exe"

!ELSE 

ALL : "omnithread - Win32 Purify" "VNCHooks - Win32 Purify" "$(OUTDIR)\WinVNC.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"VNCHooks - Win32 PurifyCLEAN" "omnithread - Win32 PurifyCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\d3des.obj"
	-@erase "$(INTDIR)\Log.obj"
	-@erase "$(INTDIR)\MinMax.obj"
	-@erase "$(INTDIR)\RectList.obj"
	-@erase "$(INTDIR)\stdhdrs.obj"
	-@erase "$(INTDIR)\translate.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vncAbout.obj"
	-@erase "$(INTDIR)\vncAcceptDialog.obj"
	-@erase "$(INTDIR)\vncAdvancedProperties.obj"
	-@erase "$(INTDIR)\vncauth.obj"
	-@erase "$(INTDIR)\vncBuffer.obj"
	-@erase "$(INTDIR)\vncClient.obj"
	-@erase "$(INTDIR)\vncConnDialog.obj"
	-@erase "$(INTDIR)\vncDesktop.obj"
	-@erase "$(INTDIR)\vncEncoder.obj"
	-@erase "$(INTDIR)\vncEncodeTight.obj"
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
	-@erase "$(INTDIR)\WinVNC.obj"
	-@erase "$(INTDIR)\WinVNC.res"
	-@erase "$(OUTDIR)\WinVNC.exe"
	-@erase "$(OUTDIR)\WinVNC.ilk"
	-@erase "$(OUTDIR)\WinVNC.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "./omnithread" /I "\\shallot\omni\release\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "__WIN32__" /D "__NT__" /D "__x86__" /D "_CORBA" /Fp"$(INTDIR)\WinVNC.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\WinVNC.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\WinVNC.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=omniORB260_rtd.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\WinVNC.pdb" /debug /machine:I386 /out:"$(OUTDIR)\WinVNC.exe" /pdbtype:sept /libpath:"\\shallot\omni\release\lib\x86_nt_3.5" 
LINK32_OBJS= \
	"$(INTDIR)\d3des.obj" \
	"$(INTDIR)\Log.obj" \
	"$(INTDIR)\MinMax.obj" \
	"$(INTDIR)\RectList.obj" \
	"$(INTDIR)\stdhdrs.obj" \
	"$(INTDIR)\translate.obj" \
	"$(INTDIR)\vncAbout.obj" \
	"$(INTDIR)\vncAcceptDialog.obj" \
	"$(INTDIR)\vncAdvancedProperties.obj" \
	"$(INTDIR)\vncauth.obj" \
	"$(INTDIR)\vncBuffer.obj" \
	"$(INTDIR)\vncClient.obj" \
	"$(INTDIR)\vncConnDialog.obj" \
	"$(INTDIR)\vncDesktop.obj" \
	"$(INTDIR)\vncEncoder.obj" \
	"$(INTDIR)\vncEncodeTight.obj" \
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
	"$(INTDIR)\WinVNC.obj" \
	"$(INTDIR)\WinVNC.res" \
	"$(OUTDIR)\VNCHooks.lib" \
	"$(OUTDIR)\omnithread_rtd.lib"

"$(OUTDIR)\WinVNC.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"

OUTDIR=.\No_CORBA
INTDIR=.\No_CORBA
# Begin Custom Macros
OutDir=.\No_CORBA
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\WinVNC.exe"

!ELSE 

ALL : "zlib - Win32 No_CORBA" "omnithread - Win32 No_CORBA" "VNCHooks - Win32 No_CORBA" "$(OUTDIR)\WinVNC.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"VNCHooks - Win32 No_CORBACLEAN" "omnithread - Win32 No_CORBACLEAN" "zlib - Win32 No_CORBACLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\d3des.obj"
	-@erase "$(INTDIR)\Log.obj"
	-@erase "$(INTDIR)\MinMax.obj"
	-@erase "$(INTDIR)\RectList.obj"
	-@erase "$(INTDIR)\stdhdrs.obj"
	-@erase "$(INTDIR)\translate.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vncAbout.obj"
	-@erase "$(INTDIR)\vncAcceptDialog.obj"
	-@erase "$(INTDIR)\vncAdvancedProperties.obj"
	-@erase "$(INTDIR)\vncauth.obj"
	-@erase "$(INTDIR)\vncBuffer.obj"
	-@erase "$(INTDIR)\vncClient.obj"
	-@erase "$(INTDIR)\vncConnDialog.obj"
	-@erase "$(INTDIR)\vncDesktop.obj"
	-@erase "$(INTDIR)\vncEncoder.obj"
	-@erase "$(INTDIR)\vncEncodeTight.obj"
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
	-@erase "$(INTDIR)\WinVNC.obj"
	-@erase "$(INTDIR)\WinVNC.res"
	-@erase "$(OUTDIR)\WinVNC.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "./omnithread" /I "./zlib" /I "../../../vgl/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "__WIN32__" /D "__NT__" /D "__x86__" /D "_WINSTATIC" /D "NCORBA" /D "ZLIB_DLL" /D "XMD_H" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
LINK32_FLAGS=wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib turbojpeg.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\WinVNC.pdb" /machine:I386 /nodefaultlib:"LIBC" /out:"$(OUTDIR)\WinVNC.exe" /libpath:"../../../vgl/windows/vnc/lib" 
LINK32_OBJS= \
	"$(INTDIR)\d3des.obj" \
	"$(INTDIR)\Log.obj" \
	"$(INTDIR)\MinMax.obj" \
	"$(INTDIR)\RectList.obj" \
	"$(INTDIR)\stdhdrs.obj" \
	"$(INTDIR)\translate.obj" \
	"$(INTDIR)\vncAbout.obj" \
	"$(INTDIR)\vncAcceptDialog.obj" \
	"$(INTDIR)\vncAdvancedProperties.obj" \
	"$(INTDIR)\vncauth.obj" \
	"$(INTDIR)\vncBuffer.obj" \
	"$(INTDIR)\vncClient.obj" \
	"$(INTDIR)\vncConnDialog.obj" \
	"$(INTDIR)\vncDesktop.obj" \
	"$(INTDIR)\vncEncoder.obj" \
	"$(INTDIR)\vncEncodeTight.obj" \
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
	"$(INTDIR)\WinVNC.obj" \
	"$(INTDIR)\WinVNC.res" \
	"$(OUTDIR)\VNCHooks.lib" \
	"$(OUTDIR)\omnithread_rt.lib" \
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

ALL : "omnithread - Win32 Profile" "VNCHooks - Win32 Profile" "$(OUTDIR)\WinVNC.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"VNCHooks - Win32 ProfileCLEAN" "omnithread - Win32 ProfileCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\d3des.obj"
	-@erase "$(INTDIR)\Log.obj"
	-@erase "$(INTDIR)\MinMax.obj"
	-@erase "$(INTDIR)\RectList.obj"
	-@erase "$(INTDIR)\stdhdrs.obj"
	-@erase "$(INTDIR)\translate.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vncAbout.obj"
	-@erase "$(INTDIR)\vncAcceptDialog.obj"
	-@erase "$(INTDIR)\vncAdvancedProperties.obj"
	-@erase "$(INTDIR)\vncauth.obj"
	-@erase "$(INTDIR)\vncBuffer.obj"
	-@erase "$(INTDIR)\vncClient.obj"
	-@erase "$(INTDIR)\vncConnDialog.obj"
	-@erase "$(INTDIR)\vncDesktop.obj"
	-@erase "$(INTDIR)\vncEncoder.obj"
	-@erase "$(INTDIR)\vncEncodeTight.obj"
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
	-@erase "$(INTDIR)\WinVNC.obj"
	-@erase "$(INTDIR)\WinVNC.res"
	-@erase "$(OUTDIR)\WinVNC.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "./omnithread" /I "\\shallot\omni\release\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "__WIN32__" /D "__NT__" /D "__x86__" /D "_CORBA" /Fp"$(INTDIR)\WinVNC.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\WinVNC.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\WinVNC.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=omniORB260_rtd.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /profile /debug /machine:I386 /out:"$(OUTDIR)\WinVNC.exe" /libpath:"\\shallot\omni\release\lib\x86_nt_3.5" 
LINK32_OBJS= \
	"$(INTDIR)\d3des.obj" \
	"$(INTDIR)\Log.obj" \
	"$(INTDIR)\MinMax.obj" \
	"$(INTDIR)\RectList.obj" \
	"$(INTDIR)\stdhdrs.obj" \
	"$(INTDIR)\translate.obj" \
	"$(INTDIR)\vncAbout.obj" \
	"$(INTDIR)\vncAcceptDialog.obj" \
	"$(INTDIR)\vncAdvancedProperties.obj" \
	"$(INTDIR)\vncauth.obj" \
	"$(INTDIR)\vncBuffer.obj" \
	"$(INTDIR)\vncClient.obj" \
	"$(INTDIR)\vncConnDialog.obj" \
	"$(INTDIR)\vncDesktop.obj" \
	"$(INTDIR)\vncEncoder.obj" \
	"$(INTDIR)\vncEncodeTight.obj" \
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
	"$(INTDIR)\WinVNC.obj" \
	"$(INTDIR)\WinVNC.res" \
	"$(OUTDIR)\VNCHooks.lib" \
	"$(OUTDIR)\omnithread_rtd.lib"

"$(OUTDIR)\WinVNC.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

OUTDIR=.\Alpha_No_CORBA
INTDIR=.\Alpha_No_CORBA
# Begin Custom Macros
OutDir=.\Alpha_No_CORBA
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\WinVNC.exe"

!ELSE 

ALL : "omnithread - Win32 Alpha No_CORBA" "VNCHooks - Win32 Alpha No_CORBA" "$(OUTDIR)\WinVNC.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"VNCHooks - Win32 Alpha No_CORBACLEAN" "omnithread - Win32 Alpha No_CORBACLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\WinVNC.res"
	-@erase "$(OUTDIR)\WinVNC.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\WinVNC.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\WinVNC.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\WinVNC.pdb" /machine:ALPHA /out:"$(OUTDIR)\WinVNC.exe" 
LINK32_OBJS= \
	"$(INTDIR)\WinVNC.res" \
	"$(OUTDIR)\VNCHooks.lib"

"$(OUTDIR)\WinVNC.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

OUTDIR=.\AlphaDbg_No_CORBA
INTDIR=.\AlphaDbg_No_CORBA
# Begin Custom Macros
OutDir=.\AlphaDbg_No_CORBA
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\WinVNC.exe"

!ELSE 

ALL : "omnithread - Win32 Alpha Debug No_CORBA" "VNCHooks - Win32 Alpha Debug No_CORBA" "$(OUTDIR)\WinVNC.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"VNCHooks - Win32 Alpha Debug No_CORBACLEAN" "omnithread - Win32 Alpha Debug No_CORBACLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\WinVNC.res"
	-@erase "$(OUTDIR)\WinVNC.exe"
	-@erase "$(OUTDIR)\WinVNC.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\WinVNC.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\WinVNC.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\WinVNC.pdb" /debug /machine:ALPHA /out:"$(OUTDIR)\WinVNC.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\WinVNC.res" \
	"$(OUTDIR)\VNCHooks.lib"

"$(OUTDIR)\WinVNC.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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


!IF "$(CFG)" == "WinVNC - Win32 Release" || "$(CFG)" == "WinVNC - Win32 Debug" || "$(CFG)" == "WinVNC - Win32 Purify" || "$(CFG)" == "WinVNC - Win32 No_CORBA" || "$(CFG)" == "WinVNC - Win32 Profile" || "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA" || "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"
SOURCE=.\d3des.c

!IF  "$(CFG)" == "WinVNC - Win32 Release"


"$(INTDIR)\d3des.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"


"$(INTDIR)\d3des.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"


"$(INTDIR)\d3des.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"


"$(INTDIR)\d3des.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"


"$(INTDIR)\d3des.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

!ENDIF 

SOURCE=.\Log.cpp

!IF  "$(CFG)" == "WinVNC - Win32 Release"


"$(INTDIR)\Log.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"


"$(INTDIR)\Log.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"


"$(INTDIR)\Log.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"


"$(INTDIR)\Log.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"


"$(INTDIR)\Log.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

!ENDIF 

SOURCE=.\MinMax.cpp

!IF  "$(CFG)" == "WinVNC - Win32 Release"


"$(INTDIR)\MinMax.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"


"$(INTDIR)\MinMax.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"


"$(INTDIR)\MinMax.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"


"$(INTDIR)\MinMax.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"


"$(INTDIR)\MinMax.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

!ENDIF 

SOURCE=.\RectList.cpp

!IF  "$(CFG)" == "WinVNC - Win32 Release"


"$(INTDIR)\RectList.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"


"$(INTDIR)\RectList.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"


"$(INTDIR)\RectList.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"


"$(INTDIR)\RectList.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"


"$(INTDIR)\RectList.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

!ENDIF 

SOURCE=.\stdhdrs.cpp

!IF  "$(CFG)" == "WinVNC - Win32 Release"


"$(INTDIR)\stdhdrs.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"


"$(INTDIR)\stdhdrs.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"


"$(INTDIR)\stdhdrs.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"


"$(INTDIR)\stdhdrs.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"


"$(INTDIR)\stdhdrs.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

!ENDIF 

SOURCE=.\tableinitcmtemplate.cpp
SOURCE=.\tableinittctemplate.cpp
SOURCE=.\tabletranstemplate.cpp
SOURCE=.\translate.cpp

!IF  "$(CFG)" == "WinVNC - Win32 Release"


"$(INTDIR)\translate.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"


"$(INTDIR)\translate.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"


"$(INTDIR)\translate.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"


"$(INTDIR)\translate.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"


"$(INTDIR)\translate.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

!ENDIF 

SOURCE=.\vncAbout.cpp

!IF  "$(CFG)" == "WinVNC - Win32 Release"


"$(INTDIR)\vncAbout.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"


"$(INTDIR)\vncAbout.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"


"$(INTDIR)\vncAbout.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"


"$(INTDIR)\vncAbout.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"


"$(INTDIR)\vncAbout.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

!ENDIF 

SOURCE=.\vncAcceptDialog.cpp

!IF  "$(CFG)" == "WinVNC - Win32 Release"


"$(INTDIR)\vncAcceptDialog.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"


"$(INTDIR)\vncAcceptDialog.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"


"$(INTDIR)\vncAcceptDialog.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"


"$(INTDIR)\vncAcceptDialog.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"


"$(INTDIR)\vncAcceptDialog.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

!ENDIF 

SOURCE=.\vncAdvancedProperties.cpp

!IF  "$(CFG)" == "WinVNC - Win32 Release"


"$(INTDIR)\vncAdvancedProperties.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"


"$(INTDIR)\vncAdvancedProperties.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"


"$(INTDIR)\vncAdvancedProperties.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"


"$(INTDIR)\vncAdvancedProperties.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"


"$(INTDIR)\vncAdvancedProperties.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

!ENDIF 

SOURCE=.\vncauth.c

!IF  "$(CFG)" == "WinVNC - Win32 Release"


"$(INTDIR)\vncauth.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"


"$(INTDIR)\vncauth.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"


"$(INTDIR)\vncauth.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"


"$(INTDIR)\vncauth.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"


"$(INTDIR)\vncauth.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

!ENDIF 

SOURCE=.\vncBuffer.cpp

!IF  "$(CFG)" == "WinVNC - Win32 Release"


"$(INTDIR)\vncBuffer.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"


"$(INTDIR)\vncBuffer.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"


"$(INTDIR)\vncBuffer.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"


"$(INTDIR)\vncBuffer.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"


"$(INTDIR)\vncBuffer.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

!ENDIF 

SOURCE=.\vncClient.cpp

!IF  "$(CFG)" == "WinVNC - Win32 Release"


"$(INTDIR)\vncClient.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"


"$(INTDIR)\vncClient.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"


"$(INTDIR)\vncClient.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"


"$(INTDIR)\vncClient.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"


"$(INTDIR)\vncClient.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

!ENDIF 

SOURCE=.\vncConnDialog.cpp

!IF  "$(CFG)" == "WinVNC - Win32 Release"


"$(INTDIR)\vncConnDialog.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"


"$(INTDIR)\vncConnDialog.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"


"$(INTDIR)\vncConnDialog.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"


"$(INTDIR)\vncConnDialog.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"


"$(INTDIR)\vncConnDialog.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

!ENDIF 

SOURCE=.\vncDesktop.cpp

!IF  "$(CFG)" == "WinVNC - Win32 Release"


"$(INTDIR)\vncDesktop.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"


"$(INTDIR)\vncDesktop.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"


"$(INTDIR)\vncDesktop.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"


"$(INTDIR)\vncDesktop.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"


"$(INTDIR)\vncDesktop.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

!ENDIF 

SOURCE=.\vncDesktopDX.cpp
SOURCE=.\vncEncoder.cpp

!IF  "$(CFG)" == "WinVNC - Win32 Release"


"$(INTDIR)\vncEncoder.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"


"$(INTDIR)\vncEncoder.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"


"$(INTDIR)\vncEncoder.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"


"$(INTDIR)\vncEncoder.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"


"$(INTDIR)\vncEncoder.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

!ENDIF 

SOURCE=.\vncEncodeTight.cpp

!IF  "$(CFG)" == "WinVNC - Win32 Release"


"$(INTDIR)\vncEncodeTight.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"


"$(INTDIR)\vncEncodeTight.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"


"$(INTDIR)\vncEncodeTight.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"


"$(INTDIR)\vncEncodeTight.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"


"$(INTDIR)\vncEncodeTight.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

!ENDIF 

SOURCE=.\vncHTTPConnect.cpp

!IF  "$(CFG)" == "WinVNC - Win32 Release"


"$(INTDIR)\vncHTTPConnect.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"


"$(INTDIR)\vncHTTPConnect.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"


"$(INTDIR)\vncHTTPConnect.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"


"$(INTDIR)\vncHTTPConnect.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"


"$(INTDIR)\vncHTTPConnect.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

!ENDIF 

SOURCE=.\vncInstHandler.cpp

!IF  "$(CFG)" == "WinVNC - Win32 Release"


"$(INTDIR)\vncInstHandler.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"


"$(INTDIR)\vncInstHandler.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"


"$(INTDIR)\vncInstHandler.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"


"$(INTDIR)\vncInstHandler.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"


"$(INTDIR)\vncInstHandler.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

!ENDIF 

SOURCE=.\vncKeymap.cpp

!IF  "$(CFG)" == "WinVNC - Win32 Release"


"$(INTDIR)\vncKeymap.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"


"$(INTDIR)\vncKeymap.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"


"$(INTDIR)\vncKeymap.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"


"$(INTDIR)\vncKeymap.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"


"$(INTDIR)\vncKeymap.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

!ENDIF 

SOURCE=.\vncMenu.cpp

!IF  "$(CFG)" == "WinVNC - Win32 Release"


"$(INTDIR)\vncMenu.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"


"$(INTDIR)\vncMenu.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"


"$(INTDIR)\vncMenu.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"


"$(INTDIR)\vncMenu.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"


"$(INTDIR)\vncMenu.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

!ENDIF 

SOURCE=.\vncProperties.cpp

!IF  "$(CFG)" == "WinVNC - Win32 Release"


"$(INTDIR)\vncProperties.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"


"$(INTDIR)\vncProperties.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"


"$(INTDIR)\vncProperties.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"


"$(INTDIR)\vncProperties.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"


"$(INTDIR)\vncProperties.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

!ENDIF 

SOURCE=.\vncRegion.cpp

!IF  "$(CFG)" == "WinVNC - Win32 Release"


"$(INTDIR)\vncRegion.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"


"$(INTDIR)\vncRegion.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"


"$(INTDIR)\vncRegion.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"


"$(INTDIR)\vncRegion.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"


"$(INTDIR)\vncRegion.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

!ENDIF 

SOURCE=.\vncServer.cpp

!IF  "$(CFG)" == "WinVNC - Win32 Release"


"$(INTDIR)\vncServer.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"


"$(INTDIR)\vncServer.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"


"$(INTDIR)\vncServer.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"


"$(INTDIR)\vncServer.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"


"$(INTDIR)\vncServer.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

!ENDIF 

SOURCE=.\vncService.cpp

!IF  "$(CFG)" == "WinVNC - Win32 Release"


"$(INTDIR)\vncService.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"


"$(INTDIR)\vncService.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"


"$(INTDIR)\vncService.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"


"$(INTDIR)\vncService.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"


"$(INTDIR)\vncService.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

!ENDIF 

SOURCE=.\vncSockConnect.cpp

!IF  "$(CFG)" == "WinVNC - Win32 Release"


"$(INTDIR)\vncSockConnect.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"


"$(INTDIR)\vncSockConnect.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"


"$(INTDIR)\vncSockConnect.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"


"$(INTDIR)\vncSockConnect.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"


"$(INTDIR)\vncSockConnect.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

!ENDIF 

SOURCE=.\vncTimedMsgBox.cpp

!IF  "$(CFG)" == "WinVNC - Win32 Release"


"$(INTDIR)\vncTimedMsgBox.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"


"$(INTDIR)\vncTimedMsgBox.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"


"$(INTDIR)\vncTimedMsgBox.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"


"$(INTDIR)\vncTimedMsgBox.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"


"$(INTDIR)\vncTimedMsgBox.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

!ENDIF 

SOURCE=.\VSocket.cpp

!IF  "$(CFG)" == "WinVNC - Win32 Release"


"$(INTDIR)\VSocket.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"


"$(INTDIR)\VSocket.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"


"$(INTDIR)\VSocket.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"


"$(INTDIR)\VSocket.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"


"$(INTDIR)\VSocket.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

!ENDIF 

SOURCE=.\WinVNC.cpp

!IF  "$(CFG)" == "WinVNC - Win32 Release"


"$(INTDIR)\WinVNC.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Debug"


"$(INTDIR)\WinVNC.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"


"$(INTDIR)\WinVNC.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"


"$(INTDIR)\WinVNC.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"


"$(INTDIR)\WinVNC.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

!ENDIF 

SOURCE=.\WinVNC.rc

"$(INTDIR)\WinVNC.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\vnc.idl

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

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"

"VNCHooks - Win32 Purify" : 
   cd ".\VNCHooks"
   $(MAKE) /$(MAKEFLAGS) /F .\VNCHooks.mak CFG="VNCHooks - Win32 Purify" 
   cd ".."

"VNCHooks - Win32 PurifyCLEAN" : 
   cd ".\VNCHooks"
   $(MAKE) /$(MAKEFLAGS) /F .\VNCHooks.mak CFG="VNCHooks - Win32 Purify" RECURSE=1 CLEAN 
   cd ".."

!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"

"VNCHooks - Win32 No_CORBA" : 
   cd ".\VNCHooks"
   $(MAKE) /$(MAKEFLAGS) /F .\VNCHooks.mak CFG="VNCHooks - Win32 No_CORBA" 
   cd ".."

"VNCHooks - Win32 No_CORBACLEAN" : 
   cd ".\VNCHooks"
   $(MAKE) /$(MAKEFLAGS) /F .\VNCHooks.mak CFG="VNCHooks - Win32 No_CORBA" RECURSE=1 CLEAN 
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

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

"VNCHooks - Win32 Alpha No_CORBA" : 
   cd ".\VNCHooks"
   $(MAKE) /$(MAKEFLAGS) /F .\VNCHooks.mak CFG="VNCHooks - Win32 Alpha No_CORBA" 
   cd ".."

"VNCHooks - Win32 Alpha No_CORBACLEAN" : 
   cd ".\VNCHooks"
   $(MAKE) /$(MAKEFLAGS) /F .\VNCHooks.mak CFG="VNCHooks - Win32 Alpha No_CORBA" RECURSE=1 CLEAN 
   cd ".."

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

"VNCHooks - Win32 Alpha Debug No_CORBA" : 
   cd ".\VNCHooks"
   $(MAKE) /$(MAKEFLAGS) /F .\VNCHooks.mak CFG="VNCHooks - Win32 Alpha Debug No_CORBA" 
   cd ".."

"VNCHooks - Win32 Alpha Debug No_CORBACLEAN" : 
   cd ".\VNCHooks"
   $(MAKE) /$(MAKEFLAGS) /F .\VNCHooks.mak CFG="VNCHooks - Win32 Alpha Debug No_CORBA" RECURSE=1 CLEAN 
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

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"

"omnithread - Win32 Purify" : 
   cd ".\omnithread"
   $(MAKE) /$(MAKEFLAGS) /F .\omnithread.mak CFG="omnithread - Win32 Purify" 
   cd ".."

"omnithread - Win32 PurifyCLEAN" : 
   cd ".\omnithread"
   $(MAKE) /$(MAKEFLAGS) /F .\omnithread.mak CFG="omnithread - Win32 Purify" RECURSE=1 CLEAN 
   cd ".."

!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"

"omnithread - Win32 No_CORBA" : 
   cd ".\omnithread"
   $(MAKE) /$(MAKEFLAGS) /F .\omnithread.mak CFG="omnithread - Win32 No_CORBA" 
   cd ".."

"omnithread - Win32 No_CORBACLEAN" : 
   cd ".\omnithread"
   $(MAKE) /$(MAKEFLAGS) /F .\omnithread.mak CFG="omnithread - Win32 No_CORBA" RECURSE=1 CLEAN 
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

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

"omnithread - Win32 Alpha No_CORBA" : 
   cd ".\omnithread"
   $(MAKE) /$(MAKEFLAGS) /F .\omnithread.mak CFG="omnithread - Win32 Alpha No_CORBA" 
   cd ".."

"omnithread - Win32 Alpha No_CORBACLEAN" : 
   cd ".\omnithread"
   $(MAKE) /$(MAKEFLAGS) /F .\omnithread.mak CFG="omnithread - Win32 Alpha No_CORBA" RECURSE=1 CLEAN 
   cd ".."

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

"omnithread - Win32 Alpha Debug No_CORBA" : 
   cd ".\omnithread"
   $(MAKE) /$(MAKEFLAGS) /F .\omnithread.mak CFG="omnithread - Win32 Alpha Debug No_CORBA" 
   cd ".."

"omnithread - Win32 Alpha Debug No_CORBACLEAN" : 
   cd ".\omnithread"
   $(MAKE) /$(MAKEFLAGS) /F .\omnithread.mak CFG="omnithread - Win32 Alpha Debug No_CORBA" RECURSE=1 CLEAN 
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

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Purify"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 No_CORBA"

"zlib - Win32 No_CORBA" : 
   cd ".\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 No_CORBA" 
   cd ".."

"zlib - Win32 No_CORBACLEAN" : 
   cd ".\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 No_CORBA" RECURSE=1 CLEAN 
   cd ".."

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Profile"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha No_CORBA"

!ELSEIF  "$(CFG)" == "WinVNC - Win32 Alpha Debug No_CORBA"

!ENDIF 


!ENDIF 

