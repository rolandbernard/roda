
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
#define MIN_NUMWIDTH 3

static const char* message_category_style[] = {
    [MESSAGE_UNKNOWN] = CONSOLE_SGR(CONSOLE_SGR_BOLD),
    [MESSAGE_ERROR] = CONSOLE_SGR(CONSOLE_SGR_BOLD;CONSOLE_SGR_FG_BRIGHT_RED),
    [MESSAGE_WARNING] = CONSOLE_SGR(CONSOLE_SGR_BOLD;CONSOLE_SGR_FG_BRIGHT_YELLOW),
    [MESSAGE_NOTE] = CONSOLE_SGR(CONSOLE_SGR_BOLD;CONSOLE_SGR_FG_BRIGHT_BLUE),
    [MESSAGE_HELP] = CONSOLE_SGR(CONSOLE_SGR_BOLD;CONSOLE_SGR_FG_BRIGHT_CYAN),
};

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

static Span findMainPosition(MessageFragment** fragments, size_t counts) {
    size_t main_frag = 0;
    for (size_t i = 0; i < counts; i++) {
        if (fragments[i]->category < fragments[main_frag]->category) {
            main_frag = i;
        }
    }
    return fragments[main_frag]->position;
}

static void printFileName(FILE* output, Span location, bool range) {
    if (location.file != NULL) {
        fprintf(output, "%s:", cstr(location.file->original_path));
    }
    if (location.begin.offset != NO_POS) {
        fprintf(output, "%zi:%zi", location.begin.line + 1, location.begin.column + 1);
        if (range && location.begin.offset + 1 < location.end.offset) {
            fprintf(output, "-%zi:%zi", location.end.line + 1, location.end.column);
        }
    }
}

static void printFragmentWithoutSource(FILE* output, MessageFragment* fragment, int numwidth, bool color) {
    for (int i = 0; i < numwidth; i++) {
        fputc(' ', output);
    }
    if (color) {
        fputs(CONSOLE_SGR(CONSOLE_SGR_BOLD), output);
    }
    fputs("  = ", output);
    if (isSpanValid(fragment->position)) {
        printFileName(output, fragment->position, true);
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

#define BUFFER_SIZE 512

static size_t findStartOfLine(FILE* file, size_t offset, size_t lines_back) {
    char buffer[BUFFER_SIZE];
    while (offset >= BUFFER_SIZE) {
        offset -= BUFFER_SIZE;
        fseek(file, offset, SEEK_SET);
        fread(buffer, 1, BUFFER_SIZE, file);
        for (size_t i = 1; i <= BUFFER_SIZE; i++) {
            if (buffer[BUFFER_SIZE - i] == '\n') {
                if (lines_back == 0) {
                    return offset + BUFFER_SIZE - i + 1;
                }
                lines_back--;
            }
        }
    }
    fseek(file, 0, SEEK_SET);
    fread(buffer, 1, offset, file);
    for (size_t i = 1; i <= offset; i++) {
        if (buffer[offset - i] == '\n') {
            if (lines_back == 0) {
                return offset - i + 1;
            }
            lines_back--;
        }
    }
    return 0;
}

static void positionToStartOfLine(FILE* file, size_t offset, size_t lines_back) {
    fseek(file, findStartOfLine(file, offset, lines_back), SEEK_SET);
}

static void printFileNameLine(FILE* output, Span location, int numwidth, bool color) {
    for (int i = 0; i < numwidth; i++) {
        fputc(' ', output);
    }
    if (color) {
        fputs(CONSOLE_SGR(CONSOLE_SGR_BOLD), output);
    }
    fputs(" --> ", output);
    if (color) {
        fputs(CONSOLE_SGR(), output);
    }
    printFileName(output, location, false);
    fputc('\n', output);
}

static void printSourceLines(FILE* output, FILE* file, size_t line, size_t linecount, int numwidth, bool color) {
    for (size_t i = 0; i < linecount; i++) {
        if (color) {
            fputs(CONSOLE_SGR(CONSOLE_SGR_BOLD), output);
        }
        fprintf(output, " %*zi", numwidth, line);
        fputs(" | ", output);
        if (color) {
            fputs(CONSOLE_SGR(), output);
        }
        int next_char = getc(file);
        while (next_char != '\n' && next_char != EOF) {
            if (next_char == '\t') {
                next_char = ' ';
            }
            putc(next_char, output);
            next_char = getc(file);
        }
        fputc('\n', output);
        line++;
    }
}

static void printFragmentsInSameFile(FILE* output, MessageFragment** start_order, size_t frag_count, int numwidth, bool color) {
    FILE* file = openFileStream(start_order[0]->position.file, "rb+");
    if (file != NULL) {
        MessageFragment* end_order[frag_count];
        memcpy(end_order, start_order, frag_count * sizeof(MessageFragment*));
        sortFragmentByEnd(end_order, frag_count);
        printFileNameLine(output, findMainPosition(start_order, frag_count), numwidth, color);
        positionToStartOfLine(file, start_order[0]->position.begin.offset, NUM_SURROUNDING_LINES);
        printSourceLines(output, file, start_order[0]->position.begin.line - NUM_SURROUNDING_LINES + 1, NUM_SURROUNDING_LINES, numwidth, color);
        size_t last_line = start_order[0]->position.begin.line - 1;
        size_t start_pos = 0;
        size_t end_pos = 0;
        while (end_pos < frag_count) {
            Location current_loc = start_order[start_pos]->position.begin;
            if (end_order[end_pos]->position.end.offset < current_loc.offset) {
                current_loc = end_order[end_pos]->position.end;
            }
            /* fprintf(stderr, "%zi %zi\n", last_line, current_loc.line); */
            if (last_line + MAX_INTERMEDIATE_LINES + 1 >= current_loc.line) {
                printSourceLines(output, file, last_line + 2, current_loc.line - last_line, numwidth, color);
            } else {
                printSourceLines(output, file, last_line + 2, NUM_SURROUNDING_LINES, numwidth, color);
                for (int i = 0; i < numwidth; i++) {
                    fputc(' ', output);
                }
                if (color) {
                    fputs(CONSOLE_SGR(CONSOLE_SGR_BOLD), output);
                }
                fputs(" ...\n", output);
                if (color) {
                    fputs(CONSOLE_SGR(), output);
                }
                positionToStartOfLine(file, current_loc.offset, NUM_SURROUNDING_LINES);
                printSourceLines(output, file, current_loc.line - NUM_SURROUNDING_LINES + 1, 1 + NUM_SURROUNDING_LINES, numwidth, color);
                last_line = current_loc.line;
            }
            size_t start_count = 0;
            while (start_pos + start_count < frag_count && start_order[start_pos + start_count]->position.begin.line == current_loc.line) {
                start_count++;
            }
            size_t end_count = 0;
            while (end_pos + end_count < frag_count && start_order[end_pos + end_count]->position.begin.line == current_loc.line) {
                end_count++;
            }
            start_pos += start_count;
            end_pos += end_count;
            last_line = current_loc.line;
        }
        printSourceLines(output, file, last_line + 2, NUM_SURROUNDING_LINES, numwidth, color);
    } else {
        for (size_t i = 0; i < frag_count; i++) {
            printFragmentWithoutSource(output, start_order[i], numwidth, color);
        }
    }
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
        if (print_source && start_order[0]->position.file != NULL) {
            int maxwidth = getDecimalNumberWidth(maxline + 1);
            if (maxwidth < MIN_NUMWIDTH) {
                maxwidth = MIN_NUMWIDTH;
            }
            size_t start = 0;
            while (start < to_print_count && start_order[start]->position.file != NULL) {
                size_t count = 1;
                while (start + count < to_print_count && start_order[start]->position.file == start_order[start + count]->position.file) {
                    count++;
                }
                printFragmentsInSameFile(output, start_order, count, maxwidth, color);
                start += count;
            }
            for (size_t i = start; i < to_print_count; i++) {
                printFragmentWithoutSource(output, start_order[i], maxwidth, color);
            }
        } else {
            for (size_t i = 0; i < to_print_count; i++) {
                printFragmentWithoutSource(output, start_order[i], MIN_NUMWIDTH, color);
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
            Span location = findMainPosition(message->fragments, message->fragment_count);
            if (isSpanValid(location)) {
                fputs(": ", output);
                if (color) {
                    fputs(CONSOLE_SGR() CONSOLE_SGR(CONSOLE_SGR_BOLD), output);
                }
                printFileName(output, location, true);
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
            Span location = findMainPosition(message->fragments, message->fragment_count);
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

