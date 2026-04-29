#include "../include/node.h"
#include "../include/token.h"

#include <stdlib.h>

NumberNode* initNumberNode(Token* token) {
  if (!token) return NULL;

  NumberNode* node = malloc(sizeof(NumberNode));
  
  if (!node) return NULL;

  node->base.type = NODE_NUMBER;
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

  if (!node) return NULL;
  
  unaryNode->base.type = NODE_UNARYOP;
  unaryNode->operTok = operTok;
  unaryNode->node = node;

  return unaryNode;
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
  }
}
