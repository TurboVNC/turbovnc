ONAME := obj/vncviewer
TOPDIR = ../..

include ../../Makerules

TARGETS = $(ODIR)/vncviewer.res

default: $(TARGETS)

$(ODIR)/vncviewer.res: vncviewer.rc
	$(RC) -l 0x409 -fo $@ $<

clean:
	rm -f $(TARGETS)
