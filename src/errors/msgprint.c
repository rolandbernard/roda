
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
        return b->position.file - a->position.file;
    } else if (isSpanWithPosition(a->position) != isSpanWithPosition(b->position)) {
        return isSpanWithPosition(a->position) ? -1 : 1;
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

static void printFragmentWithoutSource(
    FILE* output, MessageFragment* fragment, int numwidth, bool color
) {
    for (int i = 0; i < numwidth; i++) {
        fputc(' ', output);
    }
    if (color) {
        fputs(CONSOLE_SGR(CONSOLE_SGR_BOLD), output);
    }
    fputs(" = ", output);
    if (isSpanValid(fragment->position)) {
        if (fragment->position.file != NULL) {
            fprintf(output, "%s:", cstr(fragment->position.file->original_path));
        }
        if (fragment->position.begin.offset != NO_POS) {
            fprintf(output, "%zi:%zi", fragment->position.begin.line + 1, fragment->position.begin.column + 1);
            if (fragment->position.begin.offset + 1 < fragment->position.end.offset) {
                fprintf(output, "-%zi:%zi", fragment->position.end.line + 1, fragment->position.end.column);
            }
        }
    }
    if (fragment->message.length != 0) {
        if (isSpanValid(fragment->position)) {
            fputs(": ", output);
        }
        if (color) {
            fputs(CONSOLE_SGR(), output);
            fputs(message_category_style[fragment->category], output);
        }
        fwrite(fragment->message.data, sizeof(char), fragment->message.length, output);
    }
    if (color) {
        fputs(CONSOLE_SGR(), output);
    }
    fputc('\n', output);
}

static void printFragmentsInSameFile(FILE* output, MessageFragment** start_order, size_t frag_count, bool color) {
    MessageFragment* end_order[frag_count];
    memcpy(end_order, start_order, frag_count * sizeof(MessageFragment*));
    sortFragmentByEnd(end_order, frag_count);
}

static void printMessageFragments(
    FILE* output, const MessageFilter* filter, MessageFragment** fragments, size_t frag_count, bool print_source, bool color
) {
    size_t to_print_count = 0;
    for (size_t i = 0; i < frag_count; i++) {
        if (applyFilterForCategory(filter, fragments[i]->category)) {
            to_print_count++;
        }
    }
    if (to_print_count != 0) {
        MessageFragment* start_order[to_print_count];
        size_t maxline = 0;
        to_print_count = 0;
        for (size_t i = 0; i < frag_count; i++) {
            if (applyFilterForCategory(filter, fragments[i]->category)) {
                start_order[to_print_count] = fragments[i];
                if (isSpanWithPosition(fragments[i]->position)) {
                    if (fragments[i]->position.begin.line > maxline) {
                        maxline = fragments[i]->position.begin.line;
                    }
                    if (fragments[i]->position.end.line > maxline) {
                        maxline = fragments[i]->position.end.line;
                    }
                }
                to_print_count++;
            }
        }
        sortFragmentsByStart(start_order, to_print_count);
        if (print_source) {
            int maxwidth = getDecimalNumberWidth(maxline + 1);
            size_t start = 0;
            while (start < to_print_count && start_order[start]->position.file != NULL) {
                size_t count = 1;
                while (start + count < to_print_count && start_order[start]->position.file == start_order[start + count]->position.file) {
                    count++;
                }
                printFragmentsInSameFile(output, start_order, count, color);
                start += count;
            }
            for (size_t i = start; i < to_print_count; i++) {
                printFragmentWithoutSource(output, start_order[i], start == 0 ? 0 : maxwidth + 1, color);
            }
        } else {
            for (size_t i = 0; i < to_print_count; i++) {
                printFragmentWithoutSource(output, start_order[i], 0, color);
            }
        }
    }
}

void printMessage(const Message* message, FILE* output, const MessageFilter* filter, bool print_fragments, bool print_source) {
    if (applyFilterForKind(filter, message->kind)) {
        bool color = isATerminal(output);
        MessageCategory category = getMessageCategory(message->kind);
        if (color) {
            fputs(message_category_style[category], output);
        }
        fputs(toCString(getMessageCategoryName(category)), output);
        if (category == MESSAGE_ERROR && message->kind != ERROR_UNKNOWN) {
            fprintf(output, "[E%.4i]", message->kind - ERRORS_START - 1);
        } else if (category == MESSAGE_WARNING && message->kind != WARNING_UNKNOWN) {
            fprintf(output, "[W%.4i]", message->kind - WARNINGS_START - 1);
        }
        if (!print_fragments && message->fragment_count != 0) {
            Span location = message->fragments[0]->position;
            if (isSpanValid(location)) {
                fputs(": ", output);
                if (color) {
                    fputs(CONSOLE_SGR() CONSOLE_SGR(CONSOLE_SGR_BOLD), output);
                }
                if (location.file != NULL) {
                    fprintf(output, "%s:", cstr(location.file->original_path));
                }
                if (location.begin.offset != NO_POS) {
                    fprintf(output, "%zi:%zi", location.begin.line + 1, location.begin.column + 1);
                    if (location.begin.offset + 1 < location.end.offset) {
                        fprintf(output, "-%zi:%zi", location.end.line + 1, location.end.column);
                    }
                }
            }
        }
        if (message->message.length != 0) {
            fputs(": ", output);
            if (color) {
                fputs(CONSOLE_SGR() CONSOLE_SGR(CONSOLE_SGR_BOLD), output);
            }
            fwrite(message->message.data, sizeof(char), message->message.length, output);
        }
        if (color) {
            fputs(CONSOLE_SGR(), output);
        }
        fputc('\n', output);
        if (print_fragments) {
            printMessageFragments(output, filter, message->fragments, message->fragment_count, print_source, color);
        } else if (print_source && message->fragment_count != 0) {
            Span location = message->fragments[0]->position;
            if (location.begin.offset != NO_POS) {
                MessageFragment dummy = {
                    .category = category,
                    .message = createString("", 0),
                    .position = location,
                };
                MessageFragment* fragments = &dummy;
                printMessageFragments(output, filter, &fragments, 1, true, color);
            }
        }
    }
}

void printMessages(const MessageContext* message_context, FILE* output, const MessageFilter* filter, bool print_fragments, bool print_source) {
    for (size_t i = 0; i < message_context->message_count; i++) {
        printMessage(message_context->messages[i], output, filter, print_fragments, print_source);
    }
}

