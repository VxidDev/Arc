#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "node.h"
#include "error.h"

typedef struct Parser {
  Token **tokens;
  Token *currentToken;

  long tokenIndex;
  unsigned long tokenAmount;

  Error **error;
} Parser;

Parser* initParser(Token **tokens, const unsigned long tokenAmount, Error **error);
Token* advanceParser(Parser* parser);

ASTNode* parseParser(Parser* parser);

#endif // PARSER_H 
