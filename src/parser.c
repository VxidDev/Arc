#include "../include/parser.h"
#include "../include/token.h"

#include <stdlib.h>
#include <string.h>

Parser* initParser(Token** tokens, const unsigned long tokenAmount) {
  if (!tokens) return NULL;

  Parser* parser = malloc(sizeof(Parser));

  if (!parser) return NULL;

  parser->tokens = tokens;
  parser->tokenAmount = tokenAmount;
  parser->tokenIndex = -1;
  parser->currentToken = NULL;

  advanceParser(parser);

  return parser;
}

Token* advanceParser(Parser* parser) {
  if (!parser) return NULL;

  parser->tokenIndex++;

  if (parser->tokenIndex < parser->tokenAmount) {
    parser->currentToken = parser->tokens[parser->tokenIndex];
  } else {
    parser->currentToken = NULL;
  }

  return parser->currentToken;
}

ASTNode* exprParser(Parser* parser);
ASTNode* termParser(Parser* parser);
ASTNode* factorParser(Parser* parser);

ASTNode* atomParser(Parser* parser) {
  if (!parser || !parser->currentToken) return NULL;
    
  Token* token = parser->currentToken;

  if (strcmp(token->type, TOK_PLUS) == 0 ||
      strcmp(token->type, TOK_MINUS) == 0) {

    advanceParser(parser);

    ASTNode* expr = atomParser(parser); 

    if (!expr) return NULL;

    return (ASTNode*)initUnaryOpNode(token, expr);
  }

  if (strcmp(token->type, TOK_INT) == 0 || strcmp(token->type, TOK_FLOAT) == 0) {
    advanceParser(parser);
    return (ASTNode*)initNumberNode(token);
  } else if (strcmp(token->type, TOK_LPAREN) == 0) {
    advanceParser(parser);

    ASTNode* expr = exprParser(parser);
    if (!expr) return NULL;

    if (parser->currentToken && strcmp(parser->currentToken->type, TOK_RPAREN) == 0) {
      advanceParser(parser);
      return expr;
    } else {
      freeAST(expr);
      return NULL;
    }
  }

  return NULL;
}

ASTNode* powerParser(Parser* parser) {
  if (!parser) return NULL;

  ASTNode* left = atomParser(parser);
  if (!left) return NULL;

  if (parser->currentToken && strcmp(parser->currentToken->type, TOK_POW) == 0) {
    Token* opTok = parser->currentToken;
    advanceParser(parser);

    ASTNode* right = powerParser(parser);

    if (!right) {
      freeAST(left);
      return NULL;
    }

    return (ASTNode*)initBinOpNode(left, opTok, right);
  }

  return left;
}

ASTNode* factorParser(Parser* parser) {
  if (!parser || !parser->currentToken) return NULL;

  Token* token = parser->currentToken;

  if (strcmp(token->type, TOK_PLUS) == 0 || strcmp(token->type, TOK_MINUS) == 0) {
    advanceParser(parser);
    ASTNode* factor = factorParser(parser);

    if (!factor) return NULL;

    ASTNode* unaryOpNode = (ASTNode*) initUnaryOpNode(token, factor);
    return unaryOpNode; 
  }

  return powerParser(parser);
}

ASTNode* termParser(Parser* parser) {
  if (!parser) return NULL;

  ASTNode* left = factorParser(parser);
  if (!left) return NULL;

  while (parser->currentToken &&
        (strcmp(parser->currentToken->type, TOK_MUL) == 0 ||
         strcmp(parser->currentToken->type, TOK_DIV) == 0)) {

    Token* opTok = parser->currentToken;
    advanceParser(parser);

    ASTNode* right = factorParser(parser);

    if (!right) {
      freeAST(left);
      return NULL;
    }

    left = (ASTNode*)initBinOpNode(left, opTok, right);
    if (!left) {
      freeAST(right);
      return NULL;
    }
  }

  return left;
}

ASTNode* exprParser(Parser* parser) {
  if (!parser) return NULL;

  ASTNode* left = termParser(parser);
  if (!left) return NULL;

  while (parser->currentToken &&
        (strcmp(parser->currentToken->type, TOK_PLUS) == 0 ||
         strcmp(parser->currentToken->type, TOK_MINUS) == 0)) {

    Token* opTok = parser->currentToken;
    advanceParser(parser);

    ASTNode* right = termParser(parser);
    if (!right) {
      freeAST(left);
      return NULL;
    }

    left = (ASTNode*)initBinOpNode(left, opTok, right);
    if (!left) {
      freeAST(right);
      return NULL;
    }
  }

  return left;
}

ASTNode* parseParser(Parser* parser) {
  if (!parser) return NULL;

  ASTNode* res = exprParser(parser);

  if (!res) return NULL;

  if (parser->currentToken != NULL) {
    freeAST(res);
    return NULL;
  }

  return res;
}
