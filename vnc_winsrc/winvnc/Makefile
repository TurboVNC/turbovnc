ONAME := obj/winvnc

include ../Makerules

CFLAGS := $(CFLAGS) -D_WINSTATIC -D_WIN32_WINNT=0x0500 -Iomnithread -Izlib -I..

WINLIBS = wsock32.lib gdi32.lib comdlg32.lib winmm.lib htmlhelp.lib \
	comctl32.lib user32.lib advapi32.lib shell32.lib ole32.lib \
	turbojpeg-static.lib

OBJS = $(ODIR)/AdministrationControls.obj \
       $(ODIR)/BuildTime.obj \
       $(ODIR)/d3des.obj \
       $(ODIR)/DynamicFn.obj \
       $(ODIR)/FileTransferItemInfo.obj \
       $(ODIR)/IncomingConnectionsControls.obj \
       $(ODIR)/InputHandlingControls.obj \
       $(ODIR)/Log.obj \
       $(ODIR)/MatchWindow.obj \
       $(ODIR)/MinMax.obj \
       $(ODIR)/ParseHost.obj \
       $(ODIR)/PollControls.obj \
       $(ODIR)/QuerySettingsControls.obj \
       $(ODIR)/RectList.obj \
       $(ODIR)/SharedDesktopArea.obj \
       $(ODIR)/stdhdrs.obj \
       $(ODIR)/translate.obj \
       $(ODIR)/TsSessions.obj \
       $(ODIR)/VideoDriver.obj \
       $(ODIR)/vncAbout.obj \
       $(ODIR)/vncAcceptDialog.obj \
       $(ODIR)/vncAcceptReverseDlg.obj \
       $(ODIR)/vncauth.obj \
       $(ODIR)/vncBuffer.obj \
       $(ODIR)/vncClient.obj \
       $(ODIR)/vncConnDialog.obj \
       $(ODIR)/vncDesktop.obj \
       $(ODIR)/vncEncoder.obj \
       $(ODIR)/vncEncodeCoRRE.obj \
       $(ODIR)/vncEncodeHexT.obj \
       $(ODIR)/vncEncodeRRE.obj \
       $(ODIR)/vncEncodeTight.obj \
       $(ODIR)/vncEncodeZlib.obj \
       $(ODIR)/vncEncodeZlibHex.obj \
       $(ODIR)/VNCHelp.obj \
       $(ODIR)/vncHTTPConnect.obj \
       $(ODIR)/vncInstHandler.obj \
       $(ODIR)/vncKeymap.obj \
       $(ODIR)/vncMenu.obj \
       $(ODIR)/vncProperties.obj \
       $(ODIR)/vncRegion.obj \
       $(ODIR)/vncServer.obj \
       $(ODIR)/vncService.obj \
       $(ODIR)/vncSockConnect.obj \
       $(ODIR)/vncTimedMsgBox.obj \
       $(ODIR)/VSocket.obj \
       $(ODIR)/WallpaperUtils.obj \
       $(ODIR)/WinVNC.obj \
       $(ODIR)/omnithread.obj

LIBS = $(ODIR)/VNCHooks.lib $(ODIR)/zlib.lib

RES = $(ODIR)/WinVNC.res

TARGETS = $(EDIR)/WinVNC.exe \
	$(ODIR)/VNCHooks.lib $(EDIR)/VNCHooks.dll $(ODIR)/zlib.lib

default: zlib VNCHooks $(TARGETS)

HDRS := $(wildcard *.h omnithread/*.h VNCHooks/*.h zlib/*.h)
$(OBJS): $(HDRS)

$(EDIR)/WinVNC.exe: $(LIBS) $(OBJS) $(RES)
	$(LINK) $(OBJS) $(RES) -out:$@ $(LIBS) $(WINLIBS)

.PHONY: zlib VNCHooks
zlib VNCHooks:
	$(MAKE) -C $@

$(ODIR)/zlib.lib:
	$(MAKE) -C zlib

$(ODIR)/VNCHooks.lib $(EDIR)/VNCHooks.dll:
	$(MAKE) -C VNCHooks

$(ODIR)/omnithread.obj: omnithread/nt.cpp omnithread/nt.h omnithread/omnithread.h
	$(CXX) -c $(CFLAGS) -Fo$@ $<

$(RES): WinVNC.rc
	$(RC) -DWITH_JAVA_VIEWER -l 0x409 -fo $@ $< 

clean:
	$(MAKE) -C zlib clean
	$(MAKE) -C VNCHooks clean
	rm -f $(OBJS) $(RES) $(TARGETS)
