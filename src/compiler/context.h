#ifndef _COMPILER_CONTEXT_H_
#define _COMPILER_CONTEXT_H_

#include "compiler/symboltable.h"
#include "compiler/types.h"
#include "errors/msgcontext.h"
#include "files/fileset.h"
#include "text/symbol.h"

typedef enum {
    COMPILER_DEBUG_NONE = 0,
    COMPILER_DEBUG_PARSE_AST = (1 << 0),
    COMPILER_DEBUG_TYPED_AST = (1 << 1),
} CompilerDebugFlags;

typedef enum {
    COMPILER_EMIT_AUTO,
    COMPILER_EMIT_NONE,
    COMPILER_EMIT_AST,
    COMPILER_EMIT_LLVM_IR,
    COMPILER_EMIT_LLVM_BC,
    COMPILER_EMIT_ASM,
    COMPILER_EMIT_OBJ,
    COMPILER_EMIT_BIN,
} CompilerEmit;

typedef struct {
    CompilerDebugFlags debug;
    bool help;
    bool version;
    CompilerEmit emit;
    Path output_file;
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
