#!/bin/sh
# $Xorg: mkdirhier.sh,v 1.3 2000/08/17 19:41:53 cpqbld Exp $
# Courtesy of Paul Eggert

newline='
'
IFS=$newline

case ${1--} in
-*) echo >&2 "mkdirhier: usage: mkdirhier directory ..."; exit 1
esac

status=

for directory
do
	case $directory in
	'')
		echo >&2 "mkdirhier: empty directory name"
		status=1
		continue;;
	*"$newline"*)
		echo >&2 "mkdirhier: directory name contains a newline: \`\`$directory''"
		status=1
		continue;;
	///*) prefix=/;; # See Posix 2.3 "path".
	//*) prefix=//;;
	/*) prefix=/;;
	-*) prefix=./;;
	*) prefix=
	esac

	IFS=/
	set x $directory
	case $2 in
	    */*)	# IFS parsing is broken
		IFS=' '
		set x `echo $directory | tr / ' '`
		;;
	esac
	IFS=$newline
	shift

	for filename
	do
		path=$prefix$filename
		prefix=$path/
		shift

		test -d "$path" || {
			paths=$path
			for filename
			do
				if [ -n "$filename" -a "$filename" != "." ]; then
					path=$path/$filename
					paths=$paths$newline$path
				fi
			done

			mkdir $paths || status=$?

			break
		}
	done
  done

exit $status
