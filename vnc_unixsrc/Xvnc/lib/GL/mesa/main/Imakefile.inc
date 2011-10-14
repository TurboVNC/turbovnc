XCOMM $XFree86: xc/lib/GL/mesa/src/Imakefile.inc,v 1.7tsi Exp $

#ifndef MesaBuildDir
#define MesaBuildDir $(GLXLIBSRC)/mesa/main/
#endif

MESABUILDDIR = MesaBuildDir

#ifdef NeedAllMesaSrc
 DISPATCHSRCS = $(MESABUILDDIR)dispatch.c
    GLAPISRCS = $(MESABUILDDIR)glapi.c
 GLTHREADSRCS = $(MESABUILDDIR)glthread.c
#endif

MESA_CORE_INCLUDES = -I$(MESASRCDIR)/src/array_cache \
                -I$(MESASRCDIR)/src/math -I$(MESASRCDIR)/src/shader \
                -I$(MESASRCDIR)/src/swrast -I$(MESASRCDIR)/src/swrast_setup \
                -I$(MESASRCDIR)/src/tnl -I$(MESASRCDIR)/src/tnl_dd

COREMESABASESRCS = \
		$(MESABUILDDIR)api_arrayelt.c \
		$(MESABUILDDIR)api_loopback.c \
		$(MESABUILDDIR)api_noop.c \
		$(MESABUILDDIR)api_validate.c \
		$(MESABUILDDIR)accum.c \
		$(MESABUILDDIR)attrib.c \
		$(MESABUILDDIR)blend.c \
		$(MESABUILDDIR)bufferobj.c \
		$(MESABUILDDIR)buffers.c \
		$(MESABUILDDIR)clip.c \
		$(MESABUILDDIR)colortab.c \
		$(MESABUILDDIR)context.c \
		$(MESABUILDDIR)convolve.c \
		$(MESABUILDDIR)debug.c \
		$(MESABUILDDIR)depth.c \
		$(MESABUILDDIR)dlist.c \
		$(MESABUILDDIR)drawpix.c \
		$(MESABUILDDIR)enable.c \
		$(MESABUILDDIR)enums.c \
		$(MESABUILDDIR)eval.c \
		$(MESABUILDDIR)execmem.c \
		$(MESABUILDDIR)extensions.c \
		$(MESABUILDDIR)feedback.c \
                $(MESABUILDDIR)fbobject.c \
		$(MESABUILDDIR)fog.c \
                $(MESABUILDDIR)framebuffer.c \
		$(MESABUILDDIR)get.c \
                $(MESABUILDDIR)getstring.c \
		$(MESABUILDDIR)hash.c \
		$(MESABUILDDIR)hint.c \
		$(MESABUILDDIR)histogram.c \
		$(MESABUILDDIR)image.c \
		$(MESABUILDDIR)imports.c \
		$(MESABUILDDIR)light.c \
		$(MESABUILDDIR)lines.c \
		$(MESABUILDDIR)matrix.c \
		$(MESABUILDDIR)mm.c \
		$(MESABUILDDIR)occlude.c \
		$(MESABUILDDIR)pixel.c \
		$(MESABUILDDIR)points.c \
		$(MESABUILDDIR)polygon.c \
		$(MESABUILDDIR)rastpos.c \
                $(MESABUILDDIR)renderbuffer.c \
		$(MESABUILDDIR)state.c \
		$(MESABUILDDIR)stencil.c \
		$(MESABUILDDIR)texcompress.c \
		$(MESABUILDDIR)texcompress_fxt1.c \
		$(MESABUILDDIR)texcompress_s3tc.c \
                $(MESABUILDDIR)texenvprogram.c \
		$(MESABUILDDIR)texformat.c \
		$(MESABUILDDIR)teximage.c \
		$(MESABUILDDIR)texobj.c \
                $(MESABUILDDIR)texrender.c \
		$(MESABUILDDIR)texstate.c \
		$(MESABUILDDIR)texstore.c \
		$(MESABUILDDIR)varray.c \
		$(MESABUILDDIR)vtxfmt.c

#ifdef NeedToLinkMesaSrc
LinkSourceFile(api_arrayelt.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(api_arrayelt.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(api_eval.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(api_loopback.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(api_loopback.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(api_noop.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(api_noop.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(api_validate.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(api_validate.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(accum.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(accum.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(attrib.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(attrib.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(blend.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(blend.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(bufferobj.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(bufferobj.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(buffers.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(buffers.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(clip.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(clip.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(colormac.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(colortab.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(colortab.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(config.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(context.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(context.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(convolve.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(convolve.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(dd.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(debug.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(debug.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(depth.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(depth.h, $(MESASRCDIR)/src/mesa/main)
#ifdef NeedAllMesaSrc
LinkSourceFile(dispatch.c, $(MESASRCDIR)/src/mesa/main)
#endif
LinkSourceFile(dlist.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(dlist.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(drawpix.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(drawpix.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(enable.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(enable.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(enums.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(enums.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(eval.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(eval.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(execmem.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(extensions.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(extensions.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(fbobject.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(fbobject.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(feedback.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(feedback.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(fog.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(fog.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(framebuffer.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(framebuffer.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(get.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(get.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(getstring.c, $(MESASRCDIR)/src/mesa/main)
#ifdef NeedAllMesaSrc
LinkSourceFile(glapi.c, $(MESASRCDIR)/src/mesa/glapi)
LinkSourceFile(glthread.c, $(MESASRCDIR)/src/mesa/glapi)
#endif
LinkSourceFile(glheader.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(hash.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(hash.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(hint.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(hint.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(histogram.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(histogram.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(image.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(image.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(imports.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(imports.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(light.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(light.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(lines.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(lines.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(macros.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(matrix.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(matrix.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(mm.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(mm.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(mtypes.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(occlude.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(occlude.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(pixel.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(pixel.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(points.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(points.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(polygon.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(polygon.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(rastpos.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(rastpos.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(renderbuffer.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(renderbuffer.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(simple_list.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(state.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(state.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(stencil.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(stencil.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(texcompress.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(texcompress_fxt1.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(texcompress_s3tc.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(texcompress.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(texenvprogram.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(texenvprogram.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(texformat.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(texformat.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(texformat_tmp.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(teximage.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(teximage.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(texobj.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(texobj.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(texrender.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(texrender.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(texstate.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(texstate.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(texstore.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(texstore.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(varray.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(varray.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(version.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(vtxfmt.c, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(vtxfmt.h, $(MESASRCDIR)/src/mesa/main)
LinkSourceFile(vtxfmt_tmp.h, $(MESASRCDIR)/src/mesa/main)
#endif

#ifdef NeedAllMesaSrc
 DISPATCHOBJS = $(MESABUILDDIR)dispatch.o
    GLAPIOBJS = $(MESABUILDDIR)glapi.o
 GLTHREADOBJS = $(MESABUILDDIR)glthread.o
#endif

COREMESABASEOBJS = \
		$(DISPATCHOBJS) \
		$(GLAPIOBJS) \
		$(GLTHREADOBJS) \
		$(MESABUILDDIR)api_arrayelt.o \
		$(MESABUILDDIR)api_loopback.o \
		$(MESABUILDDIR)api_noop.o \
		$(MESABUILDDIR)api_validate.o \
		$(MESABUILDDIR)accum.o \
		$(MESABUILDDIR)attrib.o \
		$(MESABUILDDIR)blend.o \
		$(MESABUILDDIR)bufferobj.o \
		$(MESABUILDDIR)buffers.o \
		$(MESABUILDDIR)clip.o \
		$(MESABUILDDIR)colortab.o \
		$(MESABUILDDIR)context.o \
		$(MESABUILDDIR)convolve.o \
		$(MESABUILDDIR)debug.o \
		$(MESABUILDDIR)depth.o \
		$(MESABUILDDIR)dlist.o \
		$(MESABUILDDIR)drawpix.o \
		$(MESABUILDDIR)enable.o \
		$(MESABUILDDIR)enums.o \
		$(MESABUILDDIR)eval.o \
		$(MESABUILDDIR)execmem.o \
		$(MESABUILDDIR)extensions.o \
                $(MESABUILDDIR)fbobject.o \
		$(MESABUILDDIR)feedback.o \
		$(MESABUILDDIR)fog.o \
                $(MESABUILDDIR)framebuffer.o \
		$(MESABUILDDIR)get.o \
                $(MESABUILDDIR)getstring.o \
		$(MESABUILDDIR)hash.o \
		$(MESABUILDDIR)hint.o \
		$(MESABUILDDIR)histogram.o \
		$(MESABUILDDIR)image.o \
		$(MESABUILDDIR)imports.o \
		$(MESABUILDDIR)light.o \
		$(MESABUILDDIR)lines.o \
		$(MESABUILDDIR)matrix.o \
		$(MESABUILDDIR)mm.o \
		$(MESABUILDDIR)occlude.o \
		$(MESABUILDDIR)pixel.o \
		$(MESABUILDDIR)points.o \
		$(MESABUILDDIR)polygon.o \
		$(MESABUILDDIR)rastpos.o \
                $(MESABUILDDIR)renderbuffer.o \
		$(MESABUILDDIR)state.o \
		$(MESABUILDDIR)stencil.o \
		$(MESABUILDDIR)texcompress.o \
		$(MESABUILDDIR)texcompress_fxt1.o \
		$(MESABUILDDIR)texcompress_s3tc.o \
                $(MESABUILDDIR)texenvprogram.o \
		$(MESABUILDDIR)texformat.o \
		$(MESABUILDDIR)teximage.o \
                $(MESABUILDDIR)texrender.o \
		$(MESABUILDDIR)texobj.o \
		$(MESABUILDDIR)texstate.o \
		$(MESABUILDDIR)texstore.o \
		$(MESABUILDDIR)varray.o \
		$(MESABUILDDIR)vtxfmt.o

#ifdef NeedAllMesaSrc
 DISPATCHUOBJS = $(MESABUILDDIR)unshared/dispatch.o
    GLAPIUOBJS = $(MESABUILDDIR)unshared/glapi.o
 GLTHREADUOBJS = $(MESABUILDDIR)unshared/glthread.o
#endif

#if defined(DoSharedLib) && DoSharedLib
COREMESABASEUOBJS = \
		$(DISPATCHUOBJS) \
		$(GLAPIUOBJS) \
		$(GLTHREADUOBJS) \
		$(MESABUILDDIR)unshared/api_arrayelt.o \
		$(MESABUILDDIR)unshared/api_loopback.o \
		$(MESABUILDDIR)unshared/api_noop.o \
		$(MESABUILDDIR)unshared/api_validate.o \
		$(MESABUILDDIR)unshared/accum.o \
		$(MESABUILDDIR)unshared/attrib.o \
		$(MESABUILDDIR)unshared/blend.o \
		$(MESABUILDDIR)unshared/bufferobj.o \
		$(MESABUILDDIR)unshared/buffers.o \
		$(MESABUILDDIR)unshared/clip.o \
		$(MESABUILDDIR)unshared/colortab.o \
		$(MESABUILDDIR)unshared/context.o \
		$(MESABUILDDIR)unshared/convolve.o \
		$(MESABUILDDIR)unshared/debug.o \
		$(MESABUILDDIR)unshared/depth.o \
		$(MESABUILDDIR)unshared/dlist.o \
		$(MESABUILDDIR)unshared/drawpix.o \
		$(MESABUILDDIR)unshared/enable.o \
		$(MESABUILDDIR)unshared/enums.o \
		$(MESABUILDDIR)unshared/eval.o \
		$(MESABUILDDIR)unshared/execmem.o \
		$(MESABUILDDIR)unshared/extensions.o \
                $(MESABUILDDIR)unshared/fbobject.o \
		$(MESABUILDDIR)unshared/feedback.o \
		$(MESABUILDDIR)unshared/fog.o \
                $(MESABUILDDIR)unshared/framebuffer.o \
		$(MESABUILDDIR)unshared/get.o \
                $(MESABUILDDIR)unshared/getstring.o \
		$(MESABUILDDIR)unshared/hash.o \
		$(MESABUILDDIR)unshared/hint.o \
		$(MESABUILDDIR)unshared/histogram.o \
		$(MESABUILDDIR)unshared/image.o \
		$(MESABUILDDIR)unshared/imports.o \
		$(MESABUILDDIR)unshared/light.o \
		$(MESABUILDDIR)unshared/lines.o \
		$(MESABUILDDIR)unshared/matrix.o \
		$(MESABUILDDIR)unshared/mm.o \
		$(MESABUILDDIR)unshared/occlude.o \
		$(MESABUILDDIR)unshared/pixel.o \
		$(MESABUILDDIR)unshared/points.o \
		$(MESABUILDDIR)unshared/polygon.o \
		$(MESABUILDDIR)unshared/rastpos.o \
                $(MESABUILDDIR)unshared/renderbuffer.o \
		$(MESABUILDDIR)unshared/state.o \
		$(MESABUILDDIR)unshared/stencil.o \
		$(MESABUILDDIR)unshared/texcompress.o \
		$(MESABUILDDIR)unshared/texcompress_fxt1.o \
		$(MESABUILDDIR)unshared/texcompress_s3tc.o \
                $(MESABUILDDIR)unshared/texenvprogram.o \
		$(MESABUILDDIR)unshared/texformat.o \
		$(MESABUILDDIR)unshared/teximage.o \
		$(MESABUILDDIR)unshared/texobj.o \
                $(MESABUILDDIR)unshared/texrender.o \
		$(MESABUILDDIR)unshared/texstate.o \
		$(MESABUILDDIR)unshared/texstore.o \
		$(MESABUILDDIR)unshared/varray.o \
		$(MESABUILDDIR)unshared/vtxfmt.o
#else
COREMESABASEUOBJS = $(COREMESABASEOBJS)
#endif

#ifdef NeedAllMesaSrc
 DISPATCHDOBJS = $(MESABUILDDIR)debugger/dispatch.o
    GLAPIDOBJS = $(MESABUILDDIR)debugger/glapi.o
 GLTHREADDOBJS = $(MESABUILDDIR)debugger/glthread.o
#endif

COREMESABASEDOBJS = \
		$(DISPATCHDOBJS) \
		$(GLAPIDOBJS) \
		$(GLTHREADDOBJS) \
		$(MESABUILDDIR)debugger/api_arrayelt.o \
		$(MESABUILDDIR)debugger/api_loopback.o \
		$(MESABUILDDIR)debugger/api_noop.o \
		$(MESABUILDDIR)debugger/api_validate.o \
		$(MESABUILDDIR)debugger/accum.o \
		$(MESABUILDDIR)debugger/attrib.o \
		$(MESABUILDDIR)debugger/blend.o \
		$(MESABUILDDIR)debugger/bufferobj.o \
		$(MESABUILDDIR)debugger/buffers.o \
		$(MESABUILDDIR)debugger/clip.o \
		$(MESABUILDDIR)debugger/colortab.o \
		$(MESABUILDDIR)debugger/context.o \
		$(MESABUILDDIR)debugger/convolve.o \
		$(MESABUILDDIR)debugger/debug.o \
		$(MESABUILDDIR)debugger/depth.o \
		$(MESABUILDDIR)debugger/dlist.o \
		$(MESABUILDDIR)debugger/drawpix.o \
		$(MESABUILDDIR)debugger/enable.o \
		$(MESABUILDDIR)debugger/enums.o \
		$(MESABUILDDIR)debugger/eval.o \
		$(MESABUILDDIR)debugger/execmem.o \
		$(MESABUILDDIR)debugger/extensions.o \
                $(MESABUILDDIR)debugger/fbobject.o \
		$(MESABUILDDIR)debugger/feedback.o \
		$(MESABUILDDIR)debugger/fog.o \
                $(MESABUILDDIR)debugger/framebuffer.o \
		$(MESABUILDDIR)debugger/get.o \
                $(MESABUILDDIR)debugger/getstring.o \
		$(MESABUILDDIR)debugger/hash.o \
		$(MESABUILDDIR)debugger/hint.o \
		$(MESABUILDDIR)debugger/histogram.o \
		$(MESABUILDDIR)debugger/image.o \
		$(MESABUILDDIR)debugger/imports.o \
		$(MESABUILDDIR)debugger/light.o \
		$(MESABUILDDIR)debugger/lines.o \
		$(MESABUILDDIR)debugger/matrix.o \
		$(MESABUILDDIR)debugger/mm.o \
		$(MESABUILDDIR)debugger/occlude.o \
		$(MESABUILDDIR)debugger/pixel.o \
		$(MESABUILDDIR)debugger/points.o \
		$(MESABUILDDIR)debugger/polygon.o \
		$(MESABUILDDIR)debugger/rastpos.o \
                $(MESABUILDDIR)debugger/renderbuffer.o \
		$(MESABUILDDIR)debugger/state.o \
		$(MESABUILDDIR)debugger/stencil.o \
		$(MESABUILDDIR)debugger/texcompress.o \
		$(MESABUILDDIR)debugger/texcompress_fxt1.o \
		$(MESABUILDDIR)debugger/texcompress_s3tc.o \
                $(MESABUILDDIR)debugger/texenvprogram.o \
		$(MESABUILDDIR)debugger/texformat.o \
		$(MESABUILDDIR)debugger/teximage.o \
		$(MESABUILDDIR)debugger/texobj.o \
                $(MESABUILDDIR)debugger/texrender.o \
		$(MESABUILDDIR)debugger/texstate.o \
		$(MESABUILDDIR)debugger/texstore.o \
		$(MESABUILDDIR)debugger/varray.o \
		$(MESABUILDDIR)debugger/vtxfmt.o

#ifdef NeedAllMesaSrc
 DISPATCHPOBJS = $(MESABUILDDIR)profiled/dispatch.o
    GLAPIPOBJS = $(MESABUILDDIR)profiled/glapi.o
 GLTHREADPOBJS = $(MESABUILDDIR)profiled/glthread.o
#endif

COREMESABASEPOBJS = \
		$(DISPATCHPOBJS) \
		$(GLAPIPOBJS) \
		$(GLTHREADPOBJS) \
		$(MESABUILDDIR)profiled/api_arrayelt.o \
		$(MESABUILDDIR)profiled/api_loopback.o \
		$(MESABUILDDIR)profiled/api_noop.o \
		$(MESABUILDDIR)profiled/api_validate.o \
		$(MESABUILDDIR)profiled/accum.o \
		$(MESABUILDDIR)profiled/attrib.o \
		$(MESABUILDDIR)profiled/blend.o \
		$(MESABUILDDIR)profiled/bufferobj.o \
		$(MESABUILDDIR)profiled/buffers.o \
		$(MESABUILDDIR)profiled/clip.o \
		$(MESABUILDDIR)profiled/colortab.o \
		$(MESABUILDDIR)profiled/context.o \
		$(MESABUILDDIR)profiled/convolve.o \
		$(MESABUILDDIR)profiled/debug.o \
		$(MESABUILDDIR)profiled/depth.o \
		$(MESABUILDDIR)profiled/dlist.o \
		$(MESABUILDDIR)profiled/drawpix.o \
		$(MESABUILDDIR)profiled/enable.o \
		$(MESABUILDDIR)profiled/enums.o \
		$(MESABUILDDIR)profiled/eval.o \
		$(MESABUILDDIR)profiled/execmem.o \
		$(MESABUILDDIR)profiled/extensions.o \
                $(MESABUILDDIR)profiled/fbobject.o \
		$(MESABUILDDIR)profiled/feedback.o \
		$(MESABUILDDIR)profiled/fog.o \
                $(MESABUILDDIR)profiled/framebuffer.o \
		$(MESABUILDDIR)profiled/get.o \
                $(MESABUILDDIR)profiled/getstring.o \
		$(MESABUILDDIR)profiled/hash.o \
		$(MESABUILDDIR)profiled/hint.o \
		$(MESABUILDDIR)profiled/histogram.o \
		$(MESABUILDDIR)profiled/image.o \
		$(MESABUILDDIR)profiled/imports.o \
		$(MESABUILDDIR)profiled/light.o \
		$(MESABUILDDIR)profiled/lines.o \
		$(MESABUILDDIR)profiled/matrix.o \
		$(MESABUILDDIR)profiled/mm.o \
		$(MESABUILDDIR)profiled/occlude.o \
		$(MESABUILDDIR)profiled/pixel.o \
		$(MESABUILDDIR)profiled/points.o \
		$(MESABUILDDIR)profiled/polygon.o \
		$(MESABUILDDIR)profiled/rastpos.o \
                $(MESABUILDDIR)profiled/renderbuffer.o \
		$(MESABUILDDIR)profiled/state.o \
		$(MESABUILDDIR)profiled/stencil.o \
		$(MESABUILDDIR)profiled/texcompress.o \
		$(MESABUILDDIR)profiled/texcompress_fxt1.o \
		$(MESABUILDDIR)profiled/texcompress_s3tc.o \
                $(MESABUILDDIR)profiled/texenvprogram.o \
		$(MESABUILDDIR)profiled/texformat.o \
		$(MESABUILDDIR)profiled/teximage.o \
		$(MESABUILDDIR)profiled/texobj.o \
                $(MESABUILDDIR)profiled/texrender.o \
		$(MESABUILDDIR)profiled/texstate.o \
		$(MESABUILDDIR)profiled/texstore.o \
		$(MESABUILDDIR)profiled/varray.o \
		$(MESABUILDDIR)profiled/vtxfmt.o
