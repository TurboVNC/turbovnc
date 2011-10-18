/* REXX */
/* $XFree86: xc/config/util/buildos2.cmd,v 3.7.2.1 1997/05/12 12:52:09 hohndel Exp $
 * this file is supposed to run from the xc/ dir.
 * you must copy it manually to there before using. It is just here
 * in order not to be in the root dir.
 *
 * copy some essential files to a location where we find them again
 */
'@echo off'
env = 'OS2ENVIRONMENT'
'copy config\util\indir.cmd \ > nul 2>&1'
'copy config\util\mkdirhier.cmd \ > nul 2>&1'
'copy config\imake\imakesvc.cmd \ > nul 2>&1'

IF \ exists('config\cf\host.def') THEN
    CALL LINEOUT 'config\cf\host.def',' '

CALL create_makefile

CALL VALUE 'GCCOPT','-pipe',env
CALL VALUE 'EMXLOAD',5,env
CALL VALUE 'MAKEFLAGS','--no-print-directory',env
'emxload -e x11make.exe rm.exe mv.exe'
'emxload -e -gcc -omf'

'x11make MAKE=x11make SHELL= MFLAGS="MAKE=x11make CC=gcc BOOTSTRAPCFLAGS=-DBSD43 SHELL= " World.OS2 2>&1 | tee buildxc.log'

EXIT

/* returns 1, if file exists */
exists:
	IF STREAM(arg(1), 'C', 'QUERY EXISTS') = '' THEN
		RETURN 0
	ELSE
		RETURN 1

create_makefile:
IF exists(Makefile) THEN del Makefile
CALL LINEOUT 'Makefile','RELEASE = "Release 6.3"',1
CALL LINEOUT 'Makefile','SHELL = /bin/sh'
CALL LINEOUT 'Makefile','RM = rm -f'
CALL LINEOUT 'Makefile','MV = mv'
CALL LINEOUT 'Makefile','WORLDOPTS = -k'
CALL LINEOUT 'Makefile','TOP = .'
CALL LINEOUT 'Makefile','CURRENT_DIR = .'
CALL LINEOUT 'Makefile','CONFIGSRC = $(TOP)/config'
CALL LINEOUT 'Makefile','IMAKESRC = $(CONFIGSRC)/imake'
CALL LINEOUT 'Makefile','DEPENDSRC = $(CONFIGSRC)/makedepend'
CALL LINEOUT 'Makefile','DEPENDTOP = ../..'
CALL LINEOUT 'Makefile','IMAKETOP = ../..'
CALL LINEOUT 'Makefile','IRULESRC = $(CONFIGSRC)/cf'
CALL LINEOUT 'Makefile','IMAKE = $(IMAKESRC)/imake'
CALL LINEOUT 'Makefile','IMAKE_CMD = $(IMAKE) -I$(IRULESRC) $(IMAKE_DEFINES)'
CALL LINEOUT 'Makefile','MAKE_OPTS = '
CALL LINEOUT 'Makefile','MAKE_CMD = $(MAKE) $(MAKE_OPTS)'
CALL LINEOUT 'Makefile','FLAGS = $(MFLAGS) -f Makefile.ini BOOTSTRAPCFLAGS="$(BOOTSTRAPCFLAGS)"'
CALL LINEOUT 'Makefile',' '
CALL LINEOUT 'Makefile','World.OS2:'
CALL LINEOUT 'Makefile','	@echo :'
CALL LINEOUT 'Makefile','	@echo Building $(RELEASE) of the X Window System on OS/2.'
CALL LINEOUT 'Makefile','	@echo :'
CALL LINEOUT 'Makefile','	@echo :'
CALL LINEOUT 'Makefile','	\indir $(IMAKESRC) $(MAKE) SHELL= -f Makefile.ini clean.os2'
CALL LINEOUT 'Makefile','	\indir $(IMAKESRC) $(MAKE) SHELL= CC=gcc -f Makefile.ini imake.os2'
CALL LINEOUT 'Makefile','	-if exist Makefile.bak del Makefile.bak'
CALL LINEOUT 'Makefile','	-if exist Makefile ren Makefile Makefile.bak'
CALL LINEOUT 'Makefile','	$(subst /,\,$(IMAKE)) -I$(IRULESRC) $(IMAKE_DEFINES) -DTOPDIR=$(TOP) -DCURDIR=$(CURRENT_DIR)'
CALL LINEOUT 'Makefile','	$(MAKE) $(MFLAGS) VerifyOS'
CALL LINEOUT 'Makefile','	$(MAKE) $(MFLAGS) Makefiles'
CALL LINEOUT 'Makefile','	$(MAKE) $(MFLAGS) clean'
CALL LINEOUT 'Makefile','	$(MAKE) $(MFLAGS) includes'
CALL LINEOUT 'Makefile','	$(MAKE) $(MFLAGS) depend'
CALL LINEOUT 'Makefile','	$(MAKE) $(MFLAGS)  '
CALL LINEOUT 'Makefile','	@echo :'
CALL LINEOUT 'Makefile','	@echo :'
CALL LINEOUT 'Makefile','	@echo Full build of $(RELEASE) of the X Window System complete.'
CALL LINEOUT 'Makefile','	@echo :'
CALL LINEOUT 'Makefile',' '
CALL LINEOUT 'Makefile','# dont allow any default rules in this Makefile'
CALL LINEOUT 'Makefile','.SUFFIXES:'
CALL LINEOUT 'Makefile','# quiet "make" programs that display a message if suffix list empty'
CALL LINEOUT 'Makefile','.SUFFIXES: .Dummy'
CALL LINEOUT 'Makefile',' '
CALL LINEOUT 'Makefile','# a copy of every rule that might be invoked at top level'
CALL LINEOUT 'Makefile',' '
CALL LINEOUT 'Makefile','clean:'
CALL LINEOUT 'Makefile','	    $(MAKE_CMD) $@'
CALL LINEOUT 'Makefile','dangerous_strip_clean:'
CALL LINEOUT 'Makefile','	    $(MAKE_CMD) $@'
CALL LINEOUT 'Makefile','depend:'
CALL LINEOUT 'Makefile','	    $(MAKE_CMD) $@'
CALL LINEOUT 'Makefile','Everything:'
CALL LINEOUT 'Makefile','	    $(MAKE_CMD) $@'
CALL LINEOUT 'Makefile','includes:'
CALL LINEOUT 'Makefile','	    $(MAKE_CMD) $@'
CALL LINEOUT 'Makefile','install.man:'
CALL LINEOUT 'Makefile','	    $(MAKE_CMD) $@'
CALL LINEOUT 'Makefile','install:'
CALL LINEOUT 'Makefile','	    $(MAKE_CMD) $@'
CALL LINEOUT 'Makefile','Makefiles:'
CALL LINEOUT 'Makefile','	    $(MAKE_CMD) $@'
CALL LINEOUT 'Makefile','man_keywords:'
CALL LINEOUT 'Makefile','	    $(MAKE_CMD) $@'
CALL LINEOUT 'Makefile','tags:'
CALL LINEOUT 'Makefile','	    $(MAKE_CMD) $@'
CALL LINEOUT 'Makefile','VerifyOS:'
CALL LINEOUT 'Makefile','	    $(MAKE_CMD) $@'
CALL LINEOUT 'Makefile',' '
CALL STREAM 'Makefile','c','close'
RETURN
