#include "../../include/repl/repl.h"
#include "../../include/symbol-table.h"
#include <stdio.h>

#include "../../include/builtIns/io.h"
#include "../../include/builtIns/errors.h"
#include "../../include/builtIns/typing.h"
#include "../../include/builtIns/math.h"
#include "../../include/builtIns/properties.h"

void registerBuiltins(SymbolTable* table) {
  // I/O

  NativeFunction* printFn = initNativeFunction("print", builtIn_print, 1, true);
  setTable(table, "print", (Object*)printFn);
  
  freeObject((Object*)printFn);

  NativeFunction* get_inputFn = initNativeFunction("get_input", builtIn_get_input, 1, true);
  setTable(table, "get_input", (Object*)get_inputFn);

  freeObject((Object*)get_inputFn);

  // Errors

  NativeFunction* runtimeErrorFn = initNativeFunction("RuntimeError", builtIn_RuntimeError, 1, false);
  setTable(table, "RuntimeError", (Object*)runtimeErrorFn);

  freeObject((Object*)runtimeErrorFn);

  // Typing

  NativeFunction* typeofFn = initNativeFunction("typeof", builtIn_typeof, 1, false);
  setTable(table, "typeof", (Object*)typeofFn);

  freeObject((Object*)typeofFn);

  NativeFunction* to_intFn = initNativeFunction("to_int", builtIn_to_int, 1, false);
  setTable(table, "to_int", (Object*)to_intFn);

  freeObject((Object*)to_intFn);

  // Properties
  
  NativeFunction* len_ofFn = initNativeFunction("len_of", builtIn_len_of, 1, false);
  setTable(table, "len_of", (Object*)len_ofFn);

  freeObject((Object*)len_ofFn); 
}
