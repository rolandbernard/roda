#ifndef _RODA_MESSAGE_PRINT_H_
#define _RODA_MESSAGE_PRINT_H_

#include <stdbool.h>

#include "errors/message.h"
#include "errors/msgcontext.h"

typedef enum {
    MESSAGE_STYLE_MINIMAL,
    MESSAGE_STYLE_LESS_NO_SOURCE,
    MESSAGE_STYLE_LESS,
    MESSAGE_STYLE_NO_SOURCE,
    MESSAGE_STYLE_ALL,
} MessageStyle;

void printMessage(const Message* error, FILE* output, const MessageFilter* filter, MessageStyle style);

void printMessages(const MessageContext* message_context, FILE* output, MessageStyle style);

#endif
