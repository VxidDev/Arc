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
