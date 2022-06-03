
#include "compiler/context.h"

static void addPrimitiveTypes(CompilerContext* context) {
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

static void initCompilerSettings(CompilerSettings* settings) {
    settings->debug = false;
    settings->help = false;
    settings->version = false;
}

void initCompilerContext(CompilerContext* context) {
    initCompilerSettings(&context->settings);
    initMessageFilter(&context->msgfilter);
    initFileSet(&context->files);
    initMessageContext(&context->msgs, &context->msgfilter);
    initSymbolContext(&context->syms);
    initTypeContext(&context->types);
    initSymbolTable(&context->buildins, NULL);
    addPrimitiveTypes(context);
}

void deinitCompilerContext(CompilerContext* context) {
    deinitFileSet(&context->files);
    deinitMessageContext(&context->msgs);
    deinitSymbolContext(&context->syms);
    deinitSymbolTable(&context->buildins);
}

