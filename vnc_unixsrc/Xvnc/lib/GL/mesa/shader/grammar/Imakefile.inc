XCOMM $XFree86$

#ifndef MesaGrammarBuildDir
#define MesaGrammarBuildDir $(GLXLIBSRC)/mesa/shader/grammar/
#endif

MESAGRAMMARBUILDDIR=MesaGrammarBuildDir

  MESA_GRAMMAR_SRCS =	$(MESAGRAMMARBUILDDIR)grammar_mesa.c

#ifdef NeedToLinkMesaSrc
LinkSourceFile(grammar.c, $(MESASRCDIR)/src/mesa/shader/grammar)
LinkSourceFile(grammar.h, $(MESASRCDIR)/src/mesa/shader/grammar)
LinkSourceFile(grammar_syn.h, $(MESASRCDIR)/src/mesa/shader/grammar)
LinkSourceFile(grammar_mesa.c, $(MESASRCDIR)/src/mesa/shader/grammar)
LinkSourceFile(grammar_mesa.h, $(MESASRCDIR)/src/mesa/shader/grammar)
#endif

  MESA_GRAMMAR_OBJS =	 $(MESAGRAMMARBUILDDIR)grammar_mesa.o

#if defined(DoSharedLib) && DoSharedLib
 MESA_GRAMMAR_UOBJS = $(MESAGRAMMARBUILDDIR)unshared/grammar_mesa.o
#else
 MESA_GRAMMAR_UOBJS = $(MESA_GRAMMAR_OBJS)
#endif

 MESA_GRAMMAR_DOBJS = $(MESAGRAMMARBUILDDIR)debugger/grammar_mesa.o

 MESA_GRAMMAR_POBJS = $(MESAGRAMMARBUILDDIR)profiled/grammar_mesa.o
