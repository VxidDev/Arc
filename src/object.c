#include "../include/object.h"
#include "../include/symbol-table.h"

#include <stddef.h>
#include <stdlib.h>

Object* copyObject(Object* obj) {
  if (!obj) return NULL;
  
  switch (obj->type) {
    case OBJ_NUMBER_INT:
      return (Object*)copyNumber((Number*)obj);
    case OBJ_NUMBER_FLOAT:
      return (Object*)copyNumber((Number*)obj);
    case OBJ_LIST:
      return (Object*)copyList((List*)obj);
    case OBJ_FUNCTION:
      return (Object*)copyFunction((Function*)obj);
    case OBJ_NATIVE_FUNCTION:
      return (Object*)copyNativeFunction((NativeFunction*)obj);
    case OBJ_STRING:
      return (Object*)copyString((String*)obj);
    default:
      return NULL;
  }
}

void freeObject(Object* obj) {
  if (!obj) return;

  if (obj->type == OBJ_NUMBER_INT || obj->type == OBJ_NUMBER_FLOAT) {
    free(obj);
  } else if (obj->type == OBJ_STRING) {
    String* str = (String*)obj;

    if (str->value) free(str->value);
    free(str); 
  } else if (obj->type == OBJ_LIST) { 
    List* list = (List*)obj;

    for (uint64_t i = 0; i < list->size; i++) freeObject(list->objects[i]);

    free(list->objects);
    free(list);
  } else if (obj->type == OBJ_FUNCTION) {
    Function* func = (Function*)obj;

    for (size_t i = 0; i < func->paramCount; i++) {
      free(func->params[i]);
    }

    free(func->params);
    free(func->name);

    free(func);
  } else if (obj->type == OBJ_NATIVE_FUNCTION) {
    NativeFunction* nativeFunc = (NativeFunction*)obj;
    free(nativeFunc->name);
    free(nativeFunc);
  } else {
    FunctionCall* fncall = (FunctionCall*)obj;

    for (size_t i = 0; i < fncall->argCount; i++) {
      freeObject(fncall->args[i]);
    }

    free(fncall->args);

    freeObject((Object*)fncall->function);
    freeTable(fncall->env);

    free(fncall);
  }

  return;
}