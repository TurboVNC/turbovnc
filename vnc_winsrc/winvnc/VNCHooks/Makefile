TOPDIR = ..
include ../Makerules

CFLAGS = $(OPTFLAG) -DWIN32 -D_WINDOWS -D_WIN32_WINNT=0x0400
WINLIBS = -lgdi32

OBJS = $(ODIR)/VNCHooks.o

TARGETS = $(ODIR)/VNCHooks.dll.a $(ODIR)/VNCHooks.dll

RES = $(ODIR)/VNCHooks.res.o

default: $(TARGETS)

-include $(OBJS:.o=.d)

$(ODIR)/VNCHooks.dll $(ODIR)/VNCHooks.dll.a: $(OBJS) $(RES)
	$(CXX) $(LDFLAGS) -shared $^ -o $(ODIR)/VNCHooks.dll \
		-Wl,-out-implib,$(ODIR)/VNCHooks.dll.a $(WINLIBS)

afxres.h:
	echo "#include <windows.h>" > $@
	echo "#ifndef IDC_STATIC" >> $@
	echo "#define IDC_STATIC (-1)" >> $@
	echo "#endif" >> $@

$(RES): VNCHooks.rc afxres.h
	$(RC) -l 0x809 -i $< -o $@

clean:
	rm -f $(OBJS) $(OBJS:.o=.d) $(RES) $(TARGETS)
