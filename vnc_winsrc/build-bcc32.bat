@echo off
mkdir binaries
cd vncviewer
make -fMakefile.bcc32 -s
cd ..
copy /b vncviewer\vncviewer.exe binaries
cd winvnc
make -fMakefile.bcc32 -s
cd ..
copy /b winvnc\VNCHooks\VNCHooks.dll binaries
copy /b winvnc\winvnc.exe binaries
