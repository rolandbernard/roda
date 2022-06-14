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
    COMPILER_DEBUG_LOG = (1 << 2),
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

typedef enum {
    COMPILER_OPT_DEFAULT,
    COMPILER_OPT_NONE,
    COMPILER_OPT_SOME,
    COMPILER_OPT_FAST,
    COMPILER_OPT_FASTER,
    COMPILER_OPT_SMALL,
    COMPILER_OPT_SMALLER,
} CompilerOptLevel;

typedef struct {
    CompilerDebugFlags compiler_debug;
    bool help;
    bool version;
    CompilerEmit emit;
    Path output_file;
    String target;
    String cpu;
    String features;
    CompilerOptLevel opt_level;
    bool emit_debug;
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

#define FOR_ALL_MODULES(ACTION)                                 \
    for (size_t i = 0; i < context->files.file_count; i++) {    \
        File* file = context->files.files[i];                   \
        if (file->ast != NULL) ACTION                           \
    }

void initCompilerContext(CompilerContext* context);

void deinitCompilerContext(CompilerContext* context);

#endif
