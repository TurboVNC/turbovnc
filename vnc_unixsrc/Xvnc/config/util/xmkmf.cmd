/* OS/2 REXX */
/* make a Makefile from an Imakefile from inside or outside the sources
 * 
 * $XFree86: xc/config/util/xmkmf.cmd,v 3.1 1996/04/15 11:14:27 dawes Exp $
 */
'@ECHO OFF'

/* this is actually used here */
x11root = getenv('X11ROOT')

/* these ones are checked only, because later scripts rely on their existance */
libpath = getenv('LIBRARY_PATH')
incpath = getenv('C_INCLUDE_PATH')

/* get args */
PARSE ARG arg0 arg1 arg2 arg3

/* from here almost everything is the same as in the xmkmf sh script */
topdir = ''
curdir = '.'
do_all = 0

IF arg0 = '-a' THEN DO
	do_all = 1
	arg0 = arg1
	arg1 = arg2
	arg2 = arg3
END
ELSE DO
	do_all = 0
END

IF \(arg0 = '') THEN DO
	IF \(arg1 = '') THEN
		curdir = arg1
	topdir = arg0
END
IF \(arg2 = '') | SUBSTR(topdir,1,1) = '-' THEN DO
	SAY 'usage:  xmkmf [-a] [top_of_sources_pathname [current_directory]]'
	EXIT
END

IF exists('Makefile') THEN DO
	SAY 'mv -f Makefile Makefile.bak'
	'COPY Makefile Makefile.bak >nul 2>&1'
	'DEL Makefile >nul 2>&1'
END

IF topdir = '' THEN
	iargs = '-DUseInstalled -I'x11root'/XFree86/lib/X11/config'
ELSE
	iargs = '-I'topdir'/config/cf -DTOPDIR='topdir' -DCURDIR='curdir

SAY 'imake 'iargs
IF do_all = 1 THEN DO
	'imake 'iargs
	SAY 'make Makefiles'
	CALL make 'Makefiles'
	SAY 'make includes'
	CALL make 'includes'
	SAY 'make depend'
	CALL make 'depend'
END
ELSE
	'imake 'iargs

EXIT

exists:
'DIR "'ARG(1)'" > nul 2>nul'
IF rc = 0 THEN RETURN 1
RETURN 0

getenv:
x = VALUE(ARG(1),,'OS2ENVIRONMENT')

IF x = '' THEN DO
	SAY 'No 'ARG(1)' environment variable set!'
	EXIT
END
RETURN x
