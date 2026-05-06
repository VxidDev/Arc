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
ASTNode* andOrParser(Parser* parser);
ASTNode* compExprParser(Parser* parser);

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

    ASTNode* expr = andOrParser(parser);

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

    ASTNode* unaryOpNode = (ASTNode*)initUnaryOpNode(token, factor);
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

  if (strcmp(parser->currentToken->type, TOK_KEYWORD) == 0 && strcmp((char*)parser->currentToken->value, "IF") == 0) {
    Token* ifTok = parser->currentToken; // safe copy for error reporting
    advanceParser(parser);

    ASTNode* condition = andOrParser(parser);

    if (!condition) return NULL;

    if (!parser->currentToken || strcmp((char*)parser->currentToken->value, "THEN") != 0) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(copyPosition(ifTok->start), copyPosition(ifTok->end), ifTok->start->filename, "Expected THEN");
      freeAST(condition);
      return NULL;
    }

    advanceParser(parser);

    ASTNode* thenExpr = andOrParser(parser);

    if (!thenExpr) {
      freeAST(condition);
      return NULL;
    }

    size_t size = 0;
    size_t capacity = 8;
    ASTNode** elifConds = calloc(capacity, sizeof(ASTNode*));
    ASTNode** elifExprs = calloc(capacity, sizeof(ASTNode*));

    if (!elifConds || !elifExprs) {
      free(elifConds);
      free(elifExprs);
      freeAST(condition);
      freeAST(thenExpr);
      return NULL;
    }

    while (parser->currentToken && strcmp(parser->currentToken->type, TOK_KEYWORD) == 0 && strcmp((char*)parser->currentToken->value, "ELIF") == 0) {
      Token* elifTok = parser->currentToken; // safe copy for error reporting
      advanceParser(parser);

      if (!parser->currentToken) {
        if (*parser->error == NULL) *parser->error = initSyntaxError(copyPosition(elifTok->start), copyPosition(elifTok->end), elifTok->start->filename, "Expected expression after 'ELIF' keyword.");
        for (size_t i = 0; i < size; i++) {
          freeAST(elifConds[i]);
          freeAST(elifExprs[i]);
        }

        free(elifConds);
        free(elifExprs);
        freeAST(condition);
        freeAST(thenExpr);

        return NULL;
      }

      ASTNode* elifCond = andOrParser(parser);

      if (!elifCond) {
        for (size_t i = 0; i < size; i++) {
          freeAST(elifConds[i]);
          freeAST(elifExprs[i]);
        }

        free(elifConds);
        free(elifExprs);
        freeAST(condition);
        freeAST(thenExpr);

        return NULL;
      }

      if (!parser->currentToken || strcmp((char*)parser->currentToken->value, "THEN") != 0) {
        if (*parser->error == NULL) *parser->error = initSyntaxError(copyPosition(elifTok->start), copyPosition(elifTok->end), elifTok->start->filename, "Expected THEN after ELIF condition");
        freeAST(elifCond);

        for (size_t i = 0; i < size; i++) {
          freeAST(elifConds[i]);
          freeAST(elifExprs[i]);
        }

        free(elifConds);
        free(elifExprs);
        freeAST(condition);
        freeAST(thenExpr);

        return NULL;
      }

      advanceParser(parser);

      ASTNode* elifExpr = andOrParser(parser);

      if (!elifExpr) {
        freeAST(elifCond);
        for (size_t i = 0; i < size; i++) {
          freeAST(elifConds[i]);
          freeAST(elifExprs[i]);
        }

        free(elifConds);
        free(elifExprs);
        freeAST(condition);
        freeAST(thenExpr);

        return NULL;
      }

      if (size >= capacity) {
        capacity *= 2;
        void* tmp1 = realloc(elifConds, sizeof(ASTNode*) * capacity);
        void* tmp2 = realloc(elifExprs, sizeof(ASTNode*) * capacity);

        if (!tmp1 || !tmp2) {
          free(tmp1 ? tmp1 : elifConds);
          free(tmp2 ? tmp2 : elifExprs);
          freeAST(elifCond);
          freeAST(elifExpr);

          for (size_t i = 0; i < size; i++) {
            freeAST(elifConds[i]);
            freeAST(elifExprs[i]);
          }

          freeAST(condition);
          freeAST(thenExpr);

          return NULL;
        }

        elifConds = tmp1;
        elifExprs = tmp2;
      }

      elifConds[size] = elifCond;
      elifExprs[size] = elifExpr;
      size++;
    }

    ASTNode* elseExpr = NULL;

    if (parser->currentToken &&
        strcmp(parser->currentToken->type, TOK_KEYWORD) == 0 &&
        strcmp((char*)parser->currentToken->value, "ELSE") == 0) {

      Token* tok = parser->currentToken; // safe copy 

      advanceParser(parser);

      if (!parser->currentToken) {
        if (*parser->error == NULL) *parser->error = initSyntaxError(copyPosition(tok->start), copyPosition(tok->end), tok->start->filename, "Expected expression after 'ELSE' keyword.");
        for (size_t i = 0; i < size; i++) {
          freeAST(elifConds[i]);
          freeAST(elifExprs[i]);
        }

        free(elifConds);
        free(elifExprs);
        freeAST(condition);
        freeAST(thenExpr);

        return NULL;
      } 

      elseExpr = andOrParser(parser);

      if (!elseExpr) {
        for (size_t i = 0; i < size; i++) {
          freeAST(elifConds[i]);
          freeAST(elifExprs[i]);
        }

        free(elifConds);
        free(elifExprs);

        freeAST(condition);
        freeAST(thenExpr);

        if (*parser->error == NULL) *parser->error = initSyntaxError(copyPosition(tok->start), copyPosition(tok->end), tok->start->filename, "Expected expression after 'ELSE' keyword.");

        return NULL;
      }
    }

    return (ASTNode*)initIfNode(condition, thenExpr, elifConds, elifExprs, size, elseExpr);
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

    ASTNode* expr = andOrParser(parser);

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

ASTNode* andOrParser(Parser* parser) {
  if (!parser) return NULL;

  ASTNode* left = compExprParser(parser);
  if (!left) return NULL;

  while (parser->currentToken &&
         strcmp(parser->currentToken->type, TOK_KEYWORD) == 0 &&
        (strcmp((char*)parser->currentToken->value, "AND") == 0 ||
         strcmp((char*)parser->currentToken->value, "OR")  == 0)) {

    Token* opTok = parser->currentToken;
    advanceParser(parser);

    ASTNode* right = compExprParser(parser);

    if (!right) {
      freeAST(left);
      if (*parser->error == NULL)
        *parser->error = initSyntaxError(
          copyPosition(opTok->start), copyPosition(opTok->end),
          opTok->start->filename, "Expression expected after logical operator");
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

ASTNode* compExprParser(Parser* parser) {
  if (!parser) return NULL;

  ASTNode* left = exprParser(parser);
  if (!left) return NULL;

  while (parser->currentToken &&
        (strcmp(parser->currentToken->type, TOK_EE)  == 0 ||
         strcmp(parser->currentToken->type, TOK_NE)  == 0 ||
         strcmp(parser->currentToken->type, TOK_LT)  == 0 ||
         strcmp(parser->currentToken->type, TOK_GT)  == 0 ||
         strcmp(parser->currentToken->type, TOK_LTE) == 0 ||
         strcmp(parser->currentToken->type, TOK_GTE) == 0)) {

    Token* opTok = parser->currentToken;
    advanceParser(parser);

    ASTNode* right = exprParser(parser);

    if (!right) {
      freeAST(left);
      if (*parser->error == NULL)
        *parser->error = initSyntaxError(
          copyPosition(opTok->start), copyPosition(opTok->end),
          opTok->start->filename, "Expression expected after comparison operator");
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

  ASTNode* res = andOrParser(parser);

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

ASTNode* parseProgram(Parser* parser) {
  if (!parser) return NULL;
  
  size_t size = 0;
  size_t capacity = 32;

  ASTNode **statements = calloc(capacity, sizeof(ASTNode*));

  while (parser->currentToken != NULL) {
    ASTNode *statement = andOrParser(parser);

    if (!statement) {
      for (size_t i = 0; i < size; i++) {
        freeAST(statements[i]);
      }

      free(statements);
      return NULL;
    }

    if (size >= capacity) {
      capacity *= 2;

      void *tmp = realloc(statements, sizeof(ASTNode*) * capacity);

      if (!tmp) {
        for (size_t i = 0; i < size; i++) {
          freeAST(statements[i]);
        }

        free(statements);
        return NULL;
      }

      statements = tmp;
    }

    statements[size++] = statement;
  }

  statements[size] = NULL;

  return (ASTNode*)initProgramNode(statements, size);
}
