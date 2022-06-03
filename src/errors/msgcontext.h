#ifndef _MESSAGE_CONTEXT_H_
#define _MESSAGE_CONTEXT_H_

#include "errors/message.h"

typedef struct {
    Message** messages;
    size_t message_count;
    size_t message_capacity;
    const MessageFilter* filter;
} MessageContext;

void initMessageContext(MessageContext* message_context, const MessageFilter* filter);

void clearMessageContext(MessageContext* context);

void deinitMessageContext(MessageContext* message_context);

void addMessageToContext(MessageContext* message_context, Message* message);

void addAllMessagesFromContext(MessageContext* dest_context, MessageContext* src_context);

void addMessageToContext(MessageContext* message_context, Message* message);

size_t countMessagesInContext(MessageContext* context, MessageCategory max_cat);

#endif
