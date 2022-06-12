
#include "ast/astprinter.h"
#include "compiler/typecheck.h"
#include "compiler/controlflow.h"
#include "compiler/varbuild.h"
#include "errors/msgprint.h"
#include "parser/wrapper.h"
#include "codegen/codegen.h"
#include "util/debug.h"

#include "compiler/compiler.h"

void runCompilation(CompilerContext* context) {
    for (size_t i = 0; i < context->files.file_count; i++) {
        File* file = context->files.files[i];
        file->ast = parseFile(file, context);
        DEBUG_DO(context, COMPILER_DEBUG_PARSE_AST, { printAst(stderr, file->ast); });
        printAndClearMessages(&context->msgs, stderr, true, true);
    }
    DEBUG_LOG(context, "finished parsing all files");
    if (context->msgs.error_count == 0) {
        runControlFlowReferenceResolution(context);
        DEBUG_LOG(context, "finished control flow reference resolution");
        runSymbolResolution(context);
        DEBUG_LOG(context, "finished symbol reference resolution");
        printAndClearMessages(&context->msgs, stderr, true, true);
    }
    if (context->msgs.error_count == 0) {
        runTypeChecking(context);
        DEBUG_DO(context, COMPILER_DEBUG_TYPED_AST, {
            FOR_ALL_MODULES({ printAst(stderr, file->ast); });
        });
        DEBUG_LOG(context, "finished type checking");
        printAndClearMessages(&context->msgs, stderr, true, true);
    }
    if (context->msgs.error_count == 0) {
        runControlFlowChecking(context);
        DEBUG_LOG(context, "finished control flow checking");
        printAndClearMessages(&context->msgs, stderr, true, true);
    }
    if (context->msgs.error_count == 0) {
        runCodeGeneration(context);
        printAndClearMessages(&context->msgs, stderr, true, true);
    }
}

