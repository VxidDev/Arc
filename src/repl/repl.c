#include "../../include/repl/repl.h"
#include "../../include/symbol-table.h"
#include <stdio.h>

#include "../../include/builtIns/print.h"
#include "../../include/builtIns/errors.h"
#include "../../include/builtIns/typing.h"
#include "../../include/builtIns/math.h"

void registerBuiltins(SymbolTable* table) {
  NativeFunction* printFn = initNativeFunction("print", builtIn_print, 1, true);
  setTable(table, "print", (Object*)printFn);
  
  freeObject((Object*)printFn);

  NativeFunction* runtimeErrorFn = initNativeFunction("RuntimeError", builtIn_RuntimeError, 1, false);
  setTable(table, "RuntimeError", (Object*)runtimeErrorFn);

  freeObject((Object*)runtimeErrorFn);

  NativeFunction* typeofFn = initNativeFunction("typeof", builtIn_typeof, 1, false);
  setTable(table, "typeof", (Object*)typeofFn);

  freeObject((Object*)typeofFn);

  NativeFunction* truncateFn = initNativeFunction("truncate", builtIn_truncate, 1, false);
  setTable(table, "truncate", (Object*)truncateFn);

  freeObject((Object*)truncateFn);
}
