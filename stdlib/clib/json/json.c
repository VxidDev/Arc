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
  ARCJSON_TOK_STRING,
  ARCJSON_END 
} ARCJSON_TOKEN;

typedef struct ArcJsonToken {
  size_t type;

  union {
    struct s { char *s; size_t len; } s;
    int64_t i;
  } val;
} ArcJsonToken;

typedef enum ARCJSON_VALUE {
  ARCJSON_STRING,
  ARCJSON_INT,
  ARCJSON_OBJ
} ARCJSON_VALUE;

typedef struct ArcJsonLexer {
  String* s;
  ArcJsonToken currentTok;
  size_t cursor;
} ArcJsonLexer;

const char *arcTokenRepr(ArcJsonToken tok) {
  switch (tok.type) {
    case ARCJSON_TOK_LCURLBRACK: return "LCURLBRACK";
    case ARCJSON_TOK_RCURLBRACK: return "RCURLBRACK";
    case ARCJSON_TOK_STRING: return "STRING";
    case ARCJSON_TOK_COLON: return "COLON";
    case ARCJSON_TOK_INT: return "INT";
    case ARCJSON_TOK_COMMA: return "COMMA";
    case ARCJSON_END: return "END";
    default: return "UNKNOWN";
  }
}

Object* arcJson_parseValue(ArcJsonLexer *lexer);
ArcJsonToken arcJson_getNextToken(ArcJsonLexer* lexer); 

Object* arcJson_parseObject(ArcJsonLexer *lexer) { 
  size_t size = 0;
  size_t capacity = 8;

  Object** objects = malloc(sizeof(Object*) * capacity);

  if (!objects) {
    return NULL;
  }

  ArcJsonToken current = arcJson_getNextToken(lexer); // consumes '{'
  
  while (current.type != ARCJSON_END && current.type != ARCJSON_TOK_RCURLBRACK) {
    if (current.type != ARCJSON_TOK_STRING) {
      return (Object*)initProgramError("Key must be a string.");
    }

    Object* k = (Object*)initString(current.val.s.s, current.val.s.len); 

    if (!k || k->type == OBJ_ERROR) {
      return k;
    }

    current = arcJson_getNextToken(lexer); // consumes colon 

    if (current.type != ARCJSON_TOK_COLON) {
      return (Object*)initProgramError("Expected ':'.");
    }
 
    Object* v = arcJson_parseValue(lexer); // consumes value 

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
    
    current = arcJson_getNextToken(lexer);

    if (current.type == ARCJSON_TOK_COMMA) {
      current = arcJson_getNextToken(lexer);
    }
  }
   
  Object* obj = (Object*)initList(objects, size, capacity);

  for (size_t i = 0; i < size; i++) {
    freeObject(objects[i]);
  }

  free(objects);

  return obj;
}

Object* arcJson_parseValue(ArcJsonLexer* lexer) {
  ArcJsonToken current = arcJson_getNextToken(lexer);

  if (current.type == ARCJSON_TOK_STRING) {
    Object* o = (Object*)initString(current.val.s.s, current.val.s.len);
    return o;
  }

  if (current.type == ARCJSON_TOK_INT) {
    Object* o = (Object*)initInt(current.val.i);
    return o;
  }

  if (current.type == ARCJSON_TOK_LCURLBRACK) { 
    return arcJson_parseObject(lexer); 
  }
  
  printf("Invalid JSON value, Token type: %s\n", arcTokenRepr(current));
  return NULL; 
}

ArcJsonToken arcJson_getNextToken(ArcJsonLexer* lexer) { 
  ARCJSON_TOKEN type = ARCJSON_END;
  void* value = NULL;
  size_t len = 0;
  
  char *s = lexer->s->value;
  
  while (s[lexer->cursor] <= ' ') lexer->cursor++;

  switch (s[lexer->cursor++]) {
    case '{': type = ARCJSON_TOK_LCURLBRACK; goto end;
    case '}': type = ARCJSON_TOK_RCURLBRACK; goto end;
    case ':': type = ARCJSON_TOK_COLON; goto end;
    case ',': type = ARCJSON_TOK_COMMA; goto end;  
    case '\"': {
      size_t start = lexer->cursor;
      char *closing = memchr(s + start, '\"', lexer->s->len - start);
      len = closing - (s + start);
      *closing = '\0';
      
      lexer->cursor = (closing - s) + 1;

      type = ARCJSON_TOK_STRING;
      value = &s[start];
      goto end;
    }

    case '0' ... '9': {
      char *start = &s[lexer->cursor - 1];
      char *endptr;

      errno = 0;
      int64_t num = strtoll(start, &endptr, 10);

      if (endptr == start) {
        lexer->cursor++;
        goto end;
      }


      if (errno == ERANGE || (*endptr != '\0' && *endptr != '}' && *endptr != ']' && *endptr != ',' && *endptr > ' ')) {
        printf("Cursor at: %c\n", *endptr);
        return (ArcJsonToken){ARCJSON_END, {.i = 0}};
      }

      lexer->cursor = (size_t)(endptr - s);
      
      type = ARCJSON_TOK_INT;
      value = (void*)(uintptr_t)num;
    }
  }
  
  end:
     return type == ARCJSON_TOK_INT ? (ArcJsonToken){type, .val = {.i = (int64_t)(uintptr_t)value}} : (ArcJsonToken){type, {.s = { value, len}}};
}

Object* arcJson_loads(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_STRING, 1);
  if (err) return err;
  
  String* jsonString = (String*)args[0];
  String* copiedString = initString(jsonString->value, jsonString->len);

  ArcJsonLexer lexer = (ArcJsonLexer){copiedString, {0}, 0};
 
  Object* map = arcJson_parseValue(&lexer);
  
  freeObject((Object*)copiedString);

  return map;
}
