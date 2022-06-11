#ifndef _MESSAGE_CONTEXT_H_
#define _MESSAGE_CONTEXT_H_

#include "errors/message.h"

typedef struct {
    bool message_category_filter[NUM_MESSAGE_CATEGORY];
    bool message_kind_filter[NUM_MESSAGE_KIND];
    size_t max_errors;
} MessageFilter;

typedef struct {
    Message** messages;
    size_t message_count;
    size_t message_capacity;
    size_t error_count;
    const MessageFilter* filter;
} MessageContext;

void initMessageContext(MessageContext* message_context, const MessageFilter* filter);

void clearMessageContext(MessageContext* context);

void deinitMessageContext(MessageContext* message_context);

void addMessageToContext(MessageContext* message_context, Message* message);

void addAllMessagesFromContext(MessageContext* dest_context, MessageContext* src_context);

void addMessageToContext(MessageContext* message_context, Message* message);

void initMessageFilter(MessageFilter* filter);

bool applyFilterForKind(const MessageFilter* filter, MessageKind kind);

bool applyFilterForCategory(const MessageFilter* filter, MessageCategory category);

bool applyFilterForContext(const MessageFilter* filter, const MessageContext* context);

#endif
