@echo off
rem $XFree86: xc/config/util/instos2.cmd,v 3.0 1996/01/24 21:56:14 dawes Exp $
rem this file is supposed to run from the xc/ dir.
rem you must copy it manually to there before using. It is just here
rem in order not to be in the root dir.
rem
set GCCOPT=-pipe
set EMXLOAD=5
emxload make.exe gcc.exe rm.exe mv.exe
make SHELL= MFLAGS="CC=gcc" install 2>&1 | tee instxc.log
rem
rem
echo INSTALL OF XFREE86/OS2 IS FINISHED
