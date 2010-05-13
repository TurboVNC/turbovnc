ONAME := obj/vncviewer

include ../Makerules

CFLAGS := $(CFLAGS) -D_WINSTATIC -D_WIN32_WINNT=0x0400 -Iomnithread -Izlib -I..

WINLIBS = wsock32.lib gdi32.lib comdlg32.lib winmm.lib htmlhelp.lib \
	comctl32.lib user32.lib advapi32.lib shell32.lib \
	turbojpeg-static.lib

OBJS = $(ODIR)/AboutBox.obj \
       $(ODIR)/BuildTime.obj \
       $(ODIR)/CapsContainer.obj \
       $(ODIR)/ClientConnection.obj \
       $(ODIR)/ClientConnectionClipboard.obj \
       $(ODIR)/ClientConnectionCopyRect.obj \
       $(ODIR)/ClientConnectionCursor.obj \
       $(ODIR)/ClientConnectionFile.obj \
       $(ODIR)/ClientConnectionFullScreen.obj \
       $(ODIR)/ClientConnectionHextile.obj \
       $(ODIR)/ClientConnectionRaw.obj \
       $(ODIR)/ClientConnectionTight.obj \
       $(ODIR)/ConnectingDialog.obj \
       $(ODIR)/d3des.obj \
       $(ODIR)/Daemon.obj \
       $(ODIR)/Exception.obj \
       $(ODIR)/fbx.obj \
       $(ODIR)/FileTransfer.obj \
       $(ODIR)/FileTransferItemInfo.obj \
       $(ODIR)/HotKeys.obj \
       $(ODIR)/KeyMap.obj \
       $(ODIR)/Log.obj \
       $(ODIR)/LoginAuthDialog.obj \
       $(ODIR)/SessionDialog.obj \
       $(ODIR)/stdhdrs.obj \
       $(ODIR)/vncauth.obj \
       $(ODIR)/VNCOptions.obj \
       $(ODIR)/vncviewer.obj \
       $(ODIR)/VNCHelp.obj \
       $(ODIR)/VNCviewerApp.obj \
       $(ODIR)/VNCviewerApp32.obj \
       $(ODIR)/omnithread.obj

LIBS = $(ODIR)/zlib.lib

RES = $(ODIR)/vncviewer.res

TARGETS = $(EDIR)/vncviewer.exe $(ODIR)/zlib.lib

default: zlib res $(TARGETS)

HDRS := $(wildcard *.h omnithread/*.h zlib/*.h)
$(OBJS): $(HDRS)

$(EDIR)/vncviewer.exe: $(LIBS) $(OBJS) $(RES)
	$(LINK) $(OBJS) $(RES) -out:$@ $(LIBS) $(WINLIBS)

.PHONY: zlib res
zlib res:
	$(MAKE) -C $@

$(ODIR)/zlib.lib:
	$(MAKE) -C zlib

$(ODIR)/omnithread.obj: omnithread/nt.cpp omnithread/nt.h omnithread/omnithread.h
	$(CXX) -c $(CFLAGS) -Fo$@ $<

$(RES):
	$(MAKE) -C res

clean:
	$(MAKE) -C zlib clean
	$(MAKE) -C res clean
	rm -f $(OBJS) $(TARGETS)
