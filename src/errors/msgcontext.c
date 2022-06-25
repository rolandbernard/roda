
#include <stdint.h>

#include "errors/msgcontext.h"

#include "util/alloc.h"

void initMessageContext(MessageContext* message_context, const MessageFilter* filter) {
    message_context->messages = NULL;
    message_context->messages_last = NULL;
    message_context->error_count = 0;
    message_context->filter = filter;
}

void clearMessageContext(MessageContext* context) {
    deinitMessageContext(context);
    context->messages = NULL;
    context->messages_last = NULL;
}

void deinitMessageContext(MessageContext* message_context) {
    Message* cur = message_context->messages;
    while (cur != NULL) {
        Message* next = cur->next;
        freeMessage(cur);
        cur = next;
    }
}

void addMessageToContext(MessageContext* message_context, Message* message) {
    bool use_msg = applyFilterForKind(message_context->filter, message->kind)
        && applyFilterForContext(message_context->filter, message_context);
    if (getMessageCategory(message->kind) == MESSAGE_ERROR) {
        message_context->error_count++;
    }
    if (use_msg) {
        if (message_context->messages == NULL) {
            message_context->messages = message;
        } else {
            message_context->messages_last->next = message;
        }
        message_context->messages_last = message;
    } else {
        freeMessage(message);
    }
}

void addAllMessagesFromContext(MessageContext* dest_context, MessageContext* src_context) {
    if (dest_context->messages == NULL) {
        dest_context->messages = src_context->messages;
        dest_context->messages_last = src_context->messages_last;
    } else {
        dest_context->messages_last->next = src_context->messages;
        dest_context->messages_last = src_context->messages_last;
    }
    src_context->messages = NULL;
    src_context->messages_last = NULL;
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

