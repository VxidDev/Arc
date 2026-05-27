#ifndef TOKEN_H
#define TOKEN_H

typedef enum TokType {
  TOK_INT, TOK_FLOAT,
  TOK_PLUS, TOK_MINUS, TOK_MUL, TOK_DIV, TOK_POW,
  TOK_LPAREN, TOK_RPAREN,
  TOK_EQ, TOK_EE, TOK_NE, TOK_LT, TOK_GT, TOK_LTE, TOK_GTE,
  TOK_IDENTIFIER, TOK_KEYWORD, TOK_STRING,
  TOK_IF, TOK_THEN, TOK_ELIF, TOK_ELSE, TOK_END,
  TOK_VAR,
  TOK_AND, TOK_OR,
  TOK_LBRACK, TOK_RBRACK, TOK_COMMA,
  TOK_WHILE, 
  TOK_FN,
  TOK_IMPORT
} TokType;

extern const char* binOpStr[];

extern const char *KEYWORDS[];

#include "position.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct Token {
  TokType type;

  Position start, end;

  union {
    char* s;
    double f;
    int64_t i;
  } val;

  bool needsToBeFreed;
} Token;

Token* initToken(TokType type, void *value, bool needsToBeFreed, Position start, Position end);
char *tokenRepr(const Token* t);

void freeToken(Token* t);
void freeTokens(Token** t, unsigned long s);

#endif // TOKEN_H
