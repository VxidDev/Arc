#include "../../include/object.h"

#include <stdlib.h>

Break* initBreak() {
  Break* obj = malloc(sizeof(Break));

  if (!obj) return NULL;

  obj->base.type = OBJ_BREAK;
  obj->base.isStatic = false;

  return obj;
}
