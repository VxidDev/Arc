#include "../../include/builtIns/errors.h"

#include <stdio.h>

Object* builtIn_RuntimeError(Object** args, size_t argCount) {
  (void)argCount;

  Object* detailsObj = args[0];

  if (detailsObj->type != OBJ_STRING) {
    return (Object*)initProgramError("Argument 'details' must be of type 'string'.");
  }

  return (Object*)initProgramError(((String*)detailsObj)->value);
}
