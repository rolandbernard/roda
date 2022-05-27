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
    MESSAGE_DEBUG,
} MessageCategory;

#define NUM_MESSAGE_CATEGORY (MESSAGE_DEBUG + 1)

typedef enum {
    UNKNOWN_UNKNOWN,

    ERRORS_START,
    ERROR_UNKNOWN,
    ERROR_SYNTAX,
    ERRORS_END,

    WARNINGS_START,
    WARNING_UNKNOWN,
    WARNINGS_END,
    
    NOTES_START,
    NOTE_UNKNOWN,
    NOTES_END,
    
    HELPS_START,
    HELP_UNKNOWN,
    HELPS_END,
    
    DEBUGS_START,
    DEBUG_UNKNOWN,
    DEBUGS_END,
} MessageKind;

#define NUM_MESSAGE_KIND (DEBUGS_END + 1)

ConstString getMessageKindString(MessageKind kind);

ConstString getMessageKindName(MessageKind kind);

MessageKind getMessageKindFromName(ConstString kind);

ConstString getMessageCategoryName(MessageCategory category);

MessageCategory getMessageCategoryFromName(ConstString category);

MessageCategory getMessageCategory(MessageKind kind);

typedef struct {
    bool message_category_filter[NUM_MESSAGE_CATEGORY];
    bool message_kind_filter[NUM_MESSAGE_KIND];
} MessageFilter;

void initMessageFilter(MessageFilter* filter);

MessageFilter* createMessageFilter();

void freeMessageFilter(MessageFilter* filter);

bool applyFilterForKind(MessageFilter* filter, MessageKind kind);

bool applyFilterForCategory(MessageFilter* filter, MessageCategory category);

#endif
