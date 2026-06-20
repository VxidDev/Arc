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
      return (Object*)initInt(string->len);
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

Object* builtIn_is_digit(Object** args, size_t argCount) {
  (void)argCount;

  Object* obj = args[0];

  if (obj->type != OBJ_STRING) {
    char buf[256];
    snprintf(buf, sizeof(buf), "Expected argument 1 to be object of type 'string', received '%s'.", typeofobj(obj));
    return (Object*)initProgramError(buf);
  }

  String* s = (String*)obj;
  char c = s->value[0];

  for (uint64_t i = 0; i < s->len; i++) {
    if (!(c >= '0' && c <= '9')) return (Object*)initInt(0); 
  }

  return (Object*)initInt(1);
}
