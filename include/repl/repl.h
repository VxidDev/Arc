#ifndef REPL_H
#define REPL_H

#include <stdbool.h>

#include "../symbol-table.h"

extern char *_CODE;
extern bool _DEBUG;
extern int _FLOAT_PRECISION;
extern bool _IS_COLORED;

#define COLOR(c) (_IS_COLORED ? (c) : "")

void registerBuiltins(SymbolTable *table);

#endif // REPL_H 
