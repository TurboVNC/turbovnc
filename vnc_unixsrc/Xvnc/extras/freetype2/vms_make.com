$!---------------vms_make.com for Freetype2------------------------------------
$! make Freetype2 under OpenVMS
$!
$! In case of problems with the build you might want to contact me at
$! zinser@decus.de (preferred) or zinser@sysdev.deutsche-boerse.com (Work)
$!
$! This procedure currently does support the following commandline options
$! in arbitrary order 
$!
$! * DEBUG - Compile modules with /noopt/debug and link shareable image 
$!           with /debug
$! * LOPTS - Options to be passed to the link command
$! * CCOPT - Options to be passed to the C compiler
$!------------------------------------------------------------------------------
$! 
$! Just some general constants
$!
$ true   = 1
$ false  = 0
$ Make   = ""
$!
$! Setup variables holding "config" information
$!
$ name    = "Freetype2"
$ mapfile =  name + ".map"
$ optfile =  name + ".opt"
$ s_case  = false
$ libdefs = ""
$ libincs = ""
$ liblist = ""
$ ccopt   = ""
$ lopts   = ""
$!
$! Check for MMK/MMS
$!
$ If F$Search ("Sys$System:MMS.EXE") .nes. "" Then Make = "MMS"
$ If F$Type (MMK) .eqs. "STRING" Then Make = "MMK"
$!
$! Which command parameters were given
$!
$ gosub check_opts
$!
$! Create option file
$!
$ open/write optf 'optfile'
$!
$! Pull in external libraries
$!
$ create libs.opt
$ gosub check_create_vmslib
$!
$! Create objects
$!
$ if libdefs .nes. "" then ccopt = ccopt + "/define=(" + libdefs + ")"
$!
$ if f$locate("AS_IS",f$edit(ccopt,"UPCASE")) .lt. f$length(ccopt) - 
    then s_case = true
$ gosub crea_mms
$!
$ 'Make' /macro=(comp_flags="''ccopt'")
$ purge/nolog [...]descrip.mms
$!
$! Add them to options
$!
$FLOOP:
$  file = f$edit(f$search("[...]*.obj"),"UPCASE")
$  if (file .nes. "")
$  then
$    if f$locate("DEMOS",file) .eqs. f$length(file) then write optf file
$    goto floop
$  endif 
$!
$ close optf
$!
$!
$! Alpha gets a shareable image
$!
$ If f$getsyi("HW_MODEL") .gt. 1024
$ Then
$   write sys$output "Creating freetype2shr.exe"
$   call anal_obj_axp 'optfile' _link.opt
$   open/append  optf 'optfile'
$   if s_case then WRITE optf "case_sensitive=YES"
$   close optf
$   LINK_/NODEB/SHARE=[.lib]freetype2shr.exe - 
                            'optfile'/opt,libs.opt/opt,_link.opt/opt
$ endif
$!
$ exit
$!
$!------------------------------------------------------------------------------
$!
$! If MMS/MMK are available dump out the descrip.mms if required 
$!
$CREA_MMS:
$ write sys$output "Creating descrip.mms files ..."
$ write sys$output "... Main directory"
$ create descrip.mms
$ open/append out descrip.mms
$ copy sys$input: out
$ deck
#
# FreeType 2 build system -- top-level Makefile for OpenVMS
#


# Copyright 2001 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.
$ EOD
$ write out "CFLAGS = ", ccopt
$ copy sys$input: out
$ deck
 

all :
        define freetype [--.include.freetype] 
        define psaux [-.psaux] 
        define autohint [-.autohint] 
        define base [-.base] 
        define cache [-.cache] 
        define cff [-.cff] 
        define cid [-.cid] 
        define pcf [-.pcf] 
        define psnames [-.psnames] 
        define raster [-.raster] 
        define sfnt [-.sfnt] 
        define smooth [-.smooth] 
        define truetype [-.truetype] 
        define type1 [-.type1] 
        define winfonts [-.winfonts] 
        if f$search("lib.dir") .eqs. "" then create/directory [.lib]
        set default [.builds.vms]
        $(MMS)$(MMSQUALIFIERS)
        set default [--.src.autohint]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.base]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.bdf]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.cache]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.cff]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.cid]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.gzip]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.lzw]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.pcf]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.pfr]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.psaux]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.pshinter]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.psnames]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.raster]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.sfnt]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.smooth]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.truetype]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.type1]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.type42]
        $(MMS)$(MMSQUALIFIERS)
        set default [-.winfonts]
        $(MMS)$(MMSQUALIFIERS)
        set default [--]

# EOF
$ eod
$ close out
$ write sys$output "... [.builds.vms] directory"
$ create [.builds.vms]descrip.mms
$ open/append out [.builds.vms]descrip.mms
$ copy sys$input: out
$ deck
#
# FreeType 2 system rules for VMS
#


# Copyright 2001 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


CFLAGS=$(COMP_FLAGS)$(DEBUG)/include=([],[--.include],[--.src.base])

OBJS=ftsystem.obj

all : $(OBJS)
        library/create [--.lib]freetype.olb $(OBJS)

ftsystem.obj : ftsystem.c ftconfig.h

# EOF
$ eod
$ close out
$ write sys$output "... [.src.autohint] directory"
$ create [.src.autohint]descrip.mms
$ open/append out [.src.autohint]descrip.mms
$ copy sys$input: out
$ deck
#
# FreeType 2 auto-hinter module compilation rules for VMS
#


# Copyright 2001, 2002 Catharon Productions Inc.
#
# This file is part of the Catharon Typography Project and shall only
# be used, modified, and distributed under the terms of the Catharon
# Open Source License that should come with this file under the name
# `CatharonLicense.txt'.  By continuing to use, modify, or distribute
# this file you indicate that you have read the license and
# understand and accept it fully.
#
# Note that this license is compatible with the FreeType license.


CFLAGS=$(COMP_FLAGS)$(DEBUG)/incl=([--.include],[--.src.autohint])

OBJS=autohint.obj

all : $(OBJS)
        library [--.lib]freetype.olb $(OBJS)

# EOF
$ eod
$ close out
$ write sys$output "... [.src.gzip] directory"
$ create [.src.gzip]descrip.mms
$ open/append out [.src.gzip]descrip.mms
$ copy sys$input: out
$ deck
#
# FreeType 2 GZip support compilation rules for VMS
#


# Copyright 2002 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.
$EOD
$ if libincs .nes. "" then write out "LIBINCS = ", libincs, ","
$ copy sys$input: out
$ deck

CFLAGS=$(COMP_FLAGS)$(DEBUG)/include=($(LIBINCS)[--.include],[--.src.gzip])

OBJS=ftgzip.obj

all : $(OBJS)
        library [--.lib]freetype.olb $(OBJS)

# EOF
$ eod
$ close out
$ write sys$output "... [.src.lzw] directory"
$ create [.src.lzw]descrip.mms
$ open/append out [.src.lzw]descrip.mms
$ copy sys$input: out
$ deck
#
# FreeType 2 LZW support compilation rules for VMS
#


# Copyright 2004 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.
$EOD
$ if libincs .nes. "" then write out "LIBINCS = ", libincs, ","
$ copy sys$input: out
$ deck

CFLAGS=$(COMP_FLAGS)$(DEBUG)/include=($(LIBINCS)[--.include],[--.src.lzw])

OBJS=ftlzw.obj

all : $(OBJS)
        library [--.lib]freetype.olb $(OBJS)

# EOF
$ eod
$ close out
$ write sys$output "... [.src.type1] directory"
$ create [.src.type1]descrip.mms
$ open/append out [.src.type1]descrip.mms
$ copy sys$input: out
$ deck
#
# FreeType 2 Type1 driver compilation rules for VMS
#


# Copyright 1996-2000, 2002 by
# David Turner, Robert Wilhelm, and Werner Lemberg.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


CFLAGS=$(COMP_FLAGS)$(DEBUG)/include=([--.include],[--.src.type1])

OBJS=type1.obj

all : $(OBJS)
        library [--.lib]freetype.olb $(OBJS)

type1.obj : type1.c t1parse.c t1load.c t1objs.c t1driver.c t1gload.c t1afm.c

# EOF
$ eod
$ close out
$ return
$!------------------------------------------------------------------------------
$!
$! Check command line options and set symbols accordingly
$!
$ CHECK_OPTS:
$ i = 1
$ OPT_LOOP:
$ if i .lt. 9
$ then
$   cparm = f$edit(p'i',"upcase")
$   if cparm .eqs. "DEBUG"
$   then
$     ccopt = ccopt + "/noopt/deb"
$     lopts = lopts + "/deb"
$   endif
$!   if cparm .eqs. "LINK" then linkonly = true
$   if f$locate("LOPTS",cparm) .lt. f$length(cparm)
$   then
$     start = f$locate("=",cparm) + 1
$     len   = f$length(cparm) - start
$     lopts = lopts + f$extract(start,len,cparm)
$   endif
$   if f$locate("CCOPT",cparm) .lt. f$length(cparm)
$   then
$     start = f$locate("=",cparm) + 1
$     len   = f$length(cparm) - start
$     ccopt = ccopt + f$extract(start,len,cparm)
$   endif
$   i = i + 1
$   goto opt_loop
$ endif
$ return
$!------------------------------------------------------------------------------
$!
$! Take care of driver file with information about external libraries
$!
$CHECK_CREATE_VMSLIB:
$!
$ if f$search("VMSLIB.DAT") .eqs. ""
$ then
$   type/out=vmslib.dat sys$input
!
! This is a simple driver file with information used by make.com to
! check if external libraries (like t1lib and freetype) are available on
! the system.
!
! Layout of the file:
!
!    - Lines starting with ! are treated as comments
!    - Elements in a data line are separated by # signs
!    - The elements need to be listed in the following order
!      1.) Name of the Library 
!      2.) Location where the object library can be found
!      3.) Location where the include files for the library can be found
!      4.) Include file used to verify library location
!      5.) CPP define to pass to the build to indicate availability of
!          the library
!
! Example: The following  lines show how definitions
!          might look like. They are site specific and the locations of the
!          library and include files need almost certainly to be changed.
!
! Location: All of the libaries can be found at the following addresses
!
!   ZLIB:     http://www.decus.de:8080/www/vms/sw/zlib.htmlx
!
!ZLIB # pubbin:libz.olb # public$Root:[util.libs.zlib] # zlib.h # FT_CONFIG_OPTION_SYSTEM_ZLIB
$   write sys$output "New driver file vmslib.dat created."
$   write sys$output "Please customize libary locations for your site"
$   write sys$output "and afterwards re-execute vms_make.com"
$   write sys$output "Exiting..."
$   close/nolog optf
$   exit
$ endif
$!
$! Open data file with location of libraries
$!
$ open/read/end=end_lib/err=lib_err libdata VMSLIB.DAT
$ open/append loptf libs.opt
$LIB_LOOP:
$ read/end=end_lib libdata libline
$ libline = f$edit(libline, "UNCOMMENT,COLLAPSE")
$ if libline .eqs. "" then goto LIB_LOOP ! Comment line
$ libname = f$edit(f$element(0,"#",libline),"UPCASE")
$ liblist = liblist + "#" + libname
$ write sys$output "Processing ''libname' setup ..."
$ libloc  = f$element(1,"#",libline)
$ libsrc  = f$element(2,"#",libline)
$ testinc = f$element(3,"#",libline)
$ cppdef  = f$element(4,"#",libline)
$ old_cpp = f$locate("=1",cppdef)
$ if old_cpp.lt.f$length(cppdef) then cppdef = f$extract(0,old_cpp,cppdef)
$ if f$search("''libloc'").eqs. ""
$ then
$   write sys$output "Can not find library ''libloc' - Skipping ''libname'"
$   goto LIB_LOOP
$ endif
$ libsrc_elem = 0
$ libsrc_found = false
$LIBSRC_LOOP:
$ libsrcdir = f$element(libsrc_elem,",",libsrc)
$ if (libsrcdir .eqs. ",") then goto END_LIBSRC
$ if f$search("''libsrcdir'''testinc'") .nes. "" then libsrc_found = true
$ libsrc_elem = libsrc_elem + 1
$ goto LIBSRC_LOOP
$END_LIBSRC:
$ if .not. libsrc_found
$ then
$   write sys$output "Can not find includes at ''libsrc' - Skipping ''libname'"
$   goto LIB_LOOP
$ endif
$ if cppdef .nes. "" then libdefs = libdefs +  "," + cppdef
$ libincs = libincs + "," + libsrc
$ lqual = "/lib"
$ libtype = f$parse(libloc,,,"TYPE")
$ if f$locate("EXE",libtype) .lt. f$length(libtype) then lqual = "/share"
$ write loptf libloc , lqual
$ goto LIB_LOOP
$END_LIB:
$ close libdata
$ close loptf
$ libincs = libincs - ","
$ libdefs = libdefs - ","
$ return
$!------------------------------------------------------------------------------
$!
$! Analyze Object files for OpenVMS AXP to extract Procedure and Data 
$! information to build a symbol vector for a shareable image
$! All the "brains" of this logic was suggested by Hartmut Becker
$! (Hartmut.Becker@compaq.com). All the bugs were introduced by me
$! (zinser@decus.de), so if you do have problem reports please do not
$! bother Hartmut/HP, but get in touch with me
$!
$ ANAL_OBJ_AXP: Subroutine   
$ V = 'F$Verify(0)
$ SAY := "WRITE_ SYS$OUTPUT"
$ 
$ IF F$SEARCH("''P1'") .EQS. ""
$ THEN
$    SAY "ANAL_OBJ_AXP-E-NOSUCHFILE:  Error, inputfile ''p1' not available"
$    goto exit_aa
$ ENDIF
$ IF "''P2'" .EQS. ""
$ THEN
$    SAY "ANAL_OBJ_AXP:  Error, no output file provided"
$    goto exit_aa
$ ENDIF
$
$ open/read in 'p1
$ create a.tmp
$ open/append atmp a.tmp
$ loop:
$ read/end=end_loop in line
$ f= f$search(line)
$ if f .eqs. ""
$ then
$	write sys$output "ANAL_OBJ_AXP-w-nosuchfile, ''line'"
$	goto loop
$ endif
$ def/user sys$output nl:
$ def/user sys$error nl:
$ anal/obj/gsd 'f /out=x.tmp
$ open/read xtmp x.tmp
$ XLOOP:
$ read/end=end_xloop xtmp xline
$ xline = f$edit(xline,"compress")
$ write atmp xline
$ goto xloop
$ END_XLOOP:
$ close xtmp
$ goto loop
$ end_loop:
$ close in
$ close atmp
$ if f$search("a.tmp") .eqs. "" -
	then $ exit
$ ! all global definitions
$ search a.tmp "symbol:","EGSY$V_DEF 1","EGSY$V_NORM 1"/out=b.tmp
$ ! all procedures
$ search b.tmp "EGSY$V_NORM 1"/wind=(0,1) /out=c.tmp
$ search c.tmp "symbol:"/out=d.tmp
$ def/user sys$output nl:
$ edito/edt/command=sys$input d.tmp
sub/symbol: "/symbol_vector=(/whole
sub/"/=procedure)/whole
exit
$ ! all data
$ search b.tmp "EGSY$V_DEF 1"/wind=(0,1) /out=e.tmp
$ search e.tmp "symbol:"/out=f.tmp
$ def/user sys$output nl:
$ edito/edt/command=sys$input f.tmp
sub/symbol: "/symbol_vector=(/whole
sub/"/=data)/whole
exit
$ sort/nodupl d.tmp,f.tmp 'p2'
$ delete a.tmp;*,b.tmp;*,c.tmp;*,d.tmp;*,e.tmp;*,f.tmp;*
$ if f$search("x.tmp") .nes. "" -
	then $ delete x.tmp;*
$!
$ EXIT_AA:
$ if V then set verify
$ endsubroutine 
