@echo off
bcc32.exe -v- -O2 -3 -tWM -xd- -q -w-8066 -c -D_WINSTATIC -I. -oomnithread.obj omnithread\nt.cpp
