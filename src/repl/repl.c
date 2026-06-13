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
  setTable(table, internIdentifier("print", 5), VAL_OBJ((Object*)printFn));

  NativeFunction* get_inputFn = initNativeFunction("get_input", builtIn_get_input, 1, true);
  setTable(table, internIdentifier("get_input", 9), VAL_OBJ((Object*)get_inputFn));

  NativeFunction* open_fileFn = initNativeFunction("open_file", builtIn_open_file, 2, false);
  setTable(table, internIdentifier("open_file", 9), VAL_OBJ((Object*)open_fileFn));

  NativeFunction* close_fileFn = initNativeFunction("close_file", builtIn_close_file, 1, false);
  setTable(table, internIdentifier("close_file", 10), VAL_OBJ((Object*)close_fileFn));

  NativeFunction* read_fileFn = initNativeFunction("read_file", builtIn_read_file, 1, false);
  setTable(table, internIdentifier("read_file", 9), VAL_OBJ((Object*)read_fileFn));

  NativeFunction* write_fileFn = initNativeFunction("write_file", builtIn_write_file, 2, false);
  setTable(table, internIdentifier("write_file", 10), VAL_OBJ((Object*)write_fileFn));

  // Errors

  NativeFunction* runtimeErrorFn = initNativeFunction("RuntimeError", builtIn_RuntimeError, 1, false);
  setTable(table, internIdentifier("RuntimeError", 12), VAL_OBJ((Object*)runtimeErrorFn));

  // Typing

  NativeFunction* typeofFn = initNativeFunction("typeof", builtIn_typeof, 1, false);
  setTable(table, internIdentifier("typeof", 6), VAL_OBJ((Object*)typeofFn));

  NativeFunction* to_intFn = initNativeFunction("to_int", builtIn_to_int, 1, false);
  setTable(table, internIdentifier("to_int", 6), VAL_OBJ((Object*)to_intFn));

  NativeFunction* to_stringFn = initNativeFunction("to_string", builtIn_to_string, 1, false);
  setTable(table, internIdentifier("to_string", 9), VAL_OBJ((Object*)to_stringFn));

  // Properties
  
  NativeFunction* len_ofFn = initNativeFunction("len_of", builtIn_len_of, 1, false);
  setTable(table, internIdentifier("len_of", 6), VAL_OBJ((Object*)len_ofFn));

  // String 
  
  NativeFunction* split_stringFn = initNativeFunction("split_string", builtIn_split_string, 2, false);
  setTable(table, internIdentifier("split_string", 12), VAL_OBJ((Object*)split_stringFn));

  // List 
  
  NativeFunction* append_listFn = initNativeFunction("append_list", builtIn_append_list, 2, false);
  setTable(table, internIdentifier("append_list", 11), VAL_OBJ((Object*)append_listFn));

  NativeFunction* rangeFn = initNativeFunction("range", builtIn_range, 2, false);
  setTable(table, internIdentifier("range", 5), VAL_OBJ((Object*)rangeFn));
}
