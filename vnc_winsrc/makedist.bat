@echo off

if "%1"=="clean" (
	msdev vncviewer\vncviewer.dsw /make "vncviewer - Win32 Release" /clean
	if errorlevel 1 goto abort
	msdev winvnc\winvnc.dsw /make "winvnc - Win32 No_CORBA" /clean
	if errorlevel 1 goto abort
	if not exist ..\..\vgl\Makefile (
		echo Could not find VirtualGL build directory at ..\..\vgl !
		goto abort
	) else (
		set _PWD=%CD%
		cd ..\..\vgl
		make DISTRO=vnc jpeg clean
		if errorlevel 1 goto abort
		cd %_PWD%
	)
)

if exist TurboVNC.exe del /q TurboVNC.exe 

set _PWD=%CD%
cd ..\..\vgl
make DISTRO=vnc jpeg
if errorlevel 1 goto abort
make DISTRO=vnc jpeg
if errorlevel 1 goto abort
cd %_PWD%

msdev vncviewer\vncviewer.dsw /make "vncviewer - Win32 Release"
if errorlevel 1 goto abort
msdev winvnc\winvnc.dsw /make "winvnc - Win32 No_CORBA"
if errorlevel 1 goto abort

compil32 /cc TurboVNC.iss
if errorlevel 1 goto abort

move output\setup.exe TurboVNC.exe
rd output

:abort
