#include "../include/token.h"
#include "../include/position.h"

#include <stdlib.h>
#include <string.h>

const char *KEYWORDS[] = {
  "VAR", "AND", "OR", "NOT",
  "IF", "THEN", "ELIF", "ELSE",
  "WHILE", "TRY", "CATCH"
  "FN", "END", NULL
};

const char* binOpStr[] = {
  [TOK_PLUS] = "+",
  [TOK_MINUS] = "-",
  [TOK_MUL] = "*",
  [TOK_DIV] = "/",
  [TOK_POW] = "^",
  [TOK_EQ] = "=",
  [TOK_EE] = "==",
  [TOK_NE] = "!=",
  [TOK_LT] = "<",
  [TOK_GT] = ">",
  [TOK_LTE] = "<=",
  [TOK_GTE] = ">=",
  [TOK_AND] = "AND",
  [TOK_OR] = "OR",
};

Token initToken(TokType type, void* value, bool needsToBeFreed, Position start, Position end) {
  Token token = {0};

  token.type = type;

  switch (type) {
    case TOK_INT: token.val.i = *(long*)value; break;
    case TOK_FLOAT: token.val.f = *(double*)value; break;
    default: token.val.s = (char*)value; break;
  }

  token.needsToBeFreed = needsToBeFreed;

  token.start = start;
  token.end = end;

  return token;
}

void freeToken(Token *t) {
  if (!t) return;
  if (t->val.s && t->needsToBeFreed) free(t->val.s); // string is the only free'able object available
}

void freeTokens(Token *t, unsigned long s) {
  if (!t) return;

  for (unsigned long i = 0; i < s; i++) {
    freeToken(&t[i]);
  }

  free(t);
}
