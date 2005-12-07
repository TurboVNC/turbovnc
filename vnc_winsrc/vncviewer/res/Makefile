TOPDIR = ..
include ../Makerules

TARGETS = $(ODIR)/vncviewer.res.o

default: $(TARGETS)

afxres.h:
	echo "#include <windows.h>" > $@
	echo "#ifndef IDC_STATIC" >> $@
	echo "#define IDC_STATIC (-1)" >> $@
	echo "#endif" >> $@

$(ODIR)/vncviewer.res.o: vncviewer.rc afxres.h
	$(RC) -DWINVER=0x0400 -l 0x409 -i $< -o $@

clean:
	rm -f $(TARGETS)
