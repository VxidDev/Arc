#include "../include/node.h"
#include "../include/interpretator.h"
#include "../include/object.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

Number* visitNumberNode(ASTNode* node) {
  NumberNode* numNode = (NumberNode*)node;

  if (!numNode->token) return NULL;
  
  if (strcmp(numNode->token->type, TOK_FLOAT) == 0) {
    return initFloat(*(double*)numNode->token->value);
  }

  return initInt(*(long*)numNode->token->value);
}

Number* visitBinOpNode(ASTNode* node) {
  BinOpNode* binOper = (BinOpNode*)node;

  Number *src = visitNode(binOper->leftNode);
  Number *dest = visitNode(binOper->rightNode);
  
  if (!src || !dest) return NULL;
  
  Number* output;

  if (strcmp(binOper->operTok->type, TOK_PLUS) == 0) {
    output = addNumber(dest, src);
  } else if (strcmp(binOper->operTok->type, TOK_MINUS) == 0) {
    output = subNumber(dest, src);
  } else if (strcmp(binOper->operTok->type, TOK_MUL) == 0) {
    output = mulNumber(dest, src);
  } else if (strcmp(binOper->operTok->type, TOK_DIV) == 0) {
    output = divNumber(dest, src);
  }

  free(dest);
  free(src);

  return output;
}

Number* visitUnaryOpNode(ASTNode* node) {
  UnaryOpNode* unaryOper = (UnaryOpNode*)node;

  Number* number = visitNode(unaryOper->node);

  if (!number) return NULL;
  
  Number* output = NULL;

  if (strcmp(unaryOper->operTok->type, TOK_MINUS) == 0) {
    output = mulNumber(number, initInt(-1));
    free(number);
  }

  return output;
}

Number* visitNode(ASTNode* node) {
  if (!node) return NULL;

  switch (node->type) {
    case NODE_NUMBER: return visitNumberNode(node); 
    case NODE_BINOP: return visitBinOpNode(node);
    case NODE_UNARYOP: return visitUnaryOpNode(node);
    default: return NULL;
  }
}
