#include "../../include/object.h"

#include <stdlib.h>

List* initList(ListNode* list) {
  if (!list) return NULL;

  List* listobj = malloc(sizeof(List));

  if (!listobj) return NULL;

  listobj->base.type = OBJ_LIST;
  listobj->list = list;

  return listobj;
}

List* copyList(List* list) {
  if (!list) return NULL;

  List* newList = malloc(sizeof(List));

  if (!newList) return NULL;
  
  newList->list = malloc(sizeof(ListNode)); 

  if (!newList->list) {
    free(newList);
    return NULL;
  }

  newList->list->base.type = NODE_LIST;

  newList->base.type = OBJ_LIST;
  newList->list->size = list->list->size;
  newList->list->capacity = list->list->capacity;

  newList->list->objects = malloc(sizeof(ASTNode*) * newList->list->capacity);

  if (!newList->list->objects) {
    free(newList->list);
    free(newList);
    return NULL;
  }

  for (uint64_t i = 0; i < newList->list->size; i++) {
    newList->list->objects[i] = (ASTNode*)copyObject((Object*)list->list->objects[i]);
  }

  return newList;
}
