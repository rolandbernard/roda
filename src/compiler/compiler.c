
#include "ast/astprinter.h"
#include "compiler/typecheck.h"
#include "compiler/controlflow.h"
#include "compiler/varbuild.h"
#include "errors/msgprint.h"
#include "parser/wrapper.h"
#include "codegen/codegen.h"

#include "compiler/compiler.h"

void runCompilation(CompilerContext* context) {
    for (size_t i = 0; i < context->files.file_count; i++) {
        File* file = context->files.files[i];
        file->ast = parseFile(file, context);
#ifdef DEBUG
        if (context->settings.compiler_debug & COMPILER_DEBUG_PARSE_AST) {
            printAst(stderr, file->ast);
        }
#endif
        printAndClearMessages(&context->msgs, stderr, true, true);
    }
    if (context->msgs.error_count == 0) {
        runControlFlowReferenceResolution(context);
        runSymbolResolution(context);
        printAndClearMessages(&context->msgs, stderr, true, true);
    }
    if (context->msgs.error_count == 0) {
        runTypeChecking(context);
#ifdef DEBUG
        if (context->settings.compiler_debug & COMPILER_DEBUG_TYPED_AST) {
            for (size_t i = 0; i < context->files.file_count; i++) {
                File* file = context->files.files[i];
                printAst(stderr, file->ast);
            }
        }
#endif
        printAndClearMessages(&context->msgs, stderr, true, true);
    }
    if (context->msgs.error_count == 0) {
        runControlFlowChecking(context);
        printAndClearMessages(&context->msgs, stderr, true, true);
    }
    if (context->msgs.error_count == 0) {
        runCodeGeneration(context);
        printAndClearMessages(&context->msgs, stderr, true, true);
    }
}

