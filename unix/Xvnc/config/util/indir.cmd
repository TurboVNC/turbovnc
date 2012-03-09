/* OS/2 rexx script to emulate the "cd dir; command" mechanism in make
 * which does not work with stupid CMD.EXE
 *
 * $XFree86: xc/config/util/indir.cmd,v 3.2 2000/04/05 18:13:13 dawes Exp $
 */
curdir = directory()
line = fixbadprefix(ARG(1))
w1 = TRANSLATE(WORD(line,1),'\','/')
new = directory(w1)
/*IF (SUBSTR(w1,1,2) = '..') | (POS(w1,new) > 0) THEN DO*/
  subword(line,2)
  old = directory(curdir)
/*END
ELSE DO
  say 'Directory 'new' does not exist, ignoring command (nonfatal)'
END*/
EXIT

/* somehow make or cmd manages to convert a relative path ..\..\. to ..... */
fixbadprefix:
count = 1
str = ARG(1)
DO WHILE SUBSTR(str,count,3) = '...'
   count = count+1
   str = INSERT('\',str,count)
   count = count+2
END
RETURN str
