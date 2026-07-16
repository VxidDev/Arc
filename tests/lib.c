#include "include/object.h"
#include "include/c-bridge.h"
#include "include/repl/repl.h"

#include <stdlib.h>
#include <string.h>

Object* run_fn(Object** args, size_t argCount) {
  (void)argCount;
  
  Function* fn = (Function*)args[0];
  List* argList = (List*)args[1];
  Value passedArgs[argList->size];

  for (size_t i = 0; i < argList->size; i++) {
    passedArgs[i] = VAL_OBJ(copyObject(argList->objects[i]));
  }

  return callArcFunction(vm, fn, passedArgs, argList->size);
}

Object* add(Object** args, size_t argCount) {
  (void)args;
  (void)argCount;

  return (Object*)initInt(((Number*)args[0])->as.i + ((Number*)args[1])->as.i);
}

int c_add(int x, int y) {
  return x + y;
}

char* yield_string() {
  char *s = malloc(6);
  memcpy(s, "hello", 6);

  return s;
}

int* yield_int() {
  int* i = malloc(sizeof(int));
  *i = 5;

  return i;
}
