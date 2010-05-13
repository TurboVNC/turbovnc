ONAME := obj/winvnc
TOPDIR = ../..

include ../../Makerules

OBJS = $(ODIR)/adler32.obj \
       $(ODIR)/compress.obj \
       $(ODIR)/crc32.obj \
       $(ODIR)/deflate.obj \
       $(ODIR)/gzio.obj \
       $(ODIR)/infback.obj \
       $(ODIR)/inffast.obj \
       $(ODIR)/inflate.obj \
       $(ODIR)/inftrees.obj \
       $(ODIR)/trees.obj \
       $(ODIR)/uncompr.obj \
       $(ODIR)/zutil.obj

TARGETS = $(ODIR)/zlib.lib

default: $(TARGETS)

HDRS := $(wildcard *.h)
$(OBJS): $(HDRS)

$(ODIR)/zlib.lib: $(OBJS)
	$(AR) -out:$@ $^

clean:
	rm -f $(OBJS) $(TARGETS)
