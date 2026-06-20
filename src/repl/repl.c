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
MemPool* instancePool = NULL;

Arena* parseArena = NULL;
Arena* stringArena = NULL;
Arena* objectArena = NULL;
Arena* symbolPtrArena = NULL;
Arena* poolArena = NULL;

void initMemPools() {
  numberPool = initPool(sizeof(Number));
  stringPool = initPool(sizeof(String));
  nativeFuncPool = initPool(sizeof(NativeFunction));
  functionCallPool = initPool(sizeof(FunctionCallNode));
  symbolPool = initPool(sizeof(Symbol));
  symbolTablePool = initPool(sizeof(SymbolTable));
  functionPool = initPool(sizeof(Function));
  instancePool = initPool(sizeof(Instance));
}

void initArenas() {
  parseArena = arenaCreate();
  stringArena = arenaCreate();
  objectArena = arenaCreate();
  symbolPtrArena = arenaCreate();
  poolArena = arenaCreate();
}

void freeArenas() {
  arenaDestroy(parseArena);
  arenaDestroy(stringArena);
  arenaDestroy(objectArena);
  arenaDestroy(symbolPtrArena);
  arenaDestroy(poolArena);
}

void freeMemPools() {
  freePool(numberPool);
  freePool(stringPool);
  freePool(nativeFuncPool);
  freePool(functionCallPool);
  freePool(symbolPool);
  freePool(symbolTablePool);
  freePool(functionPool);
  freePool(instancePool);
}

void registerBuiltins(SymbolTable* table) {
  // I/O

  NativeFunction* printFn = initNativeFunction("print", builtIn_print, 1, true);
  setTable(table, internIdentifier("print", 5), VAL_OBJ((Object*)printFn));
  freeObject((Object*)printFn);

  NativeFunction* get_inputFn = initNativeFunction("get_input", builtIn_get_input, 1, true);
  setTable(table, internIdentifier("get_input", 9), VAL_OBJ((Object*)get_inputFn));
  freeObject((Object*)get_inputFn);

  NativeFunction* open_fileFn = initNativeFunction("open_file", builtIn_open_file, 2, false);
  setTable(table, internIdentifier("open_file", 9), VAL_OBJ((Object*)open_fileFn));
  freeObject((Object*)open_fileFn);

  NativeFunction* close_fileFn = initNativeFunction("close_file", builtIn_close_file, 1, false);
  setTable(table, internIdentifier("close_file", 10), VAL_OBJ((Object*)close_fileFn));
  freeObject((Object*)close_fileFn);

  NativeFunction* read_fileFn = initNativeFunction("read_file", builtIn_read_file, 1, false);
  setTable(table, internIdentifier("read_file", 9), VAL_OBJ((Object*)read_fileFn));
  freeObject((Object*)read_fileFn);

  NativeFunction* write_fileFn = initNativeFunction("write_file", builtIn_write_file, 2, false);
  setTable(table, internIdentifier("write_file", 10), VAL_OBJ((Object*)write_fileFn));
  freeObject((Object*)write_fileFn);
    
  NativeFunction* stream_read_charFn = initNativeFunction("stream_read_char", builtIn_stream_read_char, 1, false);
  setTable(table, internIdentifier("stream_read_char", 16), VAL_OBJ((Object*)stream_read_charFn));
  freeObject((Object*)stream_read_charFn);
  
  NativeFunction* stream_tellFn = initNativeFunction("stream_tell", builtIn_stream_tell, 1, false);
  setTable(table, internIdentifier("stream_tell", 11), VAL_OBJ((Object*)stream_tellFn));
  freeObject((Object*)stream_tellFn);

  NativeFunction* stream_seekFn = initNativeFunction("stream_seek", builtIn_stream_seek, 3, false);
  setTable(table, internIdentifier("stream_seek", 1), VAL_OBJ((Object*)stream_seekFn));
  freeObject((Object*)stream_seekFn);

  // Errors

  NativeFunction* runtimeErrorFn = initNativeFunction("RuntimeError", builtIn_RuntimeError, 1, false);
  setTable(table, internIdentifier("RuntimeError", 12), VAL_OBJ((Object*)runtimeErrorFn));
  freeObject((Object*)runtimeErrorFn);

  // Typing

  NativeFunction* typeofFn = initNativeFunction("typeof", builtIn_typeof, 1, false);
  setTable(table, internIdentifier("typeof", 6), VAL_OBJ((Object*)typeofFn));
  freeObject((Object*)typeofFn);

  NativeFunction* to_intFn = initNativeFunction("to_int", builtIn_to_int, 1, false);
  setTable(table, internIdentifier("to_int", 6), VAL_OBJ((Object*)to_intFn));
  freeObject((Object*)to_intFn);

  NativeFunction* to_stringFn = initNativeFunction("to_string", builtIn_to_string, 1, false);
  setTable(table, internIdentifier("to_string", 9), VAL_OBJ((Object*)to_stringFn));
  freeObject((Object*)to_stringFn);

  // Properties
  
  NativeFunction* len_ofFn = initNativeFunction("len_of", builtIn_len_of, 1, false);
  setTable(table, internIdentifier("len_of", 6), VAL_OBJ((Object*)len_ofFn));
  freeObject((Object*)len_ofFn);

  NativeFunction* is_digitFn = initNativeFunction("is_digit", builtIn_is_digit, 1, false);
  setTable(table, internIdentifier("is_digit", 8), VAL_OBJ((Object*)is_digitFn));
  freeObject((Object*)is_digitFn);

  // String 
  
  NativeFunction* split_stringFn = initNativeFunction("split_string", builtIn_split_string, 2, false);
  setTable(table, internIdentifier("split_string", 12), VAL_OBJ((Object*)split_stringFn));
  freeObject((Object*)split_stringFn);

  NativeFunction* append_charFn = initNativeFunction("append_char", builtIn_append_char, 2, false);
  setTable(table, internIdentifier("append_char", 11), VAL_OBJ((Object*)append_charFn));
  freeObject((Object*)append_charFn);

  NativeFunction* string_bufferFn = initNativeFunction("string_buffer", builtIn_string_buffer, 0, false);
  setTable(table, internIdentifier("string_buffer", 13), VAL_OBJ((Object*)string_bufferFn));
  freeObject((Object*)string_bufferFn);

  NativeFunction* string_finishFn = initNativeFunction("string_finish", builtIn_string_finish, 1, false);
  setTable(table, internIdentifier("string_finish", 13), VAL_OBJ((Object*)string_finishFn));
  freeObject((Object*)string_finishFn);

  // List 
  
  NativeFunction* append_listFn = initNativeFunction("append_list", builtIn_append_list, 2, false);
  setTable(table, internIdentifier("append_list", 11), VAL_OBJ((Object*)append_listFn));
  freeObject((Object*)append_listFn);
    
  NativeFunction* pop_listFn = initNativeFunction("pop_list", builtIn_pop_list, 1, false);
  setTable(table, internIdentifier("pop_list", 8), VAL_OBJ((Object*)pop_listFn));
  freeObject((Object*)pop_listFn);

  NativeFunction* rangeFn = initNativeFunction("range", builtIn_range, 2, false);
  setTable(table, internIdentifier("range", 5), VAL_OBJ((Object*)rangeFn));
  freeObject((Object*)rangeFn);
}
