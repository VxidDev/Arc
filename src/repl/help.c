#include "../../include/ansi-colors.h"
#include "../../include/repl/repl.h"
#include "../../include/repl/help.h"

#include <stdio.h>

void printHelp() {
  printf(
    "%sUsage: %s%sarc [file|optional] %s[options]\n"
    "\n Options:"
    "\n    -h, --help                               Show this help message"
    "\n    -c, --code <str>                         Execute code from given string"
    "\n    -d, --debug                              Enable verbose debug output"
    "\n    -p, --float-precision <n>                Set float output precision"
    "\n    -n, --disable-colored-formatting         Disable ANSI color output%s"
    "\n    -m, --mempool-size <n>                   Set custom memory pool size."
    "\n    -l, --last-result                        Print last evaluation result."
    "\n    -S, --skip-evaluation                    Skip AST evaluation."
    "\n    -A, --arena-block-size <n>               Set custom memory block arena size (in kilobytes).\n",
    COLOR(ANSI_BOLD), COLOR(ANSI_RESET), COLOR(ANSI_CYAN_FG), COLOR(ANSI_WHITE_FG), COLOR(ANSI_RESET)
  );
}
