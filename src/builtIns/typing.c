#include "../../include/builtIns/typing.h"
#include "../../include/object.h"
#include "../../include/utils.h"

#include <string.h>
#include <stdio.h>

Object* builtIn_typeof(Object** args, size_t argCount) {
  (void)argCount;

  char* objName = typeofobj(args[0]);
  return (Object*)initString(objName, strlen(objName));
}

Object* builtIn_to_int(Object** args, size_t argCount) {
  (void)argCount;

  Object* obj = args[0];

  if (obj->type != OBJ_NUMBER_FLOAT && obj->type != OBJ_NUMBER_INT) {
    char buf[256];

    snprintf(buf, sizeof(buf), "Expected 'float' or 'int' type, received '%s'.", typeofobj(obj));

    return (Object*)initProgramError(buf);
  }

  Number* num = (Number*)obj;
  
  if (num->base.type == OBJ_NUMBER_INT) {
    return (Object*)initInt(num->as.i);
  }

  return (Object*)initInt(num->as.f);
}
