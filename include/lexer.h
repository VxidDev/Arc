#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include "error.h"

#include <stddef.h>

typedef struct Lexer {
  char *text;
  char *filename;
  unsigned long textLen;
  Position pos;
  char currChar;
} Lexer;

Lexer* initLexer(char *filename, char *text);
Token lexNextToken(Lexer *lexer, Error **error);

#endif // LEXER_H 
