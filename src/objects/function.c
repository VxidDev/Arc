#include "../../include/object.h"
#include "../../include/utils.h"
#include "../../include/symbol-table.h"
#include "../../include/interpretator.h"

#include "../../include/repl/repl.h"

#include "../../include/memarena.h"
#include "../../include/mempool.h"

#include <stdlib.h>

Function* copyFunction(Function* func) {
  if (!func) return NULL;

  Function* newFunc = poolAlloc(functionPool);

  if (!newFunc) return NULL;

  newFunc->base.type = OBJ_FUNCTION;
  newFunc->body = func->body;

  newFunc->name = stringDup(func->name);

  if (!newFunc->name) { 
    return NULL; 
  }

  newFunc->params = arenaAlloc(stringArena, sizeof(char*) * func->paramCount);

  if (!newFunc->params && func->paramCount != 0) { 
    return NULL; 
  }

  for (size_t i = 0; i < func->paramCount; i++) {
    newFunc->params[i] = stringDup(func->params[i]);

    if (!newFunc->params[i]) {
      return NULL;
    }
  }

  newFunc->paramCount = func->paramCount;

  return newFunc;
}

NativeFunction* copyNativeFunction(NativeFunction* func) {
  if (!func) return NULL;
  return initNativeFunction(func->name, func->function, func->requiredArgCount, func->isVariadic);
}

Function* initFunction(FunctionNode* node) {
  if (!node->body || !node->name || (!node->params && node->paramCount != 0)) return NULL;

  Function* func = poolAlloc(functionPool);

  if (!func) return NULL;

  func->base.type = OBJ_FUNCTION;
  func->body = node->body;
  
  func->name = stringDup(node->name);

  if (!func->name) { 
    return NULL;
  }

  func->params = arenaAlloc(stringArena, sizeof(char*) * node->paramCount);

  if (!func->params && node->paramCount != 0) { 
    return NULL;
  }

  for (size_t i = 0; i < node->paramCount; i++) {
    func->params[i] = stringDup(node->params[i]);

    if (!func->params[i]) {
      return NULL;
    }
  }

  func->paramCount = node->paramCount;

  return func;
}

FunctionCall *initFunctionCall(FunctionCallNode *node, Object* calleeObj, Interpretator* ctx) {
  if (!node || !ctx || !calleeObj) return NULL;

  FunctionCall *call = poolAlloc(functionCallPool);
  if (!call) return NULL;
    
  call->base.type = OBJ_FUNCTION_CALL;
  call->argCount = node->argCount;
  
  /*
  if (call->argCount > 0) {
    call->args = arenaAlloc(objectArena, sizeof(Object*) * call->argCount);

    if (!call->args) {
      if (_DEBUG) printf("[debug] Failed to arena-allocate memory for an array of Object* @ initFunctionCall - line approx. 103.\n");
      return NULL;
    }
  } else {
    call->args = NULL;
    if (_DEBUG) printf("[debug] No arguments passed, skipping arena-allocating memory. @ initFunctionCall - line approx. 111.\n");
  }
  */

  call->args = NULL; // TODO: clean up FunctionCall object

  if (calleeObj->type != OBJ_FUNCTION) {
    free(call->args);
    return NULL;
  }

  call->function = (Function*)calleeObj;

  call->env = createTable(64, ctx->variables);

  if (!call->env) {
    return NULL;
  }

  for (size_t i = 0; i < call->argCount; i++) {
    Object *argVal = node->args[i]->visit(node->args[i], ctx);

    if (!argVal) {
      if (_DEBUG) printf("[debug] Failed to evaluate function arguments @ initFunctionCall - line approx. 123.\n");

      freeTable(call->env); 
      return NULL;
    }

    if (i < call->function->paramCount) {
      setTable(call->env, call->function->params[i], argVal, false);
    }
  }

  return call;
}

NativeFunction* initNativeFunction(char *name, NativeFunc func, size_t requiredArgCount, bool isVariadic) {
  NativeFunction* nativeFunc = poolAlloc(nativeFuncPool);

  if (!nativeFunc) return NULL;
  
  nativeFunc->name = stringDup(name);

  if (!nativeFunc->name) {
    free(nativeFunc);
    return NULL;
  }

  nativeFunc->base.type = OBJ_NATIVE_FUNCTION;
  nativeFunc->function = func;
  nativeFunc->requiredArgCount = requiredArgCount;
  nativeFunc->isVariadic = isVariadic;

  return nativeFunc;
}
