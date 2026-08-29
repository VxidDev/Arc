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
  // Shared chunk – freeObject must not free it from copies.
  // Chunk is owned by the original function (or arena); copies are non-owning.
  // freeObject for FUNCTION skips chunk free if chunk is shared via isStatic flag trick,
  // but to avoid double-free we ensure copies do not own chunk by clearing pointer
  // after marking original static ownership is handled in freeObject via refCount.
  // Simpler: copies share chunk but freeObject checks before freeing – we make copy's
  // chunk pointer non-owning by not freeing in copy path: freeObject will free only
  // if the object is the owner. We use isStatic=false but store chunk; freeObject
  // will attempt to free – so we must not share. Instead duplicate view: set copy's
  // chunk to func->chunk but mark that freeObject should skip free by using
  // a separate flag – we set copy's chunk to NULL if shared and let runtime re-compile.
  // For now share pointer but prevent double-free via freeing only if refCount logic:
  // freeObject for FUNCTION does poolFree without refCount, so sharing is unsafe.
  // Fix: copy shares chunk pointer but we null it on copy and lazy-compile will re-create.
  newFunc->chunk = NULL;
  newFunc->maxLocals = func->maxLocals;
  newFunc->base.isStatic = false;

  newFunc->name = stringDup(func->name);

  if (!newFunc->name) { 
    poolFree(functionPool, newFunc);
    return NULL; 
  }

  if (func->paramCount == 0) {
    newFunc->params = NULL;
  } else {
    newFunc->params = arenaAlloc(stringArena, sizeof(char*) * func->paramCount);
    if (!newFunc->params) { 
      poolFree(functionPool, newFunc);
      return NULL; 
    }
  }

  for (size_t i = 0; i < func->paramCount; i++) {
    newFunc->params[i] = stringDup(func->params[i]);

    if (!newFunc->params[i]) {
      poolFree(functionPool, newFunc);
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
