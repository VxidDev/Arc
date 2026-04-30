#ifndef TOKEN_H
#define TOKEN_H

#define TOK_INT "INT"
#define TOK_FLOAT "FLOAT" 
#define TOK_PLUS "PLUS"
#define TOK_MINUS "MINUS"
#define TOK_MUL "MUL"
#define TOK_DIV "DIV"
#define TOK_POW "POW"
#define TOK_LPAREN "LPAREN"
#define TOK_RPAREN "RPAREN"

#include "position.h"

#include <stdbool.h>

typedef struct Token {
  char *type;

  Position *start, *end;

  void *value;
  bool needsToBeFreed;
} Token;

Token* initToken(char *type, void *value, bool needsToBeFreed, Position* start, Position* end);
char *tokenRepr(const Token* t);

void freeToken(Token* t);
void freeTokens(Token** t, unsigned long s);

#endif // TOKEN_H 
