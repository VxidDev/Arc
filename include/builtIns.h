#ifndef BUILTINS_H
#define BUILTINS_H

#include "symbol-table.h"

void initMathModule(SymbolTable* globals);
void initSysModule(SymbolTable* globals);
void initTimeModule(SymbolTable* globals);

#endif // !BUILTINS_H
