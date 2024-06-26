
#include "text/utf8.h"

size_t decodeUTF8(CodePoint* code, const char* data, size_t max_length) {
    if (max_length <= 0) {
        *code = INVALID_CODEPOINT;
        return 0;
    } else {
        if ((unsigned char)(data[0]) <= 0x7f) {
            *code = (CodePoint)data[0];
            return 1;
        } else {
            size_t length = 1;
            while (length < 8 && (data[0] & (1 << (7 - length))) != 0) {
                length++;
            }
            if (length == 1) {
                *code = INVALID_CODEPOINT;
                return 1;
            } else if (length > max_length) {
                *code = INVALID_CODEPOINT;
                return max_length;
            } else {
                CodePoint ret = (CodePoint)(data[0] & (0xff >> (length + 1))) << ((length - 1) * 6);
                for (size_t i = 1; i < length; i++) {
                    if ((data[i] & 0xc0) != 0x80) {
                        *code = INVALID_CODEPOINT;
                        return i;
                    }
                    ret |= (CodePoint)(data[i] & 0x3f) << ((length - 1 - i) * 6);
                }
                *code = ret;
                return length;
            }
        }
    }
}

size_t encodeUTF8(CodePoint code, char* output, size_t max_length) {
    if (max_length <= 0 || code < 0) {
        return 0;
    } else if (code <= 0x7f) {
        output[0] = code;
        return 1;
    } else {
        size_t length = 2;
        CodePoint tmp = code;
        while (tmp >> ((length - 1) * 6) > (0xff >> (length + 1))) {
            length++;
        }
        if (length > max_length) {
            return 0;
        } else {
            output[0] = (0xff << (8 - length)) | (code >> ((length - 1) * 6));
            for (size_t i = 1; i < length; i++) {
                output[i] = 0x80 | (0x3f & (code >> ((length - 1 - i) * 6)));
            }
            return length;
        }
    }
}

void initUtf8Stream(Utf8Stream* stream, ConstString string) {
    stream->data = string;
    stream->offset = 0;
}

void positionUtf8Stream(Utf8Stream* stream, size_t position) {
    stream->offset = position;
}

CodePoint nextUtf8CodePoint(Utf8Stream* stream) {
    if (stream->offset < stream->data.length) {
        CodePoint next_rune;
        size_t len = decodeUTF8(&next_rune, stream->data.data + stream->offset, stream->data.length - stream->offset);
        stream->offset += len;
        return next_rune;
    } else {
        return INVALID_CODEPOINT;
    }
}

CodePoint peekUtf8CodePoint(const Utf8Stream* stream) {
    CodePoint next_rune;
    decodeUTF8(&next_rune, stream->data.data + stream->offset, stream->data.length - stream->offset);
    return next_rune;
}

size_t readUtf8FromFileStream(FILE* file, CodePoint* code) {
    int first_byte = fgetc(file);
    if (first_byte == EOF) {
        *code = INVALID_CODEPOINT;
        return 0;
    } else if (first_byte <= 0x7f) {
        *code = (CodePoint)first_byte;
        return 1;
    } else {
        size_t length = 1;
        while (length < 8 && (first_byte & (1 << (7 - length))) != 0) {
            length++;
        }
        if (length == 1) {
            *code = INVALID_CODEPOINT;
            return 1;
        } else {
            CodePoint ret = (CodePoint)(first_byte & (0xff >> (length + 1))) << ((length - 1) * 6);
            for (size_t i = 1; i < length; i++) {
                int next_byte = fgetc(file);
                if (next_byte == EOF || (next_byte & 0xc0) != 0x80) {
                    if (next_byte != EOF) {
                        ungetc(next_byte, file);
                    }
                    *code = INVALID_CODEPOINT;
                    return i;
                }
                ret |= (CodePoint)(next_byte & 0x3f) << ((length - 1 - i) * 6);
            }
            *code = ret;
            return length;
        }
    }
}

size_t getStringWidth(ConstString string) {
    size_t length = 0;
    CodePoint code;
    size_t offset = 0;
    for (;;) {
        size_t len = decodeUTF8(&code, string.data + offset, string.length - offset);
        if (len == 0) {
            break;
        }
        length += getCodePointWidth(code);
        offset += len;
    }
    return length;
}

typedef struct {
    int first;
    int last;
} Interval;

static size_t findInIntervalTable(CodePoint code, const Interval *table, size_t max) {
    if (code < table[0].first || code > table[max].last) {
        return NO_POS;
    } else {
        size_t min = 0;
        while (min <= max) {
            size_t mid = (min + max) / 2;
            if (code > table[mid].last) {
                min = mid + 1;
            } else if (code < table[mid].first) {
                max = mid - 1;
            } else {
                return mid;
            }
        }
        return NO_POS;
    }
}

// Table of zero width characters
static const Interval zero_width[] = {
    { 0x0300, 0x036F },   { 0x0483, 0x0486 },   { 0x0488, 0x0489 },   { 0x0591, 0x05BD },
    { 0x05BF, 0x05BF },   { 0x05C1, 0x05C2 },   { 0x05C4, 0x05C5 },   { 0x05C7, 0x05C7 },
    { 0x0600, 0x0603 },   { 0x0610, 0x0615 },   { 0x064B, 0x065E },   { 0x0670, 0x0670 },
    { 0x06D6, 0x06E4 },   { 0x06E7, 0x06E8 },   { 0x06EA, 0x06ED },   { 0x070F, 0x070F },
    { 0x0711, 0x0711 },   { 0x0730, 0x074A },   { 0x07A6, 0x07B0 },   { 0x07EB, 0x07F3 },
    { 0x0901, 0x0902 },   { 0x093C, 0x093C },   { 0x0941, 0x0948 },   { 0x094D, 0x094D },
    { 0x0951, 0x0954 },   { 0x0962, 0x0963 },   { 0x0981, 0x0981 },   { 0x09BC, 0x09BC },
    { 0x09C1, 0x09C4 },   { 0x09CD, 0x09CD },   { 0x09E2, 0x09E3 },   { 0x0A01, 0x0A02 },
    { 0x0A3C, 0x0A3C },   { 0x0A41, 0x0A42 },   { 0x0A47, 0x0A48 },   { 0x0A4B, 0x0A4D },
    { 0x0A70, 0x0A71 },   { 0x0A81, 0x0A82 },   { 0x0ABC, 0x0ABC },   { 0x0AC1, 0x0AC5 },
    { 0x0AC7, 0x0AC8 },   { 0x0ACD, 0x0ACD },   { 0x0AE2, 0x0AE3 },   { 0x0B01, 0x0B01 },
    { 0x0B3C, 0x0B3C },   { 0x0B3F, 0x0B3F },   { 0x0B41, 0x0B43 },   { 0x0B4D, 0x0B4D },
    { 0x0B56, 0x0B56 },   { 0x0B82, 0x0B82 },   { 0x0BC0, 0x0BC0 },   { 0x0BCD, 0x0BCD },
    { 0x0C3E, 0x0C40 },   { 0x0C46, 0x0C48 },   { 0x0C4A, 0x0C4D },   { 0x0C55, 0x0C56 },
    { 0x0CBC, 0x0CBC },   { 0x0CBF, 0x0CBF },   { 0x0CC6, 0x0CC6 },   { 0x0CCC, 0x0CCD },
    { 0x0CE2, 0x0CE3 },   { 0x0D41, 0x0D43 },   { 0x0D4D, 0x0D4D },   { 0x0DCA, 0x0DCA },
    { 0x0DD2, 0x0DD4 },   { 0x0DD6, 0x0DD6 },   { 0x0E31, 0x0E31 },   { 0x0E34, 0x0E3A },
    { 0x0E47, 0x0E4E },   { 0x0EB1, 0x0EB1 },   { 0x0EB4, 0x0EB9 },   { 0x0EBB, 0x0EBC },
    { 0x0EC8, 0x0ECD },   { 0x0F18, 0x0F19 },   { 0x0F35, 0x0F35 },   { 0x0F37, 0x0F37 },
    { 0x0F39, 0x0F39 },   { 0x0F71, 0x0F7E },   { 0x0F80, 0x0F84 },   { 0x0F86, 0x0F87 },
    { 0x0F90, 0x0F97 },   { 0x0F99, 0x0FBC },   { 0x0FC6, 0x0FC6 },   { 0x102D, 0x1030 },
    { 0x1032, 0x1032 },   { 0x1036, 0x1037 },   { 0x1039, 0x1039 },   { 0x1058, 0x1059 },
    { 0x1160, 0x11FF },   { 0x135F, 0x135F },   { 0x1712, 0x1714 },   { 0x1732, 0x1734 },
    { 0x1752, 0x1753 },   { 0x1772, 0x1773 },   { 0x17B4, 0x17B5 },   { 0x17B7, 0x17BD },
    { 0x17C6, 0x17C6 },   { 0x17C9, 0x17D3 },   { 0x17DD, 0x17DD },   { 0x180B, 0x180D },
    { 0x18A9, 0x18A9 },   { 0x1920, 0x1922 },   { 0x1927, 0x1928 },   { 0x1932, 0x1932 },
    { 0x1939, 0x193B },   { 0x1A17, 0x1A18 },   { 0x1B00, 0x1B03 },   { 0x1B34, 0x1B34 },
    { 0x1B36, 0x1B3A },   { 0x1B3C, 0x1B3C },   { 0x1B42, 0x1B42 },   { 0x1B6B, 0x1B73 },
    { 0x1DC0, 0x1DCA },   { 0x1DFE, 0x1DFF },   { 0x200B, 0x200F },   { 0x202A, 0x202E },
    { 0x2060, 0x2063 },   { 0x206A, 0x206F },   { 0x20D0, 0x20EF },   { 0x302A, 0x302F },
    { 0x3099, 0x309A },   { 0xA806, 0xA806 },   { 0xA80B, 0xA80B },   { 0xA825, 0xA826 },
    { 0xFB1E, 0xFB1E },   { 0xFE00, 0xFE0F },   { 0xFE20, 0xFE23 },   { 0xFEFF, 0xFEFF },
    { 0xFFF9, 0xFFFB },   { 0x10A01, 0x10A03 }, { 0x10A05, 0x10A06 }, { 0x10A0C, 0x10A0F },
    { 0x10A38, 0x10A3A }, { 0x10A3F, 0x10A3F }, { 0x1D167, 0x1D169 }, { 0x1D173, 0x1D182 },
    { 0x1D185, 0x1D18B }, { 0x1D1AA, 0x1D1AD }, { 0x1D242, 0x1D244 }, { 0xE0001, 0xE0001 },
    { 0xE0020, 0xE007F }, { 0xE0100, 0xE01EF }
};

int getCodePointWidth(CodePoint point) {
    if (point == '\b') {
        return -1;  // Backspace has -1 width
    } else if (point == '\t') {
        return 1;   // Tab has width 1
    } else if (point < 32 || (point >= 0x7f && point < 0xa0)) {
        return 0;   // Other control characters have 0 width
    } else if (findInIntervalTable(point, zero_width, sizeof(zero_width) / sizeof(zero_width[0]) - 1) != NO_POS) {
        return 0;   // This is a zero width character
    } else if (
        point >= 0x1100
        && (
            point <= 0x115f
            || point == 0x2329
            || point == 0x232a
            || (point >= 0x2e80 && point <= 0xa4cf && point != 0x303f)
            || (point >= 0xac00 && point <= 0xd7a3)
            || (point >= 0xf900 && point <= 0xfaff)
            || (point >= 0xfe10 && point <= 0xfe19)
            || (point >= 0xfe30 && point <= 0xfe6f)
            || (point >= 0xff00 && point <= 0xff60)
            || (point >= 0xffe0 && point <= 0xffe6)
            || (point >= 0x20000 && point <= 0x2fffd)
            || (point >= 0x30000 && point <= 0x3fffd)
        )
    ) {
        return 2;   // Some characters have a width of 2
    } else {
        return 1;   // All other characters have width 1
    }
}

