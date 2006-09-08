all: vncviewer winvnc

.PHONY: vncviewer winvnc vnclibs

vncviewer winvnc: vnclibs
	$(MAKE) -C $@

vnclibs:
	$(MAKE) -C ../../vgl vnc

.PHONY: clean
clean:
	$(MAKE) -C vncviewer clean
	$(MAKE) -C winvnc clean
	$(MAKE) -C ../../vgl DISTRO=vnc clean

.PHONY: dist
dist: vncviewer winvnc
	$(RM) TurboVNC.exe
	compil32 /cc TurboVNC.iss
	mv output/setup.exe TurboVNC.exe
