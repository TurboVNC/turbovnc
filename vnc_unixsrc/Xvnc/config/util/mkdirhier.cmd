/* OS/2 REXX SCRIPT */
/* $XFree86: xc/config/util/mkdirhier.cmd,v 3.2 1996/02/19 12:16:51 dawes Exp $ */

CALL RxFuncAdd 'SysMkDir', 'RexxUtil', 'SysMkDir'

IF ARG() = 0 THEN DO
	SAY "mkdirhier: usage: mkdirhier directory ..."
	EXIT 1
END

curdir=DIRECTORY()
drive=DELSTR(curdir,3)

status=""
PARSE ARG all

DO i=1 TO WORDS(all)
	direc = TRANSLATE(WORD(all,i),'/','\')
	SELECT
	WHEN direc = "" THEN DO
		SAY "mkdirhier: empty directory name"
		status=1
		ITERATE
		END
	WHEN POS('0a'x,direc) > 0 THEN DO
		SAY "mkdirhier: directory name contains a newline: '" direc "'"
		status=1
		ITERATE
		END
	OTHERWISE NOP
	END
	IF POS(":",direc) = 0 THEN direc = INSERT(drive,direc)

	dirbuf.0 = direc
	DO k=1 TO 1000
		direc1 = STRIP(direc,"t","/")
		dpath1 = FILESPEC("path",direc1)
		dirbuf.k=FILESPEC("drive",direc1)||STRIP(dpath1,"t","/")
		IF POS("/",dirbuf.k) = 0 THEN LEAVE k
		direc=dirbuf.k
	END
	DO m=k-1 TO 0 BY -1
		dospath = TRANSLATE(dirbuf.m,'\','/')
		targetdir=DIRECTORY(dospath)
		IF targetdir = dospath THEN
			NOP
		ELSE
			CALL SysMkDir(dospath)
		CALL DIRECTORY curdir
	END
END


EXIT status
