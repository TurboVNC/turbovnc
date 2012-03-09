/* Rexx OS/2
 * This script serves as a helper cmd file for imake. Install this in
 * the path just like imake itself.
 *
 * $XFree86: xc/config/imake/imakesvc.cmd,v 3.14 2000/04/05 18:13:11 dawes Exp $
 */
'@echo off'
ADDRESS CMD
CALL RxFuncAdd 'SysFileDelete', 'RexxUtil', 'SysFileDelete'
CALL RxFuncAdd 'SysFileTree', 'RexxUtil', 'SysFileTree'
CALL RxFuncAdd 'SysRmDir', 'RexxUtil', 'SysRmDir'
CALL RxFuncAdd 'SysMkDir', 'RexxUtil', 'SysMkDir'

PARSE ARG all
code = WORD(all,1)

SELECT
   WHEN code=1 THEN DO
      /* imakesvc 1 u/n dir ruledir top current */
      instflg = WORD(all,2)
      imakecmd = '\imake'
      ruledir = WORD(all,4)
      topdir = WORD(all,5)
      currentdir = WORD(all,6)
      IF instflg = 'u' THEN DO 
         root = VALUE('X11ROOT',,'OS2ENVIRONMENT')
         IF root = '' THEN DO
            SAY 'Fatal error: no X11ROOT environment variable found!'
            EXIT 99
         END
         imakecmd = 'imake'
	 IF POS(':',ruledir) = 0 THEN
            ruledir1 = root||ruledir
	 ELSE
	    ruledir1 = ruledir
         topdir1 = topdir
         useinst = '-DUseInstalled'
      END 
      ELSE DO
         /* in n mode, we must add a prefix ../ for MakeMakeSubdirs */
         ruledir1 = '../'||ruledir
         topdir1 = '../'||topdir
         useinst = ''
      END

      ruledir = ruledir1
      topdir = topdir1
      curdir = DIRECTORY()
      dir = fixbadprefix(WORD(all,3))
      d = DIRECTORY(dir)
      dirfwd=TRANSLATE(dir,'/','\')
      RC = SysFileDelete('Makefile.bak')
      IF exists('Makefile') THEN REN Makefile Makefile.bak
      /* There is a difficulty in the Imakefiles. Some builds refer
       * to directories that are in a different subtree. We need to adjust
       * the CURDIR and TOPDIR and -I paths
       */
      IF SUBSTR(dirfwd,1,2)='..' THEN DO
         /* must recalculate passed topdir, currentdir, and ruledir */
         ndist = nlevels(topdir)
         ncurdir = './'striplevel(dirfwd,ndist-1)
         ntopdir = maketop(nlevels(ncurdir))
         nruledir = ntopdir||'/config/cf'
      END 
      ELSE DO
         /* this is simple it is relative to this dir */
         pfx = downlevels(dirfwd)
         nruledir = ruledir
         IF instflg = 'n' THEN DO /*sm120296*/
            nruledir = pfx||ruledir
         END
         ntopdir = pfx||topdir
         ncurdir = currentdir  /* use to be pfx || currentdir */
      END
      imakecmd useinst '-I'nruledir' -DTOPDIR='ntopdir' -DCURDIR='ncurdir'/'dirfwd
      'x11make MAKE=x11make SHELL= Makefiles'
      d = DIRECTORY(curdir)
   END
   WHEN code=2 THEN DO
      /* imakesvc 2 buildincdir buildinctop currentdir file */
      bid = WORD(all,3)
      cid = WORD(all,4)
      fil = WORD(all,5)
      curdir = DIRECTORY()
      d = DIRECTORY(WORD(all,2))
      rc = SysFileDelete(fil)
      dir = TRANSLATE(bid'/'cid'/'fil,'\','/')
      COPY dir .' >nul 2>&1 '
      d = DIRECTORY(curdir)
   END
   WHEN code=3 THEN DO
      /* imakesvc 3 subdir updir file */
      sdi = WORD(all,2)
      fil = WORD(all,4)
      curdir = DIRECTORY()
      d = DIRECTORY(WORD(all,3))
      rc = SysFileDelete(fil)
      dir = TRANSLATE(sdi'/'fil,'\','/')
      COPY dir' . >nul 2>&1'
      d = DIRECTORY(curdir)
   END
   WHEN code=4 THEN DO
      /* imakesvc 4 [-r] files... */
      rec = WORD(all,2)
      fp = 2
      IF rec = '-r' THEN fp = 3 
      ELSE rec = '-n'
      DO i=fp TO WORDS(all)
	 CALL discard rec,TRANSLATE(WORD(all,i),'\','/')
      END	
   END
   WHEN code=5 THEN DO
      /* imakesvc 5 file */
      file = TRANSLATE(WORD(all,2),'\','/')
      RC = SysFileDelete(file'.bak')
      if exists(file) THEN 'REN 'file file||'.bak'
   END
   WHEN code=6 THEN DO
      /* imakesvc 6 file */
      file = TRANSLATE(WORD(all,2),'\','/')
      CALL SysFileDelete(file||'.bak')
      if exists(file) THEN 'REN 'file file||'.bak'
   END
   WHEN code=7 THEN DO
      /* imakesvc 7 from to */
      from = TRANSLATE(WORD(all,2),'\','/')
      to = TRANSLATE(WORD(all,3),'\','/')
      CALL SysFileDelete(to)
      COPY from to' >nul 2>&1'
   END
   WHEN code=8 THEN DO
      /* imakesvc 8 arg */
      SAY SUBWORD(TRANSLATE(all,'  ','222c'x),2)
   END
   WHEN code=9 THEN DO
      /* imakesvc 9 dst.c incl.h src.c */
      dst = TRANSLATE(WORD(all,2),'\','/')
      src = TRANSLATE(WORD(all,4),'\','/')
      CALL SysFileDelete(dst)
      CALL LINEOUT dst,'#include "'WORD(all,3)'"'
      CALL LINEOUT dst,'#include "'src'"'
      CALL LINEOUT dst 
   END
   WHEN code=10 THEN DO
      /* imakesvc 10 srcfile destdir destfile suffix */
      src = stripsuffix(WORD(all,2))
      destdir = TRANSLATE(WORD(all,3),'\','/')
      dest = stripsuffix(WORD(all,4))
      suffix = WORD(all,5)
      tgt = destdir'\'dest'.'suffix
      /* if you have no col.exe get one from 4.4BSD */
      'groff -e -t -man -Tascii 'src'.man | col -b >'tgt
   END
   WHEN code=11 THEN DO
      /* imakesvc 11 dirtomake */
      dirtomake = TRANSLATE(WORD(all,2),'\','/')
      rc = SysMkDir(dirtomake)
   END
   WHEN code=12 THEN DO
      /* imakesvc 12 srcfile destdir destfile */
      src = stripsuffix(WORD(all,2))
      destdir = TRANSLATE(WORD(all,3),'\','/')
      dest = stripsuffix(WORD(all,4))
      tgt = destdir'\'dest'.gz'
      /* if you have no col.exe get one from 4.4BSD */
      'groff -e -t -man -Tascii 'src'.man | col -b | gzip -n >'tgt
   END
   WHEN code=13 THEN DO
      /* imakesvc 13 indir fromfile tofile */
      ind = TRANSLATE(WORD(all,2),'\','/')
      frm = TRANSLATE(WORD(all,3),'\','/')
      tof = ind'\'WORD(all,4)
      IF \(exists(ind)) THEN call SysMkDir ind
      rc = SysFileDelete(tof)
      COPY frm' 'tof
   END
   WHEN code=14 THEN DO
      /* imakesvc 14 destdir srcfile... */
      destdir = TRANSLATE(WORD(all,2),'\','/')
      DO i=3 TO WORDS(all)
	src = stripsuffix(WORD(all,i))
	tgt = destdir'\'src'.gz'
	'groff -e -t -man -Tascii 'src'.man | col -b | gzip -n >'tgt
      END
   END
   WHEN code=15 THEN DO
      /* imakesvc 15 destdir suffix srcfile... */
      destdir = TRANSLATE(WORD(all,2),'\','/')
      suffix = WORD(all,3)
      DO i=4 TO WORDS(all)
	src = stripsuffix(WORD(all,i))
	tgt = destdir'\'src'.'suffix
	'groff -e -t -man -Tascii 'src'.man | col -b >'tgt
      END
   END
   WHEN code=16 THEN DO
      /* imakesvc 16 dirlist...*/
      mkfontdir = TRANSLATE(WORD(all,2),'\','/')
      earg=''
      DO i=3 TO WORDS(all)
        arg = WORD(all,i)
        earg = earg' -e 'arg
      END
      mkfontdir' -r -p inst/ 'earg' .'
   END
   OTHERWISE NOP
END
RETURN

downlevels:
oldpos = 1
pfx = ''
DO FOREVER
   newpos = POS('/',ARG(1),oldpos)
   IF newpos = 0 THEN LEAVE
   newpfx = '../'pfx
   oldpos = newpos+1
   pfx = newpfx
END
RETURN pfx

/* returns 1, if file exists */
exists:
'DIR "'arg(1)'" >nul 2>&1'
IF rc = 0 THEN return 1
RETURN 0

discard: PROCEDURE
rec=ARG(1)
files=ARG(2)
IF rec = '-r' THEN DO
   old = DIRECTORY()
   nd = DIRECTORY
   CALL SysFileTree files, 'deld', 'DO'
   IF deld.0 > 0 THEN DO
      DO m=1 TO deld.0
         CALL DIRECTORY deld.m
         CALL discard '-R' .
         CALL DIRECTORY ..
         CALL SysRmDir deld.m
      END 
      CALL SysRmDir files
   END
   CALL SysFileTree files, 'delf', 'FO'
   DO k=1 TO delf.0
      DEL '"'delf.k'"' '>nul 2>&1'
   END
   CALL SysRmDir files
END 
ELSE DO
   DEL '"'files'"' '>nul 2>&1'
END
RETURN

/* somehow make or cmd manages to convert a relative path ..\..\. to ..... */
fixbadprefix:
count = 1
str = ARG(1)
DO WHILE SUBSTR(str,count,2) = '..'
   count = count+1
   str = INSERT('\',str,count)
   count = count+2
END
RETURN str

striplevel:
str=ARG(1)
n=arg(2)
DO count=0 TO n
   p = POS('/',str)
   IF p = 0 THEN LEAVE
   str = DELSTR(str,1,p)
END
RETURN str

nlevels:
str = ARG(1)
count = 0
oldpos = 1
DO FOREVER
   newpos = POS('/',str,oldpos)
   IF newpos = 0 THEN LEAVE
   oldpos = newpos + 1
   count = count + 1
END
RETURN count

maketop:
str = ''
n = ARG(1)
DO k=1 TO n
  str = str||'../'
END
RETURN str||'.'

stripsuffix:
str = ARG(1)
spos = POS('.',str)
IF spos = 0 THEN
   RETURN str
ELSE
   RETURN LEFT(str,spos-1)
