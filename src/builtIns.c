#include "../include/builtIns.h"
#include "../include/builtIns/math.h"

void initMathModule(SymbolTable* table) {
  NativeFunction* truncateFn = initNativeFunction("truncate", builtIn_truncate, 1, false);
  setTable(table, "truncate", (Object*)truncateFn);

  freeObject((Object*)truncateFn); 
}
