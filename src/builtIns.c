#include "../include/builtIns.h"
#include "../include/builtIns/math.h"
#include "../include/builtIns/sys.h"
#include "../include/builtIns/time.h"
#include "../include/builtIns/ctools.h"
#include "../include/builtIns/libtools.h"
#include "../include/builtIns/random.h"

#include <time.h>
/**
 * initRandomModule
 * * Summary: Initializes the random module by registering the 'randint' native function and seeding the random number generator.
 * * @param table: Pointer to the SymbolTable where the module functions are registered.
 * * @returns: void (nothing).
 * * @note: Instantiates the 'randint' native function, registers it in the symbol table, and frees the temporary object wrapper. Seeds the PCG32 generator using the current system time.
 */
void initRandomModule(SymbolTable* table) {
  NativeFunction* randintFn = initNativeFunction("randint", builtIn_randint, 1, false);
  setTable(table, internIdentifier("randint", 7), VAL_OBJ((Object*)randintFn));
  freeObject((Object*)randintFn);

  pcg32srandom((uint64_t)time(NULL), 1);
}
/**
 * initMathModule
 * * Summary: Initializes the math module by registering math-related native functions like 'truncate'.
 * * @param table: Pointer to the SymbolTable where the math functions are registered.
 * * @returns: void (nothing).
 * * @note: Note that 'truncate' is marked as DEPRECATED in favor of 'to_int'. Registers and frees the function object wrapper.
 */
void initMathModule(SymbolTable* table) {
  NativeFunction* truncateFn = initNativeFunction("truncate", builtIn_truncate, 1, false); // DEPRECATED, to_int() is prefered.
  setTable(table, internIdentifier("truncate", 8), VAL_OBJ((Object*)truncateFn));
  freeObject((Object*)truncateFn);
}
/**
 * initLibtools
 * * Summary: Registers core library utility functions, specifically 'stdlib_path', into the system.
 * * @param table: Pointer to the SymbolTable where utility functions are registered.
 * * @returns: void (nothing).
 * * @note: Creates the native function 'stdlib_path' with 0 arguments and registers it in the symbol table before freeing the temporary object.
 */
void initLibtools(SymbolTable* table) {
  NativeFunction* stdlib_pathFn = initNativeFunction("stdlib_path", builtIn_stdlib_path, 0, false);
  setTable(table, internIdentifier("stdlib_path", 11), VAL_OBJ((Object*)stdlib_pathFn));
  freeObject((Object*)stdlib_pathFn);
}

#ifndef ARC_EXCLUDE_CTOOLS
/**
 * initCTools
 * * Summary: Registers native C interaction functions (like 'dl_open' and 'dl_sym') for dynamic linking, unless excluded.
 * * @param table: Pointer to the SymbolTable where C-interaction functions are registered.
 * * @returns: void (nothing).
 * * @note: Wrapped in 'ifndef ARC_EXCLUDE_CTOOLS' to allow disabling C-linking tools during compilation. Registers dynamic loading hooks.
 */
void initCTools(SymbolTable* table) {
  NativeFunction* dl_openFn = initNativeFunction("dl_open", builtIn_dl_open, 1, false);
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
  setTable(table, internIdentifier("C_VOID", 6), VAL_INT(9));

  NativeFunction* c_runFn = initNativeFunction("c_run", builtIn_c_run, 3, true);
  setTable(table, internIdentifier("c_run", 5), VAL_OBJ((Object*)c_runFn));
  freeObject((Object*)c_runFn);

  NativeFunction* string_atFn = initNativeFunction("string_at", builtIn_string_at, 1, false);
  setTable(table, internIdentifier("string_at", 9), VAL_OBJ((Object*)string_atFn));
  freeObject((Object*)string_atFn);

  NativeFunction* int_atFn = initNativeFunction("int_at", builtIn_int_at, 1, false);
  setTable(table, internIdentifier("int_at", 6), VAL_OBJ((Object*)int_atFn));
  freeObject((Object*)int_atFn);

  NativeFunction* pointer_atFn = initNativeFunction("pointer_at", builtIn_pointer_at, 1, false);
  setTable(table, internIdentifier("pointer_at", 10), VAL_OBJ((Object*)pointer_atFn));
  freeObject((Object*)pointer_atFn);
}

#endif // ARC_EXCLUDE_CTOOLS
/**
 * initSysModule
 * * Summary: Registers core system interaction functions such as 'exit', 'system', 'getenv', and platform-specific file tools.
 * * @param table: Pointer to the SymbolTable where system functions are registered.
 * * @returns: void (nothing).
 * * @note: Native functions 'access', 'unlink', and 'write' are wrapped inside 'ifndef _WIN32', meaning they are conditionally registered only on non-Windows (Unix-like) operating systems.
 */
void initSysModule(SymbolTable* table) {
  NativeFunction* exitFn = initNativeFunction("exit", builtIn_exit, 1, false);
  setTable(table, internIdentifier("exit", 4), VAL_OBJ((Object*)exitFn));
  freeObject((Object*)exitFn);

  NativeFunction* systemFn = initNativeFunction("system", builtIn_system, 1, false);
  setTable(table, internIdentifier("system", 6), VAL_OBJ((Object*)systemFn));
  freeObject((Object*)systemFn);

  NativeFunction* getenvFn = initNativeFunction("getenv", builtIn_getenv, 1, false);
  setTable(table, internIdentifier("getenv", 6), VAL_OBJ((Object*)getenvFn));
  freeObject((Object*)getenvFn);
  
  NativeFunction* get_osFn = initNativeFunction("get_os", builtIn_get_os, 0, false);
  setTable(table, internIdentifier("get_os", 6), VAL_OBJ((Object*)get_osFn));
  freeObject((Object*)get_osFn);

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
/**
 * initTimeModule
 * * Summary: Initializes the time module by registering performance tracking and sleep/delay utility functions.
 * * @param table: Pointer to the SymbolTable where time-related functions are registered.
 * * @returns: void (nothing).
 * * @note: Registers 'perf_counter' (high-resolution timer with 0 arguments) and 'sleep' (execution pause with 1 argument) into the runtime environment.
 */
void initTimeModule(SymbolTable* table) {
  NativeFunction* perf_counterFn = initNativeFunction("perf_counter", builtIn_perf_counter, 0, false);
  setTable(table, internIdentifier("perf_counter", 12), VAL_OBJ((Object*)perf_counterFn));
  freeObject((Object*)perf_counterFn);

  NativeFunction* sleepFn = initNativeFunction("sleep", builtIn_sleep, 1, false);
  setTable(table, internIdentifier("sleep", 5), VAL_OBJ((Object*)sleepFn));
  freeObject((Object*)sleepFn);
}
