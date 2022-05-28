
#include "text/utf8.h"

#include "util/alloc.h"

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
                        return i - 1;
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

CodePoint nextUtf8Rune(Utf8Stream* stream) {
    CodePoint next_rune;
    size_t len = decodeUTF8(&next_rune, stream->data.data, stream->data.length - stream->offset);
    stream->offset += len;
    return next_rune;
}

CodePoint peekUtf8Rune(const Utf8Stream* stream) {
    CodePoint next_rune;
    decodeUTF8(&next_rune, stream->data.data, stream->data.length - stream->offset);
    return next_rune;
}

size_t readUtf8FromFileStream(FILE* file, CodePoint* code) {
    int first_byte = fgetc(file);
    if (first_byte == EOF) {
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
                    return i - 1;
                }
                ret |= (CodePoint)(next_byte & 0x3f) << ((length - 1 - i) * 6);
            }
            *code = ret;
            return length;
        }
    }
}

size_t getUtf8Length(ConstString string) {
    size_t length = 0;
    CodePoint tmp;
    size_t offset = 0;
    for (;;) {
        size_t len = decodeUTF8(&tmp, string.data + offset, string.length - offset);
        if (len == 0) {
            break;
        }
        length++;
        offset += len;
    }
    return length;
}
