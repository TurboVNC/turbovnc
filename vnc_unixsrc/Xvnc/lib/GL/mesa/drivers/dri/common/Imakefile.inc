XCOMM $XFree86: xc/lib/GL/mesa/src/drv/common/Imakefile.inc,v 1.3 2003/05/27 16:34:32 tsi Exp $

#ifndef MesaDrvSrcDir
#define MesaDrvSrcDir $(MESASRCDIR)/src/mesa/drivers/dri
#endif

MESADRVSRCDIR = MesaDrvSrcDir

#ifndef MesaDrvCommonBuildDir
#define MesaDrvCommonBuildDir $(MESADRVSRCDIR)/common/
#endif

MESADRVCOMMONBUILDDIR = MesaDrvCommonBuildDir

#if Malloc0ReturnsNull
ALLOC_DEFINES = -DMALLOC_0_RETURNS_NULL
#endif

#if BuildXF86DRI
 COMMON_DRI_DEFINES = GlxDefines -DX_BYTE_ORDER=$(X_BYTE_ORDER) -DIN_DRI_DRIVER
 COMMON_DRI_INCLUDES = -I$(GLXLIBSRC)/dri \
		-I$(XINCLUDESRC) \
		-I$(GLXLIBSRC)/glx \
		-I$(GLXLIBSRC)/include \
		-I$(SERVERSRC)/GL/dri \
		-I$(XF86OSSRC) \
		-I$(XF86COMSRC) \
		-I$(GLXLIBSRC)/dri/drm \
		-I$(GLXLIBSRC)/include \
		-I$(DRMSRCDIR)/shared-core \
                -I$(MESASRCDIR)/include/GL/internal
#endif

COMMON_MESA_INCLUDES = -I$(MESASRCDIR)/src/mesa \
		-I$(MESASRCDIR)/src/mesa/main \
		-I$(MESASRCDIR)/src/mesa/shader \
		-I$(MESASRCDIR)/src/mesa/glapi \
		-I$(MESADRVSRCDIR)/common \
		-I$(MESADRVSRCDIR)/../common \
		-I$(MESASRCDIR)/include
   COMMON_X_INCLUDES = -I$(XINCLUDESRC) -I$(EXTINCSRC)

    COMMONSRCS = $(MESADRVCOMMONBUILDDIR)utils.c \
                 $(MESADRVCOMMONBUILDDIR)vblank.c \
                 $(MESADRVCOMMONBUILDDIR)texmem.c \
                 $(MESADRVCOMMONBUILDDIR)xmlconfig.c \
                 $(MESADRVCOMMONBUILDDIR)driverfuncs.c \
                 $(MESADRVCOMMONBUILDDIR)dri_util.c \
                 $(MESADRVCOMMONBUILDDIR)glcontextmodes.c \
                 $(MESADRVCOMMONBUILDDIR)drirenderbuffer.c

    COMMONOBJS = $(MESADRVCOMMONBUILDDIR)utils.o \
                 $(MESADRVCOMMONBUILDDIR)vblank.o \
                 $(MESADRVCOMMONBUILDDIR)texmem.o \
                 $(MESADRVCOMMONBUILDDIR)xmlconfig.o \
                 $(MESADRVCOMMONBUILDDIR)driverfuncs.o \
                 $(MESADRVCOMMONBUILDDIR)dri_util.o \
                 $(MESADRVCOMMONBUILDDIR)glcontextmodes.o \
                 $(MESADRVCOMMONBUILDDIR)drirenderbuffer.o

    COMMONUOBJS = $(MESADRVCOMMONBUILDDIR)unshared/utils.o \
                 $(MESADRVCOMMONBUILDDIR)unshared/vblank.o \
                 $(MESADRVCOMMONBUILDDIR)unshared/texmem.o \
                 $(MESADRVCOMMONBUILDDIR)unshared/xmlconfig.o \
                 $(MESADRVCOMMONBUILDDIR)unshared/driverfuncs.o \
                 $(MESADRVCOMMONBUILDDIR)unshared/dri_util.o \
                 $(MESADRVCOMMONBUILDDIR)unshared/glcontextmodes.o \
                 $(MESADRVCOMMONBUILDDIR)unshared/drirenderbuffer.o

    COMMONDOBJS = $(MESADRVCOMMONBUILDDIR)debugger/utils.o \
                 $(MESADRVCOMMONBUILDDIR)debugger/vblank.o \
                 $(MESADRVCOMMONBUILDDIR)debugger/texmem.o \
                 $(MESADRVCOMMONBUILDDIR)debugger/xmlconfig.o \
                 $(MESADRVCOMMONBUILDDIR)debugger/driverfuncs.o \
                 $(MESADRVCOMMONBUILDDIR)debugger/dri_util.o \
                 $(MESADRVCOMMONBUILDDIR)debugger/glcontextmodes.o \
                 $(MESADRVCOMMONBUILDDIR)debugger/drirenderbuffer.o

    COMMONPOBJS = $(MESADRVCOMMONBUILDDIR)profiled/utils.o \
                 $(MESADRVCOMMONBUILDDIR)profiled/vblank.o \
                 $(MESADRVCOMMONBUILDDIR)profiled/texmem.o \
                 $(MESADRVCOMMONBUILDDIR)profiled/xmlconfig.o \
                 $(MESADRVCOMMONBUILDDIR)profiled/driverfuncs.o \
                 $(MESADRVCOMMONBUILDDIR)profiled/dri_util.o \
                 $(MESADRVCOMMONBUILDDIR)profiled/glcontextmodes.o \
                 $(MESADRVCOMMONBUILDDIR)profiled/drirenderbuffer.o

#ifdef NeedToLinkMesaSrc
LinkSourceFile(texmem.c, $(MESADRVSRCDIR)/common)
LinkSourceFile(texmem.h, $(MESADRVSRCDIR)/common)
LinkSourceFile(utils.c, $(MESADRVSRCDIR)/common)
LinkSourceFile(utils.h, $(MESADRVSRCDIR)/common)
LinkSourceFile(vblank.c, $(MESADRVSRCDIR)/common)
LinkSourceFile(vblank.h, $(MESADRVSRCDIR)/common)
LinkSourceFile(xmlconfig.c, $(MESADRVSRCDIR)/common)
LinkSourceFile(xmlconfig.h, $(MESADRVSRCDIR)/common)
LinkSourceFile(driverfuncs.c, $(MESADRVSRCDIR)/../common)
LinkSourceFile(driverfuncs.h, $(MESADRVSRCDIR)/../common)
LinkSourceFile(dri_util.c, $(MESADRVSRCDIR)/common)
LinkSourceFile(dri_util.h, $(MESADRVSRCDIR)/common)
LinkSourceFile(glcontextmodes.c, $(MESADRVSRCDIR)/common)
LinkSourceFile(glcontextmodes.h, $(MESADRVSRCDIR)/common)
LinkSourceFile(drirenderbuffer.c, $(MESADRVSRCDIR)/common)
LinkSourceFile(drirenderbuffer.h, $(MESADRVSRCDIR)/common)
#endif
