#include "../../include/builtIns/typing.h"
#include "../../include/object.h"
#include "../../include/utils.h"

#include "../../include/memarena.h"

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <inttypes.h>

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

Object* builtIn_to_string(Object** args, size_t argCount) {
  (void)argCount;

  Object* arg0 = args[0];

  switch (arg0->type) {
    case OBJ_STRING: return copyObject(arg0);

    case OBJ_NUMBER_INT: { 
      int64_t val = ((Number*)arg0)->as.i;

      size_t len = snprintf(NULL, 0, "%" PRId64, val);
      char *str = malloc(len + 1);

      snprintf(str, len + 1, "%" PRId64, val);
      return (Object*)noCopyInitString(str, len);
    }
    
    case OBJ_NUMBER_FLOAT: {
      double val = ((Number*)arg0)->as.f;

      size_t len = snprintf(NULL, 0, "%f", val);
      char *str = malloc(len + 1);

      snprintf(str, len + 1, "%f", val);
      return (Object*)noCopyInitString(str, len);
    }

    case OBJ_LIST: {
      List* list = (List*)arg0;

      size_t cap = 16;
      char* buf = malloc(cap);
      size_t len = 0;

      buf[len++] = '[';

      for (uint64_t i = 0; i < list->size; i++) {
        Object* elem = list->objects[i];

        Object* strObj = builtIn_to_string(&elem, 1);
        String* s = (String*)strObj;

        size_t slen = s->len;

        // grow buffer if needed
        if (len + slen + 2 >= cap) {
          while (len + slen + 2 >= cap)
            cap *= 2;

          char* newBuf = malloc(cap);
          memcpy(newBuf, buf, len);
          buf = newBuf;
        }

        memcpy(buf + len, s->value, slen);
        len += slen;

        if (i + 1 < list->size) {
          buf[len++] = ',';
          buf[len++] = ' ';
        }
      }

      buf[len++] = ']';
      buf[len] = '\0';

      return (Object*)noCopyInitString(buf, len);
    }

    default: break;
  }
  
  char buf[256];

  snprintf(buf, sizeof(buf), "Conversion from type '%s' to type 'string' is not yet available.", typeofobj(arg0));

  return (Object*)initProgramError(buf);
}
