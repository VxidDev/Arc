#include "../../include/repl/repl.h"
#include "../../include/symbol-table.h"
#include <stdio.h>

#include "../../include/builtIns/print.h"

void registerBuiltins(SymbolTable* table) {
  NativeFunction* printFn = initNativeFunction("print", builtIn_print, 255);
  setTable(table, "print", (Object*)printFn);
  
  freeObject((Object*)printFn);
}
