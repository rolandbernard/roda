
#include "util/alloc.h"

#include "compiler/context.h"

static void initStringList(StringList* list) {
    list->strings = NULL;
    list->count = 0;
    list->capacity = 0;
}

static void deinitStringList(StringList* list) {
    for (size_t i = 0; i < list->count; i++) {
        freeString(list->strings[i]);
    }
    FREE(list->strings);
}

#define INITIAL_CAPACITY 8

void addStringToList(StringList* list, String string) {
    if (list->count == list->capacity) {
        list->capacity = list->capacity == 0 ? INITIAL_CAPACITY : 3 * list->capacity / 2;
        list->strings = REALLOC(String*, list->strings, list->capacity);
    }
    list->strings[list->count] = string;
    list->count++;
}

static void addPrimitiveTypes(CompilerContext* context) {
    addSymbolToTable(&context->buildins, (SymbolEntry*)createTypeSymbolWithType(
        getSymbol(&context->syms, str("bool")), NULL, createUnsizedPrimitiveType(&context->types, TYPE_BOOL)
    ));
    addSymbolToTable(&context->buildins, (SymbolEntry*)createTypeSymbolWithType(
        getSymbol(&context->syms, str("i8")), NULL, createSizedPrimitiveType(&context->types, TYPE_INT, 8)
    ));
    addSymbolToTable(&context->buildins, (SymbolEntry*)createTypeSymbolWithType(
        getSymbol(&context->syms, str("i16")), NULL, createSizedPrimitiveType(&context->types, TYPE_INT, 16)
    ));
    addSymbolToTable(&context->buildins, (SymbolEntry*)createTypeSymbolWithType(
        getSymbol(&context->syms, str("i32")), NULL, createSizedPrimitiveType(&context->types, TYPE_INT, 32)
    ));
    addSymbolToTable(&context->buildins, (SymbolEntry*)createTypeSymbolWithType(
        getSymbol(&context->syms, str("i64")), NULL, createSizedPrimitiveType(&context->types, TYPE_INT, 64)
    ));
    addSymbolToTable(&context->buildins, (SymbolEntry*)createTypeSymbolWithType(
        getSymbol(&context->syms, str("u8")), NULL, createSizedPrimitiveType(&context->types, TYPE_UINT, 8)
    ));
    addSymbolToTable(&context->buildins, (SymbolEntry*)createTypeSymbolWithType(
        getSymbol(&context->syms, str("u16")), NULL, createSizedPrimitiveType(&context->types, TYPE_UINT, 16)
    ));
    addSymbolToTable(&context->buildins, (SymbolEntry*)createTypeSymbolWithType(
        getSymbol(&context->syms, str("u32")), NULL, createSizedPrimitiveType(&context->types, TYPE_UINT, 32)
    ));
    addSymbolToTable(&context->buildins, (SymbolEntry*)createTypeSymbolWithType(
        getSymbol(&context->syms, str("u64")), NULL, createSizedPrimitiveType(&context->types, TYPE_UINT, 64)
    ));
    addSymbolToTable(&context->buildins, (SymbolEntry*)createTypeSymbolWithType(
        getSymbol(&context->syms, str("f32")), NULL, createSizedPrimitiveType(&context->types, TYPE_REAL, 32)
    ));
    addSymbolToTable(&context->buildins, (SymbolEntry*)createTypeSymbolWithType(
        getSymbol(&context->syms, str("f64")), NULL, createSizedPrimitiveType(&context->types, TYPE_REAL, 64)
    ));
    addSymbolToTable(&context->buildins, (SymbolEntry*)createTypeSymbolWithType(
        getSymbol(&context->syms, str("int")), NULL, createSizedPrimitiveType(&context->types, TYPE_INT, 64)
    ));
    addSymbolToTable(&context->buildins, (SymbolEntry*)createTypeSymbolWithType(
        getSymbol(&context->syms, str("uint")), NULL, createSizedPrimitiveType(&context->types, TYPE_UINT, 64)
    ));
    // TODO: isize and usize should be different basic types (because they are target dependent)
    addSymbolToTable(&context->buildins, (SymbolEntry*)createTypeSymbolWithType(
        getSymbol(&context->syms, str("isize")), NULL, createSizedPrimitiveType(&context->types, TYPE_INT, 64)
    ));
    addSymbolToTable(&context->buildins, (SymbolEntry*)createTypeSymbolWithType(
        getSymbol(&context->syms, str("usize")), NULL, createSizedPrimitiveType(&context->types, TYPE_UINT, 64)
    ));
}

static void initCompilerSettings(CompilerSettings* settings) {
    settings->compiler_debug = COMPILER_DEBUG_NONE;
    settings->help = false;
    settings->version = false;
    settings->emit = COMPILER_EMIT_AUTO;
    settings->output_file.data = NULL;
    settings->target.data = NULL;
    settings->cpu.data = NULL;
    settings->features.data = NULL;
    settings->opt_level = COMPILER_OPT_DEFAULT;
    settings->emit_debug = false;
    initStringList(&settings->libs);
    initStringList(&settings->lib_dirs);
    settings->link_type = COMPILER_LINK_DEFAULT;
    settings->pic = COMPILER_PIC_DEFAULT;
    settings->linker.data = NULL;
}

static void deinitCompilerSettings(CompilerSettings* settings) {
    freePath(settings->output_file);
    freeString(settings->target);
    freeString(settings->cpu);
    freeString(settings->features);
    deinitStringList(&settings->libs);
    deinitStringList(&settings->lib_dirs);
    freeString(settings->linker);
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
    deinitCompilerSettings(&context->settings);
    deinitFileSet(&context->files);
    deinitMessageContext(&context->msgs);
    deinitSymbolContext(&context->syms);
    deinitTypeContext(&context->types);
    deinitSymbolTable(&context->buildins);
}

