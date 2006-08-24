TOPDIR = ..
include ../Makerules

WINLIBS = user32.lib gdi32.lib advapi32.lib

OBJS = $(ODIR)/VNCHooks.obj

TARGETS = $(ODIR)/VNCHooks.lib $(ODIR)/VNCHooks.dll $(ODIR)/VNCHooks.exp

RES = $(ODIR)/VNCHooks.res

default: $(TARGETS)

HDRS := $(wildcard *.h)
$(OBJS): $(HDRS)

$(TARGETS): $(OBJS) $(RES)
	$(LINK) -dll $^ -out:$(ODIR)/VNCHooks.dll \
		-implib:$(ODIR)/VNCHooks.lib $(WINLIBS)

$(RES): VNCHooks.rc
	$(RC) -l 0x809 -fo $@ $<

clean:
	rm -f $(OBJS) $(RES) $(TARGETS)
