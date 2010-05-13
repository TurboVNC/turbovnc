ONAME := obj/winvnc
TOPDIR = ../..

include ../../Makerules

WINLIBS = user32.lib gdi32.lib advapi32.lib

OBJS = $(ODIR)/VNCHooks.obj

TARGETS = $(ODIR)/VNCHooks.lib $(EDIR)/VNCHooks.dll $(ODIR)/VNCHooks.exp

RES = $(ODIR)/VNCHooks.res

default: $(TARGETS)

HDRS := $(wildcard *.h)
$(OBJS): $(HDRS)

$(TARGETS): $(OBJS) $(RES)
	$(LINK) -dll $^ -out:$(EDIR)/VNCHooks.dll \
		-implib:$(ODIR)/VNCHooks.lib $(WINLIBS)

ifeq ($(WIN64), yes)
WIN64DEF=-DWIN64
endif

$(RES): VNCHooks.rc
	$(RC) $(WIN64DEF) -D__RCVERSION=$(RCVERSION) \
		-D__RCVERSIONSTR=\"$(RCVERSION)\\0\" -l 0x809 -fo $@ $<

clean:
	rm -f $(OBJS) $(RES) $(TARGETS)
