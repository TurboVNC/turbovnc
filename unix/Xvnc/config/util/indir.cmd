/* OS/2 rexx script to emulate the "cd dir; command" mechanism in make
 * which does not work with stupid CMD.EXE
 *
 * $XFree86: xc/config/util/indir.cmd,v 3.1 1996/01/24 21:56:12 dawes Exp $
 */
curdir = directory()
line = fixbadprefix(arg(1))
new = directory(word(line,1))
subword(line,2)
old = directory(curdir)
exit

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

