include Makerules

COMMONOBJS = $(ODIR)/BE_ALL_S.obj \
             $(ODIR)/CMDLINE.obj  \
             $(ODIR)/CPROXY.obj   \
             $(ODIR)/LDISC.obj    \
             $(ODIR)/LOGGING.obj  \
             $(ODIR)/MISC.obj     \
             $(ODIR)/PINGER.obj   \
             $(ODIR)/PORTFWD.obj  \
             $(ODIR)/PROXY.obj    \
             $(ODIR)/RAW.obj      \
             $(ODIR)/RLOGIN.obj   \
             $(ODIR)/SETTINGS.obj \
             $(ODIR)/SSH.obj      \
             $(ODIR)/SSHAES.obj   \
             $(ODIR)/SSHARCF.obj  \
             $(ODIR)/SSHBLOWF.obj \
             $(ODIR)/SSHBN.obj    \
             $(ODIR)/SSHCRC.obj   \
             $(ODIR)/SSHCRCDA.obj \
             $(ODIR)/SSHDES.obj   \
             $(ODIR)/SSHDH.obj    \
             $(ODIR)/SSHDSS.obj   \
             $(ODIR)/SSHMD5.obj   \
             $(ODIR)/SSHPUBK.obj  \
             $(ODIR)/SSHRAND.obj  \
             $(ODIR)/SSHRSA.obj   \
             $(ODIR)/SSHSH256.obj \
             $(ODIR)/SSHSH512.obj \
             $(ODIR)/SSHSHA.obj   \
             $(ODIR)/SSHZLIB.obj  \
             $(ODIR)/TELNET.obj   \
             $(ODIR)/TIMING.obj   \
             $(ODIR)/TREE234.obj  \
             $(ODIR)/VERSION.obj  \
             $(ODIR)/WILDCARD.obj \
             $(ODIR)/WINDEFS.obj  \
             $(ODIR)/WINHANDL.obj \
             $(ODIR)/WINMISC.obj  \
             $(ODIR)/WINNET.obj   \
             $(ODIR)/WINNOISE.obj \
             $(ODIR)/WINPGNTC.obj \
             $(ODIR)/WINPROXY.obj \
             $(ODIR)/WINSER.obj   \
             $(ODIR)/WINSTORE.obj \
             $(ODIR)/WINTIME.obj  \
             $(ODIR)/X11FWD.obj

PUTTYOBJS = $(ODIR)/CONFIG.obj   \
            $(ODIR)/DIALOG.obj   \
            $(ODIR)/LDISCUCS.obj \
            $(ODIR)/MINIBIDI.obj \
            $(ODIR)/SERCFG.obj   \
            $(ODIR)/SIZETIP.obj  \
            $(ODIR)/TERMINAL.obj \
            $(ODIR)/WCWIDTH.obj  \
            $(ODIR)/WINCFG.obj   \
            $(ODIR)/WINCTRLS.obj \
            $(ODIR)/WINDLG.obj   \
            $(ODIR)/WINDOW.obj   \
            $(ODIR)/WINHELP.obj  \
            $(ODIR)/WINPRINT.obj \
            $(ODIR)/WINUCS.obj   \
            $(ODIR)/WINUTILS.obj \
            $(ODIR)/PUTTY.res

$(ODIR)/%.obj: %.C
	$(CC) $(CFLAGS) -c $< -Fo$@

$(ODIR)/%.obj: WINDOWS/%.C
	$(CC) $(CFLAGS) -c $< -Fo$@

$(ODIR)/%.res: WINDOWS/%.RC
	"rc" -DNO_MANIFESTS -l 0x809 -fo $@ $<

CFLAGS := $(CFLAGS) -D_WINDOWS -DRELEASE=0.60-VGL -I./WINDOWS

TARGETS = $(ODIR)/putty.exe

all: $(TARGETS)

clean:
	-$(RM) $(TARGETS) $(PUTTYOBJS) $(COMMONOBJS)

HDRS := $(wildcard *.H WINDOWS/*.H)
$(OBJS): $(HDRS)

$(ODIR)/putty.exe: $(PUTTYOBJS) $(COMMONOBJS)
	$(LINK) $(LDFLAGS) $^ -out:$@ user32.lib gdi32.lib imm32.lib advapi32.lib \
		comctl32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib
