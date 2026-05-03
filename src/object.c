#include "../include/object.h"

#include <stddef.h>

Object* copyObject(Object* obj) {
  if (!obj) return NULL;

  if (obj->type == OBJ_NUMBER_INT || obj->type == OBJ_NUMBER_FLOAT) {
    return (Object*)copyNumber((Number*)obj);
  } else {
    return (Object*)copyString((String*)obj); 
  }

  return NULL;
}
