#include "../../include/builtIns/string.h"
#include "../../include/utils.h"

#include "../../include/mempool.h"

#include <string.h>
#include <stdlib.h>

static int __countTokens_internal(const char *str, const char *delim) {
  int count = 1;
  const char *p = str;
  size_t dlen = strlen(delim);

  while ((p = strstr(p, delim)) != NULL) {
    count++;
    p += dlen;
  }

  return count;
}

static void __freeSplit_internal(char **arr, int n) {
  for (int i = 0; i < n; i++) {
    free(arr[i]);
  }

  free(arr);
}

Object* builtIn_split_string(Object** args, size_t argCount) {
  (void)argCount;

  Object* stringObj = args[0];

  if (stringObj->type != OBJ_STRING) {
    char buf[256];

    snprintf(buf, sizeof(buf), "Expected argument 1 to be object of type 'string', received '%s'.", typeofobj(stringObj));

    return (Object*)initProgramError(buf);
  }

  String* string = (String*)stringObj;

  Object* delimObj = args[1];

  if (delimObj->type != OBJ_STRING) {
    char buf[256];

    snprintf(buf, sizeof(buf), "Expected argument 2 to be object of type 'string', received '%s'.", typeofobj(delimObj));

    return (Object*)initProgramError(buf);
  }

  String* delim = (String*)delimObj;

  if (delim->value[0] == '\0') {
    return (Object*)initProgramError("Empty delimiter not allowed.");
  }

  size_t dlen = strlen(delim->value);
  int tokens = __countTokens_internal(string->value, delim->value);

  char **out = malloc(sizeof(char *) * (tokens + 1));
  if (!out) return (Object*)initProgramError("Failed to split string. (Memory Error)");

  int i = 0;

  const char *start = string->value;
  const char *pos;

  while ((pos = strstr(start, delim->value)) != NULL) {
    size_t len = pos - start;

    out[i] = malloc(len + 1);

    if (!out[i]) {
      __freeSplit_internal(out, i);
      return (Object*)initProgramError("Failed to split string. (Memory Error)");
    } 

    memcpy(out[i], start, len);
    out[i][len] = '\0';

    i++;
    start = pos + dlen;
  }

  out[i] = strdup(start);

  if (!out[i]) {
    __freeSplit_internal(out, i);
    return (Object*)initProgramError("Failed to split string. (Memory Error)");
  }
  
  i++;
  out[i] = NULL;

  String** res = malloc(sizeof(String*) * (tokens + 1));

  if (!res) {
    __freeSplit_internal(out, i);
    return (Object*)initProgramError("Failed to split string. (Memory Error)");
  }

  for (int j = 0; j < i; j++) {
    res[j] = initString(out[j], strlen(out[j]));
  }

  __freeSplit_internal(out, i);

  Object* list = (Object*)initList((Object**)res, i, i);
  
  free(res);

  return list;
}

Object* builtIn_append_char(Object** args, size_t argCount) {
  (void)argCount;

  Object* obj = args[0];
  Object* cObj = args[1];

  if (obj->type != OBJ_STRING) {
    char buf[256];
    snprintf(buf, sizeof(buf), "Expected argument 1 to be object of type 'string', received '%s'.", typeofobj(obj));
    return (Object*)initProgramError(buf);
  }

  if (cObj->type != OBJ_STRING) {
    char buf[256];
    snprintf(buf, sizeof(buf), "Expected argument 2 to be object of type 'string', received '%s'.", typeofobj(cObj));
    return (Object*)initProgramError(buf);
  }

  String* s = (String*)obj; 
  String* cStr = (String*)cObj;

  if (cStr->len != 1) {
    return (Object*)initProgramError("Expected 2nd argument's length to be 1.");
  }

  char c = cStr->value[0];
  
  if (s->base.isStatic && !s->isBuffer) {
    String* fresh = initString(s->value, s->len);
    if (!fresh) return (Object*)initProgramError("Failed to append character (Memory error)");
    s = fresh;
  }

  if (s->len + 1 >= s->capacity) {
    size_t newCap = s->capacity ? s->capacity * 2 : 8;

    char *buf = realloc(s->value, newCap);

    if (!buf) return (Object*)initProgramError("Failed to append character (Memory error)");

    s->value = buf;
    s->capacity = newCap;
  }

  s->value[s->len++] = c;
  s->value[s->len] = '\0';

  return (Object*)s;
}

Object* builtIn_string_buffer(Object** args, size_t argCount) {
  (void)args;
  (void)argCount;
  String *s = initStringBuffer(16);
  if (!s) return (Object*)initProgramError("Failed to create string buffer.");
  return (Object*)s;
}

Object* builtIn_string_finish(Object** args, size_t argCount) {
  (void)argCount;

  Object* obj = args[0];

  if (obj->type != OBJ_STRING) {
    return (Object*)initProgramError("Expected argument 1 to be object of type 'string'.");
  }

  String* buf = (String*)obj;

  String* result = noCopyInitString(buf->value, buf->len);
  if (!result) return (Object*)initProgramError("Failed to finalize string buffer.");

  return (Object*)result;
}
