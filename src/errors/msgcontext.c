
#include <stdint.h>

#include "errors/msgcontext.h"

#include "util/alloc.h"

#define INITIAL_MESSAGE_CAPACITY 32

void initMessageContext(MessageContext* message_context, const MessageFilter* filter) {
    message_context->messages = NULL;
    message_context->message_count = 0;
    message_context->message_capacity = 0;
    message_context->error_count = 0;
    message_context->filter = filter;
}

void clearMessageContext(MessageContext* context) {
    deinitMessageContext(context);
    context->messages = NULL;
    context->message_count = 0;
    context->message_capacity = 0;
}

void deinitMessageContext(MessageContext* message_context) {
    for (size_t i = 0; i < message_context->message_count; i++) {
        freeMessage(message_context->messages[i]);
    }
    FREE(message_context->messages);
}

static void extendMessageContextCapacity(MessageContext* message_context) {
    if (message_context->message_capacity != 0) {
        message_context->message_capacity *= 2;
    } else {
        message_context->message_capacity = INITIAL_MESSAGE_CAPACITY;
    }
    message_context->messages = REALLOC(Message*, message_context->messages, message_context->message_capacity);
}

void addMessageToContext(MessageContext* message_context, Message* message) {
    if (applyFilterForKind(message_context->filter, message->kind) && applyFilterForContext(message_context->filter, message_context)) {
        if (message_context->message_count == message_context->message_capacity) {
            extendMessageContextCapacity(message_context);
        }
        message_context->messages[message_context->message_count] = message;
        message_context->message_count++;
        if (getMessageCategory(message->kind) == MESSAGE_ERROR) {
            message_context->error_count++;
        }
    } else {
        if (getMessageCategory(message->kind) == MESSAGE_ERROR) {
            message_context->error_count++;
        }
        freeMessage(message);
    }
}

void addAllMessagesFromContext(MessageContext* dest_context, MessageContext* src_context) {
    for (size_t i = 0; i < src_context->message_count; i++) {
        addMessageToContext(dest_context, src_context->messages[i]);
    }
}

void initMessageFilter(MessageFilter* filter) {
    // Setup default filter
    filter->max_errors = SIZE_MAX;
    filter->message_category_filter[MESSAGE_UNKNOWN] = true;
    filter->message_category_filter[MESSAGE_ERROR] = true;
    filter->message_category_filter[MESSAGE_WARNING] = true;
    filter->message_category_filter[MESSAGE_NOTE] = true;
    filter->message_category_filter[MESSAGE_HELP] = true;
    for (int i = 0; i < NUM_MESSAGE_KIND; i++) {
        filter->message_kind_filter[i] = true;
    }
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
        if (category >= MESSAGE_UNKNOWN && category <= MESSAGE_HELP) {
            return filter->message_category_filter[category];
        } else {
            return filter->message_category_filter[MESSAGE_UNKNOWN];
        }
    }
}

bool applyFilterForContext(const MessageFilter* filter, const MessageContext* context) {
    return context->error_count < filter->max_errors;
}

