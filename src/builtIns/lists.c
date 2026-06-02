#include "../../include/builtIns/lists.h"
#include "../../include/utils.h"

#include <stdlib.h>

Object* builtIn_append_list(Object** args, size_t argCount) {
  (void)argCount;

  Object* arg0 = args[0];

  if (arg0->type != OBJ_LIST) {
    char buf[256];

    snprintf(buf, sizeof(buf), "Expected argument 1 to be object of type 'list', received '%s'.", typeofobj(arg0));

    return (Object*)initProgramError(buf);
  }

  List* listRaw = (List*)arg0;
  List* list = copyList(listRaw);

  if (!list) {
    return (Object*)initProgramError("Failed to append item to a list.");
  }

  if (list->size != list->capacity) {
    size_t tmpCap = list->capacity * 2;

    void* tmp = realloc(list->objects, tmpCap * sizeof(Object*));

    if (!tmp) {
      freeObject((Object*)list);
      return (Object*)initProgramError("Failed to append item to a list.");
    }

    list->capacity = tmpCap;
    list->objects = tmp;
  }

  list->objects[list->size++] = copyObject(args[1]);

  return (Object*)list;

}

Object* builtIn_range(Object** args, size_t argCount) {
  Object* startObj = args[0];
  Object* endObj = args[1];

  if (startObj->type != OBJ_NUMBER_INT) {
    char buf[256];

    snprintf(buf, sizeof(buf), "Expected argument 1 to be object of type 'int', received '%s'.", typeofobj(startObj));

    return (Object*)initProgramError(buf);
  }

  if (endObj->type != OBJ_NUMBER_INT) {
    char buf[256];

    snprintf(buf, sizeof(buf), "Expected argument 2 to be object of type 'int', received '%s'.", typeofobj(endObj));

    return (Object*)initProgramError(buf);
  }

  int64_t start = ((Number*)startObj)->as.i;
  int64_t end = ((Number*)endObj)->as.i;

  int64_t step = (start <= end) ? 1 : -1;
  int64_t count = llabs(end - start);

  Object** objects = malloc(sizeof(Object*) * count);

  for (int64_t i = 0; i < count; i++) {
    objects[i] = (Object*)initInt(start + i * step);
  }

  Object* ret =  (Object*)initList(objects, count, count);
  free(objects);

  return ret;
}
