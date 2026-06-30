#ifndef VM_H
#define VM_H

#include "compiler.h"
#include "object.h"
#include "symbol-table.h"
#include "error.h"
#include "value.h"

#define VM_STACK_MAX 4096
#define VM_TRY_STACK_MAX 256
#define VM_CALL_STACK_MAX 8192

#define VM_LOCALS_MAX 65536

typedef struct {
  Chunk *chunk;
  uint8_t *ip;
  SymbolTable *variables;
  int tryStackTop;

  int localsBase;
  int localCount;

  char *filename;

  uint32_t currentInstr;
  Object* instance;

  bool ownsChunk;
} CallFrame;

typedef struct {
  uint8_t *ip;
  int frameTop;
  int stackTop;
} TryFrame;

typedef struct VM {
  CallFrame frames[VM_CALL_STACK_MAX];
  int frameTop;

  Value stack[VM_STACK_MAX];
  int stackTop;

  Value locals[VM_LOCALS_MAX];
  int localsTop;

  TryFrame tryStack[VM_TRY_STACK_MAX];
  int tryStackTop;

  Error **err;
  char *filename;
  char *sourcetext;
} VM;

VM *initVM(Chunk *chunk, SymbolTable *variables, Error **err, char *filename, char *sourcetext);
void deinitVM(VM *vm);
Object *vmRun(VM *vm);

#endif
