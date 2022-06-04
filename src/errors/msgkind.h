#ifndef _MESSAGE_KIND_H_
#define _MESSAGE_KIND_H_

#include <stdbool.h>

#include "text/string.h"

typedef enum {
    MESSAGE_UNKNOWN,
    MESSAGE_FATAL_ERROR,
    MESSAGE_ERROR,
    MESSAGE_WARNING,
    MESSAGE_NOTE,
    MESSAGE_HELP,
} MessageCategory;

#define NUM_MESSAGE_CATEGORY (MESSAGE_HELP + 1)

typedef enum {
    UNKNOWN_UNKNOWN,

    ERRORS_START,
    ERROR_UNKNOWN,
    ERROR_SYNTAX,
    ERROR_ALREADY_DEFINED,
    ERROR_UNDEFINED,
    ERROR_CANT_OPEN_FILE,
    ERROR_INVALID_STR,
    ERROR_INVALID_INT,
    ERROR_INVALID_REAL,
    ERROR_INCOMPATIBLE_TYPE,
    ERROR_NOT_CONSTEXPR,
    ERROR_INVALID_ARRAY_LENGTH,
    ERRORS_END,

    WARNINGS_START,
    WARNING_UNKNOWN,
    WARNING_CMD_ARGS,
    WARNINGS_END,
    
    NOTES_START,
    NOTE_UNKNOWN,
    NOTES_END,
    
    HELPS_START,
    HELP_UNKNOWN,
    HELPS_END,
} MessageKind;

#define NUM_MESSAGE_KIND (HELPS_END + 1)

ConstString getMessageKindString(MessageKind kind);

ConstString getMessageCategoryName(MessageCategory category);

MessageCategory getMessageCategoryFromName(ConstString category);

MessageCategory getMessageCategory(MessageKind kind);

typedef struct {
    bool message_category_filter[NUM_MESSAGE_CATEGORY];
    bool message_kind_filter[NUM_MESSAGE_KIND];
} MessageFilter;

void initMessageFilter(MessageFilter* filter);

bool applyFilterForKind(const MessageFilter* filter, MessageKind kind);

bool applyFilterForCategory(const MessageFilter* filter, MessageCategory category);

#endif
