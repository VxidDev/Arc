#include "../include/c-bridge.h"

#include "../include/object.h"
#include "../include/vm.h"

Object *callArcFunction(VM *vm, Function *func, Value *args, int argCount) {
  if (!func->chunk && func->body) {
    func->chunk = compileAST(func->body, vm->err, vm->filename, vm->sourcetext);
    if (func->chunk) func->maxLocals = func->chunk->maxLocals;
    func->body = NULL;
  }

  if (!func->chunk) {
    if (vm->err && !*vm->err)
      *vm->err = initRuntimeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Bytecode chunk is null. (internal error)", vm->sourcetext);

    return NULL;
  }

  if (argCount != (int)func->paramCount) {
    char buf[256];
    snprintf(buf, sizeof(buf), "Function \"%s\" expects %zu arguments, got %d.", func->name, func->paramCount, argCount);

    if (vm->err && !*vm->err)
      *vm->err = initRuntimeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, buf, vm->sourcetext);

    return NULL;
  }

  if (vm->frameTop >= VM_CALL_STACK_MAX || vm->localsTop + (int)func->maxLocals > VM_LOCALS_MAX) {
    if (vm->err && !*vm->err)
      *vm->err = initRuntimeError((Position){0,0,0}, (Position){0,0,0}, vm->filename, "Call stack or locals overflow.", vm->sourcetext);

    return NULL;
  }

  int callerFrameTop = vm->frameTop;
  int localsBase = vm->localsTop;

  SymbolTable *vars = vm->frames[vm->frameTop - 1].variables; // reuse caller's/global vars

  CallFrame *newFrame = &vm->frames[vm->frameTop];

  newFrame->chunk = func->chunk;
  newFrame->ip = func->chunk->code;
  newFrame->variables = vars;
  newFrame->tryStackTop = vm->tryStackTop;
  newFrame->localsBase = vm->localsTop;
  newFrame->localCount = func->maxLocals;
  newFrame->currentInstr = 0;
  newFrame->instance = NULL;
  newFrame->filename = vm->filename;
  newFrame->ownsChunk = false;

  int base = newFrame->localsBase;

  for (int i = 0; i < argCount; i++)
    vm->locals[base + i] = args[i];

  for (int i = argCount; i < func->maxLocals; i++)
    vm->locals[base + i] = VAL_UNDEF();

  vm->localsTop += func->maxLocals;
  vm->frameTop++;

  int savedExit = vm->exitFrameTop;
  vm->exitFrameTop = callerFrameTop + 1; // stop exactly when this call finishes
  Object *result = vmRun(vm);
  vm->exitFrameTop = savedExit;

  CallFrame *finishedFrame = &vm->frames[callerFrameTop];

  for (int i = 0; i < finishedFrame->localCount; i++)
    freeValue(vm->locals[finishedFrame->localsBase + i]);

  if (finishedFrame->ownsChunk)
    freeChunk(finishedFrame->chunk);

  vm->localsTop = localsBase;
  vm->frameTop = callerFrameTop;

  return result;
}
