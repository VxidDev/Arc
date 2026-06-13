#include "../../include/object.h"

#include <stdlib.h>

Return* initReturn(Object* value) {
  if (!value) return NULL;

  Return* ret = malloc(sizeof(Return));

  if (!ret) return NULL;
  
  ret->base.type = OBJ_RETURN;
  ret->base.isStatic = false;

  ret->value = value;

  return ret;
}
