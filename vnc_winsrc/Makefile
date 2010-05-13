TOPDIR = .
include Makerules

all: vncviewer winvnc putty

.PHONY: vncviewer winvnc putty

vncviewer winvnc putty:
	$(MAKE) -C $@

.PHONY: clean
clean:
	$(MAKE) -C vncviewer clean
	$(MAKE) -C winvnc clean
	$(MAKE) -C putty clean

.PHONY: dist
ifeq ($(subplatform), 64)
dist: vncviewer winvnc putty
	iscc /O$(BLDDIR) /FTurboVNC /dVERSION=$(VERSION) /dBUILD=$(BUILD) \
		/dBLDDIR=$(BLDDIR) /dWIN64 TurboVNC.iss 
else
dist: vncviewer winvnc putty
	iscc /O$(BLDDIR) /FTurboVNC /dVERSION=$(VERSION) /dBUILD=$(BUILD) \
		/dBLDDIR=$(BLDDIR) TurboVNC.iss 
endif
