# Microsoft Developer Studio Generated NMAKE File, Based on omnithread.dsp
!IF "$(CFG)" == ""
CFG=omnithread - Win32 No_CORBA
!MESSAGE No configuration specified. Defaulting to omnithread - Win32 No_CORBA.
!ENDIF 

!IF "$(CFG)" != "omnithread - Win32 Release" && "$(CFG)" != "omnithread - Win32 Debug" && "$(CFG)" != "omnithread - Win32 Purify" && "$(CFG)" != "omnithread - Win32 No_CORBA" && "$(CFG)" != "omnithread - Win32 Profile" && "$(CFG)" != "omnithread - Win32 Alpha No_CORBA" && "$(CFG)" != "omnithread - Win32 Alpha Debug No_CORBA"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "omnithread.mak" CFG="omnithread - Win32 Profile"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "omnithread - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "omnithread - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "omnithread - Win32 Purify" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "omnithread - Win32 No_CORBA" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "omnithread - Win32 Profile" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "omnithread - Win32 Alpha No_CORBA" (based on "Win32 (ALPHA) Dynamic-Link Library")
!MESSAGE "omnithread - Win32 Alpha Debug No_CORBA" (based on "Win32 (ALPHA) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "omnithread - Win32 Release"

OUTDIR=.\../Release
INTDIR=.\../Release
# Begin Custom Macros
OutDir=.\../Release
# End Custom Macros

ALL : "$(OUTDIR)\omnithread_rt.dll"


CLEAN :
	-@erase "$(INTDIR)\nt.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\omnithread_rt.dll"
	-@erase "$(OUTDIR)\omnithread_rt.exp"
	-@erase "$(OUTDIR)\omnithread_rt.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "." /I "\\shallot\omni\release\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__WIN32__" /D "__NT__" /D "__x86__" /D "_OMNITHREAD_DLL" /Fp"$(INTDIR)\omnithread.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\omnithread.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\omnithread_rt.pdb" /machine:I386 /out:"$(OUTDIR)\omnithread_rt.dll" /implib:"$(OUTDIR)\omnithread_rt.lib" /libpath:"\\shallot\omni\release\lib\x86_nt_4.0" 
LINK32_OBJS= \
	"$(INTDIR)\nt.obj"

"$(OUTDIR)\omnithread_rt.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "omnithread - Win32 Debug"

OUTDIR=.\../Debug
INTDIR=.\../Debug
# Begin Custom Macros
OutDir=.\../Debug
# End Custom Macros

ALL : "$(OUTDIR)\omnithread_rtd.dll"


CLEAN :
	-@erase "$(INTDIR)\nt.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\omnithread_rtd.dll"
	-@erase "$(OUTDIR)\omnithread_rtd.exp"
	-@erase "$(OUTDIR)\omnithread_rtd.ilk"
	-@erase "$(OUTDIR)\omnithread_rtd.lib"
	-@erase "$(OUTDIR)\omnithread_rtd.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "." /I "\\shallot\omni\release\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "__WIN32__" /D "__NT__" /D "__x86__" /D "_OMNITHREAD_DLL" /Fp"$(INTDIR)\omnithread.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\omnithread.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\omnithread_rtd.pdb" /debug /machine:I386 /out:"$(OUTDIR)\omnithread_rtd.dll" /implib:"$(OUTDIR)\omnithread_rtd.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\nt.obj"

"$(OUTDIR)\omnithread_rtd.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "omnithread - Win32 Purify"

OUTDIR=.\../Purify
INTDIR=.\../Purify
# Begin Custom Macros
OutDir=.\../Purify
# End Custom Macros

ALL : "$(OUTDIR)\omnithread_rtd.dll"


CLEAN :
	-@erase "$(INTDIR)\nt.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\omnithread_rtd.dll"
	-@erase "$(OUTDIR)\omnithread_rtd.exp"
	-@erase "$(OUTDIR)\omnithread_rtd.ilk"
	-@erase "$(OUTDIR)\omnithread_rtd.lib"
	-@erase "$(OUTDIR)\omnithread_rtd.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "__WIN32__" /D "__NT__" /D "__x86__" /D "_OMNITHREAD_DLL" /Fp"$(INTDIR)\omnithread.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\omnithread.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\omnithread_rtd.pdb" /debug /machine:I386 /out:"$(OUTDIR)\omnithread_rtd.dll" /implib:"$(OUTDIR)\omnithread_rtd.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\nt.obj"

"$(OUTDIR)\omnithread_rtd.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "omnithread - Win32 No_CORBA"

OUTDIR=.\../No_CORBA
INTDIR=.\../No_CORBA
# Begin Custom Macros
OutDir=.\../No_CORBA
# End Custom Macros

ALL : "$(OUTDIR)\omnithread_rt.dll"


CLEAN :
	-@erase "$(INTDIR)\nt.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\omnithread_rt.dll"
	-@erase "$(OUTDIR)\omnithread_rt.exp"
	-@erase "$(OUTDIR)\omnithread_rt.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "." /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__WIN32__" /D "__NT__" /D "__x86__" /D "_OMNITHREAD_DLL" /Fp"$(INTDIR)\omnithread.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\omnithread.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\omnithread_rt.pdb" /machine:I386 /out:"$(OUTDIR)\omnithread_rt.dll" /implib:"$(OUTDIR)\omnithread_rt.lib" 
LINK32_OBJS= \
	"$(INTDIR)\nt.obj"

"$(OUTDIR)\omnithread_rt.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "omnithread - Win32 Profile"

OUTDIR=.\../Profile
INTDIR=.\../Profile
# Begin Custom Macros
OutDir=.\../Profile
# End Custom Macros

ALL : "$(OUTDIR)\omnithread_rtd.dll"


CLEAN :
	-@erase "$(INTDIR)\nt.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\omnithread_rtd.dll"
	-@erase "$(OUTDIR)\omnithread_rtd.exp"
	-@erase "$(OUTDIR)\omnithread_rtd.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "." /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "__WIN32__" /D "__NT__" /D "__x86__" /D "_OMNITHREAD_DLL" /Fp"$(INTDIR)\omnithread.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\omnithread.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /profile /debug /machine:I386 /out:"$(OUTDIR)\omnithread_rtd.dll" /implib:"$(OUTDIR)\omnithread_rtd.lib" 
LINK32_OBJS= \
	"$(INTDIR)\nt.obj"

"$(OUTDIR)\omnithread_rtd.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "omnithread - Win32 Alpha No_CORBA"

OUTDIR=.\../Alpha_No_CORBA
INTDIR=.\../Alpha_No_CORBA

ALL : 


CLEAN :
	-@erase 

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\omnithread.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\omnithread_rt.pdb" /machine:ALPHA /out:"$(OUTDIR)\omnithread_rt.dll" /implib:"$(OUTDIR)\omnithread_rt.lib" 
LINK32_OBJS= \
	

!ELSEIF  "$(CFG)" == "omnithread - Win32 Alpha Debug No_CORBA"

OUTDIR=.\../AlphaDbg_No_CORBA
INTDIR=.\../AlphaDbg_No_CORBA

ALL : 


CLEAN :
	-@erase 

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\omnithread.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\omnithread_rt.pdb" /debug /machine:ALPHA /out:"$(OUTDIR)\omnithread_rt.dll" /implib:"$(OUTDIR)\omnithread_rt.lib" /pdbtype:sept 
LINK32_OBJS= \
	

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("omnithread.dep")
!INCLUDE "omnithread.dep"
!ELSE 
!MESSAGE Warning: cannot find "omnithread.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "omnithread - Win32 Release" || "$(CFG)" == "omnithread - Win32 Debug" || "$(CFG)" == "omnithread - Win32 Purify" || "$(CFG)" == "omnithread - Win32 No_CORBA" || "$(CFG)" == "omnithread - Win32 Profile" || "$(CFG)" == "omnithread - Win32 Alpha No_CORBA" || "$(CFG)" == "omnithread - Win32 Alpha Debug No_CORBA"
SOURCE=.\omnithread\nt.cpp

!IF  "$(CFG)" == "omnithread - Win32 Release"


"$(INTDIR)\nt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "omnithread - Win32 Debug"


"$(INTDIR)\nt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "omnithread - Win32 Purify"


"$(INTDIR)\nt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "omnithread - Win32 No_CORBA"


"$(INTDIR)\nt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "omnithread - Win32 Profile"


"$(INTDIR)\nt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "omnithread - Win32 Alpha No_CORBA"

!ELSEIF  "$(CFG)" == "omnithread - Win32 Alpha Debug No_CORBA"

!ENDIF 


!ENDIF 

