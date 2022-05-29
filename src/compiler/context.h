#ifndef _COMPILER_CONTEXT_H_
#define _COMPILER_CONTEXT_H_

#include "compiler/symboltable.h"
#include "errors/msgcontext.h"
#include "files/fileset.h"
#include "text/symbol.h"

typedef struct {
    FileSet files;
    MessageContext msgs;
    MessageFilter msgfilter;
    SymbolContext syms;
    SymbolTable buildins;
} CompilerContext;

void initCompilerContext(CompilerContext* context);

void deinitCompilerContext(CompilerContext* context);

#endif
