
#include "errors/msgcontext.h"

#include "util/alloc.h"

#define INITIAL_MESSAGE_CAPACITY 32

void initMessageContext(MessageContext* message_context, const MessageFilter* filter) {
    message_context->messages = NULL;
    message_context->message_count = 0;
    message_context->message_capacity = 0;
    message_context->filter = filter;
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
    if (applyFilterForKind(message_context->filter, message->kind)) {
        if (message_context->message_count == message_context->message_capacity) {
            extendMessageContextCapacity(message_context);
        }
        message_context->messages[message_context->message_count] = message;
        message_context->message_count++;
    } else {
        freeMessage(message);
    }
}

void addAllMessagesFromContext(MessageContext* dest_context, MessageContext* src_context) {
    for (size_t i = 0; i < src_context->message_count; i++) {
        addMessageToContext(dest_context, src_context->messages[i]);
    }
}

