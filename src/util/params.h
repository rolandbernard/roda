#ifndef _UTIL_PARAMS_H_
#define _UTIL_PARAMS_H_

#include <stdbool.h>
#include <string.h>

#define PARAM_SPEC_FUNCTION(NAME, PARAM, BODY)                                                          \
    void _warning_ ## NAME(                                                                             \
        const char* option, const char* warning, int argc, const char* const* argv, PARAM context       \
    ) {                                                                                                 \
        bool _help = false;                                                                             \
        bool _letters = false;                                                                          \
        bool _names = false;                                                                            \
        bool _def = false;                                                                              \
        bool _warning = true;                                                                           \
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
                        const char* option = argv[_i];                                                  \
                        _args++;                                                                        \
                        BODY                                                                            \
                        _raiseWarning(option, "unknown command line option", argc, argv, context);      \
                    } else {                                                                            \
                        bool _letters = true;                                                           \
                        bool _names = false;                                                            \
                        bool _def = false;                                                              \
                        for (_j = 1; argv[_i][_j] != 0; _j++) {                                         \
                            const char option[3] = {'-', argv[_i][_j], 0};                              \
                            _args++;                                                                    \
                            BODY                                                                        \
                            _raiseWarning(option, "unknown command line option", argc, argv, context);  \
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
        if (LETTER != NULL && argv[_i][_j] == ((const char*)LETTER)[0]) {       \
            ACTION; continue;                                                   \
        }                                                                       \
    } else if (_names) {                                                        \
        if (NAME != NULL && strcmp(argv[_i] + 2, ((const char*)NAME)) == 0) {   \
            ACTION; continue;                                                   \
        }                                                                       \
    }

#define PARAM_STRING(LETTER, NAME, ACTION, DESC)

#define PARAM_DEFAULT(ACTION)           \
    if (_def) {                         \
        const char* value = argv[_i];   \
        ACTION; continue;               \
    }

#define PARAM_WARNING(ACTION)       \
    if (_warning) { ACTION }

void printOptionHelpLine(const char* single, const char* word, const char* value, const char* desc);

#endif
