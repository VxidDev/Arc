#ifndef VM_H
#define VM_H

#include "compiler.h"
#include "object.h"
#include "symbol-table.h"
#include "error.h"

#define VM_STACK_MAX 4096
#define VM_TRY_STACK_MAX 256
#define VM_CALL_STACK_MAX 8192


typedef struct {
  Chunk *chunk;
  uint8_t *ip;
  SymbolTable *variables;
  int tryStackTop;
} CallFrame;

typedef struct VM {
  CallFrame frames[VM_CALL_STACK_MAX];
  int frameTop;

  Object *stack[VM_STACK_MAX];
  int stackTop;

  uint8_t *tryStack[VM_TRY_STACK_MAX];
  int tryStackTop;

  Error **err;
  char *filename;
  char *sourcetext;
} VM;

VM *initVM(Chunk *chunk, SymbolTable *variables, Error **err, char *filename, char *sourcetext);
Object *vmRun(VM *vm);

#endif
