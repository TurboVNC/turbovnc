/* $Xorg: imakemdep.h,v 1.6 2001/02/09 02:03:16 xorgcvs Exp $ */
/* $XdotOrg: xc/config/imake/imakemdep.h,v 1.12 2005/11/08 06:33:24 jkj Exp $ */
/*

Copyright (c) 1993, 1994, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

*/
/* $XFree86: xc/config/imake/imakemdep.h,v 3.71 2003/06/12 14:12:26 eich Exp $ */


/* 
 * This file contains machine-dependent constants for the imake utility.
 * When porting imake, read each of the steps below and add in any necessary
 * definitions.  In general you should *not* edit ccimake.c or imake.c!
 */

#ifdef __UNIXOS2__
#define lstat stat
#endif

#ifdef CCIMAKE
/*
 * Step 1:  imake_ccflags
 *     Define any special flags that will be needed to get imake.c to compile.
 *     These will be passed to the compile along with the contents of the
 *     make variable BOOTSTRAPCFLAGS.
 */
#if defined(clipper) || defined(__clipper__)
#define imake_ccflags "-O -DSYSV -DBOOTSTRAPCFLAGS=-DSYSV"
#endif

#ifdef hpux
#ifdef hp9000s800
#define imake_ccflags "-DSYSV"
#else
#define imake_ccflags "-Wc,-Nd4000,-Ns3000 -DSYSV"
#endif
#endif

#if defined(macII) || defined(_AUX_SOURCE)
#define imake_ccflags "-DmacII -DSYSV"
#endif

#ifdef stellar
#define imake_ccflags "-DSYSV"
#endif

/*
 * SCO UnixWare and OpenServer 6 are both System V Release 5 based OSes.
 * The native C compiler doesn't assert __UNIXWARE__ but gcc does, so
 * we don't redefine it if we are using gcc (as it sets it to a specific
 * value). On OpenServer 6, which is a multi-ABI world, if you attempt
 * to build with -Kosr, then the C compiler will assert __OPENSERVER__
 * and set it to the value 507.  That indicates an OSR5 compile, and
 * is handled below.
 */

#if defined(__UNIXWARE__) || defined(__USLC__) || defined(Oki) || defined(NCR)
# ifdef imake_ccflags
#  undef imake_ccflags
# endif
# ifdef __UNIXWARE__
#  ifndef __GNUC__
#   define imake_ccflags "-Xa -DSVR4 -DSVR5 -D__UNIXWARE__"
#  else
#   define imake_ccflags "-Xa -DSVR4 -DSVR5"
#  endif
# else
#  define imake_ccflags "-Xa -DSVR4"
# endif
#endif

/* SCO may define __USLC__ so put this after the USL check */
#if defined(M_UNIX) || defined(_SCO_DS) || defined(__OPENSERVER__)
# ifdef imake_ccflags
#  undef imake_ccflags
# endif
# define imake_ccflags "-DSYSV -DSCO325 -D__SCO__"
#endif

#ifdef sony
#if defined(SYSTYPE_SYSV) || defined(_SYSTYPE_SYSV)
#define imake_ccflags "-DSVR4"
#else
#include <sys/param.h>
#if NEWSOS < 41
#define imake_ccflags "-Dbsd43 -DNOSTDHDRS"
#else
#if NEWSOS < 42
#define imake_ccflags "-Dbsd43"
#endif
#endif
#endif
#endif
#ifdef _CRAY
#define imake_ccflags "-DSYSV -DUSG"
#endif

#if defined(_IBMR2) || defined(aix)
#define imake_ccflags "-Daix -DSYSV"
#endif

#ifdef Mips
#  if defined(SYSTYPE_BSD) || defined(BSD) || defined(BSD43)
#    define imake_ccflags "-DBSD43"
#  else 
#    define imake_ccflags "-DSYSV"
#  endif
#endif 

#ifdef is68k
#define imake_ccflags "-Dluna -Duniosb"
#endif

#ifdef SYSV386
# ifdef SVR4
#  define imake_ccflags "-Xa -DSVR4"
# else
#  define imake_ccflags "-DSYSV"
# endif
#endif

#ifdef SVR4
# ifdef i386
#  define imake_ccflags "-Xa -DSVR4"
# endif
#endif

#ifdef SYSV
# ifdef i386
#  define imake_ccflags "-DSYSV"
# endif
#endif

#if defined(Lynx) || defined(__Lynx__)
#define imake_ccflags "-DLynx"
#endif /* Lynx */

#ifdef __convex__
#define imake_ccflags "-fn -tm c1"
#endif

#ifdef apollo
#define imake_ccflags "-DX_NOT_POSIX"
#endif

#ifdef WIN32
#ifdef __GNUC__
#define imake_ccflags "-D__STDC__"
#else
#if _MSC_VER < 1000
#define imake_ccflags "-nologo -batch -D__STDC__"
#else
#define imake_ccflags "-nologo -D__STDC__"
#endif
#endif
#endif

#ifdef __uxp__
#define imake_ccflags "-DSVR4 -DANSICPP"
#endif

#ifdef __sxg__
#define imake_ccflags "-DSYSV -DUSG -DNOSTDHDRS"
#endif

#ifdef _SEQUENT_
#define imake_ccflags "-Xa -DSVR4"
#endif

#if defined(SX) || defined(PC_UX)
#define imake_ccflags "-DSYSV"
#endif

#ifdef nec_ews_svr2
#define imake_ccflags "-DUSG"
#endif

#if defined(nec_ews_svr4) || defined(_nec_ews_svr4) || defined(_nec_up) || defined(_nec_ft)
#define imake_ccflags "-DSVR4"
#endif

#if defined(MACH) && !defined(__GNU__)
#define imake_ccflags "-DNOSTDHDRS"
#endif

/* this is for OS/2 under UNIXOS2. This won't work with DOS */
#if defined(__UNIXOS2__)
#define imake_ccflags "-DBSD43"
#endif

#if defined(__QNX__) && !defined(__QNXNTO__)
#define imake_ccflags "-D__QNX__ -D_i386"
#endif

#if defined(__QNXNTO__)
#define imake_ccflags "-D__QNXNTO__"
#endif

#else /* not CCIMAKE */
#ifndef MAKEDEPEND
/*
 * Step 2:  dup2
 *     If your OS doesn't have a dup2() system call to duplicate one file
 *     descriptor onto another, define such a mechanism here (if you don't
 *     already fall under the existing category(ies).
 */
#if defined(SYSV) && !defined(_CRAY) && !defined(Mips) && !defined(_SEQUENT_) && !defined(__SCO__)
#define	dup2(fd1,fd2)	((fd1 == fd2) ? fd1 : (close(fd2), \
					       fcntl(fd1, F_DUPFD, fd2)))
#endif


/*
 * Step 3:  FIXUP_CPP_WHITESPACE
 *     If your cpp collapses tabs in macro expansions into a single space and
 *     replaces escaped newlines with a space, define this symbol.  This will
 *     cause imake to attempt to patch up the generated Makefile by looking
 *     for lines that have colons in them (this is why the rules file escapes
 *     all colons).  One way to tell if you need this is to see whether or not
 *     your Makefiles have no tabs in them and lots of @@ strings.
 */
#if defined(sun) || defined(SYSV) || defined(SVR4) || defined(hcx) || defined(WIN32) || defined(__SCO__) || (defined(AMOEBA) && defined(CROSS_COMPILE)) || defined(__QNX__) || defined(__sgi) || defined(__UNIXOS2__) || defined(__UNIXWARE__)
#define FIXUP_CPP_WHITESPACE
#endif
#ifdef WIN32
#define REMOVE_CPP_LEADSPACE
#define INLINE_SYNTAX
#define MAGIC_MAKE_VARS
#endif
#ifdef __minix_vmd
#define FIXUP_CPP_WHITESPACE
#endif

#if defined(Lynx)
/* On LynxOS 2.4.0 imake gets built with the old "legacy"
 * /bin/cc which has a rather pedantic builtin preprocessor.
 * Using a macro which is not #defined (as in Step 5
 * below) flags an *error*
 */
#define __NetBSD_Version__ 0
#endif

/*
 * Step 4:  USE_CC_E, DEFAULT_CC, DEFAULT_CPP
 *     If you want to use cc -E instead of cpp, define USE_CC_E.
 *     If use cc -E but want a different compiler, define DEFAULT_CC.
 *     If the cpp you need is not in /lib/cpp, define DEFAULT_CPP.
 */
#if !defined (CROSSCOMPILE) || defined (CROSSCOMPILE_CPP) 

#if defined(__APPLE__)
#define DEFAULT_CPP "/usr/bin/cpp"
#define DEFAULT_CC "cc"
#endif
#if defined(Lynx) || defined(__Lynx__)
#define DEFAULT_CC "gcc"
#define USE_CC_E
#endif
#ifdef hpux
#define USE_CC_E
#endif
#ifdef WIN32
#define USE_CC_E
#ifdef __GNUC__
#define DEFAULT_CC "gcc"
#else
#define DEFAULT_CC "cl"
#endif
#endif
#ifdef apollo
#define DEFAULT_CPP "/usr/lib/cpp"
#endif
#if defined(clipper) || defined(__clipper__)
#define DEFAULT_CPP "/usr/lib/cpp"
#endif
#if defined(_IBMR2) && !defined(DEFAULT_CPP)
#define DEFAULT_CPP "/usr/ccs/lib/cpp"
#endif
#if defined(sun) && (defined(SVR4) || defined(__svr4__) || defined(__SVR4) || defined(__sol__))
#define DEFAULT_CPP "/usr/ccs/lib/cpp"
#endif
#ifdef __bsdi__
#define DEFAULT_CPP "/usr/bin/cpp"
#endif
#ifdef __uxp__
#define DEFAULT_CPP "/usr/ccs/lib/cpp"
#endif
#ifdef __sxg__
#define DEFAULT_CPP "/usr/lib/cpp"
#endif
#ifdef _CRAY
#define DEFAULT_CPP "/lib/pcpp"
#endif
#if defined(__386BSD__)
#define DEFAULT_CPP "/usr/libexec/cpp"
#endif
#if defined(__FreeBSD__)  || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
#define USE_CC_E
#endif
#if defined(__sgi) && defined(__ANSI_CPP__)
#define USE_CC_E
#endif
#if defined(MACH) && !defined(__GNU__)
#define USE_CC_E
#endif
#ifdef __minix_vmd
#define DEFAULT_CPP "/usr/lib/cpp"
#endif
#if defined(__UNIXOS2__)
/* expects cpp in PATH */
#define DEFAULT_CPP "cpp"
#endif
#ifdef __CYGWIN__
#define DEFAULT_CC "gcc"
#define DEFAULT_CPP "/usr/bin/cpp"
#endif
#if defined (__QNX__)
#ifdef __QNXNTO__
#define DEFAULT_CPP "/usr/bin/cpp"
#else
#define DEFAULT_CPP "/usr/X11R6/bin/cpp"
#endif
#endif
#if defined(__GNUC__) && !defined(USE_CC_E)
#define USE_CC_E
#ifndef DEFAULT_CC
#define DEFAULT_CC "gcc"
#endif
#endif

#endif /* !defined (CROSSCOMPILE) || defined (CROSSCOMPILE_CPP) */
/*
 * Step 5:  cpp_argv
 *     The following table contains the flags that should be passed
 *     whenever a Makefile is being generated.  If your preprocessor 
 *     doesn't predefine any unique symbols, choose one and add it to the
 *     end of this table.  Then, do the following:
 * 
 *         a.  Use this symbol in Imake.cf when setting MacroFile.
 *         b.  Put this symbol in the definition of BootstrapCFlags in your
 *             <platform>.cf file.
 *         c.  When doing a make World, always add "BOOTSTRAPCFLAGS=-Dsymbol" 
 *             to the end of the command line.
 * 
 *     Note that you may define more than one symbol (useful for platforms 
 *     that support multiple operating systems).
 */

#define	ARGUMENTS 50	/* number of arguments in various arrays */
#if !defined (CROSSCOMPILE) || defined (CROSSCOMPILE_CPP) 
char *cpp_argv[ARGUMENTS] = {
	"cc",		/* replaced by the actual program to exec */
	"-I.",		/* add current directory to include path */
#if !defined(__NetBSD_Version__) || __NetBSD_Version__ < 103080000
#ifdef unix
	"-Uunix",	/* remove unix symbol so that filename unix.c okay */
#endif
#endif
#if defined(__386BSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || \
    defined(__FreeBSD__) || defined(__DragonFly__) || defined(MACH) || \
    defined(linux) || defined(__GNU__) || defined(__bsdi__) || \
    defined(__GNUC__) || defined(__GLIBC__)
# ifdef __i386__
	"-D__i386__",
#  if defined(__GNUC__) && (__GNUC__ >= 3)
	"-m32",
#  endif
# endif
# ifdef __i486__
	"-D__i486__",
# endif
# ifdef __i586__
	"-D__i586__",
# endif
# ifdef __i686__
	"-D__i686__",
# endif
# ifdef __k6__
	"-D__k6__",
# endif
# ifdef __ia64__
	"-D__ia64__",
# endif
# ifdef __amd64__
	"-D__amd64__",
# endif
# ifdef __x86_64__
	"-D__amd64__",
# endif
# ifdef __s390__
	"-D__s390__",
# endif
# ifdef __alpha__
	"-D__alpha__",
# endif
# ifdef __arm__
	"-D__arm__",
# endif
# ifdef __s390x__
       "-D__s390x__",
# endif
# ifdef __sparc__
	"-D__sparc__",
# endif
# ifdef __m68k__
	"-D__m68k__",
# endif
# ifdef __hppa__
	"-D__hppa__",
# endif
# ifdef __sh__
	"-D__sh__",
# endif
# ifdef __sh3__
	"-D__sh3__",
# endif
# ifdef __SH3__
	"-D__SH3__",
# endif
# ifdef __SH4__
	"-D__SH4__",
# endif
# ifdef __SH4NOFPU__
	"-D__SH4_NOFPU__",
# endif
# ifdef __GNUC__
	"-traditional",
# endif
#endif
#ifdef M4330
	"-DM4330",	/* Tektronix */
#endif
#ifdef M4310
	"-DM4310",	/* Tektronix */
#endif
#if defined(macII) || defined(_AUX_SOURCE)
	"-DmacII",	/* Apple A/UX */
#endif
#if defined(USL) || defined(__UNIXWARE__) || \
	(defined(__USLC__) && !defined(_SCO_DS))
	"-D__UNIXWARE__",	/* SCO UnixWare 7 */
#endif
#ifdef sony
	"-Dsony",	/* Sony */
#if !defined(SYSTYPE_SYSV) && !defined(_SYSTYPE_SYSV) && NEWSOS < 42
	"-Dbsd43",
#endif
#endif
#ifdef _IBMR2
	"-D_IBMR2",	/* IBM RS-6000 (we ensured that aix is defined above */
#ifndef aix
#define aix		/* allow BOOTSTRAPCFLAGS="-D_IBMR2" */
#endif
#endif /* _IBMR2 */
#ifdef aix
	"-Daix",	/* AIX instead of AOS */
#ifndef ibm
#define ibm		/* allow BOOTSTRAPCFLAGS="-Daix" */
#endif
#endif /* aix */
#ifdef ibm
	"-Dibm",	/* IBM PS/2 and RT under both AOS and AIX */
#endif
#ifdef luna
	"-Dluna",	/* OMRON luna 68K and 88K */
#ifdef luna1
	"-Dluna1",
#endif
#ifdef luna88k		/* need not on UniOS-Mach Vers. 1.13 */
	"-traditional", /* for some older version            */
#endif			/* instead of "-DXCOMM=\\#"          */
#ifdef uniosb
	"-Duniosb",
#endif
#ifdef uniosu
	"-Duniosu",
#endif
#endif /* luna */
#ifdef _CRAY		/* Cray */
	"-Ucray",
#endif
#ifdef Mips
	"-DMips",	/* Define and use Mips for Mips Co. OS/mach. */
# if defined(SYSTYPE_BSD) || defined(BSD) || defined(BSD43)
	"-DBSD43",	/* Mips RISCOS supports two environments */
# else
	"-DSYSV",	/* System V environment is the default */
# endif
#endif /* Mips */
#ifdef MOTOROLA
	"-DMOTOROLA",    /* Motorola Delta Systems */
# ifdef SYSV
	"-DSYSV", 
# endif
# ifdef SVR4
	"-DSVR4",
# endif
#endif /* MOTOROLA */
#if defined(M_UNIX) || defined(sco) || defined(__SCO__) || \
	defined(_SCO_DS) || defined(__OPENSERVER__)
	"-D__SCO__",
	"-DSYSV",
#endif
#ifdef i386
	"-Di386",
# ifdef SVR4
	"-DSVR4",
# endif
# ifdef SYSV
	"-DSYSV",
#  ifdef ISC
	"-DISC",
#   ifdef ISC40
	"-DISC40",       /* ISC 4.0 */
#   else
#    ifdef ISC202
	"-DISC202",      /* ISC 2.0.2 */
#    else
#     ifdef ISC30
	"-DISC30",       /* ISC 3.0 */
#     else
	"-DISC22",       /* ISC 2.2.1 */
#     endif
#    endif
#   endif
#  endif
#  if (_SCO_DS - 0 == 1)
    "-DSCO325",
#  endif
#  if (_SCO_DS - 0 > 1)
    "-DSCO5V6",
#  endif
# endif
# ifdef ESIX
	"-DESIX",
# endif
# ifdef ATT
	"-DATT",
# endif
# ifdef DELL
	"-DDELL",
# endif
#endif
#ifdef SYSV386           /* System V/386 folks, obsolete */
	"-Di386",
# ifdef SVR4
	"-DSVR4",
# endif
# ifdef ISC
	"-DISC",
#  ifdef ISC40
	"-DISC40",       /* ISC 4.0 */
#  else
#   ifdef ISC202
	"-DISC202",      /* ISC 2.0.2 */
#   else
#    ifdef ISC30
	"-DISC30",       /* ISC 3.0 */
#    else
	"-DISC22",       /* ISC 2.2.1 */
#    endif
#   endif
#  endif
# endif
# if (_SCO_DS - 0 == 1)
	"-DSCO325",
# endif
#  if (_SCO_DS - 0 > 1)
    "-DSCO5V6",
#  endif
# ifdef ESIX
	"-DESIX",
# endif
# ifdef ATT
	"-DATT",
# endif
# ifdef DELL
	"-DDELL",
# endif
#endif
#ifdef __osf__
	"-D__osf__",
# ifdef __mips__
	"-D__mips__",
# endif
# ifdef __alpha
	"-D__alpha",
# endif
# ifdef __amiga__
	"-D__amiga__",
# endif
# ifdef __alpha__
	"-D__alpha__",
# endif
# ifdef __i386__
	"-D__i386__",
# endif
# ifdef __GNUC__
	"-traditional",
# endif
#endif
#ifdef Oki
	"-DOki",
#endif
#ifdef sun
#if defined(SVR4) || defined(__svr4__) || defined(__SVR4) || defined(__sol__)
	"-DSVR4",
#endif
# ifdef __sparcv9
	"-D__sparcv9",
# endif
# ifdef __amd64
	"-D__amd64",
# endif
#endif
#ifdef WIN32
	"-DWIN32",
#ifndef __GNUC__
	"-nologo",
#if _MSC_VER < 1000
	"-batch",
#endif
#endif
	"-D__STDC__",
#endif
#ifdef NCR
	"-DNCR",	/* NCR */
#endif
#ifdef linux
        "-Dlinux",
#endif
#if defined(__CYGWIN__)
        "-traditional",
#endif
#if defined(Lynx) || defined(__Lynx__)
        "-traditional",
#if 0
        "-DLYNX",		/* do we really need this?? */
#endif
	"-DLynx",
# ifdef ppc
	"-Dppc",
# endif
# ifdef ppc64
	"-Dppc64",
# endif
# if defined(m68k)  || defined(M68k) || defined(m68040)
	"-Dm68k",
	"-DM68k",
# endif
# ifdef uSPARC1
	"-Dsparc",
# endif
#endif
#ifdef __uxp__
	"-D__uxp__",
#endif
#ifdef __sxg__
	"-D__sxg__",
#endif
#ifdef nec_ews_svr2
	"-Dnec_ews_svr2",
#endif
#ifdef AMOEBA
	"-DAMOEBA",
# ifdef CROSS_COMPILE
	"-DCROSS_COMPILE",
#  ifdef CROSS_i80386
	"-Di80386",
#  endif
#  ifdef CROSS_sparc
	"-Dsparc",
#  endif
#  ifdef CROSS_mc68000
	"-Dmc68000",
#  endif
# else
#  ifdef i80386
	"-Di80386",
#  endif
#  ifdef sparc
	"-Dsparc",
#  endif
#  ifdef mc68000
	"-Dmc68000",
#  endif
# endif
#endif
#if defined(__sgi) && defined(__ANSI_CPP__)
	"-cckr",
#endif
#ifdef __minix_vmd
	"-Dminix",
#endif

#if defined(__UNIXOS2__)
	"-traditional",
	"-Demxos2",
#endif
#ifdef MetroLink
	"-DMetroLink",
# ifdef SVR4
	"-DSVR4",
# endif
#endif
#ifdef __powerpc__
	"-D__powerpc__",
#endif
#ifdef __powerpc64__
	"-D__powerpc64__",
#endif
#ifdef PowerMAX_OS
	"-DPowerMAX_OS",
#endif
#if defined (__QNX__) && !defined(__QNXNTO__)
        "-traditional",
        "-D__QNX__",
#endif

#if defined(__QNXNTO__)
        "-traditional",
        "-D__QNXNTO__",
#if defined(i386)
        "-Di386",
#endif
#if defined(__i386__)
        "-D__i386__",
#endif
#if defined(PPC)
        "-DPPC",
#endif
#if defined(MIPS)
        "-DMIPS",
#endif
#endif

#if defined(__APPLE__)
        "-D__APPLE__",
        "-D__DARWIN__",
# ifdef __ppc__
        "-D__ppc__",
# endif
# ifdef __ppc64__
        "-D__ppc64__",
# endif
# ifdef __i386__
        "-D__i386__",
# endif
#endif
};
#endif /* CROSSCOMPILE */


/*
 * Step 6: DEFAULT_OS_MAJOR_REV, DEFAULT_OS_MINOR_REV, DEFAULT_OS_TEENY_REV,
 *	and DEFAULT_OS_NAME.
 *	If your system provides a way to generate the default major,
 *	minor, teeny, or system names at runtime add commands below.
 *	The syntax of the _REV strings is 'f fmt' where 'f' is an argument
 *	you would give to uname, and "fmt" is a scanf() format string.
 *	Supported uname arguments are "snrvm", and if you specify multiple
 *	arguments they will be separated by spaces.  No more than 5 arguments
 *	may be given.  Unlike uname() order of arguments matters.
 *
 *	DEFAULT_OS_MAJOR_REV_FROB, DEFAULT_OS_MINOR_REV_FROB,
 *	DEFAULT_OS_TEENY_REV_FROB, and DEFAULT_OS_NAME_FROB can be used to
 *	modify the results of the use of the various strings.
 */
#if !defined CROSSCOMPILE || defined CROSSCOMPILE_CPP
# if defined(aix)
/* uname -v returns "x" (e.g. "4"), and uname -r returns "y" (e.g. "1") */
#  define DEFAULT_OS_MAJOR_REV	"v %[0-9]"
#  define DEFAULT_OS_MINOR_REV	"r %[0-9]"
/* No information available to generate default OSTeenyVersion value. */
#  define DEFAULT_OS_NAME	"srvm %[^\n]"
# elif defined(sun) || defined(sgi) || defined(ultrix) || defined(__uxp__) || defined(sony)
/* uname -r returns "x.y[.z]", e.g. "5.4" or "4.1.3" */
#  define DEFAULT_OS_MAJOR_REV	"r %[0-9]"
#  define DEFAULT_OS_MINOR_REV	"r %*d.%[0-9]"
#  define DEFAULT_OS_TEENY_REV	"r %*d.%*d.%[0-9]"
#  define DEFAULT_OS_NAME	"srvm %[^\n]"
# elif defined(hpux)
/* uname -r returns "W.x.yz", e.g. "B.10.01" */
#  define DEFAULT_OS_MAJOR_REV	"r %*[^.].%[0-9]"
#  define DEFAULT_OS_MINOR_REV	"r %*[^.].%*d.%1s"
#  define DEFAULT_OS_TEENY_REV	"r %*[^.].%*d.%*c%[0-9]"
#  define DEFAULT_OS_NAME	"srvm %[^\n]"
# elif defined(USL) || defined(__USLC__) || defined(__UNIXWARE__) || \
	defined(__SCO__) || defined(__OPENSERVER__) || defined(_SCO_DS)
/* uname -v returns "x.yz" or "x.y.z", e.g. "2.02" or "2.1.2". */
#  define DEFAULT_OS_MAJOR_REV	"v %[0-9]"
#  define DEFAULT_OS_MINOR_REV	"v %*d.%1s"
#  define DEFAULT_OS_TEENY_REV	"v %*d.%*c%[.0-9]"
#  define DEFAULT_OS_NAME	"srvm %[^\n]"
# elif defined(__APPLE__)
/* uname -v returns "x.yz" or "x.y.z", e.g. "2.02" or "2.1.2". */
#  define DEFAULT_OS_MAJOR_REV	"r %[0-9]"
#  define DEFAULT_OS_MINOR_REV	"r %*d.%[0-9]"
#  define DEFAULT_OS_TEENY_REV	"r %*d.%*d.%[0-9]" /* this will just get 0 */
#  define DEFAULT_OS_NAME	"s %[^\n]"
# elif defined(__osf__)
/* uname -r returns "Wx.y", e.g. "V3.2" or "T4.0" */
#  define DEFAULT_OS_MAJOR_REV	"r %*[^0-9]%[0-9]"
#  define DEFAULT_OS_MINOR_REV	"r %*[^.].%[0-9]"
#  define DEFAULT_OS_NAME	"srvm %[^\n]"
# elif defined(__uxp__)
/* NOTE: "x.y[.z]" above handles UXP/DF.  This is a sample alternative. */
/* uname -v returns "VxLy Yzzzzz ....", e.g. "V20L10 Y95021 Increment 5 ..." */
#  define DEFAULT_OS_MAJOR_REV	"v V%[0-9]"
#  define DEFAULT_OS_MINOR_REV	"v V%*dL%[0-9]"
#  define DEFAULT_OS_NAME	"srvm %[^\n]"
# elif defined(linux) || defined(__bsdi__)
#  define DEFAULT_OS_MAJOR_REV	"r %[0-9]"
#  define DEFAULT_OS_MINOR_REV	"r %*d.%[0-9]"
#  define DEFAULT_OS_TEENY_REV	"r %*d.%*d.%[0-9]"
#  define DEFAULT_OS_NAME	"srm %[^\n]"
#  if defined(linux) && defined  (CROSSCOMPILE_CPP)
#   define CROSS_UTS_SYSNAME "Linux"
#   include <linux/version.h>
#   define CROSS_UTS_RELEASE UTS_RELEASE
# endif
# elif defined(__CYGWIN__)
#  define DEFAULT_OS_MAJOR_REV	"r %[0-9]"
#  define DEFAULT_OS_MINOR_REV	"r %*d.%[0-9]"
#  define DEFAULT_OS_TEENY_REV	"r %*d.%*d.%[0-9]"
#  define DEFAULT_OS_NAME	"srm %[^\n]"
#  if defined(__CYGWIN__) && defined  (CROSSCOMPILE_CPP)
#   define CROSS_UTS_SYSNAME "Cygwin"
#   include <cygwin/version.h>
#   define CROSS_UTS_RELEASE "1.3.12"
#  endif
# elif defined(__GNU__)
#  define DEFAULT_OS_MAJOR_REV	"r %[0-9]"
#  define DEFAULT_OS_MINOR_REV	"r %*d.%[0-9]"
#  define DEFAULT_OS_NAME	"srm %[^\n]"
# elif defined(ISC)
/* ISC all Versions ? */
/* uname -r returns "x.y", e.g. "3.2" ,uname -v returns "x" e.g. "2" */
#  define DEFAULT_OS_MAJOR_REV   "r %[0-9]"
#  define DEFAULT_OS_MINOR_REV   "r %*d.%[0-9]"
#  define DEFAULT_OS_TEENY_REV   "v %[0-9]" 
/* # define DEFAULT_OS_NAME        "srm %[^\n]" */ /* Not useful on ISC */
# elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__OpenBSD__) || defined(__DragonFly__)
/* BSD/OS too? */
/* uname -r returns "x.y[.z]-mumble", e.g. "2.1.5-RELEASE" or "2.2-0801SNAP" */
#  define DEFAULT_OS_MAJOR_REV   "r %[0-9]"
#  define DEFAULT_OS_MINOR_REV   "r %*d.%[0-9]"
#  define DEFAULT_OS_TEENY_REV   "r %*d.%*d.%[0-9]" 
#  define DEFAULT_OS_NAME        "srm %[^\n]"
#  if defined(__FreeBSD__) || defined(__DragonFly__)
/* Use an alternate way to find the teeny version for -STABLE, -SNAP versions */
#   ifndef CROSSCOMPILE_CPP
#    define DEFAULT_OS_TEENY_REV_FROB(buf, size)			\
      do {								\
	if (*buf == 0) {						\
		int __mib[2];						\
		size_t __len;						\
		int __osrel;						\
									\
		__mib[0] = CTL_KERN;					\
		__mib[1] = KERN_OSRELDATE;				\
		__len = sizeof(__osrel);				\
		sysctl(__mib, 2, &__osrel, &__len, NULL, 0);		\
		if (__osrel < 210000) {					\
			if (__osrel < 199607)				\
				buf[0] = '0';				\
			else if (__osrel < 199612)			\
				buf[0] = '5';				\
			else if (__osrel == 199612)			\
				buf[0] = '6';				\
			else						\
				buf[0] = '8'; /* guess */		\
		} else {						\
			buf[0] = ((__osrel / 1000) % 10) + '0';		\
		}							\
		buf[1] = 0;						\
	}								\
     } while (0)
#   endif
#  else
   /* OpenBSD - Add DEFAULT_MACHINE_ARCHITECTURE */
#   define DEFAULT_MACHINE_ARCHITECTURE "m %[^\n]"
#  endif
# elif defined(__NetBSD__)
/*
 * uname -r returns "x.y([ABCD...]|_mumble)", e.g.:
 *	1.2	1.2_BETA	1.2A	1.2B
 *
 * That means that we have to do something special to turn the
 * TEENY revision into a form that we can use (i.e., a string of
 * decimal digits).
 *
 * We also frob the name DEFAULT_OS_NAME so that it looks like the
 * 'standard' NetBSD name for the version, e.g. "NetBSD/i386 1.2B" for
 * NetBSD 1.2B on an i386.
 */
#  define DEFAULT_OS_MAJOR_REV   "r %[0-9]"
#  define DEFAULT_OS_MINOR_REV   "r %*d.%[0-9]"
#  define DEFAULT_OS_TEENY_REV   "r %*d.%*d%[A-Z]" 
#  define DEFAULT_OS_TEENY_REV_FROB(buf, size)				\
    do {								\
	int	teeny = 0;						\
	char	*ptr = (buf);						\
									\
	while (*ptr >= 'A' && *ptr <= 'Z') /* sanity check */		\
	    teeny = teeny * 26 + (int)(*ptr++ - 'A');			\
									\
	snprintf((buf), (size), "%d", teeny + 1);			\
    } while (0)
#  define DEFAULT_OS_NAME        "smr %[^\n]"
#  define DEFAULT_OS_NAME_FROB(buf, size)				\
    do {								\
	char *__sp;							\
	if ((__sp = strchr((buf), ' ')) != NULL)			\
		*__sp = '/';						\
    } while (0)
# elif defined(__Lynx__) || defined(Lynx)
/* Lynx 2.4.0 /bin/cc doesn't like #elif */
#  define DEFAULT_OS_MAJOR_REV   "r %[0-9]"
#  define DEFAULT_OS_MINOR_REV   "r %*d.%[0-9]"
#  define DEFAULT_OS_TEENY_REV   "r %*d.%*d.%[0-9]" 
#  define DEFAULT_OS_NAME        "srm %[^\n]"
# elif defined(_SEQUENT_)
/* uname -v returns 'Vx.y.z', e.g. 'V4.4.2' */
#  define DEFAULT_OS_MAJOR_REV	"v V%[0-9]"
#  define DEFAULT_OS_MINOR_REV	"v V%*d.%[0-9]"
#  define DEFAULT_OS_TEENY_REV	"v V%*d.%*d.%[0-9]"
#  define DEFAULT_OS_NAME	"s %[^\n]"
# endif
#endif /* !defined CROSSCOMPILE || defined CROSSCOMPILE_CPP */

# if defined (CROSSCOMPILE_CPP) 
#  ifndef CROSS_UTS_SYSNAME
char *cross_uts_sysname = "";
#  else
char *cross_uts_sysname = CROSS_UTS_SYSNAME;
#  endif
#  ifndef CROSS_UTS_RELEASE
char* cross_uts_release = "";
#  else
char* cross_uts_release = CROSS_UTS_RELEASE;
#  endif
#  ifndef CROSS_UTS_MACHINE
char *cross_uts_machine = "";
#  else
char *cross_uts_machine = CROSS_UTS_MACHINE;
#  endif
#  ifndef CROSS_UTS_VERSION
char * cross_uts_version = "";
#  else
char * cross_uts_version = CROSS_UTS_VERSION;
#  endif
#  ifdef DEFAULT_OS_NAME
char *defaultOsName = DEFAULT_OS_NAME;
#  else 
char *defaultOsName = NULL;
# endif
#  ifdef DEFAULT_OS_MAJOR_REV
char *defaultOsMajorRev = DEFAULT_OS_MAJOR_REV;
#  else 
char *defaultOsMajorRev = NULL;
# endif
#  ifdef DEFAULT_OS_MINOR_REV
char *defaultOsMinorRev = DEFAULT_OS_MINOR_REV;
#  else 
char *defaultOsMinorRev = NULL;
#  endif
#  ifdef DEFAULT_OS_TEENY_REV
char *defaultOsTeenyRev = DEFAULT_OS_TEENY_REV;
#  else 
char *defaultOsTeenyRev = NULL;
#  endif
#  ifdef DEFAULT_MACHINE_ARCHITECTURE
char *defaultMachineArchitecture = DEFAULT_MACHINE_ARCHITECTURE;
#  else 
char *defaultMachineArchitecture = NULL;
#  endif
# ifdef DEFAULT_OS_NAME_FROB
void defaultOsNameFrob(char *buf, int size)
{DEFAULT_OS_NAME_FROB(buf,size)}
# else
void (*defaultOsNameFrob)(char *buf, int size) = NULL;
# endif
# ifdef DEFAULT_OS_MAJOR_REV_FROB
void defaultOsMajorRevFrob(char *buf, int size)
{DEFAULT_OS_MAJOR_REV_FROB(buf,size)}
# else
void (*defaultOsMajorRevFrob)(char *buf, int size) = NULL;
# endif
# ifdef DEFAULT_OS_MINOR_REV_FROB
void defaultOsMinorRevFrob(char *buf, int size)
{DEFAULT_OS_MINOR_REV_FROB(buf,size)}
# else
void (*defaultOsMinorRevFrob)(char *buf, int size) = NULL;
# endif
# ifdef DEFAULT_OS_TEENY_REV_FROB
void defaultOsTeenyRevFrob(char *buf, int size)
{DEFAULT_OS_TEENY_REV_FROB(buf,size)}
# else
void (*defaultOsTeenyRevFrob)(char *buf, int size) = NULL;
# endif
# endif /* CROSSCOMPILE_CPP */

#else /* else MAKEDEPEND */
#if !defined (CROSSCOMPILE) || defined (CROSSCOMPILE_CPP) 
/*
 * Step 7:  predefs
 *     If your compiler and/or preprocessor define any specific symbols, add
 *     them to the the following table.  The definition of struct symtab is
 *     in util/makedepend/def.h.
 */
#undef DEF_EVALUATE
#undef DEF_STRINGIFY
#define DEF_EVALUATE(__x) #__x
#define DEF_STRINGIFY(_x) DEF_EVALUATE(_x)
struct symtab	predefs[] = {
#ifdef apollo
	{"apollo", "1"},
#endif
#if defined(clipper) || defined(__clipper__)
	{"clipper", "1"},
	{"__clipper__", "1"},
	{"clix", "1"},
	{"__clix__", "1"},
#endif
#ifdef ibm032
	{"ibm032", "1"},
#endif
#ifdef ibm
	{"ibm", "1"},
#endif
#ifdef aix
	{"aix", "1"},
#endif
#ifdef sun
	{"sun", "1"},
#endif
#ifdef sun2
	{"sun2", "1"},
#endif
#ifdef sun3
	{"sun3", "1"},
#endif
#ifdef sun4
	{"sun4", "1"},
#endif
#ifdef sparc
	{"sparc", "1"},
#endif
#ifdef __sparc
	{"__sparc", "1"},
#endif
#ifdef __sparcv9
	{"__sparcv9", "1"},
#endif
#ifdef __sparc__
	{"__sparc__", "1"},
#endif
#ifdef __sparcv9__
	{"__sparcv9__", "1"},
#endif
#ifdef hpux
	{"hpux", "1"},
#endif
#ifdef __hpux
	{"__hpux", "1"},
#endif
#ifdef __hp9000s800
	{"__hp9000s800", "1"},
#endif
#ifdef __hp9000s700
	{"__hp9000s700", "1"},
#endif
#ifdef vax
	{"vax", "1"},
#endif
#ifdef VMS
	{"VMS", "1"},
#endif
#ifdef cray
	{"cray", "1"},
#endif
#ifdef CRAY
	{"CRAY", "1"},
#endif
#ifdef _CRAY
	{"_CRAY", "1"},
#endif
#ifdef att
	{"att", "1"},
#endif
#ifdef mips
	{"mips", "1"},
#endif
#ifdef __mips__
	{"__mips__", "1"},
#endif
#ifdef ultrix
	{"ultrix", "1"},
#endif
#ifdef stellar
	{"stellar", "1"},
#endif
#ifdef mc68000
	{"mc68000", "1"},
#endif
#ifdef mc68020
	{"mc68020", "1"},
#endif
#ifdef __GNUC__
	{"__GNUC__", DEF_STRINGIFY(__GNUC__)},
#endif
#ifdef __STRICT_ANSI__
	{"__STRICT_ANSI__", "1"},
#endif
#ifdef __STDC__
	{"__STDC__", DEF_STRINGIFY(__STDC__)},
#endif
#ifdef __HIGHC__
	{"__HIGHC__", "1"},
#endif
#ifdef __OPENSERVER__
	{"__OPENSERVER__", DEF_STRINGIFY(__OPENSERVER__)},
#endif
#ifdef _SCO_DS
	{"_SCO_DS", DEF_STRINGIFY(_SCO_DS)},
#endif
#ifdef _SCO_DS_LL
	{"_SCO_DS_LL", DEF_STRINGIFY(_SCO_DS_LL)},
#endif
#ifdef __SCO_VERSION__
	{"__SCO_VERSION__", DEF_STRINGIFY(__SCO_VERSION__)},
#endif
#ifdef __UNIXWARE__
	{"__UNIXWARE__", DEF_STRINGIFY(__UNIXWARE__)},
#endif
#ifdef __USLC__
	{"__USLC__", DEF_STRINGIFY(__USLC__)},
#endif
#ifdef CMU
	{"CMU", "1"},
#endif
#ifdef luna
	{"luna", "1"},
#ifdef luna1
	{"luna1", "1"},
#endif
#ifdef luna2
	{"luna2", "1"},
#endif
#ifdef luna88k
	{"luna88k", "1"},
#endif
#ifdef uniosb
	{"uniosb", "1"},
#endif
#ifdef uniosu
	{"uniosu", "1"},
#endif
#endif
#ifdef ieeep754
	{"ieeep754", "1"},
#endif
#ifdef is68k
	{"is68k", "1"},
#endif
#ifdef m68k
        {"m68k", "1"},
#endif
#ifdef M68k
        {"M68k", "1"},
#endif
#ifdef __m68k__
	{"__m68k__", "1"},
#endif
#ifdef m88k
        {"m88k", "1"},
#endif
#ifdef __m88k__
	{"__m88k__", "1"},
#endif
#ifdef bsd43
	{"bsd43", "1"},
#endif
#ifdef hcx
	{"hcx", "1"},
#endif
#ifdef sony
	{"sony", "1"},
#ifdef SYSTYPE_SYSV
	{"SYSTYPE_SYSV", "1"},
#endif
#ifdef _SYSTYPE_SYSV
	{"_SYSTYPE_SYSV", "1"},
#endif
#endif
#ifdef __OSF__
	{"__OSF__", "1"},
#endif
#ifdef __osf__
	{"__osf__", "1"},
#endif
#ifdef __amiga__
	{"__amiga__", "1"},
#endif
#ifdef __alpha
	{"__alpha", "1"},
#endif
#ifdef __alpha__
	{"__alpha__", "1"},
#endif
#ifdef __DECC
	{"__DECC",  "1"},
#endif
#ifdef __decc
	{"__decc",  "1"},
#endif
#ifdef __unix__
	{"__unix__", "1"},
#endif
#ifdef __uxp__
	{"__uxp__", "1"},
#endif
#ifdef __sxg__
	{"__sxg__", "1"},
#endif
#ifdef _SEQUENT_
	{"_SEQUENT_", "1"},
	{"__STDC__", "1"},
#endif
#ifdef __bsdi__
	{"__bsdi__", "1"},
#endif
#ifdef nec_ews_svr2
	{"nec_ews_svr2", "1"},
#endif
#ifdef nec_ews_svr4
	{"nec_ews_svr4", "1"},
#endif
#ifdef _nec_ews_svr4
	{"_nec_ews_svr4", "1"},
#endif
#ifdef _nec_up
	{"_nec_up", "1"},
#endif
#ifdef SX
	{"SX", "1"},
#endif
#ifdef nec
	{"nec", "1"},
#endif
#ifdef _nec_ft
	{"_nec_ft", "1"},
#endif
#ifdef PC_UX
	{"PC_UX", "1"},
#endif
#ifdef sgi
	{"sgi", "1"},
#endif
#ifdef __sgi
	{"__sgi", "1"},
#endif
#ifdef _MIPS_FPSET
	{"_MIPS_FPSET", DEF_STRINGIFY(_MIPS_FPSET)},
#endif
#ifdef _MIPS_ISA
	{"_MIPS_ISA", DEF_STRINGIFY(_MIPS_ISA)},
#endif
#ifdef _MIPS_SIM
	{"_MIPS_SIM", DEF_STRINGIFY(_MIPS_SIM)},
#endif
#ifdef _MIPS_SZINT
	{"_MIPS_SZINT", DEF_STRINGIFY(_MIPS_SZINT)},
#endif
#ifdef _MIPS_SZLONG
	{"_MIPS_SZLONG", DEF_STRINGIFY(_MIPS_SZLONG)},
#endif
#ifdef _MIPS_SZPTR
	{"_MIPS_SZPTR", DEF_STRINGIFY(_MIPS_SZPTR)},
#endif
#ifdef __DragonFly__
	{"__DragonFly__", "1"},
#endif
#ifdef __FreeBSD__
	{"__FreeBSD__", "1"},
#endif
#ifdef __OpenBSD__
	{"__OpenBSD__", "1"},
#endif
#ifdef __NetBSD__
	{"__NetBSD__", "1"},
#endif
#ifdef __GNU__
	{"__GNU__", "1"},
#endif
#ifdef __ELF__
	{"__ELF__", "1"},
#endif
#ifdef __UNIXOS2__
	{"__UNIXOS2__", "1"},
#endif
#if defined(__QNX__)
        {"__QNX__", "1"},
#endif
#ifdef __QNXNTO__
        {"__QNXNTO__", "1"},
#endif
# ifdef __powerpc__
	{"__powerpc__", "1"},
# endif
# ifdef __powerpc64__
	{"__powerpc64__", "1"},
# endif
# ifdef PowerMAX_OS
	{"PowerMAX_OS", "1"},
# endif
# ifdef ia64
	{"ia64", "1"},
# endif
# ifdef __ia64__
	{"__ia64__", "1"},
# endif
# if defined (amd64) || defined (x86_64)
	{"amd64", "1"},
	{"x86_64", "1"},
# endif
# if defined (__amd64__) || defined (__x86_64__)
	{"__amd64__", "1"},
	{"__x86_64__", "1"},
# endif
# if defined (__amd64) || defined(__x86_64)
	{"__amd64", "1"},
	{"__x86_64", "1"},
# endif
# ifdef __x86
	{"__x86", "1"},
# endif
# ifdef __i386
	{"__i386", "1"},
# endif
# ifdef __i386__
	{"__i386__", "1"},
# endif
# ifdef __i486__
	{"__i486__", "1"},
# endif
# ifdef __i586__
	{"__i586__", "1"},
# endif
# ifdef __i686__
	{"__i686__", "1"},
# endif
# ifdef __k6__
	{"__k6__", "1"},
# endif
# ifdef i386
	{"i386", "1"},
# endif
# ifdef i486
	{"i486", "1"},
# endif
# ifdef i586
	{"i586", "1"},
# endif
# ifdef i686
	{ "i686", "1"},
# endif
# ifdef k6
	{"k6", "1"},
# endif
# ifdef sparc
	{"sparc", "1"},
# endif
# ifdef __sparc__
	{"__sparc__", "1"},
# endif
# ifdef __s390__
	{"__s390__", "1"},
# endif
# ifdef __hppa__
	{"__hppa__", "1"},
# endif
# ifdef __sh__
	{"__sh__", "1"},
# endif
# ifdef __sh3_
	{"__sh3__", "1"},
# endif
# ifdef __SH3__
	{"__SH3__", "1"},
# endif
# ifdef __SH4__
	{"__SH4__", "1"},
# endif
# ifdef __SH4NOFPU__
	{"__SH4NOFPU__", "1"},
# endif
#if defined(__ppc__)
        {"__ppc__", "1"},
#endif
#if defined(__ppc64__)
        {"__ppc64__", "1"},
#endif
#if defined(__BIG_ENDIAN__)
      {"__BIG_ENDIAN__", "1"},
#endif
#if defined(__LITTLE_ENDIAN__)
      {"__LITTLE_ENDIAN__", "1"},
#endif
#if defined (__CHAR_BIT__)
	{"__CHAR_BIT__", DEF_STRINGIFY(__CHAR_BIT__)},
#endif
#if defined (__BUILTIN_VA_STRUCT)
	{"__BUILTIN_VA_STRUCT", "1"},
#endif
#if defined (__BUILTIN_VA_ARG_INCR)
	{"__BUILTIN_VA_ARG_INCR", "1"},
#endif	
	/* add any additional symbols before this line */
	{NULL, NULL}
};
#undef DEF_EVALUATE
#undef DEF_STRINGIFY
#endif /* CROSSCOMPILE */
#endif /* MAKEDEPEND */

# ifndef MAKEDEPEND
#  if  defined (CROSSCOMPILE_CPP) 
#   ifdef USE_CC_E
boolean crosscompile_use_cc_e = TRUE;
#    ifdef DEFAULT_CC
char* crosscompile_cpp = DEFAULT_CC;
#    else
char* crosscompile_cpp = "cc";
#    endif
#   else
boolean crosscompile_use_cc_e = FALSE;
#    ifdef DEFAULT_CPP
char* crosscompile_cpp = DEFAULT_CPP;
#    else
char* crosscompile_cpp = "cpp";
#    endif
#   endif
#   ifdef FIXUP_CPP_WHITESPACE
boolean fixup_whitespace = TRUE;
#   else
boolean fixup_whitespace = FALSE;
#   endif
#   ifdef REMOVE_CPP_LEADSPACE
boolean remove_cpp_leadspace = TRUE;
#   else
boolean remove_cpp_leadspace = FALSE;
#   endif
#   ifdef INLINE_SYNTAX
boolean inline_syntax = TRUE;
#   else
boolean inline_syntax = FALSE;
#   endif
#   ifdef MAGIC_MAKE_VARS
boolean magic_make_vars = TRUE;
#   else
boolean magic_make_vars = FALSE;
#   endif

typedef enum {
  unknown,
  freeBSD,
  netBSD,
  LinuX,
  emx,
  win32,
  dragonfly
} System;

#   if defined(linux) || defined(__GLIBC__)
System sys = LinuX;
#   elif defined __FreeBSD__
System sys = freebsd;
#   elif defined __NetBSD__
System sys = netBSD;
#   elif defined __EMX__
System sys = emx;
#   elif defined WIN32
System sys = win32;
#   elif defined __DragonFly__
System sys = dragonfly;
#   else
System sys = unknown;
#   endif

#   if defined __GNUC__
int gnu_c = __GNUC__;
int gnu_c_minor = __GNUC_MINOR__;
#   else 
int gnu_c = 0;
int gnu_c_minor = -1;
#   endif
#   if defined(linux) || defined(__GLIBC__)
#    include <features.h>
int glibc_major = __GLIBC__ + 4;
int glibc_minor = __GLIBC_MINOR__;
#   else
int glibc_major = 0;
int glibc_minor = -1;
#   endif
#  endif /* !CROSSCOMPILE || CROSSCOMPILE_CPP */

# endif /* MAKEDEPEND */

#endif /* CCIMAKE */
