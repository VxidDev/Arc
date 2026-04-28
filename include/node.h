#ifndef NODE_H
#define NODE_H

#include "token.h"

typedef enum {
  NODE_NUMBER,
  NODE_BINOP,
  NODE_UNARYOP
} NodeType;

typedef struct ASTNode {
  NodeType type;
} ASTNode;

typedef struct NumberNode {
  ASTNode base;
  Token* token;
} NumberNode;

typedef struct BinOpNode {
  ASTNode base;

  ASTNode *leftNode;
  Token *operTok;
  ASTNode *rightNode;
} BinOpNode;

typedef struct UnaryOpNode {
  ASTNode base;

  Token *operTok;
  ASTNode* node;
} UnaryOpNode;

NumberNode* initNumberNode(Token* token);
BinOpNode* initBinOpNode(ASTNode *leftNode, Token *operTok, ASTNode *rightNode);
UnaryOpNode* initUnaryOpNode(Token* operTok, ASTNode* node);

void freeBinOpNode(BinOpNode* node);
void freeNumberNode(NumberNode* node);
void freeUnaryOpNode(UnaryOpNode *node);
void freeAST(ASTNode* root);

#endif // NODE_H
