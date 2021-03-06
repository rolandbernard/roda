#ifndef _RODA_COMPILER_CONTEXT_H_
#define _RODA_COMPILER_CONTEXT_H_

#include "compiler/symboltable.h"
#include "errors/msgcontext.h"
#include "files/fileset.h"
#include "text/symbol.h"
#include "types/types.h"
#include "util/tmpalloc.h"

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
    String* strings;
    size_t count;
    size_t capacity;
} StringList;

typedef enum {
    COMPILER_LINK_DEFAULT,
    COMPILER_LINK_STATIC,
    COMPILER_LINK_SHARED,
} CompilerLinkType;

typedef enum {
    COMPILER_PIE_DEFAULT,
    COMPILER_PIE_YES,
    COMPILER_PIE_NO,
} CompilerLinkPic;

typedef enum {
    COMPILER_MSG_DEFAULT,
    COMPILER_MSG_MINIMAL,
    COMPILER_MSG_LESS_NO_SOURCE,
    COMPILER_MSG_LESS,
    COMPILER_MSG_NO_SOURCE,
    COMPILER_MSG_ALL,
} CompilerMessageStyle;

typedef enum {
    COMPILER_RUN_DEFAULT,
    COMPILER_RUN_VERSION,
    COMPILER_RUN_HELP,
    COMPILER_RUN_TEST,
} CompilerRunKind;

typedef struct {
    CompilerDebugFlags compiler_debug;
    CompilerMessageStyle message_style;
    CompilerRunKind run_kind;
    CompilerEmit emit;
    Path output_file;
    String target;
    String cpu;
    String features;
    CompilerOptLevel opt_level;
    bool emit_debug;
    StringList libs;
    StringList lib_dirs;
    CompilerLinkType link_type;
    CompilerLinkPic pie;
    String linker;
    bool export_dynamic;
    bool defaultlibs;
    bool startfiles;
    StringList objects;
} CompilerSettings;

typedef struct {
    TempAlloc tmpalloc;
    CompilerSettings settings;
    MessageFilter msgfilter;
    FileSet files;
    MessageContext msgs;
    SymbolContext syms;
    TypeContext types;
    SymbolTable buildins;
} CompilerContext;

#define FOR_ALL_FILES_IN(CXT, ACTION) {     \
    File* file = CXT->files.files;          \
    while (file != NULL) {                  \
        ACTION                              \
        file = file->next;                  \
    }                                       \
}

#define FOR_ALL_MODULES_IN(CXT, ACTION) \
    FOR_ALL_FILES_IN(CXT, { if (file->ast != NULL) ACTION })

#define FOR_ALL_FILES(ACTION) \
    FOR_ALL_FILES_IN(context, ACTION)

#define FOR_ALL_MODULES(ACTION) \
    FOR_ALL_MODULES_IN(context, ACTION)

#define FOR_ALL_MODULES_IF_OK(ACTION) \
    if (context->msgs.error_count == 0) { FOR_ALL_MODULES(ACTION) }

CompilerContext* createCompilerContext();

void freeCompilerContext(CompilerContext* context);

void addStringToList(StringList* list, String string);

void printAndClearMessages(CompilerContext* context, FILE* output);

#endif
