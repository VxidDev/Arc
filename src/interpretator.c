#include "../include/node.h"
#include "../include/interpretator.h"
#include "../include/object.h"
#include "../include/symbol-table.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

Number* visitNumberNode(ASTNode* node, char *filename, Error **err, SymbolTable* variables) {
  if (!filename || !err) return NULL;

  NumberNode* numNode = (NumberNode*)node;

  if (!numNode->token) {
    if (*err == NULL) *err = initValueError(copyPosition(numNode->token->start), copyPosition(numNode->token->end), filename, "Expected TOK_FLOAT or TOK_INT token, received NULL");
    return NULL;
  }
  
  if (strcmp(numNode->token->type, TOK_FLOAT) == 0) {
    return initFloat(*(double*)numNode->token->value);
  }

  return initInt(*(long*)numNode->token->value);
}

Number* visitBinOpNode(ASTNode* node, char *filename, Error **err, SymbolTable* variables) {
  if (!filename || !err) return NULL;

  BinOpNode* binOper = (BinOpNode*)node;

  Number *src = visitNode(binOper->leftNode, filename, err, variables);
  Number *dest = visitNode(binOper->rightNode, filename, err, variables);
  
  if (!src || !dest) {
    if (*err == NULL) *err = initValueError(copyPosition(binOper->operTok->start), copyPosition(binOper->operTok->end), filename, "Expected TOK_FLOAT or TOK_INT token, received NULL");
    if (src) free(src);
    if (dest) free(dest);
    return NULL;
  } 
  
  EvalResultNumber output;

  if (strcmp(binOper->operTok->type, TOK_PLUS) == 0) {
    output = addNumber(dest, src);
  } else if (strcmp(binOper->operTok->type, TOK_MINUS) == 0) {
    output = subNumber(dest, src);
  } else if (strcmp(binOper->operTok->type, TOK_MUL) == 0) {
    output = mulNumber(dest, src);
  } else if (strcmp(binOper->operTok->type, TOK_DIV) == 0) {
    output = divNumber(dest, src);
  } else if (strcmp(binOper->operTok->type, TOK_POW) == 0) {
    output = powNumber(dest, src);
  }

  if (output.err) {
    if (output.err == ERR_NULL) {
      if (*err == NULL) *err = initValueError(copyPosition(binOper->operTok->start), copyPosition(binOper->operTok->end), filename, "Expected TOK_FLOAT or TOK_INT token, received NULL");
    } else if (output.err == ERR_DIV_BY_ZERO) {
      if (*err == NULL) *err = initValueError(copyPosition(binOper->operTok->start), copyPosition(binOper->operTok->end), filename, "Division by zero");
    } else { if (*err == NULL) *err = initValueError(copyPosition(binOper->operTok->start), copyPosition(binOper->operTok->end), filename, "Unknown error."); }

    free(src);
    free(dest);

    return NULL;
  }

  free(dest);
  free(src);

  return output.num;
}

Number* visitUnaryOpNode(ASTNode* node, char *filename, Error **err, SymbolTable* variables) {
  UnaryOpNode* unaryOper = (UnaryOpNode*)node;

  Number* number = visitNode(unaryOper->node, filename, err, variables);

  if (!number) {
    if (*err == NULL) *err = initValueError(copyPosition(unaryOper->operTok->start), copyPosition(unaryOper->operTok->end), filename, "Expected Number* result, received NULL.");
    return NULL;
  }
  
  EvalResultNumber output = {0};

  if (strcmp(unaryOper->operTok->type, TOK_MINUS) == 0) {
    Number* negOne = initInt(-1);
    output = mulNumber(number, negOne);
    free(number);
    free(negOne);
  } else {
    if (*err == NULL) *err = initValueError(copyPosition(unaryOper->operTok->start), copyPosition(unaryOper->operTok->end), filename, "Unknown unary operator");
    free(number);
    return NULL;
  }

  return output.num;
}

Number* visitVarAccessNode(ASTNode* node, char *filename, Error** err, SymbolTable* variables) {
  if (!node) return NULL;
  
  VarAccessNode* va = (VarAccessNode*)node;
  char *varName = (char*)va->token->value;

  if (!varName) return NULL;
  
  Number* stored = (Number*)getTable(variables, varName);

  if (!stored) {
    int len = snprintf(NULL, 0, "Undefined variable \"%s\"", varName);
    char *buffer = malloc(len + 1);

    snprintf(buffer, len + 1, "Undefined variable \"%s\"", varName);

    if (*err == NULL) *err = initNameError(copyPosition(va->token->start), copyPosition(va->token->end), filename, buffer);
    free(buffer);

    return NULL;
  }

  return copyNumber(stored); 
}

Number* visitVarAssignNode(ASTNode* node, char *filename, Error** err, SymbolTable* variables) {
  if (!node) return NULL;

  VarAssignNode* va = (VarAssignNode*)node;

  Number* value = visitNode(va->value, filename, err, variables);

  if (!value) return NULL;
  
  setTable(variables, va->identifier, value);

  return value;
}

Number* visitNode(ASTNode* node, char *filename, Error** err, SymbolTable* variables) {
  if (!node || !filename || !err) return NULL;

  switch (node->type) {
    case NODE_NUMBER: return visitNumberNode(node, filename, err, variables); 
    case NODE_BINOP: return visitBinOpNode(node, filename, err, variables);
    case NODE_UNARYOP: return visitUnaryOpNode(node, filename, err, variables);
    case NODE_VARACCESS: return visitVarAccessNode(node, filename, err, variables);
    case NODE_VARASSIGN: return visitVarAssignNode(node, filename, err, variables);
    default: return NULL;
  }
}
