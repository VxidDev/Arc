#include "../../include/builtIns/string.h"
#include "../../include/utils.h"

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
