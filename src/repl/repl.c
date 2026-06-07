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

#include "../../include/mempool.h"
#include "../../include/memarena.h"

MemPool* numberPool = NULL;
MemPool* stringPool = NULL;
MemPool* nativeFuncPool = NULL;
MemPool* functionCallPool = NULL;
MemPool* symbolPool = NULL;
MemPool* symbolTablePool = NULL;
MemPool* functionPool = NULL;

Arena* parseArena = NULL;
Arena* stringArena = NULL;
Arena* objectArena = NULL;
Arena* symbolPtrArena = NULL;

void initMemPools() {
  numberPool = initPool(sizeof(Number));
  stringPool = initPool(sizeof(String));
  nativeFuncPool = initPool(sizeof(NativeFunction));
  functionCallPool = initPool(sizeof(FunctionCallNode));
  symbolPool = initPool(sizeof(Symbol));
  symbolTablePool = initPool(sizeof(SymbolTable));
  functionPool = initPool(sizeof(Function));
}

void initArenas() {
  parseArena = arenaCreate();
  stringArena = arenaCreate();
  objectArena = arenaCreate();
  symbolPtrArena = arenaCreate();
}

void freeArenas() {
  arenaDestroy(parseArena);
  arenaDestroy(stringArena);
  arenaDestroy(objectArena);
  arenaDestroy(symbolPtrArena);
}

void freeMemPools() {
  freePool(numberPool);
  freePool(stringPool);
  freePool(nativeFuncPool);
  freePool(functionCallPool);
  freePool(symbolPool);
  freePool(symbolTablePool);
  freePool(functionPool);
}

void registerBuiltins(SymbolTable* table) {
  // I/O

  NativeFunction* printFn = initNativeFunction("print", builtIn_print, 1, true);
  setTable(table, "print", (Object*)printFn, false);

  NativeFunction* get_inputFn = initNativeFunction("get_input", builtIn_get_input, 1, true);
  setTable(table, "get_input", (Object*)get_inputFn, false);

  NativeFunction* open_fileFn = initNativeFunction("open_file", builtIn_open_file, 2, false);
  setTable(table, "open_file", (Object*)open_fileFn, false);

  NativeFunction* close_fileFn = initNativeFunction("close_file", builtIn_close_file, 1, false);
  setTable(table, "close_file", (Object*)close_fileFn, false);

  NativeFunction* read_fileFn = initNativeFunction("read_file", builtIn_read_file, 1, false);
  setTable(table, "read_file", (Object*)read_fileFn, false);

  NativeFunction* write_fileFn = initNativeFunction("write_file", builtIn_write_file, 2, false);
  setTable(table, "write_file", (Object*)write_fileFn, false);

  // Errors

  NativeFunction* runtimeErrorFn = initNativeFunction("RuntimeError", builtIn_RuntimeError, 1, false);
  setTable(table, "RuntimeError", (Object*)runtimeErrorFn, false);

  // Typing

  NativeFunction* typeofFn = initNativeFunction("typeof", builtIn_typeof, 1, false);
  setTable(table, "typeof", (Object*)typeofFn, false);

  NativeFunction* to_intFn = initNativeFunction("to_int", builtIn_to_int, 1, false);
  setTable(table, "to_int", (Object*)to_intFn, false);

  NativeFunction* to_stringFn = initNativeFunction("to_string", builtIn_to_string, 1, false);
  setTable(table, "to_string", (Object*)to_stringFn, false);

  // Properties
  
  NativeFunction* len_ofFn = initNativeFunction("len_of", builtIn_len_of, 1, false);
  setTable(table, "len_of", (Object*)len_ofFn, false);

  // String 
  
  NativeFunction* split_stringFn = initNativeFunction("split_string", builtIn_split_string, 2, false);
  setTable(table, "split_string", (Object*)split_stringFn, false);

  // List 
  
  NativeFunction* append_listFn = initNativeFunction("append_list", builtIn_append_list, 2, false);
  setTable(table, "append_list", (Object*)append_listFn, false);

  NativeFunction* rangeFn = initNativeFunction("range", builtIn_range, 2, false);
  setTable(table, "range", (Object*)rangeFn, false);
}
