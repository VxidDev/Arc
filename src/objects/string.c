#include "../../include/object.h"
#include "../../include/utils.h"

#include "../../include/mempool.h"
#include "../../include/memarena.h"

#include <stdlib.h>
#include <string.h>

String *initString(char *value, uint64_t len) {
  String* str = poolAlloc(stringPool);

  if (!str) return NULL;

  str->value = stringDup(value);
  str->base.type = OBJ_STRING;
  str->len = len;

  return str;
}

String *copyString(String *str) {
  if (!str) return NULL;
  return initString(str->value, str->len);
}

String *addString(const String *dest, const String *src) {
  if (!dest || !src || !dest->value || !src->value) return NULL;

  size_t lenDest = strlen(dest->value);
  size_t lenSrc  = strlen(src->value);

  char *newStr = arenaAlloc(stringArena, lenDest + lenSrc + 1);

  memcpy(newStr, dest->value, lenDest);
  memcpy(newStr + lenDest, src->value, lenSrc);
  newStr[lenDest + lenSrc] = '\0';

  String *res = poolAlloc(stringPool);
  res->value = newStr;
  res->base.type = OBJ_STRING;

  return res;
}

String *mulString(const String *dest, const Number *src) {
  if (!dest || !src || !dest->value || !src->as.i) return NULL;

  size_t len = strlen(dest->value);

  char *newStr = arenaAlloc(stringArena, len * src->as.i + 1);
  char *p = newStr;

  for (long int i = 0; i < src->as.i; i++) {
    memcpy(p, dest->value, len);
    p += len;
  }

  *p = '\0';

  String *res = poolAlloc(stringPool);
  res->value = newStr;
  res->base.type = OBJ_STRING;

  return res;
}
