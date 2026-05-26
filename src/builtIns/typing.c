#include "../../include/builtIns/typing.h"
#include "../../include/object.h"

#include <string.h>

Object* builtIn_typeof(Object** args, size_t argCount) {
  Object* obj = args[0];

  char* objName = NULL;

  switch (obj->type) {
    case OBJ_NUMBER_INT: objName = "int"; break;
    case OBJ_NUMBER_FLOAT: objName = "float"; break;
    case OBJ_STRING: objName = "string"; break;
    case OBJ_ERROR: objName = "error"; break;
    case OBJ_LIST: objName = "list"; break;
    case OBJ_FUNCTION: objName = "function"; break;
    default: objName = "object"; break;
  }

  return (Object*)initString(objName, strlen(objName));
}
