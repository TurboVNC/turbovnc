#
# Making the VNC applet.
#

CP = cp
JC = javac
JCFLAGS = -target 1.1 -source 1.2
JAR = jar
ARCHIVE = VncViewer.jar
MANIFEST = MANIFEST.MF
PAGES = index.vnc
INSTALL_DIR = /usr/local/vnc/classes

CLASSES = VncViewer.class RfbProto.class AuthPanel.class VncCanvas.class \
	  VncCanvas2.class \
	  OptionsFrame.class ClipboardFrame.class ButtonPanel.class \
	  DesCipher.class CapabilityInfo.class CapsContainer.class \
	  RecordingFrame.class RFBUpdateEvent.class SessionRecorder.class \
	  SocketFactory.class HTTPConnectSocketFactory.class \
	  HTTPConnectSocket.class ReloginPanel.class \
	  InStream.class MemInStream.class ZlibInStream.class

SOURCES = VncViewer.java RfbProto.java AuthPanel.java VncCanvas.java \
	  VncCanvas2.java \
	  OptionsFrame.java ClipboardFrame.java ButtonPanel.java \
	  DesCipher.java CapabilityInfo.java CapsContainer.java \
	  RecordingFrame.java RFBUpdateEvent.java SessionRecorder.java \
	  SocketFactory.java HTTPConnectSocketFactory.java \
	  HTTPConnectSocket.java ReloginPanel.java \
	  InStream.java MemInStream.java ZlibInStream.java

all: $(CLASSES) $(ARCHIVE)

$(CLASSES): $(SOURCES)
	$(JC) $(JCFLAGS) -O $(SOURCES)

$(ARCHIVE): $(CLASSES) $(MANIFEST)
	$(JAR) cfm $(ARCHIVE) $(MANIFEST) $(CLASSES)
	rm -f turbovnc.keystore
	keytool -genkey -alias TurboVNC -keystore turbovnc.keystore -keyalg RSA \
		-storepass turbovnc -keypass turbovnc -validity 7300 \
		-dname "CN=TurboVNC, OU=Software Development, O=The VirtualGL Project, L=Austin, S=Texas, C=US"
	jarsigner -keystore turbovnc.keystore -storepass turbovnc -keypass turbovnc \
		$(ARCHIVE) TurboVNC
	rm -f turbovnc.keystore

install: $(CLASSES) $(ARCHIVE)
	$(CP) $(CLASSES) $(ARCHIVE) $(PAGES) $(INSTALL_DIR)

release: $(ARCHIVE) README
	$(CP) $(ARCHIVE) ../vnc_unixsrc/classes/
	$(CP) README ../vnc_unixsrc/classes/
	$(CP) $(ARCHIVE) ../vnc_winsrc/winvnc/res/

export:: $(CLASSES) $(ARCHIVE) $(PAGES)
	@$(ExportJavaClasses)

clean::
	$(RM) *.class *.jar
