#ifndef NODE_H
#define NODE_H

#include "token.h"
#include <stddef.h>

typedef enum {
  NODE_NUMBER,
  NODE_BINOP,
  NODE_UNARYOP,
  NODE_VARASSIGN,
  NODE_VARACCESS,
  NODE_STRING,
  NODE_PROGRAM,
  NODE_IF
} NodeType;

typedef struct ASTNode {
  NodeType type;
} ASTNode;

typedef struct NumberNode {
  ASTNode base;
  Token* token;
} NumberNode;

typedef struct StringNode {
  ASTNode base;
  Token* token;
} StringNode;

typedef struct VarAccessNode {
  ASTNode base;
  Token *token;
} VarAccessNode;

typedef struct VarAssignNode {
  ASTNode base;
  char *identifier;
  ASTNode *value;
} VarAssignNode;

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

typedef struct IfNode {
  ASTNode base;           
  ASTNode* condition;
  ASTNode* thenExpr;
  ASTNode** elifConds;   
  ASTNode** elifExprs; 
  size_t elifCount;
  ASTNode* elseExpr;   
} IfNode;

typedef struct {
  ASTNode base;          
  ASTNode **statements;  
  size_t count;         
} ProgramNode;

NumberNode* initNumberNode(Token* token);
StringNode* initStringNode(Token* token);
BinOpNode* initBinOpNode(ASTNode *leftNode, Token *operTok, ASTNode *rightNode);
UnaryOpNode* initUnaryOpNode(Token* operTok, ASTNode* node);
VarAccessNode* initVarAccessNode(Token* token);
VarAssignNode* initVarAssignNode(char *identifier, ASTNode* value);
IfNode* initIfNode(ASTNode* condition, ASTNode* thenExpr, ASTNode** elifConds, ASTNode** elifExprs, size_t elifCount, ASTNode* elseExpr);
ProgramNode* initProgramNode(ASTNode** statements, size_t count);

void freeBinOpNode(BinOpNode* node);
void freeNumberNode(NumberNode* node);
void freeUnaryOpNode(UnaryOpNode *node);
void freeAST(ASTNode* root);

#endif // NODE_H
