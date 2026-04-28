#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "node.h"

typedef struct Parser {
  Token **tokens;
  Token *currentToken;

  long tokenIndex;
  unsigned long tokenAmount;
} Parser;

Parser* initParser(Token **tokens, const unsigned long tokenAmount);
Token* advanceParser(Parser* parser);

ASTNode* parseParser(Parser* parser);

#endif // PARSER_H 
