#ifndef _MESSAGE_CONTEXT_H_
#define _MESSAGE_CONTEXT_H_

#include "errors/message.h"

typedef struct {
    Message** messages;
    int message_count;
    int message_capacity;
} MessageContext;

void initMessageContext(MessageContext* message_context);

void deinitMessageContext(MessageContext* message_context);

void addMessageToContext(MessageContext* message_context, Message* message);

void addAllMessagesFromContext(MessageContext* dest_context, MessageContext* src_context);

void addMessageToContext(MessageContext* message_context, Message* message);

void addFilteredMessageToContext(MessageContext* message_context, Message* message, MessageFilter* filter);

#endif
