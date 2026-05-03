#include "../include/parser.h"
#include "../include/token.h"

#include <stdlib.h>
#include <string.h>

Parser* initParser(Token** tokens, const unsigned long tokenAmount, Error **error) {
  if (!tokens || !error) return NULL;

  Parser* parser = malloc(sizeof(Parser));

  if (!parser) return NULL;

  parser->tokens = tokens;
  parser->tokenAmount = tokenAmount;
  parser->tokenIndex = -1;
  parser->currentToken = NULL;

  parser->error = error;

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
    
    Token* tok = token; // safe copy 

    advanceParser(parser);

    ASTNode* expr = atomParser(parser); 

    if (!expr) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(copyPosition(tok->start), copyPosition(tok->end), tok->start->filename, "Expression expected");
      return NULL;
    }

    return (ASTNode*)initUnaryOpNode(token, expr);
  }

  if (strcmp(token->type, TOK_STRING) == 0) {
    advanceParser(parser);
    return (ASTNode*)initStringNode(token);
  }

  if (strcmp(token->type, TOK_INT) == 0 || strcmp(token->type, TOK_FLOAT) == 0) {
    advanceParser(parser);
    return (ASTNode*)initNumberNode(token);
  } else if (strcmp(token->type, TOK_IDENTIFIER) == 0) {
    advanceParser(parser);
    return (ASTNode*)initVarAccessNode(token);
  } else if (strcmp(token->type, TOK_LPAREN) == 0) {
    advanceParser(parser);
    
    Token* tok = token; // safe copy 

    ASTNode* expr = exprParser(parser);

    if (!expr) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(copyPosition(tok->start), copyPosition(tok->end), tok->start->filename, "Expression expected");
      return NULL;
    }

    if (parser->currentToken && strcmp(parser->currentToken->type, TOK_RPAREN) == 0) {
      advanceParser(parser);
      return expr;
    } else {
      freeAST(expr);
      if (*parser->error == NULL) *parser->error = initSyntaxError(copyPosition(token->start), copyPosition(token->end), token->start->filename, "Expression expected");
      return NULL;
    }
  }
  
  if (*parser->error == NULL) *parser->error = initSyntaxError(copyPosition(token->start), copyPosition(token->end), token->start->filename, "Expression expected");
  return NULL;
}

ASTNode* powerParser(Parser* parser) {
  if (!parser) return NULL;

  ASTNode* left = atomParser(parser);
  if (!left) return NULL; // error is already set

  if (parser->currentToken && strcmp(parser->currentToken->type, TOK_POW) == 0) {
    Token* opTok = parser->currentToken;
    advanceParser(parser);

    ASTNode* right = powerParser(parser);

    if (!right) {
      freeAST(left);
      if (*parser->error == NULL) *parser->error = initSyntaxError(copyPosition(opTok->start), copyPosition(opTok->end), opTok->start->filename, "Expression expected after '^'");
      return NULL;
    }

    return (ASTNode*)initBinOpNode(left, opTok, right);
  }

  return left;
}

ASTNode* factorParser(Parser* parser) {
  if (!parser || !parser->currentToken) return NULL;

  Token* token = parser->currentToken;

  if (!token) return NULL;

  if (strcmp(token->type, TOK_PLUS) == 0 || strcmp(token->type, TOK_MINUS) == 0) {
    Token* opTok = token;

    advanceParser(parser);
    ASTNode* factor = factorParser(parser);

    if (!factor) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(copyPosition(opTok->start), copyPosition(opTok->end), opTok->start->filename, "Expression expected");
      return NULL;
    }

    ASTNode* unaryOpNode = (ASTNode*) initUnaryOpNode(token, factor);
    return unaryOpNode; 
  }

  return powerParser(parser);
}

ASTNode* termParser(Parser* parser) {
  if (!parser) return NULL;

  ASTNode* left = factorParser(parser);
  if (!left) return NULL; // error is already set

  while (parser->currentToken &&
        (strcmp(parser->currentToken->type, TOK_MUL) == 0 ||
         strcmp(parser->currentToken->type, TOK_DIV) == 0)) {

    Token* opTok = parser->currentToken;
    advanceParser(parser);

    ASTNode* right = factorParser(parser);

    if (!right) { // error is already set
      if (*parser->error == NULL) *parser->error = initSyntaxError(copyPosition(opTok->start), copyPosition(opTok->end), opTok->start->filename, "Expression expected");
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

  if (parser->currentToken &&
    strcmp(parser->currentToken->type, TOK_RPAREN) == 0) {

    if (*parser->error == NULL) {
      *parser->error = initSyntaxError(
        copyPosition(parser->currentToken->start),
        copyPosition(parser->currentToken->end),
        parser->currentToken->start->filename,
        "Unexpected ')'"
      );
    }

    return NULL;
  }
  
  if (!parser->currentToken) {
    if (*parser->error == NULL); // Will be implemented after addition of EOF token

    return NULL;
  }

  if (strcmp(parser->currentToken->type, TOK_KEYWORD) == 0 && strcmp((char*)parser->currentToken->value, "VAR") == 0) {
    Token* tok = parser->currentToken; // safe copy
    advanceParser(parser);

    if (!parser->currentToken) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(copyPosition(tok->start), copyPosition(tok->end), tok->start->filename, "Expression token after 'VAR' keyword.");
      return NULL;
    }

    if (strcmp(parser->currentToken->type, TOK_IDENTIFIER) != 0) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(copyPosition(parser->currentToken->start), copyPosition(parser->currentToken->end), parser->currentToken->start->filename, "Expected identifier after 'VAR'");
      return NULL;
    }

    char *varName = (char*)parser->currentToken->value;
    
    tok = parser->currentToken; // update safe copy

    advanceParser(parser);

    if (!parser->currentToken) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(copyPosition(tok->start), copyPosition(tok->end), tok->start->filename, "Missing '=' after identifier");
      return NULL;
    }

    if (strcmp(parser->currentToken->type, TOK_EQ) != 0) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(copyPosition(parser->currentToken->start), copyPosition(parser->currentToken->end), parser->currentToken->start->filename, "Expected '=' after identifier");
      return NULL;
    }

    tok = parser->currentToken; // update safe copy

    advanceParser(parser);

    ASTNode* expr = exprParser(parser);

    if (!expr) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(copyPosition(tok->start), copyPosition(tok->end), tok->start->filename, "Expected expression after '='"); 
      return NULL;
    }

    return (ASTNode*)initVarAssignNode(varName, expr);
  }

  ASTNode* left = termParser(parser);
  if (!left) return NULL; // error is already set

  while (parser->currentToken &&
        (strcmp(parser->currentToken->type, TOK_PLUS) == 0 ||
         strcmp(parser->currentToken->type, TOK_MINUS) == 0)) {

    Token* opTok = parser->currentToken;
    advanceParser(parser);

    ASTNode* right = termParser(parser);

    if (!right) { 
      if (*parser->error == NULL) *parser->error = initSyntaxError(copyPosition(opTok->start), copyPosition(opTok->end), opTok->start->filename, "Expression expected");
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

  if (!res) return NULL; // error is already set

  if (parser->currentToken != NULL) {
    if (*parser->error == NULL) {
      *parser->error = initSyntaxError(
        copyPosition(parser->currentToken->start),
        copyPosition(parser->currentToken->end),
        parser->currentToken->start->filename,
        "Unexpected token after expression"
      );
    }

    freeAST(res);
    return NULL;
  }

  return res;
}
