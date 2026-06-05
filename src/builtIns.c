#include "../include/builtIns.h"
#include "../include/builtIns/math.h"
#include "../include/builtIns/sys.h"

void initMathModule(SymbolTable* table) {
  NativeFunction* truncateFn = initNativeFunction("truncate", builtIn_truncate, 1, false);
  setTable(table, "truncate", (Object*)truncateFn, false);
}

void initSysModule(SymbolTable* table) {
  NativeFunction* exitFn = initNativeFunction("exit", builtIn_exit, 1, false);
  setTable(table, "exit", (Object*)exitFn, false);
}
