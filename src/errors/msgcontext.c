
#include <stdint.h>

#include "errors/msgcontext.h"

#include "util/alloc.h"

void initMessageContext(MessageContext* message_context, const MessageFilter* filter) {
    message_context->messages = NULL;
    message_context->error_count = 0;
    message_context->filter = filter;
}

void clearMessageContext(MessageContext* context) {
    deinitMessageContext(context);
    context->messages = NULL;
}

void deinitMessageContext(MessageContext* message_context) {
    Message* cur = message_context->messages;
    while (cur != NULL) {
        Message* next = cur->next;
        freeMessage(cur);
        cur = next;
    }
    FREE(message_context->messages);
}

void addMessageToContext(MessageContext* message_context, Message* message) {
    if (getMessageCategory(message->kind) == MESSAGE_ERROR) {
        message_context->error_count++;
    }
    if (applyFilterForKind(message_context->filter, message->kind) && applyFilterForContext(message_context->filter, message_context)) {
        message->next = message_context->messages;
        message_context->messages = message->next;
    } else {
        freeMessage(message);
    }
}

void addAllMessagesFromContext(MessageContext* dest_context, MessageContext* src_context) {
    Message* cur = src_context->messages;
    while (cur != NULL) {
        Message* next = cur->next;
        addMessageToContext(dest_context, cur);
        cur = next;
    }
    src_context->messages = NULL;
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
    filter->message_kind_filter[WARNING_LLVM_BACKEND_WARNING] = false;
    filter->message_kind_filter[NOTE_LLVM_BACKEND_REMARK] = false;
    filter->message_kind_filter[NOTE_LLVM_BACKEND_NOTE] = false;
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

