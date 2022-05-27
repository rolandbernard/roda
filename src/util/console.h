#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include <stdbool.h>
#include <stdio.h>

#include "util/macro.h"

#define CONSOLE_ESC "\e"
#define CONSOLE_CSI CONSOLE_ESC "["

#define CONSOLE_CUU(N) CONSOLE_CSI STRINGIFY(N) "A"
#define CONSOLE_CUD(N) CONSOLE_CSI STRINGIFY(N) "B"
#define CONSOLE_CUF(N) CONSOLE_CSI STRINGIFY(N) "C"
#define CONSOLE_CUB(N) CONSOLE_CSI STRINGIFY(N) "D"
#define CONSOLE_CNL(N) CONSOLE_CSI STRINGIFY(N) "E"
#define CONSOLE_CPL(N) CONSOLE_CSI STRINGIFY(N) "F"
#define CONSOLE_CHA(N) CONSOLE_CSI STRINGIFY(N) "G"
#define CONSOLE_CUP(N, M) CONSOLE_CSI STRINGIFY(N) ";" STRINGIFY(N) "H"
#define CONSOLE_ED(N) CONSOLE_CSI STRINGIFY(N) "J"
#define CONSOLE_EL(N) CONSOLE_CSI STRINGIFY(N) "K"
#define CONSOLE_SU(N) CONSOLE_CSI STRINGIFY(N) "S"
#define CONSOLE_SD(N) CONSOLE_CSI STRINGIFY(N) "T"
#define CONSOLE_SGR(N) CONSOLE_CSI STRINGIFY(N) "m"
#define CONSOLE_DSR CONSOLE_CSI "6n"
#define CONSOLE_SCP CONSOLE_CSI "s"
#define CONSOLE_RCP CONSOLE_CSI "u"
#define CONSOLE_SHC CONSOLE_CSI "?25h"
#define CONSOLE_HDP CONSOLE_CSI "?25l"
#define CONSOLE_EAB CONSOLE_CSI "?1049h"
#define CONSOLE_DAB CONSOLE_CSI "?1049l"
#define CONSOLE_EBP CONSOLE_CSI "?2004h"
#define CONSOLE_DBP CONSOLE_CSI "?2004l"

#define CONSOLE_SGR_OFF 0
#define CONSOLE_SGR_BOLD 1
#define CONSOLE_SGR_FAINT 2
#define CONSOLE_SGR_ITALIC 3
#define CONSOLE_SGR_UNDERLINE 4
#define CONSOLE_SGR_BLINK 5
#define CONSOLE_SGR_INVERT 7
#define CONSOLE_SGR_STRIKE 9
#define CONSOLE_SGR_FONT(N) 1 ## N

#define CONSOLE_SGR_FG_BLACK 30
#define CONSOLE_SGR_FG_RED 31
#define CONSOLE_SGR_FG_GREEN 32
#define CONSOLE_SGR_FG_YELLOW 33
#define CONSOLE_SGR_FG_BLUE 34
#define CONSOLE_SGR_FG_MAGENTA 35
#define CONSOLE_SGR_FG_CYAN 36
#define CONSOLE_SGR_FG_WHITE 37
#define CONSOLE_SGR_FG_BRIGHT_BLACK 90
#define CONSOLE_SGR_FG_BRIGHT_RED 91
#define CONSOLE_SGR_FG_BRIGHT_GREEN 92
#define CONSOLE_SGR_FG_BRIGHT_YELLOW 93
#define CONSOLE_SGR_FG_BRIGHT_BLUE 94
#define CONSOLE_SGR_FG_BRIGHT_MAGENTA 95
#define CONSOLE_SGR_FG_BRIGHT_CYAN 96
#define CONSOLE_SGR_FG_BRIGHT_WHITE 97

#define CONSOLE_SGR_FG_BLACK 30
#define CONSOLE_SGR_FG_RED 31
#define CONSOLE_SGR_FG_GREEN 32
#define CONSOLE_SGR_FG_YELLOW 33
#define CONSOLE_SGR_FG_BLUE 34
#define CONSOLE_SGR_FG_MAGENTA 35
#define CONSOLE_SGR_FG_CYAN 36
#define CONSOLE_SGR_FG_WHITE 37
#define CONSOLE_SGR_FG_BRIGHT_BLACK 90
#define CONSOLE_SGR_FG_BRIGHT_RED 91
#define CONSOLE_SGR_FG_BRIGHT_GREEN 92
#define CONSOLE_SGR_FG_BRIGHT_YELLOW 93
#define CONSOLE_SGR_FG_BRIGHT_BLUE 94
#define CONSOLE_SGR_FG_BRIGHT_MAGENTA 95
#define CONSOLE_SGR_FG_BRIGHT_CYAN 96
#define CONSOLE_SGR_FG_BRIGHT_WHITE 97

bool isATerminal(FILE* stream);

#endif