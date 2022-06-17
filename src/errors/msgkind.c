
#include "errors/msgkind.h"

#include "util/alloc.h"

static const char* message_kind_strings[] = {
    [ERROR_UNKNOWN] = "unknown",
    [ERROR_SYNTAX] = "syntax",
    [ERROR_ALREADY_DEFINED] = "already-defined",
    [ERROR_UNDEFINED] = "undefined",
    [ERROR_CANT_OPEN_FILE] = "cant-open-file",
    [ERROR_INVALID_STR] = "invalid-str",
    [ERROR_INVALID_INT] = "invalid-int",
    [ERROR_INVALID_REAL] = "invalid-real",
    [ERROR_INCOMPATIBLE_TYPE] = "incompatible-type",
    [ERROR_NOT_CONSTEXPR] = "not-constexpr",
    [ERROR_INVALID_ARRAY_LENGTH] = "invalid-array-length",
    [ERROR_UNINFERRED_TYPE] = "uninferred-type",
    [ERROR_INVALID_TYPE] = "invalid-type",
    [ERROR_NO_OUTPUT_FILE] = "no-output-file",
    [ERROR_NO_COMPILER_BACKEND] = "no-compiler-backed",
    [ERROR_LLVM_BACKEND_ERROR] = "llvm-backend-error",
    [ERROR_LINKER] = "linker",
    [ERROR_UNSIZED_TYPE] = "unsized-type",
    [ERROR_ARGUMENT_COUNT] = "argument-mismatch",
    [ERROR_DUPLICATE_STRUCT_FIELD] = "duplicate-struct-field",
    [ERROR_NO_SUCH_FIELD] = "no-such-filed",
    [WARNING_UNKNOWN] = "unknown",
    [WARNING_CMD_ARGS] = "cmd-line-args",
    [WARNING_LLVM_BACKEND_WARNING] = "llvm-backend-warning",
    [NOTE_UNKNOWN] = "unknown",
    [NOTE_LLVM_BACKEND_REMARK] = "llvm-backend-remark",
    [NOTE_LLVM_BACKEND_NOTE] = "llvm-backend-note",
    [HELP_UNKNOWN] = "unknown",
};

ConstString getMessageKindString(MessageKind kind) {
    if (getMessageCategory(kind) == MESSAGE_UNKNOWN) {
        return getMessageCategoryName(MESSAGE_UNKNOWN);
    } else {
        return createFromConstCString(message_kind_strings[kind]);
    }
}

static const char* message_category_names[] = {
    [MESSAGE_UNKNOWN] = "unknown",
    [MESSAGE_FATAL_ERROR] = "fatal-error",
    [MESSAGE_ERROR] = "error",
    [MESSAGE_WARNING] = "warning",
    [MESSAGE_NOTE] = "note",
    [MESSAGE_HELP] = "help",
};

ConstString getMessageCategoryName(MessageCategory category) {
    if (category >= MESSAGE_UNKNOWN && category <= MESSAGE_HELP) {
        return createFromConstCString(message_category_names[category]);
    } else {
        return createFromConstCString(message_category_names[MESSAGE_UNKNOWN]);
    }
}

MessageCategory getMessageCategoryFromName(ConstString category) {
    for (int c = MESSAGE_UNKNOWN; c <= MESSAGE_HELP; c++) {
        if (compareStrings(getMessageCategoryName(c), category) == 0) {
            return c;
        }
    }
    return MESSAGE_UNKNOWN;
}

MessageCategory getMessageCategory(MessageKind kind) {
    if (kind > ERRORS_START && kind < ERRORS_END) {
        return MESSAGE_ERROR;
    } else if (kind > WARNINGS_START && kind < WARNINGS_END) {
        return MESSAGE_WARNING;
    } else if (kind > NOTES_START && kind < NOTES_END) {
        return MESSAGE_NOTE;
    } else if (kind > HELPS_START && kind < HELPS_END) {
        return MESSAGE_HELP;
    } else {
        return MESSAGE_UNKNOWN;
    }
}

