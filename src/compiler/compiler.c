
#include "ast/astprinter.h"
#include "compiler/typecheck.h"
#include "compiler/varbuild.h"
#include "errors/msgprint.h"
#include "parser/wrapper.h"

#include "compiler/compiler.h"

void runCompilation(CompilerContext* context) {
    for (size_t i = 0; i < context->files.file_count; i++) {
        File* file = context->files.files[i];
        file->ast = parseFile(file, context);
        if (context->settings.debug & COMPILER_DEBUG_AST) {
            printAst(stderr, file->ast);
        }
        printAndClearMessages(&context->msgs, stderr, true, true);
    }
    runSymbolResolution(context);
    printAndClearMessages(&context->msgs, stderr, true, true);
    runTypeChecking(context);
    printAndClearMessages(&context->msgs, stderr, true, true);
    // TODO: codegen
}

