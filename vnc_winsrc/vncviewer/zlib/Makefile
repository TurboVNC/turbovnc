TOPDIR = ..
include ../Makerules

CFLAGS = $(OPTFLAG) -Wall -D_MBCS -D_LIB -D_WINDOWS -D__NT__ -D_WINSTATIC \
	-D__WIN32__

OBJS = $(ODIR)/adler32.o \
       $(ODIR)/compress.o \
       $(ODIR)/crc32.o \
       $(ODIR)/deflate.o \
       $(ODIR)/infblock.o \
       $(ODIR)/infcodes.o \
       $(ODIR)/inffast.o \
       $(ODIR)/inflate.o \
       $(ODIR)/inftrees.o \
       $(ODIR)/infutil.o \
       $(ODIR)/trees.o \
       $(ODIR)/uncompr.o \
       $(ODIR)/zutil.o

TARGETS = $(ODIR)/libz.a

default: $(TARGETS)

-include $(OBJS:.o=.d)

HDRS := $(wildcard *.h)
$(OBJS): $(HDRS)

$(ODIR)/libz.a: $(OBJS)
	ar ruvs $@ $^

clean:
	rm -f $(OBJS) $(OBJS:.o=.d) $(TARGETS)
