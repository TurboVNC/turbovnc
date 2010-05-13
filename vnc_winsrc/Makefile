all: vncviewer winvnc putty

.PHONY: vncviewer winvnc putty

vncviewer winvnc putty:
	$(MAKE) -C $@

.PHONY: clean
clean:
	$(MAKE) -C vncviewer clean
	$(MAKE) -C winvnc clean
	$(MAKE) -C putty clean

SYSROOT := $(subst \,\\,$(SYSTEMROOT))

.PHONY: dist
dist: vncviewer winvnc putty
	iscc TurboVNC.iss 
	mv output/setup.exe TurboVNC.exe
