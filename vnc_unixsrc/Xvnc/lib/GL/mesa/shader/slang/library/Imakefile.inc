XCOMM $XFree86:$

#ifdef NeedToLinkMesaSrc
LinkSourceFile(slang_common_builtin_gc_bin.h, $(MESASRCDIR)/src/mesa/shader/slang/library)
LinkSourceFile(slang_common_builtin_gc.h, $(MESASRCDIR)/src/mesa/shader/slang/library)
LinkSourceFile(slang_core_gc_bin.h, $(MESASRCDIR)/src/mesa/shader/slang/library)
LinkSourceFile(slang_core_gc.h, $(MESASRCDIR)/src/mesa/shader/slang/library)
LinkSourceFile(slang_fragment_builtin_gc_bin.h, $(MESASRCDIR)/src/mesa/shader/slang/library)
LinkSourceFile(slang_fragment_builtin_gc.h, $(MESASRCDIR)/src/mesa/shader/slang/library)
LinkSourceFile(slang_shader_syn.h, $(MESASRCDIR)/src/mesa/shader/slang/library)
LinkSourceFile(slang_version_syn.h, $(MESASRCDIR)/src/mesa/shader/slang/library)
LinkSourceFile(slang_vertex_builtin_gc_bin.h, $(MESASRCDIR)/src/mesa/shader/slang/library)
LinkSourceFile(slang_vertex_builtin_gc.h, $(MESASRCDIR)/src/mesa/shader/slang/library)
#endif
