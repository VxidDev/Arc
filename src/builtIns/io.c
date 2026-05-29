#include "../../include/builtIns/io.h"

#include "../../include/utils.h"

#include <stdio.h>
#include <stdlib.h>

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

Object* builtIn_get_input(Object** args, size_t argCount) {
  if (argCount > 1) {
    char buf[256];

    snprintf(buf, sizeof(buf), "Expected at most 1 argument, received %zu.", argCount);

    return (Object*)initProgramError(buf);
  }

  Object* arg = args[0];

  if (arg->type != OBJ_STRING) {
    char buf[256];

    snprintf(buf, sizeof(buf), "Expected argument of type 'string', received '%s'.", typeofobj(arg));

    return (Object*)initProgramError(buf);
  }

  printf("%s", ((String*)arg)->value);
  
  size_t size = 0;
  size_t capacity = 256;

  char* buf = malloc(capacity);

  if (!buf) {
    return (Object*)initProgramError("Unable to read input from user.");
  }

  int c = 0;

  while ((c = getchar()) != EOF && c != '\n') {
    if (size >= capacity) {
      capacity *= 2;

      void* tmp = realloc(buf, capacity);

      if (!tmp) {
        free(buf);
        return (Object*)initProgramError("Unable to read input from user.");
      }

      buf = tmp;
    }

    buf[size++] = c;
  }

  buf[size] = '\0';

  Object* s = (Object*)initString(buf, size);
  free(buf);

  return s;
}
