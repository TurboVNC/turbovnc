all: vncviewer winvnc

.PHONY: vncviewer winvnc

vncviewer winvnc:
	$(MAKE) -C $@

.PHONY: clean
clean:
	$(MAKE) -C vncviewer clean
	$(MAKE) -C winvnc clean

.PHONY: dist
dist: vncviewer winvnc
	$(RM) TurboVNC.exe
	compil32 /cc TurboVNC.iss
	mv output/setup.exe TurboVNC.exe
