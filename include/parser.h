#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

#include "token.h"
#include "node.h"
#include "error.h"

typedef struct Parser {
  Token *tokens;
  Token currentToken;

  int64_t tokenIndex;
  uint64_t tokenAmount;

  Error **error;
  char* sourcetext;
  char *filename;
} Parser;

typedef struct ParserCheckpoint {
  int tokenIndex;
  Token currentToken;
} ParserCheckpoint;

ParserCheckpoint saveParser(Parser* parser);
void rewindParser(Parser* parser, ParserCheckpoint checkpoint);

Parser* initParser(Token *tokens, const unsigned long tokenAmount, Error **error, char *sourcetext, char *filename);
Token advanceParser(Parser* parser);

ASTNode* parseParser(Parser* parser);
ASTNode* parseProgram(Parser* parser);

#endif // PARSER_H
