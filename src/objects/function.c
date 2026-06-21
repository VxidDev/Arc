#include "../../include/object.h"
#include "../../include/utils.h"
#include "../../include/symbol-table.h"
#include "../../include/compiler.h"

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
  newFunc->chunk = func->chunk;
  newFunc->maxLocals = func->maxLocals;
  newFunc->base.isStatic = false;

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
  func->base.isStatic = false;

  func->body = node->body;
  func->chunk = NULL;
  func->maxLocals = (int)node->paramCount;
  
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

NativeFunction* initNativeFunction(char *name, NativeFunc func, size_t requiredArgCount, bool isVariadic) {
  NativeFunction* nativeFunc = poolAlloc(nativeFuncPool);

  if (!nativeFunc) return NULL;
  
  nativeFunc->name = stringDup(name);

  if (!nativeFunc->name) {
    poolFree(nativeFuncPool, nativeFunc);
    return NULL;
  }

  nativeFunc->base.type = OBJ_NATIVE_FUNCTION;
  nativeFunc->base.isStatic = true;

  nativeFunc->function = func;
  nativeFunc->requiredArgCount = requiredArgCount;
  nativeFunc->isVariadic = isVariadic;

  return nativeFunc;
}
