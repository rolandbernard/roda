
#include "errors/msgkind.h"

#include "util/alloc.h"

static const char* message_kind_strings[] = {
    [ERROR_UNKNOWN] = "unknown",
    [ERROR_SYNTAX] = "syntax",
    [ERROR_ALREADY_DEFINED] = "already-defined",
    [WARNING_UNKNOWN] = "unknown",
    [NOTE_UNKNOWN] = "unknown",
    [HELP_UNKNOWN] = "unknown",
    [DEBUG_UNKNOWN] = "unknown",
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
    [MESSAGE_DEBUG] = "debug",
};

ConstString getMessageCategoryName(MessageCategory category) {
    if (category >= MESSAGE_UNKNOWN && category <= MESSAGE_DEBUG) {
        return createFromConstCString(message_category_names[category]);
    } else {
        return createFromConstCString(message_category_names[MESSAGE_UNKNOWN]);
    }
}

MessageCategory getMessageCategoryFromName(ConstString category) {
    for (int c = MESSAGE_UNKNOWN; c <= MESSAGE_DEBUG; c++) {
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
    } else if (kind > DEBUGS_START && kind < DEBUGS_END) {
        return MESSAGE_DEBUG;
    } else {
        return MESSAGE_UNKNOWN;
    }
}

void initMessageFilter(MessageFilter* filter) {
    // Setup default filter
    filter->message_category_filter[MESSAGE_UNKNOWN] = true;
    filter->message_category_filter[MESSAGE_ERROR] = true;
    filter->message_category_filter[MESSAGE_WARNING] = true;
    filter->message_category_filter[MESSAGE_NOTE] = true;
    filter->message_category_filter[MESSAGE_HELP] = true;
    filter->message_category_filter[MESSAGE_DEBUG] = false;
    for (int i = 0; i < NUM_MESSAGE_KIND; i++) {
        if (getMessageCategory(i) == MESSAGE_ERROR) {
            filter->message_kind_filter[i] = true;
        } else {
            filter->message_kind_filter[i] = false;
        }
    }
}

MessageFilter* createMessageFilter() {
    MessageFilter* ret = ALLOC(MessageFilter, 1);
    initMessageFilter(ret);
    return ret;
}

void freeMessageFilter(MessageFilter* filter) {
    FREE(filter);
}

bool applyFilterForKind(const MessageFilter* filter, MessageKind kind) {
    if (filter == NULL) {
        return true;
    } else {
        MessageCategory category = getMessageCategory(kind);
        if (category == MESSAGE_UNKNOWN) {
            return applyFilterForCategory(filter, MESSAGE_UNKNOWN);
        } else {
            return applyFilterForCategory(filter, category) && filter->message_kind_filter[kind];
        }
    }
}

bool applyFilterForCategory(const MessageFilter* filter, MessageCategory category) {
    if (filter == NULL) {
        return true;
    } else {
        if (category >= MESSAGE_UNKNOWN && category <= MESSAGE_DEBUG) {
            return filter->message_category_filter[category];
        } else {
            return filter->message_category_filter[MESSAGE_UNKNOWN];
        }
    }
}
