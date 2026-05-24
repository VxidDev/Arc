#include "../include/token.h"
#include "../include/position.h"

#include <stdlib.h>
#include <string.h>

const char *KEYWORDS[] = {
  "VAR", "AND", "OR", "NOT",
  "IF", "THEN", "ELIF", "ELSE", "WHILE", NULL
};

Token* initToken(TokType type, void* value, bool needsToBeFreed, Position start, Position end) {
  Token* token = malloc(sizeof(Token));

  if (!token) return NULL;

  token->type = type;

  switch (type) {
    case TOK_INT: token->val.i = *(long*)value; break;
    case TOK_FLOAT: token->val.f = *(double*)value; break;
    default: token->val.s = (char*)value; break;
  }

  token->needsToBeFreed = needsToBeFreed;

  token->start = start;
  token->end = end;

  return token;
}

/*
 * WORKS ONLY FOR STRINGS
char *tokenRepr(const Token *t) {
  if (!t || !t->type) return NULL;

  unsigned long typeLen = strlen(t->type);
  unsigned long valueLen = t ->value ? strlen(t->value) : 0;

  unsigned long needed = typeLen + (valueLen ? (valueLen + 1) : 0);
  char *output = malloc(needed * sizeof(char) + 1);

  if (!output) return NULL;

  memcpy(output, t->type, typeLen);

  if (valueLen) {
    output[typeLen] = ':';
    memcpy(output + typeLen + 1, t->value, valueLen);
  }

  output[needed] = '\0';
  return output;
}
*/

void freeToken(Token *t) {
  if (!t) return;
  if (t->val.s && t->needsToBeFreed) free(t->val.s); // string is the only free'able object available

  free(t);
}

void freeTokens(Token **t, unsigned long s) {
  if (!t) return;

  for (unsigned long i = 0; i < s; i++) {
    freeToken(t[i]);
  }

  free(t);
}
