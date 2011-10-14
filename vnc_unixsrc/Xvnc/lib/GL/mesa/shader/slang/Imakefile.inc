XCOMM $XFree86$

#ifndef MesaSlangBuildDir
#define MesaSlangBuildDir $(GLXLIBSRC)/mesa/shader/slang/
#endif

MESASLANGBUILDDIR=MesaSlangBuildDir

  MESA_SLANG_SRCS =	$(MESASLANGBUILDDIR)slang_assemble_assignment.c \
                        $(MESASLANGBUILDDIR)slang_assemble.c \
                        $(MESASLANGBUILDDIR)slang_assemble_conditional.c \
                        $(MESASLANGBUILDDIR)slang_assemble_constructor.c \
                        $(MESASLANGBUILDDIR)slang_assemble_typeinfo.c \
                        $(MESASLANGBUILDDIR)slang_compile.c \
                        $(MESASLANGBUILDDIR)slang_execute.c \
                        $(MESASLANGBUILDDIR)slang_preprocess.c \
                        $(MESASLANGBUILDDIR)slang_storage.c \
                        $(MESASLANGBUILDDIR)slang_utility.c

#ifdef NeedToLinkMesaSrc
LinkSourceFile(slang_assemble_assignment.c, $(MESASRCDIR)/src/mesa/shader/slang)
LinkSourceFile(slang_assemble_assignment.h, $(MESASRCDIR)/src/mesa/shader/slang)
LinkSourceFile(slang_assemble.c, $(MESASRCDIR)/src/mesa/shader/slang)
LinkSourceFile(slang_assemble.h, $(MESASRCDIR)/src/mesa/shader/slang)
LinkSourceFile(slang_assemble_conditional.c, $(MESASRCDIR)/src/mesa/shader/slang)
LinkSourceFile(slang_assemble_conditional.h, $(MESASRCDIR)/src/mesa/shader/slang)
LinkSourceFile(slang_assemble_constructor.c, $(MESASRCDIR)/src/mesa/shader/slang)
LinkSourceFile(slang_assemble_constructor.h, $(MESASRCDIR)/src/mesa/shader/slang)
LinkSourceFile(slang_assemble_typeinfo.c, $(MESASRCDIR)/src/mesa/shader/slang)
LinkSourceFile(slang_assemble_typeinfo.h, $(MESASRCDIR)/src/mesa/shader/slang)
LinkSourceFile(slang_compile.c, $(MESASRCDIR)/src/mesa/shader/slang)
LinkSourceFile(slang_compile.h, $(MESASRCDIR)/src/mesa/shader/slang)
LinkSourceFile(slang_execute.c, $(MESASRCDIR)/src/mesa/shader/slang)
LinkSourceFile(slang_execute.h, $(MESASRCDIR)/src/mesa/shader/slang)
LinkSourceFile(slang_preprocess.c, $(MESASRCDIR)/src/mesa/shader/slang)
LinkSourceFile(slang_preprocess.h, $(MESASRCDIR)/src/mesa/shader/slang)
LinkSourceFile(slang_storage.c, $(MESASRCDIR)/src/mesa/shader/slang)
LinkSourceFile(slang_storage.h, $(MESASRCDIR)/src/mesa/shader/slang)
LinkSourceFile(slang_utility.c, $(MESASRCDIR)/src/mesa/shader/slang)
LinkSourceFile(slang_utility.h, $(MESASRCDIR)/src/mesa/shader/slang)
LinkSourceFile(traverse_wrap.h, $(MESASRCDIR)/src/mesa/shader/slang)
#endif

  MESA_SLANG_OBJS =	$(MESASLANGBUILDDIR)slang_assemble_assignment.o \
                        $(MESASLANGBUILDDIR)slang_assemble.o \
                        $(MESASLANGBUILDDIR)slang_assemble_conditional.o \
                        $(MESASLANGBUILDDIR)slang_assemble_constructor.o \
                        $(MESASLANGBUILDDIR)slang_assemble_typeinfo.o \
                        $(MESASLANGBUILDDIR)slang_compile.o \
                        $(MESASLANGBUILDDIR)slang_execute.o \
                        $(MESASLANGBUILDDIR)slang_preprocess.o \
                        $(MESASLANGBUILDDIR)slang_storage.o \
                        $(MESASLANGBUILDDIR)slang_utility.o

#if defined(DoSharedLib) && DoSharedLib
 MESA_SLANG_UOBJS = $(MESASLANGBUILDDIR)unshared/slang_assemble_assignment.o \
                    $(MESASLANGBUILDDIR)unshared/slang_assemble.o \
                    $(MESASLANGBUILDDIR)unshared/slang_assemble_conditional.o \
                    $(MESASLANGBUILDDIR)unshared/slang_assemble_constructor.o \
                    $(MESASLANGBUILDDIR)unshared/slang_assemble_typeinfo.o \
                    $(MESASLANGBUILDDIR)unshared/slang_compile.o \
                    $(MESASLANGBUILDDIR)unshared/slang_execute.o \
                    $(MESASLANGBUILDDIR)unshared/slang_preprocess.o \
                    $(MESASLANGBUILDDIR)unshared/slang_storage.o \
                    $(MESASLANGBUILDDIR)unshared/slang_utility.o
#else
 MESA_SLANG_UOBJS = $(MESA_SLANG_OBJS)
#endif

 MESA_SLANG_DOBJS = $(MESASLANGBUILDDIR)debuggerslang_assemble_assignment.o \
                    $(MESASLANGBUILDDIR)debuggerslang_assemble.o \
                    $(MESASLANGBUILDDIR)debuggerslang_assemble_conditional.o \
                    $(MESASLANGBUILDDIR)debuggerslang_assemble_constructor.o \
                    $(MESASLANGBUILDDIR)debuggerslang_assemble_typeinfo.o \
                    $(MESASLANGBUILDDIR)debuggerslang_compile.o \
                    $(MESASLANGBUILDDIR)debuggerslang_execute.o \
                    $(MESASLANGBUILDDIR)debuggerslang_preprocess.o \
                    $(MESASLANGBUILDDIR)debuggerslang_storage.o \
                    $(MESASLANGBUILDDIR)debuggerslang_utility.o

 MESA_SLANG_POBJS = $(MESASLANGBUILDDIR)profiled/slang_assemble_assignment.o \
                    $(MESASLANGBUILDDIR)profiled/slang_assemble.o \
                    $(MESASLANGBUILDDIR)profiled/slang_assemble_conditional.o \
                    $(MESASLANGBUILDDIR)profiled/slang_assemble_constructor.o \
                    $(MESASLANGBUILDDIR)profiled/slang_assemble_typeinfo.o \
                    $(MESASLANGBUILDDIR)profiled/slang_compile.o \
                    $(MESASLANGBUILDDIR)profiled/slang_execute.o \
                    $(MESASLANGBUILDDIR)profiled/slang_preprocess.o \
                    $(MESASLANGBUILDDIR)profiled/slang_storage.o \
                    $(MESASLANGBUILDDIR)profiled/slang_utility.o
