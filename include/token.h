#ifndef TOKEN_H
#define TOKEN_H

#define TOK_INT "INT"
#define TOK_FLOAT "FLOAT" 
#define TOK_PLUS "PLUS"
#define TOK_MINUS "MINUS"
#define TOK_MUL "MUL"
#define TOK_DIV "DIV"
#define TOK_LPAREN "LPAREN"
#define TOK_RPAREN "RPAREN"

#include <stdbool.h>

typedef struct Token {
  char *type;
  void *value;
  bool needsToBeFreed;
} Token;

Token* initToken(char *type, void *value, bool needsToBeFreed);
char *tokenRepr(const Token* t);
void freeToken(Token* t);

#endif // TOKEN_H 
