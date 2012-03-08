/* OS/2 REXX SCRIPT */

/* $XFree86: xc/config/util/os2inst.cmd,v 3.1 1996/02/19 12:16:52 dawes Exp $
 *
 * This is a wrapper for the install command. As any utility, 
 * install could have been lifted from the BSD sources, but I just wanted
 * to play with Rexx a bit :-)  (hv)
 * Note the similarity to the Unix shell scripts
 */

dst=""
src=""

'@ECHO OFF'
PARSE ARG all
DO i=1 TO WORDS(all)
	w = WORD(all,i)
	SELECT
	WHEN w = "-c" THEN NOP
	WHEN w = "-m" THEN
		i = i + 1
		/* ignore mode */
	WHEN w = "-o" THEN
		i = i + 1
		/* ignore owner */
	WHEN w = "-g" THEN
		i = i + 1
		/* ignore group */
	WHEN w = "-s" THEN NOP
	OTHERWISE
		IF src = "" THEN
			src = w
		ELSE
			dst = w
	END
END

IF src = "" THEN DO
	SAY "os2inst:  no input file specified"
	EXIT 1
END

IF dst = "" THEN DO
	SAY "os2inst:  no destination specified"
	EXIT 1
END

ADDRESS CMD 'copy' src dst '> nul'

EXIT
