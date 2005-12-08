include Makerules

CFLAGS = $(OPTFLAG) -Iomnithread -Izlib -I.. -I../../../vgl/include \
	-DWIN32 -D_WINDOWS -D__WIN32__ -D__NT__ -D__x86__ -D_WINSTATIC -DNCORBA
LDFLAGS = $(OPTFLAG)
WINLIBS = -lwsock32 -lgdi32 -lcomdlg32 -lwinmm -lhtmlhelp -lcomctl32 \
	-L../../../vgl/windows/vnc/lib -lturbojpeg

OBJS = $(ODIR)/AdministrationControls.o \
       $(ODIR)/d3des.o \
       $(ODIR)/FileTransferItemInfo.o \
       $(ODIR)/IncomingConnectionsControls.o \
       $(ODIR)/InputHandlingControls.o \
       $(ODIR)/Log.o \
       $(ODIR)/MatchWindow.o \
       $(ODIR)/MinMax.o \
       $(ODIR)/PollControls.o \
       $(ODIR)/QuerySettingsControls.o \
       $(ODIR)/RectList.o \
       $(ODIR)/SharedDesktopArea.o \
       $(ODIR)/stdhdrs.o \
       $(ODIR)/translate.o \
       $(ODIR)/VideoDriver.o \
       $(ODIR)/vncAbout.o \
       $(ODIR)/vncAcceptDialog.o \
       $(ODIR)/vncAcceptReverseDlg.o \
       $(ODIR)/vncauth.o \
       $(ODIR)/vncBuffer.o \
       $(ODIR)/vncClient.o \
       $(ODIR)/vncConnDialog.o \
       $(ODIR)/vncDesktop.o \
       $(ODIR)/vncEncoder.o \
       $(ODIR)/vncEncodeTight.o \
       $(ODIR)/VNCHelp.o \
       $(ODIR)/vncHTTPConnect.o \
       $(ODIR)/vncInstHandler.o \
       $(ODIR)/vncKeymap.o \
       $(ODIR)/vncMenu.o \
       $(ODIR)/vncProperties.o \
       $(ODIR)/vncRegion.o \
       $(ODIR)/vncServer.o \
       $(ODIR)/vncService.o \
       $(ODIR)/vncSockConnect.o \
       $(ODIR)/vncTimedMsgBox.o \
       $(ODIR)/VSocket.o \
       $(ODIR)/WinVNC.o \
       $(ODIR)/omnithread.o

LIBS = $(ODIR)/VNCHooks.dll.a $(ODIR)/VNCHooks.dll $(ODIR)/libz.a

RES = $(ODIR)/WinVNC.res.o

TARGETS = $(ODIR)/WinVNC.exe

default: $(TARGETS)

-include $(OBJS:.o=.d)

$(ODIR)/WinVNC.exe: $(LIBS) $(OBJS) $(RES)
	$(CXX) $(LDFLAGS) $(OBJS) $(RES) -o $@ $(LIBS) $(WINLIBS) -mwindows

$(ODIR)/libz.a:
	make -C zlib

$(ODIR)/VNCHooks.dll.a $(ODIR)/VNCHooks.dll:
	make -C VNCHooks

$(ODIR)/omnithread.o: omnithread/nt.cpp omnithread/nt.h omnithread/omnithread.h
	$(CXX) -c $(CFLAGS) -o $@ $<

afxres.h:
	echo "#include <windows.h>" > $@
	echo "#ifndef IDC_STATIC" >> $@
	echo "#define IDC_STATIC (-1)" >> $@
	echo "#endif" >> $@

$(RES): WinVNC.rc afxres.h
	$(RC) -DWITH_JAVA_VIEWER -l 0x809 -i $< -o $@

clean:
	make -C zlib clean
	make -C VNCHooks clean
	rm -f $(OBJS) $(OBJS:.o=.d) $(RES) $(TARGETS)
