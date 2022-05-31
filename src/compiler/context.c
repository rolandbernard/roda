
#include "compiler/context.h"

void initCompilerContext(CompilerContext* context) {
    initFileSet(&context->files);
    initMessageContext(&context->msgs);
    initMessageFilter(&context->msgfilter);
    initSymbolContext(&context->syms);
    initTypeContext(&context->types);
    initSymbolTable(&context->buildins, NULL);
    addSymbolToTable(&context->buildins, createVariable(getSymbol(&context->syms, str("i8")), invalidSpan()));
    addSymbolToTable(&context->buildins, createVariable(getSymbol(&context->syms, str("i16")), invalidSpan()));
    addSymbolToTable(&context->buildins, createVariable(getSymbol(&context->syms, str("i32")), invalidSpan()));
    addSymbolToTable(&context->buildins, createVariable(getSymbol(&context->syms, str("i64")), invalidSpan()));
    addSymbolToTable(&context->buildins, createVariable(getSymbol(&context->syms, str("u8")), invalidSpan()));
    addSymbolToTable(&context->buildins, createVariable(getSymbol(&context->syms, str("u16")), invalidSpan()));
    addSymbolToTable(&context->buildins, createVariable(getSymbol(&context->syms, str("u32")), invalidSpan()));
    addSymbolToTable(&context->buildins, createVariable(getSymbol(&context->syms, str("u64")), invalidSpan()));
    addSymbolToTable(&context->buildins, createVariable(getSymbol(&context->syms, str("f32")), invalidSpan()));
    addSymbolToTable(&context->buildins, createVariable(getSymbol(&context->syms, str("f64")), invalidSpan()));
    addSymbolToTable(&context->buildins, createVariable(getSymbol(&context->syms, str("int")), invalidSpan()));
    addSymbolToTable(&context->buildins, createVariable(getSymbol(&context->syms, str("uint")), invalidSpan()));
    addSymbolToTable(&context->buildins, createVariable(getSymbol(&context->syms, str("isize")), invalidSpan()));
    addSymbolToTable(&context->buildins, createVariable(getSymbol(&context->syms, str("usize")), invalidSpan()));
}

void deinitCompilerContext(CompilerContext* context) {
    deinitFileSet(&context->files);
    deinitMessageContext(&context->msgs);
    deinitSymbolContext(&context->syms);
    deinitSymbolTable(&context->buildins);
}

