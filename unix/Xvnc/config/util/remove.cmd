/* OS/2 REXX script to remove cleaned files
 * run with 'remove dirlist_file'
 * where dirlist_file is be a XFree86 removed-* file
 *
 * $XFree86: xc/config/util/remove.cmd,v 3.0 1994/10/20 06:01:04 dawes Exp $
 */
'@echo off'
file = arg(1)
linein(file,1,0)
curdir = directory()
do while lines(file)=1
  fs = translate(linein(file),'\','/')
  del fs
end
