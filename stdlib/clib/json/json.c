#include "../../../include/utils.h"

#include <stdlib.h>
#include <errno.h>
#include <string.h>

typedef enum ARCJSON_TOKEN {
  ARCJSON_TOK_LCURLBRACK,
  ARCJSON_TOK_RCURLBRACK,
  ARCJSON_TOK_COLON,
  ARCJSON_TOK_COMMA,
  ARCJSON_TOK_INT, 
  ARCJSON_TOK_STRING
} ARCJSON_TOKEN;

typedef struct ArcJsonToken {
  size_t type;

  union {
    char *s;
    int64_t i;
  } val;
} ArcJsonToken;

typedef enum ARCJSON_VALUE {
  ARCJSON_STRING,
  ARCJSON_INT,
  ARCJSON_OBJ
} ARCJSON_VALUE;

ArcJsonToken* initArcJsonToken(size_t type, void* content) {
  ArcJsonToken* token = malloc(sizeof(ArcJsonToken));

  if (!token) return NULL;

  token->type = type;
  
  if (type == ARCJSON_TOK_STRING) {
    token->val.s = (char*)content;
  } else if (type == ARCJSON_TOK_INT) {
    token->val.i = (int64_t)(uintptr_t)content;
  } 

  return token;
}

const char *arcTokenRepr(ArcJsonToken* tok) {
  switch (tok->type) {
    case ARCJSON_TOK_LCURLBRACK: return "LCURLBRACK";
    case ARCJSON_TOK_RCURLBRACK: return "RCURLBRACK";
    case ARCJSON_TOK_STRING: return "STRING";
    case ARCJSON_TOK_COLON: return "COLON";
    case ARCJSON_TOK_INT: return "INT";
    case ARCJSON_TOK_COMMA: return "COMMA";
    default: return "UNKNOWN";
  }
}

Object* arcJson_parseValue(ArcJsonToken** tokens, size_t *idx);

Object* arcJson_parseObject(ArcJsonToken** tokens, size_t *idx) {
  (*idx)++; // skip '{'
  
  size_t size = 0;
  size_t capacity = 128;

  Object** objects = malloc(sizeof(Object*) * capacity);

  if (!objects) {
    return NULL;
  }
  
  while (tokens[*idx] && tokens[*idx]->type != ARCJSON_TOK_RCURLBRACK) {
    if (tokens[*idx]->type != ARCJSON_TOK_STRING) {
      return (Object*)initProgramError("Key must be a string.");
    }

    Object* k = arcJson_parseValue(tokens, idx);

    if (!k || k->type == OBJ_ERROR) {
      return k;
    }

    if (!tokens[*idx] || tokens[*idx]->type != ARCJSON_TOK_COLON) {
      return (Object*)initProgramError("Expected ':'.");
    }

    (*idx)++;
    
    if (!tokens[*idx]) {
      return (Object*)initProgramError("Expected value.");
    }

    Object* v = arcJson_parseValue(tokens, idx);

    if (!v || v->type == OBJ_ERROR) {
      return v;
    }
  
    Object *kv[2] = {k, v};
    List* l = initList(kv, 2, 2);

    freeObject(k);
    freeObject(v);

    if (size + 1 > capacity) {
      capacity *= 2;

      void* tmp = realloc(objects, sizeof(Object*) * capacity);

      if (!tmp) {
        return NULL;
      }

      objects = tmp;
    }

    objects[size++] = (Object*)l;

    if (tokens[*idx] && tokens[*idx]->type == ARCJSON_TOK_COMMA) {
      (*idx)++;
    }
  }

  (*idx)++;
  
  Object* obj = (Object*)initList(objects, size, capacity);

  for (size_t i = 0; i < size; i++) {
    freeObject(objects[i]);
  }

  free(objects);

  return obj;
}

Object* arcJson_parseValue(ArcJsonToken** tokens, size_t *idx) {
  ArcJsonToken* current = tokens[*idx];

  if (current->type == ARCJSON_TOK_STRING) {
    Object* o = (Object*)initString(current->val.s, strlen(current->val.s));
    (*idx)++;

    return o;
  }

  if (current->type == ARCJSON_TOK_INT) {
    Object* o = (Object*)initInt(current->val.i);
    (*idx)++;

    return o;
  }

  if (current->type == ARCJSON_TOK_LCURLBRACK) { 
    return arcJson_parseObject(tokens, idx); 
  }
  
  printf("Invalid JSON value.");
  return NULL; 
}

bool _appendTokenList(ArcJsonToken*** tokens, ArcJsonToken* token, size_t *size, size_t *capacity) {
  if (*size + 1 > *capacity) {
    size_t newCap = (*capacity == 0) ? 1 : (*capacity * 2);
    ArcJsonToken** newTokens = realloc(*tokens, sizeof(ArcJsonToken*) * newCap);
  
    if (!newTokens) {
      return false;
    }

    *tokens = newTokens;
    *capacity = newCap;
  }

  (*tokens)[*size] = token;
  (*size)++;

  return true;
}

ArcJsonToken** arcJson_lexer(String* jsonString) { 
  size_t len = jsonString->len;
  
  size_t size = 0;
  size_t capacity = len ? len / 2 : 4;

  ArcJsonToken** tokens = malloc(sizeof(ArcJsonToken*) * capacity);

  if (!tokens) return NULL;
  
  char *val = jsonString->value;

  for (size_t i = 0; i < len; i++) {
    char c = val[i];

    if (c == ' ') continue;

    if (c == '{') {
      if (!_appendTokenList(&tokens, initArcJsonToken(ARCJSON_TOK_LCURLBRACK, NULL), &size, &capacity)) return NULL;
      continue;
    }

    if (c == '}') {
      if (!_appendTokenList(&tokens, initArcJsonToken(ARCJSON_TOK_RCURLBRACK, NULL), &size, &capacity)) return NULL;
      continue;
    }

    if (c == ':') {
      if (!_appendTokenList(&tokens, initArcJsonToken(ARCJSON_TOK_COLON, NULL), &size, &capacity)) return NULL;
      continue;
    }

    if (c == ',') {
      if (!_appendTokenList(&tokens, initArcJsonToken(ARCJSON_TOK_COMMA, NULL), &size, &capacity)) return NULL;
      continue;
    }

    if (c == '\"') {
      size_t strIdx = 0;
      char *s = malloc(1024);

      if (!s) {
        for (size_t i = 0; i < size; i++) {
          if (tokens[i]->type == ARCJSON_TOK_STRING) free(tokens[i]->val.s);
          free(tokens[i]);
        }

        free(tokens);
        return NULL;
      }

      i++;

      while (i < len && val[i] != '\"') {
        s[strIdx++] = val[i]; // TODO: add resizing logic
        i++;
      }
      
      s[strIdx] = '\0';
             
      if (!_appendTokenList(&tokens, initArcJsonToken(ARCJSON_TOK_STRING, s), &size, &capacity)) return NULL;
      continue;
    }

    if (c >= '0' && c <= '9') {
      char *start = &val[i];
      char *endptr;

      errno = 0;
      int64_t num = strtoll(start, &endptr, 10);
      
      if (endptr == start) {
        i++;
        continue;
      }

      if (errno == ERANGE || (*endptr != '\0' && *endptr != '}' && *endptr != ']' && *endptr != ',' && *endptr != ' ')) {
        printf("Cursor at: %c\n", *endptr);

        for (size_t i = 0; i < size; i++) {
          if (tokens[i]->type == ARCJSON_TOK_STRING) free(tokens[i]->val.s);
          free(tokens[i]);
        }

        free(tokens);

        return NULL;
      }

      i = (size_t)(endptr - val) - 1;
       
      if (!_appendTokenList(&tokens, initArcJsonToken(ARCJSON_TOK_INT, (void*)(uintptr_t)num), &size, &capacity)) return NULL;
      continue; 
    }
  }

  tokens[size] = NULL;
  return tokens;
}

Object* arcJson_loads(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_STRING, 1);
  if (err) return err;
  
  String* jsonString = (String*)args[0];

  ArcJsonToken** tokens = arcJson_lexer(jsonString);

  if (!tokens) {
    printf("Error had occured while parsing JSON string.\n");
    return (Object*)initInt(0);
  }
   
  size_t idx = 0;
  Object* map = arcJson_parseValue(tokens, &idx);

  for (size_t i = 0; tokens[i]; i++) {
    if (tokens[i]->type == ARCJSON_TOK_STRING) free(tokens[i]->val.s);
    free(tokens[i]);
  }

  free(tokens);
  return map;
}
