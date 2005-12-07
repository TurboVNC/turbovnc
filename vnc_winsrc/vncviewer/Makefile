include Makerules

CFLAGS = $(OPTFLAG) -I. -I.. -I../../../vgl/include -Iomnithread -DXMD_H \
	-DWIN32 -D_WINDOWS -D__NT__ -D_WINSTATIC -D__WIN32__
LDFLAGS = $(OPTFLAG)
WINLIBS = -lwsock32 -lgdi32 -lcomdlg32 -lwinmm -lhtmlhelp -lcomctl32 \
	-L../../../vgl/windows/vnc/lib -lturbojpeg -lfbx

OBJS = $(ODIR)/AboutBox.o \
       $(ODIR)/AuthDialog.o \
       $(ODIR)/CapsContainer.o \
       $(ODIR)/ClientConnection.o \
       $(ODIR)/ClientConnectionClipboard.o \
       $(ODIR)/ClientConnectionCopyRect.o \
       $(ODIR)/ClientConnectionCursor.o \
       $(ODIR)/ClientConnectionFile.o \
       $(ODIR)/ClientConnectionFullScreen.o \
       $(ODIR)/ClientConnectionTight.o \
       $(ODIR)/ConnectingDialog.o \
       $(ODIR)/d3des.o \
       $(ODIR)/Daemon.o \
       $(ODIR)/Exception.o \
       $(ODIR)/FileTransfer.o \
       $(ODIR)/FileTransferItemInfo.o \
       $(ODIR)/Flasher.o \
       $(ODIR)/HotKeys.o \
       $(ODIR)/KeyMap.o \
       $(ODIR)/Log.o \
       $(ODIR)/LoginAuthDialog.o \
       $(ODIR)/SessionDialog.o \
       $(ODIR)/stdhdrs.o \
       $(ODIR)/vncauth.o \
       $(ODIR)/VNCOptions.o \
       $(ODIR)/vncviewer.o \
       $(ODIR)/VNCHelp.o \
       $(ODIR)/VNCviewerApp.o \
       $(ODIR)/VNCviewerApp32.o \
       $(ODIR)/omnithread.o

LIBS = $(ODIR)/libz.a

RES = $(ODIR)/vncviewer.res.o

TARGETS = $(ODIR)/vncviewer.exe

default: $(TARGETS)

-include $(OBJS:.o=.d)

$(ODIR)/vncviewer.exe: $(LIBS) $(OBJS) $(RES)
	$(CXX) $(LDFLAGS) $(OBJS) $(RES) -o $@ $(LIBS) $(WINLIBS) -mwindows

$(ODIR)/libz.a:
	make -C zlib

$(ODIR)/omnithread.o: omnithread/nt.cpp omnithread/nt.h omnithread/omnithread.h
	$(CXX) -c $(CFLAGS) -o $@ $<

$(RES):
	make -C res

clean:
	make -C zlib clean
	make -C res clean
	rm -f $(OBJS) $(OBJS:.o=.d) $(TARGETS)
