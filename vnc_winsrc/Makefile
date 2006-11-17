all: vncviewer winvnc

.PHONY: vncviewer winvnc

vncviewer winvnc:
	$(MAKE) -C $@

.PHONY: clean
clean:
	$(MAKE) -C vncviewer clean
	$(MAKE) -C winvnc clean

SYSROOT := $(subst \,\\,$(SYSTEMROOT))

.PHONY: dist
dist: vncviewer winvnc
	cp $(SYSROOT)/system32/turbojpeg.dll .
	iscc TurboVNC.iss 
	$(RM) turbojpeg.dll
	mv output/setup.exe TurboVNC.exe
