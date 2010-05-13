ONAME := obj/vncviewer
TOPDIR = ../..

include ../../Makerules

TARGETS = $(ODIR)/vncviewer.res

default: $(TARGETS)

ifeq ($(WIN64), yes)
APPNAME=\""TurboVNC Viewer 64-bit v$(VERSION) ($(BUILD))"\"
WIN64DEF=-DWIN64
else
APPNAME=\""TurboVNC Viewer v$(VERSION) ($(BUILD))"\"
endif

$(ODIR)/vncviewer.res: vncviewer.rc
	$(RC) -D__APPNAME=$(APPNAME) $(WIN64DEF) -D__RCVERSION=$(RCVERSION) \
		-D__RCVERSIONSTR=\"$(RCVERSION)\\0\" -l 0x409 -fo $@ $<

clean:
	rm -f $(TARGETS)
