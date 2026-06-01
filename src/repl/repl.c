#include "../../include/repl/repl.h"
#include "../../include/symbol-table.h"
#include <stdio.h>

#include "../../include/builtIns/io.h"
#include "../../include/builtIns/errors.h"
#include "../../include/builtIns/typing.h"
#include "../../include/builtIns/math.h"
#include "../../include/builtIns/properties.h"
#include "../../include/builtIns/string.h"
#include "../../include/builtIns/lists.h"

void registerBuiltins(SymbolTable* table) {
  // I/O

  NativeFunction* printFn = initNativeFunction("print", builtIn_print, 1, true);
  setTable(table, "print", (Object*)printFn);
  
  freeObject((Object*)printFn);

  NativeFunction* get_inputFn = initNativeFunction("get_input", builtIn_get_input, 1, true);
  setTable(table, "get_input", (Object*)get_inputFn);

  freeObject((Object*)get_inputFn);

  NativeFunction* open_fileFn = initNativeFunction("open_file", builtIn_open_file, 2, false);
  setTable(table, "open_file", (Object*)open_fileFn);

  freeObject((Object*)open_fileFn);

  NativeFunction* close_fileFn = initNativeFunction("close_file", builtIn_close_file, 1, false);
  setTable(table, "close_file", (Object*)close_fileFn);

  freeObject((Object*)close_fileFn);

  NativeFunction* read_fileFn = initNativeFunction("read_file", builtIn_read_file, 1, false);
  setTable(table, "read_file", (Object*)read_fileFn);

  freeObject((Object*)read_fileFn);

  NativeFunction* write_fileFn = initNativeFunction("write_file", builtIn_write_file, 2, false);
  setTable(table, "write_file", (Object*)write_fileFn);

  freeObject((Object*)write_fileFn);

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

  // String 
  
  NativeFunction* split_stringFn = initNativeFunction("split_string", builtIn_split_string, 2, false);
  setTable(table, "split_string", (Object*)split_stringFn);

  freeObject((Object*)split_stringFn);

  // List 
  
  NativeFunction* append_listFn = initNativeFunction("append_list", builtIn_append_list, 2, false);
  setTable(table, "append_list", (Object*)append_listFn);

  freeObject((Object*)append_listFn);
}
