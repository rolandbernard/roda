
#include "ast/astprinter.h"
#include "codegen/codegen.h"
#include "compiler/controlflow.h"
#include "compiler/varbuild.h"
#include "const/defeval.h"
#include "parser/wrapper.h"
#include "types/check.h"
#include "types/infer.h"
#include "util/debug.h"

#include "compiler/compiler.h"

void runCompilation(CompilerContext* context) {
    FOR_ALL_FILES({
        if (file->type == FILE_RODA) {
            file->ast = parseFile(file, context);
            DEBUG_DO(context, COMPILER_DEBUG_PARSE_AST, { printAst(stderr, file->ast); });
            printAndClearMessages(context, stderr);
        }
    })
    DEBUG_LOG(context, "finished parsing all files");
    if (context->msgs.error_count == 0) {
        runControlFlowReferenceResolution(context);
        DEBUG_LOG(context, "finished control flow reference resolution");
        runSymbolResolution(context);
        DEBUG_LOG(context, "finished symbol reference resolution");
        printAndClearMessages(context, stderr);
    }
    if (context->msgs.error_count == 0) {
        runConstantValueEvaluation(context);
        DEBUG_LOG(context, "finished constant value evaluation");
    }
    if (context->msgs.error_count == 0) {
        runTypeInference(context);
        DEBUG_LOG(context, "finished type inference");
        DEBUG_DO(context, COMPILER_DEBUG_TYPED_AST, {
            FOR_ALL_MODULES({ printAst(stderr, file->ast); });
        });
    }
    if (context->msgs.error_count == 0) {
        runTypeChecking(context);
        DEBUG_LOG(context, "finished type checking");
        printAndClearMessages(context, stderr);
    }
    if (context->msgs.error_count == 0) {
        runControlFlowChecking(context);
        DEBUG_LOG(context, "finished control flow checking");
        printAndClearMessages(context, stderr);
    }
    if (context->msgs.error_count == 0) {
        runGlobalInitEvaluation(context);
        DEBUG_LOG(context, "finished global initializer evaluation");
    }
    if (context->msgs.error_count == 0) {
        runCodeGeneration(context);
        printAndClearMessages(context, stderr);
    }
}

