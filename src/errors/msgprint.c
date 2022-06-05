
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
#define FST_LAYER_CLEARANCE 3
#define SND_LAYER_CLEARANCE 3

static const char* message_category_style[] = {
    [MESSAGE_UNKNOWN] = CONSOLE_SGR(CONSOLE_SGR_BOLD),
    [MESSAGE_ERROR] = CONSOLE_SGR(CONSOLE_SGR_BOLD;CONSOLE_SGR_FG_BRIGHT_RED),
    [MESSAGE_WARNING] = CONSOLE_SGR(CONSOLE_SGR_BOLD;CONSOLE_SGR_FG_BRIGHT_YELLOW),
    [MESSAGE_NOTE] = CONSOLE_SGR(CONSOLE_SGR_BOLD;CONSOLE_SGR_FG_BRIGHT_BLUE),
    [MESSAGE_HELP] = CONSOLE_SGR(CONSOLE_SGR_BOLD;CONSOLE_SGR_FG_BRIGHT_CYAN),
};

static int fragmentCompare(const MessageFragment* a, const MessageFragment* b, bool by_end) {
    if (a->position.file != b->position.file) {
        return a->position.file - b->position.file;
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
        fwrite(location.file->original_path.data, 1, location.file->original_path.length, output);
    }
    if (location.begin.offset != NO_POS) {
        if (location.file != NULL) {
            fputc(':', output);
        }
        fprintf(output, "%zi:%zi", location.begin.line + 1, location.begin.column + 1);
        if (range && location.begin.offset + 1 < location.end.offset) {
            fprintf(output, "-%zi:%zi", location.end.line + 1, location.end.column);
        }
    }
}

static void printFragmentWithoutSource(FILE* output, MessageFragment* fragment, int numwidth, bool pos, bool color) {
    for (int i = 0; i < numwidth; i++) {
        fputc(' ', output);
    }
    if (color) {
        fputs(CONSOLE_SGR(CONSOLE_SGR_BOLD), output);
    }
    fputs("  = ", output);
    if (pos && isSpanValid(fragment->position)) {
        printFileName(output, fragment->position, true);
    }
    if (fragment->message.length != 0) {
        if (pos && isSpanValid(fragment->position)) {
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
        int next_char = getc(file);
        if (next_char == EOF) {
            break;
        } else {
            if (color) {
                fputs(CONSOLE_SGR(CONSOLE_SGR_BOLD), output);
            }
            fprintf(output, " %*zi", numwidth, line);
            fputs(" | ", output);
            if (color) {
                fputs(CONSOLE_SGR(), output);
            }
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
}

static void printFragmentLineStart(FILE* output, int numwidth, bool color) {
    for (int i = 0; i < numwidth; i++) {
        fputc(' ', output);
    }
    if (color) {
        fputs(CONSOLE_SGR(CONSOLE_SGR_BOLD), output);
    }
    fputs("  |", output);
    if (color) {
        fputs(CONSOLE_SGR(), output);
    }
}

static bool isMultilineSpan(Span span) {
    return span.begin.line != span.end.line;
}

static void printFragmentLineFirstLayer(
    FILE* output, MessageFragment** starts, MessageFragment** ends, size_t start_count, size_t end_count,
    bool* text_printed, int numwidth, bool color
) {
    printFragmentLineStart(output, numwidth, color);
    size_t cur_col = -1;
    size_t num_open[NUM_MESSAGE_CATEGORY];
    memset(num_open, 0, NUM_MESSAGE_CATEGORY * sizeof(size_t));
    size_t start_seen = 0;
    size_t end_seen = 0;
    MessageCategory trail = NUM_MESSAGE_CATEGORY;
    while (start_seen < start_count || end_seen < end_count || trail != NUM_MESSAGE_CATEGORY) {
        MessageCategory category = trail;
        trail = NUM_MESSAGE_CATEGORY;
        size_t mult_ext[NUM_MESSAGE_CATEGORY];
        memset(mult_ext, 0, NUM_MESSAGE_CATEGORY * sizeof(size_t));
        while (start_seen < start_count && starts[start_seen]->position.begin.column == cur_col) {
            if (isMultilineSpan(starts[start_seen]->position)) {
                mult_ext[starts[start_seen]->category]++;
                if (starts[start_seen]->category < trail) {
                    trail = starts[start_seen]->category;
                }
            } else {
                num_open[starts[start_seen]->category]++;
            }
            start_seen++;
        }
        size_t end_start = end_seen;
        while (end_seen < end_count && ends[end_seen]->position.end.column == cur_col) {
            if (!isMultilineSpan(ends[end_seen]->position)) {
                num_open[ends[end_seen]->category]--;
            }
            end_seen++;
        }
        size_t rev_trail = end_seen;
        while (rev_trail < end_count && ends[rev_trail]->position.end.column == cur_col + 1) {
            if (isMultilineSpan(ends[rev_trail]->position)) {
                mult_ext[ends[end_seen]->category]++;
            }
            rev_trail++;
        }
        while (rev_trail < end_count && ends[rev_trail]->position.end.column == cur_col + 2) {
            if (isMultilineSpan(ends[rev_trail]->position)) {
                if (ends[rev_trail]->category < category) {
                    category = ends[rev_trail]->category;
                }
            }
            rev_trail++;
        }
        for (MessageCategory i = 1; i <= NUM_MESSAGE_CATEGORY; i++) {
            if (num_open[NUM_MESSAGE_CATEGORY - i] > 0 || mult_ext[NUM_MESSAGE_CATEGORY - i] > 0) {
                category = NUM_MESSAGE_CATEGORY - i;
            }
        }
        if (category < NUM_MESSAGE_CATEGORY) {
            if (color) {
                fputs(message_category_style[category], output);
            }
            if (num_open[category] == 0 && mult_ext[category] == 0) {
                fputc('~', output);
            } else if (category <= MESSAGE_WARNING) {
                fputc('^', output);
            } else {
                fputc('-', output);
            }
        } else {
            fputc(' ', output);
            size_t next_col = start_seen < start_count ? starts[start_seen]->position.begin.column : NO_POS;
            if (end_seen < end_count && ends[end_seen]->position.end.column < next_col) {
                next_col = ends[end_seen]->position.end.column;
            }
            for (size_t i = end_start; i < end_seen; i++) {
                if (!text_printed[i]) {
                    size_t width = getStringWidth(tocnstr(ends[i]->message));
                    if (cur_col + width + FST_LAYER_CLEARANCE < next_col) {
                        if (color) {
                            fputs(message_category_style[ends[i]->category], output);
                        }
                        fwrite(ends[i]->message.data, sizeof(char), ends[i]->message.length, output);
                        text_printed[i] = true;
                        cur_col += width;
                        break;
                    }
                }
            }
        }
        if (color) {
            fputs(CONSOLE_SGR(), output);
        }
        cur_col++;
    }
    fputc('\n', output);
}

static bool printFragmentLineSecondaryLayer(
    FILE* output, MessageFragment** ends, size_t end_count, bool* text_printed, bool snd, int numwidth, bool color
) {
    size_t count = 0;
    for (size_t i = 0; i < end_count; i++) {
        if (!text_printed[i]) {
            count++;
        }
    }
    if (count == 0) {
        return false;
    } else {
        printFragmentLineStart(output, numwidth, color);
        fputc(' ', output);
        size_t cur_col = 0;
        size_t end_seen = 0;
        while (end_seen < end_count) {
            MessageCategory category = NUM_MESSAGE_CATEGORY;
            size_t end_start = end_seen;
            while (end_seen < end_count && ends[end_seen]->position.end.column == 1 + cur_col) {
                if (!text_printed[end_seen] && ends[end_seen]->category < category) {
                    category = ends[end_seen]->category;
                }
                end_seen++;
            }
            while (end_seen < end_count && text_printed[end_seen]) {
                end_seen++;
            }
            size_t next_col = end_seen < end_count ? ends[end_seen]->position.end.column : NO_POS;
            bool printed = false;
            for (size_t i = end_start; i < end_seen; i++) {
                if (!text_printed[i]) {
                    size_t width = getStringWidth(tocnstr(ends[i]->message));
                    if (cur_col + width + SND_LAYER_CLEARANCE < next_col) {
                        if (color) {
                            fputs(message_category_style[ends[i]->category], output);
                        }
                        if (snd) {
                            fputc('`', output);
                        }
                        fwrite(ends[i]->message.data, sizeof(char), ends[i]->message.length, output);
                        text_printed[i] = true;
                        cur_col += width;
                        printed = true;
                        break;
                    }
                }
            }
            if (!printed) {
                if (category < NUM_MESSAGE_CATEGORY) {
                    if (color) {
                        fputs(message_category_style[category], output);
                    }
                    fputc('|', output);
                } else {
                    fputc(' ', output);
                }
                cur_col++;
            }
            if (color) {
                fputs(CONSOLE_SGR(), output);
            }
        }
        fputc('\n', output);
        return true;
    }
}

static void printFragmentLine(
    FILE* output, MessageFragment** starts, MessageFragment** ends, size_t start_count, size_t end_count, int numwidth, bool color
) {
    bool text_printed[end_count + 1];
    for (size_t i = 0; i < end_count; i++) {
        text_printed[i] = (ends[i]->message.length == 0);
    }
    printFragmentLineFirstLayer(output, starts, ends, start_count, end_count, text_printed, numwidth, color);
    bool snd = true;
    while (printFragmentLineSecondaryLayer(output, ends, end_count, text_printed, snd, numwidth, color)) {
        snd = false;
    }
}

static void printFragmentsInSameFile(FILE* output, MessageFragment** start_order, size_t frag_count, int numwidth, bool color) {
    FILE* file = openFileStream(start_order[0]->position.file, "rb+");
    if (file != NULL) {
        size_t total_frags = frag_count;
        while (frag_count > 0 && !isSpanWithPosition(start_order[frag_count - 1]->position)) {
            frag_count--;
        }
        if (frag_count > 0) {
            MessageFragment* end_order[frag_count];
            memcpy(end_order, start_order, frag_count * sizeof(MessageFragment*));
            sortFragmentByEnd(end_order, frag_count);
            positionToStartOfLine(file, start_order[0]->position.begin.offset, NUM_SURROUNDING_LINES);
            printSourceLines(output, file, start_order[0]->position.begin.line - NUM_SURROUNDING_LINES + 1, NUM_SURROUNDING_LINES, numwidth, color);
            size_t last_line = start_order[0]->position.begin.line - 1;
            size_t start_pos = 0;
            size_t end_pos = 0;
            while (end_pos < frag_count) {
                Location current_loc = start_pos < frag_count ? start_order[start_pos]->position.begin : invalidLocation();
                if (end_order[end_pos]->position.end.offset < current_loc.offset) {
                    current_loc = end_order[end_pos]->position.end;
                }
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
                while (end_pos + end_count < frag_count && start_order[end_pos + end_count]->position.end.line == current_loc.line) {
                    end_count++;
                }
                printFragmentLine(output, start_order + start_pos, end_order + end_pos, start_count, end_count, numwidth, color);
                start_pos += start_count;
                end_pos += end_count;
                last_line = current_loc.line;
            }
            printSourceLines(output, file, last_line + 2, NUM_SURROUNDING_LINES, numwidth, color);
        }
        for (size_t i = frag_count; i < total_frags; i++) {
            printFragmentWithoutSource(output, start_order[i], numwidth, false, color);
        }
    } else {
        for (size_t i = 0; i < frag_count; i++) {
            printFragmentWithoutSource(output, start_order[i], numwidth, true, color);
        }
    }
}

static void printMessageFragments(
    FILE* output, const MessageFilter* filter, MessageFragment** fragments, size_t frag_count, bool print_source, bool print_headers, bool color
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
                if (print_headers) {
                    printFileNameLine(output, findMainPosition(start_order + start, count), maxwidth, color);
                }
                printFragmentsInSameFile(output, start_order + start, count, maxwidth, color);
                start += count;
            }
            for (size_t i = start; i < to_print_count; i++) {
                printFragmentWithoutSource(output, start_order[i], maxwidth, true, color);
            }
        } else {
            for (size_t i = 0; i < to_print_count; i++) {
                printFragmentWithoutSource(output, start_order[i], MIN_NUMWIDTH, true, color);
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
        ConstString category_name = getMessageCategoryName(category);
        fwrite(category_name.data, 1, category_name.length, output);
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
            printMessageFragments(output, filter, message->fragments, message->fragment_count, print_source, true, color);
        } else if (print_source && message->fragment_count != 0) {
            Span location = findMainPosition(message->fragments, message->fragment_count);
            if (location.begin.offset != NO_POS) {
                MessageFragment dummy = {
                    .category = category,
                    .message = createString("", 0),
                    .position = location,
                };
                MessageFragment* fragments = &dummy;
                printMessageFragments(output, filter, &fragments, 1, true, false, color);
            }
        }
    }
}

void printMessages(const MessageContext* message_context, FILE* output, bool print_fragments, bool print_source) {
    for (size_t i = 0; i < message_context->message_count; i++) {
        printMessage(message_context->messages[i], output, message_context->filter, print_fragments, print_source);
    }
}

void printAndClearMessages(MessageContext* message_context, FILE* output, bool print_fragments, bool print_source) {
    printMessages(message_context, output, print_fragments, print_source);
    clearMessageContext(message_context);
}

