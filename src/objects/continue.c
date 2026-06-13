#include "../../include/object.h"

#include <stdlib.h>

Continue* initContinue() {
  Continue* obj = malloc(sizeof(Continue));

  if (!obj) return NULL;

  obj->base.type = OBJ_CONTINUE;
  obj->base.isStatic = false;

  return obj;
}
