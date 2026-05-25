#include "../../include/builtIns/print.h"

#include <stdio.h>

Object* builtIn_print(Object** args, size_t argCount) {
  for (size_t i = 0; i < argCount; i++) {
    Object* obj = args[i];

    switch (obj->type) {
      case OBJ_NUMBER_INT:
        printf("%ld", ((Number*)obj)->as.i);
        break;

      case OBJ_NUMBER_FLOAT:
        printf("%f", ((Number*)obj)->as.f);
        break;

      case OBJ_STRING:
        printf("%s", ((String*)obj)->value);
        break;

      default:
        printf("<object>");
        break;
    }

    if (i + 1 < argCount) printf(" ");
  }

  printf("\n");

  return (Object*)initInt(1);
}
