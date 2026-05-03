#include "../include/node.h"
#include "../include/token.h"
#include "../include/utils.h"

#include <stdlib.h>

NumberNode* initNumberNode(Token* token) {
  if (!token) return NULL;

  NumberNode* node = malloc(sizeof(NumberNode));
  
  if (!node) return NULL;

  node->base.type = NODE_NUMBER;
  node->token = token; 

  return node;
}

StringNode* initStringNode(Token* token) {
  if (!token) return NULL;
  
  StringNode* node = malloc(sizeof(StringNode));

  if (!node) return NULL;

  node->base.type = NODE_STRING;
  node->token = token;

  return node;
}

BinOpNode* initBinOpNode(ASTNode *leftNode, Token *operTok, ASTNode *rightNode) {
  if (!leftNode || !operTok || !rightNode) return NULL;

  BinOpNode* node = malloc(sizeof(BinOpNode));

  if (!node) return NULL;

  node->base.type = NODE_BINOP;

  node->leftNode = leftNode;
  node->operTok = operTok;
  node->rightNode = rightNode;

  return node;
}

UnaryOpNode* initUnaryOpNode(Token* operTok, ASTNode* node) {
  if (!operTok || !node) return NULL;

  UnaryOpNode* unaryNode = malloc(sizeof(UnaryOpNode));

  if (!unaryNode) return NULL;
  
  unaryNode->base.type = NODE_UNARYOP;
  unaryNode->operTok = operTok;
  unaryNode->node = node;

  return unaryNode;
}

VarAssignNode* initVarAssignNode(char *identifier, ASTNode* value) {
  if (!identifier || !value) return NULL;

  VarAssignNode* varAssignNode = malloc(sizeof(VarAssignNode));

  if (!varAssignNode) return NULL;

  varAssignNode->identifier = stringDup(identifier);
  
  if (!varAssignNode->identifier) {
    free(varAssignNode);
    return NULL;
  }
  
  varAssignNode->base.type = NODE_VARASSIGN;
  varAssignNode->value = value;

  return varAssignNode;
}

VarAccessNode* initVarAccessNode(Token* token) {
  if (!token) return NULL;

  VarAccessNode* node = malloc(sizeof(VarAccessNode));

  if (!node) return NULL;
  
  node->base.type = NODE_VARACCESS;
  node->token = token;
  return node;
}

void freeVarAssignNode(VarAssignNode* node) {
  if (!node) return;

  if (node->identifier) free(node->identifier);
  if (node->value) freeAST(node->value);

  free(node);
}

void freeVarAccessNode(VarAccessNode* node) {
  if (!node) return;

  free(node);
}

void freeStringNode(StringNode* node) {
  if (!node) return;

  free(node);
}

void freeNumberNode(NumberNode *node) {
    if (!node) return;

    free(node);
}

void freeBinOpNode(BinOpNode *node) {
    if (!node) return;

    freeAST(node->leftNode);
    freeAST(node->rightNode);

    free(node);
}

void freeUnaryOpNode(UnaryOpNode *node) {
    if (!node) return;

    freeAST(node->node);
    free(node);
}

void freeAST(ASTNode *node) {
  if (!node) return;

  switch (node->type) {
    case NODE_NUMBER:
      freeNumberNode((NumberNode*)node);
      break;

    case NODE_BINOP:
      freeBinOpNode((BinOpNode*)node);
      break;

    case NODE_UNARYOP:
      freeUnaryOpNode((UnaryOpNode*)node);
      break;

    case NODE_VARASSIGN:
      freeVarAssignNode((VarAssignNode*)node);
      break;

    case NODE_VARACCESS:
      freeVarAccessNode((VarAccessNode*)node);
      break;

    case NODE_STRING:
      freeStringNode((StringNode*)node);
      break;
  }
}
