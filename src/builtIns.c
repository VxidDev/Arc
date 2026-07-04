#include "../include/builtIns.h"
#include "../include/builtIns/math.h"
#include "../include/builtIns/sys.h"
#include "../include/builtIns/time.h"
#include "../include/builtIns/ctools.h"
#include "../include/builtIns/libtools.h"

void initMathModule(SymbolTable* table) {
  NativeFunction* truncateFn = initNativeFunction("truncate", builtIn_truncate, 1, false); // DEPRECATED, to_int() is prefered.
  setTable(table, internIdentifier("truncate", 8), VAL_OBJ((Object*)truncateFn));
  freeObject((Object*)truncateFn);
}

void initLibtools(SymbolTable* table) {
  NativeFunction* stdlib_pathFn = initNativeFunction("stdlib_path", builtIn_stdlib_path, 0, false);
  setTable(table, internIdentifier("stdlib_path", 11), VAL_OBJ((Object*)stdlib_pathFn));
  freeObject((Object*)stdlib_pathFn);
}

void initCTools(SymbolTable* table) {
  NativeFunction* dl_openFn = initNativeFunction("dl_open", builtIn_dl_open, 2, false);
  setTable(table, internIdentifier("dl_open", 7), VAL_OBJ((Object*)dl_openFn));
  freeObject((Object*)dl_openFn);

  NativeFunction* dl_symFn = initNativeFunction("dl_sym", builtIn_dl_sym, 4, false);
  setTable(table, internIdentifier("dl_sym", 6), VAL_OBJ((Object*)dl_symFn));
  freeObject((Object*)dl_symFn);

  NativeFunction* dl_closeFn = initNativeFunction("dl_close", builtIn_dl_close, 1, false);
  setTable(table, internIdentifier("dl_close", 8), VAL_OBJ((Object*)dl_closeFn));
  freeObject((Object*)dl_closeFn);

  NativeFunction* __dl_symFn = initNativeFunction("__dl_sym", builtIn_raw_dl_sym, 2, false);
  setTable(table, internIdentifier("__dl_sym", 8), VAL_OBJ((Object*)__dl_symFn));
  freeObject((Object*)__dl_symFn);

  NativeFunction* c_func_signatureFn = initNativeFunction("c_func_signature", builtIn_c_func_signature, 1, true);
  setTable(table, internIdentifier("c_func_signature", 16), VAL_OBJ((Object*)c_func_signatureFn));
  freeObject((Object*)c_func_signatureFn);
  
  // Will be replaced after addition of .arc wrapper
  setTable(table, internIdentifier("C_INT", 5), VAL_INT(0));
  setTable(table, internIdentifier("C_INT_PTR", 9), VAL_INT(1));
  setTable(table, internIdentifier("C_FLOAT", 7), VAL_INT(2));
  setTable(table, internIdentifier("C_FLOAT_PTR", 11), VAL_INT(3));
  setTable(table, internIdentifier("C_DOUBLE", 8), VAL_INT(4));
  setTable(table, internIdentifier("C_DOUBLE_PTR", 12), VAL_INT(5));
  setTable(table, internIdentifier("C_CHAR", 6), VAL_INT(6));
  setTable(table, internIdentifier("C_CHAR_PTR", 10), VAL_INT(7));
  setTable(table, internIdentifier("C_VOID_PTR", 10), VAL_INT(8));

  NativeFunction* c_runFn = initNativeFunction("c_run", builtIn_c_run, 3, false);
  setTable(table, internIdentifier("c_run", 5), VAL_OBJ((Object*)c_runFn));
  freeObject((Object*)c_runFn);

  NativeFunction* string_atFn = initNativeFunction("string_at", builtIn_string_at, 1, false);
  setTable(table, internIdentifier("string_at", 9), VAL_OBJ((Object*)string_atFn));
  freeObject((Object*)string_atFn);

  NativeFunction* int_atFn = initNativeFunction("int_at", builtIn_int_at, 1, false);
  setTable(table, internIdentifier("int_at", 6), VAL_OBJ((Object*)int_atFn));
  freeObject((Object*)int_atFn);
}

void initSysModule(SymbolTable* table) {
  NativeFunction* exitFn = initNativeFunction("exit", builtIn_exit, 1, false);
  setTable(table, internIdentifier("exit", 4), VAL_OBJ((Object*)exitFn));
  freeObject((Object*)exitFn);

  NativeFunction* systemFn = initNativeFunction("system", builtIn_system, 1, false);
  setTable(table, internIdentifier("system", 6), VAL_OBJ((Object*)systemFn));
  freeObject((Object*)systemFn);
  
  #ifndef _WIN32
    NativeFunction* accessFn = initNativeFunction("access", builtIn_access, 2, false);
    setTable(table, internIdentifier("access", 6), VAL_OBJ((Object*)accessFn));
    freeObject((Object*)accessFn);

    NativeFunction* unlinkFn = initNativeFunction("unlink", builtIn_unlink, 1, false);
    setTable(table, internIdentifier("unlink", 6), VAL_OBJ((Object*)unlinkFn));
    freeObject((Object*)unlinkFn);

    NativeFunction* writeFn = initNativeFunction("write", builtIn_write, 3, false);
    setTable(table, internIdentifier("write", 5), VAL_OBJ((Object*)writeFn));
    freeObject((Object*)writeFn);
  #endif
}

void initTimeModule(SymbolTable* table) {
  NativeFunction* perf_counterFn = initNativeFunction("perf_counter", builtIn_perf_counter, 0, false);
  setTable(table, internIdentifier("perf_counter", 12), VAL_OBJ((Object*)perf_counterFn));
  freeObject((Object*)perf_counterFn);
}
