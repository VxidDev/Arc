#include "../include/object.h"

#include <stddef.h>
#include <stdlib.h>

Object* copyObject(Object* obj) {
  if (!obj) return NULL;

  if (obj->type == OBJ_NUMBER_INT || obj->type == OBJ_NUMBER_FLOAT) {
    return (Object*)copyNumber((Number*)obj);
  } else if (obj->type == OBJ_STRING) {
    return (Object*)copyString((String*)obj); 
  } else {
    return (Object*)copyList((List*)obj);
  }

  return NULL;
}

void freeObject(Object* obj) {
  if (!obj) return;

  if (obj->type == OBJ_NUMBER_INT || obj->type == OBJ_NUMBER_FLOAT) {
    free(obj);
  } else if (obj->type == OBJ_STRING) {
    String* str = (String*)obj;

    if (str->value) free(str->value);
    free(str); 
  } else { 
    free(obj);
  } 

  return;
}
