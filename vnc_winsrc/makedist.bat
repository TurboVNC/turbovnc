@echo off

if "%1"=="clean" (
	cd vncviewer
	nmake /f vncviewer.mak cfg="vncviewer - Win32 Release" recurse=1 clean
	if errorlevel 1 goto abort
	cd ..
	cd winvnc
	nmake /f winvnc.mak cfg="WinVNC - Win32 Release" recurse=1 clean
	if errorlevel 1 goto abort
	cd ..
	if not exist ..\..\vgl\Makefile (
		echo Could not find VirtualGL build directory at ..\..\vgl !
		goto abort
	) else (
		set _PWD=%CD%
		cd ..\..\vgl
		make DISTRO=vnc clean
		if errorlevel 1 goto abort
		cd %_PWD%
	)
	goto abort
)

if exist TurboVNC.exe del /q TurboVNC.exe 

set _PWD=%CD%
cd ..\..\vgl
make vnc
if errorlevel 1 goto abort
make vnc
if errorlevel 1 goto abort
cd %_PWD%

cd vncviewer
nmake /f vncviewer.mak cfg="vncviewer - Win32 Release"
if errorlevel 1 goto abort
cd ..
cd winvnc
nmake /f winvnc.mak cfg="WinVNC - Win32 Release"
if errorlevel 1 goto abort
cd ..

compil32 /cc TurboVNC.iss
if errorlevel 1 goto abort

move output\setup.exe TurboVNC.exe
rd output

:abort
