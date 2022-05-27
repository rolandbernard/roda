#ifndef _ERROR_CONTEXT_H_
#define _ERROR_CONTEXT_H_

#include <stdio.h>
#include <stdbool.h>

#include "errors/msgkind.h"
#include "files/file.h"
#include "text/string.h"

typedef struct {
    MessageCategory category;
    String message;
    Span position;
} MessageFragment;

typedef struct {
    MessageKind kind;
    String message;
    MessageFragment** fragments;
    int fragment_count;
} Message;

void initMessageFragment(MessageFragment* fragment, MessageCategory category, String message, Span position);

MessageFragment* createMessageFragment(MessageCategory category, String message, Span position);

void deinitMessageFragment(MessageFragment* fragment);

void freeMessageFragment(MessageFragment* fragment);

void initMessage(Message* message, MessageKind kind, String string, int fragment_count, ...);

Message* createMessage(MessageKind kind, String string, int fragment_count, ...);

void deinitMessage(Message* message);

void freeMessage(Message* message);

#endif