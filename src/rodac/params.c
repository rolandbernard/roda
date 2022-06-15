
#include <stdint.h>
#include <stdio.h>

#include "rodac/version.h"
#include "text/format.h"
#include "util/params.h"

#include "rodac/params.h"

static PARAM_SPEC_FUNCTION(parameterSpecFunction, CompilerContext*, {
    PARAM_USAGE(PROGRAM_NAME " [options] files...");
    PARAM_FLAG('h', "help", { context->settings.help = true; }, "print this help information and quit");
    PARAM_FLAG(0, "version", { context->settings.version = true; }, "print version information and quit");
    PARAM_VALUED('o', "output", {
        if (context->settings.output_file.data != NULL) {
            PARAM_WARN_MULTIPLE();
        } else {
            context->settings.output_file = createPathFromCString(value);
        }
    }, false, "=<file>", "specify the output file");
    PARAM_VALUED(0, "emit", {
        if (context->settings.emit != COMPILER_EMIT_AUTO) {
            PARAM_WARN_MULTIPLE();
        } else {
            if (strcmp("none", value) == 0) {
                context->settings.emit = COMPILER_EMIT_NONE;
            } else if (strcmp("ast", value) == 0) {
                context->settings.emit = COMPILER_EMIT_AST;
            } else if (strcmp("llvm-ir", value) == 0) {
                context->settings.emit = COMPILER_EMIT_LLVM_IR;
            } else if (strcmp("llvm-bc", value) == 0) {
                context->settings.emit = COMPILER_EMIT_LLVM_BC;
            } else if (strcmp("asm", value) == 0) {
                context->settings.emit = COMPILER_EMIT_ASM;
            } else if (strcmp("obj", value) == 0) {
                context->settings.emit = COMPILER_EMIT_OBJ;
            } else if (strcmp("bin", value) == 0) {
                context->settings.emit = COMPILER_EMIT_BIN;
            } else {
                PARAM_WARN_UNKNOWN_VALUE()
            }
        }
    }, false, "={none|ast|llvm-ir|llvm-bc|asm|obj|bin}", "select what the compiler should emit");
    PARAM_STRING_LIST('l', "libs", {
        addStringToList(&context->settings.libs, copyFromCString(value));
    }, "=<lib>[,...]", "add libraries to be passed to the linker");
    PARAM_STRING_LIST('L', "lib-dirs", {
        addStringToList(&context->settings.lib_dirs, copyFromCString(value));
    }, "=<dir>[,...]", "add library paths to be passed to the linker");
    PARAM_FLAG(0, "static", {
        if (context->settings.link_type == COMPILER_LINK_SHARED) {
            PARAM_WARN_CONFLICT("--shared")
        } else {
            context->settings.link_type = COMPILER_LINK_STATIC; 
        }
    }, "tell the linker to link libraries statically");
    PARAM_FLAG(0, "shared", {
        if (context->settings.link_type == COMPILER_LINK_STATIC) {
            PARAM_WARN_CONFLICT("--static")
        } else {
            context->settings.link_type = COMPILER_LINK_SHARED; 
        }
    }, "tell the linker to link libraries dynamically");
    PARAM_FLAG(0, "pie", {
        if (context->settings.pie == COMPILER_PIE_NO) {
            PARAM_WARN_CONFLICT("--no-pie")
        } else {
            context->settings.pie = COMPILER_PIE_YES; 
        }
    }, "generate a position independent executable");
    PARAM_FLAG(0, "no-pie", {
        if (context->settings.pie == COMPILER_PIE_YES) {
            PARAM_WARN_CONFLICT("--pie")
        } else {
            context->settings.pie = COMPILER_PIE_NO; 
        }
    }, "don't generate a position independent code executable");
    PARAM_VALUED(0, "linker", {
        if (context->settings.linker.data != NULL) {
            PARAM_WARN_MULTIPLE();
        } else {
            context->settings.linker = copyFromCString(value); 
        }
    }, false, "=<linker>", "specify the linker to use");
    PARAM_VALUED(0, "entry", {
        if (context->settings.entry.data != NULL) {
            PARAM_WARN_MULTIPLE();
        } else {
            context->settings.entry = copyFromCString(value); 
        }
    }, false, "=<symbol>", "specify the program entry to be passed to the linker");
    PARAM_FLAG(0, "no-default-libs", { context->settings.defaultlibs = false; }, "don't link with default libraries");
    PARAM_FLAG(0, "no-start-files", { context->settings.startfiles = false; }, "don't link with default startup files");
    PARAM_FLAG(0, "export-dynamic", { context->settings.export_dynamic = true; }, "tell the linker to keep all symbols");
    PARAM_FLAG('g', "debug", { context->settings.emit_debug = true; }, "include debug information in the output");
    PARAM_VALUED('O', NULL, {
        if (context->settings.opt_level != COMPILER_OPT_DEFAULT) {
            PARAM_WARN_MULTIPLE();
        } else {
            if (value == NULL) {
                context->settings.opt_level = COMPILER_OPT_FAST;
            } else if (strlen(value) != 1) {
               PARAM_WARN_UNKNOWN_VALUE()
            } else if (value[0] == '0') {
                context->settings.opt_level = COMPILER_OPT_NONE;
            } else if (value[0] == '1') {
                context->settings.opt_level = COMPILER_OPT_SOME;
            } else if (value[0] == '2') {
                context->settings.opt_level = COMPILER_OPT_FAST;
            } else if (value[0] == '3') {
                context->settings.opt_level = COMPILER_OPT_FASTER;
            } else if (value[0] == 's') {
                context->settings.opt_level = COMPILER_OPT_SMALL;
            } else if (value[0] == 'z') {
                context->settings.opt_level = COMPILER_OPT_SMALLER;
            } else {
               PARAM_WARN_UNKNOWN_VALUE()
            }
        }
    }, true, "{0|1|2|3|s|z}", "configure the level of optimization to apply");
    PARAM_VALUED(0, "target", {
        if (context->settings.target.data != NULL) {
            PARAM_WARN_MULTIPLE();
        } else {
            context->settings.target = copyFromCString(value);
        }
    }, false, "={<triple>|native}", "specify the compilation target triple");
    PARAM_VALUED(0, "cpu", {
        if (context->settings.cpu.data != NULL) {
            PARAM_WARN_MULTIPLE();
        } else {
            context->settings.cpu = copyFromCString(value);
        }
    }, false, "={<name>|native}", "specify the compilation target cpu name");
    PARAM_VALUED(0, "features", {
        if (context->settings.features.data != NULL) {
            PARAM_WARN_MULTIPLE();
        } else {
            context->settings.features = copyFromCString(value);
        }
    }, false, "={<features>|native}", "specify the compilation target cpu features");
    PARAM_INTEGER(0, "max-errors", {
        if (context->msgfilter.max_errors != SIZE_MAX) {
            PARAM_WARN_MULTIPLE();
        } else {
            context->msgfilter.max_errors = value;
        }
    }, "=<value>", "limit the maximum number of errors to show");
    PARAM_VALUED(0, "messages", {
        if (context->settings.message_style != COMPILER_MSG_DEFAULT) {
            PARAM_WARN_MULTIPLE();
        } else {
            if (strcmp("minimal", value) == 0) {
                context->settings.message_style = COMPILER_MSG_MINIMAL;
            } else if (strcmp("less-nosource", value) == 0) {
                context->settings.message_style = COMPILER_MSG_LESS_NO_SOURCE;
            } else if (strcmp("less", value) == 0) {
                context->settings.message_style = COMPILER_MSG_LESS;
            } else if (strcmp("nosource", value) == 0) {
                context->settings.message_style = COMPILER_MSG_NO_SOURCE;
            } else if (strcmp("all", value) == 0) {
                context->settings.message_style = COMPILER_MSG_ALL;
            } else {
                PARAM_WARN_UNKNOWN_VALUE()
            }
        }
    }, false, "={minimal|less-nosource|less|nosource|all}", "select how error messages should be printed");
#ifdef DEBUG
    PARAM_STRING_LIST(0, "compiler-debug", {
        if (strcmp("all", value) == 0) {
            context->settings.compiler_debug = ~0;
        } else if (strcmp("log", value) == 0) {
            context->settings.compiler_debug |= COMPILER_DEBUG_LOG;
        } else if (strcmp("parse-ast", value) == 0) {
            context->settings.compiler_debug |= COMPILER_DEBUG_PARSE_AST;
        } else if (strcmp("typed-ast", value) == 0) {
            context->settings.compiler_debug |= COMPILER_DEBUG_TYPED_AST;
        } else {
            PARAM_WARN_UNKNOWN_VALUE()
        }
    }, "={all|log|parse-ast|typed-ast}[,...]", "print debug information while compiling");
#endif
    PARAM_DEFAULT({
        Path path = createPathFromCString(value);
        if (compareStrings(str("o"), getExtention(toConstPath(path))) == 0) {
            addStringToList(&context->settings.objects, path);
        } else {
            createFileInSet(&context->files, toConstPath(path));
            freePath(path);
        }
    });
    PARAM_WARNING({
        addMessageToContext(&context->msgs, createMessage(
            WARNING_CMD_ARGS, createFormattedString("%s: %s", option, warning), 0
        ));
    });
})

void printHelpText() {
    PARAM_PRINT_HELP(parameterSpecFunction, NULL);
}

int parseProgramParams(int argc, const char* const* argv, CompilerContext* context) {
    return PARAM_PARSE_ARGS(parameterSpecFunction, argc, argv, context);
}

