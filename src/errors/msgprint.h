#ifndef _MESSAGE_PRINT_H_
#define _MESSAGE_PRINT_H_

#include <stdbool.h>

#include "errors/message.h"
#include "errors/msgcontext.h"
#include "errors/msgkind.h"

void printMessage(Message* error, FILE* output, MessageFilter* filter, bool print_fragments, bool print_source);

void printMessages(MessageContext* message_context, FILE* output, MessageFilter* filter, bool print_fragments, bool print_source);

#endif