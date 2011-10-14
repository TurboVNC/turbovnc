XCOMM $XFree86:$

#ifndef MesaShaderBuildDir
#define MesaShaderBuildDir $(GLXLIBSRC)/mesa/shader/
#endif

MESASHADERBUILDDIR = MesaShaderBuildDir

  MESA_SHADER_SRCS =	$(MESASHADERBUILDDIR)arbprogparse.c \
			$(MESASHADERBUILDDIR)arbprogram.c \
			$(MESASHADERBUILDDIR)arbfragparse.c \
			$(MESASHADERBUILDDIR)arbvertparse.c \
                        $(MESASHADERBUILDDIR)atifragshader.c \
			$(MESASHADERBUILDDIR)nvprogram.c \
			$(MESASHADERBUILDDIR)nvfragparse.c \
			$(MESASHADERBUILDDIR)nvvertexec.c \
			$(MESASHADERBUILDDIR)nvvertparse.c \
			$(MESASHADERBUILDDIR)program.c \
                        $(MESASHADERBUILDDIR)shaderobjects.c \
                        $(MESASHADERBUILDDIR)shaderobjects_3dlabs.c

#ifdef NeedToLinkMesaSrc
LinkSourceFile(arbprogparse.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(arbprogram.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(arbfragparse.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(arbvertparse.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(arbfragparse.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(arbprogparse.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(arbprogram.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(arbprogram_syn.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(arbvertparse.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(atifragshader.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(atifragshader.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(nvfragparse.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(nvfragprog.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(nvprogram.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(nvprogram.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(nvfragparse.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(nvvertexec.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(nvvertexec.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(nvvertparse.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(nvvertparse.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(nvvertprog.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(program.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(program.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(shaderobjects.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(shaderobjects.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(shaderobjects_3dlabs.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(shaderobjects_3dlabs.h, $(MESASRCDIR)/src/mesa/shader)
#endif

  MESA_SHADER_OBJS =	$(MESASHADERBUILDDIR)arbprogparse.o \
			$(MESASHADERBUILDDIR)arbprogram.o \
			$(MESASHADERBUILDDIR)arbfragparse.o \
			$(MESASHADERBUILDDIR)arbvertparse.o \
                        $(MESASHADERBUILDDIR)atifragshader.o \
			$(MESASHADERBUILDDIR)nvprogram.o \
			$(MESASHADERBUILDDIR)nvfragparse.o \
			$(MESASHADERBUILDDIR)nvvertexec.o \
			$(MESASHADERBUILDDIR)nvvertparse.o \
			$(MESASHADERBUILDDIR)program.o \
                        $(MESASHADERBUILDDIR)shaderobjects.o \
                        $(MESASHADERBUILDDIR)shaderobjects_3dlabs.o

#if defined(DoSharedLib) && DoSharedLib
  MESA_SHADER_UOBJS =	$(MESASHADERBUILDDIR)unshared/arbprogparse.o \
			$(MESASHADERBUILDDIR)unshared/arbprogram.o \
			$(MESASHADERBUILDDIR)unshared/arbfragparse.o \
			$(MESASHADERBUILDDIR)unshared/arbvertparse.o \
                        $(MESASHADERBUILDDIR)unshared/atifragshader.o \
			$(MESASHADERBUILDDIR)unshared/nvprogram.o \
			$(MESASHADERBUILDDIR)unshared/nvfragparse.o \
			$(MESASHADERBUILDDIR)unshared/nvvertexec.o \
			$(MESASHADERBUILDDIR)unshared/nvvertparse.o \
			$(MESASHADERBUILDDIR)unshared/program.o \
                        $(MESASHADERBUILDDIR)unshared/shaderobjects.o \
                        $(MESASHADERBUILDDIR)unshared/shaderobjects_3dlabs.o
#else
 MESA_SHADER_UOBJS = $(MESA_SHADER_OBJS)
#endif

  MESA_SHADER_DOBJS =	$(MESASHADERBUILDDIR)debugger/arbprogparse.o \
			$(MESASHADERBUILDDIR)debugger/arbprogram.o \
			$(MESASHADERBUILDDIR)debugger/arbfragparse.o \
			$(MESASHADERBUILDDIR)debugger/arbvertparse.o \
                        $(MESASHADERBUILDDIR)debugger/atifragshader.o \
			$(MESASHADERBUILDDIR)debugger/nvprogram.o \
			$(MESASHADERBUILDDIR)debugger/nvfragparse.o \
			$(MESASHADERBUILDDIR)debugger/nvvertexec.o \
			$(MESASHADERBUILDDIR)debugger/nvvertparse.o \
			$(MESASHADERBUILDDIR)debugger/program.o \
                        $(MESASHADERBUILDDIR)debugger/shaderobjects.o \
                        $(MESASHADERBUILDDIR)debugger/shaderobjects_3dlabs.o

  MESA_SHADER_POBJS =	$(MESASHADERBUILDDIR)profiled/arbprogparse.o \
			$(MESASHADERBUILDDIR)profiled/arbprogram.o \
			$(MESASHADERBUILDDIR)profiled/arbfragparse.o \
			$(MESASHADERBUILDDIR)profiled/arbvertparse.o \
                        $(MESASHADERBUILDDIR)profiled/atifragshader.o \
			$(MESASHADERBUILDDIR)profiled/nvprogram.o \
			$(MESASHADERBUILDDIR)profiled/nvfragparse.o \
			$(MESASHADERBUILDDIR)profiled/nvvertexec.o \
			$(MESASHADERBUILDDIR)profiled/nvvertparse.o \
			$(MESASHADERBUILDDIR)profiled/program.o \
                        $(MESASHADERBUILDDIR)profiled/shaderobjects.o \
                        $(MESASHADERBUILDDIR)profiled/shaderobjects_3dlabs.o
