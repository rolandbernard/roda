#ifndef _COMPILER_CONTEXT_H_
#define _COMPILER_CONTEXT_H_

#include "compiler/symboltable.h"
#include "compiler/types.h"
#include "errors/msgcontext.h"
#include "files/fileset.h"
#include "text/symbol.h"

typedef enum {
    COMPILER_DEBUG_NONE = 0,
    COMPILER_DEBUG_AST = (1 << 0),
} CompilerDebugFlags;

typedef struct {
    CompilerDebugFlags debug;
    bool help;
    bool version;
} CompilerSettings;

typedef struct {
    CompilerSettings settings;
    MessageFilter msgfilter;
    FileSet files;
    MessageContext msgs;
    SymbolContext syms;
    TypeContext types;
    SymbolTable buildins;
} CompilerContext;

void initCompilerContext(CompilerContext* context);

void deinitCompilerContext(CompilerContext* context);

#endif
