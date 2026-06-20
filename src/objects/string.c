#include "../../include/object.h"
#include "../../include/utils.h"

#include "../../include/mempool.h"
#include "../../include/memarena.h"

#include <stdlib.h>
#include <string.h>

#include "../../include/object.h"
#include "../../include/utils.h"

#include "../../include/mempool.h"
#include "../../include/memarena.h"

#include <stdlib.h>
#include <string.h>

typedef struct {
  char **entries;
  size_t count;
  size_t capacity;
} IdentifierTable;

static IdentifierTable globalIdentifierTable;

static char* identifierTableFind(IdentifierTable* table, const char* value, uint64_t len, uint32_t hash) {
  if (table->capacity == 0) return NULL;

  uint32_t index = hash & (table->capacity - 1);
  for (;;) {
    char* entry = table->entries[index];
    if (entry == NULL) return NULL;
    if (strlen(entry) == len && memcmp(entry, value, len) == 0) {
      return entry;
    }

    index = (index + 1) & (table->capacity - 1);
  }
}

static void identifierTableResize(IdentifierTable* table, size_t newCapacity) {
  char** entries = arenaAlloc(objectArena, sizeof(char*) * newCapacity);
  for (size_t i = 0; i < newCapacity; i++) {
    entries[i] = NULL;
  }

  for (size_t i = 0; i < table->capacity; i++) {
    char* entry = table->entries[i];
    if (entry == NULL) continue;

    uint32_t hash = hashStr(entry, strlen(entry));
    uint32_t index = hash & (newCapacity - 1);
    for (;;) {
      if (entries[index] == NULL) {
        entries[index] = entry;
        break;
      }
      index = (index + 1) & (newCapacity - 1);
    }
  }

  table->entries = entries;
  table->capacity = newCapacity;
}

static void identifierTableAdd(IdentifierTable* table, char* identifier, uint32_t hash) {
  if (table->count + 1 > table->capacity * 0.75) {
    size_t newCapacity = table->capacity < 8 ? 8 : table->capacity * 2;
    identifierTableResize(table, newCapacity);
  }

  uint32_t index = hash & (table->capacity - 1);
  for (;;) {
    char* entry = table->entries[index];
    if (entry == NULL) {
      table->entries[index] = identifier;
      table->count++;
      return;
    }

    index = (index + 1) & (table->capacity - 1);
  }
}

char* internIdentifier(const char* value, uint64_t len) {
  uint32_t hash = hashStr(value, len);
  char* interned = identifierTableFind(&globalIdentifierTable, value, len, hash);
  if (interned) return interned;

  char* identifier = stringDup(value);
  identifierTableAdd(&globalIdentifierTable, identifier, hash);
  return identifier;
}

String *initStringBuffer(size_t initialCapacity) {
  String* str = poolAlloc(stringPool);
  if (!str) return NULL;

  size_t cap = initialCapacity ? initialCapacity : 16;
  str->value = malloc(cap + 1);

  if (!str->value) {
    poolFree(stringPool, str);
    return NULL;
  }

  str->value[0] = '\0';
  str->base.type = OBJ_STRING;
  str->base.isStatic = true;  
  str->len = 0;
  str->capacity = cap;
  str->isBuffer = true;

  return str;
}

String *initString(char *value, uint64_t len) {
  String* str = poolAlloc(stringPool);

  if (!str) return NULL;

  str->value = malloc(len + 1);

  if (!str->value) {
    poolFree(stringPool, str);
    return NULL;
  }
  
  memcpy(str->value, value, len);
  str->value[len] = '\0';
  
  str->base.type = OBJ_STRING;
  str->base.isStatic = false;

  str->len = len;
  str->capacity = len;
  str->isBuffer = false;

  return str;
}

String *noCopyInitString(char *value, uint64_t len) {
  String* str = poolAlloc(stringPool);

  if (!str) {
    free(value);
    return NULL;
  }

  str->value = value;
 
  str->base.type = OBJ_STRING;
  str->base.isStatic = false;

  str->len = len;
  str->capacity = len;
  str->isBuffer = false;

  return str;
}

String *initStringConst(char *value, uint64_t len) {
  String* str = poolAlloc(stringPool);

  if (!str) return NULL;

  str->value = value;
  str->base.type = OBJ_STRING;
  str->base.isStatic = true;

  str->len = len;
  str->capacity = len;
  str->isBuffer = false;

  return str;
}

String *copyString(String *str) {
  if (!str) return NULL;
  return initString(str->value, str->len);
}

String *addString(String *dest, const String *src) {
  if (!dest || !src) return NULL;

  size_t lenDest = dest->len;
  size_t lenSrc = src->len;
  size_t total = lenDest + lenSrc;
 
  if (total + 1 > dest->capacity) {
    String *res = poolAlloc(stringPool);
    if (!res) return NULL;
    
    uint64_t newCap = total * 2;
    char *buf = malloc(newCap + 1);

    if (!buf) {
      poolFree(stringPool, res);
      return NULL;
    }

    memcpy(buf, dest->value, lenDest);
    memcpy(buf + lenDest, src->value, lenSrc);
    buf[total] = '\0';

    res->value = buf;
    res->base.type = OBJ_STRING;
    res->base.isStatic = false;
    res->len = total;
    res->capacity = newCap;
    res->isBuffer = false;

    return res;
  }
  
  memcpy(dest->value + lenDest, src->value, lenSrc);
  dest->len = total;
  dest->value[total] = '\0';

  return dest;
}

String *mulString(String *dest, const Number *src) {
  if (!dest || !src || !dest->value) return NULL;

  int64_t times = src->as.i;
  if (times <= 0) return initString("", 0);

  size_t len = dest->len;
  size_t total = len * (size_t)times;
  
  if (total + 1 > dest->capacity) {
    String *res = poolAlloc(stringPool);
    if (!res) return NULL;

    uint64_t newCap = total * 2;

    char *buf = malloc(newCap + 1);

    if (!buf) {
      poolFree(stringPool, res);
      return NULL;
    }

    char *p = buf;
    
    for (int64_t i = 0; i < times; i++, p += len)
      memcpy(p, dest->value, len);

    *p = '\0';

    res->value = buf;
    res->base.type = OBJ_STRING;
    res->base.isStatic = false;
    res->len = total;
    res->capacity = newCap;
    res->isBuffer = false;

    return res;
  }
  
  char *p = dest->value + len;

  for (int64_t i = 1; i < times; i++, p += len)
    memcpy(p, dest->value, len);
  
  dest->value[total] = '\0';
  dest->len = total;

  return dest;
}
