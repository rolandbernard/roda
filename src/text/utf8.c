
#include "text/utf8.h"

#include "util/alloc.h"

int decodeUTF8(Rune* rune, const char* data, int max_length) {
    if (max_length <= 0) {
        *rune = INVALID_RUNE;
        return 0;
    } else {
        if ((unsigned char)(data[0]) <= 0x7f) {
            *rune = (Rune)data[0];
            return 1;
        } else {
            int length = 1;
            while (length < 8 && (data[0] & (1 << (7 - length))) != 0) {
                length++;
            }
            if (length == 1) {
                *rune = INVALID_RUNE;
                return 1;
            } else if (length > max_length) {
                *rune = INVALID_RUNE;
                return max_length;
            } else {
                Rune ret = (Rune)(data[0] & (0xff >> (length + 1))) << ((length - 1) * 6);
                for (int i = 1; i < length; i++) {
                    if ((data[i] & 0xc0) != 0x80) {
                        *rune = INVALID_RUNE;
                        return i - 1;
                    }
                    ret |= (Rune)(data[i] & 0x3f) << ((length - 1 - i) * 6);
                }
                *rune = ret;
                return length;
            }
        }
    }
}

int encodeUTF8(Rune rune, char* output, int max_length) {
    if (max_length <= 0 || rune < 0) {
        return 0;
    } else if (rune <= 0x7f) {
        output[0] = rune;
        return 1;
    } else {
        int length = 2;
        Rune tmp = rune;
        while (tmp >> ((length - 1) * 6) > (0xff >> (length + 1))) {
            length++;
        }
        if (length > max_length) {
            return 0;
        } else {
            output[0] = (0xff << (8 - length)) | (rune >> ((length - 1) * 6));
            for (int i = 1; i < length; i++) {
                output[i] = 0x80 | (0x3f & (rune >> ((length - 1 - i) * 6)));
            }
            return length;
        }
    }
}

void initUtf8Stream(Utf8Stream* stream, ConstString string) {
    stream->data = string;
    stream->offset = 0;
}

void positionUtf8Stream(Utf8Stream* stream, int position) {
    stream->offset = position;
}

Rune nextUtf8Rune(Utf8Stream* stream) {
    Rune next_rune;
    int len = decodeUTF8(&next_rune, stream->data.data, stream->data.length - stream->offset);
    stream->offset += len;
    return next_rune;
}

Rune peekUtf8Rune(Utf8Stream* stream) {
    Rune next_rune;
    decodeUTF8(&next_rune, stream->data.data, stream->data.length - stream->offset);
    return next_rune;
}

int readUtf8FromFileStream(FILE* file, Rune* rune) {
    int first_byte = fgetc(file);
    if (first_byte == EOF) {
        return 0;
    } else if (first_byte <= 0x7f) {
        *rune = (Rune)first_byte;
        return 1;
    } else {
        int length = 1;
        while (length < 8 && (first_byte & (1 << (7 - length))) != 0) {
            length++;
        }
        if (length == 1) {
            *rune = INVALID_RUNE;
            return 1;
        } else {
            Rune ret = (Rune)(first_byte & (0xff >> (length + 1))) << ((length - 1) * 6);
            for (int i = 1; i < length; i++) {
                int next_byte = fgetc(file);
                if (next_byte == EOF || (next_byte & 0xc0) != 0x80) {
                    if (next_byte != EOF) {
                        ungetc(next_byte, file);
                    }
                    *rune = INVALID_RUNE;
                    return i - 1;
                }
                ret |= (Rune)(next_byte & 0x3f) << ((length - 1 - i) * 6);
            }
            *rune = ret;
            return length;
        }
    }
}

int getUtf8Length(ConstString string) {
    int length = 0;
    Rune tmp;
    int offset = 0;
    for (;;) {
        int len = decodeUTF8(&tmp, string.data + offset, string.length - offset);
        if (len == 0) {
            break;
        }
        length++;
        offset += len;
    }
    return length;
}
