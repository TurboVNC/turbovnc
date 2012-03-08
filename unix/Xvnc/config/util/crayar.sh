#!/bin/sh
lib=$1
shift
if cray2; then
        bld cr $lib `lorder $* | tsort`
else
        ar clq $lib $*
fi

