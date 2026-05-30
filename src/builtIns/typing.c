#include "../../include/builtIns/typing.h"
#include "../../include/object.h"
#include "../../include/utils.h"

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

Object* builtIn_typeof(Object** args, size_t argCount) {
  (void)argCount;

  char* objName = typeofobj(args[0]);
  return (Object*)initString(objName, strlen(objName));
}

Object* builtIn_to_int(Object** args, size_t argCount) {
  (void)argCount;

  Object* obj = args[0];
  
  bool isNum = (obj->type == OBJ_NUMBER_FLOAT || obj->type == OBJ_NUMBER_INT);

  if (!isNum && obj->type != OBJ_STRING) {
    char buf[256];

    snprintf(buf, sizeof(buf), "Expected 'float', 'int' or 'string' object, received '%s'.", typeofobj(obj));

    return (Object*)initProgramError(buf);
  }
  
  if (isNum) {
    Number* num = (Number*)obj;
    
    if (num->base.type == OBJ_NUMBER_INT) {
      return (Object*)initInt(num->as.i);
    }

    return (Object*)initInt(num->as.f);
  }

  char *s = ((String*)obj)->value;
  char* end;

  errno = 0;

  int64_t val = (int64_t)strtoll(s, &end, 10);

  if (errno == ERANGE) {
    return (Object*)initProgramError("Numeric value out of range.");
  }

  if (*end != '\0') {
    return (Object*)initProgramError("Invalid numeric value.");
  }

  return (Object*)initInt(val);
}
