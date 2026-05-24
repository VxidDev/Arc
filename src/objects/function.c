#include "../../include/object.h"
#include "../../include/utils.h"
#include "../../include/symbol-table.h"
#include "../../include/interpretator.h"

#include <stdlib.h>

Function* copyFunction(Function* func) {
  if (!func) return NULL;

  Function* newFunc = malloc(sizeof(Function));

  if (!newFunc) return NULL;

  newFunc->base.type = OBJ_FUNCTION;
  newFunc->body = func->body;

  newFunc->name = stringDup(func->name);
  if (!newFunc->name) { free(newFunc); return NULL; }

  newFunc->params = malloc(sizeof(char*) * func->paramCount);
  if (!newFunc->params) { free(newFunc->name); free(newFunc); return NULL; }

  for (size_t i = 0; i < func->paramCount; i++) {
    newFunc->params[i] = stringDup(func->params[i]);
    if (!newFunc->params[i]) {
      for (size_t j = 0; j < i; j++) free(newFunc->params[j]);
      free(newFunc->params);
      free(newFunc->name);
      free(newFunc);
      return NULL;
    }
  }

  newFunc->paramCount = func->paramCount;

  return newFunc;
}

Function* initFunction(FunctionNode* node) {
  if (!node->body || !node->name || !node->params) return NULL;

  Function* func = malloc(sizeof(Function));

  if (!func) return NULL;

  func->base.type = OBJ_FUNCTION;
  func->body = node->body;
  
  func->name = stringDup(node->name);
  if (!func->name) { free(func); return NULL; }

  func->params = malloc(sizeof(char*) * node->paramCount);
  if (!func->params) { free(func->name); free(func); return NULL; }

  for (size_t i = 0; i < node->paramCount; i++) {
    func->params[i] = stringDup(node->params[i]);
    if (!func->params[i]) {
      for (size_t j = 0; j < i; j++) free(func->params[j]);
      free(func->params);
      free(func->name);
      free(func);
      return NULL;
    }
  }

  func->paramCount = node->paramCount;

  return func;
}

FunctionCall *initFunctionCall(FunctionCallNode *node, SymbolTable *globalTable, char *filename, Error** err) {
  if (!node || !globalTable) return NULL;

  FunctionCall *call = malloc(sizeof(FunctionCall));
  if (!call) return NULL;
    
  call->base.type = OBJ_FUNCTION_CALL;
  call->argCount = node->argCount;

  call->args = malloc(sizeof(Object*) * call->argCount);

  if (!call->args) {
    free(call);
    return NULL;
  }

  Object *calleeObj = visitNode(node->callee, filename, err, globalTable);

  if (!calleeObj || calleeObj->type != OBJ_FUNCTION) {
    if (calleeObj) freeObject(calleeObj);
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
      setTable(call->env, call->function->params[i], argVal);
    }
  }

  return call;
}
