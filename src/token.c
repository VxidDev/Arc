#include "../include/token.h"
#include "../include/utils.h"
#include "../include/position.h"

#include <stdlib.h>
#include <string.h>

const char *KEYWORDS[] = {
  "VAR", "AND", "OR", "NOT", NULL
};

Token* initToken(char *type, void* value, bool needsToBeFreed, Position* start, Position* end) {
  if (!type) return NULL;

  Token* token = malloc(sizeof(Token));

  if (!token) return NULL;

  token->type = stringDup(type);

  if (!token->type) {
    free(token);
    return NULL;
  }

  token->value = value;
  token->needsToBeFreed = needsToBeFreed;
  
  token->start = start;

  if (token->start && end) {
    token->end = end;
  } else if (token->start && !end) {
    Position* copy = copyPosition(start);
    advancePosition(start, 0);
    token->end = copy;
  } else {
    token->end = NULL;
  }

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

  if (t->type) free(t->type);
  if (t->value && t->needsToBeFreed) free(t->value);
  if (t->start) freePosition(t->start);
  if (t->end) freePosition(t->end);

  free(t);
}

void freeTokens(Token **t, unsigned long s) {
  if (!t) return;

  for (unsigned long i = 0; i < s; i++) {
    freeToken(t[i]);
  }

  free(t);
}
