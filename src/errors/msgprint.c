
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "errors/msgprint.h"

#include "text/string.h"
#include "text/utf8.h"
#include "util/alloc.h"
#include "util/console.h"

#define NUM_SURROUNDING_LINES 1
#define MAX_INTERMEDIATE_LINES 2

static const char* message_category_style[] = {
    [MESSAGE_UNKNOWN] = CONSOLE_SGR(CONSOLE_SGR_BOLD),
    [MESSAGE_ERROR] = CONSOLE_SGR(CONSOLE_SGR_BOLD;CONSOLE_SGR_FG_BRIGHT_RED),
    [MESSAGE_WARNING] = CONSOLE_SGR(CONSOLE_SGR_BOLD;CONSOLE_SGR_FG_BRIGHT_YELLOW),
    [MESSAGE_NOTE] = CONSOLE_SGR(CONSOLE_SGR_BOLD;CONSOLE_SGR_FG_BRIGHT_BLUE),
    [MESSAGE_HELP] = CONSOLE_SGR(CONSOLE_SGR_BOLD;CONSOLE_SGR_FG_BRIGHT_CYAN),
};

typedef struct {
    size_t line;
    size_t column;
    size_t line_offset;
} FilePosition;

static int fragmentCompare(const MessageFragment* a, const MessageFragment* b, bool by_end) {
    if (a->position.file != b->position.file) {
        return a->position.file - b->position.file;
    } else if (isSpanFileOnly(a->position) != isSpanFileOnly(b->position)) {
        return isSpanFileOnly(a->position) ? -1 : 1;
    } else if (!by_end && a->position.end.offset != b->position.begin.offset) {
        return a->position.begin.offset - b->position.begin.offset;
    } else if (by_end && a->position.end.offset != b->position.end.offset) {
        return a->position.end.offset - b->position.end.offset;
    } else if (a->category != b->category) {
        return b->category - a->category;
    } else if (getSpanLength(a->position) != getSpanLength(b->position)) {
        return getSpanLength(a->position) - getSpanLength(b->position);
    } else {
        return 0;
    }
}

// NOTE: Printing the errors is not thread safe
static MessageFragment** fragment_sort_original_order;

static int fragmentCompareByEnd(const void* a, const void* b) {
    return fragmentCompare(*(MessageFragment**)a, *(MessageFragment**)b, true);
}

static void sortFragmentByEnd(MessageFragment** fragments, size_t fragment_count) {
    qsort(fragments, fragment_count, sizeof(size_t), fragmentCompareByEnd);
}

static int fragmentCompareByStart(const void* a, const void* b) {
    return fragmentCompare(*(MessageFragment**)a, *(MessageFragment**)b, false);
}

static void sortFragmentsByStart(MessageFragment** fragments, size_t fragment_count) {
    qsort(fragments, fragment_count, sizeof(MessageFragment*), fragmentCompareByStart);
}

static int getDecimalNumberWidth(size_t n) {
    int ret = 1;
    while (n >= 10) {
        n /= 10;
        ret++;
    }
    return ret;
}


void printMessage(const Message* message, FILE* output, const MessageFilter* filter, bool print_fragments, bool print_source) {
}

void printMessages(const MessageContext* message_context, FILE* output, const MessageFilter* filter, bool print_fragments, bool print_source) {
    for (size_t i = 0; i < message_context->message_count; i++) {
        printMessage(message_context->messages[i], output, filter, print_fragments, print_source);
    }
}

