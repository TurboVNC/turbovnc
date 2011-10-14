XCOMM $XFree86: xc/lib/GL/mesa/src/swrast/Imakefile.inc,v 1.1tsi Exp $

#ifndef MesaSwrastBuildDir
#define MesaSwrastBuildDir $(GLXLIBSRC)/mesa/swrast/
#endif

MESASWRASTBUILDDIR = MesaSwrastBuildDir

  MESA_SWRAST_SRCS = $(MESASWRASTBUILDDIR)s_aaline.c \
		     $(MESASWRASTBUILDDIR)s_aatriangle.c \
		     $(MESASWRASTBUILDDIR)s_accum.c \
		     $(MESASWRASTBUILDDIR)s_alpha.c \
                     $(MESASWRASTBUILDDIR)s_atifragshader.c \
		     $(MESASWRASTBUILDDIR)s_bitmap.c \
		     $(MESASWRASTBUILDDIR)s_blend.c \
		     $(MESASWRASTBUILDDIR)s_buffers.c \
		     $(MESASWRASTBUILDDIR)s_context.c \
		     $(MESASWRASTBUILDDIR)s_copypix.c \
		     $(MESASWRASTBUILDDIR)s_depth.c \
		     $(MESASWRASTBUILDDIR)s_drawpix.c \
		     $(MESASWRASTBUILDDIR)s_feedback.c \
		     $(MESASWRASTBUILDDIR)s_fog.c \
		     $(MESASWRASTBUILDDIR)s_imaging.c \
		     $(MESASWRASTBUILDDIR)s_lines.c \
		     $(MESASWRASTBUILDDIR)s_logic.c \
		     $(MESASWRASTBUILDDIR)s_masking.c \
		     $(MESASWRASTBUILDDIR)s_nvfragprog.c \
		     $(MESASWRASTBUILDDIR)s_pixeltex.c \
		     $(MESASWRASTBUILDDIR)s_points.c \
		     $(MESASWRASTBUILDDIR)s_readpix.c \
		     $(MESASWRASTBUILDDIR)s_span.c \
		     $(MESASWRASTBUILDDIR)s_stencil.c \
		     $(MESASWRASTBUILDDIR)s_texstore.c \
		     $(MESASWRASTBUILDDIR)s_texture.c \
		     $(MESASWRASTBUILDDIR)s_triangle.c \
		     $(MESASWRASTBUILDDIR)s_zoom.c

#ifdef NeedToLinkMesaSrc
LinkSourceFile(s_aaline.c, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_aaline.h, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_aalinetemp.h, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_aatriangle.c, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_aatriangle.h, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_aatritemp.h, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_accum.c, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_accum.h, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_alpha.c, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_alpha.h, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_atifragshader.c, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_atifragshader.h, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_bitmap.c, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_blend.c, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_blend.h, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_buffers.c, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_context.c, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_context.h, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_copypix.c, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_depth.c, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_depth.h, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_drawpix.c, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_drawpix.h, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_feedback.c, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_feedback.h, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_fog.c, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_fog.h, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_imaging.c, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_lines.c, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_lines.h, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_linetemp.h, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_logic.c, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_logic.h, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_masking.c, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_masking.h, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_nvfragprog.c, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_nvfragprog.h, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_pixeltex.c, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_pixeltex.h, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_points.c, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_points.h, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_pointtemp.h, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_readpix.c, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_span.c, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_span.h, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_spantemp.h, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_stencil.c, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_stencil.h, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_texstore.c, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_texture.c, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_texture.h, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_triangle.c, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_triangle.h, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_trispan.h, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_tritemp.h, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_zoom.c, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(s_zoom.h, $(MESASRCDIR)/src/mesa/swrast)
LinkSourceFile(swrast.h, $(MESASRCDIR)/src/mesa/swrast)
#endif

  MESA_SWRAST_OBJS = $(MESASWRASTBUILDDIR)s_aaline.o \
		     $(MESASWRASTBUILDDIR)s_aatriangle.o \
		     $(MESASWRASTBUILDDIR)s_accum.o \
		     $(MESASWRASTBUILDDIR)s_alpha.o \
                     $(MESASWRASTBUILDDIR)s_atifragshader.o \
		     $(MESASWRASTBUILDDIR)s_bitmap.o \
		     $(MESASWRASTBUILDDIR)s_blend.o \
		     $(MESASWRASTBUILDDIR)s_buffers.o \
		     $(MESASWRASTBUILDDIR)s_context.o \
		     $(MESASWRASTBUILDDIR)s_copypix.o \
		     $(MESASWRASTBUILDDIR)s_depth.o \
		     $(MESASWRASTBUILDDIR)s_drawpix.o \
		     $(MESASWRASTBUILDDIR)s_feedback.o \
		     $(MESASWRASTBUILDDIR)s_fog.o \
		     $(MESASWRASTBUILDDIR)s_imaging.o \
		     $(MESASWRASTBUILDDIR)s_lines.o \
		     $(MESASWRASTBUILDDIR)s_logic.o \
		     $(MESASWRASTBUILDDIR)s_masking.o \
		     $(MESASWRASTBUILDDIR)s_nvfragprog.o \
		     $(MESASWRASTBUILDDIR)s_pixeltex.o \
		     $(MESASWRASTBUILDDIR)s_points.o \
		     $(MESASWRASTBUILDDIR)s_readpix.o \
		     $(MESASWRASTBUILDDIR)s_span.o \
		     $(MESASWRASTBUILDDIR)s_stencil.o \
		     $(MESASWRASTBUILDDIR)s_texstore.o \
		     $(MESASWRASTBUILDDIR)s_texture.o \
		     $(MESASWRASTBUILDDIR)s_triangle.o \
		     $(MESASWRASTBUILDDIR)s_zoom.o

#if defined(DoSharedLib) && DoSharedLib
 MESA_SWRAST_UOBJS = $(MESASWRASTBUILDDIR)unshared/s_aaline.o \
		     $(MESASWRASTBUILDDIR)unshared/s_aatriangle.o \
		     $(MESASWRASTBUILDDIR)unshared/s_accum.o \
		     $(MESASWRASTBUILDDIR)unshared/s_alpha.o \
                     $(MESASWRASTBUILDDIR)unshared/s_atifragshader.o \
		     $(MESASWRASTBUILDDIR)unshared/s_bitmap.o \
		     $(MESASWRASTBUILDDIR)unshared/s_blend.o \
		     $(MESASWRASTBUILDDIR)unshared/s_buffers.o \
		     $(MESASWRASTBUILDDIR)unshared/s_context.o \
		     $(MESASWRASTBUILDDIR)unshared/s_copypix.o \
		     $(MESASWRASTBUILDDIR)unshared/s_depth.o \
		     $(MESASWRASTBUILDDIR)unshared/s_drawpix.o \
		     $(MESASWRASTBUILDDIR)unshared/s_feedback.o \
		     $(MESASWRASTBUILDDIR)unshared/s_fog.o \
		     $(MESASWRASTBUILDDIR)unshared/s_imaging.o \
		     $(MESASWRASTBUILDDIR)unshared/s_lines.o \
		     $(MESASWRASTBUILDDIR)unshared/s_logic.o \
		     $(MESASWRASTBUILDDIR)unshared/s_masking.o \
		     $(MESASWRASTBUILDDIR)unshared/s_nvfragprog.o \
		     $(MESASWRASTBUILDDIR)unshared/s_pixeltex.o \
		     $(MESASWRASTBUILDDIR)unshared/s_points.o \
		     $(MESASWRASTBUILDDIR)unshared/s_readpix.o \
		     $(MESASWRASTBUILDDIR)unshared/s_span.o \
		     $(MESASWRASTBUILDDIR)unshared/s_stencil.o \
		     $(MESASWRASTBUILDDIR)unshared/s_texstore.o \
		     $(MESASWRASTBUILDDIR)unshared/s_texture.o \
		     $(MESASWRASTBUILDDIR)unshared/s_triangle.o \
		     $(MESASWRASTBUILDDIR)unshared/s_zoom.o
#else
 MESA_SWRAST_UOBJS = $(MESA_SWRAST_OBJS)
#endif

 MESA_SWRAST_DOBJS = $(MESASWRASTBUILDDIR)debugger/s_aaline.o \
		     $(MESASWRASTBUILDDIR)debugger/s_aatriangle.o \
		     $(MESASWRASTBUILDDIR)debugger/s_accum.o \
		     $(MESASWRASTBUILDDIR)debugger/s_alpha.o \
                     $(MESASWRASTBUILDDIR)debugger/s_atifragshader.o \
		     $(MESASWRASTBUILDDIR)debugger/s_bitmap.o \
		     $(MESASWRASTBUILDDIR)debugger/s_blend.o \
		     $(MESASWRASTBUILDDIR)debugger/s_buffers.o \
		     $(MESASWRASTBUILDDIR)debugger/s_context.o \
		     $(MESASWRASTBUILDDIR)debugger/s_copypix.o \
		     $(MESASWRASTBUILDDIR)debugger/s_depth.o \
		     $(MESASWRASTBUILDDIR)debugger/s_drawpix.o \
		     $(MESASWRASTBUILDDIR)debugger/s_feedback.o \
		     $(MESASWRASTBUILDDIR)debugger/s_fog.o \
		     $(MESASWRASTBUILDDIR)debugger/s_imaging.o \
		     $(MESASWRASTBUILDDIR)debugger/s_lines.o \
		     $(MESASWRASTBUILDDIR)debugger/s_logic.o \
		     $(MESASWRASTBUILDDIR)debugger/s_masking.o \
		     $(MESASWRASTBUILDDIR)debugger/s_nvfragprog.o \
		     $(MESASWRASTBUILDDIR)debugger/s_pixeltex.o \
		     $(MESASWRASTBUILDDIR)debugger/s_points.o \
		     $(MESASWRASTBUILDDIR)debugger/s_readpix.o \
		     $(MESASWRASTBUILDDIR)debugger/s_span.o \
		     $(MESASWRASTBUILDDIR)debugger/s_stencil.o \
		     $(MESASWRASTBUILDDIR)debugger/s_texstore.o \
		     $(MESASWRASTBUILDDIR)debugger/s_texture.o \
		     $(MESASWRASTBUILDDIR)debugger/s_triangle.o \
		     $(MESASWRASTBUILDDIR)debugger/s_zoom.o

 MESA_SWRAST_POBJS = $(MESASWRASTBUILDDIR)profiled/s_aaline.o \
		     $(MESASWRASTBUILDDIR)profiled/s_aatriangle.o \
		     $(MESASWRASTBUILDDIR)profiled/s_accum.o \
		     $(MESASWRASTBUILDDIR)profiled/s_alpha.o \
                     $(MESASWRASTBUILDDIR)profiled/s_atifragshader.o \
		     $(MESASWRASTBUILDDIR)profiled/s_bitmap.o \
		     $(MESASWRASTBUILDDIR)profiled/s_blend.o \
		     $(MESASWRASTBUILDDIR)profiled/s_buffers.o \
		     $(MESASWRASTBUILDDIR)profiled/s_context.o \
		     $(MESASWRASTBUILDDIR)profiled/s_copypix.o \
		     $(MESASWRASTBUILDDIR)profiled/s_depth.o \
		     $(MESASWRASTBUILDDIR)profiled/s_drawpix.o \
		     $(MESASWRASTBUILDDIR)profiled/s_feedback.o \
		     $(MESASWRASTBUILDDIR)profiled/s_fog.o \
		     $(MESASWRASTBUILDDIR)profiled/s_imaging.o \
		     $(MESASWRASTBUILDDIR)profiled/s_lines.o \
		     $(MESASWRASTBUILDDIR)profiled/s_logic.o \
		     $(MESASWRASTBUILDDIR)profiled/s_masking.o \
		     $(MESASWRASTBUILDDIR)profiled/s_nvfragprog.o \
		     $(MESASWRASTBUILDDIR)profiled/s_pixeltex.o \
		     $(MESASWRASTBUILDDIR)profiled/s_points.o \
		     $(MESASWRASTBUILDDIR)profiled/s_readpix.o \
		     $(MESASWRASTBUILDDIR)profiled/s_span.o \
		     $(MESASWRASTBUILDDIR)profiled/s_stencil.o \
		     $(MESASWRASTBUILDDIR)profiled/s_texstore.o \
		     $(MESASWRASTBUILDDIR)profiled/s_texture.o \
		     $(MESASWRASTBUILDDIR)profiled/s_triangle.o \
		     $(MESASWRASTBUILDDIR)profiled/s_zoom.o

