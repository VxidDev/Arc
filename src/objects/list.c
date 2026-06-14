#include "../../include/object.h"

#include <stdlib.h>

List* initList(Object** list, uint64_t size, uint64_t capacity) {
  if (!list) return NULL;

  List* listobj = malloc(sizeof(List));

  if (!listobj) return NULL;

  listobj->base.type = OBJ_LIST;
  listobj->base.isStatic = false;

  listobj->size = size;
  listobj->capacity = capacity > 0 ? capacity : 1;
  
  listobj->objects = malloc(sizeof(Object*) * listobj->capacity);

  if (!listobj->objects) {
    free(listobj);
    return NULL;
  }

  for (uint64_t i = 0; i < listobj->size; i++) {
    listobj->objects[i] = copyObject(list[i]);
  }

  return listobj;
}

List* copyList(List* list) {
  if (!list) return NULL;
  return initList(list->objects, list->size, list->capacity);
}
