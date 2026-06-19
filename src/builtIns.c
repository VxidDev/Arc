#include "../include/builtIns.h"
#include "../include/builtIns/math.h"
#include "../include/builtIns/sys.h"
#include "../include/builtIns/time.h"

void initMathModule(SymbolTable* table) {
  NativeFunction* truncateFn = initNativeFunction("truncate", builtIn_truncate, 1, false); // DEPRECATED, to_int() is prefered.
  setTable(table, internIdentifier("truncate", 8), VAL_OBJ((Object*)truncateFn));
  freeObject((Object*)truncateFn);
}

void initSysModule(SymbolTable* table) {
  NativeFunction* exitFn = initNativeFunction("exit", builtIn_exit, 1, false);
  setTable(table, internIdentifier("exit", 4), VAL_OBJ((Object*)exitFn));
  freeObject((Object*)exitFn);
  
  #ifndef _WIN32
    NativeFunction* accessFn = initNativeFunction("access", builtIn_access, 2, false);
    setTable(table, internIdentifier("access", 6), VAL_OBJ((Object*)accessFn));
    freeObject((Object*)accessFn);

    NativeFunction* unlinkFn = initNativeFunction("unlink", builtIn_unlink, 1, false);
    setTable(table, internIdentifier("unlink", 6), VAL_OBJ((Object*)unlinkFn));
    freeObject((Object*)unlinkFn);
  #endif
}

void initTimeModule(SymbolTable* table) {
  NativeFunction* perf_counterFn = initNativeFunction("perf_counter", builtIn_perf_counter, 0, false);
  setTable(table, internIdentifier("perf_counter", 12), VAL_OBJ((Object*)perf_counterFn));
  freeObject((Object*)perf_counterFn);
}
