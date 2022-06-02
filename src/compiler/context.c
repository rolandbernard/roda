
#include "compiler/context.h"

void initCompilerContext(CompilerContext* context) {
    initFileSet(&context->files);
    initMessageContext(&context->msgs);
    initMessageFilter(&context->msgfilter);
    initSymbolContext(&context->syms);
    initTypeContext(&context->types);
    initSymbolTable(&context->buildins, NULL);
    addSymbolToTable(&context->buildins, (SymbolEntry*)createTypeSymbol(getSymbol(&context->syms, str("i8")), NULL));
    addSymbolToTable(&context->buildins, (SymbolEntry*)createTypeSymbol(getSymbol(&context->syms, str("i16")), NULL));
    addSymbolToTable(&context->buildins, (SymbolEntry*)createTypeSymbol(getSymbol(&context->syms, str("i32")), NULL));
    addSymbolToTable(&context->buildins, (SymbolEntry*)createTypeSymbol(getSymbol(&context->syms, str("i64")), NULL));
    addSymbolToTable(&context->buildins, (SymbolEntry*)createTypeSymbol(getSymbol(&context->syms, str("u8")), NULL));
    addSymbolToTable(&context->buildins, (SymbolEntry*)createTypeSymbol(getSymbol(&context->syms, str("u16")), NULL));
    addSymbolToTable(&context->buildins, (SymbolEntry*)createTypeSymbol(getSymbol(&context->syms, str("u32")), NULL));
    addSymbolToTable(&context->buildins, (SymbolEntry*)createTypeSymbol(getSymbol(&context->syms, str("u64")), NULL));
    addSymbolToTable(&context->buildins, (SymbolEntry*)createTypeSymbol(getSymbol(&context->syms, str("f32")), NULL));
    addSymbolToTable(&context->buildins, (SymbolEntry*)createTypeSymbol(getSymbol(&context->syms, str("f64")), NULL));
    addSymbolToTable(&context->buildins, (SymbolEntry*)createTypeSymbol(getSymbol(&context->syms, str("int")), NULL));
    addSymbolToTable(&context->buildins, (SymbolEntry*)createTypeSymbol(getSymbol(&context->syms, str("uint")), NULL));
    addSymbolToTable(&context->buildins, (SymbolEntry*)createTypeSymbol(getSymbol(&context->syms, str("isize")), NULL));
    addSymbolToTable(&context->buildins, (SymbolEntry*)createTypeSymbol(getSymbol(&context->syms, str("usize")), NULL));
}

void deinitCompilerContext(CompilerContext* context) {
    deinitFileSet(&context->files);
    deinitMessageContext(&context->msgs);
    deinitSymbolContext(&context->syms);
    deinitSymbolTable(&context->buildins);
}

