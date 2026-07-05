#include "../include/utils.h"
#include "../include/token.h"

#include "../include/repl/repl.h"

#include "../include/memarena.h"

#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
  #define PATH_SEP '\\'
#else
  #define PATH_SEP '/'
#endif

char *stringDup(const char *s) {
  size_t len = strlen(s) + 1;
  char *sDup = arenaAlloc(stringArena, len);

  memcpy(sDup, s, len);

  return sDup;
}

uint32_t hashStr(const char *str, size_t len) {
  uint32_t hash = 2166136261u;

  for (size_t i = 0; i < len; i++) {
    hash ^= (uint8_t)str[i];
    hash *= 16777619u;
  }

  return hash;
}

char* typeofobj(const Object* obj) {
  switch (obj->type) {
    case OBJ_NUMBER_INT: return "int";
    case OBJ_NUMBER_FLOAT: return "float"; 
    case OBJ_STRING: return "string"; 
    case OBJ_ERROR: return "error"; 
    case OBJ_LIST: return "list";
    case OBJ_FUNCTION: return "function"; 
    case OBJ_FILE: return "file";
    case OBJ_INSTANCE: return "instance";
    case OBJ_CLASS: return "class";
    default: return "object"; 
  }
}

char *objTypeToStr(const ObjType type) {
  switch (type) {
    case OBJ_NUMBER_INT: return "int";
    case OBJ_NUMBER_FLOAT: return "float"; 
    case OBJ_STRING: return "string"; 
    case OBJ_ERROR: return "error"; 
    case OBJ_LIST: return "list";
    case OBJ_FUNCTION: return "function"; 
    case OBJ_FILE: return "file";
    case OBJ_INSTANCE: return "instance";
    case OBJ_CLASS: return "class";
    default: return "object"; 
  }
}

char* tokToString(const TokType type) {
  switch (type) {
    case TOK_INT: return "INT";
    case TOK_FLOAT: return "FLOAT";

    case TOK_PLUS: return "PLUS";
    case TOK_MINUS: return "MINUS";
    case TOK_MUL: return "MUL";
    case TOK_DIV: return "DIV";
    case TOK_POW: return "POW";

    case TOK_LPAREN: return "LPAREN";
    case TOK_RPAREN: return "RPAREN";

    case TOK_EQ: return "EQ";
    case TOK_EE: return "EE";
    case TOK_NE: return "NE";
    case TOK_LT: return "LT";
    case TOK_GT: return "GT";
    case TOK_LTE: return "LTE";
    case TOK_GTE: return "GTE";

    case TOK_IDENTIFIER: return "IDENTIFIER";
    case TOK_KEYWORD: return "KEYWORD";
    case TOK_STRING: return "STRING";

    case TOK_LBRACK: return "LBRACK";
    case TOK_RBRACK: return "RBRACK";
    case TOK_COMMA: return "COMMA";

    case TOK_END: return "END";
    case TOK_IF: return "IF";
    case TOK_ELIF: return "ELIF";
    case TOK_ELSE: return "ELSE";
    case TOK_THEN: return "THEN";
    
    case TOK_AND: return "AND";
    case TOK_OR: return "OR";

    case TOK_VAR: return "VAR";
    case TOK_WHILE: return "WHILE";

    case TOK_FN: return "FN";

    case TOK_IMPORT: return "IMPORT";
    case TOK_RETURN: return "RETURN";
    
    case TOK_TRY: return "TRY";
    case TOK_CATCH: return "CATCH";

    case TOK_BREAK: return "BREAK";
    case TOK_CONTINUE: return "CONTINUE";
    
    case TOK_FOR: return "FOR";
    case TOK_IN: return "IN";

    case TOK_CLASS: return "CLASS";
    case TOK_DOT: return "DOT";

    case TOK_LCURLBRACK: return "LCURLBRACK";
    case TOK_RCURLBRACK: return "RCURLBRACK";
    case TOK_COLON: return "COLON";

    case TOK_EOF: return "EOF";
    case TOK_INVALID: return "INVALID";

    default: return "UNKNOWN";
  }
}

void getDirectory(const char* path, char* out) {
  strcpy(out, path);

  char* slash = strrchr(out, '/');

  #ifdef _WIN32

  char* backslash = strrchr(out, '\\');

  if (!slash || (backslash && backslash > slash))
    slash = backslash;

  #endif // _WIN32

  if (slash)
    *slash = '\0';
  else
    strcpy(out, ".");
}

static int hasExtension(const char* path) {
  const char* dot = strrchr(path, '.');

  const char* slash1 = strrchr(path, '/');
  const char* slash2 = strrchr(path, '\\');

  const char* slash = slash1;

  if (!slash || (slash2 && slash2 > slash))
    slash = slash2;

  return dot && (!slash || dot > slash);
}

char* resolveImportPath(const char* currentFile, const char* importPath) {
  if (importPath[0] == '@') {
    const char *path = importPath + 1;
    char buffer[4096];

    if (hasExtension(path)) {
      snprintf(buffer, sizeof(buffer), "%s/%s", ARC_LIB_DIR, path);
    } else {
      snprintf(buffer, sizeof(buffer), "%s/%s.arc", ARC_LIB_DIR, path);
    }
    return stringDup(buffer);
  }

  if (importPath[0] == '/') {
    return stringDup(importPath);
  }

  char dir[4096];
  getDirectory(currentFile, dir);

  char buffer[4096];

  if (hasExtension(importPath)) {
    snprintf(buffer, sizeof(buffer), "%s/%s", dir, importPath);
  } else {
    snprintf(buffer, sizeof(buffer), "%s/%s.arc", dir, importPath);
  }

  return stringDup(buffer);
}

Object* enforceType(Object* obj, ObjType expectedType, size_t argN) {
  if (obj->type != expectedType) {
    char buf[256];
    snprintf(buf, sizeof(buf), "Expected argument %zu to be object of type '%s', received '%s'.", argN, objTypeToStr(expectedType), typeofobj(obj));
    return (Object*)initProgramError(buf);
  }

  return NULL;
}
