#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

#include "token.h"
#include "node.h"
#include "error.h"
#include "lexer.h"

typedef struct Parser {
  Token currentToken;
  Lexer* lexer;
  Error **error;
} Parser;

Parser* initParser(Lexer* lexer, Error **error);
Token advanceParser(Parser* parser);

ASTNode* parseParser(Parser* parser);
ASTNode* parseProgram(Parser* parser);

#endif // PARSER_H
