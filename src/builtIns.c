#include "../include/builtIns.h"
#include "../include/builtIns/math.h"
#include "../include/builtIns/sys.h"
#include "../include/builtIns/time.h"

void initMathModule(SymbolTable* table) {
  NativeFunction* truncateFn = initNativeFunction("truncate", builtIn_truncate, 1, false); // DEPRECATED, to_int() is prefered.
  setTable(table, internIdentifier("truncate", 8), VAL_OBJ((Object*)truncateFn));
}

void initSysModule(SymbolTable* table) {
  NativeFunction* exitFn = initNativeFunction("exit", builtIn_exit, 1, false);
  setTable(table, internIdentifier("exit", 4), VAL_OBJ((Object*)exitFn));
}

void initTimeModule(SymbolTable* table) {
  NativeFunction* perf_counterFn = initNativeFunction("perf_counter", builtIn_perf_counter, 0, false);
  setTable(table, internIdentifier("perf_counter", 12), VAL_OBJ((Object*)perf_counterFn));
}
