
#include <stdarg.h>

#include "errors/message.h"

#include "util/console.h"
#include "util/alloc.h"

void initMessageFragment(MessageFragment* fragment, MessageCategory category, String message, Span position) {
    fragment->category = category;
    fragment->message = message;
    fragment->position = position;
}

MessageFragment* createMessageFragment(MessageCategory category, String message, Span position) {
    MessageFragment* ret = ALLOC(MessageFragment, 1);
    initMessageFragment(ret, category, message, position);
    return ret;
}

void deinitMessageFragment(MessageFragment* fragment) {
    freeString(fragment->message);
}

void freeMessageFragment(MessageFragment* fragment) {
    deinitMessageFragment(fragment);
    FREE(fragment);
}

static void vaInitMessage(Message* message, MessageKind kind, String string, int fragment_count, va_list args) {
    message->kind = kind;
    message->message = string;
    message->fragment_count = fragment_count;
    message->fragments = ALLOC(MessageFragment*, fragment_count);
    for (int i = 0; i < fragment_count; i++) {
        message->fragments[i] = va_arg(args, MessageFragment*);
    }
}

void initMessage(Message* message, MessageKind kind, String string, size_t fragment_count, ...) {
    va_list args;
    va_start(args, fragment_count);
    vaInitMessage(message, kind, string, fragment_count, args);
    va_end(args);
}

Message* createMessage(MessageKind kind, String string, size_t fragment_count, ...) {
    va_list args;
    va_start(args, fragment_count);
    Message* ret = ALLOC(Message, 1);
    vaInitMessage(ret, kind, string, fragment_count, args);
    va_end(args);
    return ret;
}

void deinitMessage(Message* message) {
    for (size_t i = 0; i < message->fragment_count; i++) {
        freeMessageFragment(message->fragments[i]);
    }
    FREE(message->fragments);
    freeString(message->message);
}

void freeMessage(Message* message) {
    deinitMessage(message);
    FREE(message);
}
