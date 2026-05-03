#include "../../include/object.h"

#include <stdlib.h>
#include <math.h>

String *initString(char *value) {
  String* str = malloc(sizeof(String));

  if (!str) return NULL;

  str->value = value;
  str->base.type = OBJ_STRING;

  return str;
}

String *copyString(String *str) {
  if (!str) return NULL;

  return initString(str->value);
}
