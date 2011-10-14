XCOMM $XFree86: xc/lib/GL/mesa/src/X/Imakefile.inc,v 1.3tsi Exp $

#ifndef MesaXBuildDir
#define MesaXBuildDir $(GLXLIBSRC)/src/mesa/drivers/x11/
#endif

MESAXBUILDDIR = MesaXBuildDir

#ifndef MesaInXServer
XMESA_GLX_SRCS = $(MESAXBUILDDIR)fakeglx.c \
		 $(MESAXBUILDDIR)glxapi.c \
		 $(MESAXBUILDDIR)realglx.c \
		 $(MESAXBUILDDIR)xfonts.c
#endif

   XMESA_SRCS = $(XMESA_GLX_SRCS) \
		$(MESAXBUILDDIR)xm_api.c \
                $(MESAXBUILDDIR)xm_buffer.c \
		$(MESAXBUILDDIR)xm_dd.c \
		$(MESAXBUILDDIR)xm_line.c \
		$(MESAXBUILDDIR)xm_span.c \
		$(MESAXBUILDDIR)xm_tri.c

#ifdef NeedToLinkMesaSrc
#ifndef MesaInXServer
LinkSourceFile(fakeglx.c, $(MESASRCDIR)/src/mesa/drivers/x11)
LinkSourceFile(glxapi.c, $(MESASRCDIR)/src/mesa/drivers/x11)
LinkSourceFile(glxapi.h, $(MESASRCDIR)/src/mesa/drivers/x11)
LinkSourceFile(realglx.c, $(MESASRCDIR)/src/mesa/drivers/x11)
LinkSourceFile(xfonts.c, $(MESASRCDIR)/src/mesa/drivers/x11)
#endif
LinkSourceFile(xm_api.c, $(MESASRCDIR)/src/mesa/drivers/x11)
LinkSourceFile(xm_buffer.c, $(MESASRCDIR)/src/mesa/drivers/x11)
LinkSourceFile(xm_dd.c, $(MESASRCDIR)/src/mesa/drivers/x11)
LinkSourceFile(xm_line.c, $(MESASRCDIR)/src/mesa/drivers/x11)
LinkSourceFile(xm_span.c, $(MESASRCDIR)/src/mesa/drivers/x11)
LinkSourceFile(xm_tri.c, $(MESASRCDIR)/src/mesa/drivers/x11)
#endif

#ifndef MesaInXServer
XMESA_GLX_OBJS = $(MESAXBUILDDIR)fakeglx.o \
		 $(MESAXBUILDDIR)glxapi.o \
		 $(MESAXBUILDDIR)realglx.o \
		 $(MESAXBUILDDIR)xfonts.o
#endif

   XMESA_OBJS = $(XMESA_GLX_OBJS) \
		$(MESAXBUILDDIR)xm_api.o \
                $(MESAXBUILDDIR)xm_buffer.o \
		$(MESAXBUILDDIR)xm_dd.o \
		$(MESAXBUILDDIR)xm_line.o \
		$(MESAXBUILDDIR)xm_span.o \
		$(MESAXBUILDDIR)xm_tri.o

#if defined(DoSharedLib) && DoSharedLib
#ifndef MesaInXServer
XMESA_GLX_UOBJS = $(MESAXBUILDDIR)unshared/fakeglx.o \
		  $(MESAXBUILDDIR)unshared/glxapi.o \
		  $(MESAXBUILDDIR)unshared/realglx.o \
		  $(MESAXBUILDDIR)unshared/xfonts.o
#endif

  XMESA_UOBJS = $(XMESA_GLX_UOBJS) \
		$(MESAXBUILDDIR)unshared/xm_api.o \
                $(MESAXBUILDDIR)unshared/xm_buffer.o \
		$(MESAXBUILDDIR)unshared/xm_dd.o \
		$(MESAXBUILDDIR)unshared/xm_line.o \
		$(MESAXBUILDDIR)unshared/xm_span.o \
		$(MESAXBUILDDIR)unshared/xm_tri.o
#else
XMESA_GLX_UOBJS = $(XMESA_GLX_OBJS)
  XMESA_UOBJS = $(XMESA_OBJS)
#endif

#ifndef MesaInXServer
XMESA_GLX_DOBJS = $(MESAXBUILDDIR)debugger/fakeglx.o \
		  $(MESAXBUILDDIR)debugger/glxapi.o \
		  $(MESAXBUILDDIR)debugger/realglx.o \
		  $(MESAXBUILDDIR)debugger/xfonts.o
#endif

  XMESA_DOBJS = $(XMESA_GLX_DOBJS) \
		$(MESAXBUILDDIR)debugger/xm_api.o \
                $(MESAXBUILDDIR)debugger/xm_buffer.o \
		$(MESAXBUILDDIR)debugger/xm_dd.o \
		$(MESAXBUILDDIR)debugger/xm_line.o \
		$(MESAXBUILDDIR)debugger/xm_span.o \
		$(MESAXBUILDDIR)debugger/xm_tri.o

#ifndef MesaInXServer
XMESA_GLX_POBJS = $(MESAXBUILDDIR)profiled/fakeglx.o \
		  $(MESAXBUILDDIR)profiled/glxapi.o \
		  $(MESAXBUILDDIR)profiled/realglx.o \
		  $(MESAXBUILDDIR)profiled/xfonts.o
#endif

  XMESA_POBJS = $(XMESA_GLX_POBJS) \
		$(MESAXBUILDDIR)profiled/xm_api.o \
                $(MESAXBUILDDIR)profiled/xm_buffer.o \
		$(MESAXBUILDDIR)profiled/xm_dd.o \
		$(MESAXBUILDDIR)profiled/xm_line.o \
		$(MESAXBUILDDIR)profiled/xm_span.o \
		$(MESAXBUILDDIR)profiled/xm_tri.o

