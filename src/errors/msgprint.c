
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "errors/msgprint.h"

#include "text/string.h"
#include "text/utf8.h"
#include "util/alloc.h"
#include "util/console.h"

static const char* message_category_style[] = {
    [MESSAGE_UNKNOWN] = CONSOLE_SGR(CONSOLE_SGR_BOLD),
    [MESSAGE_ERROR] = CONSOLE_SGR(CONSOLE_SGR_BOLD;CONSOLE_SGR_FG_BRIGHT_RED),
    [MESSAGE_WARNING] = CONSOLE_SGR(CONSOLE_SGR_BOLD;CONSOLE_SGR_FG_BRIGHT_YELLOW),
    [MESSAGE_NOTE] = CONSOLE_SGR(CONSOLE_SGR_BOLD;CONSOLE_SGR_FG_BRIGHT_BLUE),
    [MESSAGE_HELP] = CONSOLE_SGR(CONSOLE_SGR_BOLD;CONSOLE_SGR_FG_BRIGHT_CYAN),
    [MESSAGE_DEBUG] = CONSOLE_SGR(CONSOLE_SGR_BOLD),
};

typedef struct {
    size_t line;
    size_t column;
    size_t line_offset;
} FilePosition;

static int fragmentCompare(const MessageFragment* a, const MessageFragment* b, bool by_end) {
    if (a->position.file != b->position.file) {
        return a->position.file - b->position.file;
    }
    if (isSpanFileOnly(a->position) != isSpanFileOnly(b->position)) {
        return isSpanFileOnly(a->position) ? -1 : 1;
    } else if (!by_end && a->position.offset != b->position.offset) {
        return a->position.offset - b->position.offset;
    } else if (by_end && getSpanEndOffset(a->position) != getSpanEndOffset(b->position)) {
        return getSpanEndOffset(a->position) - getSpanEndOffset(b->position);
    } else if (a->category != b->category) {
        return b->category - a->category;
    } else if (a->position.length != b->position.length) {
        return a->position.length - b->position.length;
    } else {
        return 0;
    }
}

// NOTE: Printing the errors is not thread safe
static MessageFragment** fragment_sort_original_order;

static int fragmentCompareByEnd(const void* a, const void* b) {
    return fragmentCompare(fragment_sort_original_order[*(size_t*)a], fragment_sort_original_order[*(size_t*)b], true);
}

static void sortFragmentIndexByEnd(MessageFragment** fragments, size_t fragment_count, size_t* sorted) {
    for (size_t i = 0; i < fragment_count; i++) {
        sorted[i] = i;
    }
    fragment_sort_original_order = fragments;
    qsort(sorted, fragment_count, sizeof(size_t), fragmentCompareByEnd);
}

static int fragmentCompareByStart(const void* a, const void* b) {
    return fragmentCompare(*(MessageFragment**)a, *(MessageFragment**)b, false);
}

static void sortFragmentsByStart(MessageFragment** fragments, size_t fragment_count) {
    qsort(fragments, fragment_count, sizeof(MessageFragment*), fragmentCompareByStart);
}

static void printFileNameLine(int linewidth, const File* file, FilePosition* position, const char* note, FILE* output, bool istty) {
    if (istty) {
        fputs(CONSOLE_SGR(CONSOLE_SGR_BOLD), output);
    }
    for (int i = 0; i < linewidth; i++) {
        fputc(' ', output);
    }
    fputs("--> ", output);
    if (istty) {
        fputs(CONSOLE_SGR(), output);
    }
    fwrite(file->original_path.data, sizeof(char), file->original_path.length, output);
    if (position != NULL) {
        fprintf(output, ":%zi:%zi", position->line, position->column);
    }
    if (note != NULL) {
        fputs(": (", output);
        fputs(note, output);
        fputc(')', output);
    }
    fputc('\n', output);
}

static void printMessageFragmentWithoutSource(
    MessageFragment* fragment, int linewidth, const File* file, FilePosition* start,
    FilePosition* end, FILE* output, bool istty, const MessageFilter* filter
) {
    if (applyFilterForCategory(filter, fragment->category)) {
        if (fragment->message.length > 0 || file != NULL) {
            if (istty) {
                fputs(CONSOLE_SGR(CONSOLE_SGR_BOLD), output);
            }
            for (int i = 0; i < linewidth; i++) {
                fputc(' ', output);
            }
            fputs(" = ", output);
            if (istty) {
                fputs(CONSOLE_SGR(), output);
            }
            if (file != NULL) {
                fwrite(file->original_path.data, sizeof(char), file->original_path.length, output);
                if (start != NULL) {
                    fprintf(output, ":%zi:%zi", start->line, start->column);
                    if (end != NULL) {
                        fprintf(output, "-%zi:%zi", end->line, end->column);
                    }
                }
            }
            if (fragment->message.length > 0) {
                if (file != NULL) {
                    fputs(": ", output);
                }
                if (istty) {
                    fputs(message_category_style[fragment->category], output);
                }
                fwrite(fragment->message.data, sizeof(char), fragment->message.length, output);
                if (istty) {
                    fputs(CONSOLE_SGR(), output);
                }
            }
            fputc('\n', output);
        }
    }
}

static size_t printSourceLine(size_t offset, size_t line, int linewidth, FILE* source, FILE* output, bool istty) {
    if (offset != NO_POS) {
        fseek(source, offset, SEEK_SET);
    }
    if (istty) {
        fputs(CONSOLE_SGR(CONSOLE_SGR_BOLD), output);
    }
    fprintf(output, "%*zi | ", linewidth, line);
    if (istty) {
        fputs(CONSOLE_SGR(), output);
    }
    int next_char = getc(source);
    while (next_char != '\n' && next_char != EOF) {
        if (next_char == '\t') {
            next_char = ' ';
        }
        putc(next_char, output);
        next_char = getc(source);
        offset++;
    }
    putc('\n', output);
    return offset;
}

static int getDecimalNumberWidth(size_t n) {
    int ret = 1;
    while (n >= 10) {
        n /= 10;
        ret++;
    }
    return ret;
}

static void printFileMessageFragments(
    MessageFragment** fragments, size_t fragment_count, FILE* output,
    bool istty, const MessageFilter* filter, bool print_source
) {
    if (fragment_count > 0) {
        const File* file = fragments[0]->position.file;
        if (file != NULL) {
            MessageFragment** file_only_fragments = fragments;
            size_t file_only_fragment_count = 0;
            while (fragment_count > 0 && isSpanFileOnly(fragments[0]->position)) {
                fragments++;
                fragment_count--;
                file_only_fragment_count++;
            }
            if (fragment_count > 0) {
                FILE* stream = openFileStream(file, "rb");
                if (stream != NULL) {
                    size_t ordered_by_end[fragment_count];
                    sortFragmentIndexByEnd(fragments, fragment_count, ordered_by_end);
                    FilePosition starts[fragment_count];
                    FilePosition ends[fragment_count];
                    size_t current_offset = 0;
                    size_t current_line = 1;
                    size_t current_column = 1;
                    size_t line_offset = 0;
                    bool end_of_file = false;
                    size_t start_index = 0;
                    size_t end_index = 0;
                    while (!end_of_file) {
                        while (start_index < fragment_count && fragments[start_index]->position.offset <= current_offset) {
                            starts[start_index].line = current_line;
                            starts[start_index].column = current_column;
                            starts[start_index].line_offset = line_offset;
                            start_index++;
                        }
                        while (end_index < fragment_count && getSpanEndOffset(fragments[ordered_by_end[end_index]]->position) <= current_offset) {
                            ends[ordered_by_end[end_index]].line = current_line;
                            ends[ordered_by_end[end_index]].column = current_column;
                            ends[ordered_by_end[end_index]].line_offset = line_offset;
                            end_index++;
                        }
                        CodePoint next_rune;
                        size_t len = readUtf8FromFileStream(stream, &next_rune);
                        if (len <= 0) {
                            end_of_file = true;
                        } else {
                            current_offset += len;
                            if (next_rune == '\n') {
                                current_column = 1;
                                current_line++;
                                line_offset = current_offset;
                            } else {
                                current_column++;
                            }
                        }
                    }
                    while (start_index < fragment_count) {
                        starts[start_index].line = current_line;
                        starts[start_index].column = current_column;
                        start_index++;
                    }
                    if (print_source) {
                        int linewidth = getDecimalNumberWidth(starts[fragment_count - 1].line);
                        size_t main_fragment = 0;
                        for (size_t i = 0; i < fragment_count; i++) {
                            if (fragments[i]->category < fragments[main_fragment]->category) {
                                main_fragment = i;
                            }
                        }
                        bool is_multiline[fragment_count];
                        for (size_t i = 0; i < fragment_count; i++) {
                            if (starts[i].line != ends[i].line) {
                                is_multiline[i] = true;
                            } else {
                                is_multiline[i] = false;
                            }
                            is_multiline[i] = (starts[i].line != ends[i].line);
                        }
                        start_index = 0;
                        end_index = 0;
                        bool print_at_end[fragment_count];
                        for (size_t i = 0; i < fragment_count; i++) {
                            while (
                                start_index < fragment_count
                                && getSpanEndOffset(fragments[ordered_by_end[i]]->position) > fragments[start_index]->position.offset
                            ) {
                                start_index++;
                            }
                            while (
                                end_index < fragment_count
                                && getSpanEndOffset(fragments[ordered_by_end[i]]->position) >= getSpanEndOffset(fragments[ordered_by_end[end_index]]->position)
                            ) {
                                end_index++;
                            }
                            size_t length = getUtf8Length(toConstString(fragments[ordered_by_end[i]]->message));
                            if (length > 0) {
                                size_t end_column = ends[ordered_by_end[i]].column + length;
                                if (
                                    (
                                        start_index >= fragment_count
                                        || ends[ordered_by_end[i]].line != starts[start_index].line
                                        || end_column + 2 < starts[start_index].column
                                    ) && (
                                        end_index >= fragment_count
                                        || ends[ordered_by_end[i]].line != ends[ordered_by_end[end_index]].line
                                        || end_column + 3 < ends[ordered_by_end[end_index]].column
                                    ) && (
                                        end_index >= fragment_count
                                        || starts[ordered_by_end[end_index]].line != ends[ordered_by_end[end_index]].line
                                        || ends[ordered_by_end[i]].line != starts[ordered_by_end[end_index]].line
                                        || end_column < starts[ordered_by_end[end_index]].column
                                    )
                                ) {
                                    print_at_end[ordered_by_end[i]] = true;
                                } else {
                                    print_at_end[ordered_by_end[i]] = false;
                                }
                            } else {
                                print_at_end[ordered_by_end[i]] = true;
                            }
                        }
                        printFileNameLine(linewidth, file, &starts[main_fragment], NULL, output, istty);
                        size_t start_start = 0;
                        size_t end_start = 0;
                        size_t last_line = 0;
                        while (start_start < fragment_count || end_start < fragment_count) {
                            size_t next_line;
                            size_t next_line_offset;
                            if (start_start < fragment_count && end_start < fragment_count) {
                                if (starts[start_start].line < ends[ordered_by_end[end_start]].line_offset) {
                                    next_line = starts[start_start].line;
                                    next_line_offset = starts[start_start].line_offset;
                                } else {
                                    next_line = ends[ordered_by_end[end_start]].line;
                                    next_line_offset = ends[ordered_by_end[end_start]].line_offset;
                                }
                            } else if (start_start < fragment_count) {
                                next_line = starts[start_start].line;
                                next_line_offset = starts[start_start].line_offset;
                            } else {
                                next_line = ends[ordered_by_end[end_start]].line;
                                next_line_offset = ends[ordered_by_end[end_start]].line_offset;
                            }
                            size_t start_end = start_start;
                            while (start_end < fragment_count && starts[start_end].line == next_line) {
                                start_end++;
                            }
                            size_t end_end = end_start;
                            while (end_end < fragment_count && ends[ordered_by_end[end_end]].line == next_line) {
                                end_end++;
                            }
                            if (last_line != 0) {
                                if (last_line + 2 >= next_line) {
                                    for (size_t i = last_line + 1; i < next_line; i++) {
                                        printSourceLine(NO_POS, i, linewidth, stream, output, istty);
                                    }
                                } else {
                                    for (int i = 0; i < linewidth; i++) {
                                        fputc(' ', output);
                                    }
                                    fputs("...\n", output);
                                }
                            }
                            size_t length = printSourceLine(next_line_offset, next_line, linewidth, stream, output, istty) - next_line_offset;
                            if (istty) {
                                fputs(CONSOLE_SGR(CONSOLE_SGR_BOLD), output);
                            }
                            for (int i = 0; i < linewidth; i++) {
                                fputc(' ', output);
                            }
                            fputs(" | ", output);
                            if (istty) {
                                fputs(CONSOLE_SGR(), output);
                            }
                            size_t depths[NUM_MESSAGE_CATEGORY];
                            size_t multi_start[NUM_MESSAGE_CATEGORY];
                            size_t multi_end[NUM_MESSAGE_CATEGORY];
                            for (size_t k = 0; k < NUM_MESSAGE_CATEGORY; k++) {
                                depths[k] = 0;
                                multi_start[k] = 0;
                                multi_end[k] = 0;
                            }
                            start_index = start_start;
                            end_index = end_start;
                            for (size_t i = 1; i <= length + 2; i++) {
                                while (start_index < start_end && starts[start_index].column <= i) {
                                    if (is_multiline[start_index]) {
                                        multi_start[fragments[start_index]->category] = 2;
                                    } else {
                                        depths[fragments[start_index]->category]++;
                                    }
                                    start_index++;
                                }
                                bool print_end = false;
                                while (end_index < end_end && ends[ordered_by_end[end_index]].column <= i + 1) {
                                    if (is_multiline[ordered_by_end[end_index]]) {
                                        multi_end[fragments[ordered_by_end[end_index]]->category] = 2;
                                    } else {
                                        multi_end[fragments[ordered_by_end[end_index]]->category] = 1;
                                        depths[fragments[ordered_by_end[end_index]]->category]--;
                                    }
                                    end_index++;
                                }
                                bool printed = false;
                                for (size_t k = 0; k < NUM_MESSAGE_CATEGORY; k++) {
                                    if (!printed && (depths[k] > 0 || multi_start[k] == 2 || multi_end[k] == 1 || multi_start[k] == 1 || multi_end[k] == 2)) {
                                        if (istty) {
                                            fputs(message_category_style[k], output);
                                        }
                                        if (depths[k] > 0 || multi_start[k] == 2 || multi_end[k] == 1) {
                                            if (k <= MESSAGE_WARNING) {
                                                fputc('^', output);
                                            } else {
                                                fputc('-', output);
                                            }
                                        } else if (multi_start[k] == 1 || multi_end[k] == 2) {
                                            fputc('~', output);
                                        }
                                        if (istty) {
                                            fputs(CONSOLE_SGR(), output);
                                        }
                                        printed = true;
                                    }
                                    if (multi_start[k] > 0) {
                                        multi_start[k]--;
                                    }
                                    if (multi_end[k] > 0) {
                                        multi_end[k]--;
                                        if (multi_end[k] == 0) {
                                            print_end = true;
                                        }
                                    }
                                }
                                if (print_end && print_at_end[ordered_by_end[end_index - 1]]) {
                                    MessageFragment* fragment = fragments[ordered_by_end[end_index - 1]];
                                    ConstString msg = toConstString(fragment->message);
                                    if (msg.length != 0) {
                                        fputc(' ', output);
                                        if (istty) {
                                            fputs(message_category_style[fragment->category], output);
                                        }
                                        fwrite(msg.data, 1, msg.length, output);
                                        if (istty) {
                                            fputs(CONSOLE_SGR(), output);
                                        }
                                        i += 2 + getUtf8Length(msg);
                                        fputc(' ', output);
                                    }
                                } else if (!printed) {
                                    fputc(' ', output);
                                }
                            }
                            fputc('\n', output);
                            bool first_print = true;
                            size_t last_column;
                            bool was_last_first = false;
                            for (size_t i = start_end - 1; i >= start_start && i != NO_POS; i--) {
                                if (!print_at_end[i]) {
                                    if (istty) {
                                        fputs(CONSOLE_SGR(CONSOLE_SGR_BOLD), output);
                                    }
                                    for (int i = 0; i < linewidth; i++) {
                                        fputc(' ', output);
                                    }
                                    fputs(" | ", output);
                                    if (istty) {
                                        fputs(CONSOLE_SGR(), output);
                                    }
                                    start_index = start_start;
                                    while (start_index < start_end && print_at_end[start_index]) {
                                        start_index++;
                                    }
                                    for (size_t j = 1; j < starts[i].column; j++) {
                                        if (start_index < start_end && starts[start_index].column == j) {
                                            while (start_index < start_end && starts[start_index].column == j) {
                                                start_index++;
                                            }
                                            if (istty) {
                                                fputs(message_category_style[fragments[start_index - 1]->category], output);
                                            }
                                            fputc('|', output);
                                            if (istty) {
                                                fputs(CONSOLE_SGR(), output);
                                            }
                                            while (start_index < start_end && (print_at_end[start_index] || starts[start_index].column == j)) {
                                                start_index++;
                                            }
                                        } else {
                                            fputc(' ', output);
                                        }
                                    }
                                    if (istty) {
                                        fputs(message_category_style[fragments[i]->category], output);
                                    }
                                    if (first_print) {
                                        fputs(" `", output);
                                        first_print = false;
                                        was_last_first = true;
                                    } else if (was_last_first && starts[i].column == last_column) {
                                        fputs("  ", output);
                                    } else {
                                        was_last_first = false;
                                    }
                                    last_column = starts[i].column;
                                    ConstString msg = toConstString(fragments[i]->message);
                                    fwrite(msg.data, 1, msg.length, output);
                                    if (istty) {
                                        fputs(CONSOLE_SGR(), output);
                                    }
                                    fputc('\n', output);
                                }
                            }
                            last_line = next_line;
                            start_start = start_end;
                            end_start = end_end;
                        }
                        for (size_t i = 0; i < file_only_fragment_count; i++) {
                            printMessageFragmentWithoutSource(file_only_fragments[i], linewidth, NULL, NULL, NULL, output, istty, filter);
                        }
                    } else {
                        if (fragment_count + file_only_fragment_count == 1) {
                            printMessageFragmentWithoutSource(file_only_fragments[0], 1, file, &starts[0], &ends[0], output, istty, filter);
                        } else {
                            printFileNameLine(1, file, NULL, NULL, output, istty);
                            for (size_t i = 0; i < fragment_count; i++) {
                                printMessageFragmentWithoutSource(fragments[i], 1, file, &starts[i], &ends[i], output, istty, filter);
                            }
                            for (size_t i = 0; i < file_only_fragment_count; i++) {
                                printMessageFragmentWithoutSource(file_only_fragments[i], 1, NULL, NULL, NULL, output, istty, filter);
                            }
                        }
                    }
                    fclose(stream);
                } else {
                    const char* error = strerror(errno);
                    printFileNameLine(1, file, NULL, error, output, istty);
                    for (size_t i = 0; i < fragment_count; i++) {
                        printMessageFragmentWithoutSource(fragments[i], 1, NULL, NULL, NULL, output, istty, filter);
                    }
                    for (size_t i = 0; i < file_only_fragment_count; i++) {
                        printMessageFragmentWithoutSource(file_only_fragments[i], 1, NULL, NULL, NULL, output, istty, filter);
                    }
                }
            } else {
                if (file_only_fragment_count == 1) {
                    printMessageFragmentWithoutSource(file_only_fragments[0], 1, file, NULL, NULL, output, istty, filter);
                } else {
                    printFileNameLine(1, file, NULL, NULL, output, istty);
                    for (size_t i = 0; i < file_only_fragment_count; i++) {
                        printMessageFragmentWithoutSource(file_only_fragments[i], 1, NULL, NULL, NULL, output, istty, filter);
                    }
                }
            }
        } else {
            for (size_t i = 0; i < fragment_count; i++) {
                printMessageFragmentWithoutSource(fragments[i], 1, NULL, NULL, NULL, output, istty, filter);
            }
        }
    }
}

void printMessage(const Message* message, FILE* output, const MessageFilter* filter, bool print_fragments, bool print_source) {
    if (applyFilterForKind(filter, message->kind)) {
        bool istty = isATerminal(output);
        MessageCategory category = getMessageCategory(message->kind);
        if (istty) {
            fputs(message_category_style[category], output);
        }
        ConstString category_name = getMessageCategoryName(category);
        fwrite(category_name.data, sizeof(char), category_name.length, output);
        if (category == MESSAGE_ERROR && message->kind != ERROR_UNKNOWN) {
            fprintf(output, "[E%.4i]", message->kind - ERRORS_START - 1);
        } else if (category == MESSAGE_WARNING && message->kind != WARNING_UNKNOWN) {
            fprintf(output, "[W%.4i]", message->kind - WARNINGS_START - 1);
        }
        if (istty) {
            fputs(CONSOLE_SGR() CONSOLE_SGR(CONSOLE_SGR_BOLD), output);
        }
        if (message->message.length != 0) {
            fputs(": ", output);
            fwrite(message->message.data, sizeof(char), message->message.length, output);
        }
        fputc('\n', output);
        if (istty) {
            fputs(CONSOLE_SGR(), output);
        }
        if (print_fragments) {
            if (message->fragment_count > 0) {
                sortFragmentsByStart(message->fragments, message->fragment_count);
                size_t start = 0;
                while (start < message->fragment_count) {
                    size_t end = start + 1;
                    while (end < message->fragment_count && message->fragments[end]->position.file == message->fragments[start]->position.file) {
                        end++;
                    }
                    printFileMessageFragments(message->fragments + start, end - start, output, istty, filter, print_source);
                    start = end;
                }
            }
        }
    }
}

void printMessages(const MessageContext* message_context, FILE* output, const MessageFilter* filter, bool print_fragments, bool print_source) {
    for (size_t i = 0; i < message_context->message_count; i++) {
        printMessage(message_context->messages[i], output, filter, print_fragments, print_source);
    }
}

