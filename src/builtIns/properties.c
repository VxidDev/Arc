#include "../../include/builtIns/properties.h"
#include "../../include/utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Object* builtIn_len_of(Object** args, size_t argCount) {
  (void)argCount;

  Object* obj = args[0];

  switch (obj->type) {
    case OBJ_STRING: {
      String* string = (String*)obj;

      size_t len = strlen(string->value);
      return (Object*)initInt(len);
    }

    case OBJ_LIST: {
      List* list = (List*)obj;

      return (Object*)initInt(list->size);
    }

    default: {
      char buf[256];

      snprintf(buf, sizeof(buf), "Object of type '%s' has no length.", typeofobj(obj));

      return (Object*)initProgramError(buf);
    }
  }
}
