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
	cp $(SYSROOT)/system32/turbojpeg-ipp.dll turbojpeg.dll
	cp $(SYSROOT)/system32/turbojpeg-ipp.dll .
	cp $(SYSROOT)/system32/turbojpeg-libjpeg.dll .
	cp $(SYSROOT)/system32/switchtjpeg.bat .
	iscc TurboVNC.iss 
	$(RM) turbojpeg.dll turbojpeg-ipp.dll turbojpeg-libjpeg.dll switchtjpeg.bat
	mv output/setup.exe TurboVNC.exe
