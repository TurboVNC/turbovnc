# Microsoft Developer Studio Generated NMAKE File, Based on omnithread.dsp
!IF "$(CFG)" == ""
CFG=omnithread - Win32 Release
!MESSAGE No configuration specified. Defaulting to omnithread - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "omnithread - Win32 Release" && "$(CFG)" != "omnithread - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "omnithread.mak" CFG="omnithread - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "omnithread - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "omnithread - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "omnithread - Win32 Release"

OUTDIR=.\../Release
INTDIR=.\../Release
# Begin Custom Macros
OutDir=.\../Release
# End Custom Macros

ALL : "$(OUTDIR)\omnithread.lib"


CLEAN :
	-@erase "$(INTDIR)\nt.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\omnithread.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__NT__" /D "_WINSTATIC" /D "__WIN32__" /Fp"$(INTDIR)\omnithread.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\omnithread.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\omnithread.lib" 
LIB32_OBJS= \
	"$(INTDIR)\nt.obj"

"$(OUTDIR)\omnithread.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "omnithread - Win32 Debug"

OUTDIR=.\../Debug
INTDIR=.\../Debug
# Begin Custom Macros
OutDir=.\../Debug
# End Custom Macros

ALL : "$(OUTDIR)\omnithread.lib"


CLEAN :
	-@erase "$(INTDIR)\nt.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\omnithread.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /GX /Z7 /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "__NT__" /D "_WINSTATIC" /D "__WIN32__" /Fp"$(INTDIR)\omnithread.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\omnithread.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\omnithread.lib" 
LIB32_OBJS= \
	"$(INTDIR)\nt.obj"

"$(OUTDIR)\omnithread.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
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
!IF EXISTS("omnithread.dep")
!INCLUDE "omnithread.dep"
!ELSE 
!MESSAGE Warning: cannot find "omnithread.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "omnithread - Win32 Release" || "$(CFG)" == "omnithread - Win32 Debug"
SOURCE=.\nt.cpp

"$(INTDIR)\nt.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

