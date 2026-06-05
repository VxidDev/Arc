#include "../../include/object.h"
#include "../../include/utils.h"
#include "../../include/symbol-table.h"
#include "../../include/interpretator.h"

#include "../../include/memarena.h"

#include <stdlib.h>

Function* copyFunction(Function* func) {
  if (!func) return NULL;

  Function* newFunc = malloc(sizeof(Function));

  if (!newFunc) return NULL;

  newFunc->base.type = OBJ_FUNCTION;
  newFunc->body = func->body;

  newFunc->name = stringDup(func->name);

  if (!newFunc->name) { 
    free(newFunc); 
    return NULL; 
  }

  newFunc->params = arenaAlloc(stringArena, sizeof(char*) * func->paramCount);

  if (!newFunc->params && func->paramCount != 0) { 
    free(newFunc);
    return NULL; 
  }

  for (size_t i = 0; i < func->paramCount; i++) {
    newFunc->params[i] = stringDup(func->params[i]);

    if (!newFunc->params[i]) {
      free(newFunc);
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

  Function* func = malloc(sizeof(Function));

  if (!func) return NULL;

  func->base.type = OBJ_FUNCTION;
  func->body = node->body;
  
  func->name = stringDup(node->name);

  if (!func->name) { 
    free(func);
    return NULL;
  }

  func->params = arenaAlloc(stringArena, sizeof(char*) * node->paramCount);

  if (!func->params && node->paramCount != 0) { 
    free(func);
    return NULL;
  }

  for (size_t i = 0; i < node->paramCount; i++) {
    func->params[i] = stringDup(node->params[i]);

    if (!func->params[i]) {
      free(func);
      return NULL;
    }
  }

  func->paramCount = node->paramCount;

  return func;
}

FunctionCall *initFunctionCall(FunctionCallNode *node, Object* calleeObj, SymbolTable *globalTable, char *filename, Error** err) {
  if (!node || !globalTable || !calleeObj) return NULL;

  FunctionCall *call = malloc(sizeof(FunctionCall));
  if (!call) return NULL;
    
  call->base.type = OBJ_FUNCTION_CALL;
  call->argCount = node->argCount;

  call->args = malloc(sizeof(Object*) * call->argCount);

  if (!call->args) {
    free(call);
    return NULL;
  }

  if (calleeObj->type != OBJ_FUNCTION) {
    free(call->args);
    free(call);
    return NULL;
  }

  call->function = (Function*)calleeObj;

  call->env = createTable(64, globalTable);

  if (!call->env) {
    freeObject(calleeObj);
    free(call->args);
    free(call);

    return NULL;
  }

  for (size_t i = 0; i < call->argCount; i++) {
    Object *argVal = visitNode(node->args[i], filename, err, globalTable);

    if (!argVal) {
      for (size_t j = 0; j < i; j++) {
        freeObject(call->args[j]);
      }
  
      freeTable(call->env);
      freeObject(calleeObj);
      free(call->args);
      free(call);

      return NULL;
    }

    call->args[i] = argVal;

    if (i < call->function->paramCount) {
      setTable(call->env, call->function->params[i], argVal, true);
    }
  }

  return call;
}

NativeFunction* initNativeFunction(char *name, NativeFunc func, size_t requiredArgCount, bool isVariadic) {
  NativeFunction* nativeFunc = malloc(sizeof(NativeFunction));

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
