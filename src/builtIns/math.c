#include "../../include/builtIns/math.h"

Object* builtIn_truncate(Object** args, size_t argCount) {
  (void)args;
  (void)argCount;

  Object* arg = args[0];

  if (arg->type != OBJ_NUMBER_FLOAT) {
    return (Object*)initProgramError("Expected argument of type 'float', received 'int'.");
  }

  return (Object*)initInt((int32_t)((Number*)arg)->as.f);
}
