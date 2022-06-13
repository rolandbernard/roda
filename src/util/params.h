#ifndef _UTIL_PARAMS_H_
#define _UTIL_PARAMS_H_

#include <stdbool.h>
#include <string.h>

#define PARAM_WARN_UNKNOWN_VALUE() {                                \
    const char* msg_base = "unknown value, ignoring this value";    \
    int msg_len = strlen(msg_base);                                 \
    int val_len = strlen(value);                                    \
    char msg[val_len + msg_len + 3];                                \
    memcpy(msg, value, val_len);                                    \
    memcpy(msg + val_len, ": ", 2);                                 \
    memcpy(msg + val_len + 2, msg_base, msg_len + 1);               \
    PARAM_WARN(msg);                                                \
}

#define PARAM_WARN(ABOUT)                               \
    _raiseWarning(option, ABOUT, argc, argv, context);

#define PARAM_SPEC_FUNCTION(NAME, PARAM, BODY)                                                          \
    void _warning_ ## NAME(                                                                             \
        const char* option, const char* warning, int argc, const char* const* argv, PARAM context       \
    ) {                                                                                                 \
        void (*_raiseWarning)(                                                                          \
            const char*, const char*, int, const char* const*, PARAM                                    \
        ) = _warning_ ## NAME;                                                                          \
        bool _help = false;                                                                             \
        bool _letters = false;                                                                          \
        bool _names = false;                                                                            \
        bool _def = false;                                                                              \
        bool _warning = true;                                                                           \
        int _len = 0;                                                                                   \
        int _i = 0, _j = 0;                                                                             \
        do {                                                                                            \
            BODY                                                                                        \
        } while (false);                                                                                \
    }                                                                                                   \
    int NAME(bool _help, int argc, const char* const* argv, PARAM context) {                            \
        void (*_raiseWarning)(                                                                          \
            const char*, const char*, int, const char* const*, PARAM                                    \
        ) = _warning_ ## NAME;                                                                          \
        bool _warning = false;                                                                          \
        const char* warning = NULL;                                                                     \
        int _args = 0;                                                                                  \
        int _len = 0;                                                                                   \
        int _i = 0, _j = 0;                                                                             \
        if (_help) {                                                                                    \
            do {                                                                                        \
                bool _letters = false;                                                                  \
                bool _names = false;                                                                    \
                bool _def = false;                                                                      \
                const char* option = NULL;                                                              \
                BODY                                                                                    \
            } while (false);                                                                            \
        } else {                                                                                        \
            for (_i = 1; _i < argc; _i++) {                                                             \
                if (argv[_i][0] == '-') {                                                               \
                    if (argv[_i][1] == '-') {                                                           \
                        bool _letters = false;                                                          \
                        bool _names = true;                                                             \
                        bool _def = false;                                                              \
                        int _len = 0;                                                                   \
                        while (argv[_i][_len + 2] != 0 && argv[_i][_len + 2] != '=') {                  \
                            _len++;                                                                     \
                        }                                                                               \
                        char option[_len + 3];                                                          \
                        memcpy(option, argv[_i], _len + 2);                                             \
                        option[_len + 2] = 0;                                                           \
                        _args++;                                                                        \
                        BODY                                                                            \
                        PARAM_WARN("unknown command line option");                                      \
                        _args--;                                                                        \
                    } else {                                                                            \
                        bool _letters = true;                                                           \
                        bool _names = false;                                                            \
                        bool _def = false;                                                              \
                        for (_j = 1; argv[_i][_j] != 0; _j++) {                                         \
                            const char option[3] = {'-', argv[_i][_j], 0};                              \
                            _args++;                                                                    \
                            BODY                                                                        \
                            PARAM_WARN("unknown command line option");                                  \
                            _args--;                                                                    \
                        }                                                                               \
                    }                                                                                   \
                } else {                                                                                \
                    bool _letters = false;                                                              \
                    bool _names = false;                                                                \
                    bool _def = true;                                                                   \
                    const char* option = NULL;                                                          \
                    _args++;                                                                            \
                    BODY                                                                                \
                }                                                                                       \
            }                                                                                           \
        }                                                                                               \
        return _args;                                                                                   \
    }

#define PARAM_PRINT_HELP(SPEC, PARAM) SPEC(true, 0, NULL, PARAM)

#define PARAM_PARSE_ARGS(SPEC, ARGC, ARGV, PARAM) SPEC(false, ARGC, ARGV, PARAM)

#define PARAM_USAGE(STRING)                     \
    if (_help) {                                \
        fputs("Usage: " STRING "\n", stderr);   \
        fputs("Options:\n", stderr);            \
    }

#define PARAM_FLAG(LETTER, NAME, ACTION, DESC)                                  \
    if (_help) {                                                                \
        printOptionHelpLine(LETTER, NAME, NULL, DESC);                          \
    } else if (_letters) {                                                      \
        if (LETTER != 0 && argv[_i][_j] == LETTER) {                            \
            ACTION; continue;                                                   \
        }                                                                       \
    } else if (_names) {                                                        \
        if (NAME != NULL && strcmp(option + 2, ((const char*)NAME)) == 0) {     \
            if (argv[_i][_len + 2] == '=') {                                    \
                PARAM_WARN("option does not expect a value, ignoring it");      \
            }                                                                   \
            ACTION; continue;                                                   \
        }                                                                       \
    }

#define PARAM_VALUED_BASE(LETTER, NAME, ACTION, TAGGED, FOLLOWED, VALUE_DESC, DESC)         \
    if (_help) {                                                                            \
        printOptionHelpLine(LETTER, NAME, VALUE_DESC, DESC);                                \
    } else if (_letters) {                                                                  \
        if (LETTER != 0 && argv[_i][_j] == LETTER) {                                        \
            if (argv[_i][_j + 1] == '=') {                                                  \
                const char* value = argv[_i] + _j + 2;                                      \
                _j = strlen(argv[_i]) - 1;                                                  \
                ACTION; continue;                                                           \
            } else {                                                                        \
                const char* value = NULL;                                                   \
                if (_j == 1) {                                                              \
                    if (argv[_i][_j + 1] != 0) {                                            \
                        value = argv[_i] + _j + 1;                                          \
                        _j = strlen(argv[_i]) - 1;                                          \
                    } else if (_i + 1 < argc && argv[_i + 1][0] != '-') {                   \
                        value = argv[_i + 1];                                               \
                        _i++;                                                               \
                        _j = strlen(argv[_i]) - 1;                                          \
                    }                                                                       \
                }                                                                           \
                ACTION; continue;                                                           \
            }                                                                               \
        }                                                                                   \
    } else if (_names) {                                                                    \
        const char* name = NAME;                                                            \
        if (name != NULL) {                                                                 \
            if(strcmp(option + 2, name) == 0) {                                             \
                if (argv[_i][_len + 2] == '=') {                                            \
                    const char* value = argv[_i] + _len + 3;                                \
                    ACTION; continue;                                                       \
                } else {                                                                    \
                    const char* value = NULL;                                               \
                    if (FOLLOWED && _i + 1 < argc && argv[_i + 1][0] != '-') {              \
                        value = argv[_i + 1];                                               \
                        _i++;                                                               \
                    }                                                                       \
                    ACTION; continue;                                                       \
                }                                                                           \
            }                                                                               \
        }                                                                                   \
    }

#define PARAM_VALUED(LETTER, NAME, ACTION, OPTIONAL, VALUE_DESC, DESC)                      \
    PARAM_VALUED_BASE(LETTER, NAME, {                                                       \
        if (!OPTIONAL && value == NULL) {                                                   \
            PARAM_WARN("expected a value, ignoring this option");                           \
        } else {                                                                            \
            ACTION                                                                          \
        }                                                                                   \
    }, true, !OPTIONAL, VALUE_DESC, DESC)

#define PARAM_STRING_LIST(LETTER, NAME, ACTION, VALUE_DESC, DESC)                           \
    PARAM_VALUED(LETTER, NAME, {                                                            \
        if (value != NULL) {                                                                \
            while (value[0] != 0) {                                                         \
                int len = 0;                                                                \
                while (value[len] != 0 && value[len] != ',') {                              \
                    len++;                                                                  \
                }                                                                           \
                {                                                                           \
                    char next_value[len + 1];                                               \
                    memcpy(next_value, value, len);                                         \
                    next_value[len] = 0;                                                    \
                    char* value = next_value;                                               \
                    ACTION                                                                  \
                }                                                                           \
                if (value[len] == ',') {                                                    \
                    value += len + 1;                                                       \
                } else {                                                                    \
                    value += len;                                                           \
                }                                                                           \
            }                                                                               \
        }                                                                                   \
    }, false, VALUE_DESC, DESC);

#define PARAM_INTEGER(LETTER, NAME, ACTION, VALUE_DESC, DESC)                               \
    PARAM_VALUED(LETTER, NAME, {                                                            \
        bool is_integer = true;                                                             \
        int int_value = 0;                                                                  \
        for (int i = 0; value[i] != 0; i++) {                                               \
            if (value[i] >= '0' && value[i] <= '9') {                                       \
                int_value *= 10;                                                            \
                int_value += value[i] - '0';                                                \
            } else {                                                                        \
                is_integer = false;                                                         \
                break;                                                                      \
            }                                                                               \
        }                                                                                   \
        if (!is_integer) {                                                                  \
            PARAM_WARN("expected an integer value, ignoring this option");                  \
        } else {                                                                            \
            int value = int_value;                                                          \
            ACTION                                                                          \
        }                                                                                   \
    }, false, VALUE_DESC, DESC)

#define PARAM_DEFAULT(ACTION)           \
    if (_def) {                         \
        const char* value = argv[_i];   \
        ACTION; continue;               \
    }

#define PARAM_WARNING(ACTION)       \
    if (_warning) { ACTION }

void printOptionHelpLine(char single, const char* word, const char* value, const char* desc);

#endif
