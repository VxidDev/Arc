#include "../../include/object.h"

#include <stdlib.h>

List* initList(Object** list, uint64_t size, uint64_t capacity) {
  if (!list) return NULL;

  List* listobj = malloc(sizeof(List));

  if (!listobj) return NULL;

  listobj->base.type = OBJ_LIST;
  listobj->size = size;
  listobj->capacity = capacity;
  
  listobj->objects = malloc(sizeof(Object*) * listobj->capacity);

  if (!listobj->objects) {
    free(listobj);
    return NULL;
  }

  for (uint64_t i = 0; i < listobj->size; i++) {
    listobj->objects[i] = copyObject(list[i]);
    freeObject(list[i]);
  }

  return listobj;
}

List* copyList(List* list) {
  if (!list) return NULL;

  List* newList = malloc(sizeof(List));

  if (!newList) return NULL;

  newList->base.type = OBJ_LIST;

  newList->size = list->size;
  newList->capacity = list->capacity;
  newList->objects = malloc(sizeof(Object*) * newList->capacity);

  if (!newList->objects) {
    free(newList);
    return NULL;
  }

  for (uint64_t i = 0; i < newList->size; i++) {
    newList->objects[i] = copyObject(list->objects[i]);
  }

  return newList;
}
